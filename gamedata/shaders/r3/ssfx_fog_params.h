/**
 * @ Description: Fog redesign uniforms ( docs/FOG_DESIGN.md ), shared by screenspace_fog.h and ssfx_fog_atmo.h
 * @ NLC: 2026-11-14. Binders: Blender_Recorder_StandartBinding.cpp ( ssfx_fog_* ). Live console variables, no restart.
 */

#ifndef SSFX_FOG_PARAMS_H
#define SSFX_FOG_PARAMS_H

uniform float4 ssfx_fog_atmo; // x: mode ( 0 old fog, also while ssfx_compare ), y: split view, z: debug view, w: sky colour strength
uniform float4 ssfx_fog_gate; // x: gate strength, y: camera sky openness, z: interior mode, w: interior density
uniform float4 ssfx_fog_gate2; // x / y: hemi remap low / high, z: interior colour scale, w: height base ( world Y )

// The new fog runs at this screen position ( mode 1; with the split view only on the right half ).
// screen_x: SV_Position.x in pixels
bool ssfx_fog_new(float screen_x) { return ssfx_fog_atmo.x > 0.5f && (ssfx_fog_atmo.y < 0.5f || screen_x >= screen_res.x * 0.5f); }

// Height fog base for shaders that cannot tell the split half ( vertex shaders ): the level base in mode 1
float ssfx_fog_base_any() { return ssfx_fog_atmo.x > 0.5f ? ssfx_fog_gate2.w : 0.0f; }

#endif // SSFX_FOG_PARAMS_H
