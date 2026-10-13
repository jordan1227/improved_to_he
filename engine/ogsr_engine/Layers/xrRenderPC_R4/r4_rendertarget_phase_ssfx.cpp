#include "stdafx.h"
#include "ssfx_features.h"
#include "../../xr_3da/igame_persistent.h"
#include "../../xr_3da/environment.h"
#include "../xrRender/FBasicVisual.h"
#include "../xrRender/dxEnvironmentRender.h"

// NLC: SSS 23 temporal passes ( Screen Space Shaders by Ascii1457, adapted to OGSR ).
// - Sun screen-space shadows      phase_ssfx_sss          before sun accumulation
// - Point/spot light SSS (8 max)  phase_ssfx_sss_lights   before light accumulation
// - Ambient occlusion             phase_ssfx_ao           start of combine
// - Indirect light                phase_ssfx_il           start of combine
// Build B:
// - Water reflection prepass      phase_ssfx_water        before the forward pass ( water itself renders in the normal graph )
// - Fog scattering                phase_ssfx_fog_scattering  after volumetrics, before bloom ( HDR )
// - SSR                           phase_ssfx_ssr          replaces the OGSR SSR pass when on
// - Volumetric lights             reduced-size targets ( r4_rendertarget_accum_spot.cpp / _direct.cpp ), upsampled in
//                                 phase_combine_volumetric
// Each pass reprojects its previous result with the OGSR velocity buffer and rejects history
// with the previous frame position buffer ( rt_ssfx_prev_pos ). HUD pixels are found from the
// depth copy ( the HUD is drawn with a [0, 0.02] depth range ). Targets and shaders exist only
// for enabled passes ( ssfx_features.h ); switches and quality take effect after a game restart,
// resolution after vid_restart. ssfx_compare skips the passes ( consumers read ssfx_compare.x ).

namespace
{
const Fcolor ssfx_clear_sss{1.f, 0.f, 1.f, 0.f}; // r: shadow 1 = lit, b: blurred result
const Fcolor ssfx_clear_lights{1.f, 1.f, 1.f, 1.f};
const Fcolor ssfx_clear_ao{0.f, 0.f, 0.f, 1.f}; // w: occlusion 1 = none
const Fcolor ssfx_clear_zero{0.f, 0.f, 0.f, 0.f};

constexpr u32 ssfx_light_slots = 8;
light* ssfx_light_slot[ssfx_light_slots]{}; // compared only, never dereferenced unless found in this frame's package
u32 ssfx_light_uid[ssfx_light_slots]{}; // .85: light::ssfx_uid of the slot ( pointer + uid identify the light )
} // namespace

float ssfx_sss_slot(const light* L)
{
    const light* key = L->ssfx_parent ? L->ssfx_parent : L;
    return key->ssfx_sss_frame == Device.dwFrame ? static_cast<float>(key->ssfx_sss_id) : -1.f;
}

