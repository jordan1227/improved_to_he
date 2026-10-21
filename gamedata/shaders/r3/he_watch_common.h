#ifndef HE_WATCH_COMMON_H
#define HE_WATCH_COMMON_H
/*
	[HE-PORT] Shared watch HUD contract for the radiation ring and alarm editor.
	Lua writers: gamedata/scripts/ogsr_shaders_control.script (watch_update);
	m_he_watch_alarm_fx only: gamedata/scripts/he_watch_ui.script.
*/

uniform float4 m_he_watch_page; // x = time/bleeding 0 or radiation/psy 1, y = icon slide 0..1
uniform float4 m_he_watch_ring;   // x = radiation fill 0..1, y = ring available 1/0, z/w reserved
uniform float4 m_he_watch_alarm;  // x = alarm enabled 1/0, y = editor active 1/0, z = saved alarm hour, w = saved alarm minute
uniform float4 m_he_watch_alarm_fx; // x = alarm ringing 1/0, y = hold progress 0..1 (he_watch_ui.script), z/w reserved

// NLC page-2 body layout and adrenaline meter (docs/damage_model/DESIGN_recovery_and_body_status.md 16.5, 22).
// Writer: ogsr_shaders_control.script (watch_update).
uniform float4 m_he_watch_injury;    // x = arm level, y = leg level (0 none, 1 light, 2 severe), z = head (1 = concussion running)
uniform float4 m_he_watch_body;      // white health: head, torso, left arm, right arm (total health with body health off)
uniform float4 m_he_watch_body2;     // left leg, right leg, z = mode (0 = page 2 as before, 1 = injuries only, 2 = body health), w = psy damage
uniform float4 m_he_watch_green;     // green health: head, torso, left arm, right arm
uniform float4 m_he_watch_green2;    // left leg, right leg, reserved, reserved
uniform float4 m_he_watch_adrenaline; // x = meter 0..1, y = state (0 ready, 1 rush, 2 crash, 3 cooldown), z = meter shown 1/0
uniform float4 m_he_watch_body_rect; // figure rect in the frame (watch_ui.dds) UV: x, y, w, h

// 0 on page 1 .. 1 on page 2 for the meshes the body figure replaces; 0 without the figure or while editing the alarm
float he_watch_body_hide()
{
	return (m_he_watch_body2.z > 0.5 && m_he_watch_alarm.y < 0.5) ? saturate(m_he_watch_page.y) : 0.0;
}

// Warm amber shared by the bell icon, ringing pulse and hold arc; distinct from the radiation yellow.
static const float3 he_watch_alarm_amber = float3(1.0, 0.88, 0.55);

// ~1 Hz, close to the beeper rate: short bright peak, longer dim tail.
float he_watch_alarm_pulse()
{
	float p = 0.5 + 0.5 * cos(timers.x * 6.28318530718);
	return p * p;
}

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
// NLC debug mesh map (docs/damage_model/DESIGN_recovery_and_body_status.md 17/20): x > 0.5 paints every
// watch mesh in its own colour, brighter with UV x, a black 10x10 UV grid and a white square at UV (0,0).
// Writer: ogsr_shaders_control.watch_debug(on). Off by default. y > 0.5: body figure always shown with its rect outlined.
uniform float4 m_he_watch_debug;
float4 he_watch_debug_map(float2 uv, float3 col)
{
	float2 g = abs(frac(uv * 10.0) - 0.5);
	float3 c = col * lerp(0.35, 1.0, saturate(uv.x));
	if (g.x > 0.44 || g.y > 0.44) c *= 0.4;
	if (uv.x < 0.12 && uv.y < 0.12) c = float3(1.0, 1.0, 1.0);
	return float4(c, 1.0);
}
// NLC page 2 with the body figure (docs/damage_model/DESIGN_recovery_and_body_status.md 12.3, 22, 24, 25).
// The watch pieces are separate meshes (the frame mesh is outline strips with holes), so page 2 is composed
// in one shared space, the clock's own UV, and every piece draws its share: all hands models put the clock at
// u 0..1, v 0..1 (surface 1.83 : 1), the health bar at v 1.159..1.355, the tiredness bar 1.588..1.783, the
// bottom tiles 1.917..2.455 (measured from the OGF vertices); the frame's inner face maps to it affinely.
// Content: the body figure in m_he_watch_body_rect, the radiation number centred over the bottom tiles.
Texture2D s_he_body; // act\\watch\\icon_body, 128x128: R part index (32 head .. 192 right leg), G fill height, B outline, A figure
Texture2D s_he_rad;  // addon glyph atlas: 14 columns, 12 rows (not 14x14)

static const float3 he_watch_page2_bg = float3(16.0, 18.0, 20.0) / 255.0; // the frame's background
static const float4 he_watch_rad_box = float4(0.233, 1.956, 0.46, 0.46);  // radiation number box in clock UV: x, y, w, h

// 0..1 how much of page 2 is shown (also 1 in the placement debug)
float he_watch_page2_k()
{
    return m_he_watch_debug.y > 0.5 ? 1.0 : he_watch_body_hide();
}

