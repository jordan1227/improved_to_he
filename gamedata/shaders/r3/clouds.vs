#include "common.h"
#include "shared\cloudconfig.h"

// NLC ( docs/RAIN_CLOUDS_DESIGN.md R5 ): the texture offsets come from the engine ( CEnvironment::nlc_clouds_update ),
// integrated from the wind direction, so the clouds drift with the wind and turn with it without jumping. Was
// dir ( vertex colour, fixed setHP( PI/4 ) / setHP( 3PI/8 ) ) x timers.z x CLOUD_SPEED. ssfx_cloud_shadows.h uses the
// same offsets. cloud_drift 0 gives the old directions.

struct vi
{
    float4 p : POSITION;
    float4 dir : COLOR0; // dir0,dir1(w<->z) ( NLC: unused )
    float4 color : COLOR1; // rgb. intensity
};

struct vf
{
    float4 color : COLOR0; // rgb. intensity, for SM3 - tonemap-prescaled, HI-res
    float2 tc0 : TEXCOORD0;
    float2 tc1 : TEXCOORD1;
    float4 hpos : SV_Position;
};

uniform float4 nlc_cloud_ofs; // layer 0 xy, layer 1 zw

vf main(vi v)
{
    vf o;
    o.hpos = mul(m_WVP, v.p); // xform, input in world coords
    o.hpos.xy = get_taa_jitter(o.hpos);

    // generate tcs
    o.tc0 = v.p.xz * CLOUD_TILE0 + nlc_cloud_ofs.xy;
    o.tc1 = v.p.xz * CLOUD_TILE1 + nlc_cloud_ofs.zw;

    o.color = v.color;
    o.color.w *= pow(v.p.y, 25.0f);

    return o;
}
