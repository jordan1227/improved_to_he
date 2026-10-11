#pragma once
#include "../BaseMonster/base_monster.h"
#include "../telekinesis.h"
#include "../anim_triple.h"
#include "Actor.h"
#include "../scanning_ability.h"
#include "script_export_space.h"

class CCharacterPhysicsSupport;
class CBurerFastGravi;
class CGrenade;
class CWeaponMagazined;
class CTelekineticObject;
class CExplosiveRocket;
class CParticlesObject;
class CPhysicsElement;
struct SHit;

class CBurer : public CBaseMonster, public CTelekinesis
{
    typedef CBaseMonster inherited;

private:
    xr_vector<CObject*> m_nearest;

public:
    static bool can_scan;

    u32 last_hit_frame;
    u32 time_last_scan;

    typedef CTelekinesis TTelekinesis;

    struct GraviObject
    {
        bool active;
        Fvector cur_pos;
        Fvector target_pos;
        Fvector from_pos;

        u32 time_last_update{};

        const CEntityAlive* enemy;

        GraviObject()
        {
            active = false;
            enemy = 0;
        }

        void activate(const CEntityAlive* e, const Fvector& cp, const Fvector& tp)
        {
            active = true;
            from_pos = cp;
            cur_pos = cp;
            target_pos = tp;
            time_last_update = Device.dwTimeGlobal;
            enemy = e;
        }

        void deactivate() { active = false; }

    } m_gravi_object;

    LPCSTR particle_gravi_wave;
    LPCSTR particle_gravi_prepare;
    LPCSTR particle_tele_object;

    //////////////////////////////////////////////////////////////////////////
    // Sounds
    ref_sound sound_gravi_wave;
    ref_sound sound_scan;

    ref_sound sound_tele_hold;
    ref_sound sound_tele_throw;

    enum EBurerSounds
    {
        eAdditionalSounds = MonsterSound::eMonsterSoundCustom,

        eMonsterSoundGraviAttack = eAdditionalSounds | 0,
        eMonsterSoundTeleAttack = eAdditionalSounds | 1,
    };
    //////////////////////////////////////////////////////////////////////////

    struct gravi_params
    {
        float speed;
        u32 cooldown;
        float min_dist;
        float max_dist;
        float step;
        TTime time_to_hold;
        float radius;
        float impulse_to_objects;
        float impulse_to_enemy;
        float hit_power;

    } m_gravi;

    u32 m_tele_max_handled_objects;
    u32 m_tele_time_to_hold;
    u32 m_tele_max_time;
    float m_tele_object_min_mass;
    float m_tele_object_max_mass;
    float m_tele_find_radius;
    float m_tele_min_distance;
    float m_tele_max_distance;
    float m_tele_raise_speed;
    float m_tele_fly_velocity;
    float m_tele_object_height;

    float m_weight_to_stamina_hit;
    float m_weapon_drop_stamina_k;
    float m_runaway_distance;
    float m_normal_distance;
    TTime m_max_runaway_time;

    float m_weapon_drop_velocity;

    TTime m_shield_cooldown;
    TTime m_shield_time;
    bool m_shield_active;
    LPCSTR m_shield_keep_particle;
    TTime m_shield_keep_particle_period;
    LPCSTR particle_fire_shield;

    CBurerFastGravi* m_fast_gravi;

    // ---------------------------------------------------------------------
    // NLC burer pass 1 (docs/BURER_REDESIGN.md 5.2)
    // nlc_burer_debug: "~ [burer]" decision log for test rounds
    bool m_nlc_debug{};
    void nlc_log(LPCSTR format, ...);

    // tele throws: mass-scaled impact damage, lead, clear path
    float m_nlc_tele_dmg_min{};
    float m_nlc_tele_dmg_max{};
    float m_nlc_tele_dmg_mass_min{10.f};
    float m_nlc_tele_dmg_mass_max{150.f};
    float m_nlc_tele_lead{};

    float nlc_tele_damage(CPhysicsShellHolder* obj) const;
    // aim point moved by the target's movement over the flight time (actor only), max 4 m
    Fvector nlc_lead_point(const CEntityAlive* target, const Fvector& aim, float flight_time, float lead_k) const;
    // true when nothing but 'target' blocks the segment (ignore = the moving object itself)
    bool nlc_clear_path(const Fvector& from, const Fvector& to, CObject* ignore, const CObject* target) const;

