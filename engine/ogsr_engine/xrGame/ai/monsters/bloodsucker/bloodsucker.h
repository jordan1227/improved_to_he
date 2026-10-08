#pragma once

#include "../BaseMonster/base_monster.h"
#include "../ai_monster_bones.h"
#include "../controlled_actor.h"
#include "../anim_triple.h"
#include "script_export_space.h"
#include "bloodsucker_alien.h"

class CActor;

class CAI_Bloodsucker : public CBaseMonster, public CControlledActor
{
    typedef CBaseMonster inherited;

public:
    CAI_Bloodsucker();
    virtual ~CAI_Bloodsucker();

    virtual void reinit();
    virtual void reload(LPCSTR section);

    virtual void UpdateCL();
    virtual void shedule_Update(u32 dt);
    virtual void Die(CObject* who);
    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void Load(LPCSTR section);
    virtual void Hit(SHit* pHDS);

    virtual void CheckSpecParams(u32 spec_params);
    virtual bool ability_invisibility() { return true; }
    virtual bool ability_pitch_correction() { return false; }
    virtual bool ability_can_drag() { return true; }

    virtual void post_fsm_update();

    virtual bool use_center_to_aim() const { return true; }
    virtual bool check_start_conditions(ControlCom::EControlType);
    virtual void on_activate_control(ControlCom::EControlType); // NLC
    virtual void HitEntityInJump(const CEntity* pEntity); // NLC: pounce
    virtual float step_volume_k(); // NLC: faint steps while cloaked
    virtual bool nlc_cloak_keeps_gait(); // NLC: edge sneak
    void HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type = ALife::eHitTypeWound, bool draw_hit_marks = true) override;
    virtual bool nlc_run_target_override(Fvector& position, u32& vertex); // NLC: flank charge while unwatched

    //--------------------------------------------------------------------
    // Utils
    //--------------------------------------------------------------------
    void move_actor_cam(float angle);

    //--------------------------------------------------------------------
    // Bones
    //--------------------------------------------------------------------
private:
    static void BoneCallback(CBoneInstance* B);
    void vfAssignBones();
    void LookDirection(Fvector to_dir, float bone_turn_speed);

    bonesManipulation Bones;

    CBoneInstance* bone_spine;
    CBoneInstance* bone_head;
    bool collision_hit_off;

    //--------------------------------------------------------------------
    // Invisibility
    //--------------------------------------------------------------------
private:
    SMotionVel invisible_vel;
    LPCSTR invisible_particle_name;

public:
    void start_invisible_predator();
    void stop_invisible_predator();

    virtual bool in_solid_state();

    //--------------------------------------------------------------------
    // Vampire
    //--------------------------------------------------------------------
public:
    bool m_vampire_enable;
    u32 m_vampire_min_delay;
    float m_vampire_wound;
    float m_vampire_hold_time;
    float m_vampire_loss_health_speed;

    static u32 m_time_last_vampire;
    SAnimationTripleData anim_triple_vampire;

    SPPInfo pp_vampire_effector;

    void ActivateVampireEffector();
    bool WantVampire();
    void SatisfyVampire();

    // u32             get_last_critical_hit_tick () { return m_last_critical_hit_tick; }
    // void            clear_last_critical_hit_tick () { m_last_critical_hit_tick = 0; }
private:
    // TTime                   m_last_critical_hit_tick;
    // float                   m_critical_hit_chance; //0..1
    // float                   m_critical_hit_camera_effector_angle;

    float m_vampire_want_value;
    float m_vampire_want_speed; // load from ltx

    float m_vampire_gain_health;
    float m_vampire_distance;

    void LoadVampirePPEffector(LPCSTR section);

    //--------------------------------------------------------------------
    // Alien
    //--------------------------------------------------------------------
public:
    CBloodsuckerAlien m_alien_control;

    void set_alien_control(bool val);

    //--------------------------------------------------------------------
    // Predator
    //--------------------------------------------------------------------
public:
    shared_str m_visual_default;
    LPCSTR m_visual_predator;
    bool m_predator;

    void predator_start();
    void predator_stop();
    void predator_freeze();
    void predator_unfreeze();
    void jump(const Fvector& position, float factor);
    //--------------------------------------------------------------------
    // Sounds
    //--------------------------------------------------------------------
