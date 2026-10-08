#pragma once

#include "../../../CustomMonster.h"

#include "../monster_enemy_memory.h"
#include "../monster_corpse_memory.h"
#include "../monster_sound_memory.h"
#include "../monster_hit_memory.h"

#include "../monster_enemy_manager.h"
#include "../monster_corpse_manager.h"

#include "../../../step_manager.h"
#include "../monster_event_manager.h"
#include "../melee_checker.h"
#include "../monster_morale.h"

#include "../control_manager.h"
#include "../control_sequencer.h"

#include "../ai_monster_utils.h"

#include "../control_manager_custom.h"
#include "../ai_monster_shared_data.h"
#include "../monster_sound_defs.h"
#include "../monster_aura.h"
#include "../../../inventoryowner.h"
#include "../ai_monster_squad.h"

class CCharacterPhysicsSupport;
class CMonsterCorpseCoverEvaluator;
class CCoverEvaluatorFarFromEnemy;
class CCoverEvaluatorCloseToEnemy;
class CMonsterEventManager;
class CJumping;
class CControlledEntityBase;
class CMovementManager;
class IStateManagerBase;
class CAnomalyDetector;

class CControlAnimationBase;
class CControlMovementBase;
class CControlPathBuilderBase;
class CControlDirectionBase;
class CMonsterCoverManager;

class CMonsterHome;

// Lain: added
class CMonsterSquad;
class squad_grouping_behaviour;

class anti_aim_ability;

class CBaseMonster : public CCustomMonster, public CStepManager, public CInventoryOwner
{
    typedef CCustomMonster inherited;

public:
    CBaseMonster();
    virtual ~CBaseMonster();

public:
    virtual Feel::Sound* dcast_FeelSound() { return this; }
    virtual CCharacterPhysicsSupport* character_physics_support() { return m_pPhysics_support; }
    virtual CPHDestroyable* ph_destroyable();
    virtual CEntityAlive* cast_entity_alive() { return this; }
    virtual CEntity* cast_entity() { return this; }
    virtual CPhysicsShellHolder* cast_physics_shell_holder() { return this; }
    virtual CParticlesPlayer* cast_particles_player() { return this; }
    virtual CCustomMonster* cast_custom_monster() { return this; }
    virtual CScriptEntity* cast_script_entity() { return this; }
    virtual CBaseMonster* cast_base_monster() { return this; }

    virtual CInventoryOwner* cast_inventory_owner() { return this; }
    virtual bool unlimited_ammo() { return false; }
    virtual CGameObject* cast_game_object() { return this; }

public:
    virtual void Die(CObject* who);
    virtual void HitSignal(float amount, Fvector& vLocalDir, CObject* who, s16 element);
    virtual void Hit(SHit* pHDS);
    virtual void PHHit(SHit& H);
    virtual void SelectAnimation(const Fvector& _view, const Fvector& _move, float speed);

    virtual void Load(LPCSTR section);
    virtual void PostLoad(LPCSTR);
    virtual DLL_Pure* _construct();

    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Destroy();
    virtual void net_Save(NET_Packet& P);
    virtual BOOL net_SaveRelevant();
    virtual void net_Export(CSE_Abstract* E);
    virtual void net_Relcase(CObject* O);

    // save/load server serialization
    virtual void save(NET_Packet& output_packet) { inherited::save(output_packet); }
    virtual void load(IReader& input_packet) { inherited::load(input_packet); }

    virtual void UpdateCL();
    virtual void shedule_Update(u32 dt);

    virtual void InitThink() {}
    virtual void Think();
    virtual void reinit();
    virtual void reload(LPCSTR section);

    virtual void init() {}

    virtual void feel_sound_new(CObject* who, int eType, CSound_UserDataPtr user_data, const Fvector& Position, float power);
    virtual BOOL feel_vision_isRelevant(CObject* O);
    virtual BOOL feel_touch_on_contact(CObject* O);
    virtual BOOL feel_touch_contact(CObject*);

    virtual bool useful(const CItemManager* manager, const CGameObject* object) const;
    virtual float evaluate(const CItemManager* manager, const CGameObject* object) const;

    virtual void OnEvent(NET_Packet& P, u16 type);
    virtual void OnHUDDraw(CCustomHUD* hud, u32 context_id, IRenderable* root) override { return inherited::OnHUDDraw(hud, context_id, root); }
    virtual u16 PHGetSyncItemsNumber() { return inherited::PHGetSyncItemsNumber(); }
    virtual CPHSynchronize* PHGetSyncItem(u16 item) { return inherited::PHGetSyncItem(item); }
    virtual void PHUnFreeze() { return inherited::PHUnFreeze(); }
    virtual void PHFreeze() { return inherited::PHFreeze(); }
    virtual BOOL UsedAI_Locations() { return inherited::UsedAI_Locations(); }