    // repulse (close-range blast)
    struct NlcRepulse
    {
        bool enabled{};
        float trigger_dist{4.f};
        float radius{6.f};
        u32 cooldown{9000};
        float hit{0.12f};
        float push_speed{9.f}; // horizontal speed set on the target at the burer (falloff to the edge)
        float push_up{4.5f}; // upward speed: off the ground, so friction and the character controller do not eat it
        u32 push_ms{250}; // the actor's horizontal push is held this long against the controller
        float impulse_objects{8.f};
        float hit_time{0.35f};
        float cam_inert{0.6f};
        float falloff_min{0.7f}; // push / damage share at the edge of the radius
        shared_str particle;
        ref_sound sound;
    } m_nlc_repulse;
    u32 m_nlc_repulse_next{};
    u32 m_nlc_push_until{};
    Fvector m_nlc_push_vel{};
    Fvector m_nlc_push_origin{};
    u32 m_nlc_push_log_at{};
    LPCSTR m_nlc_push_kind{""};
    void nlc_push_update();
    // velocity push (horizontal along dir + up); the actor's horizontal part is held for hold_ms against the controller
    void nlc_push(CEntityAlive* entity, const Fvector& dir, float speed, float up, u32 hold_ms, LPCSTR kind);
    float m_nlc_gravi_push_speed{}, m_nlc_gravi_push_up{};
    u32 m_nlc_gravi_push_ms{300};

    // tele: pause between tele states so the burer moves and casts instead of channelling for half a minute
    u32 m_nlc_tele_cooldown{5000};
    u32 m_nlc_tele_next{};

    bool nlc_repulse_ready();
    void nlc_repulse_blast();

    // disarm grace shared with the script stare (bind_monster.burer_on_disarm)
    u32 m_nlc_disarm_grace{20000};

    // gravi: weapons and other inventory items get their own impulse (x mass), and no gravi right after a disarm
    float m_nlc_gravi_item_impulse{6.f};
    u32 m_nlc_gravi_after_disarm_ms{2500};
    u32 m_nlc_disarm_query_next{};
    bool m_nlc_disarm_recent{};
    bool nlc_gravi_blocked_by_disarm();

    // top-level state last written to the nlc_burer_debug log (CStateManagerBurer)
    u32 m_nlc_logged_top_state{u32(-1)};

    // ---------------------------------------------------------------------
    // NLC burer pass 2-4 (docs/BURER_REDESIGN.md 3.1, 3.2, 4): puppet gun, defenses, brain (burer_nlc.cpp)

    // --- defenses: threat meter (the actor's crosshair on the burer), shield on threat, camera wrench, shield strain
    float m_nlc_threat{};
    float m_nlc_threat_gain{2.5f}; // per second while aimed at (x phase factor)
    float m_nlc_threat_loss{1.5f};
    float m_nlc_threat_radius{0.7f}; // crosshair-ray distance from the burer's centre that counts as aimed at
    bool m_nlc_threat_shield{};
    u32 m_nlc_threat_shield_ms{2500};
    u32 m_nlc_shield_next{}; // shared by the shield state and the threat shield
    u32 m_nlc_overlay_shield_until{};
    u32 m_nlc_overlay_particle_next{};
    float m_nlc_shield_strain{}; // health the blocked hits would have cost
    float m_nlc_shield_strain_hp{}; // capacity, share of max health (0 = never breaks)
    float m_nlc_shield_leak_start{0.5f}; // strain share from which hits leak through
    float m_nlc_shield_leak_max{1.f};
    u32 m_nlc_shield_crackle_next{};
    u32 m_nlc_shield_break_cooldown{8000};
    float m_nlc_break_stamina{0.35f}; // psi pulse on a break
    float m_nlc_break_shake{20.f}; // degrees of the pulse's camera shake (non-wrench burers)
    float m_nlc_deflect_chance{}; // passive: bullets turned away while not busy
    bool m_nlc_busy{}; // casting / channelling (set by the attack state)
    u32 m_nlc_deflect_count{};
    u32 m_nlc_deflect_log_next{};
    bool m_nlc_wrench{};
    u32 m_nlc_wrench_cooldown{15000};
    u32 m_nlc_wrench_next{};
    float m_nlc_wrench_min{90.f}, m_nlc_wrench_max{180.f}; // degrees
    float m_nlc_wrench_pitch{15.f}; // degrees, random sign
    u32 m_nlc_wrench_ms_min{250}, m_nlc_wrench_ms_max{450};
    struct NlcWrench
    {
        bool active{};
        u32 start{}, dur{};
        float yaw_total{}, pitch_total{}, yaw_applied{}, pitch_applied{};
        float shake{}, shake_yaw_prev{}, shake_pitch_prev{};
    } m_nlc_wr;

