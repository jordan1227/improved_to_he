/**
 * @ Description: NLC .216 puddle / water ripples ( docs/REFLECTIONS_AO_DESIGN.md 4.28 )
 *   - impact rings: bullets on static geometry, physics contacts ( bolts, grenades, bodies ), explosions, footfalls
 *     ( engine nlc_impact_add, binder ssfx_impacts* in Blender_Recorder_StandartBinding.cpp, console ssfx_impact_ripples )
 *   - rain rings whose count follows the rain ( a share of the ring drops per cycle ) instead of only their strength
 */

#ifndef NLC_RIPPLES_H
#define NLC_RIPPLES_H

uniform float4 ssfx_impacts[16]; // xyz: position, w: age s
uniform float4 ssfx_impacts_p[16]; // x: strength, y: ring front speed m/s, z: wave number 1/m, w: lifetime s
uniform float4 ssfx_impacts_n; // x: count, y: strength ( ssfx_impact_ripples, 0 = off )
uniform float4 ssfx_puddle_ripples; // A/B, 1 = old: x level-wide UVs + old base normal, y rain ring density, z rain pass rings on terrain, w no wind streaks

// Expanding ring packets. W: world position, px: metres per pixel ( rings fade when a wavelength gets below ~3 pixels ).
// Returns a tangent offset ( world x / z ) for the surface normal
float2 nlc_impact_ripples(float3 W, float px)
{
    float2 n = 0;
    int cnt = (int)ssfx_impacts_n.x;
    [loop] for (int i = 0; i < cnt; i++)
    {
        float4 e = ssfx_impacts[i];
        float4 p = ssfx_impacts_p[i];
        float2 d = W.xz - e.xz;
        float r2 = dot(d, d);
        float rmax = p.y * p.w + 0.3f;
        if (r2 > rmax * rmax || abs(W.y - e.y) > 0.4f + 0.5f * p.y)
            continue;

        float r = sqrt(r2);
        // .217: far away the waves get longer ( wavelength >= ~4.5 pixels, at least 0.5 m: k >= 12.6 ) instead of fading
        // ( a 6 cm bullet ring vanished at ~15 m )
        float k = max(min(p.z, 1.0f / (1.5f * px)), min(p.z, 12.6f));
        float front = p.y * e.w;
        float w = 0.06f + 0.25f * front; // the packet widens as it spreads
        float x = (r - front) / w;
        float life = saturate(1.0f - e.w / p.w);
        float aa = saturate(2.0f - k * px);
        float h = cos(k * (r - front)) * exp(-x * x) * p.x * life * life / (1.0f + 2.0f * r) * aa;
        n += d / max(r, 0.001f) * h;
    }
    return n * (0.5f * ssfx_impacts_n.y);
}

// ssfx_process_ripples ( screenspace_common_ripples.h ) with a density: each ring drop of the atlas ( its time offset )
// is drawn in that share of its cycles, hashed per drop and cycle
float2 nlc_process_ripples(float4 ripples, float3 setup, float density)
{
    float2 ripples_N = ripples.yz * 2.0 - 1.0;

    float t = ripples.w + timers.x * setup.x;
    float RFrac = frac(t);
    float h = frac((ripples.w * 7.13f + floor(t)) * 0.1031f);
    h *= h + 33.33f;
    h = frac((h + h) * h);
    float on = step(h, density);

    float TimeFrac = RFrac - 1.0f + ripples.x;
    float RFreq = clamp(TimeFrac * setup.z, 0.0f, 4.0);
    float FinalFactor = saturate(0.7f - RFrac) * ripples.x * sin(RFreq * SSFX_ripples_PI);
    FinalFactor *= smoothstep(4.0f, 0, RFreq);

    return ripples_N * (FinalFactor * setup.y * on);
}

float2 nlc_rain_ripples(Texture2D ripples_tex, float2 uvs, float3 setup, float depth, float density)
{
    float Dist = 15.0f - depth;
    if (Dist < 0)
        return 0;

    float4 Layer0 = ripples_tex.Sample(smp_linear, uvs);
    float4 Layer1 = ripples_tex.Sample(smp_linear, uvs * 0.61f + SSFX_ripples_offset.xy);
    float4 Layer2 = ripples_tex.Sample(smp_linear, uvs * 0.87f + SSFX_ripples_offset.zw);

    float2 result = nlc_process_ripples(Layer0, float3(SSFX_ripples_speed.x * setup.x, setup.yz), density) +
        nlc_process_ripples(Layer1, float3(SSFX_ripples_speed.y * setup.x, setup.yz), density) +
        nlc_process_ripples(Layer2, float3(SSFX_ripples_speed.z * setup.x, setup.yz), density);

    result *= Dist * 0.0666f;
    return clamp(result, -1.0f, 1.0f);
}

#endif // NLC_RIPPLES_H
