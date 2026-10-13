// NLC .84 override of the 3.589 shadow.h: cheaper rotated PCSS filter ( see the quality tokens )
#ifndef SHADOW_H
#define SHADOW_H

#include "common.h"

Texture2D s_smap : register(ps, t0); // 2D/cube shadowmap

SamplerComparisonState smp_smap; //	Special comare sampler
sampler smp_jitter;

uniform float4 ssfx_shadow_bias;

Texture2D jitter0;
Texture2D jitter1;

#define PCSS_PIXEL int(4)
#define PCSS_STEP int(2)
#define PCSS_PIXEL_MIN float(1.0)
#define PCSS_SUN_WIDTH float(150.0)

float sample_hw_pcf(float4 tc, float4 shift)
{
    // Simp: TODO: может быть попробовать тут 1.0? Код юзается в volumetric_sun и rain_patch_normal
    // #ifndef USE_ULTRA_SHADOWS
    static const float KERNEL = 0.6f;
    // #else
    //     static const float KERNEL = 1.0f;
    // #endif
    static const float ts = KERNEL / float(SMAP_size);

    tc.xyz /= tc.w;
    tc.xy += shift.xy * ts;

    return s_smap.SampleCmpLevelZero(smp_smap, tc.xy, tc.z).x;
}

// PCSS shadows
static const float2 poissonDisk[32] = {
    float2(0.0617981, 0.07294159),  float2(0.6470215, 0.7474022),   float2(-0.5987766, -0.7512833), float2(-0.693034, 0.6913887),   float2(0.6987045, -0.6843052),
    float2(-0.9402866, 0.04474335), float2(0.8934509, 0.07369385),  float2(0.1592735, -0.9686295),  float2(-0.05664673, 0.995282),  float2(-0.1203411, -0.1301079),
    float2(0.1741608, -0.1682285),  float2(-0.09369049, 0.3196758), float2(0.185363, 0.3213367),    float2(-0.1493771, -0.3147511), float2(0.4452095, 0.2580113),
    float2(-0.1080467, -0.5329178), float2(0.1604507, 0.5460774),   float2(-0.4037193, -0.2611179), float2(0.5947998, -0.2146744),  float2(0.3276062, 0.9244621),
    float2(-0.6518704, -0.2503952), float2(-0.3580975, 0.2806469),  float2(0.8587891, 0.4838005),   float2(-0.1596546, -0.8791054), float2(-0.3096867, 0.5588146),
    float2(-0.5128918, 0.1448544),  float2(0.8581337, -0.424046),   float2(0.1562584, -0.5610626),  float2(-0.7647934, 0.2709858),  float2(-0.3090832, 0.9020988),
    float2(0.3935608, 0.4609676),   float2(0.3929337, -0.5010948),
};

// Quality tokens --Fine
// NLC .84: fewer taps, each pixel rotates the Poisson disk ( interleaved gradient noise on the shadow map texel plus a
// time offset: TAA / DLSS resolve the noise into a smooth penumbra ), early-out when the first 4 taps agree ( fully lit
// or fully shadowed, most pixels ), blocker search 16 instead of 25 loads. Low ( no SUN_QUALITY, 1 tap ) unchanged.
#if !defined(SUN_QUALITY)
#define PCSS_NUM_SAMPLES 1
#elif SUN_QUALITY == 1
#define PCSS_NUM_SAMPLES 6
#elif SUN_QUALITY == 2
#define PCSS_NUM_SAMPLES 8
#elif SUN_QUALITY == 3
#define PCSS_NUM_SAMPLES 12
#elif (SUN_QUALITY == 4 || SUN_QUALITY == 5)
#define PCSS_NUM_SAMPLES 16
#endif

#if PCSS_NUM_SAMPLES > 1
float2x2 pcss_rotation(float2 tc)
{
    float2 p = tc * float(SMAP_size);
    float a = frac(52.9829189f * frac(dot(p, float2(0.06711056f, 0.00583715f))) + timers.x * 0.618034f) * 6.2831853f;
    float s, c;
    sincos(a, s, c);
    return float2x2(c, -s, s, c);
}

