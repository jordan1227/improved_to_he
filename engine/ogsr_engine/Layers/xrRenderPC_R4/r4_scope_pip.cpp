#include "stdafx.h"

#include "../../xr_3da/igame_persistent.h"
#include "../../xr_3da/igame_level.h"
#include "../../xr_3da/environment.h"
#include "../xrRender/FBasicVisual.h"
#include "ssfx_features.h"

// NLC: true PiP scope view ( docs/SCOPE_TRUE_PIP_DESIGN.md ).
//
// While the 3D scope lens is up in lens / hybrid mode and r_3dss_pip is 1, the world is drawn a second time from the
// main eye with the main view direction and a narrow square projection ( the lens magnification ), into a square
// set of targets sized to the lens on screen. 3dss_reticle.ps then samples that image instead of magnifying the main
// frame. Ideas from smoke-0n-the-water's "True PiP" ( TheLostInPlace's xray-monolith-pip line ), written for OGSR:
//
// - own traversal in a dsgraph context ( build_subspace, LOD and SSA at the scope FOV ), started after the main
//   dynamic traversal and joined in W1;
// - the scope set is swapped in for the main targets ( members and the views of their stock-named textures ), so
//   the existing phases and shaders work on it; Device projection / FOV / ViewBase and the TAA jitter are swapped
//   only inside main-thread windows where no render worker reads them:
//     W1 after the sun accumulation: sky, G-buffer, rain wetness, sun ( kept cascades ), emissive;
//     W2 in render_lights: each batch of lights again, while the spot atlas holds that batch;
//     W3 before the main combine: clouds, combine_1, haze, forward, tonemap with last frame's exposure;
// - AO / IL / screen-space shadows / water reflections read as neutral ( ssfx_compare constant ) in the scope.

namespace
{
constexpr float PIP_BASE_FOV = 67.f; // BASE_FOV of the LOD rules ( r__dsgraph_build.cpp )
constexpr float PIP_E_MIN = 0.08f;
constexpr float PIP_E_MAX = 1.2f;
constexpr float PIP_PUDDLES_RANGE_SQR = 150.f * 150.f; // as PUDDLES_RENDER_RANGE_SQR ( r4_R_render.cpp )

float pip_clamp_e(const float e) { return std::clamp(e, PIP_E_MIN, PIP_E_MAX); }

// image side in px: the lens disc ( with its margin ) at display size x the resolution share x supersample
u32 pip_side(const float E)
{
    const float disp_h = static_cast<float>(RImplementation.Target->GetDisplayHeight());
    const float ss = std::clamp(ps_r_3dss_pip_ss, 1.f, 2.f);
    float s = E * disp_h * std::clamp(ps_r_3dss_pip_res, 0.5f, 1.f) * ss;
    s = std::clamp(s, 256.f, std::max(256.f, disp_h * ss));
    return (static_cast<u32>(s) + 31u) & ~31u;
}
} // namespace

//////////////////////////////////////////////////////////////////////////
// CRenderTarget: the scope target set

u32 CRenderTarget::pip_want_flags() const
{
    u32 f{};
    if (ps_r_3dss_pip_ao && rt_ssfx_ao && rt_ssfx_prev_pos)
        f |= PIP_T_AO;
    if (ps_r_3dss_pip_volumetric && (rt_ssfx_vol_sun || (rt_Generic_2 && rt_Generic_3)))
        f |= PIP_T_VOL;
    if (ps_r_3dss_pip_ssr && rt_ssfx_ssr && rt_ssfx_ssr_b && rt_ssfx_ssr_hist && rt_ssfx_prev_pos)
        f |= PIP_T_SSR;
    return f;
}