    virtual const SRotation Orientation() const { return inherited::Orientation(); }
    virtual void renderable_Render(u32 context_id, IRenderable* root) override { return inherited::renderable_Render(context_id, root); }

    virtual void on_restrictions_change();

    virtual void SetAttackEffector();

    virtual void update_fsm();

    virtual void post_fsm_update();
    void squad_notify();

    virtual bool IsTalkEnabled() { return false; }

    virtual void HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type = ALife::eHitTypeWound, bool draw_hit_marks = true);
    virtual void HitEntityInJump(const CEntity* pEntity) {}

    virtual void on_before_sell(CInventoryItem* item);
    float GetSatiety() { return 0.5f; }
    void ChangeSatiety(float v) {}
    // ---------------------------------------------------------------------------------
    // Process scripts
    // ---------------------------------------------------------------------------------
    virtual bool bfAssignMovement(CScriptEntityAction* tpEntityAction);
    bool AssignGamePathIfNeeded(Fvector target_pos, u32 level_vertex);
    virtual bool bfAssignObject(CScriptEntityAction* tpEntityAction);
    virtual bool bfAssignWatch(CScriptEntityAction* tpEntityAction);
    virtual bool bfAssignAnimation(CScriptEntityAction* tpEntityAction);
    virtual bool bfAssignMonsterAction(CScriptEntityAction* tpEntityAction);
    virtual bool bfAssignSound(CScriptEntityAction* tpEntityAction);

    virtual void vfFinishAction(CScriptEntityAction* tpEntityAction);

    virtual void ProcessScripts();

    virtual CEntity* GetCurrentEnemy();
    virtual CEntity* GetCurrentCorpse();
    virtual int get_enemy_strength();

    virtual void SetScriptControl(const bool bScriptControl, shared_str caSciptName);

    virtual void SetEnemy(const CEntityAlive* sent);
    bool m_force_real_speed;
    bool m_script_processing_active;
    bool m_script_state_must_execute;

    virtual void jump(const Fvector& position, float factor) {}

    bool m_skip_transfer_enemy;
    IC void skip_transfer_enemy(bool value) { m_skip_transfer_enemy = value; }

    IC int Rank() { return m_rank; }

    //----------------------------------------------------------------------------------

    virtual void SetTurnAnimation(bool turn_left);

    // установка специфических анимаций
    virtual void CheckSpecParams(u32 /**spec_params/**/) {}
    virtual void ForceFinalAnimation() {}
    virtual void LookPosition(Fvector to_point, float angular_speed = PI_DIV_3); // каждый монстр может по-разному реализвать эту функ (e.g. кровосос с поворотом головы и т.п.)

    // Team
    virtual void ChangeTeam(int team, int squad, int group);

    // ---------------------------------------------------------------------------------
    // Abilities
    // ---------------------------------------------------------------------------------
    virtual bool ability_invisibility() { return false; }
    virtual bool ability_can_drag() { return false; }
    virtual bool ability_psi_attack() { return false; }
    virtual bool ability_earthquake() { return false; }
    virtual bool ability_can_jump() { return false; }
    virtual bool ability_distant_feel() { return false; }
    virtual bool ability_run_attack() { return false; }
    virtual bool ability_rotation_jump() { return false; }
    virtual bool ability_jump_over_physics() { return false; }
    virtual bool ability_pitch_correction() { return true; }
    // ---------------------------------------------------------------------------------

    virtual void event_on_step() {}
    virtual void on_threaten_execute() {}
    // ---------------------------------------------------------------------------------
    // Memory
    void UpdateMemory();

    // Cover
    bool GetCorpseCover(Fvector& position, u32& vertex_id);
    bool GetCoverFromEnemy(const Fvector& enemy_pos, Fvector& position, u32& vertex_id);
    bool GetCoverFromPoint(const Fvector& pos, Fvector& position, u32& vertex_id, float min_dist, float max_dist, float radius);
    bool GetCoverCloseToPoint(const Fvector& dest_pos, float min_dist, float max_dist, float deviation, float radius, Fvector& position, u32& vertex_id);

    // Movement Manager
protected:
    CControlPathBuilder* m_movement_manager;

protected:
    virtual CMovementManager* create_movement_manager();
    
    // --------------------------------------------------------------------------------------
    // Monster Settings
    SMonsterSettings m_current_settings;

    // members
