/**
 * @ Version: SCREEN SPACE SHADERS - UPDATE 23
 * @ Description: Glass Shader - VS
 * @ Modified time: 2025-05-10 23:25:24
 * @ Author: https://www.moddb.com/members/ascii1457
 * @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
 * @ NLC: 2026-10-12 - OGSR model_env_lq.vs with the SSS 23 glass outputs; keeps the OGSR lighting ( c0 ) so
 *        ssfx_compare can show the old glass. Height fog uses the world height ( model_env_lq.vs passes the
 *        object-space height ).
 */

#include "common.h"
#include "skin.h"
#include "screenspace_fog.h"

struct vf
{
    float2 tc0 : TEXCOORD0; // base
    float3 tc1 : TEXCOORD1; // environment
    float3 c0 : COLOR0; // OGSR model light
    float3 P : TEXCOORD2; // object space position ( triplanar glass normal )
    float3 v2point : TEXCOORD3;
    float3 nor : TEXCOORD4;
    float fog : FOG;
    float4 hpos : SV_Position;
};

vf _main(v_model v, float3 psp)
{
    vf o;

    float4 pos = v.P;
    float3 pos_w = mul(m_W, pos);
    float3 norm_w = normalize(mul(m_W, v.N));

    o.hpos = mul(m_WVP, pos);
    o.hpos.xy = get_taa_jitter(o.hpos);

    o.tc0 = v.tc.xy;
    o.tc1 = calc_reflection(pos_w, norm_w);
    o.c0 = calc_model_lq_lighting(float3(0, 1, 0));

    o.fog = saturate(calc_fogging(float4(pos_w, 1)));
#ifdef SSFX_FOG
    o.fog = SSFX_FOGGING(1.0 - o.fog, pos_w.y);
#endif

    o.v2point = normalize(pos_w - eye_position);
    o.nor = norm_w;
    o.P = all(psp) ? psp : pos.xyz; // skinned: bind-pose position, so the pattern does not swim
    return o;
}

/////////////////////////////////////////////////////////////////////////
#ifdef SKIN_NONE
vf main(v_model v) { return _main(v, 0); }
#endif

#ifdef SKIN_0
vf main(v_model_skinned_0 v) { return _main(skinning_0(v), v.P.xyz); }
#endif

#ifdef SKIN_1
vf main(v_model_skinned_1 v) { return _main(skinning_1(v), v.P.xyz); }
#endif

#ifdef SKIN_2
vf main(v_model_skinned_2 v) { return _main(skinning_2(v), v.P.xyz); }
#endif

#ifdef SKIN_3
vf main(v_model_skinned_3 v) { return _main(skinning_3(v), v.P.xyz); }
#endif

#ifdef SKIN_4
vf main(v_model_skinned_4 v) { return _main(skinning_4(v), v.P.xyz); }
#endif