float pcss_filter(float4 tc, float fRatio)
{
    float2x2 rot = pcss_rotation(tc.xy);

    // The disk's first taps are spread over it: if they agree the pixel is fully lit or fully shadowed
    float s = 0.0;
    [unroll] for (uint i = 0; i < 4; ++i)
        s += s_smap.SampleCmpLevelZero(smp_smap, tc.xy + mul(rot, poissonDisk[i * 7]) * fRatio, tc.z).x;

    [branch] if (s < 0.001f || s > 3.999f)
        return s * 0.25f;

    [unroll] for (uint j = 0; j < PCSS_NUM_SAMPLES; ++j)
    {
        if (j % 7 == 0 && j / 7 < 4)
            continue; // already taken
        s += s_smap.SampleCmpLevelZero(smp_smap, tc.xy + mul(rot, poissonDisk[j]) * fRatio, tc.z).x;
    }

    uint taken = PCSS_NUM_SAMPLES;
    [unroll] for (uint k = 0; k < 4; ++k)
        taken += (k * 7 >= PCSS_NUM_SAMPLES) ? 1 : 0;
    return s / float(taken);
}
#endif

float shadow_pcss(float4 tc)
{
    tc.xyz /= tc.w;

#if !defined(SUN_QUALITY)
    float fRatio = max(PCSS_PIXEL_MIN, 0.5f * float(PCSS_PIXEL)) / float(SMAP_size);
    return s_smap.SampleCmpLevelZero(smp_smap, tc.xy + poissonDisk[0] * fRatio, tc.z).x;

#elif SUN_QUALITY > 3 // Blocker search ( Penumbra ) and filter

    int3 uv = int3(tc.xy * float(SMAP_size), 0);
    float zBlock = tc.z - 0.0001;
    float avgBlockerDepth = 0.0;
    float blockerCount = 0.0;

    [unroll] for (int row = -3; row <= 3; row += 2)
    {
        [unroll] for (int col = -3; col <= 3; col += 2)
        {
            float shadowMapDepth = s_smap.Load(uv, int2(col, row)).x;
            float b1 = (shadowMapDepth < zBlock) ? 1.0 : 0.0;
            blockerCount += b1;
            avgBlockerDepth += shadowMapDepth * b1;
        }
    }

    if (blockerCount < 1)
        return 1.0;

    avgBlockerDepth /= blockerCount;
    float fRatio = saturate(((tc.z - avgBlockerDepth) * PCSS_SUN_WIDTH) / avgBlockerDepth);
    fRatio *= fRatio;
    fRatio = max(PCSS_PIXEL_MIN, fRatio * float(PCSS_PIXEL)) / float(SMAP_size);

    return pcss_filter(tc, fRatio);

#else // No blocker search ( Penumbra ), just filter

    float fRatio = max(PCSS_PIXEL_MIN, 0.5f * float(PCSS_PIXEL)) / float(SMAP_size);
    return pcss_filter(tc, fRatio);

#endif
}

float shadow(float4 tc) { return shadow_pcss(tc); }

uniform float3x4 m_sunmask; // ortho-projection
#ifdef USE_SUNMASK
float4 sun_shafts_intensity;

float sunmask(float4 P)
{
    float2 tc = mul(m_sunmask, P);
    float sunmask = s_lmap.SampleLevel(smp_linear, tc, 0).w;
    float sunmask_correction;

    const float intensity = 0.6, ss_bebuff = 10;

    sunmask = sunmask * intensity + (1.0 - intensity);
    sunmask_correction = saturate(sun_shafts_intensity.x * ss_bebuff);

    sunmask = lerp(sunmask, 1.0h, sunmask_correction);
    return sunmask;
}
#else
float sunmask(float4 P) { return 1.0; } //
#endif

uniform float4x4 m_shadow;

#endif