public:
    void set_force_anti_aim(bool force_anti_aim) { m_force_anti_aim = force_anti_aim; }
    bool get_force_anti_aim() const { return m_force_anti_aim; }

    void settings_read(CInifile* ini, LPCSTR section, SMonsterSettings& data);
    void settings_load(LPCSTR section);
    void settings_overrides();

    SMonsterSettings& db() { return (m_current_settings); }
    // --------------------------------------------------------------------------------------

    CCharacterPhysicsSupport* m_pPhysics_support;

    CMonsterCorpseCoverEvaluator* m_corpse_cover_evaluator;
    CCoverEvaluatorFarFromEnemy* m_enemy_cover_evaluator;
    CCoverEvaluatorCloseToEnemy* m_cover_evaluator_close_point;

    // ---------------------------------------------------------------------------------
    IStateManagerBase* StateMan;
    // ---------------------------------------------------------------------------------

    CMonsterEnemyMemory EnemyMemory;
    CMonsterSoundMemory SoundMemory;
    CMonsterCorpseMemory CorpseMemory;
    CMonsterHitMemory HitMemory;

    CMonsterEnemyManager EnemyMan;
    CMonsterCorpseManager CorpseMan;

    const CEntityAlive* EatedCorpse;
    // Lain: added
    bool check_eated_corpse_draggable();
    virtual bool is_base_monster_with_enemy() { return EnemyMan.get_enemy() != NULL; }

    bool hear_dangerous_sound;
    bool hear_interesting_sound;

    // -----------------------------------------------------------------------------
    CMonsterEventManager EventMan;
    // -----------------------------------------------------------------------------

    CMeleeChecker MeleeChecker;
    CMonsterMorale Morale;

    // -----------------------------------------------------------------------------

    CMonsterCoverManager* CoverMan;

    // -----------------------------------------------------------------------------

    CControlledEntityBase* m_controlled;

    // -----------------------------------------------------------------------------
    enum EMonsterType
    {
        eMonsterTypeUniversal = u32(0),
        eMonsterTypeIndoor,
        eMonsterTypeOutdoor,
    } m_monster_type;

    // -----------------------------------------------------------------------------
    // Home
    CMonsterHome* Home;

private:
    bool m_force_anti_aim;

    //	//-----------------------------------------------------------------
    //	// Spawn Inventory Item
    //	//-----------------------------------------------------------------
    // private:
    //	LPCSTR					m_item_section;
    //	float					m_spawn_probability;

    //--------------------------------------------------------------------
    // Berserk
    //--------------------------------------------------------------------
public:
    bool script_berserk{};
    bool berserk_always;

    //--------------------------------------------------------------------
    // Panic Threshold (extension for scripts)
    //--------------------------------------------------------------------

    float m_default_panic_threshold;
    IC void set_custom_panic_threshold(float value);
    IC void set_default_panic_threshold();
    //--------------------------------------------------------------------

    //////////////////////////////////////////////////////////////////////////
    // -----------------------------------------------------------------------------
    // Special Services (refactoring needed)

    void on_kill_enemy(const CEntity* obj);
    void Hit_Psy(CObject* object, float value);
    void Hit_Wound(CObject* object, float value, const Fvector& dir, float impulse);
    CParticlesObject* PlayParticles(const shared_str& name, const Fvector& position, const Fvector& dir, BOOL auto_remove = TRUE, BOOL xformed = TRUE);
    void load_effector(LPCSTR section, LPCSTR line, SAttackEffector& effector);

    // --------------------------------------------------------------------------------------
    // Kill From Here
    // --------------------------------------------------------------------------------------
    // State flags
    bool m_bDamaged;
    bool m_bAngry;
    bool m_bGrowling;
    bool m_bAggressive;
    bool m_bSleep;
    bool m_bRunTurnLeft;
    bool m_bRunTurnRight;

    void set_aggressive(bool val = true) { m_bAggressive = val; }

    //---------------------------------------------------------------------------------------

    u32 m_prev_sound_type;
    u32 get_attack_rebuild_time();

    IC virtual EAction CustomVelocityIndex2Action(u32 velocity_index) { return ACT_STAND_IDLE; }
    virtual void TranslateActionToPathParams();

    bool state_invisible;

    void set_action(EAction action);
    void set_state_sound(u32 type, bool once = false);
    IC void fall_asleep() { m_bSleep = true; }
    IC void wake_up() { m_bSleep = false; }

    // Temp
    u32 m_time_last_attack_success;
    int m_rank;
    float m_melee_rotation_factor;

private:
    bool ignore_collision_hit;

public:
    IC void set_ignore_collision_hit(bool value) { ignore_collision_hit = value; }
    // -----------------------------------------------------------------------------
    //////////////////////////////////////////////////////////////////////////

public:
    CControl_Manager& control() { return (*m_control_manager); }

    CControlAnimationBase& anim() { return (*m_anim_base); }
    CControlMovementBase& move() { return (*m_move_base); }
    CControlPathBuilderBase& path() { return (*m_path_base); }
    CControlDirectionBase& dir() { return (*m_dir_base); }

    CControlManagerCustom& com_man() { return m_com_manager; }

    virtual bool check_start_conditions(ControlCom::EControlType);
    virtual void on_activate_control(ControlCom::EControlType) {}