    void nlc_defense_update();
    void nlc_overlay_shield_stop(LPCSTR why);
    bool nlc_shield_allowed() const { return Device.dwTimeGlobal >= m_nlc_shield_next; }
    float nlc_on_shield_blocked(const SHit* hit); // returns the share that leaks through
    void nlc_shield_break_reaction();
    bool nlc_try_deflect(const SHit* hit);
    void nlc_start_wrench(float yaw_deg, float pitch_deg, u32 ms, float shake_deg, LPCSTR why);
    void nlc_wrench_update();

    // --- barricade: a held object parked between the burer and its enemy while it channels tele
    float m_nlc_barricade_dist{1.6f};
    float m_nlc_barricade_height{1.1f};
    float m_nlc_barricade_min_size{0.8f}; // largest bounding-box side, m
    float m_nlc_barricade_min_volume{0.12f}; // m^3
    float m_nlc_barricade_mass_min{20.f}, m_nlc_barricade_mass_max{400.f};
    xr_vector<shared_str> m_nlc_barricade_allow, m_nlc_barricade_deny, m_nlc_corpse_species;
    bool m_nlc_barricade{};
    u32 m_nlc_barricade_mode{1}; // 0 off, 1 when threatened / hurt / wounded, 2 whenever fighting
    u32 m_nlc_barricade_max_ms{12000};
    u32 m_nlc_barricade_cooldown{4000};
    CPhysicsShellHolder* m_nlc_bar_obj{};
    CTelekinesis m_nlc_bar_tele;
    u32 m_nlc_bar_next{};
    u32 m_nlc_bar_until{};
    u32 m_nlc_bar_far_since{}; // the object has been far from its post since then
    float m_nlc_barricade_speed{9.f}; // faster than the burer runs: it keeps up when it moves
    float m_nlc_barricade_gain{5.f}; // spring gain: how fast it swings round to stay between the burer and the enemy
    float m_nlc_barricade_accel{20.f};
    float m_nlc_tele_weapon_speed_k{1.6f}; // thrown guns fly this much faster (flatter) than props
    u32 m_nlc_puppet_blind_ms{8000}; // no shot and no line of fire this long: the gun is put down
    u32 m_nlc_grudge_log_next{};
    float nlc_throw_speed(CPhysicsShellHolder* obj) const; // tele throw speed (guns faster)
    float m_nlc_barricade_leash{8.f}; // m from its post; dropped after 2 s beyond it
    bool nlc_barricade_ok(CPhysicsShellHolder* obj) const;
    CPhysicsShellHolder* nlc_find_corpse_barricade();
    void nlc_barricade_update(); // a free action: held through every state, no animation
    void nlc_barricade_drop(LPCSTR why);
    bool nlc_own_held(CPhysicsShellHolder* obj); // held by any of this burer's telekinesis (its own waves skip it)
    static xr_vector<CBurer*> s_nlc_burers; // live burers: searches skip objects any of them holds
    static bool nlc_held_by_any(CPhysicsShellHolder* obj);
    u32 m_nlc_pick_retry{}; // a failed tactical pick is not repeated before then
    u32 m_nlc_rocket_scan_next{};
    u32 m_nlc_bar_ext_next{};
    float m_nlc_bar_ext[6]{}; // lo/hi x, y, z of the held object's shell (refreshed every 100 ms)
    bool m_nlc_tele_vs_npc{};

