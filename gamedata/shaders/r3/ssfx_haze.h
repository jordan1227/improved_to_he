/**
 * @ Description: Anomaly haze ( docs/ANOMALY_HAZE_DESIGN.md ): one analytic volume of local haze, integrated over the
 *                part of the view ray inside it and in front of the scene
 * @ NLC: 2026-11-14. Drawn once per volume by phase_ssfx_haze ( r4_rendertarget_phase_ssfx.cpp ), scissored to the
 *        volume, back to front, blend one / srcalpha: dst = in-scattered light + dst x transmittance. Colours are in the
 *        same ( gamma ) space the combine fog uses. Per-volume constants are set by the engine for each draw.
 * @ NLC: .203 - softer zone light window, debug view 4 shows the light term alone.
 * @ NLC: 2026-11-17 vapour pass ( shader only, ANOMALY_HAZE_DESIGN.md 20 ): ground-shaped volumes read as vapour
 *        boiling off a pool - a thin dense layer on the ground, tall swaying wisps from the second noise octave that
 *        grow with height, a slow density swell per zone. The noise now moves along noise_drift ( was against it ).
 */

#ifndef SSFX_HAZE_H
#define SSFX_HAZE_H

#define NLC_HAZE_VAPOUR 1 // 0 = the .203 look ( one layer, isotropic noise ); restart the game after a change
#define G_HAZE_WISP_STRETCH 0.4f // second octave, frequency along Y x this: strands ~2.5x taller than wide
#define G_HAZE_SWAY 0.3f // sideways bend of the strands with height, noise units of the second octave
#define G_HAZE_POOL_H 0.22f // m, e-fold height of the dense layer on the ground
#define G_HAZE_POOL_K 1.5f // pool layer density, x the core density
#define G_HAZE_BREATH 0.12f // slow density swell ( 4.5 s ), each zone on its own phase

uniform float4 haze_p0; // xyz: centre ( world ), w: radius
uniform float4 haze_p1; // x: height ( ground shape ), y: density per metre ( x fade x global ), z: radial falloff, w: noise amount
uniform float4 haze_p2; // rgb: ambient light x albedo, w: shape ( 0 sphere, 1 ground )
uniform float4 haze_p3; // rgb: sun light x albedo x sun visibility, w: phase g
uniform float4 haze_p4; // x: noise scale ( 1/m ), yzw: noise drift ( m/s )
uniform float4 haze_p5; // rgb: emissive, w: samples
uniform float4 haze_p6; // xyz: zone light position, w: light range ( 0 = no light )
uniform float4 haze_p7; // rgb: zone light x albedo x scatter, w: debug view
uniform float4 haze_p8; // xyz: direction towards the sun ( world )
uniform float4 haze_p9; // x: noise time ( s ), y: frame index ( jitter ), z: split view

float ssfx_haze_hash(float3 p)
{
    p = frac(p * 0.1031f);
    p += dot(p, p.yzx + 33.33f);
    return frac((p.x + p.y) * p.z);
}

float ssfx_haze_noise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    f = f * f * (3.0f - 2.0f * f);

    return lerp(lerp(lerp(ssfx_haze_hash(i), ssfx_haze_hash(i + float3(1, 0, 0)), f.x),
                     lerp(ssfx_haze_hash(i + float3(0, 1, 0)), ssfx_haze_hash(i + float3(1, 1, 0)), f.x), f.y),
                lerp(lerp(ssfx_haze_hash(i + float3(0, 0, 1)), ssfx_haze_hash(i + float3(1, 0, 1)), f.x),
                     lerp(ssfx_haze_hash(i + float3(0, 1, 1)), ssfx_haze_hash(i + float3(1, 1, 1)), f.x), f.y), f.z);
}

// Henyey-Greenstein, 1 = isotropic. c: cosine between the light's travel direction and the direction to the eye
float ssfx_haze_phase(float c, float g)
{
    float g2 = g * g;
    return (1.0f - g2) / pow(max(1.0f + g2 - 2.0f * g * c, 0.0001f), 1.5f);
}

float3 ssfx_haze_heat(float v)
{
    v = saturate(v);
    return saturate(float3(v * 3.0f - 1.0f, v * 3.0f, 3.0f - v * 3.0f) * float3(1.0f, 1.0f - v, 0.6f));
}

