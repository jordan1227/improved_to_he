#ifndef IMG_CORRECTIONS_H
#define IMG_CORRECTIONS_H
#include "common.h"

// NLC .85: look presets and colour balance ( Shaders tab ). LDR, after tone mapping: only the picture changes, not the
// scene light levels ( stealth ). ssfx_look.x: 0 Neutral, 1 NLC, 2 Cold, 3 Classic; .y: contrast. ssfx_color_balance: rgb.
uniform float4 ssfx_look;
uniform float4 ssfx_color_balance;

struct ssfx_look_setup
{
    float curve; // S-curve amount ( contrast around the mid-tones, keeps black and white )
    float3 shadow_tint, light_tint; // split toning, multipliers
    float vibrance; // saturates the less saturated colours more
    float3 haze; // lifted blacks ( added in the darks )
};

ssfx_look_setup ssfx_get_look(int id)
{
    ssfx_look_setup L;
    L.curve = 0; L.shadow_tint = 1; L.light_tint = 1; L.vibrance = 0; L.haze = 0;
    if (id == 1) // NLC: punchier, cooler shadows, neutral light ( .86 test: less yellow sky, grass not oversaturated )
    {
        L.curve = 0.3f; L.shadow_tint = float3(0.98f, 1.0f, 1.04f); L.light_tint = float3(0.995f, 1.0f, 1.012f); L.vibrance = 0.05f;
    }
    else if (id == 2) // Cold: slightly dreamy, a touch blue, soft haze, less yellow
    {
        L.curve = 0.08f; L.shadow_tint = float3(0.96f, 0.99f, 1.06f); L.light_tint = float3(0.98f, 1.0f, 1.02f); L.vibrance = 0.05f;
        L.haze = float3(0.018f, 0.024f, 0.036f);
    }
    else if (id == 3) // Classic: old NLC ReShade feel, strong contrast, warm highlights / cool shadows
    {
        L.curve = 0.5f; L.shadow_tint = float3(0.95f, 0.99f, 1.07f); L.light_tint = float3(1.05f, 1.0f, 0.94f); L.vibrance = 0.25f;
    }
    return L;
}

float3 ssfx_apply_look(float3 img)
{
    int id = (int)(ssfx_look.x + 0.5f);
    [branch] if (id > 0)
    {
        ssfx_look_setup L = ssfx_get_look(id);
        img = saturate(img);

        float3 s = img * img * (3.0f - 2.0f * img);
        img = lerp(img, s, L.curve);

        float lum = dot(img, LUMINANCE_VECTOR);
        img *= lerp(L.shadow_tint, L.light_tint, smoothstep(0.1f, 0.8f, lum));

        float mx = max(img.r, max(img.g, img.b)), mn = min(img.r, min(img.g, img.b));
        img = lerp(dot(img, LUMINANCE_VECTOR), img, 1.0f + L.vibrance * (1.0f - saturate((mx - mn) * 2.0f)));

        img += L.haze * (1.0f - saturate(lum * 2.5f));
    }

    img = (img - 0.5f) * ssfx_look.y + 0.5f; // contrast slider
    img *= ssfx_color_balance.rgb; // colour balance sliders
    return saturate(img);
}

float3 img_corrections(float3 img)
{
    // exposure
    img.xyz *= pp_img_corrections.x;

    // color grading (thanks KD and Crytek and Cjayho)
    if (pp_img_cg.x > 0.0 || pp_img_cg.y > 0.0 || pp_img_cg.z > 0.0)
    {
        float fLum = dot(img.xyz, LUMINANCE_VECTOR);
        float3 cColor = lerp(0.0, pp_img_cg.xyz, saturate(fLum * 2.0));
        cColor = lerp(cColor, 1.0, saturate(fLum - 0.5) * 2.0);

        img.xyz = saturate(lerp(img.xyz, cColor.xyz, saturate(fLum * 0.15)));
    }

    img.xyz = ssfx_apply_look(img.xyz); // NLC .85

    // saturation
    img.xyz = saturate(lerp(img.xyz, dot(img.xyz, LUMINANCE_VECTOR), (1.0 - pp_img_corrections.z)));

    // gamma correction
    img.xyz = pow(img, (1.0 / pp_img_corrections.y));

    // that's all :)
    return img.xyz;
}
#endif