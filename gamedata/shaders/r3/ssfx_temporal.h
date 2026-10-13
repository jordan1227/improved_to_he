/**
 * @ Description: Shared helpers of the NLC SSS 23 temporal passes ( AO, IL, screen-space shadows )
 * @ Based on: SCREEN SPACE SHADERS - UPDATE 22/23 reprojection code ( https://www.moddb.com/members/ascii1457 )
 * @ NLC: 2026-10-12 - one implementation for all passes, OGSR velocity buffer ( NDC, unjittered ),
 *        HUD mask from the depth copy instead of an extra HUD render, history-valid flag.
 */

#ifndef SSFX_TEMPORAL_H
#define SSFX_TEMPORAL_H

Texture2D<float2> s_ssfx_velocity; // $user$velocity: current - previous, NDC ( OGSR get_motion_vector )
Texture2D s_ssfx_zbuffer; // $user$zbuffer: hardware depth after the HUD pass
Texture2D s_prev_pos; // $user$ssfx_prev_pos: previous frame position buffer ( .z = view depth )

float4x4 m_current; // view -> clip, current frame
float4x4 m_previous; // view -> clip, previous frame

// Motion in UV units: previous_uv = uv - ssfx_motion_uv( uv )
float2 ssfx_motion_uv(float2 tc) { return s_ssfx_velocity.SampleLevel(smp_nofilter, tc, 0) * float2(0.5f, -0.5f); }

// The HUD is drawn with a [ 0, 0.02 ] depth range ( CRender::rmNear ). World geometry only gets
// there inside ~0.2 m of the camera.
float ssfx_hud_mask(float2 tc) { return s_ssfx_zbuffer.SampleLevel(smp_nofilter, tc, 0).x < 0.0199f ? 1.0f : 0.0f; }

// Camera-only motion of a view space position ( UV units )
float2 ssfx_camera_motion_uv(float3 P)
{
    float4 p_current = mul(m_current, float4(P, 1.0f));
    float4 p_previous = mul(m_previous, float4(P, 1.0f));
    return (p_current.xy / p_current.w - p_previous.xy / p_previous.w) * float2(0.5f, -0.5f);
}

float ssfx_offscreen(float2 uv) { return (any(uv < 0.0f) || any(uv > 1.0f)) ? 1.0f : 0.0f; }

// Relative depth change against the previous frame ( 0 = same surface )
float ssfx_disocclusion(float z, float2 prev_uv)
{
    float prev_z = s_prev_pos.SampleLevel(smp_nofilter, prev_uv, 0).z;
    return saturate(abs(1.0f - z / max(prev_z, 0.0001f)));
}

// Sine-free hashes ( Dave Hoskins )
float ssfx_hash12(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * 0.1031f);
    p3 += dot(p3, p3.yzx + 33.33f);
    return frac((p3.x + p3.y) * p3.z);
}

float2 ssfx_hash22(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * float3(0.1031f, 0.1030f, 0.0973f));
    p3 += dot(p3, p3.yzx + 33.33f);
    return frac((p3.xx + p3.yz) * p3.zy);
}

float3 ssfx_hash33(float3 p3)
{
    p3 = frac(p3 * float3(0.1031f, 0.1030f, 0.0973f));
    p3 += dot(p3, p3.yxz + 33.33f);
    return frac((p3.xxy + p3.yxx) * p3.zyx);
}

// Sub-pixel noise for the history fetch ( hides the reprojection grid )
float2 ssfx_history_noise(float2 tc, float amount)
{
    return (ssfx_hash22(tc * 100.0f + timers.xx * 100.0f) * 2.0f - 1.0f) * render_res.zw * amount;
}

#endif // SSFX_TEMPORAL_H
