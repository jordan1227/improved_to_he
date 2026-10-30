// NLC 2026-10-27: low psy health vision. Replaces the full-screen snd_shock.ppe double vision.
// Stage A/B: an edge tunnel (ripple, inward-sampled ghost, defocus, tint) with a clear centre.
// Stage C and episodes: full-screen double vision, tunnel snaps and blinks. All values come from
// sivol_psy_vision.script (the nine vectors are listed in system.ltx [shader_params_export]).
// Stage A (2026-10-27 pass 5): the distant world breathes and swims by depth, edges fringe; a heartbeat pulse,
// an ADS focus wobble, a vertigo snap and a near-total blink with a grey recovery.
// Called from combine_2_naa.ps after motion blur, before bloom; all-zero inputs return the pixel untouched.

#ifndef NLC_PSY_VISION_H
#define NLC_PSY_VISION_H

uniform float4 m_nlc_psy_vision; // x tunnel strength 0..1, y shock envelope 0..1, z HUD warp share, w ghost weight
uniform float4 m_nlc_psy_vision_2; // x tunnel radius (script: stage, snap), y snap lean side (+-1), z feather, w breathe
uniform float4 m_nlc_psy_vision_3; // x ripple amplitude, y flow speed, z blur radius, w edge darkening
uniform float4 m_nlc_psy_vision_4; // xyz edge tint, w desaturation
uniform float4 m_nlc_psy_vision_5; // x ghost offset, y wave progress 0..1 (reaches the tunnel at 1), z full-screen double vision 0..1, w blink 0..1
uniform float4 m_nlc_psy_vision_6; // xyz cold shock colour, w its strength at the pulse peak
uniform float4 m_nlc_psy_vision_7; // x double vision offset, y its spin speed, z edge gain, w cold share in the centre
uniform float4 m_nlc_psy_vision_8; // x snap (vertigo) 0..1, y ADS focus wobble 0..1, z blink recovery 0..1, w heartbeat pulse 0..1
uniform float4 m_nlc_psy_vision_9; // x distant-world swim 0..2, y edge colour fringe 0..1, z near / w far distance (m)
uniform float4 m_nlc_psy_vision_10; // x colour loss 0..1, y red hue tolerance, z grey contrast, w red saturation
uniform float4 m_nlc_psy_vision_11; // x side of the psy source (height units, + = right): the tunnel centre shifts away from it

#define NLC_PSY_ELLIPSE 0.8f // < 1 widens the tunnel horizontally
#define NLC_PSY_HUD_NEAR 1.2f // view distance (m): closer pixels are treated as hands/weapon
#define NLC_PSY_HUD_FAR 1.8f
#define NLC_PSY_SHOCK_TIGHTEN 0.12f // tunnel radius lost at shock 1
#define NLC_PSY_EDGE_FADE 0.015f // the warp fades out this close to the frame edge
#define NLC_PSY_LUM float3(0.2126f, 0.7152f, 0.0722f)
#define NLC_PSY_WORLD_AMP 0.006f // distant-world swim at strength 1 (screen-height units)
#define NLC_PSY_FOCUS_ZOOM 0.035f // centre magnification swing while aiming at focus 1
#define NLC_PSY_SNAP_ZOOM 0.13f // centre zoom-in at snap 1
#define NLC_PSY_SNAP_ROLL 0.035f // image roll at snap 1 (radians)
#define NLC_PSY_SNAP_SMEAR 0.03f // radial smear toward the centre at snap 1
#define NLC_PSY_FRINGE 0.004f // edge colour split at fringe 1

// Mirror small overshoots back into the frame. Clamping repeated the edge row, which showed as streaks.
float2 nlc_psy_fold(float2 uv) { return 1.f - abs(1.f - abs(uv)); }

// One image tap with the full-screen double vision: a second copy along a slowly turning offset, with a
// slight colour split.
float3 nlc_psy_tap(float2 uv, float2 doff, float dw)
{
    float3 c = s_image.Sample(smp_rtlinear, nlc_psy_fold(uv)).rgb;
    [branch] if (dw > 0.001f)
    {
        float3 g;
        g.r = s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + doff * 1.08f)).r;
        g.g = s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + doff)).g;
        g.b = s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + doff * 0.92f)).b;
        c = lerp(c, g, dw);
    }
    return c;
}

