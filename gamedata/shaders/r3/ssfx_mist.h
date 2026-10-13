/**
 * @ Description: Ground mist - drifting patches of fog in the low ground ( hollows, swamps, fields )
 * @ NLC: 2026-10-13. The SSS height fog is a uniform slab that only thickens the distance fog. Here a slow 3D value
 *        noise breaks it into patches that drift with the time, added inside the height fog layer from ~5 m on, so
 *        nearby hollows can hold mist on their own. Shared by combine ( ssfx_fog_sun.h ), fog scattering and water so
 *        they agree. Strength ssfx_extra2.y ( SSFX_GROUND_MIST ).
 */

#ifndef SSFX_MIST_H
#define SSFX_MIST_H

#include "ssfx_extra.h"

float ssfx_mist_hash(float3 p)
{
    p = frac(p * 0.1031f);
    p += dot(p, p.yzx + 33.33f);
    return frac((p.x + p.y) * p.z);
}

float ssfx_mist_noise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    f = f * f * (3.0f - 2.0f * f);

    return lerp(lerp(lerp(ssfx_mist_hash(i), ssfx_mist_hash(i + float3(1, 0, 0)), f.x),
                     lerp(ssfx_mist_hash(i + float3(0, 1, 0)), ssfx_mist_hash(i + float3(1, 1, 0)), f.x), f.y),
                lerp(lerp(ssfx_mist_hash(i + float3(0, 0, 1)), ssfx_mist_hash(i + float3(1, 0, 1)), f.x),
                     lerp(ssfx_mist_hash(i + float3(0, 1, 1)), ssfx_mist_hash(i + float3(1, 1, 1)), f.x), f.y), f.z);
}

// Extra fog amount ( 0 ~ about 0.35 ) at world position Pw, dist metres from the eye, inside the height fog layer
// ( fogheight from the SSS height fog, 0 ~ 1 )
float ssfx_mist(float3 Pw, float dist, float fogheight)
{
    [branch] if (ssfx_extra2.y <= 0.0f || fogheight <= 0.001f)
        return 0.0f;

    float3 p = float3(Pw.x * 0.025f + timers.x * 0.012f, Pw.y * 0.08f, Pw.z * 0.025f + timers.x * 0.007f);
    float n = ssfx_mist_noise(p) * 0.65f + ssfx_mist_noise(p * 2.7f + 11.3f) * 0.35f;
    float patches = smoothstep(0.42f, 0.8f, n);

    return fogheight * patches * ssfx_extra2.y * 0.35f * smoothstep(5.0f, 40.0f, dist);
}

#endif // SSFX_MIST_H
