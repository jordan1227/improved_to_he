// NLC 2026-11-12: Flytrap stasis anomaly, the view from inside (docs/ANOMALY_DESIGN.md 3.4, 11).
// A slight lens bulge that grows toward the frame edge and a radial colour fringe; the centre stays clean.
// Values come from sivol_flytrap.script (m_nlc_flytrap is listed in system.ltx [shader_params_export]).
// Called from combine_2_naa.ps around nlc_psy_vision: the lens moves the coordinate the psy effect samples from,
// the fringe is added on top of its result, so the two stack. NVD and the gas mask run in later passes.

#ifndef NLC_FLYTRAP_H
#define NLC_FLYTRAP_H

uniform float4 m_nlc_flytrap; // x strength 0..1 (depth in the zone), y fringe (uv at the frame edge), z lens bulge, w breathing 0..1

float nlc_flytrap_on() { return m_nlc_flytrap.x; }

// The coordinate to read the image from: pulled toward the centre by the squared distance, so the edges magnify
// and nothing is read from outside the frame.
float2 nlc_flytrap_uv(float2 tc)
{
    float s = m_nlc_flytrap.x;
    float aspect = screen_res.x * screen_res.w;
    float2 d = tc - 0.5f;
    float2 da = d * float2(aspect, 1.f);
    float r2 = dot(da, da);
    float breathe = 1.f + m_nlc_flytrap.w * 0.25f * sin(timers.x * 0.7f);
    return 0.5f + d * (1.f - m_nlc_flytrap.z * s * breathe * r2);
}

// Colour fringe as a difference to add: red read a little inward, blue a little outward, nothing in the centre.
float3 nlc_flytrap_fringe(float2 uv)
{
    float s = m_nlc_flytrap.x;
    float aspect = screen_res.x * screen_res.w;
    float2 d = uv - 0.5f;
    float r = length(d * float2(aspect, 1.f));
    float2 off = d * (m_nlc_flytrap.y * s * smoothstep(0.12f, 0.75f, r) * 2.f);
    float3 base = s_image.Sample(smp_rtlinear, uv).rgb;
    float2 uo = 1.f - abs(1.f - abs(uv + off)); // mirrored back into the frame
    float red = s_image.Sample(smp_rtlinear, uv - off).r;
    float blue = s_image.Sample(smp_rtlinear, uo).b;
    return float3(red - base.r, 0.f, blue - base.b);
}

#endif