    // --- puppet gun: a loaded gun lying around is raised, aimed and fired at the enemy
    struct NlcPuppet
    {
        CWeaponMagazined* gun{};
        u32 phase{}; // 0 none, 1 seize, 2 aim, 3 fire
        u32 phase_start{};
        u32 seized{};
        u32 ubgl_at{};
        bool ubgl_done{};
        bool burst_start{};
        float seed{};
        u32 ubgl_log_next{};
        float lift_kick{}; // m, vertical bounce from shots
        u32 unjam_until{}; // a jam: no shots, rattling, until then
        bool unjam_sound_done{};
        u32 hover_since{}; // the current hover point was taken then (kept for a while)
        u32 los_lost_since{};
        u32 next_shot{};
        u32 burst_left{};
        u32 next_hover{};
        u32 end_by{};
        u32 shots{};
        Fvector hover{};
        float kick_pitch{}, kick_yaw{};
        float hp_at_start{};
        bool keep_away{};
        bool los{};
    } m_nlc_puppet;
    CTelekinesis m_nlc_gun_tele;
    bool m_nlc_puppet_enabled{};
    float m_nlc_puppet_radius{20.f}; // search around the burer (and around the enemy: half of it)
    float m_nlc_puppet_max_dist{30.f}; // gun - burer
    u32 m_nlc_puppet_seize_delay{1500}; // a gun must lie this long before it is taken (time to grab it back)
    u32 m_nlc_puppet_settle_ms{900};
    u32 m_nlc_puppet_cooldown{4000};
    u32 m_nlc_puppet_max_ms{25000};
    float m_nlc_puppet_hit_k{1.f};
    float m_nlc_puppet_disp_k{2.f};
    float m_nlc_puppet_blind_disp_k{1.6f}; // the burer itself does not see the target
    float m_nlc_puppet_fire_cone{4.f}; // degrees between the barrel and the aim
    float m_nlc_puppet_recoil{0.06f}; // radians of barrel kick per shot
    float m_nlc_puppet_break_damage{0.05f}; // health lost since the seize that breaks the hold
    float m_nlc_puppet_keep_away_chance{0.5f};
    float m_nlc_puppet_keep_dist{5.f};
    float m_nlc_puppet_drift_speed{3.f};
    float m_nlc_puppet_hover_height{2.f};
    u32 m_nlc_puppet_lock_ms{1500};
    u32 m_nlc_puppet_burst_min{2}, m_nlc_puppet_burst_max{5};
    bool m_nlc_puppet_head_aim{};
    float m_nlc_puppet_bob{0.06f}, m_nlc_puppet_tremor{0.015f}, m_nlc_puppet_jitter{1.f};
    bool m_nlc_puppet_ubgl{};
    u32 m_nlc_puppet_ubgl_min{6000}, m_nlc_puppet_ubgl_max{8000};
    float m_nlc_puppet_ubgl_min_dist{10.f};
    float m_nlc_puppet_jam_cap{0.03f};
    u32 m_nlc_puppet_unjam_ms{3000};
    u32 m_nlc_puppet_hover_commit{2500};
    u32 m_nlc_puppet_los_grace{600};
    u32 m_nlc_puppet_next{};
    u16 m_nlc_puppet_candidate{u16(-1)};
    u32 m_nlc_puppet_candidate_since{};

    void nlc_puppet_update();
    bool nlc_puppet_active() const { return m_nlc_puppet.gun != nullptr; }
    bool nlc_puppet_reserved(CPhysicsShellHolder* obj) const; // loaded guns are left to the puppet, not thrown
    void nlc_puppet_release(LPCSTR why, bool throw_at_enemy);
    bool nlc_puppet_hover_ok(const Fvector& p) const;
    void nlc_puppet_pick_hover(const CEntityAlive* enemy, const Fvector& aim, bool crowded);

