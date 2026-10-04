#pragma once
#include "../BaseMonster/base_monster.h"
#include "../controlled_entity.h"
#include "script_export_space.h"

class CPseudoGigant : public CBaseMonster, public CControlledEntity<CPseudoGigant>
{
    typedef CBaseMonster inherited;
    typedef CControlledEntity<CPseudoGigant> CControlled;

private:
    xr_vector<CObject*> m_nearest;

    // step_effector
    struct
    {
        float time;
        float amplitude;
        float period_number;
    } step_effector;

    SAttackEffector m_threaten_effector;
    ref_sound m_sound_threaten_hit; // звук, который играется в голове у актера
    ref_sound m_sound_start_threaten; // звук, который играется в голове у актера

    u32 m_time_next_threaten;

    u32 m_threaten_delay_min;
    u32 m_threaten_delay_max;
    float m_threaten_dist_min;
    float m_threaten_dist_max;

    float m_kick_damage;
    bool m_kick_hit_jumping_actor;

    u32 m_time_kick_actor_slow_down;

    SVelocityParam m_fsVelocityJumpPrepare;
    SVelocityParam m_fsVelocityJumpGround;

    LPCSTR m_kick_particles;

    // NLC: crowd stomp, splash on other creatures, precise jump dodge, grenade deflection.
    // All keys optional ([m_gigant_e] in m_giant.ltx); missing keys keep the original behavior.
    enum EStompMode
    {
        eStompNormal,
        eStompCrowd,
        eStompGrenade,
        eStompRecover,
        eStompRage,
    };
    EStompMode m_stomp_mode{eStompNormal};

    float m_crowd_radius{}; // HugeKick_Crowd_Radius
    u32 m_crowd_min{}; // HugeKick_Crowd_Min, 0 = off
    float m_height_gate{3.f}; // HugeKick_Height_Gate
    float m_splash_radius{}; // HugeKick_Splash_Radius, 0 = off
    bool m_splash_hit_neutrals{}; // HugeKick_Splash_Hit_Neutrals
    float m_damage_k_monster{1.f}; // HugeKick_Damage_K_Monster
    float m_damage_k_stalker{1.f}; // HugeKick_Damage_K_Stalker
    ALife::EHitType m_hit_type_monster{ALife::eHitTypeStrike}; // HugeKick_Hit_Type_Monster: strike | explosion
    ALife::EHitType m_hit_type_stalker{ALife::eHitTypeStrike}; // HugeKick_Hit_Type_Stalker: strike | explosion
    shared_str m_hit_bone_monster; // HugeKick_Hit_Bone_Monster: aimed bone (damage-table scale), root if missing
    float m_impulse{}; // HugeKick_Impulse (per kg, scaled by falloff)
    float m_stagger_min{}; // HugeKick_Stagger_Min, 0 = no stagger
    float m_slow_k{}; // HugeKick_Slow_K (monsters)
    u32 m_slow_time{}; // HugeKick_Slow_Time, ms
    bool m_hit_actor_in_splash{}; // HugeKick_Hit_Actor_In_Splash
    float m_dodge_height{}; // HugeKick_Dodge_Height, 0 = old is_jump() rule

    float m_grenade_radius{}; // HugeKick_Grenade_Radius, 0 = off
    u32 m_grenade_delay{}; // HugeKick_Grenade_Delay, ms
    float m_grenade_damage_k{1.f}; // HugeKick_Grenade_Damage_K
    float m_grenade_anim_speed{1.f}; // HugeKick_Grenade_Anim_Speed
    u32 m_time_next_grenade_stomp{};
    u32 m_time_next_grenade_scan{};
    u32 m_time_grenade_seen{};
    bool m_grenade_seen{};

    // NLC: rage (low health), recovery stomp (burst damage), charge (rush + empowered strike).
    // Each has its own *_Enabled switch; missing keys = off.
    bool m_rage_enabled{};
    float m_rage_health{}; // HugeKick_Rage_Health
    float m_rage_delay_k{1.f}; // HugeKick_Rage_Delay_K
    u32 m_rage_crowd_min{}; // HugeKick_Rage_Crowd_Min (0 = keep)
    float m_rage_splash_k{1.f}; // HugeKick_Rage_Splash_K
    float m_rage_speed_k{1.f}; // HugeKick_Rage_Speed_K
    bool m_rage_entry_stomp{}; // HugeKick_Rage_Entry_Stomp
    bool m_rage{};
    bool m_rage_stomp_pending{};

