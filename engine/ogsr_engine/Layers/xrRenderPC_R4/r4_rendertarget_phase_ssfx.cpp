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
Fvector ssfx_light_last_pos[ssfx_light_slots]{}; // NLC .150: world position of the slot's light last frame ( moving torches )
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
        rt_ssfx_pos_world.create(r2_RT_ssfx_pos_world, w, h, DXGI_FORMAT_R16G16B16A16_FLOAT); // NLC .210: as rt_Position
        ssfx_pos_world_frame = 0;

        s_ssfx_ao.create("ssfx_ao");
    }

    if (ssfx_il_pass_enabled())
    {
        const u32 div = static_cast<u32>(std::clamp(ps_ssfx_il_res, 2, 8));
        const u32 iw = std::max(1u, w / div), ih = std::max(1u, h / div);

        rt_ssfx_il.create(r2_RT_ssfx_il, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_il_a.create(r2_RT_ssfx_il_a, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_il_b.create(r2_RT_ssfx_il_b, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);

        // NLC: local-light bounce ( additive ) and the accumulator snapshot it is derived from ( same size and
        // format as rt_Accumulator, for CopyResource )
        rt_ssfx_il_local.create(r2_RT_ssfx_il_local, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_il_local_a.create(r2_RT_ssfx_il_local_a, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_il_local_b.create(r2_RT_ssfx_il_local_b, iw, ih, DXGI_FORMAT_R16G16B16A16_FLOAT);
        rt_ssfx_accum_pre.create(r2_RT_ssfx_accum_pre, w, h, DXGI_FORMAT_R16G16B16A16_FLOAT);

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
    s_ssfx_haze.create("ssfx_haze"); // NLC anomaly haze, live switch

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

    // NLC .209: reflections of the forward pass ( haze, particles, beams ): same size and format as rt_Generic_0
    ssfx_scene_prev_frame = 0;
    if (ssfx_ssr_enabled() || ssfx_water_enabled())
        rt_ssfx_scene_prev.create(r2_RT_ssfx_scene_prev, w, h, DXGI_FORMAT_R16G16B16A16_FLOAT);
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
        ssfx_il_slot_count = 0; // NLC .208: no known-light slots from the previous level / before the reset
        for (auto& M : ssfx_il_vis) // NLC .210: nor their visibility maps ( light addresses are reused )
        {
            M.key = nullptr;
            M.valid = M.building = false;
        }
        ssfx_scene_prev_frame = 0; // NLC .209: last frame's scene is from another view

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
            cmd_list.ClearRT(rt_ssfx_il_local, ssfx_clear_zero);
            cmd_list.ClearRT(rt_ssfx_il_local_a, ssfx_clear_zero);
            cmd_list.ClearRT(rt_ssfx_il_local_b, ssfx_clear_zero);
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

// NLC .210: the position buffer with all world geometry and nothing else, copied right before the HUD is drawn. The AO
// samples it for world pixels: the weapon no longer hides the geometry behind it ( AO on cover popped as the barrel
// moved ), and the normals are those before draw_rain bends them ( running water lightened the AO under cars ).
void CRenderTarget::ssfx_copy_pos_world(CBackend& cmd_list)
{
    if (!rt_ssfx_pos_world || ps_ssfx_compare)
        return;

    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_pos_world->pSurface, rt_Position->pSurface);
    ssfx_pos_world_frame = Device.dwFrame;
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

    // Light data for the shader ( view space position, w: fade 0 = full ~ 1 = off, +2 = new light in this slot,
    // +4 = NLC .150: the slot's light moves ( > 0.3 m/s ): its shadows keep little history )
    Fvector4 data[ssfx_light_slots]{};
    for (u32 s = 0; s < ssfx_light_slots; ++s)
    {
        const bool fresh = ssfx_light_slot[s] != next_slot[s] || (next_slot[s] && next_slot[s]->ssfx_uid != ssfx_light_uid[s]);
        // moved: faster than ~0.3 m/s ( a carried torch ), frame-rate independent; at least 2 mm to ignore jitter
        const float move_eps = std::max(0.3f * Device.fTimeDelta, 0.002f);
        const bool moved = !fresh && next_slot[s] && next_slot[s]->position.distance_to_sqr(ssfx_light_last_pos[s]) > move_eps * move_eps;
        if (next_slot[s])
            ssfx_light_last_pos[s] = next_slot[s]->position;
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
        data[s].set(pos.x, pos.y, pos.z, fresh ? fade + 2.f : (moved ? fade + 4.f : fade));
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
    const Fvector4 params2{1.f, ps_ssfx_ao_flora, 0.f, ps_ssfx_ao_radius_fix ? 1.f : 0.f}; // HUD int, flora int, max occlusion, radius fix

    RenderScreenTriangle(cmd_list, rt_ssfx_ao_a, s_ssfx_ao->E[0], [&]() {
        cmd_list.set_c("m_current", ssfx_m_current);
        cmd_list.set_c("m_previous", ssfx_m_previous);
        cmd_list.set_c("ssfx_ao_params", params);
        cmd_list.set_c("ssfx_ao_params2", params2);
        // NLC .210: x = the world copy of this frame exists ( samples see through the weapon, normals before the rain pass )
        cmd_list.set_c("ssfx_ao_params3", rt_ssfx_pos_world && ssfx_pos_world_frame == Device.dwFrame ? 1.f : 0.f, 0.f, 0.f, 0.f);
    });

    // Unblurred, accumulated AO is the history
    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_ao->pSurface, rt_ssfx_ao_a->pSurface);

    // Depth-aware blur, two rotated passes ( result in rt_ssfx_ao_a.w )
    RenderScreenTriangle(cmd_list, rt_ssfx_ao_b, s_ssfx_ao->E[1], [&]() { cmd_list.set_c("blur_setup", res, 0.5f, aw, ah); });
    RenderScreenTriangle(cmd_list, rt_ssfx_ao_a, s_ssfx_ao->E[2], [&]() { cmd_list.set_c("blur_setup", res, 1.0f, aw, ah); });

    // Reduced-size pass: restore the full render-size viewport for the combine draws
    cmd_list.SetViewport({0.f, 0.f, static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight()), 0.f, 1.f});
}

// NLC .208: fill centre ( world ) and reach of a known light ( docs/IL_DESIGN.md 4.3 / 4.4 ). Point light: centre at the
// light, reach 1.275 range + 0.85 m ( the .207h shader-tuned value, 4.7 m for a fruit punch ). Spot: centre along the
// axis at half the range but at most 1.5 m ( a 12 m ceiling spot no longer puts it under the floor ), reach
// 0.64 range + 0.85 m. Both x ssfx_il_lights_reach, at most 12 m.
static float ssfx_il_light_fill(const Fvector& pos, const Fvector& dir, float range, bool spot, Fvector& centre)
{
    centre = pos;
    float reach = 1.275f * range + 0.85f;
    if (spot)
    {
        centre.mad(dir, std::min(0.5f * range, 1.5f));
        reach = 0.64f * range + 0.85f;
    }
    return reach; // .210: the base value; ssfx_il_reach applies the cvar and the cap
}

static float ssfx_il_reach(float base) { return std::min(base * ps_ssfx_il_lights_reach, 12.f); }

// NLC .210 ( docs/IL_DESIGN.md 4.6 ): a spot bounces where its beam lands, not around the lamp. One static ray along
// the axis: the centre sits just in front of the hit, the reach follows the beam radius there ( the same 1.275 r + 0.85
// as a point light of that radius ), att is the light's falloff at the hit. A beam that hits nothing within its range
// has no bounce ( att 0 ). Point lights are as before. A 30 m spot used to get a 12 m glow around the lamp head.
static float ssfx_il_light_fill_traced(const Fvector& pos, const Fvector& dir, float range, float cone, bool spot, Fvector& centre, float& att)
{
    att = 1.f;
    const float base = ssfx_il_light_fill(pos, dir, range, spot, centre);
    if (!spot || !g_pGameLevel || range < 0.5f)
        return base;

    collide::rq_result RQ;
    g_pGameLevel->ObjectSpace.RayPick(pos, dir, range, collide::rqtStatic, RQ, nullptr);
    const float h = RQ.range;
    if (h >= range - 0.01f)
    {
        att = 0.f;
        return base;
    }
    centre.mad(pos, dir, std::max(h - 0.4f, h * 0.5f));
    const float r = std::min(h * tanf(std::min(cone * 0.5f, deg2rad(60.f))), range);
    const float k = h / range;
    att = 1.f - k * k;
    return 1.275f * r + 0.85f;
}

// NLC .210: unit direction of a point of the octahedral map, u / v 0 .. 1. Matches ssfx_il_oct_uv in ssfx_il.ps
static Fvector ssfx_il_oct_dir(float u, float v)
{
    const float fx = u * 2.f - 1.f, fy = v * 2.f - 1.f;
    Fvector n{fx, fy, 1.f - _abs(fx) - _abs(fy)};
    const float t = std::max(-n.z, 0.f);
    n.x += n.x >= 0.f ? -t : t;
    n.y += n.y >= 0.f ? -t : t;
    n.normalize();
    return n;
}

// NLC .210: the map of this light, or a free one ( not referred to by any slot this frame ) reset for it; -1 = none
// free this frame ( the light waits with fade 0 and asks again next frame )
int CRenderTarget::ssfx_il_vis_acquire(const void* key)
{
    int free_id = -1;
    for (u32 i = 0; i < SSFX_IL_LIGHTS_MAX; ++i)
    {
        auto& M = ssfx_il_vis[i];
        if (M.key == key)
        {
            M.used_frame = Device.dwFrame;
            return int(i);
        }
        if (M.used_frame != Device.dwFrame && (free_id < 0 || (!M.valid && ssfx_il_vis[free_id].valid)))
            free_id = int(i);
    }
    if (free_id >= 0)
    {
        auto& M = ssfx_il_vis[free_id];
        M.key = key;
        M.used_frame = Device.dwFrame;
        M.valid = M.building = false;
    }
    return free_id;
}

// NLC .210: trace the visibility maps of the first live_slots slots ( the lights picked this frame; fading slots keep
// the map they have ). A map is rebuilt when its light's fill centre moved 0.5 m ( NPC torches ); the finished map
// replaces the old one in one step. Budget: 128 texels a frame while a light waits for its first map, else 48;
// 4 static rays per texel, the farthest hit is kept ( a texel is ~13 degrees wide: light leaks rather than
// false shadow on surfaces seen at a slant ).
void CRenderTarget::ssfx_il_vis_update(u32 live_slots)
{
    if (!g_pGameLevel)
        return;

    constexpr u32 N = SSFX_IL_VIS_N, total = N * N;
    constexpr float start_off = 0.2f, max_range = 13.f; // the reach is at most 12 m

    bool waiting = false;
    for (u32 i = 0; i < live_slots; ++i)
    {
        const auto& S = ssfx_il_slots[i];
        if (S.vis < 0)
            continue;
        auto& M = ssfx_il_vis[S.vis];
        if (M.valid && M.origin.distance_to_sqr(S.centre) > 3.f * 3.f)
            M.valid = false; // a jump: another light at a recycled address, or a teleported one
        if (M.building && M.b_origin.distance_to_sqr(S.centre) > 3.f * 3.f)
            M.building = false;
        if (!M.building && (!M.valid || M.origin.distance_to_sqr(S.centre) > 0.5f * 0.5f))
        {
            M.building = true;
            M.b_origin = S.centre;
            M.b_next = 0;
        }
        waiting = waiting || !M.valid;
    }

    u32 budget = waiting ? 128 : 48;
    for (u32 pass = 0; pass < 2 && budget; ++pass) // first the lights without a map, then the refreshes
        for (u32 i = 0; i < live_slots && budget; ++i)
        {
            const auto& S = ssfx_il_slots[i];
            if (S.vis < 0)
                continue;
            auto& M = ssfx_il_vis[S.vis];
            if (!M.building || (pass == 0 && M.valid))
                continue;
            while (M.b_next < total && budget)
            {
                const u32 tx = M.b_next % N, ty = M.b_next / N;
                float best = 0.f;
                for (u32 s = 0; s < 4; ++s)
                {
                    const Fvector d = ssfx_il_oct_dir((float(tx) + ((s & 1) ? 0.75f : 0.25f)) / float(N), (float(ty) + ((s & 2) ? 0.75f : 0.25f)) / float(N));
                    Fvector start;
                    start.mad(M.b_origin, d, start_off);
                    collide::rq_result RQ;
                    g_pGameLevel->ObjectSpace.RayPick(start, d, max_range, collide::rqtStatic, RQ, nullptr);
                    best = std::max(best, start_off + RQ.range);
                }
                M.b_dist[M.b_next++] = best;
                --budget;
            }
            if (M.b_next >= total)
            {
                std::copy(std::begin(M.b_dist), std::end(M.b_dist), std::begin(M.dist));
                M.origin = M.b_origin;
                M.valid = true;
                M.building = false;
            }
        }
}

void CRenderTarget::phase_ssfx_il(CBackend& cmd_list)
{
    if (!rt_ssfx_il || ps_ssfx_compare)
        return;

    PIX_EVENT_CTX(cmd_list, ssfx_il);

    const float res = static_cast<float>(std::clamp(ps_ssfx_il_res, 2, 8));
    const float iw = static_cast<float>(rt_ssfx_il->dwWidth), ih = static_cast<float>(rt_ssfx_il->dwHeight);

    // NLC: intensity and vibrance are applied in combine_1 ( ssfx_il_combine ); only the distance is read here.
    // .207: y = known-light slot count, z = their strength ( ssfx_il_lights ), params2.z = ssfx_il_lights_debug
    // .208: params2.x = floor factor ( ssfx_il_lights_floor ), .w = occluder thickness k ( ssfx_il_lights_thick )
    const Fvector4 params2{ps_ssfx_il_lights_floor, 0.5f, static_cast<float>(ps_ssfx_il_lights_debug), ps_ssfx_il_lights_thick}; // floor, flora int, light debug, thickness

    // NLC .207: known lights in view space. [0] fill centre / reach, [1] raw colour x LOD / fade ( ssfx_il_light_fill ).
    // .208: slots whose fill sphere misses the view frustum are not uploaded ( no visible pixel can receive them )
    // .210: only slots with a finished visibility map are uploaded, at the centre the map was traced from
    Fvector4 il_lights[SSFX_IL_LIGHTS_MAX * 2]{};
    const ssfx_il_vis_map* il_maps[SSFX_IL_LIGHTS_MAX]{};
    u32 light_count = 0;
    for (u32 i = 0; i < ssfx_il_slot_count && ps_ssfx_il_lights > 0.f; ++i)
    {
        const auto& S = ssfx_il_slots[i];
        if (S.fade <= 0.f || S.vis < 0 || !ssfx_il_vis[S.vis].valid)
            continue;
        const ssfx_il_vis_map& M = ssfx_il_vis[S.vis];
        const float reach = ssfx_il_reach(S.reach);
        if (!RImplementation.ViewBase.testSphere_dirty(M.origin, reach))
            continue;
        Fvector cv;
        Device.mView.transform_tiny(cv, M.origin);
        il_lights[light_count * 2].set(cv.x, cv.y, cv.z, reach);
        il_lights[light_count * 2 + 1].set(S.col.x, S.col.y, S.col.z, S.fade);
        il_maps[light_count] = &M;
        ++light_count;
    }
    // .210: a negative res divider tells ssfx_il.ps that the visibility maps are uploaded ( the shader does not read the
    // divider; with an older exe it stays positive and the shader keeps its screen-space checks and short reach )
    const Fvector4 params{-res, static_cast<float>(light_count), ps_ssfx_il_lights, ps_ssfx_il_distance}; // -res, light count, light strength, distance

    // NLC: SV_Target0 = ambient IL ( rgb ) + torch bounce ( a ), SV_Target1 = local-light / emissive bounce ( rgb )
    RenderScreenTriangle2(cmd_list, rt_ssfx_il_a, rt_ssfx_il_local_a, s_ssfx_il->E[0], [&]() {
        cmd_list.set_c("m_current", ssfx_m_current);
        cmd_list.set_c("m_previous", ssfx_m_previous);
        cmd_list.set_c("ssfx_il_params", params);
        cmd_list.set_c("ssfx_il_params2", params2);
        for (u32 i = 0; i < light_count * 2; ++i)
            cmd_list.set_ca("ssfx_il_lights", i, il_lights[i]);
        // .210: visibility maps, 4 distances per float4, SSFX_IL_VIS_N^2 / 4 float4 per uploaded slot
        if (light_count)
        {
            const ref_constant vis_c = cmd_list.get_c("ssfx_il_vis");
            R_constant* vis_p = vis_c._get(); // null when the shader does not declare it
            constexpr u32 per_slot = SSFX_IL_VIS_N * SSFX_IL_VIS_N / 4;
            for (u32 i = 0; i < light_count && vis_p; ++i)
            {
                const float* d = il_maps[i]->dist;
                for (u32 k = 0; k < per_slot; ++k)
                    cmd_list.set_ca(vis_p, i * per_slot + k, Fvector4{d[k * 4], d[k * 4 + 1], d[k * 4 + 2], d[k * 4 + 3]});
            }
        }
    });

    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_il->pSurface, rt_ssfx_il_a->pSurface);
    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_il_local->pSurface, rt_ssfx_il_local_a->pSurface);

    RenderScreenTriangle2(cmd_list, rt_ssfx_il_b, rt_ssfx_il_local_b, s_ssfx_il->E[1], [&]() { cmd_list.set_c("blur_setup", res, 0.5f, iw, ih); });
    RenderScreenTriangle2(cmd_list, rt_ssfx_il_a, rt_ssfx_il_local_a, s_ssfx_il->E[2], [&]() { cmd_list.set_c("blur_setup", res, 1.0f, iw, ih); });

    // Reduced-size pass: restore the full render-size viewport for the combine draws
    cmd_list.SetViewport({0.f, 0.f, static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight()), 0.f, 1.f});
}

