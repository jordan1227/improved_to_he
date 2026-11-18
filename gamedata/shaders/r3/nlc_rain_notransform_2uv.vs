#include "common_iostructs.h"

// NLC .209p: stub_notransform_2uv for the wet-surface passes ( rain.s ). draw_rain places its full-screen triangle at
// the depth of r3_dynamic_wet_surfaces_far; beyond the weather's far plane ( rain weathers ) the triangle was clipped
// and the whole pass ( wet gloss, running water, ripples on objects ) never drew. The depth is kept just inside the
// far plane: the GREATER depth test still skips the sky, the distance fade is in rain_patch_normal.ps
v2p_TL2uv main(v_TL2uv I)
{
    v2p_TL2uv O;

    O.HPos = I.P;
    O.HPos.z = min(O.HPos.z, O.HPos.w * 0.99999f);
    O.Tex0 = I.Tex0;
    O.Tex1 = I.Tex1;
    O.Color = I.Color.bgra; //	swizzle vertex colour

    return O;
}