public:
    enum EBloodsuckerSounds
    {
        eAdditionalSounds = MonsterSound::eMonsterSoundCustom,

        eVampireGrasp = eAdditionalSounds | 0,
        eVampireSucking = eAdditionalSounds | 1,
        eVampireHit = eAdditionalSounds | 2,
        eVampireStartHunt = eAdditionalSounds | 3,
        eGrowl = eAdditionalSounds | 5,
        eChangeVisibility = eAdditionalSounds | 6,
        eAlien = eAdditionalSounds | 7,
    };

    //--------------------------------------------------------------------

public:
    void set_manual_control(bool value) {}
    void manual_activate();
    void manual_deactivate();

    float get_vampire_distance() const { return m_vampire_distance; }

    virtual void renderable_Render(u32 context_id, IRenderable* root) override;

#ifdef DEBUG
    virtual CBaseMonster::SDebugInfo show_debug_info();

#ifdef DEBUG
    void debug_on_key(int key);
#endif

#endif

    //-------------------------------------------------------------------
    // Bloodsucker's    Visibility States
    //-------------------------------------------------------------------
public:
    enum visibility_t
    {
        unset = -1,
        no_visibility = 0,
        partial_visibility = 1,
        full_visibility = 2
    };

private:
    u32 m_visibility_state_change_min_delay;

    float m_full_visibility_radius;
    float m_partial_visibility_radius;

    visibility_t m_visibility_state;
    visibility_t m_force_visibility_state;
    TTime m_visibility_state_last_changed_time;

    TTime m_runaway_invisible_time;

public:
    virtual float GetTransparency() override;

    float get_full_visibility_radius();
    float get_partial_visibility_radius();
    TTime get_visibility_state_change_min_delay();

    void start_runaway_invisible() { m_runaway_invisible_time = Device.dwTimeGlobal; }
    void clear_runaway_invisible() { m_runaway_invisible_time = 0; }

    virtual bool can_be_seen() const { return get_visibility_state() == full_visibility; }
    visibility_t get_visibility_state() const;
    void set_visibility_state(visibility_t new_state);
    void force_visibility_state(int state);
    void update_invisibility();

public:
    u32 m_hits_before_vampire;
    u32 m_sufficient_hits_before_vampire;
    int m_sufficient_hits_before_vampire_random;
    virtual void on_attack_on_run_hit();
    bool done_enough_hits_before_vampire();

    //-------------------------------------------------------------------
    // NLC: evasion pass (docs/DESIGN_monster_movement_under_fire.md 6, 8-10, 20). Every key is read with
    // READ_IF_EXISTS from the monster's own section and defaults to the stock behaviour; evade_enabled = false
    // ([monster_evasion] or the section) turns all of it off.
    //-------------------------------------------------------------------
