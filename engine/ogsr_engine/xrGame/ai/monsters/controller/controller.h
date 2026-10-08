#pragma once
#include "../BaseMonster/base_monster.h"
#include "../anim_triple.h"
#include "script_export_space.h"
#include "../controlled_actor.h"

class CControllerAnimation;
class CControllerDirection;
class SndShockEffector;
class CControllerPsyHit;
class CControllerAura;

class CController : public CBaseMonster, public CControlledActor
{
    typedef CBaseMonster inherited;

    u8 m_max_controlled_number;
    ref_sound control_start_sound; // звук, который играется в голове у актера
    ref_sound control_hit_sound; // звук, который играется в голове у актера

    ref_sound m_sound_hit_fx;
    SndShockEffector* m_sndShockEffector;

    SAttackEffector m_control_effector;

    u32 time_control_hit_started;
    bool active_control_fx;

    LPCSTR particles_fire;

    CControllerAnimation* m_custom_anim_base;
    CControllerDirection* m_custom_dir_base;

    u32 m_psy_fire_start_time;
    u32 m_psy_fire_delay;

    bool m_tube_at_once;

public:
    float aura_radius;
    float aura_damage;

    //////////////////////////////////////////////////////////////////////////

public:
    CControllerPsyHit* m_psy_hit;

    ref_sound m_sound_aura_left_channel;
    ref_sound m_sound_aura_right_channel;
    ref_sound m_sound_aura_hit_left_channel;
    ref_sound m_sound_aura_hit_right_channel;

    ref_sound m_sound_tube_start;
    ref_sound m_sound_tube_pull;
    ref_sound m_sound_tube_hit_left;
    ref_sound m_sound_tube_hit_right;

    ref_sound m_sound_tube_prepare;

public:
    SVelocityParam m_velocity_move_fwd;
    SVelocityParam m_velocity_move_bkwd;

public:
    CControllerAnimation& custom_anim() { return (*m_custom_anim_base); }
    CControllerDirection& custom_dir() { return (*m_custom_dir_base); }

public:
    xr_vector<CEntity*> m_controlled_objects;

public:
    CController();
    virtual ~CController();

    virtual void Load(LPCSTR section);
    virtual void reload(LPCSTR section);
    virtual void reinit();
    virtual void UpdateCL();
    virtual void shedule_Update(u32 dt);
    virtual void Die(CObject* who);

    virtual void net_Destroy();
    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Relcase(CObject* O);

    virtual void CheckSpecParams(u32 spec_params);
    virtual void InitThink();

    virtual void create_base_controls();

    virtual const MonsterSpace::SBoneRotation& head_orientation() const;

    virtual void TranslateActionToPathParams();

    virtual bool ability_pitch_correction() { return false; }

    //-------------------------------------------------------------------

    virtual bool is_relation_enemy(const CEntityAlive* tpEntityAlive) const;
    xr_vector<shared_str> m_friend_community_overrides;
    void load_friend_community_overrides(LPCSTR section);
    bool is_community_friend_overrides(const CEntityAlive* tpEntityAlive) const;
    //-------------------------------------------------------------------
    // Controller ability
    bool HasUnderControl() { return (!m_controlled_objects.empty()); }
    void TakeUnderControl(CEntity*);
    void UpdateControlled();
    void FreeFromControl();
    void OnFreedFromControl(const CEntity*); // если монстр сам себя освободил (destroyed || die)

    void set_controlled_task(u32 task);

    void play_control_sound_start();
    void play_control_sound_start_at(const CEntityAlive* target); // NLC: recruit channel
    void play_control_sound_hit();

    void control_hit();

    void psy_fire();
    bool can_psy_fire();

    void tube_fire();
    bool can_tube_fire();

    float m_tube_damage;
    u32 m_tube_condition_see_duration;
    u32 m_tube_condition_min_delay;
    float m_tube_condition_min_distance;

    void set_psy_fire_delay_zero();
    void set_psy_fire_delay_default();

    float get_tube_min_distance() const { return m_tube_condition_min_distance; }
    bool tube_ready() const;

