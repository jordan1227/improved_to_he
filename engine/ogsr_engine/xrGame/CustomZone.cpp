#include "stdafx.h"
#include "..\xr_3da\XR_IOConsole.h"
#include "customzone.h"
#include "hit.h"
#include "PHDestroyable.h"
#include "actor.h"
#include "hudmanager.h"
#include "ParticlesObject.h"
#include "xrserver_objects_alife_monsters.h"
#include "../xr_3da/LightAnimLibrary.h"
#include "level.h"
#include "game_cl_base.h"
#include "..\xr_3da\IGame_Persistent.h"
#include "artifact.h"
#include "ai_object_location.h"
#include "../Include/xrRender/Kinematics.h"
#include "zone_effector.h"
#include "breakableobject.h"
#include "script_callback_ex.h"
#include "game_object_space.h"
#include "script_game_object.h"
#include "Level_Bullet_Manager.h" // NLC flytrap
#include "PhysicsShell.h"
#include "PhysicsShellHolder.h"
#include "Missile.h"
#include "entity_alive.h"
#include "ai/monsters/basemonster/base_monster.h"
#include "ai/monsters/anomaly_detector.h"
#include "PHMovementControl.h"
#include "CharacterPhysicsSupport.h"
#include "player_hud.h" // g_nlc_actor_field_speed_k
#include "PHCapture.h" // NLC flytrap
#include "PHWorld.h"

//////////////////////////////////////////////////////////////////////////
#define PREFETCHED_ARTEFACTS_NUM 1 //количество предварительно проспавненых артефактов
#define WIND_RADIUS (4 * Radius()) //расстояние до актера, когда появляется ветер
#define FASTMODE_DISTANCE (50.f) // distance to camera from sphere, when zone switches to fast update sequence

extern Fvector4 ps_ssfx_int_grass_params_1;

CCustomZone::CCustomZone(void)
{
    m_fMaxPower = 100.f;
    m_fAttenuation = 1.f;
    m_dwPeriod = 1100;
    m_fEffectiveRadius = 0.75f;
    m_bZoneActive = false;
    m_eHitTypeBlowout = ALife::eHitTypeWound;
    m_pLocalActor = NULL;
    m_pIdleParticles = NULL;
    m_pLight = NULL;
    m_pIdleLight = NULL;
    m_pIdleLAnim = NULL;

    m_StateTime.resize(eZoneStateMax);
    for (int i = 0; i < eZoneStateMax; i++)
        m_StateTime[i] = 0;

    m_dwAffectFrameNum = 0;
    m_fArtefactSpawnProbability = 0.f;
    m_fThrowOutPower = 0.f;
    m_fArtefactSpawnHeight = 0.f;
    m_fBlowoutWindPowerMax = m_fStoreWindPower = 0.f;
    m_fDistanceToCurEntity = flt_max;
    m_ef_weapon_type = u32(-1);
    m_owner_id = u32(-1);

    m_effector = NULL;
    m_bIdleObjectParticlesDontStop = FALSE;
    m_b_always_fastmode = false;

    m_bBornOnBlowoutFlag = false;
    m_keep_update = false;
}

CCustomZone::~CCustomZone(void)
{
    if (grassbender_id)
        g_pGamePersistent->GrassBendersRemoveByIndex(grassbender_id);
    m_idle_sound.destroy();
    m_accum_sound.destroy();
    m_awaking_sound.destroy();
    m_blowout_sound.destroy();
    m_hit_sound.destroy();
    m_entrance_sound.destroy();
    m_ArtefactBornSound.destroy();

    xr_delete(m_effector);
}