    // --- brain: phases, cover / advance moves (eStateBurerAttack_Tactical)
    bool m_nlc_brain{};
    float m_nlc_phase_wounded{0.6f}, m_nlc_phase_desperate{0.3f};
    u32 m_nlc_logged_phase{u32(-1)};
    float m_nlc_engage_max{25.f}; // beyond this it advances
    float m_nlc_cover_min{8.f}, m_nlc_cover_max{22.f};
    u32 m_nlc_cover_hold_min{2000}, m_nlc_cover_hold_max{4000};
    u32 m_nlc_move_next{};
    u32 m_nlc_hold_until{};
    Fvector m_nlc_move_target{};
    u32 m_nlc_move_node{u32(-1)};
    bool m_nlc_move_run{};
    LPCSTR m_nlc_move_kind{""};
    u32 m_nlc_move_until{};
    bool m_nlc_move_active{};
    float m_nlc_move_start_dist{};
    u32 m_nlc_last_cover_move{};
    u32 m_nlc_cover_retry{6000};
    u32 m_nlc_advance_straight_until{};
    bool m_nlc_force_cover{}; // a broken shield sends it to cover at once
    void nlc_on_move_end();

    u32 nlc_phase(); // 0 healthy, 1 wounded, 2 desperate (logged on change)
    float nlc_phase_k(float healthy, float wounded, float desperate) { const u32 p = nlc_phase(); return p == 0 ? healthy : (p == 1 ? wounded : desperate); }
    bool nlc_brain_pick_move(bool want_cover, bool hurt, bool ready); // sets m_nlc_move_* for the tactical state
    bool nlc_set_move(const Fvector& pos, u32 node, bool run, LPCSTR kind);
    bool nlc_gravi_path_clear(); // the wave would not hit cover first
    // ---------------------------------------------------------------------

    // ---------------------------------------------------------------------
    // NLC burer pass 5 (docs/BURER_REDESIGN.md 5.13, burer_nlc5.cpp): barricade fit, gravi push timing, NPC disarm,
    // grenade snatch, shield bullet catch, pull, elevation (overwatch), gaze
    enum ENlcCast
    {
        eNlcCastStare = 0, // the gaze's stare window
        eNlcCastNpcDisarm,
        eNlcCastPull,
    };
    bool nlc_cast_ready(u32 kind);
    void nlc_cast_begin(u32 kind);
    void nlc_cast_execute(u32 kind);
    bool nlc_cast_done(u32 kind);
    void nlc_cast_end(u32 kind, bool critical);
    u32 m_nlc_cast_start{}, m_nlc_cast_hit{}, m_nlc_cast_end{};
    bool m_nlc_cast_hit_done{};
    void nlc_load_pass5(LPCSTR section);
    void nlc_reinit_pass5();
    void nlc_pass5_update(); // every UpdateCL while alive

    // barricade fit: box centre at chest height, broad side to the enemy, effort tremor, optional one-bone grip
    float m_nlc_barricade_tremor{0.03f}; // m, x effort
    float m_nlc_barricade_turn{2.f}; // rad/s cap of the broad-side turn
    u8 m_nlc_barricade_grip{}; // 0 whole, 1 head, 2 torso (corpses)
    xr_vector<shared_str> m_nlc_grip_head_bones, m_nlc_grip_torso_bones;
    float m_nlc_bar_hp{}; // health when it was raised
    float m_nlc_bar_hang{}; // grip: how far the body hangs below the held element
    bool m_nlc_bar_corpse{};
    u32 m_nlc_bar_log_next{};
    CPhysicsElement* nlc_grip_element(CPhysicsShellHolder* obj, const xr_vector<shared_str>& bones);

    // gravi push applied the frame after the hit, horizontally away from the burer; velocity logs
    u16 m_nlc_gravi_push_id{u16(-1)};
    u32 m_nlc_gravi_push_frame{};
    Fvector m_nlc_gravi_push_dir{};
    u32 m_nlc_push_frame{};
    bool m_nlc_push_first_logged{true};

