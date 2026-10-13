/**
 * @ Version: SCREEN SPACE SHADERS - UPDATE 19
 * @ Description: Wind Main File
 * @ Modified time: 2023-12-23 16:05
 * @ Author: https://www.moddb.com/members/ascii1457
 * @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
 * @ NLC: 2026-10-12 - wind gusts ( SSFX_WIND_GUSTS, strength ssfx_extra.x ): a large, slow noise field drifts
 *        downwind and scales the wind strength, so fields and tree lines no longer sway in sync. Trees take the gust
 *        at their origin ( trunk and branches move together ).
 */

uniform float4 wind_params;
uniform float4 wind_params_old;

uniform float4 ssfx_wsetup_grass; // Anim Speed - Turbulence - Push - Wave
uniform float4 ssfx_wsetup_trees; // Branches Speed - Trunk Speed - Bending - Min Wind Speed
uniform float4 ssfx_wind_anim;
uniform float4 ssfx_wind_anim_old;

// Simp: костыль для отключения движения веток на флоре, которая не подходит под эти шейдеры
// Для старых елочек в OGSR GA например. В новом рендере пока отключено, соотв. нужно отключить и тут.
// В новых модах надо делать норм флору под этот ветер без подобных костылей.
//uniform float is_bugged_flora;

Texture2D s_waves;
sampler smp_linear2;

#ifdef SSFX_WIND_GUSTS
#include "ssfx_extra.h" // x: wind gust strength ( 0 ~ 1 )

// NLC: breezes in calm weather - added wind speed at a world position ( 0 ~ about 0.4 at defaults ). The wind motion is
// scaled by the wind speed, so near calm a gust multiplier shows nothing: breeze patches add wind of their own, fading
// out as the weather wind gets stronger. The patches drift on their own ( ~4 m/s along the weather wind direction; calm
// wind_anim barely moves ) and a slow global envelope ( two sines, ~30 s and ~70 s ) lets them build up and die down
// over several seconds, so they read as passing events rather than a static pattern or an on / off switch.
float ssfx_wind_breeze_at(float2 pos_xz, float speed, const bool for_old_frame)
{
    float t = timers.x;
    float r = -(for_old_frame ? wind_params_old.x : wind_params.x) + 1.57079f;
    float2 drift = float2(cos(r), sin(r)) * t * 4.0f;
    float b = s_waves.SampleLevel(smp_linear2, (pos_xz - drift) * 0.008f + 0.37f, 0).z;
    b = smoothstep(0.2f, 0.9f, b);
    float env = smoothstep(-0.2f, 0.9f, sin(t * 0.21f) * 0.6f + sin(t * 0.087f + 2.1f) * 0.4f);
    return b * env * ssfx_extra.x * 0.8f * saturate(1.0f - speed * 2.0f);
}

// Gust multiplier of the wind strength at a world position ( 1 = no gust ), breeze excluded
float ssfx_wind_gust_mult(float2 pos_xz, float sqrt_speed, const bool for_old_frame)
{
    // Blobs of a few tens of metres moving downwind ( wind_anim advances with the wind velocity )
    float2 anim = for_old_frame ? ssfx_wind_anim_old.xy : ssfx_wind_anim.xy;
    float g = s_waves.SampleLevel(smp_linear2, (pos_xz - anim * 6.0f) * 0.006f, 0).z;
    g = smoothstep(0.25f, 0.85f, g) * 2.0f - 1.0f; // -1 lull ~ +1 gust

    // More contrast in stronger wind
    return max(0.0f, 1.0f + g * ssfx_extra.x * (0.4f + 0.6f * sqrt_speed));
}

// Gust multiplier with the breeze folded in ( trees: their motion scales with the wind speed )
float ssfx_wind_gust_at(float2 pos_xz, float speed, float sqrt_speed, const bool for_old_frame)
{
    // NLC .82: the breeze boost is capped ( near calm breeze / speed reached x5 and trees thrashed )
    return ssfx_wind_gust_mult(pos_xz, sqrt_speed, for_old_frame) + min(ssfx_wind_breeze_at(pos_xz, speed, for_old_frame) / max(speed, 0.01f), 1.5f);
}
#endif