public:
    struct SNlcBloodsuckerParams
    {
        // cloak and lunge
        bool run_attack_cfg{}; // read Run_Attack_Dist / Run_Attack_Delay (the ability is added after control().load(), so they were never read)
        bool run_attack_decloak{}; // the lunge may start cloaked and decloaks on its start
        float cloak_lunge_radius{}; // full-visibility radius while the lunge is ready (0 = stock radius)
        float cloak_step_volume{1.f}; // step volume factor while cloaked
        bool cloak_melee_reveal{}; // the first melee swing decloaks
        bool amb_cloak_hold{}; // cloaked at the siege hold point, even within the full-visibility radius
        float sneak_near_dist{}; // edge sneak: closer than this, unseen, near the edge of the view (0 = off)
        float sneak_edge_cone{deg2rad(75.f)};
        bool amb_pounce{}; // experimental jump strike
        float pounce_min{4.f}, pounce_max{7.f}; // perch pounce range (pounce_perch_dist, else jump_min/max_distance), 3D feet to the enemy's centre
        float pounce_flat_min{4.f}, pounce_flat_max{7.f}; // flat-ground pounce range (pounce_flat_dist)
        float pounce_flat_factor{};
        float pounce_min_vy{}; // the flat pounce keeps at least this takeoff speed upward (m/s; too flat, it dies in place) // jump time divider on flat ground (> jump_factor: lower, faster, longer; 0 = jump_factor)
        shared_str pounce_anim; // glide clip (pounce_anim, default stand_attack_1)
        float pounce_hit_dist{2.2f};
        float pounce_max_angle{deg2rad(20.f)}; // jump_max_angle (rad in the config, as CControlJump reads it) // own hit test during the pounce: centre to centre (the stock test missed perched targets)
        float cloak_xray_radius{}; // never fully invisible this close to the enemy: at least x-ray (0 = off)
        u32 tac_recover_cooldown_rand{}; // + random 0..this ms on the hit-and-run cooldown
        int tac_pair_rank{}; // pair: the highest rank baits (strong 2, normal 1); a pair forms only with a rank >= 1
        Fvector2 tac_flank_arc_dist{12.f, 15.f}; // flanker: the arc around the player it moves on, cloaked
        float tac_flank_arc_angle{deg2rad(110.f)}; // ... until this far off the player's view (side or back)
        u32 tac_flank_arc_max_ms{8000}; // ... strikes anyway after this long
        u32 tac_flank_wait_ms{1500}; // in position: strikes when the player watches the bait, or after this long
        bool tac_bait_mock{}; // bait: mock charges in and out, roaring, instead of standing
        float tac_bait_mock_dist{3.5f};
        float tac_bait_sync_dist{5.f}; // bait: the real charge once a striking flanker is this close to the player
        bool cloak_counts_hidden{}; // cloaked, beyond the x-ray radius and outside the view cone counts as hidden
        bool tac_recover_zz{}; // hit and run: zigzag legs while it is watched
        bool tac_feint_lateral{}; // feint: vanish sideways when aimed at
        u32 pounce_perch_hold_ms{500}; // a perch counts after the enemy stood on it this long (a jump is no perch)
        u8 lunge_reveal{2}; // visibility a lunge or pounce reveals: 1 x-ray (partial), 2 full (stock-like); the pounce never goes above x-ray
        u32 lunge_reveal_ms{}; // after a lunge or pounce: hold the reveal state this long or until the first melee swing (0 = off)
        float vampire_intent_dist{8.f}; // a grab from behind is planned this close: no lunge or pounce, stay cloaked
        float strike_reveal_margin{}; // > 0: fully visible within (start distance of a ready lunge / pounce + this); 0 = cloak_lunge_radius rule
        u32 strike_reveal_ms{300}; // ... and a strike only after this long fully visible (the tell)
        float cloak_cooldown_radius{5.f}; // full-visibility radius while no strike is ready
        float pounce_chance{1.f}; // chance a ready pounce is used (else this cycle is left to the lunge)
        u32 pounce_delay{3000}; // jump_delay: ms between pounces (also the cycle after a refused roll)
        float pounce_max_h{2.5f}; // pounce_perch_max_h (else jump_max_height): a perched enemy this high (feet above its ground point) can be pounced at
        bool pounce_flat{true}; // pounce at an enemy on flat ground (else only at a perch)
        float pounce_perch_chance{1.f}; // chance a ready pounce is used at a perched enemy (rolled per pounce cycle)
        u32 pounce_perch_wait_ms{4000}; // siege perch-strike spot: no pounce this long after arriving -> hide
        u32 pounce_perch_block_ms{20000}; // ... and no perch strike again for this long
        u32 pounce_bounce_grace_ms{}; // the pounce ignores velocity bounces this long after takeoff (0 = stock)
        u32 strike_reveal_max_ms{}; // a strike reveal without a strike ends after this long (0 = never)
        u32 strike_reveal_block_ms{3000}; // ... then no strike reveal for this long
        // flanks
        bool tac_stalk_flank{}; // stalk points on the player's flank first (perpendicular to the view)
        float tac_stalk_flank_angle{deg2rad(35.f)};
        bool charge_flank{}; // unwatched charge curves toward the player's flank or back
        float charge_flank_angle{deg2rad(100.f)}; // goal bearing from the player's view (90 flank, 180 back)
        float charge_flank_step{deg2rad(40.f)}; // bearing gained per waypoint
        Fvector2 charge_flank_dist{8.f, 30.f}; // only between these distances
        // vampire grab as a true ambush
        bool vampire_ambush{};
        float vampire_ambush_chance{1.f};
        u32 vampire_unseen_ms{3000};
        float vampire_behind_angle{deg2rad(120.f)};
        float vampire_struggle_look{8.f};
        float vampire_struggle_need{1500.f}; // raw mouse counts, abs(dx) + abs(dy)
        float vampire_struggle_wound_k{0.6f};
        float vampire_backhit_chance{}; // a hit from behind turns into the grab (0 = off)
        float vampire_backhit_damage_k{0.2f}; // share of that hit's damage dealt at once (the rest only when no grab follows)
        u32 vampire_backhit_window_ms{1500}; // the grab must start within this long after the hit
        float vampire_break_dist{}; // an ambush grab breaks beyond this distance (0 = MaxAttackDist)
        // tactics
        bool tac_recover_enabled{};
        float tac_recover_health{0.35f}, tac_recover_until{0.8f}, tac_recover_min_dist{4.f};
        Fvector2 tac_recover_dist{18.f, 30.f};
        u32 tac_recover_max_time{25000}, tac_recover_cooldown{45000};
        u32 tac_recover_min_time{}; // hide at least this long even when regeneration is fast
        float tac_recover_break_health{}; // below this it breaks away even at point blank (0 = never: cornered fights)
        float tac_return_angle{deg2rad(90.f)};
        bool tac_stalk_enabled{};
        Fvector2 tac_stalk_dist{15.f, 25.f};
        u32 tac_stalk_max_time{40000};
        bool tac_open_reload{}, tac_open_switch{};
        float tac_open_away_angle{deg2rad(100.f)};
        u32 tac_open_away_ms{1500};
        bool tac_feint_enabled{};
        Fvector2 tac_feint_dist{8.f, 14.f};
        float tac_feint_chance{0.4f};
        u32 tac_feint_cooldown{30000}, tac_feint_show_ms{1200};
        float tac_feint_flank_angle{deg2rad(100.f)};
        u8 tac_feint_vis{2}; // 1 x-ray, 2 full
        u32 tac_feint_cloak_ms{2000}; // after the show: cloaked this long while it vanishes or charges on
        bool tac_pair_enabled{};
        Fvector2 tac_bait_dist{10.f, 14.f};
        u32 tac_bait_max_time{15000};
        bool tac_bait_advance{}; // the bait in place walks slowly at the player instead of standing
        u8 tac_bait_vis{2}; // 1 x-ray, 2 full
        float tac_night_brightness{0.15f}, tac_night_dist_k{1.3f}, tac_night_time_k{1.5f}, tac_night_chance_add{0.2f};
    };

    SNlcBloodsuckerParams m_nlc_b;
    bool m_nlc_on{}; // evade_enabled and at least one of the features above is configured
    bool m_nlc_vampire_ambush{}; // vampire_ambush with the vampire grab enabled (read by the execute state)

    void nlc_bs_load(LPCSTR section);
    void nlc_set_cloak(visibility_t state, bool bypass_delay);
    bool nlc_cloak_override(visibility_t& state);
    bool nlc_cloak_rules(visibility_t& state);
    void nlc_pounce_hit_update();
    bool nlc_lunge_ready();
    bool nlc_vampire_ambush_ok(const CEntityAlive* enemy);
    void nlc_tactic_update();
    void nlc_pounce_update();

    virtual bool nlc_tactic_wanted() { return m_nlc_tac.want; }
    virtual bool nlc_siege_perch_strike(const CEntityAlive* enemy, float& strike_dist);
    virtual void nlc_siege_perch_shot();
    virtual bool nlc_siege_cloak_hidden(const Fvector& feet, const CEntityAlive* enemy) const;
    virtual u32 nlc_jump_bounce_grace() const { return m_nlc_on ? m_nlc_b.pounce_bounce_grace_ms : 0; }
    virtual float nlc_jump_min_vy() const { return m_nlc_on ? m_nlc_b.pounce_min_vy : 0.f; }
    virtual void nlc_tactic_begin();
    virtual void nlc_tactic_execute();
    virtual void nlc_tactic_end();

