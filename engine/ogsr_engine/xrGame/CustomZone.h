#pragma once

#include "space_restrictor.h"

class CActor;
class CLAItem;
class CArtefact;
class CParticlesObject;
class CZoneEffector;
class CPhysicsShellHolder;

#define SMALL_OBJECT_RADIUS 0.6f

//информация о объекте, находящемся в зоне
struct SZoneObjectInfo
{
    SZoneObjectInfo() : object(NULL), zone_ignore(false), time_in_zone(0), hit_num(0), total_damage(0), small_object(false), nonalive_object(false), death_in_zone(false) {}
    CGameObject* object;
    bool small_object;
    bool nonalive_object;
    //игнорирование объекта в зоне
    bool zone_ignore;
    //присоединенные партиклы
    xr_vector<CParticlesObject*> particles_vector;
    //время прибывания в зоне
    u32 time_in_zone;
    //количество раз, сколько зона воздействовала на объект
    u32 hit_num;
    //количество повреждений нанесенных зоной
    float total_damage;
    //существо померло в зоне
    bool death_in_zone;

    bool operator==(const CGameObject* O) const { return object == O; }
};

class CCustomZone : public CSpaceRestrictor
{
    friend class CAnomalyZoneScript;

private:
    typedef CSpaceRestrictor inherited;

public:
    CZoneEffector* m_effector;

public:
    CCustomZone(void);
    virtual ~CCustomZone();

    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Export(CSE_Abstract* E);
    virtual void Load(LPCSTR section);
    virtual void net_Destroy();

    virtual void UpdateCL();
    virtual void UpdateWorkload(u32 dt); // related to fast-mode optimizations
    virtual void shedule_Update(u32 dt);
    virtual void enter_Zone(SZoneObjectInfo& io);
    virtual void exit_Zone(SZoneObjectInfo& io);
    virtual void feel_touch_new(CObject* O);
    virtual void feel_touch_delete(CObject* O);
    virtual BOOL feel_touch_contact(CObject* O);
    virtual BOOL feel_touch_on_contact(CObject* O);
    virtual float effective_radius();
    virtual float distance_to_center(CObject* O);
    virtual void Postprocess(float val) {}
    virtual void net_Relcase(CObject* O);
    virtual void OnEvent(NET_Packet& P, u16 type);
    void OnOwnershipTake(u16 id);

    float GetMaxPower() { return m_fMaxPower; }
    void SetMaxPower(float p) { m_fMaxPower = p; }

    //вычисление силы хита в зависимости от расстояния до центра зоны
    //относительный размер силы (от 0 до 1)
    virtual float RelativePower(float dist);
    //абсолютный размер
    float Power(float dist);

    virtual CCustomZone* cast_custom_zone() { return this; }

    //различные состояния в которых может находиться зона
    typedef enum
    {
        eZoneStateIdle = 0, //состояние зоны, когда внутри нее нет активных объектов
        eZoneStateAwaking, //пробуждение зоны (объект попал в зону)
        eZoneStateBlowout, //выброс
        eZoneStateAccumulate, //накапливание энергии, после выброса
        eZoneStateDisabled,
        eZoneStateMax
    } EZoneState;

public:
    void SetIgnoreAny(bool status) { IgnoreAny = status; }

protected:
    bool IgnoreAny{};
    bool IgnoreAnomalyDetector{};
    bool IgnoreNonAlive{};
    bool IgnoreSmall{};
    bool IgnoreArtefact{};
    bool BirthOnNonAlive{};
    bool BirthOnAlive{};
    bool BirthOnDead{};
    bool VisibleDetector{};
    bool BlowoutWind{};
    bool SpawnBlowoutArtefacts{};
    bool UseOnOffTime{};

    u32 m_owner_id; // if created from artefact
    u32 m_ttl;
    //список объетков, находящихся в зоне
    CActor* m_pLocalActor;

    //максимальная сила заряда зоны
    float m_fMaxPower;

    //линейный коэффициент затухания в зависимости от расстояния
    float m_fAttenuation;
    //процент удара зоны, который пойдет на физический импульс
    float m_fHitImpulseScale;
    //размер радиуса в процентах от оригинального,
    //где действует зона
    float m_fEffectiveRadius;

    //тип наносимого хита
    ALife::EHitType m_eHitTypeBlowout;

    EZoneState m_eZoneState;
    bool m_keep_update;

    //текущее время пребывания зоны в определенном состоянии
    int m_iStateTime;
    int m_iPreviousStateTime;

    u32 m_TimeToDisable;
    u32 m_TimeToEnable;
    u32 m_TimeShift;
    u32 m_StartTime;

    //массив с временами, сколько каждое состояние должно
    //длиться (если 0, то мгновенно -1 - бесконечность,
    //-2 - вообще не должно вызываться)
    typedef svector<int, eZoneStateMax> StateTimeSVec;
    StateTimeSVec m_StateTime;

