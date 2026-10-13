// NLC: Build C strengths ( Blender_Recorder_StandartBinding.cpp, ssfx_extra ). All 0 / 1 neutral while ssfx_compare is on.
#ifndef SSFX_EXTRA_H
#define SSFX_EXTRA_H
uniform float4 ssfx_extra; // x: wind gusts, y: HUD bloom, z: sun-coloured fog, w: specular occlusion
uniform float4 ssfx_extra2; // NLC .84: x: AO on direct light, y: ground mist, zw: - ( 0 while ssfx_compare )
#endif
