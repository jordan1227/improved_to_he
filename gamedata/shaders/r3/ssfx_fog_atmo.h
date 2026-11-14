/**
 * @ Description: Atmospheric fog ( docs/FOG_DESIGN.md ): one fog evaluation for combine, fog scattering, water, glass
 *                and puddles
 * @ Based on: SCREEN SPACE SHADERS - UPDATE 14.4 height fog ( screenspace_fog.h, https://www.moddb.com/members/ascii1457 )
 * @ NLC: 2026-11-14 - fog colour from the sky near the horizon ( as skybox.ps draws it ), sky-access gate, interior fog
 *        modes, height fog from the level base, debug views. Used only with SSFX_FOG where ssfx_fog_new() is true; each
 *        consumer keeps its old fog code for ssfx_fog_mode 0 and the left half of the split view.
 *        Colour users bind s_fog_sky0 / s_fog_sky1 ( $user$sky0 / $user$sky1 ) and smp_rtlinear.
 * @ NLC: .201 - gate mode 2: the view ray is sampled in the rain depth map ( top-down depth of the level around the
 *        camera, $user$smap_rain as s_fog_map, smp_nofilter ): fog builds up only over the open-air part of the ray,
 *        interior fog over the covered part. Without the map ( mode 1, or SSFX_FOG_NO_MAP: water ) the .200 gate:
 *        camera sky openness + G-buffer hemi.
 */

#ifndef SSFX_FOG_ATMO_H
#define SSFX_FOG_ATMO_H

#include "settings_screenspace_FOG.h"
#include "ssfx_fog_params.h"
#include "ssfx_extra.h"
#ifdef SSFX_GROUND_MIST
#include "ssfx_mist.h"
#endif

uniform float4 ssfx_fog_sky; // rgb: sky tint ( skybox.vs: sky_color x 1.7 ), w: sky0 / sky1 weight
uniform float4 ssfx_fog_sky2; // x: sky rotation, y: underground level

// A consumer that already binds $user$sky0 / $user$sky1 under another name defines SSFX_FOG_SKY0 / SSFX_FOG_SKY1 before
// including this file ( water: sky_s0 / sky_s1 - it is at the 16 pixel-texture limit, mtMaxPixelShaderTextures )
#ifndef SSFX_FOG_SKY0
TextureCube s_fog_sky0; // $user$sky0
TextureCube s_fog_sky1; // $user$sky1
#define SSFX_FOG_SKY0 s_fog_sky0
#define SSFX_FOG_SKY1 s_fog_sky1
#endif

uniform float4 ssfx_fog_map; // x: map rendered this frame ( gate mode 2 ), y: map depth per metre, z: map texel size in metres

#ifndef SSFX_FOG_NO_MAP
uniform float4x4 ssfx_fog_map_xf; // world -> map ( uv, depth )
Texture2D s_fog_map; // $user$smap_rain

// 1 = no geometry above Pw ( or Pw outside the map ), 0 = covered. Tolerance max( 0.3 m, one texel ): the top surface
// itself and gentle slopes inside a texel stay open ( .205: was a fixed 0.3 m, coarse wet maps thinned hillside fog )
// .201b: the 2x2 texels around the point are tested and averaged ( soft edges instead of texel steps that blinked
// as the map follows the camera )
float ssfx_fog_map_open(float3 Pw)
{
    float4 p = mul(ssfx_fog_map_xf, float4(Pw, 1.0f));
    p.xyz /= p.w;
    float4 d4 = s_fog_map.GatherRed(smp_nofilter, saturate(p.xy));
    float open = dot(step(p.z - max(0.3f, ssfx_fog_map.z) * ssfx_fog_map.y, d4), 0.25f);
    bool inside = all(p.xy > 0.0f) && all(p.xy < 1.0f);
    return inside ? open : 1.0f;
}

// Open-air share of the segment eye -> Pw ( 8 samples, jittered per pixel and frame for TAA )
float ssfx_fog_path_open(float3 Pw)
{
    float noise = frac(sin(dot(Pw, float3(12.9898f, 78.233f, 37.719f))) * 43758.5453f + timers.x * 7.13f);
    float sum = 0.0f;
    [unroll] for (int i = 0; i < 8; i++)
        sum += ssfx_fog_map_open(lerp(eye_position, Pw, (float(i) + noise) / 8.0f));
    return sum / 8.0f;
}
#endif

struct ssfx_fog_core_t
{
    float3 dir_w; // eye -> point, normalized
    float dist;
    float fog_d; // weather distance fog over the whole ray
    float fog_d_out; // distance fog over the open-air part ( sky colour, sky alpha )
    float fog_out; // outdoor fog amount ( distance + height fog + mist )
    float fog_in; // interior fog amount
    float open; // outdoor share of the fog colour
    float h; // height above the level base
    float underground;
    float3 dbg; // debug view 2
};