// eye, dir: view ray ( world, dir normalized ); scene: distance to the scene along the ray; pix: pixel position
// Returns rgb = in-scattered light, a = transmittance
float4 ssfx_haze_eval(float3 eye, float3 dir, float scene, float2 pix)
{
    float debug = haze_p7.w;

    // Debug view 1: the scissor rectangle of every volume ( draw cost )
    if (debug > 0.5f && debug < 1.5f)
        return float4(0.04f, 0.0f, 0.05f, 1.0f);

    // Exact segment of the ray inside the bounding sphere, cut by the scene
    float3 c = haze_p0.xyz;
    float R = haze_p0.w;
    float3 oc = eye - c;
    float b = dot(oc, dir);
    float disc = b * b - (dot(oc, oc) - R * R);
    if (disc <= 0.0f)
    {
        discard;
        return float4(0.0f, 0.0f, 0.0f, 1.0f); // never reached ( X4000 )
    }
    float sq = sqrt(disc);
    float t0 = max(-b - sq, 0.0f);
    float t1 = min(-b + sq, scene);
    if (t1 <= t0)
    {
        discard;
        return float4(0.0f, 0.0f, 0.0f, 1.0f); // never reached ( X4000 )
    }

    bool flat_view = debug > 2.5f && debug < 3.5f;
    bool light_only = debug > 3.5f;
    float g = haze_p3.w;

    int n = (int)clamp(haze_p5.w, 2.0f, 16.0f);
    float dt = (t1 - t0) / n;
    // interleaved gradient noise, rotated per frame ( resolved by TAA / DLSS )
    float jitter = frac(52.9829189f * frac(dot(pix, float2(0.06711056f, 0.00583715f))) + haze_p9.y * 0.6180339f);

    float3 Li_base = light_only ? float3(0.0f, 0.0f, 0.0f) : haze_p2.rgb + haze_p5.rgb + haze_p3.rgb * ssfx_haze_phase(dot(dir, haze_p8.xyz), g);
    if (flat_view)
        Li_base = float3(0.25f, 0.9f, 0.3f);

    float invR2 = 1.0f / (R * R);
    float T = 1.0f;
    float3 L = float3(0.0f, 0.0f, 0.0f);

    bool ground = haze_p2.w > 0.5f;
    float core = haze_p1.y;
#if NLC_HAZE_VAPOUR
    // slow swell; the angular speeds here and in the sway are whole cycles per 3600 s, the noise time wraps there
    if (ground && !flat_view)
        core *= 1.0f + G_HAZE_BREATH * sin(haze_p9.x * 1.396263f + dot(c.xz, float2(1.7f, 2.3f)));
#endif

    [loop] for (int i = 0; i < n; i++)
    {
        float t = t0 + (float(i) + jitter) * dt;
        float3 x = eye + dir * t;
        float3 q = x - c;

        float sigma = core * pow(saturate(1.0f - dot(q, q) * invR2), haze_p1.z);
        float hk = 0.0f; // 0 on the ground .. 1 at the vapour's e-fold height
        if (ground)
        {
            float h = max(q.y, 0.0f);
#if NLC_HAZE_VAPOUR
            hk = saturate(h / haze_p1.x);
            sigma *= exp(-h / haze_p1.x) + G_HAZE_POOL_K * exp(-h / G_HAZE_POOL_H);
#else
            sigma *= exp(-h / haze_p1.x);
#endif
        }

        [branch] if (!flat_view && haze_p1.w > 0.0f)
        {
#if NLC_HAZE_VAPOUR
            float3 np = (x - haze_p4.yzw * haze_p9.x) * haze_p4.x; // the pattern moves along noise_drift
            float3 np2 = np * 2.3f + 7.1f;
            float w2 = 0.35f, lo = 0.2f, hi = 0.8f, amount = haze_p1.w;
            if (ground)
            {
                // strands: the fine octave stretched upwards, bent sideways more the higher it is, and stronger
                // and crisper with height; the pool below stays smooth
                np2.y *= G_HAZE_WISP_STRETCH;
                np2.xz += float2(sin(x.y * 2.1f + haze_p9.x * 0.698132f), sin(x.y * 1.7f - haze_p9.x * 0.872665f + 1.3f)) * (G_HAZE_SWAY * hk);
                w2 = lerp(0.35f, 0.6f, hk);
                lo = lerp(0.2f, 0.36f, hk);
                hi = lerp(0.8f, 0.74f, hk);
                amount *= lerp(0.5f, 1.0f, hk);
            }
            float nz = ssfx_haze_noise(np) * (1.0f - w2) + ssfx_haze_noise(np2) * w2;
            sigma *= lerp(1.0f, smoothstep(lo, hi, nz) * 2.0f, amount);
#else
            float3 np = x * haze_p4.x + haze_p4.yzw * haze_p9.x * haze_p4.x;
            float nz = ssfx_haze_noise(np) * 0.65f + ssfx_haze_noise(np * 2.3f + 7.1f) * 0.35f;
            sigma *= lerp(1.0f, smoothstep(0.2f, 0.8f, nz) * 2.0f, haze_p1.w);
#endif
        }

        float3 Li = Li_base;
        [branch] if (!flat_view && haze_p6.w > 0.0f)
        {
            float3 lv = x - haze_p6.xyz;
            float ld = length(lv);
            float att = 1.0f - smoothstep(0.0f, 1.0f, ld / haze_p6.w); // .203: softer than att^2, range x light_range_k
            Li += haze_p7.rgb * att * ssfx_haze_phase(dot(-dir, lv / max(ld, 0.001f)), g);
        }

        float tr = exp(-sigma * dt);
        L += T * Li * (1.0f - tr);
        T *= tr;
    }

    // Debug view 4: the zone light term alone, replaces the scene ( black where the haze gets no zone light )
    if (light_only)
        return float4(L, 0.0f);

    // Debug view 2: optical depth ( blue thin .. red dense, replaces the scene )
    if (debug > 1.5f && debug < 2.5f)
        return float4(ssfx_haze_heat(-log(max(T, 0.0001f)) / 3.0f), 0.0f);

    // Weather distance fog in front of the haze
    float f = saturate((t0 + t1) * 0.5f * fog_params.w + fog_params.x);
    L *= 1.0f - f;
    T = lerp(T, 1.0f, f);

    return float4(L, T);
}

#endif // SSFX_HAZE_H