struct wind_setup
{
    float2 direction;
    float speed;
    float sqrt_speed;

    float trees_animspeed;
    float trees_trunk_animspeed;
    float trees_bend;

    float grass_animspeed;
    float grass_turbulence;
    float grass_push;
    float grass_wave;
};

wind_setup ssfx_wind_setup(const bool for_old_frame = false)
{
    wind_setup wsetup;

    // Direction. Radians to Vector
    float r = -(for_old_frame ? wind_params_old.x : wind_params.x) + 1.57079f;
    wsetup.direction = float2(cos(r), sin(r));

    // Wind Speed
    wsetup.speed = max(ssfx_wsetup_trees.w, saturate((for_old_frame ? wind_params_old.y : wind_params.y) * 0.001));
    wsetup.sqrt_speed = saturate(sqrt(wsetup.speed * 1.66f));

    // Setup
    wsetup.grass_animspeed = ssfx_wsetup_grass.x;
    wsetup.grass_turbulence = ssfx_wsetup_grass.y;
    wsetup.grass_push = ssfx_wsetup_grass.z;
    wsetup.grass_wave = ssfx_wsetup_grass.w;

    wsetup.trees_animspeed = ssfx_wsetup_trees.x;
    wsetup.trees_trunk_animspeed = ssfx_wsetup_trees.y;
    wsetup.trees_bend = ssfx_wsetup_trees.z;

    return wsetup;
}

#ifdef SSFX_WIND_ISGRASS

// Flow Map - X: X-Anim | Y: Z-Anim | Z: Wind Wave | W : Detail

float3 ssfx_wind_grass(float3 pos, float H, wind_setup W, const bool for_old_frame = false)
{
    // Height Limit. ( Add stiffness to tall grass )
    float HLimit = saturate(H * H - 0.01f) * saturate(1.0f - H * 0.1f);

    // Offset animation
    float2 Offset = -(for_old_frame ? ssfx_wind_anim_old.xy : ssfx_wind_anim.xy) * W.grass_animspeed;

    // Sample ( The scale defines the detail of the motion )
    float3 Flow = s_waves.SampleLevel(smp_linear2, (pos.xz + Offset) * 0.018f, 0);

    // Grass Motion ( -1.0 ~ 1.0 ). Turbulence.
    float2 GrassMotion = (Flow.xy * 2.0f - 1.0f) * W.grass_turbulence;

    // Apply wind direction and flow. Wind push.
    float2 WindDir = Flow.z * W.direction * W.grass_push;

    // Add everything and apply height limit
    float3 Final = float3(GrassMotion.x + WindDir.x, Flow.z * W.grass_wave, GrassMotion.y + WindDir.y) * W.speed * HLimit;

#ifdef SSFX_WIND_GUSTS
    Final *= ssfx_wind_gust_mult(pos.xz, W.sqrt_speed, for_old_frame);

    // NLC .81: breezes get their own waves running through the grass ( flow sampled along the breeze drift ), 1.5x
    // stronger than the same wind speed would give, so a breeze reads on fields, not only on trees
    float breeze = ssfx_wind_breeze_at(pos.xz, W.speed, for_old_frame);
    [branch] if (breeze > 0.001f)
    {
        float3 FlowB = s_waves.SampleLevel(smp_linear2, (pos.xz - W.direction * timers.x * 4.0f) * 0.018f, 0);
        float2 BMotion = (FlowB.xy * 2.0f - 1.0f) * W.grass_turbulence + FlowB.z * W.direction * W.grass_push;
        Final += float3(BMotion.x, FlowB.z * W.grass_wave, BMotion.y) * breeze * 1.5f * HLimit;
    }
#endif

    return Final;
}

#else // Non Grass