void CCustomZone::Load(LPCSTR section)
{
    inherited::Load(section);

    m_iDisableHitTime = pSettings->r_s32(section, "disable_time");
    m_iDisableHitTimeSmall = pSettings->r_s32(section, "disable_time_small");
    m_iDisableIdleTime = pSettings->r_s32(section, "disable_idle_time");
    m_fHitImpulseScale = pSettings->r_float(section, "hit_impulse_scale");
    m_fEffectiveRadius = pSettings->r_float(section, "effective_radius");
    m_eHitTypeBlowout = ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type"));

    IgnoreAny = READ_IF_EXISTS(pSettings, r_bool, section, "ignore_any", false);
    IgnoreAnomalyDetector = READ_IF_EXISTS(pSettings, r_bool, section, "ignore_anomaly_detector", false);
    IgnoreNonAlive = pSettings->r_bool(section, "ignore_nonalive");
    IgnoreSmall = pSettings->r_bool(section, "ignore_small");
    IgnoreArtefact = pSettings->r_bool(section, "ignore_artefacts");
    VisibleDetector = pSettings->r_bool(section, "visible_by_detector");

    // NLC flytrap (docs/ANOMALY_DESIGN.md 3)
    m_nlc_detect_k = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "monster_detect_k", 1.f), 0.f, 1.f);
    m_nlc_follow_enemy = READ_IF_EXISTS(pSettings, r_bool, section, "monster_follow_enemy", false);
    m_nlc_soft_object_particles = READ_IF_EXISTS(pSettings, r_bool, section, "object_particles_soft_stop", false);
    m_nlc_particles_log = READ_IF_EXISTS(pSettings, r_bool, section, "object_particles_log", false);
    m_nlc_flytrap = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_flytrap", false);
    if (m_nlc_flytrap)
    {
        m_nlc_ft_bullet_cap = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_bullet_speed", 8.f), 1.f);
        m_nlc_ft_bullet_ramp = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_bullet_ramp", 1.5f), 0.f);
        m_nlc_ft_slow_rim = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_slow_rim", 0.3f), 0.f, 0.9f);
        m_nlc_ft_slow_core = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_slow_core", 0.7f), 0.f, 0.9f);
        m_nlc_ft_ramp_k = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_ramp_k", 0.6f), 0.05f, 1.f);
        // objects: by default the same time factor as the monsters' clips (1 - slow)
        m_nlc_ft_obj_time_rim = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_time_rim", 1.f - m_nlc_ft_slow_rim), 0.01f, 1.f);
        m_nlc_ft_obj_time_core = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_time_core", 1.f - m_nlc_ft_slow_core), 0.01f, 1.f);
        m_nlc_ft_obj_hit_exp = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_hit_k_exp", 1.f), 0.f, 1.f);
        m_nlc_ft_obj_ramp = READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_ramp", 1.f); // m; 0 = the monsters' ramp
        m_nlc_ft_obj_gravity_k = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_gravity_k", 1.f), 0.f, 4.f);
        m_nlc_ft_obj_exit_speed = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_exit_speed", 20.f), 1.f);
        m_nlc_ft_ragdoll_exit_speed = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_ragdoll_exit_speed", 8.f), 0.5f);
        m_nlc_ft_obj_exit_spin = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_object_exit_spin", 20.f), 1.f);
        m_nlc_ft_monster_cap_rim = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_monster_speed_rim", 4.5f), 0.3f);
        m_nlc_ft_monster_cap_core = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_monster_speed_core", 1.8f), 0.3f);
        m_nlc_ft_actor_air = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_actor_air_speed", 5.f), 0.5f);
        m_nlc_ft_learn_radius = std::max(READ_IF_EXISTS(pSettings, r_float, section, "flytrap_learn_radius", 25.f), 0.f);
        m_nlc_ft_learn_ms = READ_IF_EXISTS(pSettings, r_u32, section, "flytrap_learn_ms", 700);
        m_nlc_ft_log = READ_IF_EXISTS(pSettings, r_bool, section, "flytrap_log", false);
    }
    // NLC silence (docs/ANOMALY_DESIGN.md 16)
    m_nlc_silence = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_silence", false);
    if (m_nlc_silence)
    {
        m_nlc_silence_rim = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "silence_ai_rim", 0.7f), 0.f, 1.f);
        m_nlc_silence_core = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "silence_ai_core", 1.f), 0.f, 1.f);
        m_nlc_silence_ramp_k = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "silence_ramp_k", 0.5f), 0.05f, 1.f);
        m_nlc_silence_log = READ_IF_EXISTS(pSettings, r_bool, section, "silence_log", false);
    }

    // bak
    BirthOnNonAlive = READ_IF_EXISTS(pSettings, r_bool, section, "birth_on_nonalive", false);
    BirthOnAlive = READ_IF_EXISTS(pSettings, r_bool, section, "birth_on_alive", false);
    BirthOnDead = READ_IF_EXISTS(pSettings, r_bool, section, "birth_on_dead", false);

    //загрузить времена для зоны
    m_StateTime[eZoneStateIdle] = -1;
    m_StateTime[eZoneStateAwaking] = pSettings->r_s32(section, "awaking_time");
    m_StateTime[eZoneStateBlowout] = pSettings->r_s32(section, "blowout_time");
    m_StateTime[eZoneStateAccumulate] = pSettings->r_s32(section, "accamulate_time");

    //////////////////////////////////////////////////////////////////////////
    ISpatial* self = smart_cast<ISpatial*>(this);
    if (self)
        self->spatial.type |= (STYPE_COLLIDEABLE | STYPE_SHAPE);
    //////////////////////////////////////////////////////////////////////////

    // -- Interactive Grass - IDLE
    m_BendGrass_idle_anim = READ_IF_EXISTS(pSettings, r_s8, section, "bend_grass_idle_anim", -1);

    m_BendGrass_idle_str = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_idle_str", 1.f);

    m_BendGrass_idle_radius = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_idle_radius", 1.f);

    m_BendGrass_idle_speed = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_idle_speed", 1.f);

    // -- Interactive Grass - ACTIVE
    m_BendGrass_whenactive_anim = READ_IF_EXISTS(pSettings, r_s8, section, "bend_grass_whenactive_anim", -1);

    m_BendGrass_whenactive_speed = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_whenactive_speed", -1.f);

    m_BendGrass_whenactive_str = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_whenactive_str", -1.f);

    // -- Interactive Grass - BLOWOUT
    m_BendGrass_Blowout_time = READ_IF_EXISTS(pSettings, r_u32, section, "bend_grass_blowout_duration", -1);

    m_BendGrass_Blowout = READ_IF_EXISTS(pSettings, r_bool, section, "bend_grass_blowout", false);

    m_BendGrass_Blowout_speed = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_blowout_speed", 0.f);

    m_BendGrass_Blowout_radius = READ_IF_EXISTS(pSettings, r_float, section, "bend_grass_blowout_radius", 0.f);
    // --

    LPCSTR sound_str = NULL;

    if (pSettings->line_exist(section, "idle_sound"))
    {
        sound_str = pSettings->r_string(section, "idle_sound");
        m_idle_sound.create(sound_str, st_Effect, sg_SourceType);
    }

    if (pSettings->line_exist(section, "accum_sound"))
    {
        sound_str = pSettings->r_string(section, "accum_sound");
        m_accum_sound.create(sound_str, st_Effect, sg_SourceType);
    }
    if (pSettings->line_exist(section, "awake_sound"))
    {
        sound_str = pSettings->r_string(section, "awake_sound");
        m_awaking_sound.create(sound_str, st_Effect, sg_SourceType);
    }

    if (pSettings->line_exist(section, "blowout_sound"))
    {
        sound_str = pSettings->r_string(section, "blowout_sound");
        m_blowout_sound.create(sound_str, st_Effect, sg_SourceType);
    }

    if (pSettings->line_exist(section, "hit_sound"))
    {
        sound_str = pSettings->r_string(section, "hit_sound");
        m_hit_sound.create(sound_str, st_Effect, sg_SourceType);
    }

    if (pSettings->line_exist(section, "entrance_sound"))
    {
        sound_str = pSettings->r_string(section, "entrance_sound");
        m_entrance_sound.create(sound_str, st_Effect, sg_SourceType);
    }

    if (pSettings->line_exist(section, "idle_particles"))
        m_sIdleParticles = pSettings->r_string(section, "idle_particles");
    if (pSettings->line_exist(section, "blowout_particles"))
        m_sBlowoutParticles = pSettings->r_string(section, "blowout_particles");

    if (pSettings->line_exist(section, "accum_particles"))
        m_sAccumParticles = pSettings->r_string(section, "accum_particles");

    if (pSettings->line_exist(section, "awake_particles"))
        m_sAwakingParticles = pSettings->r_string(section, "awake_particles");

    if (pSettings->line_exist(section, "entrance_small_particles"))
        m_sEntranceParticlesSmall = pSettings->r_string(section, "entrance_small_particles");
    if (pSettings->line_exist(section, "entrance_big_particles"))
        m_sEntranceParticlesBig = pSettings->r_string(section, "entrance_big_particles");

    if (pSettings->line_exist(section, "hit_small_particles"))
        m_sHitParticlesSmall = pSettings->r_string(section, "hit_small_particles");
    if (pSettings->line_exist(section, "hit_big_particles"))
        m_sHitParticlesBig = pSettings->r_string(section, "hit_big_particles");

    if (pSettings->line_exist(section, "idle_small_particles"))
        m_sIdleObjectParticlesBig = pSettings->r_string(section, "idle_big_particles");
    if (pSettings->line_exist(section, "idle_big_particles"))
        m_sIdleObjectParticlesSmall = pSettings->r_string(section, "idle_small_particles");
    if (pSettings->line_exist(section, "idle_particles_dont_stop"))
        m_bIdleObjectParticlesDontStop = pSettings->r_bool(section, "idle_particles_dont_stop");

    if (pSettings->line_exist(section, "postprocess"))
    {
        m_effector = xr_new<CZoneEffector>();
        m_effector->Load(pSettings->r_string(section, "postprocess"));
    }

    if (pSettings->line_exist(section, "blowout_particles_time"))
    {
        m_dwBlowoutParticlesTime = pSettings->r_u32(section, "blowout_particles_time");
        if (s32(m_dwBlowoutParticlesTime) > m_StateTime[eZoneStateBlowout])
        {
            m_dwBlowoutParticlesTime = m_StateTime[eZoneStateBlowout];
            Msg("! ERROR: invalid 'blowout_particles_time' in '%s'", section);
        }
    }
    else
        m_dwBlowoutParticlesTime = 0;

    if (pSettings->line_exist(section, "blowout_light_time"))
    {
        m_dwBlowoutLightTime = pSettings->r_u32(section, "blowout_light_time");
        if (s32(m_dwBlowoutLightTime) > m_StateTime[eZoneStateBlowout])
        {
            m_dwBlowoutLightTime = m_StateTime[eZoneStateBlowout];
            Msg("! ERROR: invalid 'blowout_light_time' in '%s'", section);
        }
    }
    else
        m_dwBlowoutLightTime = 0;

    if (pSettings->line_exist(section, "blowout_sound_time"))
    {
        m_dwBlowoutSoundTime = pSettings->r_u32(section, "blowout_sound_time");
        if (s32(m_dwBlowoutSoundTime) > m_StateTime[eZoneStateBlowout])
        {
            m_dwBlowoutSoundTime = m_StateTime[eZoneStateBlowout];
            Msg("! ERROR: invalid 'blowout_sound_time' in '%s'", section);
        }
    }
    else
        m_dwBlowoutSoundTime = 0;

    if (pSettings->line_exist(section, "blowout_explosion_time"))
    {
        m_dwBlowoutExplosionTime = pSettings->r_u32(section, "blowout_explosion_time");
        if (s32(m_dwBlowoutExplosionTime) > m_StateTime[eZoneStateBlowout])
        {
            m_dwBlowoutExplosionTime = m_StateTime[eZoneStateBlowout];
            Msg("! ERROR: invalid 'blowout_explosion_time' in '%s'", section);
        }
    }
    else
        m_dwBlowoutExplosionTime = 0;

    BlowoutWind = pSettings->r_bool(section, "blowout_wind");
    if (BlowoutWind)
    {
        m_dwBlowoutWindTimeStart = pSettings->r_u32(section, "blowout_wind_time_start");
        m_dwBlowoutWindTimePeak = pSettings->r_u32(section, "blowout_wind_time_peak");
        m_dwBlowoutWindTimeEnd = pSettings->r_u32(section, "blowout_wind_time_end");
        R_ASSERT(m_dwBlowoutWindTimeStart < m_dwBlowoutWindTimePeak);
        R_ASSERT(m_dwBlowoutWindTimePeak < m_dwBlowoutWindTimeEnd);

        if ((s32)m_dwBlowoutWindTimeEnd < m_StateTime[eZoneStateBlowout])
        {
            m_dwBlowoutWindTimeEnd = u32(m_StateTime[eZoneStateBlowout] - 1);
            Msg("! ERROR: invalid 'blowout_wind_time_end' in '%s'", section);
        }

        m_fBlowoutWindPowerMax = pSettings->r_float(section, "blowout_wind_power");
    }

    //загрузить параметры световой вспышки от взрыва
    BlowoutLight = pSettings->r_bool(section, "blowout_light");
    BlowoutLightShadow = READ_IF_EXISTS(pSettings, r_bool, section, "blowout_light_shadow", true);

    if (BlowoutLight)
    {
        sscanf(pSettings->r_string(section, "light_color"), "%f,%f,%f", &m_LightColor.r, &m_LightColor.g, &m_LightColor.b);
        m_fLightRange = pSettings->r_float(section, "light_range");
        m_fLightTime = pSettings->r_float(section, "light_time");
        m_fLightTimeLeft = 0;

        m_fLightHeight = pSettings->r_float(section, "light_height");

        BlowLightVolumetric = READ_IF_EXISTS(pSettings, r_bool, section, "light_volumetric_enable", false);
        if (BlowLightVolumetric)
        {
            BlowLightVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "light_volumetric_intensity", 0.25f);
            BlowLightVolumetricIntensity = std::clamp(BlowLightVolumetricIntensity, 0.f, 1.f);

            BlowLightVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "light_volumetric_distance", 1.f);
            BlowLightVolumetricDistance = std::clamp(BlowLightVolumetricDistance, 0.f, 1.f);
        }
    }

    //загрузить параметры idle подсветки
    IdleLight = pSettings->r_bool(section, "idle_light");
    if (IdleLight)
    {
        m_fIdleLightRange = pSettings->r_float(section, "idle_light_range");
        m_fIdleLightRangeDelta = pSettings->r_float(section, "idle_light_range_delta");
        LPCSTR light_anim = pSettings->r_string(section, "idle_light_anim");
        m_pIdleLAnim = LALib.FindItem(light_anim);
        m_fIdleLightHeight = pSettings->r_float(section, "idle_light_height");

        IdleLightVolumetric = READ_IF_EXISTS(pSettings, r_bool, section, "idle_light_volumetric_enable", false);
        if (IdleLightVolumetric)
        {
            IdleLightVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "idle_light_volumetric_intensity", 0.25f);
            IdleLightVolumetricIntensity = std::clamp(IdleLightVolumetricIntensity, 0.f, 1.f);

            IdleLightVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "idle_light_volumetric_distance", 1.0f);
            IdleLightVolumetricDistance = std::clamp(IdleLightVolumetricDistance, 0.f, 1.f);
        }

        IdleLightShadow = READ_IF_EXISTS(pSettings, r_bool, section, "idle_light_shadow", true);
    }

    //загрузить параметры для разбрасывания артефактов
    SpawnBlowoutArtefacts = pSettings->r_bool(section, "spawn_blowout_artefacts");
    if (SpawnBlowoutArtefacts)
    {
        m_fArtefactSpawnProbability = pSettings->r_float(section, "artefact_spawn_probability");
        m_fArtefactSpawnOnDeathProbability = READ_IF_EXISTS(pSettings, r_float, section, "birth_on_death_probability", 0.0f);
        if (pSettings->line_exist(section, "artefact_spawn_particles"))
            m_sArtefactSpawnParticles = pSettings->r_string(section, "artefact_spawn_particles");
        else
            m_sArtefactSpawnParticles = NULL;

        if (pSettings->line_exist(section, "artefact_born_sound"))
        {
            sound_str = pSettings->r_string(section, "artefact_born_sound");
            m_ArtefactBornSound.create(sound_str, st_Effect, sg_SourceType);
        }

        m_fThrowOutPower = pSettings->r_float(section, "throw_out_power");
        m_fArtefactSpawnHeight = pSettings->r_float(section, "artefact_spawn_height");

        LPCSTR l_caParameters = pSettings->r_string(section, "artefacts");

        u16 m_wItemCount = (u16)_GetItemCount(l_caParameters);
        R_ASSERT2(!(m_wItemCount & 1), "Invalid number of parameters in string 'artefacts' in the 'system.ltx'!");
        m_wItemCount >>= 1;

        m_ArtefactSpawn.clear();
        string512 l_caBuffer;

        float total_probability = 0.f;

        m_ArtefactSpawn.resize(m_wItemCount);
        for (u16 i = 0; i < m_wItemCount; ++i)
        {
            ARTEFACT_SPAWN& artefact_spawn = m_ArtefactSpawn[i];
            artefact_spawn.section = _GetItem(l_caParameters, i << 1, l_caBuffer);
            artefact_spawn.probability = (float)atof(_GetItem(l_caParameters, (i << 1) | 1, l_caBuffer));
            total_probability += artefact_spawn.probability;
        }

        R_ASSERT3(!fis_zero(total_probability), "The probability of artefact spawn is zero!", *cName());
        //нормализировать вероятности
        for (u32 i = 0; i < m_ArtefactSpawn.size(); ++i)
        {
            m_ArtefactSpawn[i].probability = m_ArtefactSpawn[i].probability / total_probability;
        }
    }

    m_ef_anomaly_type = pSettings->r_u32(section, "ef_anomaly_type");
    m_ef_weapon_type = pSettings->r_u32(section, "ef_weapon_type");

    DestroyAfterBlowout = READ_IF_EXISTS(pSettings, r_bool, section, "DestroyAfterBlowout", false);

    m_b_always_fastmode = READ_IF_EXISTS(pSettings, r_bool, section, "always_fast_mode", false);
}