struct ssfx_fog_res
{
    float amount; // blend towards colour ( 0 none .. 1 full fog )
    float3 colour;
    float sky_alpha; // combine: share of the sky drawn behind ( was skyblend = fog^3 )
};

// Pw: world position of the fogged point. hemi: G-buffer sky visibility, < 0 for forward shaders ( camera term only )
ssfx_fog_core_t ssfx_fog_core(float3 Pw, float hemi)
{
    ssfx_fog_core_t c;

    float3 to_p = Pw - eye_position;
    c.dist = length(to_p);
    c.dir_w = to_p / max(c.dist, 0.001f);
    c.fog_d = saturate(c.dist * fog_params.w + fog_params.x);

    // Underground levels keep the fog their weather defines ( no gate, no height fog )
    c.underground = ssfx_fog_sky2.y;
    float gate = ssfx_fog_gate.x * (1.0f - c.underground);
    bool interior = ssfx_fog_gate.z > 0.5f;

    // Height fog layer from the level base
    c.h = Pw.y - ssfx_fog_gate2.w;
    float fog_h0 = smoothstep(G_FOG_HEIGHT, -G_FOG_HEIGHT, c.h) * G_FOG_HEIGHT_INTENSITY * (1.0f - c.underground);

#ifndef SSFX_FOG_NO_MAP
    bool use_map = ssfx_fog_map.x > 0.5f && gate > 0.0f;
#else
    // .205: no map texture ( water, 16-texture limit ), but in gate mode 2 the camera gate is skipped too: open-air
    // fog over the whole ray ( the .200 camera gate left water unfogged when seen from inside a building )
    bool use_map = ssfx_fog_map.x > 0.5f && gate > 0.0f;
#endif

    [branch] if (use_map)
    {
        // Gate mode 2: fog over the open-air part of the view ray, interior fog over the covered part
        float path = 1.0f, pt_open = 1.0f;
#ifndef SSFX_FOG_NO_MAP
        path = lerp(1.0f, ssfx_fog_path_open(Pw), gate);
        [branch] if (ssfx_fog_atmo.z > 1.5f && ssfx_fog_atmo.z < 2.5f) // debug view 2 only
            pt_open = lerp(1.0f, ssfx_fog_map_open(Pw + float3(0.0f, 0.05f, 0.0f)), gate);
#endif
        c.fog_d_out = saturate(c.dist * path * fog_params.w + fog_params.x);
        // .201b: ground fog / mist by the open share of the ray, not by the point's own cover ( walls under eaves and
        // tree trunks lost all ground fog and blinked at map texel edges ); pt_open is only shown in debug view 2
        float fog_h = fog_h0 * path;
        c.fog_out = saturate(c.fog_d_out + fog_h * (c.fog_d_out * G_FOG_HEIGHT_DENSITY));
#ifdef SSFX_GROUND_MIST
        c.fog_out = saturate(c.fog_out + ssfx_mist(Pw, c.dist, fog_h));
#endif
        c.fog_in = interior ? saturate(c.dist * (1.0f - path) * fog_params.w + fog_params.x) * ssfx_fog_gate.w : 0.0f;
        c.dbg = float3(pt_open, path, 0.0f);
    }
    else
    {
        // Gate mode 1 ( .200 ): camera sky openness + surface sky access ( G-buffer hemi )
        float cam = smoothstep(0.15f, 0.5f, ssfx_fog_gate.y); // a bus-stop roof or a canopy still counts as outside
        bool has_hemi = hemi >= 0.0f;
        float surf = has_hemi ? smoothstep(ssfx_fog_gate2.x, ssfx_fog_gate2.y, hemi) : 0.0f;
        float open = lerp(1.0f, has_hemi ? 1.0f - (1.0f - cam) * (1.0f - surf) : cam, gate);
        float fog_h = fog_h0 * lerp(1.0f, has_hemi ? surf : cam, gate);
        float fog = saturate(c.fog_d + fog_h * (c.fog_d * G_FOG_HEIGHT_DENSITY));
#ifdef SSFX_GROUND_MIST
        fog = saturate(fog + ssfx_mist(Pw, c.dist, fog_h));
#endif
        c.fog_out = fog * open;
        c.fog_in = interior ? c.fog_d * ssfx_fog_gate.w * (1.0f - open) : 0.0f;
        c.fog_d_out = c.fog_d * open;
        c.dbg = float3(surf, cam, 0.0f);
    }

    float sum = c.fog_out + c.fog_in;
    c.open = sum > 0.0001f ? c.fog_out / sum : 1.0f;
    c.dbg.z = c.open;
    return c;
}

