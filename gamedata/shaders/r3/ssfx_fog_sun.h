/**
 * @ Description: Sun-coloured fog ( aerial perspective towards the sun ), for every fogged surface
 * @ Based on: SCREEN SPACE SHADERS - UPDATE 14.4 height fog sun colour ( screenspace_fog.h, https://www.moddb.com/members/ascii1457 )
 * @ NLC: 2026-10-12 - SSS tinted only the height fog, weakly. Here the fog colour is pulled towards the sun colour
 *        with a forward-scattering lobe, for distance and height fog; it only brightens / warms ( max ), so night
 *        and overcast fog stay as the weather defines them. Strength ssfx_extra.z ( SSFX_FOG_SUN ).
 *        Globals only ( L_sun_dir_w, L_sun_color ), so forward shaders ( water ) match the deferred combine.
 */

#ifndef SSFX_FOG_SUN_H
#define SSFX_FOG_SUN_H

#include "ssfx_extra.h"

// dir_w: normalized world direction from the eye to the fogged point
float3 ssfx_fog_color_sun(float3 dir_w)
{
    float s = saturate(dot(L_sun_dir_w.xyz, -dir_w)); // 1 = looking into the sun ( sun_dir is the light travel direction )
    float lobe = pow(s, 8.0f) * 0.75f + s * s * 0.25f;
    return lerp(fog_color.rgb, max(fog_color.rgb, saturate(L_sun_color.rgb)), lobe * ssfx_extra.z);
}

#ifdef SSFX_FOG
#include "settings_screenspace_FOG.h"
#ifdef SSFX_GROUND_MIST
#include "ssfx_mist.h"
#endif

// SSFX_HEIGHT_FOG ( screenspace_fog.h ) with the sun-coloured fog colour
float ssfx_height_fog_sun(float3 P, float World_Py, inout float3 color)
{
    float3 dir_w = normalize(mul(m_inv_V, P));
    float3 fog_col = ssfx_fog_color_sun(dir_w);

    float3 Sun = saturate(dot(normalize(Ldynamic_dir), -normalize(P)));
    Sun = lerp(fog_col, Ldynamic_color.rgb, Sun);

    float fog = saturate(length(P) * fog_params.w + fog_params.x);
    float fogheight = smoothstep(G_FOG_HEIGHT, -G_FOG_HEIGHT, World_Py) * G_FOG_HEIGHT_INTENSITY;
    float fogresult = saturate(fog + fogheight * (fog * G_FOG_HEIGHT_DENSITY));
#ifdef SSFX_GROUND_MIST
    fogresult = saturate(fogresult + ssfx_mist(mul(m_inv_V, float4(P, 1.0f)).xyz, length(P), fogheight)); // NLC .84
#endif
    float FogBlend = fogheight * G_FOG_SUNCOLOR_INTENSITY;

    color = lerp(color, lerp(fog_col, Sun, FogBlend), fogresult);
    return fog;
}
#endif

#endif // SSFX_FOG_SUN_H