float3 nlc_psy_vision_apply(float2 tc, float3 P, float3 img)
{
    float s = m_nlc_psy_vision.x;
    float shock = m_nlc_psy_vision.y;

    float t = timers.x;
    float aspect = screen_res.x * screen_res.w;
    float2 to_uv = float2(1.f / (aspect * NLC_PSY_ELLIPSE), 1.f);

    float2 d = tc - 0.5f;
    d.x *= aspect * NLC_PSY_ELLIPSE;
    float r = length(d);
    float2 dir = r > 0.0001f ? d / r : float2(0.f, 0.f);
    float ang = atan2(d.y, d.x);

    float snap = m_nlc_psy_vision_8.x;
    float focus = m_nlc_psy_vision_8.y;
    float beat = m_nlc_psy_vision_8.w;
    float world = m_nlc_psy_vision_9.x;

    // Base image position: distant-world swim, ADS focus wobble and the snap vertigo move it before any tap.
    float2 buv = tc;
    [branch] if (world > 0.001f)
    {
        // near pixels (hands, weapon, anything within world_near) stay put; the sky counts as far
        float depth = (P.z > SKY_EPS) ? length(P) : m_nlc_psy_vision_9.w;
        float wk = world * smoothstep(m_nlc_psy_vision_9.z, m_nlc_psy_vision_9.w, depth) * (1.f + beat * 0.5f);
        float2 swim = float2(sin(tc.y * 6.f + t * 0.9f + sin(tc.x * 4.f + t * 0.5f)), cos(tc.x * 5.f - t * 0.7f + sin(tc.y * 3.f - t * 0.4f)));
        buv += swim * NLC_PSY_WORLD_AMP * wk * float2(1.f / aspect, 1.f) + (tc - 0.5f) * sin(t * 0.6f) * 0.004f * wk;
    }
    float centre = 1.f - smoothstep(0.f, 0.35f, r);
    [branch] if (focus > 0.001f || snap > 0.001f)
    {
        // focus: the clear centre swims in and out while aiming; snap: it lurches inward and rolls
        float zoom = focus * NLC_PSY_FOCUS_ZOOM * (0.6f + 0.4f * sin(t * 2.3f) + beat * 0.5f) * centre + snap * NLC_PSY_SNAP_ZOOM;
        // the snap leans to a random side (script, _2.y = +-1) with a small sway on top
        float roll = focus * 0.02f * sin(t * 1.1f) * centre + snap * NLC_PSY_SNAP_ROLL * (m_nlc_psy_vision_2.y + 0.3f * sin(t * 3.f + 1.f));
        float2 q = (buv - 0.5f) * float2(aspect, 1.f);
        float cs = cos(roll), sn = sin(roll);
        q = float2(q.x * cs - q.y * sn, q.x * sn + q.y * cs) * (1.f - zoom);
        buv = 0.5f + q * float2(1.f / aspect, 1.f);
    }
    bool moved = any(buv != tc);

    // Full-screen double vision (stage C, focus-loss episodes, bursts).
    float D = m_nlc_psy_vision_5.z;
    float dang = t * m_nlc_psy_vision_7.y + sin(t * 0.37f) * 1.2f;
    float2 doff = float2(cos(dang), sin(dang)) * m_nlc_psy_vision_7.x * D * float2(1.f / aspect, 1.f);
    float dw = 0.45f * saturate(D * 1.5f);

    // Tunnel: the script sets the radius (strength, stage C, snaps); it breathes and tightens on a shock.
    float breathe = sin(t * 1.7f) * 0.6f + sin(t * 0.63f + 1.3f) * 0.4f + beat;
    float radius = max(m_nlc_psy_vision_2.x + breathe * m_nlc_psy_vision_2.w * s - shock * NLC_PSY_SHOCK_TIGHTEN, 0.05f);
    float feather = max(m_nlc_psy_vision_2.z, 0.02f);
    // A known psy source shifts the centre away from it, so that side closes in first.
    float lean = m_nlc_psy_vision_11.x;
    float rt = length(d + float2(lean, 0.f));
    float mask = smoothstep(radius, radius + feather, rt);

    // Psy wave: runs from the frame edge in to the tunnel edge, then fades out; the rest of the pulse stays behind it.
    float r_corner = length(float2(0.5f * aspect * NLC_PSY_ELLIPSE, 0.5f)) + abs(lean);
    float ring_r = lerp(r_corner, radius, m_nlc_psy_vision_5.y);
    float ring = shock * (1.f - smoothstep(0.85f, 1.f, m_nlc_psy_vision_5.y)) * exp(-(rt - ring_r) * (rt - ring_r) / 0.004f);

    float amount = saturate(mask * saturate(s * m_nlc_psy_vision_7.z + shock * 0.8f) + ring);

    float3 outc = img;
    [branch] if (dw > 0.001f || moved)
        outc = nlc_psy_tap(buv, doff, dw);

    // Stage A edge fringe: red and blue drift apart toward the frame edge.
    float fringe = m_nlc_psy_vision_9.y;
    [branch] if (fringe > 0.001f)
    {
        float fr = fringe * NLC_PSY_FRINGE * smoothstep(0.35f, 0.9f, r) * (1.f + beat * 0.5f);
        float2 fo = dir * fr * float2(1.f / aspect, 1.f);
        outc.r = s_image.Sample(smp_rtlinear, nlc_psy_fold(buv + fo)).r;
        outc.b = s_image.Sample(smp_rtlinear, nlc_psy_fold(buv - fo)).b;
    }

    [branch] if (amount > 0.001f)
    {
        // Liquid flow: radial waves bent by a slow angular drift plus a tangential swirl. Integer angular
        // frequencies keep the atan2 seam continuous.
        float speed = m_nlc_psy_vision_3.y;
        float w1 = sin(r * 18.f - t * speed * 2.f + sin(ang * 3.f + t * 0.5f) * 1.5f);
        float w2 = sin(ang * 5.f + r * 7.f + t * speed * 1.3f);
        float2 flow = dir * (w1 * 0.6f + ring * 1.5f) + float2(-dir.y, dir.x) * w2 * 0.4f;

        // Hands and weapon warp by the script's share; the sky has no depth and counts as world.
        float hud = (P.z > SKY_EPS) ? 1.f - smoothstep(NLC_PSY_HUD_NEAR, NLC_PSY_HUD_FAR, length(P)) : 0.f;
        float warp_k = lerp(1.f, m_nlc_psy_vision.z, hud);

        float edge = smoothstep(0.f, NLC_PSY_EDGE_FADE, min(min(tc.x, 1.f - tc.x), min(tc.y, 1.f - tc.y)));
        float2 off = flow * m_nlc_psy_vision_3.x * amount * warp_k * edge * to_uv * (1.f + beat * 0.4f);

        // Ghost: sampled toward the centre, so the copy appears pushed outward and never reads past the frame.
        float ghost_amp = m_nlc_psy_vision_5.x * (0.6f + 0.4f * sin(t * 0.9f));
        float2 ghost = dir * ghost_amp * amount * warp_k * to_uv;

        float2 uv = buv + off;
        float3 c0 = nlc_psy_tap(uv, doff, dw);

        // Snap: the edges smear toward the centre.
        [branch] if (snap > 0.001f)
        {
            float2 sm = dir * snap * NLC_PSY_SNAP_SMEAR * amount * to_uv;
            c0 = (c0 + s_image.Sample(smp_rtlinear, nlc_psy_fold(uv - sm)).rgb + s_image.Sample(smp_rtlinear, nlc_psy_fold(uv - sm * 2.f)).rgb) / 3.f;
        }

        // Cheap 4-tap defocus around the warped point (the blur_* targets are built before NV/gas mask).
        float br = m_nlc_psy_vision_3.z * amount;
        [branch] if (br > 0.0001f)
        {
            float2 bo = float2(br / aspect, br); // br is in screen-height uv units
            float3 b = s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + float2(bo.x, bo.y))).rgb;
            b += s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + float2(-bo.x, bo.y))).rgb;
            b += s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + float2(bo.x, -bo.y))).rgb;
            b += s_image.Sample(smp_rtlinear, nlc_psy_fold(uv + float2(-bo.x, -bo.y))).rgb;
            c0 = lerp(c0, b * 0.25f, saturate(amount * 1.2f));
        }

        float3 cg = nlc_psy_tap(uv - ghost, doff, dw);
        float3 res = lerp(c0, cg, saturate(m_nlc_psy_vision.w + shock * 0.3f) * amount);

        // Edge colour: desaturate toward the tint, darken; the shock pulse chills the edges and the ring.
        float lum = dot(res, NLC_PSY_LUM);
        res = lerp(res, lum * m_nlc_psy_vision_4.xyz, m_nlc_psy_vision_4.w * amount);
        res *= 1.f - m_nlc_psy_vision_3.w * mask * saturate(s);
        float cold = m_nlc_psy_vision_6.w * shock * saturate(mask + ring);
        res = lerp(res, lum * m_nlc_psy_vision_6.xyz, cold);
        res += m_nlc_psy_vision_6.xyz * ring * 0.10f;

        // Fade in from the untouched pixel so the tunnel edge has no seam.
        outc = lerp(outc, res, saturate(amount * 4.f));
    }

    // Cold flash over the whole view at the pulse peak.
    float cold_all = m_nlc_psy_vision_6.w * shock * m_nlc_psy_vision_7.w;
    outc = lerp(outc, dot(outc, NLC_PSY_LUM) * m_nlc_psy_vision_6.xyz, cold_all);

    // Blink recovery: the world comes back grey and dim.
    float rec = m_nlc_psy_vision_8.z;
    outc = lerp(outc, dot(outc, NLC_PSY_LUM).xxx, rec * 0.7f) * (1.f - rec * 0.15f);

    return outc;
}

