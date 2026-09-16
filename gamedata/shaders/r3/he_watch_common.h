#ifndef HE_WATCH_COMMON_H
#define HE_WATCH_COMMON_H
/*
	[HE-PORT] Shared watch HUD contract for the radiation ring and alarm editor.
	Lua writer: gamedata/scripts/ogsr_shaders_control.script (watch_update).
*/

uniform float4 m_he_watch_page; // x = time/bleeding 0 or radiation/psy 1, y = icon slide 0..1
uniform float4 m_he_watch_ring;   // x = radiation fill 0..1, y = ring available 1/0, z/w reserved
uniform float4 m_he_watch_alarm;  // x = alarm enabled 1/0, y = editor active 1/0, z = saved alarm hour, w = saved alarm minute

float he_watch_arc(float progress, float fraction)
{
	if (fraction <= 0.0) return 0.0;
	if (fraction >= 1.0) return 1.0;
	return step(progress, fraction);
}

float3 he_watch_radiation_color(float fraction)
{
	float f = saturate(fraction);
	float3 yellow = float3(1.0, 1.0, 0.0);
	float3 orange = float3(1.0, 0.5, 0.0);
	float3 red    = float3(1.0, 0.0, 0.0);

	if (f <= 0.5)
		return lerp(yellow, orange, f * 2.0);

	return lerp(orange, red, (f - 0.5) * 2.0);
}

uniform float4 m_he_watch_effects; // x = interference 0..1, y = unavailable 1/0

float he_watch_hash(float2 p)
{
    return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);
}

float2 he_watch_distort_uv(float2 uv)
{
    float strength = saturate(m_he_watch_effects.x);
    if (strength <= 0.0) return uv;
    float frame = floor(timers.x * 24.0);
    float row = floor(uv.y * 32.0);
    float gate = step(1.0 - strength * 0.45, he_watch_hash(float2(frame, row)));
    float shift = (he_watch_hash(float2(row + 19.0, frame)) - 0.5);
    uv.x += shift * 0.12 * strength * gate;
    return saturate(uv); // stay in this icon BEFORE its atlas remapping
}

float he_watch_signal(float2 uv)
{
    if (m_he_watch_effects.y > 0.5) return 0.0;
    float strength = saturate(m_he_watch_effects.x);
    if (strength <= 0.0) return 1.0;
    float frame = floor(timers.x * 24.0);
    // Shared temporal pulse makes separate mesh surfaces fail together.
    float pulse = he_watch_hash(float2(frame, 71.0));
    float flicker = 1.0 - strength * (0.18 + 0.45 * pulse);
    float dropout = step(strength * 0.10, pulse);
    float row = floor(uv.y * 48.0);
    float strip = step(strength * 0.30, he_watch_hash(float2(row, frame + 31.0)));
    return flicker * dropout * lerp(0.12, 1.0, strip);
}

float3 he_watch_apply_signal(float3 color, float2 uv)
{
    // Additive materials: black contributes no light, with no opaque rectangles.
    return color * he_watch_signal(uv);
}
#endif