BOOL CCustomZone::net_Spawn(CSE_Abstract* DC)
{
    if (!inherited::net_Spawn(DC))
        return (FALSE);

    CSE_Abstract* e = (CSE_Abstract*)(DC);
    CSE_ALifeCustomZone* Z = smart_cast<CSE_ALifeCustomZone*>(e);
    VERIFY(Z);

    m_fMaxPower = pSettings->r_float(cNameSect(), "max_start_power");
    m_fAttenuation = pSettings->r_float(cNameSect(), "attenuation");
    m_dwPeriod = pSettings->r_u32(cNameSect(), "period");
    m_owner_id = Z->m_owner_id;
    if (m_owner_id != u32(-1))
        m_ttl = Device.dwTimeGlobal + 40000; // 40 sec
    else
        m_ttl = u32(-1);

    m_TimeToDisable = Z->m_disabled_time * 1000;
    m_TimeToEnable = Z->m_enabled_time * 1000;
    m_TimeShift = Z->m_start_time_shift * 1000;
    m_StartTime = Device.dwTimeGlobal;
    UseOnOffTime = (m_TimeToDisable != 0) && (m_TimeToEnable != 0);

    //добавить источники света
    if (IdleLight)
    {
        m_pIdleLight = ::Render->light_create();
        m_pIdleLight->set_shadow(IdleLightShadow);

        if (IdleLightVolumetric)
        {
            m_pIdleLight->set_volumetric(true);
            m_pIdleLight->set_volumetric_intensity(IdleLightVolumetricIntensity);
            m_pIdleLight->set_volumetric_distance(IdleLightVolumetricDistance);
        }
    }
    else
        m_pIdleLight = NULL;

    if (BlowoutLight)
    {
        m_pLight = ::Render->light_create();
        m_pLight->set_shadow(BlowoutLightShadow);

        if (BlowLightVolumetric)
        {
            m_pLight->set_volumetric(true);
            m_pLight->set_volumetric_intensity(BlowLightVolumetricIntensity);
            m_pLight->set_volumetric_distance(BlowLightVolumetricDistance);
        }
    }
    else
        m_pLight = NULL;

    setEnabled(TRUE);

    PlayIdleParticles();

    m_eZoneState = eZoneStateIdle;
    m_iPreviousStateTime = m_iStateTime = 0;

    if (m_effector)
        m_effector->SetRadius(CFORM()->getSphere().R);

    m_dwLastTimeMoved = Device.dwTimeGlobal;
    m_vPrevPos.set(Position());

    m_fDistanceToCurEntity = flt_max;
    m_bBlowoutWindActive = false;

    o_fastmode = true; // start initially with fast-mode enabled
    if (spawn_ini() && spawn_ini()->line_exist("fast_mode", "always_fast"))
    {
        m_b_always_fastmode = spawn_ini()->r_bool("fast_mode", "always_fast");
    }
    return (TRUE);
}

void CCustomZone::net_Destroy()
{
    nlc_flytrap_stasis(false); // NLC
    nlc_silence_register(false); // NLC

    StopIdleParticles();

    inherited::net_Destroy();

    StopWind();

    m_pLight.destroy();
    m_pIdleLight.destroy();

    CParticlesObject::Destroy(m_pIdleParticles);

    if (m_effector)
        m_effector->Stop();
    //---------------------------------------------
    OBJECT_INFO_VEC_IT i = m_ObjectInfoMap.begin(), e = m_ObjectInfoMap.end();
    for (; e != i; ++i)
        exit_Zone(*i);
    m_ObjectInfoMap.clear();
}

void CCustomZone::net_Export(CSE_Abstract* E) { inherited::net_Export(E); }

bool CCustomZone::IdleState()
{
    UpdateOnOffState();

    return false;
}

bool CCustomZone::AwakingState()
{
    if (m_iStateTime >= m_StateTime[eZoneStateAwaking])
    {
        SwitchZoneState(eZoneStateBlowout);
        return true;
    }
    return false;
}

bool CCustomZone::BlowoutState()
{
    if (m_iStateTime >= m_StateTime[eZoneStateBlowout])
    {
        SwitchZoneState(eZoneStateAccumulate);
        return true;
    }
    return false;
}
bool CCustomZone::AccumulateState()
{
    if (m_iStateTime >= m_StateTime[eZoneStateAccumulate])
    {
        if (m_bZoneActive)
            SwitchZoneState(eZoneStateBlowout);
        else
            SwitchZoneState(eZoneStateIdle);

        return true;
    }
    return false;
}

void CCustomZone::UpdateWorkload(u32 dt)
{
    m_iPreviousStateTime = m_iStateTime;
    m_iStateTime += (int)dt;

    if (m_nlc_flytrap)
        nlc_flytrap_update(dt); // NLC
    if (m_nlc_silence)
        nlc_silence_register(IsEnabled()); // NLC: every update (the registry is cleared with the level)

    if (!IsEnabled())
    {
        if (m_effector)
            m_effector->Stop();
        // KRodin: чуть переделал фикс неотключения света после отключения аномалии. Это более оптимальный вариант, на мой взгляд.
        if (m_pIdleLight && m_pIdleLight->get_active())
            StopIdleLight();
        if (m_pLight && m_pLight->get_active())
            StopBlowoutLight();
        return;
    }

    UpdateIdleLight();

    switch (m_eZoneState)
    {
    case eZoneStateIdle: IdleState(); break;
    case eZoneStateAwaking: AwakingState(); break;
    case eZoneStateBlowout: BlowoutState(); break;
    case eZoneStateAccumulate: AccumulateState(); break;
    case eZoneStateDisabled: break;
    default: NODEFAULT;
    }

    //вычислить время срабатывания зоны
    if (m_bZoneActive)
        m_dwDeltaTime += dt;
    else
        m_dwDeltaTime = 0;

    if (m_dwDeltaTime > m_dwPeriod)
    {
        m_dwDeltaTime = m_dwPeriod;
    }

    if (Level().CurrentEntity())
    {
        m_fDistanceToCurEntity = Level().CurrentEntity()->Position().distance_to(Position());

        if (m_effector)
            m_effector->Update(m_fDistanceToCurEntity);
    }

    if (m_pLight && m_pLight->get_active())
        UpdateBlowoutLight();
}

// called only in "fast-mode"
void CCustomZone::UpdateCL()
{
    inherited::UpdateCL();
    if (o_fastmode)
        UpdateWorkload(Device.dwTimeDelta);
}

