#ifndef MBLUR_H
#define MBLUR_H

#ifndef USE_MBLUR
float3 mblur(float2 UV, float3 pos, float3 c_original) { return c_original; }
#else

#include "common.h"
#include "slb_motion_blur.h"

////////////////////////////////////////
float4x4 m_current; // Current projection matrix
float4x4 m_previous; // Previous projection matrix
float2 m_blur;
////////////////////////////////////////

#ifdef SSFX_MBLUR_OBJECTS
// NLC 2026-10-13: per-object motion blur. The OGSR velocity buffer ( current - previous, NDC, unjittered, incl. skinned
// meshes ) has the same units as the camera-only velocity below, so moving NPCs, mutants and vehicles blur too.
Texture2D<float2> s_mblur_velocity; // $user$velocity
#endif

float3 mblur(float2 UV, float3 pos, float3 img)
{
    // Fix sky ghosting caused by infinite depth value (KD)
    pos.z = is_sky(pos.z) > 0.5 ? 10000.0 : pos.z;

    // Sample position buffer
    float4 pos4 = float4(pos, 1.0);

    // Get current texture coordinates
    float4 p_current = mul(m_current, pos4);
    float2 current_tc = p_current.xy /= p_current.w;

    // Get previous texture coordinates
    float4 p_previous = mul(m_previous, pos4);
    float2 previous_tc = p_previous.xy / p_previous.w;

    // Get velocity (multiplied with motion blur intensity)
#ifdef SSFX_MBLUR_OBJECTS
    float2 p_velocity = s_mblur_velocity.SampleLevel(smp_nofilter, UV, 0) * m_blur;
#else
    float2 p_velocity = (current_tc - previous_tc) * m_blur;
#endif

    return SLB_MBlur(UV, pos, img, p_velocity, 0);
}
#endif
#endif