    virtual void SwitchZoneState(EZoneState new_state);
    virtual void OnStateSwitch(EZoneState new_state);
    virtual void CheckForAwaking();
    //обработка зоны в различных состояниях
    virtual bool IdleState();
    virtual bool AwakingState();
    virtual bool BlowoutState();
    virtual bool AccumulateState();

    bool Enable();
    bool Disable();
    void UpdateOnOffState();
    virtual void GoEnabledState();
    virtual void GoDisabledState();

public:
    bool IsEnabled() { return m_eZoneState != eZoneStateDisabled; };
    void ZoneEnable();
    void ZoneDisable(bool = false);
    EZoneState ZoneState() { return m_eZoneState; }

protected:
    //воздействие зоной на объект
    virtual void Affect(SZoneObjectInfo* O) {}

    //воздействовать на все объекты в зоне
    virtual void AffectObjects();

    u32 m_dwAffectFrameNum;

    u32 m_dwDeltaTime;
    u32 m_dwPeriod;
    //	bool					m_bZoneReady;
    //если в зоне есть не disabled объекты
    bool m_bZoneActive;

    //параметры для выброса, с какой задержкой
    //включать эффекты и логику
    u32 m_dwBlowoutParticlesTime;
    u32 m_dwBlowoutLightTime;
    u32 m_dwBlowoutSoundTime;
    u32 m_dwBlowoutExplosionTime;
    void UpdateBlowout();

    //ветер
    bool m_bBlowoutWindActive;
    u32 m_dwBlowoutWindTimeStart;
    u32 m_dwBlowoutWindTimePeak;
    u32 m_dwBlowoutWindTimeEnd;
    //сила ветра (увеличение текущего) (0,1) когда в аномалию попадает актер
    float m_fBlowoutWindPowerMax;
    float m_fStoreWindPower;

    void StartWind();
    void StopWind();
    void UpdateWind();

    //время, через которое, зона перестает реагировать
    //на объект мертвый объект (-1 если не указано)
    int m_iDisableHitTime;
    //тоже самое но для маленьких объектов
    int m_iDisableHitTimeSmall;
    int m_iDisableIdleTime;

    ////////////////////////////////
    // имена партиклов зоны

    //обычное состояние зоны
    shared_str m_sIdleParticles;
    //выброс зоны
    shared_str m_sBlowoutParticles;
    shared_str m_sAccumParticles;
    shared_str m_sAwakingParticles;

    //появление большого и мальнекого объекта в зоне
    shared_str m_sEntranceParticlesSmall;
    shared_str m_sEntranceParticlesBig;
    //поражение большого и мальнекого объекта в зоне
    shared_str m_sHitParticlesSmall;
    shared_str m_sHitParticlesBig;
    //нахождение большого и мальнекого объекта в зоне
    shared_str m_sIdleObjectParticlesSmall;
    shared_str m_sIdleObjectParticlesBig;
    BOOL m_bIdleObjectParticlesDontStop;

    ref_sound m_idle_sound;
    ref_sound m_awaking_sound;
    ref_sound m_accum_sound;
    ref_sound m_blowout_sound;
    ref_sound m_hit_sound;
    ref_sound m_entrance_sound;

    // Interactive grass Settings
    u32 m_fBlowoutTimeLeft{};

    s8 m_BendGrass_idle_anim{};
    float m_BendGrass_idle_radius{};
    float m_BendGrass_idle_speed{};
    float m_BendGrass_idle_str{};

    s8 m_BendGrass_whenactive_anim{};
    float m_BendGrass_whenactive_speed{};
    float m_BendGrass_whenactive_str{};

    bool m_BendGrass_Blowout{};
    u32 m_BendGrass_Blowout_time{};
    float m_BendGrass_Blowout_speed{};
    float m_BendGrass_Blowout_radius{};

    size_t grassbender_id{};
    u32 grassbender_frame{};

    void GrassZoneUpdate();

    //объект партиклов обычного состояния зоны
    CParticlesObject* m_pIdleParticles;

    //////////////////////////////
    //подсветка аномалии

    //подсветка idle состояния
    ref_light m_pIdleLight;
    Fcolor m_IdleLightColor;
    float m_fIdleLightRange;
    float m_fIdleLightRangeDelta;
    float m_fIdleLightHeight;
    CLAItem* m_pIdleLAnim;

    // подсветка выброса
    ref_light m_pLight;
    Fcolor m_LightColor;
    float m_fLightRange;
    float m_fLightHeight;
    float m_fLightTime;
    float m_fLightTimeLeft;

    bool IdleLight{};
    bool IdleLightShadow{};
    bool IdleLightVolumetric{};
    float IdleLightVolumetricIntensity{};
    float IdleLightVolumetricDistance{};