protected:
    CControl_Manager* m_control_manager;

    CControlAnimationBase* m_anim_base;
    CControlMovementBase* m_move_base;
    CControlPathBuilderBase* m_path_base;
    CControlDirectionBase* m_dir_base;

    CControlManagerCustom m_com_manager;

    virtual void create_base_controls();

    //////////////////////////////////////////////////////////////////////////
    // Critical Wounded
    //////////////////////////////////////////////////////////////////////////
    enum
    {
        critical_wound_type_head = u32(0),
        critical_wound_type_torso,
        critical_wound_type_legs
    };

    virtual void load_critical_wound_bones();
    virtual bool critical_wound_external_conditions_suitable();
    virtual void critical_wounded_state_start();

    void fill_bones_body_parts(LPCSTR body_part, CriticalWoundType wound_type);

    LPCSTR m_critical_wound_anim_head{};
    LPCSTR m_critical_wound_anim_torso{};
    LPCSTR m_critical_wound_anim_legs{};

    //////////////////////////////////////////////////////////////////////////
    // NLC: external stagger and movement slow (pseudogiant stomp)
    //////////////////////////////////////////////////////////////////////////
private:
    s8 m_nlc_stagger_state{-1}; // -1 = not resolved, 0 = no usable anim, 1 = m_nlc_stagger_anim valid
    LPCSTR m_nlc_stagger_anim{};
    float m_nlc_slow_k{};
    u32 m_nlc_slow_start{};
    u32 m_nlc_slow_end{};
    float m_nlc_speed_base{1.f}; // section key move_speed_k (movement + clip speed together)
    float m_nlc_speed_bonus{1.f}; // persistent multiplier (pseudogiant rage)
    float m_nlc_haste_k{1.f}; // temporary multiplier (pseudogiant charge)
    u32 m_nlc_haste_end{};

public:
    void nlc_set_speed_bonus(float k) { m_nlc_speed_bonus = k; }
    void nlc_set_speed_base(float k) { m_nlc_speed_base = k; }
    void nlc_set_haste(float k, u32 time_ms)
    {
        m_nlc_haste_k = k;
        m_nlc_haste_end = Device.dwTimeGlobal + time_ms;
    }
    void nlc_clear_haste() { m_nlc_haste_end = 0; }
    // forces the damaged walk/run (clips + velocities) for time_ms, on top of the DamagedThreshold health rule
    void nlc_force_damaged(u32 time_ms) { m_nlc_damaged_until = Device.dwTimeGlobal + time_ms; }
    // attack run substate: path to this point instead of the enemy (e.g. a locked charge line)
    virtual bool nlc_run_target_override(Fvector& position, u32& vertex) { return false; }

private:
    u32 m_nlc_damaged_until{};

public:
    // called from Hit with the health actually lost (fraction of max health)
    virtual void nlc_on_health_lost(const CObject* who, float fraction) {}
    // plays the species' critical-hit animation (torso, legs, head key order), ignoring the threshold
    bool nlc_stagger(float speed_k = 1.f);
    // plays the stagger `count` times in a row (controller thrall release), speed_k < 1 = slower
    void nlc_stagger_repeat(u32 count, float speed_k);
    void nlc_update_stagger_repeat();

private:
    u32 m_nlc_stagger_left{};
    float m_nlc_stagger_speed{1.f};
    u32 m_nlc_stagger_until{}; // a stagger that cannot start by then (running, busy sequencer) is dropped

public:
    // k = fraction of speed removed (0..0.9), fades out over the last quarter of time_ms
    void nlc_apply_move_slow(float k, u32 time_ms);
    float nlc_move_speed_k() const;

    // threaten (stomp) control hooks: skip the facing check / scale the animation speed
    virtual bool threaten_skip_facing() { return false; }
    virtual float threaten_anim_speed_k() { return 1.f; }

    //////////////////////////////////////////////////////////////////////////

public:
    virtual void on_attack_on_run_hit() {}

//////////////////////////////////////////////////////////////////////////
// DEBUG stuff
#ifdef DEBUG
public:
    struct SDebugInfo
    {
        bool active;
        float x;
        float y;
        float delta_y;
        u32 color;
        u32 delimiter_color;

        SDebugInfo() : active(false) {}
        SDebugInfo(float px, float py, float dy, u32 c, u32 dc) : active(true), x(px), y(py), delta_y(dy), color(c), delimiter_color(dc) {}
    };

    u8 m_show_debug_info; // 0 - none, 1 - first column, 2 - second column
    void set_show_debug_info(u8 show = 1) { m_show_debug_info = show; }
    virtual SDebugInfo show_debug_info();

    void debug_fsm();
#endif

#ifdef DEBUG
    virtual void debug_on_key(int key) {}
#endif
    //////////////////////////////////////////////////////////////////////////

public:
    bool is_jumping();

    //-------------------------------------------------------------------
    // CBaseMonster's      Steering Behaviour
    //-------------------------------------------------------------------