    //-------------------------------------------------------------------

public:
    void draw_fire_particles();

    void test_covers();

public:
    enum EMentalState
    {
        eStateIdle,
        eStateDanger
    } m_mental_state;

    void set_mental_state(EMentalState state);
    virtual void HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type, bool draw_hit_marks);
    virtual void Hit(SHit* pHDS); // NLC: a Mirage decoy vanishes on any hit

public:
    virtual bool use_center_to_aim() const { return true; }

    SAnimationTripleData anim_triple_control;

private:
    float m_stamina_hit;

    /*
    #ifdef DEBUG
        virtual CBaseMonster::SDebugInfo show_debug_info();

    #endif

    private:
    #ifdef DEBUG
            virtual void	debug_on_key		(int key);

            Fvector			P1,P2;
    #endif
    */

public:
    virtual bool run_home_point_when_enemy_inaccessible() const { return false; }
    virtual bool nlc_siege_capable() const { return false; } // NLC: own reposition loop

    //////////////////////////////////////////////////////////////////////////
    // NLC: controller redesign (docs/CONTROLLER_REDESIGN.md). Every key is optional;
    // without the nlc_ctrl_* keys the controller behaves as in stock OGSR.
    //////////////////////////////////////////////////////////////////////////
public:
    enum ENlcRanged : u8
    {
        eNlcRangedStock, // stock approach / melee
        eNlcRangedMove, // moving to a cover or peek point
        eNlcRangedHold, // the caller stands and faces the enemy
    };

    // attack state: ranged behavior (cover loop, or reposition when the enemy is inaccessible)
    ENlcRanged nlc_ranged_execute(bool enemy_inaccessible);
    void nlc_reposition_stop(LPCSTR why = "attack over");
    // running to cover: melee only when the enemy is right on top of it
    bool nlc_keep_ranged();
    // aiming from a peek point with a clear line to the actor seen moments ago: the tube may fire
    // without waiting for the stealth vision build-up (a psionic senses its target)
    bool nlc_psy_sense() const { return m_nlc_aim_los; }

    // script access (thrall save/load persistence, via CCustomMonster)
    void nlc_thrall_ids(xr_string& out) const;
    bool nlc_script_take(CEntityAlive* thrall);
    // script access (bind_monster mind kit): the ranged phase, and the herd call (every thrall attacks)
    LPCSTR nlc_script_phase() const;
    u32 nlc_herd_call(u32 duration_ms);