    // NPC disarm: a stalker that keeps a main weapon on the burer loses it (stamina-attack clip)
    bool m_nlc_npc_disarm{};
    float m_nlc_npc_disarm_range{20.f};
    u32 m_nlc_npc_disarm_aim_ms{1500};
    u32 m_nlc_npc_disarm_grace{30000}; // per stalker
    u32 m_nlc_npc_disarm_cooldown{8000};
    u32 m_nlc_npc_disarm_next{};
    u16 m_nlc_npc_aim_id{u16(-1)};
    u32 m_nlc_npc_aim_since{};
    u32 m_nlc_npc_track_next{};
    u16 m_nlc_npc_target{u16(-1)};
    xr_map<u16, u32> m_nlc_npc_disarmed; // stalker id -> no disarm again until
    float m_nlc_npc_disarm_mag_k{0.33f}; // only a gun with at least this share of its magazine (not mid-reload)
    u32 m_nlc_npc_drop_lock_ms{6000}; // nobody takes the torn-out gun for this long (the burer seizes it)
    u32 m_nlc_npc_seize_delay{500}; // puppet seize delay for that gun (the player's grab-back delay is longer)
    u16 m_nlc_npc_drop_gun{u16(-1)};
    bool m_nlc_npc_aiming{}; // the stalker it fights has a gun on it (threat meter, nlc_threat_npc_k)
    bool m_nlc_npc_disarm_need_sidearm{true}; // only a stalker left with another usable gun is disarmed
    shared_str m_nlc_npc_spare; // a stalker disarmed without another usable gun gets this pistol (empty: none)
    shared_str m_nlc_npc_spare_ammo;
    u16 m_nlc_npc_spare_rounds{16};
    float m_nlc_npc_spare_cond_min{0.4f}, m_nlc_npc_spare_cond_max{0.7f};
    bool nlc_npc_has_fallback(const CEntityAlive* stalker, const CWeapon* except) const; // another gun it can fire
    void nlc_npc_give_spare(CEntityAlive* stalker);
    u16 m_nlc_npc_skip_logged{u16(-1)};

    // grudge: an enemy that hurt it recently is not forgotten, and it keeps looking for a line of sight
    u32 m_nlc_grudge_ms{60000};
    u32 m_nlc_grudge_next{};
    u16 m_nlc_grudge_id{u16(-1)};
    u32 m_nlc_grudge_unseen_since{};
    void nlc_grudge_update();
    bool nlc_grudge_hunt() const; // grudge held and the enemy unseen a while: peek even with nothing ready
    float m_nlc_threat_npc_k{0.6f};
    bool nlc_npc_aiming_at_me(const CEntityAlive* enemy, bool primary_only) const;
    bool nlc_tele_skip(CPhysicsShellHolder* obj); // tele never throws a gun at a stalker (it would pick it up)
    bool nlc_npc_disarm_candidate(const CEntityAlive* enemy) const;
    void nlc_npc_track();
    void nlc_npc_disarm_fire();

    // grenade snatch (strong / hos / yan): the grenade is yanked out of the actor's hand, armed, held, thrown back
    bool m_nlc_snatch{};
    bool m_nlc_snatch_idle{}; // also a merely selected grenade (nlc_snatch_selected)
    float m_nlc_snatch_range{14.f};
    float m_nlc_snatch_chance{0.5f}; // per check (twice a second)
    u32 m_nlc_snatch_cooldown{20000};
    float m_nlc_snatch_speed{4.f};
    u32 m_nlc_snatch_next{};
    u32 m_nlc_snatch_check_next{};
    u16 m_nlc_snatch_id{u16(-1)};
    u32 m_nlc_snatch_until{};
    void nlc_snatch_update();

    // shield bullet catch: blocked bullets hang in the air, go back when the shield drops, fall when it breaks
    struct NlcCaught
    {
        shared_str ammo;
        u8 ammo_type{};
        Fvector offset{}; // from the burer's position
        float power{}, impulse{}, speed{};
        u16 shooter{u16(-1)};
        CParticlesObject* ps{};
        u32 glow_next{};
    };
    xr_vector<NlcCaught> m_nlc_caught;
    u32 m_nlc_catch_max{}; // 0 = off
    float m_nlc_catch_return_k{0.7f};
    float m_nlc_catch_disp{1.5f}; // degrees
    u32 m_nlc_catch_delay_ms{200};
    u32 m_nlc_catch_interval_min{40}, m_nlc_catch_interval_max{80};
    bool m_nlc_catch_drop_on_break{true};
    shared_str m_nlc_catch_particle; // looping, one per bullet (optional)
    shared_str m_nlc_catch_glow; // replayed every m_nlc_catch_glow_ms at each bullet
    u32 m_nlc_catch_glow_ms{1200};
    u32 m_nlc_catch_glow_fast_ms{350}; // in the shield's last second
    shared_str m_nlc_catch_flare; // the tell: a flash at every caught bullet when the return starts
    u32 m_nlc_shield_ends{}; // when the running shield (state or threat) is due to drop
    u32 m_nlc_catch_frame{};
    u16 m_nlc_catch_weapon{u16(-1)};
    bool m_nlc_catch_broken{};
    bool m_nlc_catch_returning{};
    u32 m_nlc_catch_next_shot{};
    u32 m_nlc_catch_returned{};
    void nlc_catch_bullet(const SHit* hit);
    void nlc_catch_update();
    void nlc_catch_drop(LPCSTR why);