public:
    steering_behaviour::manager* get_steer_manager();

    float get_feel_enemy_who_just_hit_max_distance() { return m_feel_enemy_who_just_hit_max_distance; }
    float get_feel_enemy_who_made_sound_max_distance() { return m_feel_enemy_who_made_sound_max_distance; }
    float get_feel_enemy_max_distance() { return m_feel_enemy_max_distance; }
    virtual bool can_use_agressive_jump(const CObject*) { return false; }

private:
    steering_behaviour::manager* m_steer_manager;
    squad_grouping_behaviour* m_grouping_behaviour; // freed by manager

    void update_enemy_accessible_and_at_home_info();
    // updates position by applying little "pushing" force
    // so that monsters rarely intersect
    void update_pos_by_grouping_behaviour();
    TTime m_last_grouping_behaviour_update_tick;

    float m_feel_enemy_who_made_sound_max_distance;
    float m_feel_enemy_who_just_hit_max_distance;
    float m_feel_enemy_max_distance;

    //-------------------------------------------------------------------
    // CBaseMonster's  Atack on Move Parameters
    //-------------------------------------------------------------------
public:
    struct attack_on_move_params_t
    {
        bool enabled;
        float max_go_close_time;
        float far_radius;
        float prepare_radius;
        float prepare_time;
        float attack_radius;
        float update_side_period;
        float prediction_factor;
        float close_melee_dist; // NLC: "aom_close_melee_dist", 0 = off
    };

    bool can_attack_on_move();
    // NLC: attack-on-move monsters fall back to the standing melee state when the enemy is
    // closer than aom_close_melee_dist (or keep it once started); classic melee then ends past MaxAttackDist
    bool aom_close_melee_allowed(bool continuing);
    float get_attack_on_move_max_go_close_time();
    float get_attack_on_move_far_radius();
    float get_attack_on_move_attack_radius();
    float get_attack_on_move_update_side_period();
    float get_attack_on_move_prediction_factor();
    float get_attack_on_move_prepare_radius();
    float get_attack_on_move_prepare_time();

    bool enemy_accessible();
    bool at_home();

protected:
    attack_on_move_params_t m_attack_on_move_params;

    //-------------------------------------------------------------------
    // CBaseMonster's  Auras
    //-------------------------------------------------------------------
public:
    float get_psy_influence();
    float get_radiation_influence();
    float get_fire_influence();
    void play_detector_sound();

    enum EAuraType
    {
        eAuraTypeBase = 0,
        eAuraTypePsy,
        eAuraTypeFire,
        eAuraTypeRad,
    };

    void enable_aura(EAuraType aura_type, bool enable);
    bool is_aura_enabled(EAuraType aura_type);

private:
    monster_aura m_psy_aura;
    monster_aura m_radiation_aura;
    monster_aura m_fire_aura;
    monster_aura m_base_aura;

protected:
    //-------------------------------------------------------------------
    // CBaseMonster's  Anti-Aim Ability
    //-------------------------------------------------------------------
    anti_aim_ability* m_anti_aim;

private:
    pcstr m_head_bone_name;
    pcstr m_left_eye_bone_name;
    pcstr m_right_eye_bone_name;

public:
    pcstr get_head_bone_name() const { return m_head_bone_name; }
    anti_aim_ability* get_anti_aim() { return m_anti_aim; }

private:
    void update_eyes_visibility();
    float get_screen_space_coverage_diagonal();

    void GenerateNewOffsetFromLeader();
    u32 m_offset_from_leader_chosen_tick;
    Fvector m_offset_from_leader;

    // very special copies, used when pos is not on ai-map
    // in that situation m_action_target_node is close node
    Fvector m_action_target_pos;
    u32 m_action_target_node;

    TTime m_first_tick_enemy_inaccessible;
    TTime m_last_tick_enemy_inaccessible;
    TTime m_first_tick_object_not_at_home;

public:
    // NLC: why the current enemy counts as inaccessible (last update), for diagnostics
    enum ENlcInaccessible : u8
    {
        eNlcAccessible = 0,
        eNlcInaccessibleHigh, // far above/below its ai-map vertex (roof, ledge)
        eNlcInaccessibleOffMap, // more than 1.2 m off its ai-map vertex (crate, vehicle)
        eNlcInaccessibleHome, // outside this monster's home restrictor
        eNlcInaccessibleRestrictor, // no accessible point within 1.5 m (restrictor, anomaly)
        eNlcInaccessibleVertexPos,
        eNlcInaccessibleVertexId,
    };
    u8 nlc_inaccessible_reason() const { return m_nlc_inaccessible_reason; }

private:
    u8 m_nlc_inaccessible_reason{eNlcAccessible};

public:
    virtual bool run_home_point_when_enemy_inaccessible() const { return true; }
    virtual bool need_shotmark() const { return true; }

    //////////////////////////////////////////////////////////////////////////
    // NLC: siege of an elevated enemy (docs/DESIGN_monster_elevation_siege.md). Config
    // [monster_elevation] in game_relations.ltx, the same keys override per monster section.
    //////////////////////////////////////////////////////////////////////////