    bool BlowoutLight{};
    bool BlowoutLightShadow{};
    bool BlowLightVolumetric{};
    float BlowLightVolumetricIntensity{};
    float BlowLightVolumetricDistance{};

    void StartIdleLight();
    void StopIdleLight();
    void UpdateIdleLight();

    void StartBlowoutLight();
    void StopBlowoutLight();
    void UpdateBlowoutLight();

    //список партиклов для объетов внутри зоны
    //	DEFINE_MAP (CObject*, SZoneObjectInfo, OBJECT_INFO_MAP, OBJECT_INFO_MAP_IT);
    DEFINE_VECTOR(SZoneObjectInfo, OBJECT_INFO_VEC, OBJECT_INFO_VEC_IT);
    OBJECT_INFO_VEC m_ObjectInfoMap;

    void CreateHit(u16 id_to, u16 id_from, const Fvector& hit_dir, float hit_power, s16 bone_id, const Fvector& pos_in_bone, float hit_impulse, ALife::EHitType hit_type);

    virtual void Hit(SHit* pHDS);

    //для визуализации зоны
    void PlayIdleParticles();
    void StopIdleParticles();
    void PlayAccumParticles();
    void PlayAwakingParticles();
    void PlayBlowoutParticles();
    void PlayEntranceParticles(CGameObject* pObject);
    void PlayBulletParticles(Fvector& pos);

    void PlayHitParticles(CGameObject* pObject);

    void PlayObjectIdleParticles(CGameObject* pObject);
    void StopObjectIdleParticles(CGameObject* pObject);

    virtual bool IsVisibleForZones() { return false; }

    //обновление, если зона передвигается
    virtual void OnMove();
    Fvector m_vPrevPos;
    u32 m_dwLastTimeMoved;

    //видимость зоны детектором
public:
    bool VisibleByDetector() { return VisibleDetector; }

    //////////////////////////////////////////////////////////////////////////
    // список артефактов
protected:
    virtual void SpawnArtefact();

    //рождение артефакта в зоне, во время ее срабатывания
    //и присоединение его к зоне
    void BornArtefact(bool forced);
    //выброс артефактов из зоны
    void ThrowOutArtefact(CArtefact* pArtefact);

    // void	PrefetchArtefacts			();
    virtual BOOL AlwaysTheCrow();

protected:
    DEFINE_VECTOR(CArtefact*, ARTEFACT_VECTOR, ARTEFACT_VECTOR_IT);
    ARTEFACT_VECTOR m_SpawnedArtefacts;

    //есть ли вообще функция выбрасывания артефактов во время срабатывания
    //	bool					m_bSpawnBlowoutArtefacts;
    //вероятность того, что артефакт засповниться при единичном
    //срабатывании аномалии
    float m_fArtefactSpawnProbability;
    // bak вероятность спавна при смерти в зоне
    float m_fArtefactSpawnOnDeathProbability;

    //величина импульса выкидывания артефакта из зоны
    float m_fThrowOutPower;
    //высота над центром зоны, где будет появляться артефакт
    float m_fArtefactSpawnHeight;

    //имя партиклов, которые проигрываются во время и на месте рождения артефакта
    shared_str m_sArtefactSpawnParticles;
    //звук рождения артефакта
    ref_sound m_ArtefactBornSound;

    struct ARTEFACT_SPAWN
    {
        shared_str section;
        float probability;
    };

    DEFINE_VECTOR(ARTEFACT_SPAWN, ARTEFACT_SPAWN_VECTOR, ARTEFACT_SPAWN_IT);
    ARTEFACT_SPAWN_VECTOR m_ArtefactSpawn;

    //расстояние от зоны до текущего актера
    float m_fDistanceToCurEntity;

    // bak / флаг для рождения артефакта
    bool m_bBornOnBlowoutFlag;

protected:
    u32 m_ef_anomaly_type;
    u32 m_ef_weapon_type;
    bool m_b_always_fastmode;

    bool DestroyAfterBlowout{}; //Для самоудаления мин после взрыва
    u32 LastBlowoutTime{};

public:
    virtual u32 ef_anomaly_type() const;
    virtual u32 ef_weapon_type() const;
    virtual bool register_schedule() const { return true; }