// NLC .82: 1 near, 0 from 75 m ( tree origin to camera )
float ssfx_wind_tree_fade(const float3x4 m_xform)
{
    return 1.0f - smoothstep(50.0f, 75.0f, distance(float3(m_xform._14, m_xform._24, m_xform._34), eye_position));
}

float3 ssfx_wind_tree_trunk(const float3x4 m_xform, float3 pos, float Tree_H, wind_setup W, const bool for_old_frame = false)
{
    // Phase ( from matrix ) + Offset
    float Phase = m_xform._24 + (for_old_frame ? ssfx_wind_anim_old.z : ssfx_wind_anim.z) * W.trees_trunk_animspeed;

    // Trunk wave
    float TWave = (cos(Phase) * sin(Phase * 5.0f) + 0.5f) * W.trees_bend;

    // Wind speed
    float WSpeed = saturate(W.sqrt_speed * 1.5f);

    // Base wind speed displacement
    // NLC .80: 0.006 -> 0.01, trunks visibly sway with the branches in light wind ( fir trunks looked static )
    float Base_Bend = WSpeed * 0.01f * saturate(1.0f - Tree_H * 0.005f);

    // Intensity ( Squared height )
    Base_Bend *= Tree_H * Tree_H * TWave * WSpeed;

#ifdef SSFX_WIND_GUSTS
    Base_Bend *= ssfx_wind_gust_at(float2(m_xform._14, m_xform._34), W.speed, W.sqrt_speed, for_old_frame);
#endif

    // NLC .82: wind fades out before the distance where trees switch to their static LOD ( ~80 m ), so the swap no
    // longer pops a swaying tree into a still one
    Base_Bend *= ssfx_wind_tree_fade(m_xform);

    // Apply direction
    float2 Final = Base_Bend.xx * W.direction;

    return float3(Final, saturate((TWave + 1.0f) * 0.5));
}

float3 ssfx_wind_tree_branches(const float3x4 m_xform, float3 pos, float Tree_H, float tc_y, wind_setup W, const bool for_old_frame = false)
{
    // UV Offset
    float2 Offset = -(for_old_frame ? ssfx_wind_anim_old.xy : ssfx_wind_anim.xy) * W.trees_animspeed;

    // Sample flow map
    float3 Flow = s_waves.SampleLevel(smp_linear2, (pos.xz + Offset) * 0.02f, 0);

    // Sample 2, slower and detailed
    float3 Flow2 = s_waves.SampleLevel(smp_linear2, (pos.xz + Offset * 0.2f) * 0.1f, 0);

    // Branch motion [ -1.0 ~ 1.0 ]
    float3 branchMotion = 0;

    //if (!is_bugged_flora)
        branchMotion = float3(Flow.x, Flow2.y, Flow.y) * 2.0f - 1.0f;

    // Trunk position
    float3 Trunk = ssfx_wind_tree_trunk(m_xform, pos, Tree_H, W, for_old_frame);

    // Gust from trunk data.
    branchMotion.xz *= Trunk.z * clamp(Tree_H * 0.1f, 1.0f, 2.5f);

    // Add wind direction
    //if (!is_bugged_flora)
        branchMotion.xz += Flow2.z * W.direction;

    // Add wind gust
    branchMotion.y *= saturate(Tree_H * 0.1f);

    // Everything is limited by the UV and wind speed
    //if (!is_bugged_flora)
        branchMotion *= (1.0f - tc_y) * W.speed;

#ifdef SSFX_WIND_GUSTS
    // Trunk already carries the gust ( ssfx_wind_tree_trunk ), so trunk and branches stay attached
    branchMotion *= ssfx_wind_gust_at(float2(m_xform._14, m_xform._34), W.speed, W.sqrt_speed, for_old_frame);
#endif

    branchMotion *= ssfx_wind_tree_fade(m_xform); // NLC .82: see ssfx_wind_tree_trunk

    // Add trunk animation ( already faded )
    branchMotion.xz += Trunk.xy;

    return branchMotion;
}

#endif