float ssfx_fog_core_amount(ssfx_fog_core_t c) { return saturate(c.fog_out + c.fog_in); }

float ssfx_fog_amount(float3 Pw, float hemi) { return ssfx_fog_core_amount(ssfx_fog_core(Pw, hemi)); }

// .205b: distance part only, for effect fades that used the plain weather distance fog ( SSR, water SSR ): on an
// open-air ray it equals that fog exactly, under a roof it follows the gate ( outdoor part + interior fog )
float ssfx_fog_amount_dist(float3 Pw, float hemi)
{
    ssfx_fog_core_t c = ssfx_fog_core(Pw, hemi);
    return saturate(c.fog_d_out + c.fog_in);
}

// Sky colour in a world direction as skybox.ps draws it: the cube direction is the world direction turned by
// -sky_rotation ( inverse of the skybox world matrix rotateY( sky_rotation ) ), tint and weight as skybox.vs.
// Clamped to the horizon band; mip 3 smooths clouds and the sun disk when the cube has mips.
float3 ssfx_fog_sky_colour(float3 dir_w)
{
    float3 d = float3(dir_w.x, clamp(dir_w.y, 0.03f, 0.35f), dir_w.z);
    float s, co;
    sincos(-ssfx_fog_sky2.x, s, co);
    d = normalize(float3(co * d.x + s * d.z, d.y, -s * d.x + co * d.z));
    float3 s0 = SSFX_FOG_SKY0.SampleLevel(smp_rtlinear, d, 3).rgb;
    float3 s1 = SSFX_FOG_SKY1.SampleLevel(smp_rtlinear, d, 3).rgb;
    return lerp(s0, s1, ssfx_fog_sky.w) * ssfx_fog_sky.rgb;
}

// ssfx_fog_sun.h lobe ( forward scattering towards the sun, strength ssfx_fog_sun ) on any base colour
float3 ssfx_fog_sun_lobe(float3 base, float3 dir_w)
{
    float s = saturate(dot(L_sun_dir_w.xyz, -dir_w));
    float lobe = pow(s, 8.0f) * 0.75f + s * s * 0.25f;
    return lerp(base, max(base, saturate(L_sun_color.rgb)), lobe * ssfx_extra.z);
}

ssfx_fog_res ssfx_fog_eval_core(ssfx_fog_core_t c)
{
    ssfx_fog_res r;

    // Outdoor colour: the weather fog colour, towards the sky behind it with distance, then the sun lobe
    float3 col = fog_color.rgb;
    float k = saturate(ssfx_fog_atmo.w * smoothstep(0.25f, 1.0f, c.fog_d_out)) * (1.0f - c.underground);
    [branch] if (k > 0.001f)
        col = lerp(col, ssfx_fog_sky_colour(c.dir_w), k);
    col = ssfx_fog_sun_lobe(col, c.dir_w);

    // Interior colour: the fog colour dimmed ( depth without the daylight glow )
    float3 col_in = fog_color.rgb * ssfx_fog_gate2.z;

    r.amount = ssfx_fog_core_amount(c);
    r.colour = lerp(col_in, col, c.open);
    r.sky_alpha = saturate(pow(c.fog_d_out, 3.0f));
    return r;
}

ssfx_fog_res ssfx_fog_eval(float3 Pw, float hemi) { return ssfx_fog_eval_core(ssfx_fog_core(Pw, hemi)); }

// ssfx_fog_debug views ( combine ): 1 fog amount, 2 sky access ( gate mode 2: r point open, g open share of the view
// ray; mode 1: r surface, g camera; b outdoor share of the fog colour ), 3 height above the base ( 2 m bands, red inside
// the height fog layer, white at the base ), 4 fog colour, 5 sky lookup at full strength
float3 ssfx_fog_debug_view(ssfx_fog_core_t c, ssfx_fog_res r)
{
    float view = ssfx_fog_atmo.z;
    float3 res;

    [branch] if (view < 1.5f)
        res = r.amount.xxx;
    else if (view < 2.5f)
        res = c.dbg;
    else if (view < 3.5f)
    {
        float inside = step(abs(c.h), G_FOG_HEIGHT);
        res = lerp(float3(0.1f, 0.35f, 1.0f), float3(1.0f, 0.25f, 0.05f), inside) * (0.45f + 0.55f * frac(c.h * 0.5f));
        res = abs(c.h) < 0.15f ? float3(1.0f, 1.0f, 1.0f) : res;
    }
    else if (view < 4.5f)
        res = r.colour;
    else
        res = ssfx_fog_sky_colour(c.dir_w);

    return res;
}

#endif // SSFX_FOG_ATMO_H