bool CRenderTarget::pip_create(const u32 size, const u32 flags)
{
    R_ASSERT(!m_pip_in);
    pip_destroy();

    const auto make_wh = [](ref_rt& rt, LPCSTR stock, const u32 w, const u32 h, const DXGI_FORMAT f) {
        string256 name;
        xr_sprintf(name, "%s$pip", stock);
        rt.create(name, std::max(w, 1u), std::max(h, 1u), f);
        return rt && rt->pSurface && rt->pTexture && (rt->pRT || rt->pZRT[CHW::IMM_CTX_ID]);
    };
    const auto make = [&](ref_rt& rt, LPCSTR stock, const DXGI_FORMAT f) { return make_wh(rt, stock, size, size, f); };

    bool ok = make(m_pip.depth, r2_RT_base_depth, DXGI_FORMAT_R24G8_TYPELESS) && make(m_pip.position, r2_RT_P, DXGI_FORMAT_R16G16B16A16_FLOAT) &&
        make(m_pip.color, r2_RT_albedo, DXGI_FORMAT_R8G8B8A8_UNORM) && make(m_pip.accum, r2_RT_accum, DXGI_FORMAT_R16G16B16A16_FLOAT) &&
        make(m_pip.generic0, r2_RT_generic0, DXGI_FORMAT_R16G16B16A16_FLOAT) && make(m_pip.generic0_temp, r2_RT_generic0_temp, DXGI_FORMAT_R16G16B16A16_FLOAT) &&
        make(m_pip.zbuffer, r2_RT_zbuffer, DXGI_FORMAT_R24G8_TYPELESS) && make(m_pip.scratch, "$user$generic_scene_scratch", DXGI_FORMAT_R16G16B16A16_FLOAT);

    // B2: the optional targets, in the main targets' formats and size ratios ( docs/SCOPE_TRUE_PIP_DESIGN.md 15 )
    if (ok && rt_Generic_1 && RImplementation.o.distortion_enabled)
        ok = make(m_pip.generic1, r2_RT_generic1, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (ok && rt_ssfx_fog_4 && rt_ssfx_fog_2)
        ok = make_wh(m_pip.fog4, r2_RT_ssfx_fog_4, size / 4, size / 4, DXGI_FORMAT_R16G16B16A16_FLOAT) &&
            make_wh(m_pip.fog2, r2_RT_ssfx_fog_2, size / 2, size / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
    if (ok && rt_ssfx_rain)
        ok = make_wh(m_pip.rain, r2_RT_ssfx_rain, size / 8, size / 8, DXGI_FORMAT_R16G16B16A16_FLOAT);
    if (ok && rt_nlc_sky_bg) // NLC .237: fixed size, sampled by screen uv ( any aspect )
        ok = make_wh(m_pip.sky_bg, r2_RT_nlc_sky_bg, 80, 45, DXGI_FORMAT_R16G16B16A16_FLOAT);
    if (ok && (flags & PIP_T_AO))
    {
        // at least half size: the scope image is supersampled ( r_3dss_pip_ss ), half of it is about display density
        const u32 a = size / static_cast<u32>(std::clamp(ps_ssfx_ao_res, 2, 4));
        ok = make_wh(m_pip.ao, r2_RT_ssfx_ao, a, a, DXGI_FORMAT_R16G16B16A16_FLOAT) && make_wh(m_pip.ao_a, r2_RT_ssfx_ao_a, a, a, DXGI_FORMAT_R16G16B16A16_FLOAT) &&
            make_wh(m_pip.ao_b, r2_RT_ssfx_ao_b, a, a, DXGI_FORMAT_R16G16B16A16_FLOAT);
    }
    if (ok && (flags & PIP_T_SSR))
    {
        const u32 s = size / 2; // half size always ( .222 test: the scope SSR cost ~8 FPS in rain at the main divider )
        ok = make_wh(m_pip.ssr, r2_RT_ssfx_ssr, s, s, DXGI_FORMAT_R16G16B16A16_FLOAT) && make_wh(m_pip.ssr_b, r2_RT_ssfx_ssr_b, s, s, DXGI_FORMAT_R16G16B16A16_FLOAT) &&
            make_wh(m_pip.ssr_hist, r2_RT_ssfx_ssr_hist, s, s, DXGI_FORMAT_R16G16B16A16_FLOAT);
    }
    // previous position buffer and motion vectors: the temporal passes ( AO, SSR ) of the scope view
    if (ok && (flags & (PIP_T_AO | PIP_T_SSR)))
    {
        ok = make(m_pip.prev_pos, r2_RT_ssfx_prev_pos, DXGI_FORMAT_R16G16B16A16_FLOAT);
        if (ok && rt_Velocity)
            ok = make(m_pip.velocity, r2_RT_velocity, DXGI_FORMAT_R16G16_FLOAT);
    }
    if (ok && (flags & PIP_T_VOL))
    {
        if (rt_ssfx_vol_sun)
        {
            const u32 v = size / static_cast<u32>(std::clamp(ps_ssfx_volumetric_res, 2, 4));
            ok = make_wh(m_pip.vol_sun, r2_RT_ssfx_vol_sun, v, v, DXGI_FORMAT_R16G16B16A16_FLOAT);
        }
        else
            ok = make(m_pip.generic2, r2_RT_generic2, DXGI_FORMAT_R16G16B16A16_FLOAT) && make(m_pip.generic3, r2_RT_generic3, DXGI_FORMAT_R16G16B16A16_FLOAT);
    }

    if (!ok)
    {
        pip_destroy();
        return false;
    }

    m_pip.size = size;
    m_pip.flags = flags;
    m_pip_accum_mark = 0;
    m_pip_light_marker = 5;
    m_pip_history = false;
    return true;
}

void CRenderTarget::pip_destroy()
{
    R_ASSERT(!m_pip_in);
    m_pip.depth.destroy();
    m_pip.position.destroy();
    m_pip.color.destroy();
    m_pip.accum.destroy();
    m_pip.generic0.destroy();
    m_pip.generic0_temp.destroy();
    m_pip.zbuffer.destroy();
    m_pip.scratch.destroy();
    m_pip.generic1.destroy();
    m_pip.ao.destroy();
    m_pip.ao_a.destroy();
    m_pip.ao_b.destroy();
    m_pip.prev_pos.destroy();
    m_pip.fog4.destroy();
    m_pip.fog2.destroy();
    m_pip.rain.destroy();
    m_pip.vol_sun.destroy();
    m_pip.generic2.destroy();
    m_pip.generic3.destroy();
    m_pip.velocity.destroy();
    m_pip.ssr.destroy();
    m_pip.ssr_b.destroy();
    m_pip.ssr_hist.destroy();
    m_pip.sky_bg.destroy();
    m_pip.size = 0;
    m_pip.flags = 0;
    m_pip_history = false;
}

void CRenderTarget::pip_swap(CBackend& cmd_list_imm, const bool in)
{
    if (in == m_pip_in)
        return;

    // member <-> scope target, and the stock-named texture ( what shaders bind ) <-> the scope texture's views;
    // the same exchange both ways
    const auto exchange = [](ref_rt& member, ref_rt& other) {
        member->pTexture->swap_views(*other->pTexture);
        std::swap(member, other);
    };
    // optional scope targets ( B2 ): created only where the main one exists, so both or neither are set
    const auto exchange_opt = [&exchange](ref_rt& member, ref_rt& other) {
        if (member && other)
            exchange(member, other);
    };

    const u32 imm = cmd_list_imm.context_id;

    if (in)
    {
        m_pip_saved = {m_renderWidth, m_renderHeight, m_displayWidth, m_displayHeight, dwWidth[imm], dwHeight[imm], dwAccumulatorClearMark, dwLightMarkerID, m_bHasActiveVolumetric};

        m_renderWidth = m_renderHeight = m_displayWidth = m_displayHeight = m_pip.size;
        dwAccumulatorClearMark = m_pip_accum_mark;
        dwLightMarkerID = m_pip_light_marker;
        m_bHasActiveVolumetric = m_pip_vol && m_pip_vol_active;
    }

    exchange(rt_Base_Depth, m_pip.depth);
    exchange(rt_Position, m_pip.position);
    exchange(rt_Color, m_pip.color);
    exchange(rt_Accumulator, m_pip.accum);
    exchange(rt_Generic_0, m_pip.generic0);
    exchange(rt_Generic_0_temp, m_pip.generic0_temp);
    exchange(rt_zbuffer, m_pip.zbuffer);
    exchange(rt_Generic_scene_scratch, m_pip.scratch);
    exchange_opt(rt_Generic_1, m_pip.generic1);
    exchange_opt(rt_ssfx_ao, m_pip.ao);
    exchange_opt(rt_ssfx_ao_a, m_pip.ao_a);
    exchange_opt(rt_ssfx_ao_b, m_pip.ao_b);
    exchange_opt(rt_ssfx_prev_pos, m_pip.prev_pos);
    exchange_opt(rt_ssfx_fog_4, m_pip.fog4);
    exchange_opt(rt_ssfx_fog_2, m_pip.fog2);
    exchange_opt(rt_ssfx_rain, m_pip.rain);
    exchange_opt(rt_ssfx_vol_sun, m_pip.vol_sun);
    exchange_opt(rt_Generic_2, m_pip.generic2);
    exchange_opt(rt_Generic_3, m_pip.generic3);
    exchange_opt(rt_Velocity, m_pip.velocity);
    exchange_opt(rt_ssfx_ssr, m_pip.ssr);
    exchange_opt(rt_ssfx_ssr_b, m_pip.ssr_b);
    exchange_opt(rt_ssfx_ssr_hist, m_pip.ssr_hist);
    exchange_opt(rt_nlc_sky_bg, m_pip.sky_bg); // NLC .237

    if (!in)
    {
        m_pip_accum_mark = dwAccumulatorClearMark;
        m_pip_light_marker = dwLightMarkerID;
        m_pip_vol_active = m_pip_vol && m_bHasActiveVolumetric;

        m_renderWidth = m_pip_saved.render_w;
        m_renderHeight = m_pip_saved.render_h;
        m_displayWidth = m_pip_saved.display_w;
        m_displayHeight = m_pip_saved.display_h;
        dwWidth[imm] = m_pip_saved.view_w;
        dwHeight[imm] = m_pip_saved.view_h;
        dwAccumulatorClearMark = m_pip_saved.accum_mark;
        dwLightMarkerID = m_pip_saved.light_marker;
        m_bHasActiveVolumetric = m_pip_saved.volumetric;
    }

    m_pip_in = in;
}

void CRenderTarget::phase_pip_combine(CBackend& cmd_list, R_dsgraph_structure& scope, const Fmatrix& m_previous)
{
    PIX_EVENT(phase_pip_combine);

    // exposure: the adapted luminance the main view used last frame ( rt_LUM_pool[0] after the swap at the end of the
    // main combine ); this frame's main pass rebinds both textures
    t_LUM_dest->surface_set(rt_LUM_pool[0]->pSurface);

    // temporal passes of the scope view start from a cleared previous position after a gap
    if (m_pip.prev_pos && !m_pip_history)
        cmd_list.ClearRT(rt_ssfx_prev_pos, {});

    // B2.4: ambient occlusion of the scope view ( its own history ), read by combine_1 through ssfx_compare.y
    ps_r_3dss_pip_ao_live = false;
    if (pip_ao())
    {
        phase_pip_ao(cmd_list, Device.mProject, m_previous);
        ps_r_3dss_pip_ao_live = true;
    }

    // ( rmNormal after u_setrt: it reads the size of the bound target, and the AO's last target is reduced-size )
    u_setrt(cmd_list, rt_Generic_0, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[cmd_list.context_id]);
    RImplementation.rmNormal(cmd_list);
    cmd_list.set_CullMode(CULL_NONE);
    cmd_list.set_Stencil(FALSE);

    g_pGamePersistent->Environment().RenderClouds(cmd_list);

    phase_nlc_sky_bg(cmd_list); // NLC .237: the scope view's sky + clouds ( its own target, swapped in ), fog colour

    cmd_list.set_Stencil(TRUE, D3DCMP_LESSEQUAL, 0x01, 0xff, 0x00); // stencil should be >= 1
    draw_combine_1(cmd_list);

    // refraction source of glass and water
    HW.get_context(cmd_list.context_id)->CopyResource(rt_Generic_0_temp->pSurface, rt_Generic_0->pSurface);

    // wet-surface reflections of the scope view ( its own history ); glass refracts the scene with them, as in the
    // main combine
    if (pip_ssr() && ps_r2_ls_flags_ext.test(R2FLAGEXT_SSLR) && phase_pip_ssr(cmd_list, Device.mProject, m_previous))
    {
        if (ssfx_glass_enabled())
            HW.get_context(cmd_list.context_id)->CopyResource(rt_Generic_0_temp->pSurface, rt_Generic_0->pSurface);
    }

    // history of the next scope frame ( AO, SSR ): this frame's positions
    if (m_pip.prev_pos && (pip_ao() || pip_ssr()))
    {
        HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_prev_pos->pSurface, rt_Position->pSurface);
        m_pip_history = true;
    }
    else
        m_pip_history = false; // AO / SSR come back with a fresh history

    // B2.3: rain refraction buffer of the scope image ( no-op when dry )
    phase_ssfx_rain(cmd_list);

    if (ps_ssfx_haze_order == 0)
        phase_ssfx_haze(cmd_list);

    // B2.3: rain streaks, splashes, impact splashes and thunderbolts, drawn only ( the main pass steps them ); on the
    // immediate context ( dynamic vertex stream ), before the forward list is submitted, as in render_forward
    {
        u_setrt(cmd_list, rt_Generic_0, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[cmd_list.context_id]);
        RImplementation.rmNormal(cmd_list);
        cmd_list.set_CullMode(CULL_CCW);
        cmd_list.set_Stencil(FALSE);
        cmd_list.set_ColorWriteEnable();
        cmd_list.set_xform_world(Fidentity);
        cmd_list.set_xform_view(Device.mView);
        cmd_list.set_xform_project(Device.mProject);
        g_pGamePersistent->Environment().RenderLastScope(cmd_list);

        // rain puddles ( as render_forward )
        RImplementation.pip_render_puddles(cmd_list);
    }

    // forward geometry of the scope traversal ( water, glass, particles ), recorded in its context: no HUD
    {
        CBackend& sc = scope.cmd_list;
        sc.Invalidate();
        u_setrt(sc, rt_Generic_0, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[sc.context_id]);
        RImplementation.rmNormal(sc);
        sc.set_CullMode(CULL_CCW);
        sc.set_Stencil(FALSE);
        sc.set_ColorWriteEnable();
        sc.set_xform_world(Fidentity);
        sc.set_xform_view(Device.mView);
        sc.set_xform_project(Device.mProject);

        scope.r_dsgraph_render_graph(1);
        scope.r_dsgraph_render_sorted();

        sc.submit();
        sc.Invalidate();

        cmd_list.Invalidate();
        cmd_list.set_xform_world(Fidentity);
        cmd_list.set_xform_view(Device.mView);
        cmd_list.set_xform_project(Device.mProject);
        // the submit cleared the immediate context's targets and viewport ( the volumetric combine sets no viewport )
        u_setrt(cmd_list, rt_Generic_0, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[cmd_list.context_id]);
        RImplementation.rmNormal(cmd_list);
    }

    if (ps_ssfx_haze_order == 1 && phase_ssfx_haze(cmd_list))
    {
        u_setrt(cmd_list, rt_Generic_0, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[cmd_list.context_id]);
        RImplementation.rmNormal(cmd_list);
    }

    // NLC: distant rain of the scope view ( docs/RAIN_CLOUDS_DESIGN.md )
    if (phase_nlc_rain_far(cmd_list))
    {
        u_setrt(cmd_list, rt_Generic_0, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[cmd_list.context_id]);
        RImplementation.rmNormal(cmd_list);
    }

    // B2.5: volumetric sun / lamp beams of the scope view ( r_3dss_pip_volumetric )
    if (m_pip_vol && m_bHasActiveVolumetric)
    {
        phase_combine_volumetric(cmd_list);
        RImplementation.rmNormal(cmd_list);
    }

    // B2.2: heat haze of anomalies, fires and exhausts: the scope traversal's distortion objects into the scope mask
    // ( recorded in its context ), then the distortion combine
    if (rt_Generic_1 && m_pip.generic1 && RImplementation.o.distortion_enabled && !scope.mapDistort.empty())
    {
        {
            CBackend& sc = scope.cmd_list;
            sc.Invalidate();
            u_setrt(sc, rt_Generic_1, nullptr, nullptr, nullptr, rt_Base_Depth->pZRT[sc.context_id]);
            RImplementation.rmNormal(sc);
            sc.ClearRT(rt_Generic_1, color_rgba(127, 127, 0, 127));
            sc.set_CullMode(CULL_CCW);
            sc.set_Stencil(FALSE);
            sc.set_ColorWriteEnable();
            sc.set_xform_world(Fidentity);
            sc.set_xform_view(Device.mView);
            sc.set_xform_project(Device.mProject);
            scope.r_dsgraph_render_distort();
            sc.submit();
            sc.Invalidate();
        }

        cmd_list.Invalidate();
        cmd_list.set_xform_world(Fidentity);
        cmd_list.set_xform_view(Device.mView);
        cmd_list.set_xform_project(Device.mProject);
        RenderScreenTriangle(cmd_list, rt_Generic_scene_scratch, s_combine->E[1]);
        HW.get_context(cmd_list.context_id)->CopyResource(rt_Generic_0->pSurface, rt_Generic_scene_scratch->pSurface);
    }

    // B2.6: fog scattering glow ( HDR, before the tonemap; no-op in clear weather )
    phase_ssfx_fog_scattering(cmd_list);
    RImplementation.rmNormal(cmd_list);

    // tonemap ( combine_tonemap reads $user$tonemap )
    RenderScreenTriangle(cmd_list, rt_Generic_scene_scratch, s_combine->E[2]);
    HW.get_context(cmd_list.context_id)->CopyResource(rt_Generic_0->pSurface, rt_Generic_scene_scratch->pSurface);

    t_LUM_dest->surface_set(nullptr);
}

void CRenderTarget::pip_composite_begin()
{
    if (!rt_Generic_combine_scope || !m_pip.generic0 || m_pip_in)
        return;

    rt_Generic_combine_scope->pTexture->swap_views(*m_pip.generic0->pTexture);
    ps_r_3dss_pip_params.x = 1.f;
}

void CRenderTarget::pip_composite_end()
{
    if (ps_r_3dss_pip_params.x < 0.5f)
        return;

    rt_Generic_combine_scope->pTexture->swap_views(*m_pip.generic0->pTexture);
    ps_r_3dss_pip_params.x = 0.f;
}

//////////////////////////////////////////////////////////////////////////
// CRender: schedule

// Largest lens disc of the HUD lens items ( 3D scope lens shaders, Render_MAIN of this frame ): E = margin x ( centre
// offset + radius ) in main NDC height units. A new optic takes its measurement at once; otherwise E is learned once
// per aim when fully raised, so the targets are never resized mid-aim more than once.
void CRender::pip_measure_lens()
{
    float best_r{}, best_off{};
    const void* key{};

    const float aspect = static_cast<float>(Target->GetDisplayWidth()) / static_cast<float>(Target->GetDisplayHeight());
    auto& map = get_imm_context().mapScopeHUDSorted;
    for (auto& node : map)
    {
        const R_dsgraph::_MatrixItemS& it = node.second;
        if (!it.pVisual)
            continue;

        const vis_data& v = it.pVisual->getVisData();
        Fvector c_w, c_v, c_n;
        it.Matrix.transform_tiny(c_w, v.sphere.P);
        Device.mView_hud.transform_tiny(c_v, c_w);
        if (c_v.z < 0.01f)
            continue;

        const float scale = it.Matrix.i.magnitude();
        const float r = v.sphere.R * scale * Device.mProject_hud._22 / c_v.z;
        Device.mFullTransform_hud.transform(c_n, c_w);
        const float off = _sqrt(_sqr(c_n.x * aspect) + _sqr(c_n.y));
        if (r > best_r && _valid(r) && _valid(off))
        {
            best_r = r;
            best_off = off;
            key = it.pVisual;
        }
    }

    const float E_meas = best_r > 0.f ? std::clamp(ps_r_3dss_pip_margin, 1.f, 1.5f) * (best_off + best_r) : 0.f;

    const float raise = Device.m_ScopePip.raise;
    if (!m_pip.was_on)
        m_pip.settled = false;
    if (key && key != m_pip.lens_key && E_meas > 0.f)
    {
        m_pip.lens_key = key;
        m_pip.E = pip_clamp_e(E_meas);
        m_pip.settled = false;
    }
    if (!m_pip.settled && raise > 0.999f && E_meas > 0.f)
    {
        if (_abs(E_meas - m_pip.E) > 0.08f * m_pip.E)
            m_pip.E = pip_clamp_e(E_meas);
        m_pip.settled = true;
    }
}

void CRender::pip_start()
{
    m_pip.frame_on = false;
    m_pip.ready = false;
    ps_r_3dss_pip_params.set(0.f, 0.f, 0.f, 0.f);

    const auto& req = Device.m_ScopePip;
    const bool heat = ps_pnv_mode == 2 || ps_pnv_mode == 3;
    const bool want = ps_r_3dss_pip && req.active && req.frame == Device.dwFrame && !heat && g_pGameLevel && !get_imm_context().mapScopeHUDSorted.empty();
    ps_r_3dss_pip_sun.set(0.f, 0.f, 0.f, 0.f);
    if (!want)
    {
        if (m_pip.was_on && ps_r_3dss_pip_debug)
            Msg("[3DSS-PIP] off");
        m_pip.was_on = false;
        m_pip.prev_valid = false;
        return;
    }

    const bool edge = !m_pip.was_on;
    pip_measure_lens();
    m_pip.was_on = true;
    // AO / SSR history, previous position and projection are only from the frame before ( not after a new aim, a
    // skipped scope frame or a camera cut that reset the main history )
    if (edge || m_pip.done_frame + 1 != Device.dwFrame || !Target->ssfx_history_valid)
    {
        m_pip.prev_valid = false;
        Target->m_pip_history = false;
    }

    // target set
    const u32 side = pip_side(m_pip.E);
    const u32 flags = Target->pip_want_flags();
    if (Target->m_pip.size != side || Target->m_pip.flags != flags)
    {
        CTimer t;
        t.Start();
        if (!Target->pip_create(side, flags))
        {
            if (!m_pip.fail_log_frame || m_pip.fail_log_frame + 300 < Device.dwFrame)
            {
                m_pip.fail_log_frame = Device.dwFrame;
                Msg("! [3DSS-PIP] scope targets %ux%u could not be created ( VRAM? ), the lens uses the frame", side, side);
            }
            return;
        }
        if (ps_r_3dss_pip_debug)
            Msg("[3DSS-PIP] targets %ux%u ( E %.3f ), ao %s, volumetric %s, distortion %s, fog %s, rain %s; %.1f ms", side, side, m_pip.E,
                flags & CRenderTarget::PIP_T_AO ? "yes" : "no", flags & CRenderTarget::PIP_T_VOL ? "yes" : "no", Target->m_pip.generic1 ? "yes" : "no",
                Target->m_pip.fog4 ? "yes" : "no", Target->m_pip.rain ? "yes" : "no", t.GetElapsed_sec() * 1000.f);
        m_pip.prev_valid = false;
    }
    m_pip.size = side;
    // with ssfx_compare the volumetric code uses the full generic2/3 pair, which a reduced scope set does not have
    // ( it would write the main view's beams ): no scope volumetrics then
    Target->m_pip_vol = (Target->m_pip.flags & CRenderTarget::PIP_T_VOL) != 0 && !ps_ssfx_compare;

    if (free_contexts() == 0)
    {
        if (!m_pip.fail_log_frame || m_pip.fail_log_frame + 300 < Device.dwFrame)
        {
            m_pip.fail_log_frame = Device.dwFrame;
            Msg("! [3DSS-PIP] no free render context, the lens uses the frame");
        }
        return;
    }

    // camera: main eye and view, square projection at the lens magnification over the image extent
    m_pip.mag = std::max(req.mag, 1.f);
    const float cot_s = m_pip.mag * Device.mProject._22 / m_pip.E;
    m_pip.project = Device.mProject;
    m_pip.project._11 = cot_s;
    m_pip.project._22 = cot_s;
    m_pip.project._31 = 0.f;
    m_pip.project._32 = 0.f;
    m_pip.full.mul(m_pip.project, Device.mView);
    m_pip.inv_full.invert_44(m_pip.full);
    m_pip.fov_deg = rad2deg(2.f * atanf(1.f / cot_s));
    m_pip.frustum.CreateFromMatrix(m_pip.full, FRUSTUM_P_LRTB + FRUSTUM_P_FAR);

    // LOD / small-object cull at the scope's own FOV and pixel density ( r_3dss_pip_lod 0 = as the main view )
    const float lod = std::clamp(ps_r_3dss_pip_lod, 0.f, 1.f);
    const float fov_main = Device.fFOV;
    m_pip.fov_k = (PIP_BASE_FOV / fov_main) * (1.f - lod) + (PIP_BASE_FOV / m_pip.fov_deg) * lod;
    const float px_main = static_cast<float>(Device.dwWidth) * static_cast<float>(Device.dwHeight);
    const float k_full = (static_cast<float>(side) * static_cast<float>(side) / px_main) * _sqr(fov_main / m_pip.fov_deg);
    m_pip.ssa_k = std::clamp(1.f + (k_full - 1.f) * lod, 0.25f, 1000.f);

    m_pip.ctx = alloc_context();
    R_dsgraph_structure& ds = get_context(m_pip.ctx);
    m_pip.dsgraph = &ds;
    ds.phase = PHASE_NORMAL;
    ds.r_pmask(true, true, true); // B2.1: level decals ( mapWmark ) too
    ds.main_pass = false;
    ds.nlc_fov_k = m_pip.fov_k;
    ds.nlc_ssa_k = m_pip.ssa_k;

    const float aspect = static_cast<float>(Target->GetDisplayWidth()) / static_cast<float>(Target->GetDisplayHeight());
    ps_r_3dss_pip_params.set(0.f, m_pip.E, aspect, 1.f / static_cast<float>(side));

    // B3.5: sun glare in the eyepiece. Sun position in the scope image ( tc, may lie outside it ) and the glare amount:
    // sun brightness x its visibility ( the lens flare's ray test of last frame ) x closeness to the scope axis, full
    // within the image half-angle and gone at 4x it
    {
        const CEnvDescriptorMixer& env = *g_pGamePersistent->Environment().CurrentEnv;
        Fvector to_sun;
        to_sun.invert(env.sun_dir);
        to_sun.normalize_safe();
        Fvector v;
        Device.mView.transform_dir(v, to_sun);
        if (v.z > 0.05f)
        {
            extern float ps_lens_flare_sun_blend;
            const float tx = 0.5f + 0.5f * v.x / v.z * cot_s;
            const float ty = 0.5f - 0.5f * v.y / v.z * cot_s;
            const float half = atanf(1.f / cot_s);
            const float angle = acosf(std::clamp(v.z, -1.f, 1.f));
            const float closeness = 1.f - std::clamp((angle - half) / (3.f * half), 0.f, 1.f);
            const Fvector& c = env.sun_color;
            const float lum = std::clamp((c.x * 0.3f + c.y * 0.59f + c.z * 0.11f) * 1.5f, 0.f, 1.f);
            const float amount = lum * std::clamp(ps_lens_flare_sun_blend, 0.f, 1.f) * closeness * closeness;
            if (amount > 0.001f)
                ps_r_3dss_pip_sun.set(tx, ty, amount, 0.f);
        }
    }

    // traversal on a worker: captured camera only ( the main thread swaps the Device matrices for the HUD meanwhile )
    {
        R_dsgraph_structure* d = &ds;
        const IRender_Sector::sector_id_t sector = last_sector_id;
        CFrustum frustum = m_pip.frustum;
        const Fmatrix full = m_pip.full;
        const Fvector cam = Device.vCameraPosition;
        m_pip.traversal = TTAPI->submit([d, sector, frustum, full, cam]() {
            ZoneScopedN("pip_traversal");
            CFrustum f = frustum; // build_subspace takes it by reference ( the pool calls tasks as const )
            d->build_subspace(sector, f, full, cam, TRUE);
        });
        m_pip.traversal_started = true;
    }

    m_pip.frame_on = true;

    if (edge && ps_r_3dss_pip_debug)
        Msg("[3DSS-PIP] on: image %ux%u, E %.3f, mag %.2f, fov %.2f ( main %.1f ), lod fov_k %.2f ssa_k %.1f", side, side, m_pip.E, m_pip.mag, m_pip.fov_deg, fov_main,
            m_pip.fov_k, m_pip.ssa_k);
}

void CRender::pip_enter()
{
    R_ASSERT(!m_pip.in);

    m_pip.s_project = Device.mProject;
    m_pip.s_full = Device.mFullTransform;
    m_pip.s_inv_full = Device.mInvFullTransform;
    m_pip.s_fov = Device.fFOV;
    m_pip.s_aspect = Device.fASPECT;
    m_pip.s_view = ViewBase;
    m_pip.s_jitter = ps_r_taa_jitter;
    m_pip.s_jitter_full = ps_r_taa_jitter_full;

    Device.mProject = m_pip.project;
    Device.mFullTransform = m_pip.full;
    Device.mInvFullTransform = m_pip.inv_full;
    Device.fFOV = m_pip.fov_deg;
    Device.fASPECT = 1.f;
    ViewBase = m_pip.frustum;
    ps_r_taa_jitter.x = 0.f;
    ps_r_taa_jitter.y = 0.f;
    ps_r_taa_jitter_full.set(0.f, 0.f);

    CBackend& imm = get_imm_context().cmd_list;
    Target->pip_swap(imm, true);
    ps_r_3dss_pip_pass = true;

    imm.Invalidate();
    imm.set_xform_world(Fidentity);
    imm.set_xform_view(Device.mView);
    imm.set_xform_project(Device.mProject);

    m_pip.in = true;
}

void CRender::pip_leave()
{
    R_ASSERT(m_pip.in);

    CBackend& imm = get_imm_context().cmd_list;
    ps_r_3dss_pip_pass = false;
    Target->pip_swap(imm, false);

    Device.mProject = m_pip.s_project;
    Device.mFullTransform = m_pip.s_full;
    Device.mInvFullTransform = m_pip.s_inv_full;
    Device.fFOV = m_pip.s_fov;
    Device.fASPECT = m_pip.s_aspect;
    ViewBase = m_pip.s_view;
    ps_r_taa_jitter = m_pip.s_jitter;
    ps_r_taa_jitter_full = m_pip.s_jitter_full;

    imm.Invalidate();
    imm.set_xform_world(Fidentity);
    imm.set_xform_view(Device.mView);
    imm.set_xform_project(Device.mProject);
    imm.set_xform_view_old(Device.mView_old); // the scope G-buffer set the scope's ( HUDInitializer restores the same )
    imm.set_xform_project_old(Device.mProject_old);
    Target->u_setrt(imm, Target->rt_Accumulator, nullptr, nullptr, nullptr, Target->get_base_zb());
    rmNormal(imm);

    m_pip.in = false;
}

void CRender::pip_render_gbuffer(const bool bSUN)
{
    if (!m_pip.frame_on)
        return;

    ZoneScopedN("pip_gbuffer");
    PIX_EVENT(PIP_GBUFFER);

    CTimer t;
    t.Start();
    if (m_pip.traversal_started)
    {
        m_pip.traversal.wait();
        m_pip.traversal_started = false;
    }
    m_pip.ms_trav += t.GetElapsed_sec() * 1000.f;
    t.Start();

    R_dsgraph_structure& ds = *m_pip.dsgraph;
    CBackend& sc = ds.cmd_list;
    CBackend& imm = get_imm_context().cmd_list;

    Target->m_pip_vol_active = false; // the scope's volumetric targets are cleared at their first draw this frame
    pip_enter();

    {
        const Fcolor black{};
        imm.ClearZB(Target->get_base_zb(), 1.0f, 0);
        imm.ClearRT(Target->rt_Generic_0->pRT, black);
        imm.ClearRT(Target->rt_Position->pRT, black);
        imm.ClearRT(Target->rt_Color->pRT, black);
        imm.ClearRT(Target->rt_Accumulator->pRT, black);
        if (Target->m_pip.velocity) // the scope's motion vectors ( swapped in ): sky = no motion
            imm.ClearRT(Target->rt_Velocity->pRT, black);
    }

    // sky on the immediate context ( it fills the shared dynamic vertex stream, which a deferred list reads only later )
    imm.set_xform_world_old(Fidentity);
    Target->u_setrt(imm, Target->rt_Generic_0, nullptr, nullptr, nullptr, Target->get_base_zb());
    imm.set_CullMode(CULL_NONE);
    imm.set_Stencil(FALSE);
    g_pGamePersistent->Environment().RenderSky(imm);

    // phase_scene_begin without the heat target ( no heat vision in the scope ); motion vectors when the scope set has them
    // ( AO / SSR history ), relative to the last scope frame: main eye of last frame, last scope projection
    // ( rmNormal: a deferred list starts without a viewport, and the immediate context loses its own after each submit;
    // a main-size viewport here drew the scope image stretched and anchored at its corner, 2026-11-25 test )
    const Fmatrix& project_old = m_pip.prev_valid ? m_pip.prev_project : Device.mProject;
    const bool velocity = static_cast<bool>(Target->m_pip.velocity);
    const auto scene_begin = [this, velocity, &project_old](CBackend& cl) {
        Target->u_setrt(cl, Target->rt_Position, Target->rt_Color, velocity ? Target->rt_Velocity : ref_rt{}, nullptr, Target->rt_Base_Depth->pZRT[cl.context_id]);
        cl.set_xform_view_old(Device.mView_old);
        cl.set_xform_project_old(project_old);
        rmNormal(cl);
        cl.set_Stencil(TRUE, D3DCMP_ALWAYS, 0x01, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP);
        cl.set_CullMode(CULL_CCW);
        cl.set_ColorWriteEnable();
    };

    // G-buffer recorded in the scope context ( its PM LODs and grass instances are per context ), like the shadow passes
    sc.set_xform_world(Fidentity);
    sc.set_xform_view(Device.mView);
    sc.set_xform_project(Device.mProject);
    sc.set_xform_world_old(Fidentity);
    scene_begin(sc);
    ds.r_dsgraph_render_graph_static(0);
    ds.r_dsgraph_render_graph_dynamic(0);
    if (Details)
        Details->Render(sc);

    sc.submit();
    sc.Invalidate();
    imm.Invalidate();
    imm.set_xform_world(Fidentity);
    imm.set_xform_view(Device.mView);
    imm.set_xform_project(Device.mProject);

    // far LOD imposters ( dynamic vertex stream ) on the immediate context; its own list was drawn and emptied earlier
    {
        R_dsgraph_structure& imm_ds = get_imm_context();
        imm_ds.lstLODs.swap(ds.lstLODs);
        scene_begin(imm);
        imm_ds.r_dsgraph_render_lods();
        imm_ds.lstLODs.clear();
    }

    // depth copy for the passes that read $user$zbuffer
    {
        ID3D11Resource* res{};
        Target->get_base_zb()->GetResource(&res);
        HW.get_context(CHW::IMM_CTX_ID)->CopyResource(Target->rt_zbuffer->pSurface, res);
        _RELEASE(res);
    }

    // B2.1: bullet holes, blood and level decals into the scope albedo, after the depth copy as in the main view: level
    // decals of the scope traversal in its context, static and body marks of the wallmark engine on the immediate one
    if (Wallmarks)
    {
        PIX_EVENT(PIP_WALLMARKS);

        if (!ds.mapWmark.empty())
        {
            sc.Invalidate();
            Target->phase_wallmarks(sc);
            rmNormal(sc);
            CWallmarksEngine::SetScopeXforms(sc);
            ds.r_dsgraph_render_wmarks();
            sc.submit();
            sc.Invalidate();
        }

        imm.Invalidate();
        Target->phase_wallmarks(imm);
        rmNormal(imm);
        Wallmarks->RenderScope(imm, m_pip.ssa_k);
        imm.set_ColorWriteEnable();
        imm.set_xform_world(Fidentity);
        imm.set_xform_view(Device.mView);
        imm.set_xform_project(Device.mProject);
    }

    // wet surfaces ( the rain map of this frame )
    rmNormal(imm);
    if (r_rain.nlc_wet_applied)
        Target->draw_rain(imm, r_rain.RainLight);

    // sun from the cascades of this frame ( no second shadow render )
    if (bSUN)
    {
        r_sun.accumulate_scope(imm);
        Target->accum_direct_blend(imm);
    }

    // emissive geometry into the accumulator
    {
        Target->phase_accumulator(sc);
        sc.set_xform_world(Fidentity);
        sc.set_xform_view(Device.mView);
        sc.set_xform_project(Device.mProject);
        sc.set_Stencil(TRUE, D3DCMP_ALWAYS, 0x01, 0xff, 0xff, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP);
        sc.set_CullMode(CULL_CCW);
        sc.set_ColorWriteEnable();
        ds.r_dsgraph_render_emissive(true);

        sc.submit();
        sc.Invalidate();
    }

    pip_leave();

    m_pip.ms_gbuffer += t.GetElapsed_sec() * 1000.f;
}

void CRender::pip_accum_lights(xr_vector<light*>& lights, const u32 kind)
{
    if (!m_pip.frame_on || lights.empty())
        return;

    ZoneScopedN("pip_lights");

    CTimer t;
    t.Start();

    // B2.5: lamp beams of shadowed spots, as in the main loop
    const bool vol = kind == 2 && Target->m_pip_vol && ps_r2_ls_flags.is(R2FLAG_VOLUMETRIC_LIGHTS);

    bool entered = false;
    CBackend& imm = get_imm_context().cmd_list;
    for (light* L : lights)
    {
        if (!L || !L->vis.visible || L->flags.bHudMode)
            continue;

        // B2.7: only lights whose volume reaches into the scope's cone
        const Fsphere& S = L->spatial.sphere;
        if (S.R > 0.f && !m_pip.frustum.testSphere_dirty(S.P, S.R + 0.5f))
        {
            m_pip.lights_culled++;
            continue;
        }
        m_pip.lights_drawn++;

        if (!entered)
        {
            pip_enter();
            entered = true;
        }

        if (kind == 2)
            Target->rt_smap_lights->set_slice_read(L->vis.smap_ID);

        if (kind == 0)
            Target->accum_point(imm, L);
        else
            Target->accum_spot(imm, L);

        if (vol)
            Target->accum_volumetric(imm, L);
    }
    if (entered)
        pip_leave();

    m_pip.ms_lights += t.GetElapsed_sec() * 1000.f;
}

void CRender::pip_combine()
{
    if (!m_pip.frame_on)
        return;

    ZoneScopedN("pip_combine");
    PIX_EVENT(PIP_COMBINE);

    CTimer t;
    t.Start();

    // AO reprojection: scope view space -> last scope frame's clip ( same eye; this frame's matrices without history )
    Fmatrix m_previous;
    m_previous.mul(m_pip.prev_valid ? m_pip.prev_full : m_pip.full, Device.mInvView);

    pip_enter();
    Target->phase_pip_combine(get_imm_context().cmd_list, *m_pip.dsgraph, m_previous);
    pip_leave();
    ps_r_3dss_pip_ao_live = false;

    m_pip.prev_full = m_pip.full;
    m_pip.prev_project = m_pip.project;
    m_pip.done_frame = Device.dwFrame;
    m_pip.prev_valid = true;

    release_context(m_pip.ctx);
    m_pip.ctx = CHW::INVALID_CONTEXT_ID;
    m_pip.dsgraph = nullptr;
    m_pip.ready = true;

    m_pip.ms_combine += t.GetElapsed_sec() * 1000.f;
    m_pip.frames++;

    if (ps_r_3dss_pip_debug >= 2 && Device.dwTimeGlobal > m_pip.log_time + 1000)
    {
        const float n = static_cast<float>(std::max(m_pip.frames, 1u));
        Msg("[3DSS-PIP] %u frames, cpu ms/frame: traversal wait %.2f, g-buffer+sun %.2f, lights %.2f, combine %.2f; image %ux%u mag %.2f fov %.2f; lights/frame %.1f drawn "
            "%.1f culled; ao %d vol %d; sun glare %.2f",
            m_pip.frames, m_pip.ms_trav / n, m_pip.ms_gbuffer / n, m_pip.ms_lights / n, m_pip.ms_combine / n, m_pip.size, m_pip.size, m_pip.mag, m_pip.fov_deg,
            m_pip.lights_drawn / n, m_pip.lights_culled / n, (Target->m_pip.flags & CRenderTarget::PIP_T_AO) ? 1 : 0, Target->m_pip_vol ? 1 : 0, ps_r_3dss_pip_sun.z);
        m_pip.log_time = Device.dwTimeGlobal;
        m_pip.frames = 0;
        m_pip.lights_drawn = m_pip.lights_culled = 0;
        m_pip.ms_trav = m_pip.ms_gbuffer = m_pip.ms_lights = m_pip.ms_combine = 0.f;
    }
}

// rain puddles of the scope view ( render_forward's loop, scope frustum in ViewBase )
void CRender::pip_render_puddles(CBackend& cmd_list)
{
    const auto& Env = g_pGamePersistent->Environment();
    if (!ps_r2_ls_flags_ext.test(R2FLAGEXT_SSLR) || fis_zero(Env.wetness_factor))
        return;

    const Fvector& cam_pos = Device.vCameraPositionSaved;
    for (const auto& puddle : current_level_puddles)
    {
        if (cam_pos.distance_to_sqr(puddle.xform.c) > PIP_PUDDLES_RANGE_SQR || !ViewBase.testSphere_dirty(puddle.xform.c, puddle.radius))
            continue;

        cmd_list.set_Shader(Target->s_puddles);
        cmd_list.set_xform_world(puddle.xform);
        cmd_list.set_c("puddle_height", Env.wetness_factor * puddle.height);
        cmd_list.Render(6);
    }
    cmd_list.set_xform_world(Fidentity);
}

void CRender::pip_end_frame()
{
    if (m_pip.traversal_started)
    {
        m_pip.traversal.wait();
        m_pip.traversal_started = false;
    }
    if (m_pip.ctx != CHW::INVALID_CONTEXT_ID)
    {
        release_context(m_pip.ctx);
        m_pip.ctx = CHW::INVALID_CONTEXT_ID;
        m_pip.dsgraph = nullptr;
    }

    m_pip.frame_on = false;
    m_pip.ready = false;
    ps_r_3dss_pip_params.set(0.f, 0.f, 0.f, 0.f);

    // the scope set lives while the option is on ( kept for the next aim )
    if (!ps_r_3dss_pip && Target && Target->m_pip.size)
        Target->pip_destroy();
}