public:
    // species without the attack substate or with their own logic return false
    virtual bool nlc_siege_capable() const { return true; }
    // the attack state machine owns a siege substate (CStateMonsterAttackSiege)
    void nlc_siege_register() { m_nlc_siege_supported = true; }
    bool nlc_siege_wanted() const { return m_nlc_siege.want; }
    bool nlc_siege_active() const { return m_nlc_siege.active; }
    void nlc_siege_begin();
    void nlc_siege_execute();
    void nlc_siege_end(bool clear_anim = true);
    // hooks: a heard sound (feel_sound_new, before the faint-shot return), a near miss and a bolt
    // (nlc_stealth, main thread), a squad mate died (Die)
    void nlc_siege_on_noise(const CObject* who, int sound_type, const Fvector& position, float power);
    void nlc_siege_on_near_miss(const Fvector& origin);
    bool nlc_siege_on_bolt(const Fvector& position);
    void nlc_siege_on_pack_loss();

    // a point on the ai-map reachable for this monster at or near `wanted` (also used by the controller)
    bool nlc_point_on_map(const Fvector& wanted, Fvector& pos, u32& node) const;
    // static-geometry line from `feet` + eye_height to the enemy's chest
    bool nlc_los_to(const Fvector& feet, const CEntityAlive* enemy, float eye_height) const;
    // the siege's hidden test for species tactics: can the enemy see a monster standing at `feet`
    bool nlc_hidden_test_visible(const Fvector& feet, const CEntityAlive* enemy) const { return nlc_siege_visible(feet, enemy); }
    // a bloodsucker's ambush hold: siege active, not fled, holding a hidden point or moving there unseen
    bool nlc_siege_hidden_hold() const;
    bool nlc_siege_perch() const { return m_nlc_siege.active && m_nlc_siege.perch; } // NLC: holding a perch-strike spot
    // NLC: arrived at the perch-strike spot and holding it
    bool nlc_siege_perch_holding() const { return m_nlc_siege.active && m_nlc_siege.perch && m_nlc_siege.move == eNlcSiegeHold; }
    // NLC: leave the perch-strike spot: the siege picks a hidden point next (the species blocks the perch strike itself)
    void nlc_siege_perch_abandon();
    // NLC: a species that can jump up at a perched enemy goes to strike_dist from its ground point instead of hiding
    virtual bool nlc_siege_perch_strike(const CEntityAlive* enemy, float& strike_dist) { return false; }
    // NLC: shot at the perch-strike spot: the species stops using it for a while
    virtual void nlc_siege_perch_shot() {}
    // NLC: CControlJump ignores velocity bounces this long after takeoff (0 = stock)
    virtual u32 nlc_jump_bounce_grace() const { return 0; }
    // turn toward a point with the standing turn clips (the siege's facing, for species tactics)
    void nlc_face_point(const Fvector& point) { nlc_siege_face(point); }
    // NLC: the attack state machine owns a tactic substate (CStateMonsterAttackTactic); species override the hooks
    virtual bool nlc_tactic_wanted() { return false; }
    virtual void nlc_tactic_begin() {}
    virtual void nlc_tactic_execute() {}
    virtual void nlc_tactic_end() {}