    // pull: an enemy up on a roof / ledge is yanked toward the burer, over the edge
    bool m_nlc_pull{};
    float m_nlc_pull_range{20.f};
    float m_nlc_pull_speed{10.f}, m_nlc_pull_up{3.5f};
    u32 m_nlc_pull_ms{300};
    u32 m_nlc_pull_cooldown{12000};
    u32 m_nlc_pull_delay_ms{500};
    float m_nlc_pull_tug{8.f}; // degrees of the camera tug
    u32 m_nlc_pull_next{};
    u32 m_nlc_pull_check_next{};
    bool m_nlc_pull_ok{};
    Fvector m_nlc_pull_dir{};
    float m_nlc_pull_landing_dist{};
    bool nlc_pull_landing(const CEntityAlive* enemy, Fvector& dir, float& landing) const;
    void nlc_pull_fire();

    // elevation: the enemy up on a roof is pinned in memory and watched from a point with a line of sight
    bool m_nlc_elev_enabled{};
    bool m_nlc_elev{};
    float m_nlc_elev_min_height{1.8f};
    float m_nlc_elev_height{};
    u32 m_nlc_elev_change_since{};
    u32 m_nlc_elev_unseen_since{};
    u32 m_nlc_elev_next{};
    u32 m_nlc_elev_patience{90000}; // the pin lasts this long after the last contact
    bool m_nlc_elev_pinned{};
    float m_nlc_overwatch_min{10.f}, m_nlc_overwatch_max{20.f};
    u32 m_nlc_overwatch_hold_min{4000}, m_nlc_overwatch_hold_max{8000};
    u32 m_nlc_overwatch_settle_until{}; // just arrived: it turns to the enemy before the point counts as lost
    void nlc_elev_update();
    bool nlc_overwatch_pick();

    // gaze: mutual sight fills a meter; the stare window fills it fast; full = the stamina attack
    bool m_nlc_gaze{};
    float m_nlc_gaze_level{};
    float m_nlc_gaze_range{25.f};
    float m_nlc_gaze_min_dist{4.f};
    float m_nlc_gaze_cone{12.f}, m_nlc_gaze_cone_full{3.f}; // degrees off its head
    float m_nlc_gaze_gain{0.6f}; // per second at full contact in the window
    float m_nlc_gaze_passive_k{0.25f}; // outside the window
    float m_nlc_gaze_passive_cap{0.5f};
    float m_nlc_gaze_loss{0.5f};
    float m_nlc_gaze_drain{0.15f}; // stamina per second at a full meter (from half)
    float m_nlc_gaze_seize_stamina{0.5f};
    u32 m_nlc_gaze_window_min{2000}, m_nlc_gaze_window_max{3000};
    u32 m_nlc_gaze_cooldown{8000};
    u32 m_nlc_gaze_look_ms{500};
    u32 m_nlc_gaze_next{};
    u32 m_nlc_gaze_look_since{};
    u32 m_nlc_gaze_unseen_since{};
    u32 m_nlc_stare_until{};
    bool m_nlc_gaze_seize{}; // the next StaminaHit is the gaze's seize
    u32 m_nlc_gaze_force_until{};
    u32 m_nlc_gaze_script_next{};
    float m_nlc_gaze_sent{};
    float m_nlc_gaze_logged_band{};
    u32 m_nlc_glass_check_next{};
    bool m_nlc_glass{};
    ref_sound m_nlc_gaze_warn_sound;
    void nlc_gaze_update();
    void nlc_gaze_send(float level, bool window);
    void nlc_gaze_seize();
    // ---------------------------------------------------------------------

private:
    // grenade catch: own telekinesis container, so the tele attack state never touches grenades
    struct NlcGrenadeParams
    {
        bool enabled{};
        float catch_radius{10.f};
        u32 max_held{2};
        u32 min_hold_ms{700};
        u32 max_hold_ms{10000};
        float safe_dist{7.f};
        float throw_speed{14.f};
        bool airburst{true};
        u32 fuse_extra_ms{1500}; // airburst: fuse = flight time + this (time to run after it lands)
        u32 late_ms{4000}; // held this long: thrown at a closer target too
        float late_safe_dist{5.f};
    } m_nlc_gren;
    struct NlcHeldGrenade
    {
        CGrenade* grenade;
        u32 remaining; // paused fuse
        u32 caught;
        u16 sender; // CExplosive::Initiator at catch time
    };
    CTelekinesis m_nlc_gren_tele;
    xr_vector<NlcHeldGrenade> m_nlc_held;
    struct NlcHeldRocket // underbarrel / RPG rounds caught in flight
    {
        CExplosiveRocket* rocket;
        float fuse_left; // seconds
        u32 caught;
        u16 sender;
    };
    xr_vector<NlcHeldRocket> m_nlc_held_rockets;
    void nlc_rocket_update(const Fvector& eye);
    void nlc_rocket_throw(u32 index, const Fvector& target, LPCSTR reason);
    xr_vector<CObject*> m_nlc_nearest;
    u32 m_nlc_gren_scan_next{};

