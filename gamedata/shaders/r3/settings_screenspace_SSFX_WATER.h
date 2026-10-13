// [ SETTINGS ] [ SSS 23 WATER ] ( NLC: ssfx_water*.ps )
// Started from the SSS 23 ssfx_water_setup1/2 defaults; refraction, soft border, reflection colour and turbidity
// are tuned back towards the OGSR water look ( 2026-10-12 test: SSS values made shores transparent and smeared
// the reflections ). The old water ( water.ps ) keeps settings_screenspace_WATER.h.

#define G_SSFX_WATER_DISTORTION 0.6f // Waves normal intensity ( refraction and reflection offset )
#define G_SSFX_WATER_TURBIDITY 3.0f // Turbidity factor ( 0.0f = clear water )
#define G_SSFX_WATER_SOFTBORDER 0.1f // Soft border depth in metres ( 0.0f = hard edge; SSS 23: 0.3f )
#define G_SSFX_WATER_REFRACTION 0.4f // Refraction offset, fraction of the wave normal ( OGSR water ~ 0.4 )
#define G_SSFX_WATER_REFL_DISTORTION 0.35f // Reflection offset, fraction of the wave normal
#define G_SSFX_WATER_PARALLAX_HEIGHT 0.05f // Parallax wave height
#define G_SSFX_WATER_PARALLAX_DISTANCE 25.0f // Max distance of the parallax waves
#define G_SSFX_WATER_REFLECTION 0.8f // Reflection intensity, scaled by ssfx_water_reflection
#define G_SSFX_WATER_SPECULAR 6.0f // Sun / moon specular intensity
#define G_SSFX_WATER_SPECULAR_NORMAL 0.2f // Specular normal intensity
#define G_SSFX_WATER_CAUSTICS 0.3f // Caustics intensity
#define G_SSFX_WATER_CAUSTICS_SCALE 1.0f // Caustics size
#define G_SSFX_WATER_RIPPLES 0.5f // Rain ripples intensity
#define G_SSFX_WATER_FOG_MAXDEPTH 2.0f // Maximum visibility under water
#define G_SSFX_WATER_VIBRANCE 0.9f // Reflection colour vibrance ( 0 = grey ~ 1 = full colour; SSS 23: 0.6f )
#define G_SSFX_WATER_SKY_REFLECTION 1.0f // Sky reflection factor
#define G_SSFX_WATER_MAP_REFLECTION 1.0f // Scene reflection factor
#define G_SSFX_WATER_HISTORY 0.95f // Reflection history weight ( 0 = no accumulation )