// NLC: the accumulator holds sun + emissive here ( r4_R_render.cpp, right before render_lights ). ssfx_il.ps takes
// ( accum - snapshot ) as the light of point / spot lights ( anomalies, lamps, campfires ) for the additive bounce.
void CRenderTarget::phase_ssfx_il_snapshot(CBackend& cmd_list, const light_Package& LP)
{
    if (!rt_ssfx_accum_pre || ps_ssfx_compare)
    {
        if (ps_ssfx_il_lights_info) // .208: answer the request instead of keeping it latched
        {
            ps_ssfx_il_lights_info = 0;
            Msg("~ [il lights] IL pass %s: no known lights", rt_ssfx_accum_pre ? "skipped ( ssfx_compare )" : "off ( ssfx_indirect_light / ssfx_effects_off )");
        }
        return;
    }

    PIX_EVENT_CTX(cmd_list, ssfx_il_snapshot);
    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_accum_pre->pSurface, rt_Accumulator->pSurface);

    ssfx_il_collect_lights(LP);
}

// NLC .207: pick the known lights for the analytic IL fill ( docs/IL_DESIGN.md 4.3 ). Runs on the render thread right
// before render_lights, which empties the package. Shadowed point lights come as 6 OMNIPART pieces: counted once by
// their omni light ( ssfx_parent ).
void CRenderTarget::ssfx_il_collect_lights(const light_Package& LP)
{
    struct cand
    {
        const light* L;
        float score;
    };
    xr_vector<cand> cands;
    cands.reserve(LP.v_point.size() + LP.v_spot.size() + LP.v_shadowed.size() / 4 + 16);

    const auto& T = g_pGamePersistent->actor_torch;
    const bool torch_on = T.range > 0.f && T.frame + 1 >= Device.dwFrame && T.frame <= Device.dwFrame;
    const float fade_step = Device.fTimeDelta / 0.25f; // fade in / out over 0.25 s

    auto was_selected = [this](const void* key) {
        for (u32 i = 0; i < ssfx_il_slot_count; ++i)
            if (ssfx_il_slots[i].key == key && ssfx_il_slots[i].fade > 0.f)
                return true;
        return false;
    };

    // light: a live light of this frame ( from the package or the spatial DB ), never an old slot's key
    auto consider = [&](const light* L) {
        if ((L->flags.type != IRender_Light::POINT && L->flags.type != IRender_Light::SPOT) || L->flags.bHudMode || L->range < 0.5f)
            return;
        for (const auto& c : cands)
            if (c.L == L)
                return; // another OMNIPART piece of the same omni light, or found by both sources
        const float cam_d2 = Device.vCameraPosition.distance_to_sqr(L->position);
        if (cam_d2 < 0.3f * 0.3f)
            return; // .208: lights at the camera ( was 1.5 m: standing at a fruit punch dropped its fill )
        if (torch_on && (L->position.distance_to_sqr(T.pos) < 0.05f * 0.05f || L->position.distance_to_sqr(T.omni_pos) < 0.05f * 0.05f))
            return; // the player's torch spot and omni: the spot has the analytic torch bounce
        const float lod = L->flags.bShadow ? L->get_LOD() : 1.f;
        const float lum = (0.3f * L->color.r + 0.59f * L->color.g + 0.11f * L->color.b) * lod;
        if (lum < 0.01f)
            return;
        // candidate test with the untraced fill ( .210: the traced one is only made for the lights picked below ). For a
        // spot it is a bound around the lamp; the upload tests the real fill sphere again
        Fvector c;
        const float reach = ssfx_il_reach(ssfx_il_light_fill(L->position, L->direction, L->range, L->flags.type == IRender_Light::SPOT, c));
        if (!RImplementation.ViewBase.testSphere_dirty(c, reach))
            return; // .208: its fill cannot reach a visible pixel
        float score = lum * L->range / (1.f + cam_d2 / 100.f);
        if (was_selected(L))
            score *= 1.25f; // hysteresis: a light in the list keeps its slot against a close rival
        cands.push_back({L, score});
    };

    // 1. The render's light package: lights whose range sphere is in a visible sector and the frustum ( HOM-culled )
    auto consider_piece = [&](const light* piece) {
        if (!piece || !piece->vis.visible)
            return;
        consider((piece->flags.type == IRender_Light::OMNIPART && piece->ssfx_parent) ? piece->ssfx_parent : piece);
    };
    for (const light* L : LP.v_point)
        consider_piece(L);
    for (const light* L : LP.v_spot)
        consider_piece(L);
    for (const light* L : LP.v_shadowed)
        consider_piece(L);

    // 2. .208: lights within 16 m whose range sphere left the frustum ( source behind the player ) but whose fill
    // still reaches the view, while their sector is reached by this frame's portal traversal ( no rooms behind walls )
    {
        auto& dsgraph = RImplementation.get_imm_context();
        if (dsgraph.PortalTraverser.frame() == Device.dwFrame)
        {
            static xr_vector<ISpatial*> near_lights;
            near_lights.clear();
            g_SpatialSpace->q_sphere(near_lights, 0, STYPE_LIGHTSOURCE, Device.vCameraPosition, 16.f);
            for (ISpatial* spatial : near_lights)
            {
                const auto sector_id = spatial->spatial.sector_id;
                if (sector_id == IRender_Sector::INVALID_SECTOR_ID || sector_id >= dsgraph.sector_portals_structure.Sectors.size())
                    continue;
                if (dsgraph.PortalTraverser.marker() != dsgraph.sector_portals_structure.Sectors[sector_id]->r_marker)
                    continue; // sector not seen this frame
                const light* L = smart_cast<light*>(spatial->dcast_Light());
                if (L && L->flags.type != IRender_Light::OMNIPART)
                    consider(L);
            }
        }
    }

    const size_t top = std::min<size_t>(cands.size(), SSFX_IL_LIGHTS_TOP);
    std::partial_sort(cands.begin(), cands.begin() + top, cands.end(), [](const cand& a, const cand& b) { return a.score > b.score; });

    ssfx_il_light_slot next[SSFX_IL_LIGHTS_MAX]{};
    u32 n = 0;

    // .210: the maps of last frame's slots stay taken this frame ( a slot that fades out below keeps its map )
    for (u32 j = 0; j < ssfx_il_slot_count; ++j)
        if (ssfx_il_slots[j].vis >= 0)
            ssfx_il_vis[ssfx_il_slots[j].vis].used_frame = Device.dwFrame;

    // Selected lights: values from this frame, fade carried over from the previous slot of the same light
    for (size_t i = 0; i < top; ++i)
    {
        const light* L = cands[i].L;
        auto& S = next[n++];
        S.key = L;
        S.pos = L->position;
        S.dir = L->direction;
        S.spot = L->flags.type == IRender_Light::SPOT;
        S.range = L->range;
        // .210: fill from the beam's hit point ( spot ), colour x the falloff there; visibility map of this light
        float att = 1.f;
        S.reach = ssfx_il_light_fill_traced(L->position, L->direction, L->range, L->cone, S.spot, S.centre, att);
        S.vis = ssfx_il_vis_acquire(L);
        const float lod = (L->flags.bShadow ? L->get_LOD() : 1.f) * att;
        S.col.set(L->color.r * lod, L->color.g * lod, L->color.b * lod);
        S.fade = fade_step;
        for (u32 j = 0; j < ssfx_il_slot_count; ++j)
            if (ssfx_il_slots[j].key == L)
            {
                S.fade = ssfx_il_slots[j].fade + fade_step;
                break;
            }
        S.fade = std::min(S.fade, 1.f);
        if (S.vis < 0 || !ssfx_il_vis[S.vis].valid)
            S.fade = 0.f; // .210: the fill waits for its map ( a few frames ), then fades in
    }
    const u32 live_slots = n;

    // Lights that left the list fade out with their last values ( never dereferenced: they may be gone )
    for (u32 j = 0; j < ssfx_il_slot_count && n < SSFX_IL_LIGHTS_MAX; ++j)
    {
        const auto& O = ssfx_il_slots[j];
        bool kept = false;
        for (u32 i = 0; i < n; ++i)
            if (next[i].key == O.key)
            {
                kept = true;
                break;
            }
        if (kept || O.fade - fade_step <= 0.f)
            continue;
        next[n] = O;
        next[n].fade = O.fade - fade_step;
        ++n;
    }

    std::copy(std::begin(next), std::end(next), std::begin(ssfx_il_slots));
    ssfx_il_slot_count = n;

    ssfx_il_vis_update(live_slots); // .210: trace this frame's share of the visibility maps

    if (ps_ssfx_il_lights_info)
    {
        ps_ssfx_il_lights_info = 0;
        Msg("~ [il lights] %u slots ( %u candidates ), strength %.2f, debug %d", n, static_cast<u32>(cands.size()), ps_ssfx_il_lights, ps_ssfx_il_lights_debug);
        for (u32 i = 0; i < n; ++i)
        {
            const auto& S = ssfx_il_slots[i];
            Msg("~ [il lights] %u: %p %s pos %.1f,%.1f,%.1f range %.2f colour x LOD %.2f,%.2f,%.2f fade %.2f, %.1f m", i, S.key, S.spot ? "spot" : "point", S.pos.x, S.pos.y, S.pos.z,
                S.range, S.col.x, S.col.y, S.col.z, S.fade, Device.vCameraPosition.distance_to(S.pos));
            // .210: the fill itself and its visibility map
            const bool mapped = S.vis >= 0 && ssfx_il_vis[S.vis].valid;
            Msg("~ [il lights]    fill centre %.1f,%.1f,%.1f reach %.2f, map %s%s", S.centre.x, S.centre.y, S.centre.z, ssfx_il_reach(S.reach), mapped ? "ready" : "pending",
                S.vis >= 0 && ssfx_il_vis[S.vis].building ? " ( tracing )" : "");
        }
    }
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

    cmd_list.set_c("ssfx_scene_prev_k", ssfx_scene_prev_strength(), 0.f, 0.f, 0.f);

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

// NLC anomaly haze ( docs/ANOMALY_HAZE_DESIGN.md 3 ): registered zone volumes ( Environment.h ) nearest first, culled by
// the view frustum, then drawn far to near, each as a screen triangle scissored to the projected bounding sphere.
bool CRenderTarget::phase_ssfx_haze(CBackend& cmd_list)
{
    ps_ssfx_haze_stat_drawn = 0;
    ps_ssfx_haze_stat_cover = 0.f;

    if (!ps_ssfx_haze || ps_ssfx_compare || !g_pGameLevel || !s_ssfx_haze)
        return false;

    // fades advance once per frame even if the scene renders twice
    static u32 last_frame = u32(-1);
    const float dt = last_frame != Device.dwFrame ? Device.fTimeDelta : 0.f;
    last_frame = Device.dwFrame;

    const xr_vector<SNlcHazeZone*>& zones = nlc_haze_collect(dt, ps_ssfx_haze_dist);
    if (zones.empty())
        return false;

    struct Item
    {
        const SNlcHazeZone* z;
        const SNlcHazeProfile* p;
        float R;
        Irect rect;
        float key; // distance from the camera to the centre
        int rank; // position in last frame's draw order, -1 = new
    };
    xr_vector<Item> items;
    items.reserve(zones.size());

    const float W = static_cast<float>(rt_Generic_0->dwWidth), H = static_cast<float>(rt_Generic_0->dwHeight);
    const Fmatrix& M = Device.mFullTransform;
    const Fvector& cam = Device.vCameraPosition;

    for (const SNlcHazeZone* z : zones)
    {
        if (int(items.size()) >= ps_ssfx_haze_max)
            break;
        const SNlcHazeProfile* p = nlc_haze_profile_get(z->profile);
        const float R = z->radius * p->radius_k;
        if (R <= 0.01f || !RImplementation.ViewBase.testSphere_dirty(z->centre, R))
            continue;

        // screen rectangle of the sphere's bounding box; camera inside or a corner behind the camera: full screen
        Irect rc{0, 0, int(W), int(H)};
        if (cam.distance_to(z->centre) > R + 0.5f)
        {
            float x0 = flt_max, y0 = flt_max, x1 = -flt_max, y1 = -flt_max;
            bool behind = false;
            for (u32 i = 0; i < 8 && !behind; ++i)
            {
                const Fvector v{z->centre.x + ((i & 1) ? R : -R), z->centre.y + ((i & 2) ? R : -R), z->centre.z + ((i & 4) ? R : -R)};
                const float cx = v.x * M._11 + v.y * M._21 + v.z * M._31 + M._41;
                const float cy = v.x * M._12 + v.y * M._22 + v.z * M._32 + M._42;
                const float cw = v.x * M._14 + v.y * M._24 + v.z * M._34 + M._44;
                if (cw <= 0.05f)
                {
                    behind = true;
                    break;
                }
                const float sx = (cx / cw * 0.5f + 0.5f) * W, sy = (0.5f - cy / cw * 0.5f) * H;
                x0 = std::min(x0, sx);
                x1 = std::max(x1, sx);
                y0 = std::min(y0, sy);
                y1 = std::max(y1, sy);
            }
            if (!behind)
            {
                rc.x1 = std::clamp(int(x0) - 2, 0, int(W));
                rc.y1 = std::clamp(int(y0) - 2, 0, int(H));
                rc.x2 = std::clamp(int(x1) + 3, 0, int(W));
                rc.y2 = std::clamp(int(y1) + 3, 0, int(H));
            }
        }
        if (rc.x2 <= rc.x1 || rc.y2 <= rc.y1)
            continue;
        items.push_back({z, p, R, rc, cam.distance_to(z->centre), -1});
    }
    if (items.empty())
        return false;

    // Draw order far to near by the distance to the centre. Volumes within 0.5 m of each other in that distance keep
    // last frame's order: the blend is order dependent and camera bob swapped overlapping volumes every frame ( E2 )
    static xr_vector<u16> prev_order; // far to near
    for (Item& it : items)
    {
        const auto pos = std::find(prev_order.begin(), prev_order.end(), it.z->id);
        it.rank = pos == prev_order.end() ? -1 : int(pos - prev_order.begin());
    }
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.key > b.key; });
    for (u32 pass = 0; pass < 2; ++pass)
        for (u32 i = 0; i + 1 < items.size(); ++i)
        {
            Item& a = items[i];
            Item& b = items[i + 1];
            if (a.key - b.key < 0.5f && a.rank >= 0 && b.rank >= 0 && b.rank < a.rank)
                std::swap(a, b);
        }
    prev_order.clear();
    for (const Item& it : items)
        prev_order.push_back(it.z->id);

    PIX_EVENT_CTX(cmd_list, ssfx_haze);

    // light shared by all volumes ( weather ): ambient = hemi + ambient, sun colour and direction
    const CEnvDescriptorMixer& env = *g_pGamePersistent->Environment().CurrentEnv;
    const Fvector amb{env.hemi_color.x + env.ambient.x, env.hemi_color.y + env.ambient.y, env.hemi_color.z + env.ambient.z};
    Fvector to_sun = env.sun_dir;
    to_sun.invert().normalize_safe();
    const float sun_k = ps_ssfx_is_underground ? 0.f : 1.f;
    const Fvector4& LK = ps_ssfx_haze_light;
    const float split = (ps_ssfx_haze_split || ps_ssfx_fog_split) ? 1.f : 0.f;
    const float time = std::fmod(Device.fTimeGlobal, 3600.f);
    const float frame = float(Device.dwFrame % 64);

    for (auto it = items.begin(); it != items.end(); ++it)
    {
        const SNlcHazeZone& z = *it->z;
        const SNlcHazeProfile& p = *it->p;
        Irect rc = it->rect;
        const Fvector& a = p.albedo;
        const float sun_vis = z.sun_vis * sun_k;
        // per-profile multipliers ( E5 ), blowout pulse ( E4 )
        const float k_amb = LK.x * p.ambient_k, k_sun = LK.y * p.sun_k, k_em = LK.w * p.emissive_k;
        const float dens = p.density * z.k * ps_ssfx_haze_density * (1.f + (p.blowout_density - 1.f) * z.pulse);
        const float ls = p.light_scatter * LK.z * p.light_k * (1.f + (p.blowout_light - 1.f) * z.pulse);
        const float lrange = z.light_col.magnitude() > 0.001f ? z.light_range * p.light_range_k : 0.f;

        RenderScreenTriangle(cmd_list, rt_Generic_0, s_ssfx_haze->E[0], [&]() {
            cmd_list.set_Scissor(&rc);
            cmd_list.set_c("haze_p0", z.centre.x, z.centre.y, z.centre.z, it->R);
            cmd_list.set_c("haze_p1", p.height, dens, p.falloff, p.noise_amount);
            cmd_list.set_c("haze_p2", amb.x * a.x * k_amb, amb.y * a.y * k_amb, amb.z * a.z * k_amb, float(p.shape));
            cmd_list.set_c("haze_p3", env.sun_color.x * a.x * sun_vis * k_sun, env.sun_color.y * a.y * sun_vis * k_sun, env.sun_color.z * a.z * sun_vis * k_sun, p.sun_g);
            cmd_list.set_c("haze_p4", p.noise_scale, p.noise_drift.x, p.noise_drift.y, p.noise_drift.z);
            cmd_list.set_c("haze_p5", p.emissive.x * k_em, p.emissive.y * k_em, p.emissive.z * k_em, float(ps_ssfx_haze_samples));
            cmd_list.set_c("haze_p6", z.light_pos.x, z.light_pos.y, z.light_pos.z, lrange);
            cmd_list.set_c("haze_p7", z.light_col.x * a.x * ls, z.light_col.y * a.y * ls, z.light_col.z * a.z * ls, float(ps_ssfx_haze_debug));
            cmd_list.set_c("haze_p8", to_sun.x, to_sun.y, to_sun.z, 0.f);
            cmd_list.set_c("haze_p9", time, frame, split, 0.f);
        });

        ps_ssfx_haze_stat_cover += float((rc.x2 - rc.x1) * (rc.y2 - rc.y1)) / (W * H);
        ++ps_ssfx_haze_stat_drawn;
    }

    cmd_list.set_Scissor(nullptr);
    return true;
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
    // NLC .210: w = strength of surfaces under cover while it rains ( ssfx_ssr_dry; the shader had 0 there )
    const Fvector4 setup{strength, ps_ssfx_ssr_blur, ps_ssfx_ssr_terrain ? 1.f : 0.f, ps_ssfx_ssr_dry};

    // Reflection + temporal accumulation ( reads rt_ssfx_ssr_hist )
    RenderScreenTriangle(cmd_list, rt_ssfx_ssr, s_ssfx_ssr->E[0], [&]() {
        cmd_list.set_c("ssr_setup", ps_ssfx_ssr_1);
        cmd_list.set_c("ssfx_ssr_2", ps_ssfx_ssr_2);
        cmd_list.set_c("ssfx_ssr_setup", setup);
        cmd_list.set_c("ssfx_scene_prev_k", ssfx_scene_prev_strength(), 0.f, 0.f, 0.f);
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

// NLC .209: SSR and the water prepass run before the forward pass, so they could only reflect the deferred scene.
// The finished scene ( forward pass, haze, volumetric light, distortion; before fog scattering and bloom ) is kept for
// the next frame, where a reflection hit looks it up through its motion vector ( ssfx_scene_prev.h ).
void CRenderTarget::phase_ssfx_scene_prev(CBackend& cmd_list)
{
    if (!rt_ssfx_scene_prev || ps_ssfx_compare || ps_ssfx_ssr_forward <= 0.f)
        return;

    // NLC .210: only when a reflection pass ran this frame ( it will most likely run next frame too ); dry weather without
    // water skips the full-resolution copy. After a pause the first frame falls back to the current scene copy
    if (!ssfx_ssr_composited && ssfx_water_frame != Device.dwFrame)
        return;

    HW.get_context(cmd_list.context_id)->CopyResource(rt_ssfx_scene_prev->pSurface, rt_Generic_0->pSurface);
    ssfx_scene_prev_frame = Device.dwFrame;
}

float CRenderTarget::ssfx_scene_prev_strength() const
{
    // the copy of the frame before this one, and a history that survived ( no resize, level change, camera cut )
    const bool valid = rt_ssfx_scene_prev && ssfx_history_valid && ssfx_scene_prev_frame && ssfx_scene_prev_frame + 1 == Device.dwFrame;
    return valid ? ps_ssfx_ssr_forward : 0.f;
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