void CRenderTarget::ssfx_create_targets(const u32 w, const u32 h)
{
    ssfx_history_valid = false;

    if (ssfx_temporal_enabled())
        rt_ssfx_prev_pos.create(r2_RT_ssfx_prev_pos, w, h, DXGI_FORMAT_R16G16B16A16_FLOAT);

    if (ssfx_sss_pass_enabled())
    {
        rt_ssfx_sss.create(r2_RT_ssfx_sss, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
        rt_ssfx_sss_a.create(r2_RT_ssfx_sss_a, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
        rt_ssfx_sss_b.create(r2_RT_ssfx_sss_b, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);

        if (ssfx_sss_lights_enabled())
        {
            rt_ssfx_sss_ext.create(r2_RT_ssfx_sss_ext, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
            rt_ssfx_sss_ext2.create(r2_RT_ssfx_sss_ext2, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
        }

        s_ssfx_sss.create("ssfx_sss");
    }

    if (ssfx_ao_pass_enabled())
    {
        const u32 div = static_cast<u32>(std::clamp(ps_ssfx_ao_res, 1, 4));
        const u32 aw = std::max(1u, w / div), ah = std::max(1u, h / div);

        rt_ssfx_ao.create(r2_RT_ssfx_ao, aw, ah, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_ao_a.create(r2_RT_ssfx_ao_a, aw, ah, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_ao_b.create(r2_RT_ssfx_ao_b, aw, ah, DXGI_FORMAT_R16G16B16A16_FLOAT);

        s_ssfx_ao.create("ssfx_ao");
    }

    if (ssfx_il_pass_enabled())
    {
        const u32 div = static_cast<u32>(std::clamp(ps_ssfx_il_res, 2, 8));
        const u32 iw = std::max(1u, w / div), ih = std::max(1u, h / div);

        rt_ssfx_il.create(r2_RT_ssfx_il, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_il_a.create(r2_RT_ssfx_il_a, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_il_b.create(r2_RT_ssfx_il_b, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);

        s_ssfx_il.create("ssfx_il");
    }

    if (ssfx_water_enabled())
    {
        const u32 div = static_cast<u32>(std::clamp(ps_ssfx_water_res, 1, 4));
        const u32 ww = std::max(1u, w / div), wh = std::max(1u, h / div);

        rt_ssfx_water.create(r2_RT_ssfx_water, ww, wh, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_water_a.create(r2_RT_ssfx_water_a, ww, wh, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_water_b.create(r2_RT_ssfx_water_b, ww, wh, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_water_waves.create(r2_RT_ssfx_water_waves, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);

        s_ssfx_water.create("ssfx_water_pass");
    }

    if (ssfx_features().cloud_shadows)
    {
        t_ssfx_clouds0.create("$user$ssfx_clouds0");
        t_ssfx_clouds1.create("$user$ssfx_clouds1");
    }

    // Fog scattering strength is live ( 0 = pass off ), so its two small targets always exist
    rt_ssfx_fog_4.create(r2_RT_ssfx_fog_4, std::max(1u, w / 4), std::max(1u, h / 4), DXGI_FORMAT_R16G16B16A16_FLOAT);
    rt_ssfx_fog_2.create(r2_RT_ssfx_fog_2, std::max(1u, w / 2), std::max(1u, h / 2), DXGI_FORMAT_R16G16B16A16_FLOAT);
    s_ssfx_fog.create("ssfx_fog_scattering");

    if (ps_ssfx_volumetric_res > 1)
    {
        const u32 div = static_cast<u32>(std::clamp(ps_ssfx_volumetric_res, 2, 4));
        const u32 vw = std::max(1u, w / div), vh = std::max(1u, h / div);

        rt_ssfx_vol_sun.create(r2_RT_ssfx_vol_sun, vw, vh, DXGI_FORMAT_R16G16B16A16_FLOAT);

        s_ssfx_volumetric.create("ssfx_volumetric");
    }

    if (ssfx_features().rain_beams)
    {
        rt_ssfx_rain.create(r2_RT_ssfx_rain, std::max(1u, w / 8), std::max(1u, h / 8), DXGI_FORMAT_R16G16B16A16_FLOAT);
        s_ssfx_rain.create("ssfx_rain");
    }

    if (ssfx_ssr_enabled())
    {
        const u32 div = static_cast<u32>(std::clamp(ps_ssfx_ssr_res, 1, 2));
        const u32 sw = std::max(1u, w / div), sh = std::max(1u, h / div);

        rt_ssfx_ssr.create(r2_RT_ssfx_ssr, sw, sh, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_ssr_b.create(r2_RT_ssfx_ssr_b, sw, sh, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_ssr_hist.create(r2_RT_ssfx_ssr_hist, sw, sh, DXGI_FORMAT_R16G16B16A16_FLOAT);

        s_ssfx_ssr.create("ssfx_ssr");
    }
}

void CRenderTarget::ssfx_begin_frame()
{
    // Cloud shadows: point the named textures at the current weather's clouds ( as phase_combine does for env maps )
    if (t_ssfx_clouds0 && g_pGamePersistent && g_pGamePersistent->Environment().CurrentEnv)
    {
        auto& mix = *smart_cast<dxEnvDescriptorMixerRender*>(&*g_pGamePersistent->Environment().CurrentEnv->m_pDescriptorMixer);
        ref_texture* dst[2] = {&t_ssfx_clouds0, &t_ssfx_clouds1};
        for (u32 i = 0; i < 2; ++i)
        {
            ID3DBaseTexture* surf = i < mix.clouds_r_textures.size() && mix.clouds_r_textures[i].second ? mix.clouds_r_textures[i].second->surface_get() : nullptr;
            (*dst[i])->surface_set(surf); // takes its own reference ( surface_get does not add one, as in phase_combine )
        }
    }

    if (!ssfx_matrices_enabled())
        return;

    // ssfx_compare: passes are skipped; when they come back, their history is stale
    static int last_compare{};
    if (last_compare && !ps_ssfx_compare)
        ssfx_history_valid = false;
    last_compare = ps_ssfx_compare;

    static u32 last_frame{};
    if (last_frame != Device.dwFrame)
    {
        last_frame = Device.dwFrame;

        // View space -> current clip / previous clip ( same as the OGSR SSR pass )
        static Fmatrix saved_viewproj{};
        ssfx_m_previous.mul(saved_viewproj, Device.mInvView);
        ssfx_m_current.set(Device.mProject);
        saved_viewproj.set(Device.mFullTransform);
    }

    if (!ssfx_history_valid)
    {
        // First frame after ( re )creation: neutral history, no reprojection from garbage
        auto& cmd_list = RImplementation.get_imm_context().cmd_list;

        if (rt_ssfx_prev_pos)
            cmd_list.ClearRT(rt_ssfx_prev_pos, ssfx_clear_zero);
        if (rt_ssfx_sss)
        {
            cmd_list.ClearRT(rt_ssfx_sss, ssfx_clear_sss);
            cmd_list.ClearRT(rt_ssfx_sss_a, ssfx_clear_sss);
            cmd_list.ClearRT(rt_ssfx_sss_b, ssfx_clear_sss);
        }
        if (rt_ssfx_sss_ext)
        {
            cmd_list.ClearRT(rt_ssfx_sss_ext, ssfx_clear_lights);
            cmd_list.ClearRT(rt_ssfx_sss_ext2, ssfx_clear_lights);
        }
        if (rt_ssfx_ao)
        {
            cmd_list.ClearRT(rt_ssfx_ao, ssfx_clear_ao);
            cmd_list.ClearRT(rt_ssfx_ao_a, ssfx_clear_ao);
            cmd_list.ClearRT(rt_ssfx_ao_b, ssfx_clear_ao);
        }
        if (rt_ssfx_il)
        {
            cmd_list.ClearRT(rt_ssfx_il, ssfx_clear_zero);
            cmd_list.ClearRT(rt_ssfx_il_a, ssfx_clear_zero);
            cmd_list.ClearRT(rt_ssfx_il_b, ssfx_clear_zero);
        }
        if (rt_ssfx_water)
        {
            cmd_list.ClearRT(rt_ssfx_water, ssfx_clear_zero); // alpha 0: no history
            cmd_list.ClearRT(rt_ssfx_water_a, ssfx_clear_zero);
            cmd_list.ClearRT(rt_ssfx_water_waves, ssfx_clear_zero);
        }
        if (rt_ssfx_ssr_hist)
            cmd_list.ClearRT(rt_ssfx_ssr_hist, ssfx_clear_zero);

        for (auto& slot : ssfx_light_slot)
            slot = nullptr;

        ssfx_history_valid = true;
    }
}

void CRenderTarget::ssfx_end_frame(CBackend& cmd_list)
{
    if (!rt_ssfx_prev_pos)
        return;

    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_prev_pos->pSurface, rt_Position->pSurface);
}

void CRenderTarget::phase_ssfx_sss(CBackend& cmd_list)
{
    if (!rt_ssfx_sss || ps_ssfx_compare)
        return;

    // NLC .84: night ( moon as the sun ): contact shadows of a dim light are invisible. The cascades read rt_ssfx_sss,
    // so it is cleared to "lit" once; the pass resumes with a fresh history ( frame check below ).
    {
        const Fvector& sc = g_pGamePersistent->Environment().CurrentEnv->sun_color;
        if (sc.x * 0.3f + sc.y * 0.59f + sc.z * 0.11f < 0.03f)
        {
            if (ssfx_sss_frame + 1 == Device.dwFrame)
                cmd_list.ClearRT(rt_ssfx_sss, ssfx_clear_sss);
            return;
        }
    }

    PIX_EVENT_CTX(cmd_list, ssfx_sss);

    if (ssfx_sss_frame + 1 != Device.dwFrame)
        cmd_list.ClearRT(rt_ssfx_sss, ssfx_clear_sss);  // NLC: pass skipped ( night, compare ): history is from another scene
    ssfx_sss_frame = Device.dwFrame;

    const Fvector4 setup{ps_ssfx_sss_length, ps_ssfx_sss_length, 0.f, 0.f};

    // Trace + temporal accumulation ( reads history rt_ssfx_sss )
    RenderScreenTriangle(cmd_list, rt_ssfx_sss_a, s_ssfx_sss->E[0], [&]() {
        cmd_list.set_c("m_current", ssfx_m_current);
        cmd_list.set_c("m_previous", ssfx_m_previous);
        cmd_list.set_c("ssfx_sss", setup);
    });

    // Disocclusion blur of the result into .b ( two rotated passes )
    RenderScreenTriangle(cmd_list, rt_ssfx_sss_b, s_ssfx_sss->E[1], [&]() { cmd_list.set_c("blur_setup", 0.f, 1.f, 0.f, 0.f); });
    RenderScreenTriangle(cmd_list, rt_ssfx_sss_a, s_ssfx_sss->E[2], [&]() { cmd_list.set_c("blur_setup", 1.f, 0.f, 2.f, 0.f); });

    // History for the next frame and source for the sun accumulation ( .b )
    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_sss->pSurface, rt_ssfx_sss_a->pSurface);
}

void CRenderTarget::phase_ssfx_sss_lights(CBackend& cmd_list, light_Package& LP)
{
    if (!rt_ssfx_sss_ext || ps_ssfx_compare)
        return;

    PIX_EVENT_CTX(cmd_list, ssfx_sss_lights);

    // Candidates: shadowed spot lights and omni lights ( one entry for the 6 omni parts )
    struct candidate
    {
        light* L;
        float dist;
    };
    xr_vector<candidate> candidates;
    candidates.reserve(LP.v_shadowed.size());

    const Fvector& cam = Device.vCameraPosition;
    for (light* part : LP.v_shadowed)
    {
        light* L = part->ssfx_parent ? part->ssfx_parent : part;
        if (!L->flags.bActive || L->range <= 1.5f)
            continue;

        const float dist = cam.distance_to(L->position);
        if (dist > L->range * 2.f)
            continue;

        if (std::any_of(candidates.begin(), candidates.end(), [L](const candidate& c) { return c.L == L; }))
            continue;

        candidates.push_back({L, dist});
    }

    // Lights the camera is inside first, then by distance
    std::sort(candidates.begin(), candidates.end(), [](const candidate& a, const candidate& b) {
        const bool a_in = a.dist <= a.L->range, b_in = b.dist <= b.L->range;
        if (a_in != b_in)
            return a_in;
        return a.dist < b.dist;
    });
    if (candidates.size() > ssfx_light_slots * 2)
        candidates.resize(ssfx_light_slots * 2);

    // Keep lights in their previous slot ( stable history ), then fill free slots
    light* next_slot[ssfx_light_slots]{};
    for (u32 s = 0; s < ssfx_light_slots; ++s)
    {
        if (ssfx_light_slot[s] &&
            std::any_of(candidates.begin(), candidates.end(), [&](const candidate& c) { return c.L == ssfx_light_slot[s] && c.L->ssfx_uid == ssfx_light_uid[s]; }))
            next_slot[s] = ssfx_light_slot[s];
    }
    for (const candidate& c : candidates)
    {
        if (std::find(std::begin(next_slot), std::end(next_slot), c.L) != std::end(next_slot))
            continue;
        const auto free_slot = std::find(std::begin(next_slot), std::end(next_slot), nullptr);
        if (free_slot == std::end(next_slot))
            break;
        *free_slot = c.L;
    }

    // Light data for the shader ( view space position, w: fade 0 = full ~ 1 = off, +2 = new light in this slot )
    Fvector4 data[ssfx_light_slots]{};
    for (u32 s = 0; s < ssfx_light_slots; ++s)
    {
        const bool fresh = ssfx_light_slot[s] != next_slot[s] || (next_slot[s] && next_slot[s]->ssfx_uid != ssfx_light_uid[s]);
        ssfx_light_slot[s] = next_slot[s];
        ssfx_light_uid[s] = next_slot[s] ? next_slot[s]->ssfx_uid : 0;
        light* L = next_slot[s];
        if (!L)
        {
            data[s].set(0.f, 0.f, 0.f, 1.f);
            continue;
        }

        L->ssfx_sss_id = static_cast<int>(s);
        L->ssfx_sss_frame = Device.dwFrame;

        Fvector pos;
        Device.mView.transform_tiny(pos, L->position);

        const float dist = cam.distance_to(L->position);
        const float fade = clampr((dist - L->range * 1.5f) / (L->range * 0.5f), 0.f, 1.f);
        data[s].set(pos.x, pos.y, pos.z, fresh ? fade + 2.f : fade);
    }

    const Fvector4 setup{ps_ssfx_sss_length, ps_ssfx_sss_length, 0.f, 0.f};

    // Slots 0-3 -> rt_ssfx_sss_ext, slots 4-7 -> rt_ssfx_sss_ext2. Each pass reads its own history.
    for (u32 pass = 0; pass < 2; ++pass)
    {
        ref_rt& history = pass ? rt_ssfx_sss_ext2 : rt_ssfx_sss_ext;

        // NLC .84: no light in this pass's 4 slots ( most outdoor frames ): clear its history once and skip the pass
        if (!next_slot[pass * 4] && !next_slot[pass * 4 + 1] && !next_slot[pass * 4 + 2] && !next_slot[pass * 4 + 3])
        {
            if (!ssfx_light_pass_empty[pass])
                cmd_list.ClearRT(history, ssfx_clear_lights);
            ssfx_light_pass_empty[pass] = true;
            continue;
        }
        ssfx_light_pass_empty[pass] = false;

        RenderScreenTriangle(cmd_list, rt_ssfx_sss_b, s_ssfx_sss->E[3 + pass], [&]() {
            cmd_list.set_c("m_current", ssfx_m_current);
            cmd_list.set_c("m_previous", ssfx_m_previous);
            cmd_list.set_c("ssfx_sss", setup);
            for (u32 i = 0; i < 4; ++i)
                cmd_list.set_ca("ssfx_lights_data", i, data[pass * 4 + i]);
        });

        HW.get_context(cmd_list.context_id)->CopyResource(history->pSurface, rt_ssfx_sss_b->pSurface);
    }
}

void CRenderTarget::phase_ssfx_ao(CBackend& cmd_list)
{
    if (!rt_ssfx_ao || ps_ssfx_compare)
        return;

    PIX_EVENT_CTX(cmd_list, ssfx_ao);

    const float res = static_cast<float>(std::clamp(ps_ssfx_ao_res, 1, 4));
    const float aw = static_cast<float>(rt_ssfx_ao->dwWidth), ah = static_cast<float>(rt_ssfx_ao->dwHeight);

    const Fvector4 params{res, 5.f * ps_ssfx_ao_intensity, ps_ssfx_ao_radius, 150.f}; // res, intensity, radius, distance
    const Fvector4 params2{1.f, 1.f, 0.f, 0.f}; // HUD int, flora int, max occlusion, -

    RenderScreenTriangle(cmd_list, rt_ssfx_ao_a, s_ssfx_ao->E[0], [&]() {
        cmd_list.set_c("m_current", ssfx_m_current);
        cmd_list.set_c("m_previous", ssfx_m_previous);
        cmd_list.set_c("ssfx_ao_params", params);
        cmd_list.set_c("ssfx_ao_params2", params2);
    });

    // Unblurred, accumulated AO is the history
    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_ao->pSurface, rt_ssfx_ao_a->pSurface);

    // Depth-aware blur, two rotated passes ( result in rt_ssfx_ao_a.w )
    RenderScreenTriangle(cmd_list, rt_ssfx_ao_b, s_ssfx_ao->E[1], [&]() { cmd_list.set_c("blur_setup", res, 0.5f, aw, ah); });
    RenderScreenTriangle(cmd_list, rt_ssfx_ao_a, s_ssfx_ao->E[2], [&]() { cmd_list.set_c("blur_setup", res, 1.0f, aw, ah); });

    // Reduced-size pass: restore the full render-size viewport for the combine draws
    cmd_list.SetViewport({0.f, 0.f, static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight()), 0.f, 1.f});
}

void CRenderTarget::phase_ssfx_il(CBackend& cmd_list)
{
    if (!rt_ssfx_il || ps_ssfx_compare)
        return;

    PIX_EVENT_CTX(cmd_list, ssfx_il);

    const float res = static_cast<float>(std::clamp(ps_ssfx_il_res, 2, 8));
    const float iw = static_cast<float>(rt_ssfx_il->dwWidth), ih = static_cast<float>(rt_ssfx_il->dwHeight);

    const Fvector4 params{res, ps_ssfx_il_intensity, 1.f, 150.f}; // res, intensity, vibrance, distance
    const Fvector4 params2{1.f, 0.5f, 0.f, 0.f}; // HUD int, flora int, -, -

    RenderScreenTriangle(cmd_list, rt_ssfx_il_a, s_ssfx_il->E[0], [&]() {
        cmd_list.set_c("m_current", ssfx_m_current);
        cmd_list.set_c("m_previous", ssfx_m_previous);
        cmd_list.set_c("ssfx_il_params", params);
        cmd_list.set_c("ssfx_il_params2", params2);
    });

    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_il->pSurface, rt_ssfx_il_a->pSurface);

    RenderScreenTriangle(cmd_list, rt_ssfx_il_b, s_ssfx_il->E[1], [&]() { cmd_list.set_c("blur_setup", res, 0.5f, iw, ih); });
    RenderScreenTriangle(cmd_list, rt_ssfx_il_a, s_ssfx_il->E[2], [&]() { cmd_list.set_c("blur_setup", res, 1.0f, iw, ih); });

    // Reduced-size pass: restore the full render-size viewport for the combine draws
    cmd_list.SetViewport({0.f, 0.f, static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight()), 0.f, 1.f});
}

void CRenderTarget::phase_ssfx_water(CBackend& cmd_list)
{
    auto& water = RImplementation.get_imm_context().waterNodes;

    if (!rt_ssfx_water || ps_ssfx_compare || water.empty())
    {
        water.clear();
        return;
    }

    PIX_EVENT_CTX(cmd_list, ssfx_water);

    if (ssfx_water_frame + 1 != Device.dwFrame)
        cmd_list.ClearRT(rt_ssfx_water, ssfx_clear_zero); // NLC: no water visible for a while: drop old reflections
    ssfx_water_frame = Device.dwFrame;

    const float ww = static_cast<float>(rt_ssfx_water->dwWidth), wh = static_cast<float>(rt_ssfx_water->dwHeight);

    // Reflection + temporal accumulation: water geometry into the reduced target ( no depth buffer, the shader skips
    // water behind scene depth ). Alpha 1 marks water pixels for the blur and the final water shader.
    u_setrt(cmd_list, rt_ssfx_water_a, nullptr, nullptr, nullptr, nullptr);
    cmd_list.SetViewport({0.f, 0.f, ww, wh, 0.f, 1.f});
    cmd_list.ClearRT(rt_ssfx_water_a, ssfx_clear_zero);
    cmd_list.set_Stencil(FALSE);

    // Matrices first: binders such as m_texgen are evaluated when the element is set
    cmd_list.set_xform_world(Fidentity);
    cmd_list.set_xform_view(Device.mView);
    cmd_list.set_xform_project(Device.mProject);
    cmd_list.set_Element(s_ssfx_water->E[0]);
    cmd_list.set_c("m_current", ssfx_m_current);
    cmd_list.set_c("m_previous", ssfx_m_previous);

    for (dxRender_Visual* V : water)
        V->Render(cmd_list, 1.f, false);
    water.clear();

    // Unblurred, accumulated reflection is the history ( and the sharp layer of the final shader )
    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_water->pSurface, rt_ssfx_water_a->pSurface);

    // Separable blur ( result in rt_ssfx_water_a )
    const float blur = ps_ssfx_water_blur;
    const float step = 1.f + blur;
    RenderScreenTriangle(cmd_list, rt_ssfx_water_b, s_ssfx_water->E[1], [&]() { cmd_list.set_c("blur_setup", step / ww, 0.f, blur, 0.f); });
    RenderScreenTriangle(cmd_list, rt_ssfx_water_a, s_ssfx_water->E[2], [&]() { cmd_list.set_c("blur_setup", 0.f, step / wh, blur, 0.f); });

    // Animated wave height for the parallax waves
    if (ssfx_features().water_parallax > 0)
    {
        const float wind = g_pGamePersistent->Environment().CurrentEnv->wind_velocity;
        RenderScreenTriangle(cmd_list, rt_ssfx_water_waves, s_ssfx_water->E[3], [&]() { cmd_list.set_c("ssfx_waves_setup", Device.fTimeGlobal, wind, 0.f, 0.f); });
    }

    // Reduced-size pass: restore the full render-size viewport for the forward pass
    cmd_list.SetViewport({0.f, 0.f, static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight()), 0.f, 1.f});
}

void CRenderTarget::phase_ssfx_fog_scattering(CBackend& cmd_list)
{
    if (!rt_ssfx_fog_4 || ps_ssfx_fog_scattering <= 0.f || ps_ssfx_compare)
        return;

    // NLC .84: no visible fog within the view distance ( clear weather ): the shader would change nothing. Its fog
    // factor is ( linear fog )^2, at most doubled by the height fog, and the effect starts at 0.1: below 0.15 linear
    // at the far plane nothing on screen reaches it.
    {
        const auto& E = *g_pGamePersistent->Environment().CurrentEnv;
        const float fog_lin = (E.far_plane - E.fog_near) / std::max(E.fog_far - E.fog_near, 0.001f);
        if (fog_lin < 0.15f && ps_ssfx_ground_mist <= 0.f) // .90: ground mist can reach the threshold on its own
            return;
    }

    PIX_EVENT_CTX(cmd_list, ssfx_fog_scattering);

    // Blurred scene: 1/4, then 1/2 ( HDR, render resolution )
    const float w4 = static_cast<float>(rt_ssfx_fog_4->dwWidth), h4 = static_cast<float>(rt_ssfx_fog_4->dwHeight);
    const float w2 = static_cast<float>(rt_ssfx_fog_2->dwWidth), h2 = static_cast<float>(rt_ssfx_fog_2->dwHeight);
    RenderScreenTriangle(cmd_list, rt_ssfx_fog_4, s_ssfx_fog->E[1], [&]() { cmd_list.set_c("blur_setup", w4, h4, 0.f, 0.f); });
    RenderScreenTriangle(cmd_list, rt_ssfx_fog_2, s_ssfx_fog->E[2], [&]() { cmd_list.set_c("blur_setup", w2, h2, 0.f, 0.f); });

    // Scattering: blurred scene mixed in where the fog is dense
    RenderScreenTriangle(cmd_list, rt_Generic_scene_scratch, s_ssfx_fog->E[0], [&]() { cmd_list.set_c("ssfx_scattering_setup", ps_ssfx_fog_scattering, 0.f, 0.f, 0.f); });
    HW.get_context(cmd_list.context_id)->CopyResource(rt_Generic_0->pSurface, rt_Generic_scene_scratch->pSurface);
}

bool CRenderTarget::phase_ssfx_ssr(CBackend& cmd_list)
{
    ssfx_ssr_composited = false;
    if (!rt_ssfx_ssr || ps_ssfx_compare || !ps_r2_ls_flags_ext.test(R2FLAGEXT_SSLR))
        return false;

    // Wet surfaces always; dry glossy surfaces only with ssfx_ssr_dry
    const bool wet = !fis_zero(g_pGamePersistent->Environment().wetness_factor);
    const float strength = wet ? 1.f : ps_ssfx_ssr_dry;
    if (strength <= 0.f)
        return true;

    PIX_EVENT_CTX(cmd_list, ssfx_ssr);

    if (ssfx_ssr_frame + 1 != Device.dwFrame)
        cmd_list.ClearRT(rt_ssfx_ssr_hist, ssfx_clear_zero); // NLC: dry spell / compare: old reflections are elsewhere
    ssfx_ssr_frame = Device.dwFrame;

    const float w = static_cast<float>(rt_ssfx_ssr->dwWidth), h = static_cast<float>(rt_ssfx_ssr->dwHeight);
    const Fvector4 setup{strength, ps_ssfx_ssr_blur, 0.f, 0.f};

    // Reflection + temporal accumulation ( reads rt_ssfx_ssr_hist )
    RenderScreenTriangle(cmd_list, rt_ssfx_ssr, s_ssfx_ssr->E[0], [&]() {
        cmd_list.set_c("ssr_setup", ps_ssfx_ssr_1);
        cmd_list.set_c("ssfx_ssr_2", ps_ssfx_ssr_2);
        cmd_list.set_c("ssfx_ssr_setup", setup);
        cmd_list.set_c("m_current", ssfx_m_current);
        cmd_list.set_c("m_previous", ssfx_m_previous);
    });

    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_ssr_hist->pSurface, rt_ssfx_ssr->pSurface);

    // Gloss-based blur ( rough surfaces get blurrier reflections ), result in rt_ssfx_ssr
    RenderScreenTriangle(cmd_list, rt_ssfx_ssr_b, s_ssfx_ssr->E[1], [&]() { cmd_list.set_c("blur_setup", 1.f / w, 0.f, ps_ssfx_ssr_blur, 0.f); });
    RenderScreenTriangle(cmd_list, rt_ssfx_ssr, s_ssfx_ssr->E[2], [&]() { cmd_list.set_c("blur_setup", 0.f, 1.f / h, ps_ssfx_ssr_blur, 0.f); });

    // Combine: blended into the scene ( .84, was scratch + full-resolution copy )
    RenderScreenTriangle(cmd_list, rt_Generic_0, s_ssfx_ssr->E[3]);
    ssfx_ssr_composited = true;

    return true;
}

void CRenderTarget::ssfx_volumetric_restore_viewport(CBackend& cmd_list) const
{
    cmd_list.SetViewport({0.f, 0.f, static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight()), 0.f, 1.f});
}

// NLC .85: SSS 22 rain refraction buffer. The rain drops refract a 1/8 copy of the current HDR scene with the volumetric
// light added ( lamp and torch beams light the streaks ), instead of $user$blur_4 ( the previous frame after tone
// mapping ). Only while it rains.
void CRenderTarget::phase_ssfx_rain(CBackend& cmd_list)
{
    if (!rt_ssfx_rain || ps_ssfx_compare || g_pGamePersistent->Environment().CurrentEnv->rain_density <= 0.f)
        return;

    PIX_EVENT_CTX(cmd_list, ssfx_rain);

    // E[0]: reduced volumetric target ( sun + spot ), E[1]: full-size generic2 / generic3
    const bool reduced = ssfx_volumetric_reduced();
    RenderScreenTriangle(cmd_list, rt_ssfx_rain, s_ssfx_rain->E[reduced ? 0 : 1], [&]() {
        cmd_list.set_c("ssfx_rain_vol", m_bHasActiveVolumetric ? 1.f : 0.f, reduced ? 0.f : 1.f, 0.f, 0.f);
    });

    ssfx_volumetric_restore_viewport(cmd_list);
}
