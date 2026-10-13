// [ SETTINGS ] [ FLORA FIXES ]

#ifndef settings_screenspace_FLORA
#define settings_screenspace_FLORA

uniform float4 ssfx_florafixes_1;
uniform float4 ssfx_florafixes_2;

// [ NLC ] Grass colour variation ( ssfx_flora_variation.h ). Only compiled with SSFX_FLORA_VARIATION ( engine macro:
// ssfx_flora_variation or ssfx_tree_variation > 0 at game start ), so Minimum / Low pay nothing for it.
#ifdef SSFX_FLORA_VARIATION
#define G_FLORA_GRASS_VARIATION 0.7f // Strength of the dry/lush tint. [ 0.0f = off ~ 1.0f = full tint ]
#endif
#define G_FLORA_GRASS_PATCH_SIZE 14.0f // Average size of a dry or lush patch in metres.
#define G_FLORA_GRASS_DRY_TINT float3(1.10f, 1.00f, 0.74f) // Albedo multiplier of the driest patches ( straw ).
#define G_FLORA_GRASS_LUSH_TINT float3(0.86f, 0.98f, 0.88f) // Albedo multiplier of the lushest patches ( deeper green ).
#define G_FLORA_GRASS_BRIGHTNESS 0.05f // Random brightness per grass instance. [ 0.05f = +-5% ]

#endif