// Colour loss, applied by combine_2_naa.ps after bloom, flares and colour grading (bright skies and fires would keep
// their colour otherwise): black and white, only red keeps its colour. A colour key (hue near pure red, saturated, not
// near black), so it catches every blood source (decals, hit and anomaly blood, pools) and other deep reds as well.
float3 nlc_psy_colour(float3 img)
{
    float bs = m_nlc_psy_vision_10.x;
    [branch] if (bs > 0.001f)
    {
        float mx = max(img.r, max(img.g, img.b));
        float mn = min(img.r, min(img.g, img.b));
        float sat = (mx - mn) / max(mx, 0.0001f);
        float hue = (img.r >= mx && mx > mn) ? abs((img.g - img.b) / (mx - mn)) / 6.f : 0.5f; // 0 = red, 1/6 = yellow
        float tol = max(m_nlc_psy_vision_10.y, 0.005f);
        float red = (1.f - smoothstep(tol * 0.6f, tol, hue)) * smoothstep(0.25f, 0.5f, sat) * smoothstep(0.01f, 0.04f, mx);
        float g = dot(img, NLC_PSY_LUM);
        float gc = max(0.f, (g - 0.25f) * m_nlc_psy_vision_10.z + 0.25f);
        float3 keep = max(0.f, g + (img - g) * m_nlc_psy_vision_10.w);
        img = lerp(img, lerp(gc.xxx, keep, red), bs);
    }
    return img;
}