    // NLC flytrap (docs/ANOMALY_DESIGN.md 3): a stasis field. Section key nlc_flytrap; no damage of its own.
public:
    bool nlc_flytrap() const { return m_nlc_flytrap; }
    float nlc_detect_k() const { return m_nlc_detect_k; } // multiplies a monster's Anomaly_Detect_Probability for this zone
    bool nlc_follow_enemy() const { return m_nlc_follow_enemy; } // monsters enter it while their enemy is inside

protected:
    bool m_nlc_soft_object_particles{}; // NLC: section key object_particles_soft_stop
    bool m_nlc_flytrap{};
    bool m_nlc_follow_enemy{};
    bool m_nlc_stasis_on{};
    bool m_nlc_ft_log{};
    float m_nlc_detect_k{1.f};
    float m_nlc_ft_bullet_cap{8.f}; // m/s
    float m_nlc_ft_bullet_ramp{1.5f}; // m
    float m_nlc_ft_slow_rim{0.3f}; // monster speed removed at the edge
    float m_nlc_ft_slow_core{0.7f}; // and past ramp_k of the radius
    float m_nlc_ft_ramp_k{0.6f};
    // loose physics objects (items, bolts, grenades, corpses): slowed time (docs/ANOMALY_DESIGN.md 18)
    float m_nlc_ft_obj_time_rim{0.7f}; // time factor at the edge (velocity, spin, fuse, hit impulse)
    float m_nlc_ft_obj_time_core{0.3f}; // ... and past ramp_k of the radius
    float m_nlc_ft_obj_hit_exp{1.f}; // hit impulses inside x k ^ this (flytrap_object_hit_k_exp)
    float m_nlc_ft_obj_ramp{1.f}; // m from the surface to the core factor (geometric); 0 = R x flytrap_ramp_k
    float m_nlc_ft_obj_gravity_k{1.f}; // gravity inside = g x k^2 x this
    float m_nlc_ft_obj_exit_speed{20.f}; // m/s, own speed limit given back on release
    float m_nlc_ft_ragdoll_exit_speed{8.f}; // m/s, the same for bodies of several elements (corpses)
    float m_nlc_ft_obj_exit_spin{20.f}; // rad/s, own spin limit given back on release
    float m_nlc_ft_monster_cap_rim{4.5f}; // m/s, the physical speed limit of a live monster at the edge
    float m_nlc_ft_monster_cap_core{1.8f}; // ... and in the core (jumps, charges and falls included)
    bool m_nlc_particles_log{}; // section key object_particles_log
    float m_nlc_ft_actor_air{5.f}; // m/s x the actor field factor: the actor's speed limit off the ground
    float m_nlc_ft_learn_radius{25.f}; // m, mates of a trapped monster that learn the zone
    u32 m_nlc_ft_learn_ms{700}; // ms inside before a monster understands
    void nlc_flytrap_update(u32 dt);
    void nlc_flytrap_object(CPhysicsShellHolder* sh, const Fvector& P, float R, float ramp, u32 dt);
    void nlc_flytrap_release(CGameObject* obj);
    void nlc_flytrap_release_all();
    void nlc_flytrap_stasis(bool on);

    // NLC silence (docs/ANOMALY_DESIGN.md 16): sounds made or heard inside do not reach the AI. Section key nlc_silence;
    // the actor's own hearing is scripted (sivol_silence.script).
    bool m_nlc_silence{};
    bool m_nlc_silence_on{};
    bool m_nlc_silence_log{};
    float m_nlc_silence_rim{0.7f}; // share of a sound's power removed at the edge
    float m_nlc_silence_core{1.f}; // ... and past ramp_k of the radius
    float m_nlc_silence_ramp_k{0.5f};
    void nlc_silence_register(bool on);

    // NLC anomaly haze (docs/ANOMALY_HAZE_DESIGN.md): section key nlc_haze = <profile> ( config\environment\anomaly_haze.ltx ).
    // Registered while the zone is enabled; the zone lights feed the haze and lose their volumetric flag meanwhile.
    shared_str m_nlc_haze_name;
    int m_nlc_haze{-1}; // profile index at spawn, -1 = none or filtered out by the profile's levels list
    bool m_nlc_haze_on{};
    bool m_nlc_haze_vol_off{}; // the zone lights' volumetric flag is switched off by the haze
    bool m_nlc_haze_ptcl_hidden{}; // idle particles stopped by ssfx_haze_particles 0
    u32 m_nlc_haze_sun_next{};
    shared_str m_nlc_haze_ptcl; // key nlc_haze_idle_particles: idle particles while hazed ( trimmed set ), empty = keep
    shared_str m_nlc_haze_ptcl_orig; // the section's idle_particles
    bool m_nlc_haze_ptcl_swapped{};
    void nlc_haze_swap_particles(bool haze);
    void nlc_haze_play_idle_particles();
    Fvector m_nlc_haze_idle_col{}; // last idle light colour ( UpdateIdleLight )
    float m_nlc_haze_idle_range{};
    void nlc_haze_update();
    void nlc_haze_volumetric(bool off);

    // optimization FAST/SLOW mode
public:
    bool o_fastmode;
    IC void o_switch_2_fast()
    {
        if (o_fastmode)
            return;
        o_fastmode = true;
        processing_activate();
    }
    IC void o_switch_2_slow()
    {
        if (!o_fastmode)
            return;
        o_fastmode = false;
        processing_deactivate();
    }
};