private:
    enum ENlcSiegeMove : u8
    {
        eNlcSiegeNone,
        eNlcSiegeGoto, // to a hidden point
        eNlcSiegeHold, // at a hidden point
        eNlcSiegePeek, // watcher: to / at a point with line of sight
        eNlcSiegeRetreat, // shot: out of sight, farther away
        eNlcSiegeDistract, // bolt: go and look
        eNlcSiegeFlee, // pack losses: far away, no lock
    };

    struct SNlcSiegeParams
    {
        bool enabled{};
        bool vs_npc{};
        bool distractible{};
        u32 grace_first{3000};
        u32 grace_repeat{1000};
        float reengage_close{10.f};
        Fvector reengage_delay{3000.f, 6000.f, 10000.f};
        u32 toggle_decay{60000};
        u32 min_episode{1000};
        float reach_height{1.6f};
        u32 reach_timeout{4000};
        float eye_height{1.f};
        Fvector2 ambush_dist{8.f, 20.f};
        u32 ambush_time{45000};
        Fvector2 watch_dist{20.f, 35.f};
        u32 watch_count{1};
        Fvector2 peek_interval{15000.f, 25000.f};
        u32 peek_time{2500};
        float peek_detect_k{1.5f}; // vision rate factor while a watcher peeks
        Fvector2 hold_time{6000.f, 12000.f};
        Fvector2 growl_interval{8000.f, 15000.f};
        float lock_radius{10.f};
        u32 patience{75000};
        u32 faint_patience{30000};
        u32 distract_after{10000};
        float retreat_dist{28.f};
        u32 retreat_time{7000};
        u32 max_time{600000};
        u32 flee_losses{}; // 0 = never
        EAction gait{ACT_RUN};
        EAction rest{ACT_STAND_IDLE};
        bool pick_near_anchor{}; // ambush placement: the hidden point nearest the enemy's ground point wins (elev_pick_near_anchor)
    };

    struct SNlcSiege
    {
        // tracking, every UpdateCL
        u16 enemy_id{u16(-1)};
        bool want{};
        bool owns{}; // elevation case handled by the siege: stock enemy_accessible() stays true
        u32 elev_since{};
        u32 ground_since{};
        u32 reach_since{};
        u32 reach_block_until{}; // low-perch attack given up: not again before this
        u32 toggles{};
        u32 toggle_time{};
        LPCSTR end_reason{};
        // siege
        bool active{};
        bool watch{}; // Watch phase (else Ambush)
        bool watcher{};
        bool patience_over{};
        bool max_logged{};
        bool fled{};
        bool sneak{}; // this move uses the species gait (started out of sight)
        bool escape{}; // side guard: a radial escape move; on arrival the monster reselects at once
        bool perch{}; // NLC: a perch-strike spot (nlc_siege_perch_strike): visible on purpose, no exposure reselect
        bool fallback{}; // holding a visible point (no hidden one found): lie low, no exposure checks
        bool peek_boost{}; // m_nlc_rate_k raised for this peek
        u32 diag_until{}; // debug: log which state interrupted the siege
        u32 start{};
        u32 last_exec{}; // a siege not executed for 2 s was left without finalize: ended by tracking
        u32 contact{};
        Fvector anchor{};
        u32 anchor_node{u32(-1)};
        u32 pack_losses{};
        // movement
        ENlcSiegeMove move{eNlcSiegeNone};
        bool reselect{};
        Fvector target{};
        u32 target_node{u32(-1)};
        u32 locked_node{u32(-1)};
        u32 move_until{};
        u32 hold_until{};
        u32 next_los{};
        u32 next_peek{};
        u32 next_growl{};
        u32 next_watcher{};
        u32 next_near_miss{};
        u32 hit_seen{};
        u32 burned[3]{u32(-1), u32(-1), u32(-1)};
        u8 burned_next{};
    };

    void nlc_siege_load(LPCSTR section);
    void nlc_siege_track();
    void nlc_siege_reset();
    bool nlc_siege_pick(ENlcSiegeMove kind, LPCSTR why);
    void nlc_siege_set_target(const Fvector& pos, u32 node, ENlcSiegeMove kind, u32 timeout);
    void nlc_siege_hold(u32 time_ms);
    void nlc_siege_burn_target();
    void nlc_siege_unlock();
    void nlc_siege_update_watcher();
    void nlc_siege_face(const Fvector& point);
    // can the enemy see a monster standing at `feet` (head and both body ends; hidden only if all are blocked)
    bool nlc_siege_visible(const Fvector& feet, const CEntityAlive* enemy) const;
    bool nlc_siege_in_ring(const Fvector& pos) const;

    SNlcSiegeParams m_nlc_siege_p;
    SNlcSiege m_nlc_siege;
    bool m_nlc_siege_supported{};
    static bool s_nlc_siege_debug;

    //////////////////////////////////////////////////////////////////////////
    // NLC: evasion pass (docs/DESIGN_monster_movement_under_fire.md): threat test, zigzag approach, side
    // guard, dodge speed. Config [monster_evasion] in game_relations.ltx, the same keys override per monster
    // section; every feature has its own key and defaults to off.
    //////////////////////////////////////////////////////////////////////////
