/**
 * @ Description: Grass colour variation (natural dry/lush patches)
 * @ Based on: SCREEN SPACE SHADERS - UPDATE 23 flora colour variation ( https://www.moddb.com/members/ascii1457 )
 * @ NLC: 2026-10-12 - smooth warped-sine patches instead of grid cells, dry/lush multiplicative tint
 *        instead of a hue wheel, weighted by how green the texel is. Settings: settings_screenspace_FLORA.h
 */

#ifndef SSFX_FLORA_VARIATION_H
#define SSFX_FLORA_VARIATION_H

#include "settings_screenspace_FLORA.h"

#ifdef G_FLORA_GRASS_VARIATION

uniform float4 ssfx_flora_variation; // x: strength multiplier ( console ssfx_flora_variation, 0 = off )

// Hash without sine ( Dave Hoskins ), stable for world coordinates in the thousands
float nlc_flora_hash(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * 0.1031f);
    p3 += dot(p3, p3.yzx + 33.33f);
    return frac((p3.x + p3.y) * p3.z);
}

// VS: per-instance variation from the instance origin ( world XZ )
// x = patch value [ -1 = lush ~ +1 = dry ], y = per-instance brightness jitter [ -1 ~ 1 ]
// Patches: product of two domain-warped sines with incommensurate frequencies. Smooth, no grid
// edges and no visible repetition, at a fraction of the cost of hashed value noise ( vertex budget ).
float2 ssfx_flora_variation_vs(float2 origin_xz)
{
    float2 p = origin_xz * (6.2831853f / G_FLORA_GRASS_PATCH_SIZE);

    float a = sin(p.x * 0.731f + 1.7f * sin(p.y * 0.537f));
    float b = sin(p.y * 0.913f + 1.3f * sin(p.x * 0.419f + 0.5f));
    float patch = saturate(abs(a * b) * 1.6f) * sign(a * b);

    float jitter = nlc_flora_hash(origin_xz * 7.13f) * 2.0f - 1.0f;

    return float2(patch, jitter);
}

// PS: apply to albedo ( gamma space, before G-buffer packing )
float3 ssfx_flora_variation_ps(float3 albedo, float2 v)
{
    // Vegetation weight: green/yellow texels > blue. Grey stones and debris stay ~unchanged
    float plant = saturate((albedo.g - albedo.b) * 6.0f);

    float3 tint = v.x >= 0.0f ? lerp(1.0f, G_FLORA_GRASS_DRY_TINT, v.x) : lerp(1.0f, G_FLORA_GRASS_LUSH_TINT, -v.x);
    tint *= 1.0f + v.y * G_FLORA_GRASS_BRIGHTNESS;

    return saturate(albedo * lerp(1.0f, tint, G_FLORA_GRASS_VARIATION * ssfx_flora_variation.x * plant));
}

#endif // G_FLORA_GRASS_VARIATION

#endif // SSFX_FLORA_VARIATION_H