// called as usual
void CCustomZone::shedule_Update(u32 dt)
{
    m_bZoneActive = false;

    if (IsEnabled())
    {
        const Fsphere& s = CFORM()->getSphere();
        Fvector P;
        XFORM().transform_tiny(P, s.P);

        // update
        feel_touch_update(P, s.R);

        //пройтись по всем объектам в зоне
        //и проверить их состояние
        for (OBJECT_INFO_VEC_IT it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it)
        {
            CObject* pObject = (*it).object;
            if (!pObject)
                continue;
            CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(pObject);
            SZoneObjectInfo& info = (*it);

            info.time_in_zone += dt;

            if (pEntityAlive && !pEntityAlive->g_Alive() && info.death_in_zone != true)
            {
                info.death_in_zone = true;
                m_bBornOnBlowoutFlag = true;
            }
            if ((!info.small_object && m_iDisableHitTime != -1 && (int)info.time_in_zone > m_iDisableHitTime) ||
                (info.small_object && m_iDisableHitTimeSmall != -1 && (int)info.time_in_zone > m_iDisableHitTimeSmall))
            {
                if (!pEntityAlive || !pEntityAlive->g_Alive())
                    info.zone_ignore = true;
            }
            if (m_iDisableIdleTime != -1 && (int)info.time_in_zone > m_iDisableIdleTime)
            {
                CPhysicsShellHolder* holder = smart_cast<CPhysicsShellHolder*>(pObject);
                if ((!pEntityAlive || !pEntityAlive->g_Alive()) && holder)
                    StopObjectIdleParticles(holder);
            }

            //если есть хотя бы один не дисабленый объект, то
            //зона считается активной
            if (info.zone_ignore == false)
                m_bZoneActive = true;
        }

        if (eZoneStateIdle == m_eZoneState)
            CheckForAwaking();

        inherited::shedule_Update(dt);

        // check "fast-mode" border
        float cam_distance = Device.vCameraPosition.distance_to(P) - s.R;
        if (cam_distance > FASTMODE_DISTANCE && !m_b_always_fastmode)
            o_switch_2_slow();
        else
            o_switch_2_fast();

        if (!o_fastmode)
            UpdateWorkload(dt);

        if (cam_distance < ps_ssfx_int_grass_params_1.w)
            GrassZoneUpdate();
        else
        {
            // Out of range, fadeOut if a grassbender_id is assigned
            if (grassbender_id)
            {
                // If the ID doesn't match... Just remove the grassbender_id.
                if (grass_shader_data.id[grassbender_id] == ID())
                {
                    grass_shader_data.str_target[grassbender_id] += g_pGamePersistent->GrassBenderToValue(grass_shader_data.str_target[grassbender_id], 0.0f, 4.0f, false);

                    // Remove ( Don't worry, GrassBenderToValue() it's going to get the == 0 )
                    if (grass_shader_data.str_target[grassbender_id] <= 0.f)
                        g_pGamePersistent->GrassBendersRemoveByIndex(grassbender_id);
                }
                else
                {
                    grassbender_id = 0;
                }
            }
        }

    }
    else if (m_keep_update)
        inherited::shedule_Update(dt);

    UpdateOnOffState();

    if (LastBlowoutTime && (Device.dwTimeGlobal - LastBlowoutTime) > 300)
        DestroyObject();
}

void CCustomZone::CheckForAwaking()
{
    if (m_bZoneActive && eZoneStateIdle == m_eZoneState)
        SwitchZoneState(eZoneStateAwaking);
}

void CCustomZone::feel_touch_new(CObject* O)
{
    if (smart_cast<CActor*>(O) && O == Level().CurrentEntity())
        m_pLocalActor = smart_cast<CActor*>(O);

    CGameObject* pGameObject = smart_cast<CGameObject*>(O);
    CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(pGameObject);
    CArtefact* pArtefact = smart_cast<CArtefact*>(pGameObject);

    SZoneObjectInfo object_info;
    object_info.object = pGameObject;

    if (pEntityAlive)
    {
        if (pEntityAlive->g_Alive())
            object_info.nonalive_object = false;
        else
        {
            object_info.nonalive_object = true;
            object_info.death_in_zone = true;
        }
    }
    else
        object_info.nonalive_object = true;

    if (pGameObject->Radius() < SMALL_OBJECT_RADIUS)
        object_info.small_object = true;
    else
        object_info.small_object = false;

    if (IgnoreAny || (object_info.small_object && IgnoreSmall) || (object_info.nonalive_object && IgnoreNonAlive) || (pArtefact && IgnoreArtefact))
        object_info.zone_ignore = true;
    else
        object_info.zone_ignore = false;

    enter_Zone(object_info);
    m_ObjectInfoMap.push_back(object_info);

    if (IsEnabled())
    {
        PlayEntranceParticles(pGameObject);
        PlayObjectIdleParticles(pGameObject);
    }
};

void CCustomZone::feel_touch_delete(CObject* O)
{
#ifdef DEBUG
    if (bDebug)
        Msg("%s %s", *O->cName(), "leaving a zone.");
#endif

    if (smart_cast<CActor*>(O))
        m_pLocalActor = NULL;
    CGameObject* pGameObject = smart_cast<CGameObject*>(O);
    if (!pGameObject->getDestroy())
    {
        StopObjectIdleParticles(pGameObject);
    }

    OBJECT_INFO_VEC_IT it = std::find(m_ObjectInfoMap.begin(), m_ObjectInfoMap.end(), pGameObject);
    if (it != m_ObjectInfoMap.end())
    {
        exit_Zone(*it);
        m_ObjectInfoMap.erase(it);
    }
}

BOOL CCustomZone::feel_touch_contact(CObject* O)
{
    if (smart_cast<CCustomZone*>(O))
        return FALSE;
    if (smart_cast<CBreakableObject*>(O))
        return FALSE;
    if (0 == smart_cast<IKinematics*>(O->Visual()))
        return FALSE;

    if (O->ID() == ID())
        return (FALSE);

    CGameObject* object = smart_cast<CGameObject*>(O);
    if (!object || !object->IsVisibleForZones())
        return (FALSE);

    if (!((CCF_Shape*)CFORM())->Contact(O))
        return (FALSE);

    return (object->feel_touch_on_contact(this));
}

float CCustomZone::RelativePower(float dist)
{
    float radius = effective_radius();
    float power = radius < dist ? 0 : (1.f - m_fAttenuation * (dist / radius) * (dist / radius));
    return power < 0 ? 0 : power;
}
float CCustomZone::effective_radius() { return Radius() * m_fEffectiveRadius; }

float CCustomZone::distance_to_center(CObject* O)
{
    Fvector P;
    XFORM().transform_tiny(P, CFORM()->getSphere().P);
    return P.distance_to(O->Position());
}
float CCustomZone::Power(float dist) { return m_fMaxPower * RelativePower(dist); }

void CCustomZone::PlayIdleParticles()
{
    m_idle_sound.play_at_pos(0, Position(), true);

    if (*m_sIdleParticles)
    {
        if (!m_pIdleParticles)
        {
            m_pIdleParticles = CParticlesObject::Create(*m_sIdleParticles, FALSE);
            m_pIdleParticles->UpdateParent(XFORM(), {});
        }
        m_pIdleParticles->UpdateParent(XFORM(), {});
        m_pIdleParticles->Play(false);
    }

    StartIdleLight();
}

void CCustomZone::StopIdleParticles()
{
    m_idle_sound.stop();

    if (m_pIdleParticles)
        m_pIdleParticles->Stop(FALSE);

    StopIdleLight();
}

void CCustomZone::StartIdleLight()
{
    if (m_pIdleLight)
    {
        m_pIdleLight->set_range(m_fIdleLightRange);
        Fvector pos = Position();
        pos.y += m_fIdleLightHeight;
        m_pIdleLight->set_position(pos);
        m_pIdleLight->set_active(true);
    }
}
void CCustomZone::StopIdleLight()
{
    if (m_pIdleLight)
        m_pIdleLight->set_active(false);
}
void CCustomZone::UpdateIdleLight()
{
    if (!m_pIdleLight || !m_pIdleLight->get_active())
        return;

    VERIFY(m_pIdleLAnim);

    int frame = 0;
    u32 clr = m_pIdleLAnim->CalculateBGR(Device.fTimeGlobal, frame); // возвращает в формате BGR
    Fcolor fclr;
    fclr.set((float)color_get_B(clr) / 255.f, (float)color_get_G(clr) / 255.f, (float)color_get_R(clr) / 255.f, 1.f);

    float range = m_fIdleLightRange + m_fIdleLightRangeDelta * ::Random.randF(-1.f, 1.f);
    m_pIdleLight->set_range(range);
    m_pIdleLight->set_color(fclr);

    Fvector pos = Position();
    pos.y += m_fIdleLightHeight;
    m_pIdleLight->set_position(pos);
}

void CCustomZone::PlayBlowoutParticles()
{
    if (!m_sBlowoutParticles)
        return;

    CParticlesObject* pParticles;
    pParticles = CParticlesObject::Create(*m_sBlowoutParticles, TRUE);
    pParticles->UpdateParent(XFORM(), {});
    pParticles->Play(false);

    m_fBlowoutTimeLeft = Device.dwTimeGlobal + m_BendGrass_Blowout_time;

    if (DestroyAfterBlowout)
        LastBlowoutTime = Device.dwTimeGlobal;
}

void CCustomZone::PlayHitParticles(CGameObject* pObject)
{
    m_hit_sound.play_at_pos(0, pObject->Position());

    shared_str particle_str = NULL;

    if (pObject->Radius() < SMALL_OBJECT_RADIUS)
    {
        if (!m_sHitParticlesSmall)
            return;
        particle_str = m_sHitParticlesSmall;
    }
    else
    {
        if (!m_sHitParticlesBig)
            return;
        particle_str = m_sHitParticlesBig;
    }

    if (particle_str.size())
    {
        CParticlesPlayer* PP = smart_cast<CParticlesPlayer*>(pObject);
        if (PP)
        {
            u16 play_bone = PP->GetRandomBone();
            if (play_bone != BI_NONE)
                PP->StartParticles(particle_str, play_bone, Fvector().set(0, 1, 0), ID());
        }
    }
}

void CCustomZone::PlayEntranceParticles(CGameObject* pObject)
{
    if (!IsEnabled())
        return;

    m_entrance_sound.play_at_pos(0, pObject->Position());

    shared_str particle_str = NULL;

    if (pObject->Radius() < SMALL_OBJECT_RADIUS)
    {
        if (!m_sEntranceParticlesSmall)
            return;
        particle_str = m_sEntranceParticlesSmall;
    }
    else
    {
        if (!m_sEntranceParticlesBig)
            return;
        particle_str = m_sEntranceParticlesBig;
    }

    Fvector vel;
    CPhysicsShellHolder* shell_holder = smart_cast<CPhysicsShellHolder*>(pObject);
    if (shell_holder)
        shell_holder->PHGetLinearVell(vel);
    else
        vel.set(0, 0, 0);

    //выбрать случайную косточку на объекте
    CParticlesPlayer* PP = smart_cast<CParticlesPlayer*>(pObject);
    if (PP)
    {
        u16 play_bone = PP->GetRandomBone();
        if (play_bone != BI_NONE)
        {
            CParticlesObject* pParticles = CParticlesObject::Create(*particle_str, TRUE);
            Fmatrix xform;

            Fvector dir;
            if (fis_zero(vel.magnitude()))
                dir.set(0, 1, 0);
            else
            {
                dir.set(vel);
                dir.normalize();
            }

            PP->MakeXFORM(pObject, play_bone, dir, Fvector().set(0, 0, 0), xform);
            pParticles->UpdateParent(xform, vel);
            {
                pParticles->Play(false);
                //. <-->
                //. PP->StartParticles (particle_str, play_bone, dir, ID());
            }
        }
    }
}

