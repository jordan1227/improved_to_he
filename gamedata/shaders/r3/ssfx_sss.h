/**
 * @ Description: Screen-space shadows, sun and point/spot lights ( temporal pass version )
 * @ Based on: SCREEN SPACE SHADERS - UPDATE 22 screenspace_shadows.h ( https://www.moddb.com/members/ascii1457 )
 * @ NLC: 2026-10-12 - OGSR G-buffer, HUD mask from ssfx_temporal.h, no MSAA paths.
 *        The old inline version ( screenspace_shadows.h, Update 11.3 ) stays for upstream shaders.
 */

#ifndef SSFX_SSS_H
#define SSFX_SSS_H

#include "screenspace_common.h"
#include "settings_screenspace_SSS.h"
#include "ssfx_temporal.h"

uniform float4 ssfx_sss; // Sun length, light length, -, -

#ifndef SSFX_SSS_DIR_QUALITY
#define SSFX_SSS_DIR_QUALITY 12
#endif
#ifndef SSFX_SSS_OMNI_QUALITY
#define SSFX_SSS_OMNI_QUALITY 4
#endif

float SSFX_ScreenSpaceShadows_Sun(float3 P, float2 tc, float HudMask)
{
    if (ssfx_sss.x <= 0.0f) // ssfx_sss_length 0 = off
        return 1;

    bool IsHUD = HudMask > 0;

    float pLen = length(P);

    // Distance factor ( far shadows: thicker and longer rays )
    float DFactor = saturate((pLen - 80.0f) * 0.01f);

    float ray_detail = lerp(G_SSS_DETAIL, 0.05f, pLen * 0.02f);
    float ray_thick = lerp(0.2f, 10.0f, DFactor);
    float ray_len = lerp(1.0f, 20.0f, DFactor);

    ray_len *= saturate(P.z); // Avoid issues when surfaces are extremely close

    if (IsHUD)
    {
        ray_detail = 0;
        ray_len = G_SSS_WEAPON_LENGTH;
        ray_thick = 1.0f;
    }

    float noise = ssfx_hash12(tc * 100.0f + timers.xx * 100.0f);
    RayTrace sss_ray = SSFX_ray_init(P, -L_sun_dir_e, ray_len * ssfx_sss.x, SSFX_SSS_DIR_QUALITY, noise);

    [unroll(SSFX_SSS_DIR_QUALITY)]
    for (int i = 0; i < SSFX_SSS_DIR_QUALITY; i++)
    {
        if (!SSFX_is_valid_uv(sss_ray.r_pos))
            return 1;

        // x = depth difference, y = sampled depth, z = ray length
        float3 depth_ray = SSFX_ray_intersect(sss_ray, 0);
        float diff = depth_ray.x;

        if (depth_ray.y <= SKY_EPS)
            return 1;

        // The HUD must not shadow the ground
        if (!IsHUD)
            diff *= depth_ray.y >= 1.0f;

        if (diff > ray_detail && diff < ray_thick)
            return saturate(1.0f - G_SSS_INTENSITY);

        sss_ray.r_pos += sss_ray.r_step * (1.0f + noise);
    }

    return 1;
}

float SSFX_ScreenSpaceShadows_Light(float3 P, float mtl, float2 tc, float3 LightPos, float HudMask)
{
    if (ssfx_sss.y <= 0.0f)
        return 1;

    float3 L_dir = -normalize(P - LightPos);

    bool mat_flora = abs(mtl - MAT_FLORA) <= 0.04f;
    bool IsHUD = HudMask > 0;

    float ray_detail = G_SSS_DETAIL;
    float ray_thick = 0.2f;
    float ray_len = 0.5f;

    if (IsHUD)
    {
        ray_thick = 1.0f;
        ray_len = 0.06f;
    }
    else if (mat_flora)
    {
        ray_thick = 10.0f;
        ray_len = 0.5f;
    }

    float noise = ssfx_hash12(tc * 100.0f + timers.xx * 100.0f);
    RayTrace sss_ray = SSFX_ray_init(P, L_dir, ray_len * ssfx_sss.y, SSFX_SSS_OMNI_QUALITY, noise);

    [unroll(SSFX_SSS_OMNI_QUALITY)]
    for (int i = 0; i < SSFX_SSS_OMNI_QUALITY; i++)
    {
        if (!SSFX_is_valid_uv(sss_ray.r_pos))
            return 1;

        float3 depth_ray = SSFX_ray_intersect(sss_ray, 0);
        float diff = depth_ray.x;

        if (depth_ray.y <= SKY_EPS)
            return 1;

        if (!IsHUD)
            diff *= depth_ray.y >= 1.0f;

        if (diff > ray_detail && diff < ray_thick)
            return 1.0f - saturate(G_SSS_INTENSITY - smoothstep(SSFX_SSS_OMNI_QUALITY * 0.5f, SSFX_SSS_OMNI_QUALITY + 1, i));

        sss_ray.r_pos += sss_ray.r_step * (1.0f + noise);
    }

    return 1;
}

#endif // SSFX_SSS_H
