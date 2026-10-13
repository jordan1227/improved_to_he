#pragma once

// NLC: SSS 23 passes ( Build A: AO, IL, screen-space shadows; Build B: water, glass, SSR ).
// One place that decides whether a pass is on, used by the shader macros ( r4.cpp ) and by
// CRenderTarget when it creates the pass render targets.
//
// vid_restart rebuilds render targets but not compiled shaders, so the on/off and quality state is
// a snapshot taken once per game session ( first use ). Shader macros, targets and passes therefore
// always agree; changing a switch or a quality level needs a game restart ( the menu says so ).
// Resolution dividers are read live: they only size the targets.

#include "../xrRender/xrRender_console.h"

struct ssfx_feature_state
{
    bool ao{}, il{}, sss{}, sss_lights{};
    int ao_samples{}, il_samples{}, sss_dir_samples{}, sss_omni_samples{};

    bool water{}, glass{}, ssr{}, foliage{}, flora_variation{};
    bool gusts{}, fog_sun{}, cloud_shadows{}, step_ripples{}, spec_aa{}, mblur_objects{}, ground_mist{}, rain_beams{};
    int water_quality{}, water_parallax{}; // SSFX_WATER_QUALITY ( q_steps index 1..3 ), SSFX_WATER_PARALLAX 0..3
};

inline const ssfx_feature_state& ssfx_features()
{
    static const ssfx_feature_state state = [] {
        constexpr int sss_dir[3] = {8, 12, 20};

        ssfx_feature_state s;
        s.ao = ps_r_ao_mode == AO_MODE_SSFX && ps_r_ao_quality > 0;
        s.il = ps_r2_ls_flags.test(R2FLAG_SSFX_INDIRECT_LIGHT);
        s.sss = ps_r2_ls_flags.test(R2FLAGEXT_SSFX_SSS);
        s.sss_lights = s.sss && ps_ssfx_sss_lights != 0;
        s.ao_samples = std::clamp(static_cast<int>(ps_r_ao_quality), 1, 3) + 1; // 2 / 3 / 4 directions ( x2 rings )
        s.il_samples = std::clamp(ps_ssfx_il_quality, 1, 3) * 8 + 8; // 16 / 24 / 32
        s.sss_dir_samples = sss_dir[std::clamp(ps_ssfx_sss_quality, 1, 3) - 1];
        s.sss_omni_samples = ps_ssfx_sss_quality >= 3 ? 6 : 4;

        s.water = !!ps_ssfx_water_enable;
        s.water_quality = std::clamp(ps_ssfx_water_quality, 1, 3);
        s.water_parallax = std::clamp(ps_ssfx_water_parallax, 0, 3);
        s.glass = !!ps_ssfx_glass_enable;
        s.foliage = ps_ssfx_tree_variation > 0.f || ps_ssfx_foliage_mask; // tree branch shaders + foliage material choice
        s.flora_variation = ps_ssfx_flora_variation > 0.f || ps_ssfx_tree_variation > 0.f; // grass / tree colour variation code
        s.gusts = ps_ssfx_wind_gusts > 0.f;
        s.fog_sun = ps_ssfx_fog_sun > 0.f;
        s.cloud_shadows = ps_ssfx_cloud_shadows > 0.f;
        s.ground_mist = ps_ssfx_ground_mist > 0.f;
        s.rain_beams = !!ps_ssfx_rain_beams;
        s.step_ripples = ps_ssfx_step_ripples > 0.f;
        s.spec_aa = !!ps_ssfx_spec_aa;
        s.mblur_objects = !!ps_ssfx_mblur_objects;
        s.ssr = ps_ssfx_ssr_enable && ps_r2_ls_flags_ext.test(R2FLAGEXT_SSLR); // the SSR pass itself is r_sslr_enable

        Msg("* [SSFX] session state: AO %d ( %d ), IL %d ( %d ), SSS %d ( %d ), light SSS %d ( %d ), water %d ( %d / %d ), glass %d, SSR %d, foliage %d, gusts %d, sun fog %d, cloud shadows %d, step ripples %d, spec AA %d, object blur %d, flora variation %d, ground mist %d, rain beams %d", s.ao,
            s.ao_samples, s.il, s.il_samples, s.sss, s.sss_dir_samples, s.sss_lights, s.sss_omni_samples, s.water, s.water_quality, s.water_parallax,
            s.glass, s.ssr, s.foliage, s.gusts, s.fog_sun, s.cloud_shadows, s.step_ripples, s.spec_aa, s.mblur_objects, s.flora_variation, s.ground_mist, s.rain_beams);
        return s;
    }();
    return state;
}

inline bool ssfx_ao_pass_enabled() { return ssfx_features().ao; }
inline bool ssfx_il_pass_enabled() { return ssfx_features().il; }
inline bool ssfx_sss_pass_enabled() { return ssfx_features().sss; }
inline bool ssfx_sss_lights_enabled() { return ssfx_features().sss_lights; }
inline bool ssfx_water_enabled() { return ssfx_features().water; }
inline bool ssfx_glass_enabled() { return ssfx_features().glass; }
inline bool ssfx_ssr_enabled() { return ssfx_features().ssr; }
inline bool ssfx_foliage_enabled() { return ssfx_features().foliage; }

// Passes that read the previous frame position buffer ( rt_ssfx_prev_pos )
inline bool ssfx_temporal_enabled() { return ssfx_ao_pass_enabled() || ssfx_il_pass_enabled() || ssfx_sss_pass_enabled() || ssfx_ssr_enabled(); }
// Passes that need the view -> clip matrices of this and the previous frame
inline bool ssfx_matrices_enabled() { return ssfx_temporal_enabled() || ssfx_water_enabled(); }

inline int ssfx_ao_samples() { return ssfx_features().ao_samples; }
inline int ssfx_il_samples() { return ssfx_features().il_samples; }
inline int ssfx_sss_dir_samples() { return ssfx_features().sss_dir_samples; }
inline int ssfx_sss_omni_samples() { return ssfx_features().sss_omni_samples; }