    bool m_recover_enabled{};
    float m_recover_damage{}; // HugeKick_Recover_Damage (fraction of max health)
    u32 m_recover_window{}; // HugeKick_Recover_Window, ms
    u32 m_recover_delay{}; // HugeKick_Recover_Delay, ms
    u32 m_time_next_recover{};
    bool m_recover_pending{};
    xr_vector<std::pair<u32, float>> m_recent_damage; // (time, fraction)

    bool m_charge_enabled{};
    float m_charge_dist_min{}, m_charge_dist_max{}; // Charge_Dist
    float m_charge_max_height{2.f}; // Charge_Max_Height
    float m_charge_speed_k{1.f}; // Charge_Speed_K
    u32 m_charge_max_time{}; // Charge_Max_Time, ms
    float m_charge_end_dist{}; // Charge_End_Dist
    u32 m_charge_delay_min{}, m_charge_delay_max{}; // Charge_Delay
    u32 m_charge_strike_window{}; // Charge_Strike_Window, ms
    float m_charge_damage_k{1.f}; // Charge_Damage_K
    float m_charge_impulse_k{1.f}; // Charge_Impulse_K
    float m_charge_stumble_k{}; // Charge_Stumble_K
    u32 m_charge_stumble_time{}; // Charge_Stumble_Time, ms
    float m_charge_contact_dist{}; // Charge_Contact_Dist: ram hit on touch during the rush (0 = off)
    float m_charge_contact_yaw{}; // Charge_Contact_Yaw, rad: facing window for the ram
    float m_charge_contact_power{}; // Charge_Contact_Power
    float m_charge_contact_impulse{}; // Charge_Contact_Impulse
    u32 m_charge_windup{}; // Charge_Windup, ms: telegraph (sound + near stop) before the rush
    float m_charge_max_turn{}; // unused since .25 (the rush follows a locked point instead)
    float m_charge_overshoot{}; // Charge_Overshoot, m: the locked point lies this far past the target's spot
    float m_charge_face_yaw{}; // Charge_Face_Yaw, rad: the giant must already face the target to start
    shared_str m_charge_windup_anim; // Charge_Windup_Anim (clip played in place during the wind-up)
    float m_charge_windup_anim_speed{1.f}; // Charge_Windup_Anim_Speed
    Fvector m_charge_point{}; // locked rush point
    u32 m_charge_vertex{u32(-1)};
    bool m_charge_locked{};
    u32 m_charge_rush_start{}; // rush begins (end of wind-up)
    Fvector m_charge_dir{}; // locked rush direction (xz)
    u32 m_time_next_charge{};
    u32 m_charge_until{}; // rush active until
    u32 m_charge_strike_until{}; // empowered strike window
    u32 m_time_next_moves_update{};

    void load_moves_params(LPCSTR section);
    void update_moves();
    void update_charge();
    float stomp_splash_radius() const { return m_rage ? m_splash_radius * m_rage_splash_k : m_splash_radius; }
    u32 stomp_crowd_min() const { return (m_rage && m_rage_crowd_min) ? m_rage_crowd_min : m_crowd_min; }

    void load_stomp_params(LPCSTR section);
    u32 count_crowd() const;
    bool is_stomp_victim(const CEntityAlive* entity) const;
    void scan_for_grenades();
    void stomp_hit(CEntityAlive* victim, float power, float falloff, ALife::EHitType hit_type, LPCSTR bone);
    void stomp_hit_actor(float falloff, float mode_k);

public:
    CPseudoGigant();
    virtual ~CPseudoGigant();

    virtual void Load(LPCSTR section);
    virtual void reinit();

    virtual bool ability_earthquake() { return true; }
    virtual void event_on_step();

    virtual bool check_start_conditions(ControlCom::EControlType type);
    virtual void on_activate_control(ControlCom::EControlType);

    virtual void on_threaten_execute();

    virtual bool threaten_skip_facing();
    virtual float threaten_anim_speed_k();
    virtual bool nlc_kill_hit_ragdoll() const { return true; }

    virtual void shedule_Update(u32 dt);
    virtual void HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type = ALife::eHitTypeWound,
                           bool draw_hit_marks = true);
    virtual void nlc_on_health_lost(const CObject* who, float fraction);
    virtual bool nlc_run_target_override(Fvector& position, u32& vertex);

    virtual void HitEntityInJump(const CEntity* pEntity);
    virtual void TranslateActionToPathParams();

    DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CPseudoGigant)
#undef script_type_list
#define script_type_list save_type_list(CPseudoGigant)