void CCustomZone::PlayBulletParticles(Fvector& pos)
{
    m_entrance_sound.play_at_pos(0, pos);

    if (!m_sEntranceParticlesSmall)
        return;

    CParticlesObject* pParticles;
    pParticles = CParticlesObject::Create(*m_sEntranceParticlesSmall, TRUE);

    Fmatrix M;
    M = XFORM();
    M.c.set(pos);

    pParticles->UpdateParent(M, {});
    pParticles->Play(false);
}

void CCustomZone::PlayObjectIdleParticles(CGameObject* pObject)
{
    CParticlesPlayer* PP = smart_cast<CParticlesPlayer*>(pObject);
    if (!PP)
        return;

    shared_str particle_str = NULL;

    //разные партиклы для объектов разного размера
    if (pObject->Radius() < SMALL_OBJECT_RADIUS)
    {
        if (!m_sIdleObjectParticlesSmall)
            return;
        particle_str = m_sIdleObjectParticlesSmall;
    }
    else
    {
        if (!m_sIdleObjectParticlesBig)
            return;
        particle_str = m_sIdleObjectParticlesBig;
    }

    //запустить партиклы на объекте
    //. new
    PP->StopParticles(particle_str, BI_NONE, true);

    PP->StartParticles(particle_str, Fvector().set(0, 1, 0), ID());
    if (!IsEnabled())
        PP->StopParticles(particle_str, BI_NONE, true);
}

void CCustomZone::StopObjectIdleParticles(CGameObject* pObject)
{
    //. new
    if (m_bIdleObjectParticlesDontStop && !pObject->cast_actor())
        return;

    CParticlesPlayer* PP = smart_cast<CParticlesPlayer*>(pObject);
    if (!PP)
        return;

    OBJECT_INFO_VEC_IT it = std::find(m_ObjectInfoMap.begin(), m_ObjectInfoMap.end(), pObject);
    if (m_ObjectInfoMap.end() == it)
        return;

    shared_str particle_str = NULL;
    //разные партиклы для объектов разного размера
    if (pObject->Radius() < SMALL_OBJECT_RADIUS)
    {
        if (!m_sIdleObjectParticlesSmall)
            return;
        particle_str = m_sIdleObjectParticlesSmall;
    }
    else
    {
        if (!m_sIdleObjectParticlesBig)
            return;
        particle_str = m_sIdleObjectParticlesBig;
    }

    // NLC: object_particles_soft_stop lets the emitted particles live out instead of vanishing with the emitter
    PP->StopParticles(particle_str, BI_NONE, !m_nlc_soft_object_particles);
    if (m_nlc_particles_log)
        Msg("~ [zone_particles] [%s]: idle [%s] stopped on [%s] (soft %d)", cName().c_str(), particle_str.c_str(), pObject->cName().c_str(), m_nlc_soft_object_particles);
}

void CCustomZone::Hit(SHit* pHDS)
{
    Fmatrix M;
    M.identity();
    M.translate_over(pHDS->p_in_bone_space);
    M.mulA_43(XFORM());
    PlayBulletParticles(M.c);
}

void CCustomZone::StartBlowoutLight()
{
    if (!m_pLight || m_fLightTime <= 0.f)
        return;

    m_fLightTimeLeft = m_fLightTime;

    m_pLight->set_color(m_LightColor.r, m_LightColor.g, m_LightColor.b);
    m_pLight->set_range(m_fLightRange);

    Fvector pos = Position();
    pos.y += m_fLightHeight;
    m_pLight->set_position(pos);
    m_pLight->set_active(true);
}

void CCustomZone::StopBlowoutLight()
{
    m_fLightTimeLeft = 0.f;
    m_pLight->set_active(false);
}

void CCustomZone::UpdateBlowoutLight()
{
    if (m_fLightTimeLeft > 0)
    {
        m_fLightTimeLeft -= Device.fTimeDelta;
        clamp(m_fLightTimeLeft, 0.0f, m_fLightTime);

        float scale = m_fLightTimeLeft / m_fLightTime;
        scale = powf(scale + EPS_L, 0.15f);
        float r = m_fLightRange * scale;
        VERIFY(_valid(r));
        m_pLight->set_color(m_LightColor.r * scale, m_LightColor.g * scale, m_LightColor.b * scale);
        m_pLight->set_range(r);

        Fvector pos = Position();
        pos.y += m_fLightHeight;
        m_pLight->set_position(pos);
    }
    else
        StopBlowoutLight();
}

void CCustomZone::AffectObjects()
{
    if (m_dwAffectFrameNum == Device.dwFrame)
        return;
    m_dwAffectFrameNum = Device.dwFrame;

    if (Device.dwPrecacheFrame)
        return;

    OBJECT_INFO_VEC_IT it;
    for (it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it)
    {
        if (!(*it).object->getDestroy())
            Affect(&(*it));
    }

    m_dwDeltaTime = 0;
}

void CCustomZone::UpdateBlowout()
{
    if (m_dwBlowoutParticlesTime >= (u32)m_iPreviousStateTime && m_dwBlowoutParticlesTime < (u32)m_iStateTime)
        PlayBlowoutParticles();

    if (m_dwBlowoutLightTime >= (u32)m_iPreviousStateTime && m_dwBlowoutLightTime < (u32)m_iStateTime)
        StartBlowoutLight();

    if (m_dwBlowoutSoundTime >= (u32)m_iPreviousStateTime && m_dwBlowoutSoundTime < (u32)m_iStateTime)
        m_blowout_sound.play_at_pos(0, Position());

    if (BlowoutWind && m_dwBlowoutWindTimeStart >= (u32)m_iPreviousStateTime && m_dwBlowoutWindTimeStart < (u32)m_iStateTime)
        StartWind();

    UpdateWind();

    if (m_dwBlowoutExplosionTime >= (u32)m_iPreviousStateTime && m_dwBlowoutExplosionTime < (u32)m_iStateTime)
    {
        AffectObjects();
        BornArtefact(false);

        if (m_BendGrass_Blowout)
            g_pGamePersistent->GrassBendersAddExplosion(ID(), Position(), {0, -99, 0}, 1.33f, m_BendGrass_Blowout_speed, 1.0f, m_BendGrass_Blowout_radius);
    }
}

void CCustomZone::OnMove()
{
    if (m_dwLastTimeMoved == 0)
    {
        m_dwLastTimeMoved = Device.dwTimeGlobal;
        m_vPrevPos.set(Position());
    }
    else
    {
        float time_delta = float(Device.dwTimeGlobal - m_dwLastTimeMoved) / 1000.f;
        m_dwLastTimeMoved = Device.dwTimeGlobal;

        Fvector vel;

        if (fis_zero(time_delta))
            vel = {};
        else
        {
            vel.sub(Position(), m_vPrevPos);
            vel.div(time_delta);
        }

        if (m_pIdleParticles)
            m_pIdleParticles->UpdateParent(XFORM(), vel);

        if (m_pLight && m_pLight->get_active())
            m_pLight->set_position(Position());

        if (m_pIdleLight && m_pIdleLight->get_active())
            m_pIdleLight->set_position(Position());

        if (grassbender_id)
        {
            // Check ID, just in case...
            if (grass_shader_data.id[grassbender_id] == ID())
            {
                const float saved_radius = grass_shader_data.pos[grassbender_id].w;
                const Fvector pos = Position();
                grass_shader_data.pos[grassbender_id].set(pos.x, pos.y, pos.z, saved_radius);
            }
        }
    }
}

void CCustomZone::OnEvent(NET_Packet& P, u16 type)
{
    switch (type)
    {
    case GE_OWNERSHIP_TAKE: {
        u16 id;
        P.r_u16(id);
        OnOwnershipTake(id);
        break;
    }
    case GE_OWNERSHIP_REJECT: {
        u16 id;
        P.r_u16(id);
        CArtefact* artefact = smart_cast<CArtefact*>(Level().Objects.net_Find(id));
        if (artefact)
        {
            bool just_before_destroy = !P.r_eof() && P.r_u8();
            artefact->H_SetParent(NULL, just_before_destroy);
            if (!just_before_destroy)
                ThrowOutArtefact(artefact);
        }
        break;
    }
    }
    inherited::OnEvent(P, type);
};
void CCustomZone::OnOwnershipTake(u16 id)
{
    CGameObject* GO = smart_cast<CGameObject*>(Level().Objects.net_Find(id));
    VERIFY(GO);
    if (!smart_cast<CArtefact*>(GO))
    {
        Msg("[%s] zone_name[%s] object_name[%s]", __FUNCTION__, cName().c_str(), GO->cName().c_str());
    }
    CArtefact* artefact = smart_cast<CArtefact*>(Level().Objects.net_Find(id));
    VERIFY(artefact);
    artefact->H_SetParent(this);

    artefact->setVisible(FALSE);
    artefact->setEnabled(FALSE);

    if (Local())
    {
        NET_Packet P;
        u_EventGen(P, GE_OWNERSHIP_REJECT, ID());
        P.w_u16(id);
        u_EventSend(P);
    }
}

void CCustomZone::OnStateSwitch(EZoneState new_state)
{
    if (eZoneStateDisabled == new_state)
        Disable();
    else
        Enable();

    if (new_state == eZoneStateAccumulate)
        PlayAccumParticles();

    if (new_state == eZoneStateAwaking)
        PlayAwakingParticles();

    m_eZoneState = new_state;
    m_iPreviousStateTime = m_iStateTime = 0;
};