// Blink, applied by combine_2_naa.ps after bloom, flares and colour grading: the whole view darkens (the centre keeps
// about a fifth of its light) while pitch black closes in from the edges to about 0.3 from the centre.
float nlc_psy_blink_factor(float2 tc)
{
    float blink = m_nlc_psy_vision_5.w;
    float k = 1.f;
    [branch] if (blink > 0.001f)
    {
        float aspect = screen_res.x * screen_res.w;
        float2 d = tc - 0.5f;
        d.x *= aspect * NLC_PSY_ELLIPSE;
        float r = length(d);
        float edge_black = smoothstep(lerp(0.85f, 0.25f, blink), lerp(1.0f, 0.45f, blink), r);
        k = (1.f - blink * 0.78f) * (1.f - edge_black * saturate(blink * 1.5f));
    }
    return k;
}

// Uniform gate: with every input at zero this costs one test.
float3 nlc_psy_vision(float2 tc, float3 P, float3 img)
{
    float3 outc = img;
    [branch] if (m_nlc_psy_vision.x + m_nlc_psy_vision.y + m_nlc_psy_vision_5.z + m_nlc_psy_vision_5.w + m_nlc_psy_vision_8.x +
                 m_nlc_psy_vision_8.y + m_nlc_psy_vision_8.z + m_nlc_psy_vision_9.x + m_nlc_psy_vision_9.y > 0.001f)
        outc = nlc_psy_vision_apply(tc, P, img);
    return outc;
}

#endif
