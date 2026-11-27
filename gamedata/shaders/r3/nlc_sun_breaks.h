/**
 * @ Description: Sun breaks in overcast ( docs/RAIN_CLOUDS_DESIGN.md 5.4 / 5.7 )
 * @ NLC .239: a weather section with nlc_sun_breaks has its sun on, but cloud shadows ( ssfx_cloud_shadows.h ) hide it
 *        everywhere except the gaps. Sun terms that do not go through the cloud shadow ( fog sun lobe, water and
 *        puddle glints, splash glint ) fade out with nlc_sun_breaks_k(), so the overcast shows no sun position.
 */

#ifndef NLC_SUN_BREAKS_H
#define NLC_SUN_BREAKS_H

uniform float4 nlc_sun_breaks; // x = sun-break coverage of the weather blend ( Blender_Recorder_StandartBinding.cpp )

// 0 .. 1, how far the sun-break mode applies ( blended in over the first 0.3 of coverage )
float nlc_sun_breaks_k() { return smoothstep(0.0f, 0.3f, nlc_sun_breaks.x); }

#endif // NLC_SUN_BREAKS_H