void CCustomZone::SwitchZoneState(EZoneState new_state)
{
    OnStateSwitch(new_state);

    m_iPreviousStateTime = m_iStateTime = 0;
}

bool CCustomZone::Enable()
{
    if (IsEnabled())
        return false;

    for (OBJECT_INFO_VEC_IT it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it)
    {
        CGameObject* pObject = (*it).object;
        if (!pObject)
            continue;
        PlayEntranceParticles(pObject);
        PlayObjectIdleParticles(pObject);
    }

    PlayIdleParticles();
    return true;
};

bool CCustomZone::Disable()
{
    nlc_silence_register(false); // NLC: a disabled zone may get no further update (slow mode)
    if (m_nlc_flytrap)
    {
        // NLC flytrap: the same, and a disabled zone never sees its objects leave: give them their time back now
        nlc_flytrap_stasis(false);
        nlc_flytrap_release_all();
    }

    if (!IsEnabled())
        return false;

    for (OBJECT_INFO_VEC_IT it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it)
    {
        CGameObject* pObject = (*it).object;
        if (!pObject)
            continue;
        StopObjectIdleParticles(pObject);
    }

    StopIdleParticles();
    return false;
};

void CCustomZone::ZoneEnable() { SwitchZoneState(eZoneStateIdle); };

void CCustomZone::ZoneDisable(bool keep_update)
{
    m_keep_update = keep_update;
    SwitchZoneState(eZoneStateDisabled);
};

void CCustomZone::SpawnArtefact()
{
    //вычислить согласно распределению вероятностей
    //какой артефакт из списка ставить
    float rnd = ::Random.randF(.0f, 1.f - EPS_L);
    float prob_threshold = 0.f;

    std::size_t i = 0;
    for (; i < m_ArtefactSpawn.size(); i++)
    {
        prob_threshold += m_ArtefactSpawn[i].probability;
        if (rnd < prob_threshold)
            break;
    }
    R_ASSERT(i < m_ArtefactSpawn.size());
#ifdef DEBUG
    Msg("--[%s] anom: [%s], art: [%s]", __FUNCTION__, cName().c_str(), *m_ArtefactSpawn[i].section);
#endif
    Fvector pos;
    Center(pos);
    pos.y = pos.y + 1;

    pos.x = pos.x + ::Random.randF(-0.5, 0.5);
    pos.z = pos.z + ::Random.randF(-0.5, 0.5);
    Level().spawn_item(*m_ArtefactSpawn[i].section, pos, ai_location().level_vertex_id(), ID());
}

void CCustomZone::BornArtefact(bool forced)
{
#ifdef DEBUG
    Msg("BornArtefact[%s] prob %f cnt2 %f forced %d", cName().c_str(), m_fArtefactSpawnProbability, (float)m_ArtefactSpawn.size(), forced);
#endif
    if (!SpawnBlowoutArtefacts || m_ArtefactSpawn.empty())
        return;
    if (Device.dwPrecacheFrame)
        return;

    bool can_birth = false;
    if (forced == true)
        can_birth = true;
    else if (m_bBornOnBlowoutFlag == true && ::Random.randF(0.f, 1.f) < m_fArtefactSpawnOnDeathProbability)
        can_birth = true;

    if (can_birth != true)
    {
        if (::Random.randF(0.f, 1.f) > m_fArtefactSpawnProbability)
            return;
        OBJECT_INFO_VEC_IT it;
        for (it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it)
        {
            SZoneObjectInfo& info = (*it);
            if (!info.zone_ignore && !info.object->getDestroy())
            {
                if (info.nonalive_object == true)
                {
                    if (BirthOnNonAlive)
                    {
                        can_birth = true;
                        break;
                    }
                }
                else
                {
                    if (smart_cast<CActor*>(info.object))
                    { // KRodin: актора тут учитывать не надо.
                        can_birth = false;
                        break;
                    }
                    CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(info.object);
                    if (pEntityAlive && pEntityAlive->g_Alive())
                    {
                        if (BirthOnAlive)
                        {
                            can_birth = true;
                            break;
                        }
                    }
                    else if (BirthOnDead)
                    {
                        can_birth = true;
                        break;
                    }
                }
            }
        }
    }
    if (can_birth != true)
        return;

    m_bBornOnBlowoutFlag = false;

    SpawnArtefact();
}

void CCustomZone::ThrowOutArtefact(CArtefact* pArtefact)
{
    Fvector pos;
    pos.set(Position());
    pos.y += m_fArtefactSpawnHeight;
    pArtefact->XFORM().c.set(pos);

    if (*m_sArtefactSpawnParticles)
    {
        CParticlesObject* pParticles;
        pParticles = CParticlesObject::Create(*m_sArtefactSpawnParticles, TRUE);
        pParticles->UpdateParent(pArtefact->XFORM(), {});
        pParticles->Play(false);
    }

    m_ArtefactBornSound.play_at_pos(0, pos);

    NET_Packet PP;
    CGameObject::u_EventGen(PP, GE_CHANGE_POS, pArtefact->ID());
    PP.w_vec3(pos);
    CGameObject::u_EventSend(PP);

    Fvector dir;
    dir.random_dir();
    dir.normalize();
    pArtefact->m_pPhysicsShell->applyImpulse(dir, m_fThrowOutPower);
}

// void CCustomZone::PrefetchArtefacts()
//{
//	if (FALSE==m_zone_flags.test(eSpawnBlowoutArtefacts) || m_ArtefactSpawn.empty()) return;
//
//	for(u32 i = m_SpawnedArtefacts.size(); i < PREFETCHED_ARTEFACTS_NUM; ++i)
//		SpawnArtefact();
// }

void CCustomZone::StartWind()
{
    if (m_fDistanceToCurEntity > WIND_RADIUS)
        return;

    m_bBlowoutWindActive = true;
    m_fStoreWindPower = g_pGamePersistent->Environment().wind_strength_factor;
    clamp(g_pGamePersistent->Environment().wind_strength_factor, 0.f, 1.f);
}

void CCustomZone::StopWind()
{
    if (!m_bBlowoutWindActive)
        return;
    m_bBlowoutWindActive = false;
    g_pGamePersistent->Environment().wind_strength_factor = m_fStoreWindPower;
}

void CCustomZone::UpdateWind()
{
    if (!m_bBlowoutWindActive)
        return;

    if (m_fDistanceToCurEntity > WIND_RADIUS || m_dwBlowoutWindTimeEnd < (u32)m_iStateTime)
    {
        StopWind();
        return;
    }

    if (m_dwBlowoutWindTimePeak > (u32)m_iStateTime)
    {
        g_pGamePersistent->Environment().wind_strength_factor = m_fBlowoutWindPowerMax +
            (m_fStoreWindPower - m_fBlowoutWindPowerMax) * float(m_dwBlowoutWindTimePeak - (u32)m_iStateTime) / float(m_dwBlowoutWindTimePeak - m_dwBlowoutWindTimeStart);
        clamp(g_pGamePersistent->Environment().wind_strength_factor, 0.f, 1.f);
    }
    else
    {
        g_pGamePersistent->Environment().wind_strength_factor = m_fBlowoutWindPowerMax +
            (m_fStoreWindPower - m_fBlowoutWindPowerMax) * float((u32)m_iStateTime - m_dwBlowoutWindTimePeak) / float(m_dwBlowoutWindTimeEnd - m_dwBlowoutWindTimePeak);
        clamp(g_pGamePersistent->Environment().wind_strength_factor, 0.f, 1.f);
    }
}

u32 CCustomZone::ef_anomaly_type() const { return (m_ef_anomaly_type); }

u32 CCustomZone::ef_weapon_type() const
{
    VERIFY(m_ef_weapon_type != u32(-1));
    return (m_ef_weapon_type);
}

void CCustomZone::CreateHit(u16 id_to, u16 id_from, const Fvector& hit_dir, float hit_power, s16 bone_id, const Fvector& pos_in_bone, float hit_impulse, ALife::EHitType hit_type)
{
    {
        if (m_owner_id != u32(-1))
            id_from = (u16)m_owner_id;

        NET_Packet l_P;
        Fvector hdir = hit_dir;
        SHit Hit = SHit(hit_power, hdir, this, bone_id, pos_in_bone, hit_impulse, hit_type);
        Hit.GenHeader(GE_HIT, id_to);
        Hit.whoID = id_from;
        Hit.weaponID = this->ID();
        Hit.Write_Packet(l_P);

        u_EventSend(l_P);
    };
}

void CCustomZone::net_Relcase(CObject* O)
{
    CGameObject* GO = smart_cast<CGameObject*>(O);
    OBJECT_INFO_VEC_IT it = std::find(m_ObjectInfoMap.begin(), m_ObjectInfoMap.end(), GO);
    if (it != m_ObjectInfoMap.end())
    {
        exit_Zone(*it);
        m_ObjectInfoMap.erase(it);
    }
    if (GO->ID() == m_owner_id)
        m_owner_id = u32(-1);

    if (m_effector && m_effector->m_pActor && m_effector->m_pActor->ID() == GO->ID())
        m_effector->Stop();

    inherited::net_Relcase(O);
}

void CCustomZone::enter_Zone(SZoneObjectInfo& io) { callback(GameObject::eZoneEnter)(lua_game_object(), io.object->lua_game_object()); }

// NLC flytrap (docs/ANOMALY_DESIGN.md 18): loose physics objects (items, bolts, grenades, corpses) live in slowed time.
// Every element's velocity and spin are kept at k x their own, gravity is g x k^2 (so friction and falls stay
// consistent), hit impulses are x k (CPhysicsShellHolder::PHHit, CCharacterPhysicsSupport::in_Hit) and a fuse runs at k.
// The own velocity comes back on release. One zone owns a body at a time (overlapping Flytraps do not stack).
static bool nlc_flytrap_held_by_actor(CPhysicsShellHolder* sh)
{
    CActor* actor = Actor();
    if (!actor || !actor->character_physics_support())
        return false;
    CPHMovementControl* mc = actor->character_physics_support()->movement();
    CPHCapture* capture = mc ? mc->PHCapture() : nullptr;
    return capture && capture->taget_object() == sh;
}

