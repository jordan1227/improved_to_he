/**
 * @ Description: Cloud shadows: the weather's own cloud textures projected along the sun direction
 * @ NLC: 2026-10-13. The visible clouds ( clouds.vs: dome object space, CLOUD_TILE / CLOUD_SPEED, fixed scroll directions
 *        setHP( PI/4 ) and setHP( 3PI/8 ), dome rotated by sky_rotation and scaled 10 / 0.4 / 10, see
 *        dxEnvironmentRender::RenderClouds ) are mapped onto a layer above the camera; each lit point looks up the cloud
 *        between it and the sun, so the shadows match the sky and move with it. Overcast weather shades more, clear
 *        weather with thin clouds hardly at all. Strength ssfx_clouds.z ( SSFX_CLOUD_SHADOWS ).
 */

#ifndef SSFX_CLOUD_SHADOWS_H
#define SSFX_CLOUD_SHADOWS_H

#include "shared\cloudconfig.h"

Texture2D s_ssfx_clouds0; // current weather clouds ( $user$ssfx_clouds0 / 1, set every frame )
Texture2D s_ssfx_clouds1;

uniform float4 ssfx_clouds; // x: sky rotation, y: clouds opacity ( clouds_color.w ), z: strength, w: 0 ssfx_compare, 1 on, 2 debug view

#define SSFX_CLOUD_LAYER_HEIGHT 600.0f // metres above the camera
#define SSFX_CLOUD_REPEAT 1000.0f // metres per texture repeat at tile 1 ( layer 0 ~1.4 km, layer 1 ~360 m )
#define SSFX_CLOUD_SOFTNESS 2.0f // texture mip level: softer shadow edges

// NLC .81: the exact sky-dome projection made each cloud shadow 0.5 ~ 1 km wide ( the dome is flat and wide,
// scale 10 / 0.4 / 10 ), so on the ground it read as uniform dimming. The shadows now use a world-space layer with the
// same textures, tiles, scroll directions and speeds as clouds.vs ( rotated by the sky rotation ), so patches of
// ~100 - 250 m drift across the ground the way the sky scrolls.
// Returns the cloud opacity between the point and the sun ( 0 ~ 1 ), -1 when the sun is too low.
float ssfx_cloud_opacity(float3 P_view)
{
    float3 W = mul(m_inv_V, float4(P_view, 1.0f)).xyz;
    float3 to_sun = -normalize(L_sun_dir_w.xyz); // sun_dir is the light travel direction
    if (to_sun.y < 0.05f)
        return -1.0f;

    // Point on the cloud layer between W and the sun
    float2 C = W.xz + to_sun.xz * ((eye_position.y + SSFX_CLOUD_LAYER_HEIGHT - W.y) / to_sun.y);

    // World -> dome object space orientation ( inverse rotateY( sky_rotation ) )
    float s, c;
    sincos(ssfx_clouds.x, s, c);
    float2 o = float2(c * C.x - s * C.y, s * C.x + c * C.y) / SSFX_CLOUD_REPEAT;

    const float2 d0 = float2(-0.7071068f, 0.7071068f);
    const float2 d1 = float2(-0.9238795f, 0.3826834f);
    float2 tc0 = o * CLOUD_TILE0 + d0 * timers.z * CLOUD_SPEED0;
    float2 tc1 = o * CLOUD_TILE1 + d1 * timers.z * CLOUD_SPEED1;

    float a0 = s_ssfx_clouds0.SampleLevel(smp_linear, tc0, SSFX_CLOUD_SOFTNESS).a;
    float a1 = s_ssfx_clouds1.SampleLevel(smp_linear, tc1, SSFX_CLOUD_SOFTNESS).a;

    return saturate((a0 + a1) * ssfx_clouds.y);
}

float ssfx_cloud_shadow(float3 P_view)
{
    float opacity = ssfx_cloud_opacity(P_view);
    if (opacity < 0.0f)
        return 1.0f;
    return 1.0f - ssfx_clouds.z * saturate(ssfx_clouds.w) * smoothstep(0.15f, 0.6f, opacity);
}

// Debug view ( ssfx_clouds.w = 2, ssfx_debug_clouds 1 ): r = cloud opacity, g = sun high enough, b = weather clouds opacity
float3 ssfx_cloud_debug(float3 P_view)
{
    float opacity = ssfx_cloud_opacity(P_view);
    return float3(saturate(opacity), opacity >= 0.0f ? 0.5f : 0.0f, ssfx_clouds.y * 0.5f);
}

#endif // SSFX_CLOUD_SHADOWS_H
