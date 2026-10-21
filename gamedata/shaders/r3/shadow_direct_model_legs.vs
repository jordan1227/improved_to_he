#include "common.h"
#include "skin.h"

// NLC: shadow pass of the visible-body puppet (models_selflight_det_3.s, elements 2 and 3). Same as
// shadow_direct_model.vs, plus the skinned model-space height, which shadow_legs.ps clips: the puppet is a full body
// and only its legs may cast the shadow (the actor model supplies the upper body).
struct v2p_shadow_legs
{
    float4 hpos : SV_Position; // Clip-space position         (for rasterization)
    float h : TEXCOORD0; // Model-space height above the puppet's feet
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
v2p_shadow_legs _main(v_model I)
{
    v2p_shadow_legs O;
    O.hpos = mul(m_WVP, I.P);
    O.h = I.P.y;
    return O;
}

/////////////////////////////////////////////////////////////////////////
#ifdef SKIN_NONE
v2p_shadow_legs main(v_model v) { return _main(v); }
#endif

#ifdef SKIN_0
v2p_shadow_legs main(v_model_skinned_0 v) { return _main(skinning_0(v)); }
#endif

#ifdef SKIN_1
v2p_shadow_legs main(v_model_skinned_1 v) { return _main(skinning_1(v)); }
#endif

#ifdef SKIN_2
v2p_shadow_legs main(v_model_skinned_2 v) { return _main(skinning_2(v)); }
#endif

#ifdef SKIN_3
v2p_shadow_legs main(v_model_skinned_3 v) { return _main(skinning_3(v)); }
#endif

#ifdef SKIN_4
v2p_shadow_legs main(v_model_skinned_4 v) { return _main(skinning_4(v)); }
#endif

FXVS;