private:
    enum ENlcTacMode : u8
    {
        eTacNone,
        eTacRecover, // hurt: back off hidden, regenerate, return from another side
        eTacStalk, // shadow the enemy outside its view, strike on an opening
        eTacFeint, // show itself, vanish, return from a flank
        eTacBait, // decloaked decoy in front while the flankers come from behind
    };
    enum ENlcTacRole : u8
    {
        eRoleNone,
        eRoleBait,
        eRoleFlanker,
    };
    enum ENlcTacPick : int
    {
        eTacPickNone,
        eTacPickFound,
        eTacPickDeferred, // the per-frame search budget is spent: try again next update
    };

    struct SNlcTactic
    {
        ENlcTacMode mode{eTacNone};
        bool want{};
        visibility_t force_vis{unset};
        u16 enemy_id{u16(-1)};
        u32 next_update{};
        u32 mode_start{};
        // movement
        bool moving{};
        bool have_target{};
        Fvector target{};
        u32 node{u32(-1)};
        u32 move_until{};
        u32 burned{u32(-1)};
        u32 hit_seen{};
        // cooldowns
        u32 recover_cd{};
        u32 feint_cd{};
        u32 stalk_block{};
        // recover / return
        Fvector left_dir{};
        bool bias_set{};
        Fvector bias{};
        // stalk
        u32 next_pick{};
        u32 next_repick_ok{};
        u32 away_since{};
        // feint
        u8 feint_phase{};
        u32 feint_until{};
        // bait / flank
        ENlcTacRole role{eRoleNone};
        u32 next_rank{};
        u32 committed_until{}; // this monster left a stalk to strike (the bait reads it)
        u32 next_growl{};
        u32 flank_pos_since{}; // flanker in position (off the view) since
        bool flank_struck{}; // flanker committed from the arc (the bait reads it)
        bool mock_out{}; // bait: on the way in of a mock charge
        u32 next_mock{};
        Fvector bait_home{};
        u32 bait_home_node{u32(-1)};
        bool roared{}; // bait: the tell before the flanker strikes
        s8 zz_side{}; // recover: zigzag leg side
        u32 zz_next{};
        // vampire grab roll (tactics.cpp)
        u8 bias_stage{}; // stalk pick preference: 0 flank (tac_stalk_flank), 1 own side, 2 none; back to 0 after a pick
        bool rolled_grab{};
        bool grab_ok{};
        bool night{};
    };

    SNlcTactic m_nlc_tac;
    bool m_nlc_tac_any{}; // some tac_*_enabled
    bool m_nlc_allow_jump{}; // true only inside the pounce call (check_start_conditions(eControlJump))
    u32 m_nlc_next_pounce{};
    bool m_nlc_melee_logged{};
    u32 m_nlc_reveal_until{}; // lunge / pounce reveal window (lunge_reveal_ms), ended by the first melee swing
    bool m_nlc_grab_intent{}; // a grab from behind is planned (vampire_intent_dist): no lunge, no pounce, cloaked
    u32 m_nlc_full_since{}; // fully visible since (0 = not fully visible), for strike_reveal_ms
    u32 m_nlc_pounce_ready_at{}; // next pounce cycle (pounce_delay after a pounce or a refused roll)
    u32 m_nlc_cloak_until{}; // forced cloak window (after a feint)
    // pounce rolls (pounce_chance on flat ground, pounce_perch_chance at a perch): once per pounce cycle, a refused
    // roll again after jump_delay
    struct SNlcPounceRoll
    {
        bool rolled{};
        bool go{};
        u32 next{};
    };
    SNlcPounceRoll m_nlc_roll_flat, m_nlc_roll_perch;
    u32 m_nlc_pounce_fail_log{};
    u32 m_nlc_perch_block_log{}; // diagnostics: why a ready perch pounce does not start
    u32 m_nlc_perch_repick_at{}; // perch spot out of pounce range: pick it again (not before this)
    u32 m_nlc_perched_since{}; // the enemy stands on a perch since (pounce_perch_hold_ms)
    bool m_nlc_pounce_hit{}; // the current pounce has hit (own hit test or CControlJump)
    bool m_nlc_pouncing{}; // a pounce of ours is in the air
    u32 m_nlc_perch_since{}; // holding the siege perch-strike spot since
    u32 m_nlc_perch_block_until{}; // no perch strike before this
    // strike reveal timeout (strike_reveal_max_ms)
    u32 m_nlc_strike_reveal_since{};
    u32 m_nlc_strike_reveal_block_until{};
    // back-hit grab (vampire_backhit_chance)
    bool m_nlc_backhit{};
    bool m_nlc_backhit_in_grab{}; // the grab started from it is running
    u32 m_nlc_backhit_until{};
    u16 m_nlc_backhit_target{u16(-1)};
    float m_nlc_backhit_rest{};
    float m_nlc_backhit_impulse{};
    Fvector m_nlc_backhit_dir{};
    ALife::EHitType m_nlc_backhit_type{ALife::eHitTypeWound};
    // flank charge
    s8 m_nlc_flank_side{};
    u32 m_nlc_flank_side_until{};
    bool m_nlc_flank_on{};

    bool nlc_pounce_ready() const;
    void nlc_pounce_cooldown(u32 ms); // next pounce cycle in ms (a new roll then)
    bool nlc_pounce_wanted(bool perch) const; // ready and this cycle's roll allows it
    bool nlc_enemy_perch_h(const CEntityAlive* enemy, float& h) const; // enemy feet above its ground point
    bool nlc_enemy_perched(const CEntityAlive* enemy, float& h); // on a perch for real: held, not a jump, off the ai-map
    bool nlc_tac_hidden(const Fvector& pt, const CEntityAlive* enemy) const; // out of sight, or cloaked out of view
    bool nlc_tac_lateral(const CEntityAlive* enemy, float min_d, float max_d, Fvector& pos, u32& node); // sideways off the line of fire
    void nlc_perch_give_up(const char* why);
    void nlc_backhit_update();
    bool nlc_strike_tell_done() const; // fully visible long enough to strike (always true without strike_reveal_margin)

    visibility_t nlc_reveal_state(bool pounce) const;
    void nlc_strike_reveal(const char* what, bool pounce);
    bool nlc_grab_possible(const CEntityAlive* enemy, bool roll);
    bool nlc_backhit_try(const CEntity* entity, float damage, float impulse, Fvector& dir, ALife::EHitType hit_type, bool draw_hit_marks);

