/**
 * @ Description: Tree branches / bushes, deferred ( deffer_tree_branch_aref_*.ps, Blender_tree with SSFX_FOLIAGE_PASS )
 * @ Based on: SCREEN SPACE SHADERS - UPDATE 14 / 18 flora fixes ( deffer_base_aref_bump-hq.ps ) and the SSS 23
 *            branch shaders ( https://www.moddb.com/members/ascii1457 )
 * @ NLC: 2026-10-12 - branches are always foliage ( MAT_FLORA ), also with detail textures; per-tree colour
 *        variation from the branch VS ( RDrops.xy, ssfx_flora_variation.h ), strength ssfx_flora_variation.y.
 */

#include "common.h"
#include "sload.h"
#include "common_brdf.h"
#include "settings_screenspace_FLORA.h"
#include "ssfx_flora_variation.h"

f_deffer main(p_bumped I)
{
    f_deffer O;

    surface_bumped S = sload(I);

    calc_alpha_test(I.tcdh.xy, S.base.a);

    // FLORA FIXES & IMPROVEMENTS - SSS Update 18
    S.normal.xy += float2(.02f, .02f);
    S.normal.xy *= max(5.0f * rain_params.y, 3.0f);

    // FLORA FIXES & IMPROVEMENTS - SSS Update 14: fake normal for a soft foliage look
    float3 Ne = mul(m_WV, (float3(S.normal.x, 1.0f, S.normal.y)));
    Ne = normalize(Ne);

    float ms = MAT_FLORA;

    S.gloss = lerp(ssfx_florafixes_1.z, ssfx_florafixes_1.w, rain_params.y);

#ifdef G_FLORA_GRASS_VARIATION
    // Same dry / lush patches as the grass, strength of the tree slider
    float plant = saturate((S.base.g - S.base.b) * 6.0f);
    float3 tint = I.RDrops.x >= 0.0f ? lerp(1.0f, G_FLORA_GRASS_DRY_TINT, I.RDrops.x) : lerp(1.0f, G_FLORA_GRASS_LUSH_TINT, -I.RDrops.x);
    tint *= 1.0f + I.RDrops.y * G_FLORA_GRASS_BRIGHTNESS;
    S.base.rgb = saturate(S.base.rgb * lerp(1.0f, tint, G_FLORA_GRASS_VARIATION * ssfx_flora_variation.y * plant));
#endif

#ifdef USE_LM_HEMI
    float4 lm = s_hemi.Sample(smp_rtlinear, I.lmh);
    float h = get_hemi(lm);
#ifdef USE_R2_STATIC_SUN
    ms = get_sun(lm);
#endif
#else
    float h = I.position.w;
#ifdef USE_R2_STATIC_SUN
    ms = I.tcdh.w;
#endif
#endif

    O = pack_gbuffer(float4(Ne, h), float4(I.position.xyz + Ne * S.height * def_virtualh, ms), float4(S.base.rgb, S.gloss));

    O.Velocity = get_motion_vector(I.hpos_curr, I.hpos_old);

    return O;
}