public:
    enum ENlcThreat : u8
    {
        eThreatWatched = 1, // the enemy looks toward this monster and has a clear line to it
        eThreatArmed = 2, // the enemy holds a gun (no knife, no binoculars)
        eThreatAimed = 4, // armed and the crosshair cone is on this monster
        eThreatLocked = 8, // actor: the crosshair ray hits this monster
    };

    // flags above, recomputed at most every 200 ms; 0 without an enemy or with evade_enabled off
    u8 nlc_threat();
    // eye position and view direction of an enemy (actor camera, NPC head and Direction())
    bool nlc_enemy_eye(const CEntityAlive* enemy, Fvector& eye, Fvector& view) const;
    // active item is a CWeapon, not a knife or binoculars
    bool nlc_enemy_armed(const CEntityAlive* enemy) const;
    // zigzag: replaces the charge target and returns true while a leg is active
    bool nlc_zz_target(Fvector& pos, u32& node);
    // an enemy shot heard, a near miss or a hit: the next leg goes to the other side
    void nlc_zz_on_fire();
    // side guard and the threat test says watched
    bool nlc_side_guard_watched();
    // would a move from here to `pt` cross the line of sight of a watching enemy (side guard)
    bool nlc_guard_lateral(const Fvector& pt, const CEntityAlive* enemy) const;
    // lateral sampler (diagnostics), after nlc_siege_track
    void nlc_evade_update();

    // timed speed factor on top of nlc_move_speed_k (zigzag legs, lunge); not the giant's haste slot
    void nlc_set_dodge(float k, u32 time_ms)
    {
        m_nlc_evade.dodge_k = k;
        m_nlc_evade.dodge_end = Device.dwTimeGlobal + time_ms;
    }
    void nlc_clear_dodge() { m_nlc_evade.dodge_end = 0; }
    // the dodge factor now (1 = none); the heading speed follows it so the turn radius stays as planned
    float nlc_dodge_k() const { return Device.dwTimeGlobal < m_nlc_evade.dodge_end ? m_nlc_evade.dodge_k : 1.f; }
    float nlc_run_attack_haste_k() const { return m_nlc_evade_p.enabled ? m_nlc_evade_p.run_attack_haste_k : 1.f; }

    bool nlc_evade_enabled() const { return m_nlc_evade_p.enabled; }
    bool nlc_side_guard_on() const { return m_nlc_evade_p.enabled && m_nlc_evade_p.side_guard; }
    float nlc_watch_cone() const { return m_nlc_evade_p.watch_cone; }
    u32 nlc_last_watched() const { return m_nlc_evade.last_watched; }
    static bool nlc_evade_debug() { return s_nlc_evade_debug; }

    // bloodsucker: a cloaked monster keeps its sneak gait for an edge approach (TranslateActionToPathParams)
    virtual bool nlc_cloak_keeps_gait() { return false; }

private:
    struct SNlcEvadeParams
    {
        bool enabled{true};
        float watch_cone{deg2rad(55.f)};
        float aim_angle{deg2rad(12.f)};
        bool zz_enabled{};
        int zz_need_gun{}; // 0 watched, 1 watched and armed, 2 aimed, 3 aiming down sights or crosshair lock
        u32 zz_react_ms{}; // the trigger must hold this long before the first leg (reaction time)
        Fvector2 zz_dist{8.f, 40.f};
        float zz_speed_k{1.2f};
        float zz_max_angle{deg2rad(35.f)};
        Fvector2 zz_leg_time{500.f, 900.f};
        float zz_aim_leg_k{0.7f};
        float zz_lock_leg_k{0.5f};
        float zz_same_side{0.25f};
        bool zz_fire_switch{true};
        bool zz_vs_npc{};
        u32 zz_seen_grace{800}; // ms the "monster sees the enemy" gate holds after the last sighting (vision flickers)
        float zz_chance{1.f}; // chance that a burst of legs starts (rolled per burst)
        Fvector2 zz_burst{0.f, 0.f}; // legs per burst (0, 0 = continuous zigzag)
        Fvector2 zz_rest{0.f, 0.f}; // ms of straight run between bursts (and after a failed roll)
        float zz_turn_share{0.5f}; // the side switch (2 x leg angle) must fit in this share of a leg at the run turn rate
        float zz_min_leg_k{1.5f}; // a leg shorter than this x the run turn radius runs straight instead
        float zz_heading_gate{deg2rad(60.f)}; // no new leg while the body heads more than this away from the enemy
        bool side_guard{};
        float side_guard_angle{deg2rad(30.f)};
        float side_guard_short{3.f};
        float run_attack_haste_k{1.f};
    };

    struct SNlcEvade
    {
        u16 enemy_id{u16(-1)};
        // threat cache
        u32 threat_next{};
        u8 threat{};
        u32 last_watched{};
        // zigzag
        bool zz_on{};
        bool zz_leg{}; // a leg is planned
        bool zz_flip{}; // fire switch pending: the next leg goes to the other side
        s8 zz_side{}; // +1 / -1, 0 = no leg yet
        u32 zz_threat_until{}; // the threat gate holds for 1 s after it last passed
        u32 zz_seen_until{}; // the sight gate holds until then (zz_seen_grace)
        u32 zz_ok_since{}; // the trigger holds since (zz_react_ms), 0 = not now
        u32 zz_legs_left{}; // legs left in the current burst (0 = none: rest or roll)
        u32 zz_rest_until{}; // straight run until then (between bursts)
        u32 zz_leg_start{};
        u32 zz_leg_end{};
        Fvector zz_target{};
        u32 zz_node{u32(-1)};
        u32 zz_log_next{};
        // dodge speed factor
        float dodge_k{1.f};
        u32 dodge_end{};
        // lateral sampler
        u32 next_sample{};
        u32 lateral_since{};
        float lateral_max{};
    };

    void nlc_evade_load(LPCSTR section);
    void nlc_evade_reset();
    // resets the runtime state when the enemy changed; true with an enemy
    bool nlc_evade_sync(const CEntityAlive* enemy);
    void nlc_zz_stop(const char* reason);

    SNlcEvadeParams m_nlc_evade_p;
    SNlcEvade m_nlc_evade;
    static bool s_nlc_evade_debug;
};

#include "base_monster_inline.h"
