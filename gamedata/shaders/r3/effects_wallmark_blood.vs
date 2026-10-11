/**
 * @ Version: SCREEN SPACE SHADERS - UPDATE 17
 * @ Description: Decals - VS
 * @ Modified time: 2023-07-05 05:58
 * @ NLC: view-ray depth bias against z-fighting with the ground
 * @ Author: https://www.moddb.com/members/ascii1457
 * @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
 */

#include "common.h"

v2p_TL main(v_TL I)
{
    v2p_TL O;

    // NLC: the marks are clipped to the collision mesh and carry no normal, so the only depth
    // bias was the constant projection shift of CWallmarksEngine::Render. Pull each vertex
    // towards the eye along its own view ray: the screen position does not change, only the
    // depth, so the mark wins the depth test against the (coplanar) ground at any range.
    float3 to_eye = eye_position - I.P.xyz;
    float dist = length(to_eye);
    float shift = min(0.02f + 0.003f * dist, 0.25f);
    float4 P = float4(I.P.xyz + to_eye * (shift / max(dist, 0.001f)), 1.0f);

    O.HPos = mul(m_VP, P);
    O.HPos.xy = get_taa_jitter(O.HPos);
    O.Tex0 = I.Tex0;
    O.Color = I.Color.bgra;

    return O;
}