    void nlc_grenade_update();
    bool nlc_grenade_catch(CGrenade* grenade, u16 sender);
    void nlc_grenade_throw(u32 index, const Fvector& target, LPCSTR reason);
    void nlc_grenade_release_all();
    void nlc_on_grenade_destroyed(CGrenade* grenade);
    void nlc_grenade_forget(CObject* O);

    // scan feedback for the stealth M2 psy sense
    bool m_nlc_scan_fx{};
    u32 m_nlc_scan_fx_cooldown{30000};
    u32 m_nlc_scan_next{};
    float m_nlc_scan_prev_level{};
    bool m_nlc_scan_playing{};
    SPPInfo m_nlc_scan_ppi;
    float m_nlc_scan_time{}, m_nlc_scan_attack{}, m_nlc_scan_release{};

    void nlc_load(LPCSTR section);
    void nlc_load_pass2(LPCSTR section);
    void nlc_reinit_pass2();
    void nlc_scan_update();
    // ---------------------------------------------------------------------

public:
    CBurer();
    virtual ~CBurer();

    virtual bool nlc_siege_capable() const { return false; } // NLC: own attack state (species pass)

    virtual void reinit();
    virtual void reload(LPCSTR section);

    virtual void Load(LPCSTR section);
    virtual void PostLoad(LPCSTR section);

    virtual void net_Destroy();
    virtual void net_Relcase(CObject* O);
    virtual void shedule_Update(u32 dt);
    virtual void UpdateCL();
    virtual void Hit(SHit* pHDS);
    virtual void Die(CObject* who);
    void ProcessTurn();
    virtual void CheckSpecParams(u32 spec_params);

    void UpdateGraviObject();

    void StartGraviPrepare();
    void StopGraviPrepare();

    void StartTeleObjectParticle(CGameObject* pO);
    void StopTeleObjectParticle(CGameObject* pO);

    void ActivateShield();
    void DeactivateShield();

    bool need_shotmark() const { return !m_shield_active; }

    virtual bool ability_distant_feel() { return true; }

    /*
    #ifdef DEBUG
        virtual CBaseMonster::SDebugInfo show_debug_info();
    #endif
    */

    void set_force_gravi_attack(bool force_gravi) { m_force_gravi_attack = force_gravi; }
    bool get_force_gravi_attack() const { return m_force_gravi_attack; }

private:
    bool m_force_gravi_attack;

    void StaminaHit();

    DECLARE_SCRIPT_REGISTER_FUNCTION

    void face_enemy();
};

bool actor_is_reloading_weapon();

add_to_type_list(CBurer)
#undef script_type_list
#define script_type_list save_type_list(CBurer)
