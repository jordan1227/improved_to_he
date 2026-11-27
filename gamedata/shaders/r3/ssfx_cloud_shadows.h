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
#include "ssfx_fog_params.h" // .237: ssfx_fog_gate2.w, the level base height
#include "nlc_sun_breaks.h" // .239: nlc_sun_breaks

Texture2D s_ssfx_clouds0; // current weather clouds ( $user$ssfx_clouds0 / 1, set every frame )
Texture2D s_ssfx_clouds1;

uniform float4 ssfx_clouds; // x: sky rotation, y: clouds opacity ( clouds_color.w ), z: strength, w: 0 ssfx_compare, 1 on, 2 debug view
uniform float4 nlc_cloud_ofs; // NLC: cloud texture offsets ( layer 0 xy, layer 1 zw ), integrated from the wind ( CEnvironment::nlc_clouds_update )

#define SSFX_CLOUD_LAYER_HEIGHT 600.0f // metres above the level base
#define SSFX_CLOUD_REPEAT 1000.0f // metres per texture repeat at tile 1 ( layer 0 ~1.4 km, layer 1 ~360 m )
#define SSFX_CLOUD_SOFTNESS 2.0f // texture mip level: softer shadow edges

// NLC .81: the exact sky-dome projection made each cloud shadow 0.5 ~ 1 km wide ( the dome is flat and wide,
// scale 10 / 0.4 / 10 ), so on the ground it read as uniform dimming. The shadows now use a world-space layer with the
// same textures, tiles, scroll directions and speeds as clouds.vs ( rotated by the sky rotation ), so patches of
// ~100 - 250 m drift across the ground the way the sky scrolls.
// NLC K1 ( docs/RAIN_CLOUDS_DESIGN.md ): opacity ( a0 + a1 ) x clouds_color.w stayed below the 0.15 ramp almost
// everywhere in sunny weathers ( clouds_color.w 0.17 - 0.36; debug view all green, 2026-11-26 ), so the slider only
// darkened the rare thick spots. The shadow now thresholds the cloud density relative to the texture's own mean ( its
// smallest mip ): the slider sets both how much of the ground is covered ( ~30 - 40 % at 1 in clear weather ) and how
// dark it gets ( 90 % of the direct sun at 1 ); weathers with more clouds cover more. The drift follows the wind
// ( nlc_cloud_ofs, the same offsets as the sky in clouds.vs ).
float ssfx_cloud_mean(Texture2D t)
{
    uint w, h, levels;
    t.GetDimensions(0, w, h, levels);
    return levels >= 4 ? t.SampleLevel(smp_linear, float2(0.5f, 0.5f), levels - 1).a : 0.35f;
}

// Cloud density between the point and the sun relative to the sky's mean ( 1 = average ), -1 when the sun is too low
float ssfx_cloud_density(float3 P_view)
{
    float3 W = mul(m_inv_V, float4(P_view, 1.0f)).xyz;
    float3 to_sun = -normalize(L_sun_dir_w.xyz);
    if (to_sun.y < 0.05f)
        return -1.0f;

    // .237: the layer sits at a fixed height above the level base ( ssfx_fog_gate2.w, fog_base ), not above the camera:
    // a camera moving up or down ( demo_record, a ladder, a jump ) slid the shadows by its height change / tan( sun
    // elevation ) on the ground
    float2 C = W.xz + to_sun.xz * ((ssfx_fog_gate2.w + SSFX_CLOUD_LAYER_HEIGHT - W.y) / to_sun.y);

    float s, c;
    sincos(ssfx_clouds.x, s, c);
    float2 o = float2(c * C.x - s * C.y, s * C.x + c * C.y) / SSFX_CLOUD_REPEAT;

    float2 tc0 = o * CLOUD_TILE0 + nlc_cloud_ofs.xy;
    float2 tc1 = o * CLOUD_TILE1 + nlc_cloud_ofs.zw;

    float a0 = s_ssfx_clouds0.SampleLevel(smp_linear, tc0, SSFX_CLOUD_SOFTNESS).a;
    float a1 = s_ssfx_clouds1.SampleLevel(smp_linear, tc1, SSFX_CLOUD_SOFTNESS).a;
    float m = ssfx_cloud_mean(s_ssfx_clouds0) + ssfx_cloud_mean(s_ssfx_clouds1);

    // floor 0.2: a nearly empty texture would turn its faint wisps into full shadows; and the visible alpha itself must
    // reach a little ( the sky shows ( a0 + a1 ) x clouds_color.w )
    return (a0 + a1) / max(m, 0.2f) * smoothstep(0.01f, 0.06f, (a0 + a1) * ssfx_clouds.y);
}

// K4 ( 5.4 ): 0 .. 1, how far the sun-break mode applies ( nlc_sun_breaks.h )
float ssfx_cloud_breaks_k() { return nlc_sun_breaks_k(); }

// Shadow amount ( 0 none .. 1 full ) from the relative density, the slider and the weather's cloudiness
float ssfx_cloud_amount(float density)
{
    float k = ssfx_clouds.z; // slider
    float w = ssfx_clouds.y; // clouds_color.w
    float thr = 1.9f - 0.75f * k * (0.6f + 0.8f * w);
    float dark = 0.9f * k * smoothstep(0.03f, 0.2f, w);
    // K4: an overcast section with the sun on ( nlc_sun_breaks = share in shadow ): the threshold drops until about that
    // share is covered and the shadow is full whatever the slider, so the sun only comes through the thin gaps. The
    // threshold for a share is a first guess ( the texture histogram is unmeasured ): tune with ssfx_debug_clouds 1
    float kb = ssfx_cloud_breaks_k();
    thr = lerp(thr, 0.15f + 1.6f * (1.0f - nlc_sun_breaks.x), kb);
    dark = lerp(dark, 1.0f, kb);
    float cover = smoothstep(thr - 0.2f, thr + 0.2f, density);
    return cover * dark;
}

float ssfx_cloud_shadow(float3 P_view)
{
    float kb = ssfx_cloud_breaks_k();
    float density = ssfx_cloud_density(P_view);
    if (density < 0.0f)
        return 1.0f - kb; // K4: a low sun stays behind the overcast
    // .239: with ssfx_compare ( w = 0 ) a sun-break section stays fully overcast ( was full sun everywhere )
    float on = saturate(ssfx_clouds.w);
    return 1.0f - lerp(kb, ssfx_cloud_amount(density), on);
}

// Debug view ( ssfx_clouds.w = 2, ssfx_debug_clouds 1 ): r = shadow amount, g = sun high enough, b = relative cloud
// density / 4 ( 0.25 = the sky's mean )
float3 ssfx_cloud_debug(float3 P_view)
{
    float density = ssfx_cloud_density(P_view);
    return float3(density >= 0.0f ? ssfx_cloud_amount(density) : 0.0f, density >= 0.0f ? 0.5f : 0.0f, saturate(density * 0.25f));
}

#endif // SSFX_CLOUD_SHADOWS_H
