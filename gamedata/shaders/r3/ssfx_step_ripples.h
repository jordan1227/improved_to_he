/**
 * @ Description: Footstep ripples in water and puddles
 * @ NLC: 2026-10-13. Rings spread from the feet of the player and of moving NPCs / mutants ( grass bender positions,
 *        ssfx_steps, Blender_Recorder_StandartBinding.cpp ), only while they move. Returns a tangent xy offset for the
 *        surface normal. Strength ssfx_step_ripples ( SSFX_STEP_RIPPLES ).
 */

#ifndef SSFX_STEP_RIPPLES_H
#define SSFX_STEP_RIPPLES_H

uniform float4 ssfx_steps[8]; // xyz: feet, w: movement ( 0 = still )

float2 ssfx_step_ripples(float3 W)
{
    float2 n = 0;

    [unroll] for (int i = 0; i < 8; i++)
    {
        float4 st = ssfx_steps[i];
        float2 d = W.xz - st.xz;
        float dist = length(d);

        // Near the feet, within the height of a body, and only while moving
        float near = saturate(1.0f - dist * 0.5f) * step(abs(W.y - st.y), 1.5f) * st.w;
        if (near <= 0.0f)
            continue;

        float ring = sin(dist * 22.0f - timers.x * 9.0f) * exp(-dist * 2.2f);
        n += (d / max(dist, 0.001f)) * ring * near * 0.25f;
    }

    return n;
}

#endif // SSFX_STEP_RIPPLES_H