// rgb + a (1 inside the figure rect)
float4 he_watch_body_figure(float2 cuv)
{
    float2 r = (cuv - m_he_watch_body_rect.xy) / max(m_he_watch_body_rect.zw, 0.001);
    if (any(r < 0.0) || any(r > 1.0))
        return float4(0, 0, 0, 0);
    float3 col = he_watch_page2_bg;
    float4 m = s_he_body.SampleLevel(smp_nofilter, r, 0);
    int part = int(m.r * 255.0 / 32.0 + 0.5);
    if (m.a > 0.5 && part >= 1 && part <= 6)
    {
        float white[6] = {m_he_watch_body.x, m_he_watch_body.y, m_he_watch_body.z, m_he_watch_body.w, m_he_watch_body2.x, m_he_watch_body2.y};
        float green[6] = {m_he_watch_green.x, m_he_watch_green.y, m_he_watch_green.z, m_he_watch_green.w, m_he_watch_green2.x, m_he_watch_green2.y};
        float w = saturate(white[part - 1]);
        float g = saturate(green[part - 1]);
        float3 wc = lerp(float3(1.0, 0.18, 0.08), float3(1.0, 0.72, 0.25), saturate((w - 0.3) / 0.3));
        wc = lerp(wc, float3(0.72, 0.80, 0.72), saturate((w - 0.6) / 0.3));
        float h = m.g; // 0 at the part's bottom .. 1 at its top
        if (h <= w)
            col = wc;
        else if (h <= w + g)
            col = float3(0.25, 0.85, 0.35);
        else
            col = he_watch_page2_bg * 1.8;

        float inj = part == 1 ? m_he_watch_injury.z : (part <= 2 ? 0.0 : (part <= 4 ? m_he_watch_injury.x : m_he_watch_injury.y));
        if (inj > 1.5)
            col = lerp(col, float3(1.0, 0.22, 0.12), 0.65 * lerp(0.3, 1.0, he_watch_alarm_pulse()));
        else if (inj > 0.5)
            col = lerp(col, he_watch_alarm_amber, 0.45);
        if (part == 1)
            col = lerp(col, float3(0.55, 0.35, 1.0), 0.7 * saturate(m_he_watch_body2.w));
    }
    if (m.b > 0.5)
        col = float3(0.55, 0.62, 0.55);
    if (m_he_watch_debug.y > 0.5 && (r.x < 0.01 || r.y < 0.008 || r.x > 0.99 || r.y > 0.992))
        col = float3(1.0, 0.0, 1.0);
    return float4(col, 1.0);
}

float4 he_watch_rad_digit(float2 uv, int digit)
{
    if (any(uv < 0.0) || any(uv > 1.0) || digit == 11)
        return float4(0, 0, 0, 0);
    if (digit == 14) // unavailable measurement: a dash, never a false zero
    {
        float mask = step(0.25, uv.x) * step(uv.x, 0.75) * step(0.45, uv.y) * step(uv.y, 0.55);
        return float4(float3(0.55, 0.62, 0.55) * mask, mask);
    }
    uv = clamp(uv, 0.5 / 128.0, 1.0 - 0.5 / 128.0);
    return s_he_rad.SampleLevel(smp_base, (uv + float2(digit, 0)) / float2(14.0, 12.0), 0);
}

// the clock's radiation readout (model_clock.ps), drawn in he_watch_rad_box; rgb + a (glyph coverage)
float4 he_watch_rad_number(float2 cuv)
{
    float2 uv = (cuv - he_watch_rad_box.xy) / he_watch_rad_box.zw;
    if (any(uv < 0.0) || any(uv > 1.0))
        return float4(0, 0, 0, 0);
    int value = int(floor(saturate(m_he_watch_ring.x) * 1000.0 + 0.5));
    bool available = m_he_watch_ring.y > 0.5;
    int d0 = available ? (value >= 1000 ? 1 : 11) : 14;
    int d1 = available ? (value / 100) % 10 : 14;
    int d2 = available ? (value / 10) % 10 : 14;
    int d3 = available ? value % 10 : 14;
    float2 cell = float2(0.21, 1.0);
    float4 r = float4(0, 0, 0, 0);
    r += he_watch_rad_digit((uv - float2(0.00, 0)) / cell, d0);
    r += he_watch_rad_digit((uv - float2(0.17, 0)) / cell, d1);
    r += he_watch_rad_digit((uv - float2(0.265, 0)) / cell, 13); // decimal point
    r += he_watch_rad_digit((uv - float2(0.43, 0)) / cell, d2);
    r += he_watch_rad_digit((uv - float2(0.60, 0)) / cell, d3);
    r += he_watch_rad_digit((uv - float2(0.78, 0)) / cell, 12); // Cyrillic R glyph
    return float4(r.rgb, saturate(r.a));
}

// page-2 colour of a pixel at clock UV cuv. keep: the piece keeps its own picture where page 2 has no
// content (frame artwork); otherwise it shows the background there (hidden icons and bars).
float3 he_watch_page2(float3 base, float2 cuv, bool keep)
{
    float k = he_watch_page2_k();
    if (k <= 0.0)
        return base;
    float3 col = keep ? base : he_watch_page2_bg;
    float4 fig = he_watch_body_figure(cuv);
    if (fig.a > 0.5)
        col = fig.rgb;
    float4 num = he_watch_rad_number(cuv);
    col = lerp(col, num.rgb, num.a);
    return lerp(base, col, k);
}

// a piece with no page-2 content at all (heart, sleep icon): the background on page 2
float3 he_watch_page2_blank(float3 base)
{
    return lerp(base, he_watch_page2_bg, he_watch_body_hide());
}
#endif