void CCustomZone::nlc_flytrap_release(CGameObject* obj)
{
    CPhysicsShellHolder* sh = smart_cast<CPhysicsShellHolder*>(obj);
    if (!sh || sh->m_nlc_ph_time_zone != ID())
        return;
    const float k = sh->m_nlc_ph_time_k;
    sh->m_nlc_ph_time_k = 1.f;
    sh->m_nlc_ph_hit_k = 1.f;
    sh->m_nlc_ph_time_zone = u16(-1);
    if (sh->getDestroy() || sh->H_Parent())
        return;
    CPhysicsShell* shell = sh->PPhysicsShell();
    if (!shell || !shell->isActive())
        return;

    float out = 0.f;
    bool clamped = false;
    const float ratio = k > 0.f ? 1.f / k : 1.f;
    const u16 n = shell->get_ElementsNumber();
    // a ragdoll's joints correct their errors in real time: that jitter is multiplied by 1 / k here, so lower limits
    const float speed_max = n > 1 ? std::min(m_nlc_ft_obj_exit_speed, m_nlc_ft_ragdoll_exit_speed) : m_nlc_ft_obj_exit_speed;
    for (u16 i = 0; i < n; ++i)
    {
        CPhysicsElement* e = shell->get_ElementByStoreOrder(i);
        if (e->isFixed())
            continue;
        Fvector v, w;
        e->get_LinearVel(v);
        e->get_AngularVel(w);
        v.mul(ratio);
        w.mul(ratio);
        const float speed = v.magnitude();
        if (speed > speed_max)
        {
            // something set this body's velocity from outside the slowed time (a telekinetic throw, a drag), or jitter
            v.mul(speed_max / speed);
            clamped = true;
        }
        const float spin = w.magnitude();
        if (spin > m_nlc_ft_obj_exit_spin)
        {
            w.mul(m_nlc_ft_obj_exit_spin / spin);
            clamped = true;
        }
        out = std::max(out, v.magnitude());
        e->set_LinearVel(v);
        e->set_AngularVel(w);
    }
    shell->set_ApplyByGravity(true);
    shell->Enable(); // a body that fell asleep weightless would hang in the air
    if (m_nlc_ft_log)
        Msg("~ [flytrap] [%s]: released [%s], k %.2f -> own speed %.1f m/s%s", cName().c_str(), sh->cName().c_str(), k, out, clamped ? " (clamped)" : "");
}

void CCustomZone::nlc_flytrap_release_all()
{
    for (SZoneObjectInfo& io : m_ObjectInfoMap)
        nlc_flytrap_release(io.object);
}

void CCustomZone::nlc_flytrap_object(CPhysicsShellHolder* sh, const Fvector& P, float R, float ramp, u32 dt)
{
    CPhysicsShell* shell = sh->PPhysicsShell();
    // no shell yet (a creature that just died, before its ragdoll exists) or one the zone must not move
    if (!shell || !shell->isActive() || sh->H_Parent() || sh->hasFixedBones() || nlc_flytrap_held_by_actor(sh))
    {
        nlc_flytrap_release(sh);
        return;
    }
    if (sh->m_nlc_ph_time_zone != u16(-1) && sh->m_nlc_ph_time_zone != ID())
        return; // another Flytrap has it

    const float depth = R - P.distance_to(sh->Position());
    if (depth < -0.5f)
    {
        nlc_flytrap_release(sh); // outside (the object list is not updated while the zone is off)
        return;
    }
    // objects have their own short ramp (a thrown grenade must hang where it enters the dome, not fall through a 3 m
    // edge), and the factor falls geometrically: each part of the ramp divides time by the same amount
    const float obj_ramp = m_nlc_ft_obj_ramp > 0.f ? m_nlc_ft_obj_ramp : ramp;
    const float depth_k = std::clamp(depth / obj_ramp, 0.f, 1.f);
    const float k = m_nlc_ft_obj_time_rim * powf(m_nlc_ft_obj_time_core / m_nlc_ft_obj_time_rim, depth_k);
    const bool first = sh->m_nlc_ph_time_zone != ID();
    const float ratio = k / sh->m_nlc_ph_time_k;
    sh->m_nlc_ph_time_zone = ID();
    sh->m_nlc_ph_time_k = k;
    // a hit is k^exp: 1 = fully slowed, lower = a blast shows inside (the rest of the own speed comes out on exit, limited)
    sh->m_nlc_ph_hit_k = powf(k, m_nlc_ft_obj_hit_exp);

    shell->set_ApplyByGravity(false);
    const float dt_sec = std::min(float(dt) / 1000.f, 0.5f); // the zone's update interval (see nlc_flytrap_update)
    const float fall = ph_world ? ph_world->Gravity() * k * k * m_nlc_ft_obj_gravity_k * dt_sec : 0.f;
    const bool rescale = !fsimilar(ratio, 1.f, EPS_L);
    float speed = 0.f;
    for (u16 i = 0, n = shell->get_ElementsNumber(); i < n; ++i)
    {
        CPhysicsElement* e = shell->get_ElementByStoreOrder(i);
        if (e->isFixed() || !e->isEnabled())
            continue; // a sleeping body keeps no velocity to scale and must not store a fall
        Fvector v;
        e->get_LinearVel(v);
        if (rescale)
        {
            Fvector w;
            e->get_AngularVel(w);
            v.mul(ratio);
            w.mul(ratio);
            e->set_AngularVel(w);
        }
        v.y -= fall;
        e->set_LinearVel(v);
        speed = std::max(speed, v.magnitude());
    }

    if (CMissile* missile = smart_cast<CMissile*>(sh))
        missile->nlc_fuse_delay(u32(float(dt) * (1.f - k)));

    if (first && m_nlc_ft_log)
        Msg("~ [flytrap] [%s]: caught [%s], k %.2f, speed %.1f m/s (own %.1f)", cName().c_str(), sh->cName().c_str(), k, speed, speed / k);
}

void CCustomZone::nlc_flytrap_stasis(bool on)
{
    if (!m_nlc_flytrap)
        return;
    if (on)
    {
        Fvector P;
        XFORM().transform_tiny(P, CFORM()->getSphere().P);
        // every update: the registry is cleared with the level
        CBulletManager::nlc_stasis_set(ID(), P, CFORM()->getSphere().R, m_nlc_ft_bullet_cap, m_nlc_ft_bullet_ramp);
        m_nlc_stasis_on = true;
    }
    else if (m_nlc_stasis_on)
    {
        CBulletManager::nlc_stasis_remove(ID());
        m_nlc_stasis_on = false;
    }
}

// NLC flytrap (docs/ANOMALY_DESIGN.md 3): called from UpdateWorkload (every frame near the camera, else per schedule)
void CCustomZone::nlc_flytrap_update(u32 dt)
{
    if (!IsEnabled())
    {
        nlc_flytrap_stasis(false);
        return;
    }
    nlc_flytrap_stasis(true);

    Fvector P;
    XFORM().transform_tiny(P, CFORM()->getSphere().P);
    const float R = CFORM()->getSphere().R;
    const float ramp = std::max(R * m_nlc_ft_ramp_k, 0.1f);
    const float dt_sec = float(dt) / 1000.f;
    // gravity undone per update covers the whole interval: a zone away from the camera updates on its schedule (a few
    // hundred ms), and the old 0.1 s clamp left most of g there (bodies fell fast); the clamp only stops a load hitch
    const float g_dt = std::min(dt_sec, 0.5f);

    for (SZoneObjectInfo& io : m_ObjectInfoMap)
    {
        CGameObject* obj = io.object;
        if (!obj || obj->getDestroy())
            continue;

        CEntityAlive* ea = smart_cast<CEntityAlive*>(obj);
        if (ea && ea->g_Alive())
        {
            CBaseMonster* monster = smart_cast<CBaseMonster*>(obj);
            if (!monster)
            {
                // the actor's side is scripted (sivol_flytrap.script) except the air. Stalkers are kept out, not slowed.
                // In the air (docs/ANOMALY_DESIGN.md 19): slowed time with the movement factor f. The jump speed is
                // already x f (Actor_Movement.cpp), gravity becomes f^2 g, so a jump keeps its height and lasts 1 / f
                // longer. The horizontal speed and a fall from above are still limited (a jump does not carry him out).
                static u32 s_nlc_actor_air_frame = u32(-1); // overlapping Flytraps: once per frame
                if (obj == Actor() && s_nlc_actor_air_frame != Device.dwFrame)
                {
                    CPHMovementControl* mc = ea->character_physics_support()->movement();
                    if (mc && mc->Environment() != CPHMovementControl::peOnGround)
                    {
                        s_nlc_actor_air_frame = Device.dwFrame;
                        const float f = g_nlc_actor_field_speed_k;
                        const float cap = m_nlc_ft_actor_air * f;
                        Fvector v = mc->GetVelocity();
                        bool changed = false;
                        const float horiz = _sqrt(v.x * v.x + v.z * v.z);
                        if (horiz > cap)
                        {
                            const float m = std::max(cap / horiz, expf(-dt_sec / 0.15f));
                            v.x *= m;
                            v.z *= m;
                            changed = true;
                        }
                        if (v.y < -cap)
                        {
                            v.y *= std::max(cap / -v.y, expf(-dt_sec / 0.15f));
                            changed = true;
                        }
                        if (f < 0.999f && ph_world)
                        {
                            v.y += ph_world->Gravity() * (1.f - f * f) * g_dt; // the rest of g is undone
                            changed = true;
                        }
                        if (changed)
                            mc->SetVelocity(v);
                    }
                }
                continue;
            }

            const float depth_k = std::clamp((R - P.distance_to(monster->Position())) / ramp, 0.f, 1.f);
            const float slow = m_nlc_ft_slow_rim + (m_nlc_ft_slow_core - m_nlc_ft_slow_rim) * depth_k;
            monster->nlc_apply_move_slow(slow, 500, 0.4f);
            monster->nlc_set_time_k(1.f - slow, 300, m_nlc_ft_log); // every clip (CControlAnimation::play); log: [flytrap_anim]

            // the physical limit: whatever drives the body (a jump, a charge, a fall) cannot go faster than this.
            // The path movement sets its own limit every step and a jump adds to it once, so it is only ever lowered.
            if (CPHMovementControl* mc = monster->character_physics_support()->movement())
            {
                const float cap = m_nlc_ft_monster_cap_rim + (m_nlc_ft_monster_cap_core - m_nlc_ft_monster_cap_rim) * depth_k;
                if (mc->VelocityLimit() > cap)
                    mc->SetVelocityLimit(cap);
                // a jump or a fall: the character has no control then and its limit is not used
                Fvector v = mc->GetVelocity();
                const float jk = monster->m_nlc_jump_k;
                if (jk < 0.999f && mc->Environment() != CPHMovementControl::peOnGround)
                {
                    // a slow-motion jump (CControlJump::nlc_launch): launched at k x speed, gravity k^2 g, not braked.
                    // Once per frame (overlapping Flytraps would undo gravity twice)
                    if (ph_world && monster->m_nlc_jump_air_frame != Device.dwFrame)
                    {
                        monster->m_nlc_jump_air_frame = Device.dwFrame;
                        v.y += ph_world->Gravity() * (1.f - jk * jk) * g_dt;
                        mc->SetVelocity(v);
                    }
                }
                else
                {
                    const float speed = v.magnitude();
                    if (speed > cap * 1.15f)
                    {
                        v.mul(std::max(cap / speed, expf(-dt_sec / 0.15f)));
                        mc->SetVelocity(v);
                    }
                }
            }

            // it understands where it is after a moment, and so do the mates of its kind around
            if (!io.hit_num && io.time_in_zone >= m_nlc_ft_learn_ms)
            {
                io.hit_num = 1;
                monster->anomaly_detector().nlc_learn(this);
                u32 mates = 0;
                if (m_nlc_ft_learn_radius > 0.f)
                {
                    xr_vector<CObject*> nearest;
                    Level().ObjectSpace.GetNearest(nearest, P, m_nlc_ft_learn_radius, NULL);
                    for (CObject* it : nearest)
                    {
                        CBaseMonster* mate = smart_cast<CBaseMonster*>(it);
                        if (!mate || mate == monster || !mate->g_Alive() || mate->CLS_ID != monster->CLS_ID)
                            continue;
                        mate->anomaly_detector().nlc_learn(this);
                        ++mates;
                    }
                }
                if (m_nlc_ft_log)
                    Msg("~ [flytrap] [%s]: [%s] trapped, %u mates learned the zone", cName().c_str(), monster->cName().c_str(), mates);
            }
            continue;
        }

        // loose physics objects and corpses: slowed time
        if (obj == Actor())
            continue;
        if (CPhysicsShellHolder* sh = smart_cast<CPhysicsShellHolder*>(obj))
            nlc_flytrap_object(sh, P, R, ramp, dt);
    }
}