public:
    bool nlc_backhit_ready(); // a back hit landed and the grab may start now (read by the execute state)
    void nlc_backhit_grab_started();
    void nlc_backhit_end(bool landed); // the grab ended: landed or broke early (the rest of the hit lands)
    bool nlc_backhit_actor_ok(CActor* actor);

private:

    void nlc_tac_finish(const char* reason);
    void nlc_tac_commit(const char* reason);
    void nlc_tac_enter(ENlcTacMode mode, const Fvector& target, u32 node);
    void nlc_tac_try_enter(const CEntityAlive* enemy, bool actor_enemy);
    void nlc_tac_continue(const CEntityAlive* enemy, bool actor_enemy);
    void nlc_tac_update_role(const CEntityAlive* enemy);
    bool nlc_tac_flanker_struck(const CEntityAlive* enemy, float* nearest = nullptr); // a flanker of this squad left its stalk to strike (and its distance to the enemy)
    bool nlc_tac_night();
    // hidden / unwatched point on 3 radii x 8 angles around `center`, nearest to the monster (at most 4 searches per frame, all bloodsuckers)
    int nlc_tac_pick(const Fvector& center, Fvector2 ring, float bias_angle, const Fvector* bias_dir, bool need_hidden, bool prefer_hidden, bool need_unwatched, Fvector& pos,
                     u32& node);
    bool nlc_tac_radial(const CEntityAlive* enemy, float min_d, float max_d, bool need_hidden, Fvector& pos, u32& node);
    bool nlc_tac_bait_point(const CEntityAlive* enemy, Fvector& pos, u32& node);
    void nlc_tac_move(const Fvector& target, u32 node, EAction act);
    void nlc_tac_hold(const CEntityAlive* enemy);

    DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CAI_Bloodsucker)
#undef script_type_list
#define script_type_list save_type_list(CAI_Bloodsucker)