private:
    enum ENlcPhase : u8
    {
        eNlcPhaseOff,
        eNlcPhaseHide, // moving to cover
        eNlcPhaseCamp, // waiting in cover
        eNlcPhasePeek, // moving to a point with line of sight (or into tube range)
        eNlcPhaseAim, // waiting at that point for the tube
        eNlcPhaseHold, // nothing better found, stand
    };

    enum ENlcRole : u8
    {
        eNlcRoleHunt,
        eNlcRoleFlank,
        eNlcRoleGuard,
    };

    struct SNlcThrall
    {
        ENlcRole role{eNlcRoleHunt};
        float weight{1.f};
        s8 side{1}; // flank side
        bool guard_engaged{}; // guard biting the enemy (hysteresis 4 m in, 7 m out)
        bool guard_capable{}; // species listed in nlc_ctrl_guard_species (can be promoted to guard)
        bool flank_done{};
        u32 flank_until{};
        Fvector flank_point{};
        u32 flank_node{u32(-1)};
    };

    void nlc_load(LPCSTR section);
    void nlc_reinit();

    // thralls
    void nlc_take(CEntityAlive* thrall, LPCSTR how, bool stagger = true);
    bool nlc_can_take(const CEntityAlive* entity) const;
    float nlc_thrall_weight(const CEntity* entity) const;
    float nlc_thralls_weight() const;
    void nlc_release(CEntity* thrall, bool stagger, LPCSTR why);
    void nlc_release_all(bool stagger);
    void nlc_update_thrall_tasks();
    void nlc_update_recruit();

    // a reachable point for `monster` at or near `wanted` (accessible_nearest only accepts inaccessible points)
    bool nlc_resolve_point(CBaseMonster* monster, const Fvector& wanted, Fvector& pos, u32& node) const;
    void nlc_log_thralls(const CEntityAlive* enemy);

    // ranged
    bool nlc_has_los(const Fvector& feet, const CEntityAlive* enemy) const;
    bool nlc_set_target(Fvector pos, u32 node, ENlcPhase phase, LPCSTR kind);
    bool nlc_select_cover();
    bool nlc_select_peek();
    bool nlc_select_range();
    void nlc_set_phase_hold(ENlcPhase phase, u32 time_ms);

    bool m_nlc_debug{}; // nlc_ctrl_debug

    bool m_nlc_thrall_attack{}; // nlc_ctrl_thrall_attack
    u32 m_nlc_thrall_memory{}; // nlc_ctrl_thrall_memory, ms since the controller last saw its enemy
    float m_nlc_budget{-1.f}; // nlc_ctrl_thrall_budget, < 0 = no weight limit
    shared_str m_nlc_thrall_table; // nlc_ctrl_thrall_table: species = weight, role
    xr_vector<shared_str> m_nlc_guard_species; // nlc_ctrl_guard_species
    u32 m_nlc_guard_max{}; // nlc_ctrl_guard_max
    float m_nlc_guard_dist{}; // nlc_ctrl_guard_dist, from the controller toward the enemy
    float m_nlc_flank_offset{}; // nlc_ctrl_flank_offset
    float m_nlc_leash{}; // nlc_ctrl_leash_dist, 0 = off
    u32 m_nlc_release_staggers{}; // nlc_ctrl_release_stagger = count, speed
    u32 m_nlc_take_staggers{}; // nlc_ctrl_take_stagger = count, speed (the moment of domination)
    float m_nlc_take_stagger_speed{1.f};
    float m_nlc_release_stagger_speed{1.f};
    float m_nlc_release_slow_k{}; // nlc_ctrl_release_slow = k, ms
    u32 m_nlc_release_slow_time{};

    float m_nlc_recruit_radius{}; // nlc_ctrl_recruit_radius, 0 = off
    u32 m_nlc_recruit_time{}; // nlc_ctrl_recruit_time, channel length
    u32 m_nlc_recruit_delay{}; // nlc_ctrl_recruit_delay
    bool m_nlc_recruit_idle{}; // nlc_ctrl_recruit_idle, also outside combat
    shared_str m_nlc_recruit_anim; // nlc_ctrl_recruit_anim: clip list, first one the visual has wins
    MotionID m_nlc_recruit_motion;
    bool m_nlc_triple_ok{}; // the stock control triple (sit down, control, stand up) exists in the visual
    bool m_nlc_channel_triple{}; // the running channel uses the triple
    u16 m_nlc_channel_target{u16(-1)};
    u32 m_nlc_channel_start{};
    u32 m_nlc_channel_end{};
    u32 m_nlc_recruit_next{};
    xr_vector<CObject*> m_nlc_nearest;

    bool m_nlc_reposition{}; // nlc_ctrl_reposition (enemy inaccessible)
    bool m_nlc_cover_loop{}; // nlc_ctrl_cover_loop (always fight at range)
    float m_nlc_close_dist{}; // nlc_ctrl_close_dist, stock melee below this
    u32 m_nlc_lost_time{}; // nlc_ctrl_lost_time, stock hunt when unseen this long
    float m_nlc_cover_min{}, m_nlc_cover_max{}; // nlc_ctrl_cover_dist, distance from the enemy
    float m_nlc_range_min{}, m_nlc_range_max{}; // nlc_ctrl_range_dist, distance from the enemy
    float m_nlc_peek_radius{}; // nlc_ctrl_peek_radius, search around the controller
    u32 m_nlc_hold_min{}, m_nlc_hold_max{}; // nlc_ctrl_hold_time
    u32 m_nlc_aim_time{}; // nlc_ctrl_aim_time
    u32 m_nlc_move_timeout{}; // nlc_ctrl_move_timeout

    shared_str m_nlc_run_anim; // nlc_ctrl_run_anim, registered only if the visual has it
    bool m_nlc_run_registered{};
    bool m_nlc_run_ok{};
    bool m_nlc_running{}; // replaces eAnimRun and uses run velocity while set
    bool m_nlc_run_this_move{}; // decided when a hide move starts, kept until it ends
    u32 m_nlc_stock_until{}; // close-range stock melee hysteresis
    u32 m_nlc_next_los_check{};
    bool m_nlc_aim_los{};
    void nlc_release_effects(CEntity* thrall);

    ENlcPhase m_nlc_phase{eNlcPhaseOff};
    Fvector m_nlc_target_pos{};
    u32 m_nlc_target_node{u32(-1)};
    u32 m_nlc_phase_start{};
    u32 m_nlc_phase_until{};
    u8 m_nlc_logged_reason{};
    s8 m_nlc_next_side{1};
    bool m_nlc_overflow_flank{true}; // guard-species thralls beyond nlc_ctrl_guard_max alternate flank / hunt
    u32 m_nlc_next_status_log{};

    xr_map<const CEntity*, SNlcThrall> m_nlc_thralls;

    // Mirage decoy: this controller is a decoy (nlc_ctrl_decoy), or spawns one when it reaches cover
    void nlc_decoy_self_update();
    void nlc_decoy_vanish(LPCSTR why);
    void nlc_try_decoy(const CEntityAlive* enemy);
    void nlc_decoy_update();
    bool nlc_decoy_active() const { return m_nlc_decoy_id != u16(-1); }

    // flushed: the enemy walked into this controller's cover
    bool nlc_try_flush(const CEntityAlive* enemy, float dist);
    void nlc_psy_jolt(const CEntityAlive* target, float power, float amplitude_k);
    float m_nlc_flush_dist{}; // nlc_ctrl_flush_dist, 0 = off
    float m_nlc_flush_psy{}; // nlc_ctrl_flush_psy, psy hit of the close-range jolt
    u32 m_nlc_flush_cooldown{}; // nlc_ctrl_flush_cooldown
    u32 m_nlc_flush_next{};

    // thrall eyes: a weak psy jab through a thrall that sees the actor while the controller does not
    void nlc_update_thrall_eyes();
    bool m_nlc_thrall_eyes{}; // nlc_ctrl_thrall_eyes
    float m_nlc_eyes_power{}; // nlc_ctrl_thrall_eyes_power
    float m_nlc_eyes_range{}; // nlc_ctrl_thrall_eyes_range
    u32 m_nlc_eyes_cooldown{}; // nlc_ctrl_thrall_eyes_cooldown
    u32 m_nlc_eyes_next{};
    // the jab is a quick tube: a wind-up through the thrall (its head flares, the tube hums), then the strike
    u32 m_nlc_eyes_windup{}; // nlc_ctrl_thrall_eyes_windup, ms
    u16 m_nlc_eyes_thrall{u16(-1)}; // thrall the wind-up runs through
    u32 m_nlc_eyes_strike{}; // when the wind-up strikes
    u32 m_nlc_herd_until{}; // herd call: every thrall attacks the enemy until then

    bool m_nlc_is_decoy{}; // nlc_ctrl_decoy
    u32 m_nlc_decoy_life{}; // nlc_ctrl_decoy_life, ms
    u32 m_nlc_decoy_die_at{};
    bool m_nlc_decoy_vanishing{};
    shared_str m_nlc_decoy_section; // nlc_ctrl_decoy_section, empty = no decoys
    float m_nlc_decoy_chance{}; // nlc_ctrl_decoy_chance per cover reached
    u32 m_nlc_decoy_cooldown{}; // nlc_ctrl_decoy_cooldown, ms
    u32 m_nlc_decoy_next{};
    u16 m_nlc_decoy_id{u16(-1)};
    u32 m_nlc_decoy_spawned_at{};
    bool m_nlc_decoy_fed{};

public:
    DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CController)
#undef script_type_list
#define script_type_list save_type_list(CController)
