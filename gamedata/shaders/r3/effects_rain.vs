/**
 * @ Version: SCREEN SPACE SHADERS - UPDATE 17
 * @ Description: Rain Shader - VS
 * @ Modified time: 2023-06-22 20:16
 * @ Author: https://www.moddb.com/members/ascii1457
 * @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
 * @ NLC .213b: passes the vertex colour alpha ( rain strength from the weather density / 2, dxRainRender.cpp )
 */

#include "common.h"

// Vertex Struct
struct v2p_Rain
{
    float2 Tex0 : TEXCOORD0;
    float4 tc : TEXCOORD1;
    float rain_a : TEXCOORD2; // NLC .213b
    float4 HPos : SV_Position;
};

uniform float4x4 mVPTexgen;

v2p_Rain main(v_TL I)
{
    v2p_Rain O;

    // Basic Stuff
    O.HPos = mul(m_WVP, I.P);
    O.Tex0 = I.Tex0;

    // Screen Space Data
    O.tc = mul(mVPTexgen, I.P);
    O.tc.z = O.HPos.z;

    O.rain_a = I.Color.a * 2.0f; // NLC .213b: 0.3 ~ 1.5

    return O;
}