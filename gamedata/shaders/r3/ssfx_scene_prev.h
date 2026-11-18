/**
 * @ Description: Reflection source with the forward pass in it ( docs/REFLECTIONS_AO_DESIGN.md 3 )
 * @ NLC: 2026-11-16 ( nlc-3.589.209 ) - SSR and the water reflection prepass read $user$generic_temp, a copy of the
 *        scene taken before the forward pass: anomaly haze, particles, fire and light beams were never reflected.
 *        The engine keeps last frame's finished scene ( $user$ssfx_scene_prev, before bloom ); a hit is looked up
 *        there through its motion vector and falls back to the current copy when the point was hidden, off screen
 *        or behind the weapon last frame. Needs ssfx_temporal.h and s_position.
 */

#ifndef SSFX_SCENE_PREV_H
#define SSFX_SCENE_PREV_H

Texture2D s_ssfx_scene_prev; // $user$ssfx_scene_prev
uniform float4 ssfx_scene_prev_k; // x: strength ( ssfx_ssr_forward ), 0 = no valid previous frame

// current: the colour of the hit in this frame's pre-forward copy
float3 ssfx_reflect_image(float2 hit_tc, float3 current)
{
    [branch] if (ssfx_scene_prev_k.x <= 0.0f)
        return current;

    float2 prev_tc = hit_tc - ssfx_motion_uv(hit_tc);
    float hz = s_position.SampleLevel(smp_nofilter, hit_tc, 0).z;

    // Same surface last frame: within ~2 % of depth, gone at ~7 %. .209d ( review ): against the depth the point had
    // in last frame's view ( m_previous ), not its depth now - walking forward alone changed that ratio for near hits
    float3 hit_p = float3(hz * (hit_tc * screen_res.xy * pos_decompression_params.zw - pos_decompression_params.xy), hz);
    float hz_prev = mul(m_previous, float4(hit_p, 1.0f)).w;
    float ok = 1.0f - ssfx_offscreen(prev_tc);
    ok *= 1.0f - saturate(ssfx_disocclusion(hz_prev, prev_tc) * 20.0f - 0.4f);
    ok *= 1.0f - ssfx_hud_mask(prev_tc);

    // max / min: a NaN or a runaway value would live on through the feedback ( reflections are in the next copy )
    float3 prev = min(max(s_ssfx_scene_prev.SampleLevel(smp_rtlinear, saturate(prev_tc), 0).rgb, 0.0f), 64.0f);

    return lerp(current, prev, ok * saturate(ssfx_scene_prev_k.x));
}

#endif // SSFX_SCENE_PREV_H