// NLC silence (docs/ANOMALY_DESIGN.md 16): the zone's sphere in the AI hearing registry (xr_3da/xr_area.cpp)
void CCustomZone::nlc_silence_register(bool on)
{
    if (!m_nlc_silence)
        return;
    if (on && !getDestroy())
    {
        Fvector P;
        XFORM().transform_tiny(P, CFORM()->getSphere().P);
        nlc_silence_set(ID(), P, CFORM()->getSphere().R, m_nlc_silence_rim, m_nlc_silence_core, m_nlc_silence_ramp_k);
        if (!m_nlc_silence_on && m_nlc_silence_log)
            Msg("~ [silence] [%s]: on, r=%.1f at %.1f,%.1f,%.1f (ai rim %.2f core %.2f)", cName().c_str(), CFORM()->getSphere().R, P.x, P.y, P.z, m_nlc_silence_rim,
                m_nlc_silence_core);
        m_nlc_silence_on = true;
    }
    else if (m_nlc_silence_on)
    {
        nlc_silence_remove(ID());
        m_nlc_silence_on = false;
        if (m_nlc_silence_log)
            Msg("~ [silence] [%s]: off (%u sound deliveries weakened on this level so far)", cName().c_str(), nlc_silence_muted());
    }
}

void CCustomZone::exit_Zone(SZoneObjectInfo& io)
{
    if (m_nlc_flytrap)
        nlc_flytrap_release(io.object); // NLC

    StopObjectIdleParticles(io.object);
    if (!getDestroy())
        callback(GameObject::eZoneExit)(lua_game_object(), io.object->lua_game_object());
}

void CCustomZone::PlayAccumParticles()
{
    if (m_sAccumParticles.size())
    {
        CParticlesObject* pParticles;
        pParticles = CParticlesObject::Create(*m_sAccumParticles, TRUE);
        pParticles->UpdateParent(XFORM(), {});
        pParticles->Play(false);
    }

    if (m_accum_sound._handle())
        m_accum_sound.play_at_pos(0, Position());
}

void CCustomZone::PlayAwakingParticles()
{
    if (m_sAwakingParticles.size())
    {
        CParticlesObject* pParticles;
        pParticles = CParticlesObject::Create(*m_sAwakingParticles, TRUE);
        pParticles->UpdateParent(XFORM(), {});
        pParticles->Play(false);
    }

    if (m_awaking_sound._handle())
        m_awaking_sound.play_at_pos(0, Position());
}

void CCustomZone::UpdateOnOffState()
{
    if (!UseOnOffTime)
        return;

    bool dest_state;
    u32 t = (Device.dwTimeGlobal - m_StartTime + m_TimeShift) % (m_TimeToEnable + m_TimeToDisable);
    if (t < m_TimeToEnable)
        dest_state = true;
    else if (t >= (m_TimeToEnable + m_TimeToDisable))
        dest_state = true;
    else
    {
        dest_state = false;
        VERIFY(t < (m_TimeToEnable + m_TimeToDisable));
    }

    if ((eZoneStateDisabled == m_eZoneState) && dest_state)
    {
        GoEnabledState();
    }
    else if ((eZoneStateIdle == m_eZoneState) && !dest_state)
    {
        GoDisabledState();
    }
}

void CCustomZone::GoDisabledState()
{
    // switch to disable
    OnStateSwitch(eZoneStateDisabled);

    OBJECT_INFO_VEC_IT it = m_ObjectInfoMap.begin();
    OBJECT_INFO_VEC_IT it_e = m_ObjectInfoMap.end();

    for (; it != it_e; ++it)
        exit_Zone(*it);

    m_ObjectInfoMap.clear();
    feel_touch.clear();
}

void CCustomZone::GoEnabledState()
{
    // switch to idle
    OnStateSwitch(eZoneStateIdle);
}

BOOL CCustomZone::feel_touch_on_contact(CObject* O)
{
    if ((spatial.type | STYPE_VISIBLEFORAI) != spatial.type)
        return (FALSE);

    return (inherited::feel_touch_on_contact(O));
}

BOOL CCustomZone::AlwaysTheCrow()
{
    if (m_b_always_fastmode && IsEnabled())
        return TRUE;
    else
        return inherited::AlwaysTheCrow();
}

void CCustomZone::GrassZoneUpdate()
{
    if (m_BendGrass_idle_anim <= -1 && m_BendGrass_whenactive_anim <= -1)
        return;

    bool IsActive{};
    s8 targetAnim = -1;

    // If m_BendGrass_Blowout_time is not set, use m_eZoneState to detect activation
    if (m_BendGrass_Blowout_time <= -1)
        IsActive = m_eZoneState != eZoneStateIdle;
    else
        IsActive = m_fBlowoutTimeLeft > Device.dwTimeGlobal;

    // Target animation depending if Zone is active
    targetAnim = (IsActive && m_BendGrass_whenactive_anim > -1) ? m_BendGrass_whenactive_anim : m_BendGrass_idle_anim;

    // Update grass bender if the animation is > -1
    if (targetAnim > -1 || (grassbender_id > 0 && grass_shader_data.anim[grassbender_id] > -1))
        g_pGamePersistent->GrassBendersUpdate(ID(), grassbender_id, grassbender_frame, Position(), m_BendGrass_idle_radius, 0.0f);
    else
        g_pGamePersistent->GrassBendersRemoveByIndex(grassbender_id);

    // Return if grassbender_id doesn't exist
    if (grassbender_id == 0)
        return;

    // Animation transition, diminish intensity to 0 and change.
    if (grass_shader_data.anim[grassbender_id] != targetAnim)
    {
        grass_shader_data.str_target[grassbender_id] += g_pGamePersistent->GrassBenderToValue(grass_shader_data.str_target[grassbender_id], 0.0f, 7.5f, false);

        if (grass_shader_data.str_target[grassbender_id] <= 0.05f)
            grass_shader_data.anim[grassbender_id] = targetAnim;

        return;
    }

    // Apply settings when needed
    if (IsActive)
    {
        if (m_BendGrass_whenactive_speed >= 0)
            grass_shader_data.speed[grassbender_id] += g_pGamePersistent->GrassBenderToValue(grass_shader_data.speed[grassbender_id], m_BendGrass_whenactive_speed, 10.0f, true);

        if (m_BendGrass_whenactive_str >= 0)
            grass_shader_data.str_target[grassbender_id] += g_pGamePersistent->GrassBenderToValue(grass_shader_data.str_target[grassbender_id], m_BendGrass_whenactive_str, 10.0f, true);
    }
    else
    {
        grass_shader_data.speed[grassbender_id] += g_pGamePersistent->GrassBenderToValue(grass_shader_data.speed[grassbender_id], m_BendGrass_idle_speed, 10.0f, true);
        grass_shader_data.str_target[grassbender_id] += g_pGamePersistent->GrassBenderToValue(grass_shader_data.str_target[grassbender_id], m_BendGrass_idle_str, 10.0f, true);
    }
}