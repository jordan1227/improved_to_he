#pragma once

// NLC: actor health by body parts, fractures, pain (ported from A.R.E.A.).
// Total health of the actor is the average of the six parts.

class CActorCondition;
class CEatableItem;
class CWound;
class NET_Packet;
class IReader;
struct SHit;

namespace body_part
{
enum : u8
{
    all = 0,
    head,
    torso,
    left_arm,
    right_arm,
    left_leg,
    right_leg,
    count,

    automatic = 0xFF,
    none = 0xFE,
};

enum heal_type : u8
{
    heal_health = 1 << 0,
    heal_bleeding = 1 << 1,
    heal_fracture = 1 << 2,
};

LPCSTR name(u8 part);
u8 by_name(LPCSTR name);
} // namespace body_part

class CActorBodyHealth
{
public:
    explicit CActorBodyHealth(CActorCondition* owner);

    void Load(LPCSTR section);
    void Reinit();
    void save(NET_Packet& packet);
    void load(IReader& packet);

    bool Enabled() const { return m_enabled && m_user_enabled; }
    // game option: off = old NLC health, no skeleton
    void SetUserEnabled(bool on);

    void OnHit(SHit* hit, float health_lost);
    // NLC: one-shot guard (docs/damage_model/DESIGN_actor_damage_model.md A4, 15.1). Returns the guarded health
    // of a part (body_part::all = the single health pool when the system is off).
    bool GuardHitType(u32 hit_type) const { return hit_type < 32 && (m_guard_types & (1u << hit_type)); }
    float Guard(u8 part, float before, float after);
    bool GuardWould(u8 part, float before, float after) const; // no state change (god-mode record)
    void OnRadiation(float health_lost);
    void Update(float dt);

    u8 PartByBone(u16 bone);

    // white (permanent) health: everything that reads limb function uses it
    float PartHealth(u8 part) const;
    float PartMaxHealth(u8 part) const;
    void ChangePartHealth(u8 part, float value);
    // NLC: a heal worth `value` of total health, aimed at a part: the part takes up to 6 x value, the rest
    // spreads over the other parts, so the total gains value wherever it is aimed (part all: every part + value)
    void HealPart(u8 part, float value);
    // NLC: white / green of every part after using `section` on `part` (all = whole body), without side
    // effects: instant keys, the engine boost and total_heal (the treatment's total, a fraction of total
    // health, as HealPart gets it). For the part bars preview (DESIGN_recovery_and_body_status.md 32)
    void Predict(LPCSTR section, u8 part, float total_heal, float* white, float* green) const;

    // NLC green health (docs/damage_model/DESIGN_recovery_and_body_status.md 14): artifact / outfit
    // regeneration adds green to every part; green counts for survival (total health, death), is lost
    // first on hits and turns white slowly (faster with stronger regen) or at once with a medkit.
    float PartGreen(u8 part) const;
    float PartTotal(u8 part) const; // white + green
    // NLC: health for the HUD / watch bars: the total, or the head / torso when lower (those parts kill,
    // so the average must not hide a dying torso). Display only (DESIGN_recovery_and_body_status.md 31)
    float DisplayHealth(float total) const;
    void AddGreen(float value);
    void SetGreenRate(float rate) { m_green_rate = rate; } // current regen per game second
    void ConvertGreen(u8 part, float fraction);
    void AddGreenPart(u8 part, float value); // green on one part (all: every part), item key eat_green

    // injury state of sivol_actor_crit.script, pushed from Lua for the inventory window (not saved)
    void SetInjurySystem(bool on) { m_injury_system = on; }
    bool InjurySystem() const { return m_injury_system; }
    void SetInjury(u8 zone, u8 level);
    u8 Injury(u8 part) const; // level of the part's zone: arms / legs 0..2 (3 = splinted), head 1 = concussion

    float Fracture(u8 part) const;
    void ChangeFracture(u8 part, float value);
    void AddFracture(u8 part, float power);

    float Bleeding(u8 part) const;
    void ChangeBleeding(u8 part, float percent);

    float Pain() const { return m_pain; }
    float PainMax() const { return m_pain_max; }
    void ChangePain(float value);

    // item use with a chosen part (body_part::automatic = pick by heal_type)
    u8 UsePart() const { return m_use_part; }
    void SetUsePart(u8 part) { m_use_part = part; }
    bool IsBodyItem(LPCSTR section) const;
    u8 ItemHealMask(LPCSTR section) const;
    bool ItemTargetsPart(LPCSTR section, u8 part) const;
    u8 ResolveUsePart(LPCSTR section, u8 part) const;
    void UseItem(LPCSTR section, u8 part);

    float PartDamage(u8 part, u8 heal_mask) const;

    // movement speed multiplier from the legs health (A.R.E.A. area_health_system)
    float LegsSpeedK(bool run, bool sprint) const;

private:
    struct SBoost
    {
        u8 type;
        u8 part;
        float value;
        float time;
    };
    enum : u8
    {
        boost_health = 0,
        boost_bleeding,
        boost_fracture,
        boost_pain,
    };

    void Clamp();
    void Damage(u8 part, float amount, bool guard); // green first, then white
    void UpdateGreen(float dt);
    void UpdatePain(float dt);
    void UpdateBoosts(float dt);
    void AddBoost(u8 type, u8 part, float value, float time);
    void ResolveBones();

    CActorCondition* m_owner;
    bool m_enabled{};
    bool m_user_enabled{true};

    float m_health[body_part::count]{};
    float m_green[body_part::count]{};
    float m_green_rate{};
    float m_green_base{0.05f}; // white per game hour turned from green
    float m_green_factor{0.15f}; // + this x the regen per game hour
    bool m_injury_system{};
    u8 m_injury[3]{}; // arm, leg, head
    float m_fracture[body_part::count]{};
    float m_pain{};
    float m_last_total{1.f};
    float m_rad_lost{};
    float m_rad_deficit{};
    u8 m_use_part{body_part::automatic};
    xr_vector<SBoost> m_boosts;

    // config
    float m_hit_k[body_part::count]{};
    float m_death_threshold[body_part::count]{};
    float m_spread_hit_k{1.f};
    float m_pain_max{1.f};
    float m_pain_v{};
    float m_pain_coef{};
    float m_wound_pain_k{};
    float m_wound_pain_rate{};
    float m_strike_pain_k{};
    float m_other_pain_k{};
    float m_fracture_threshold{1.f};
    float m_fracture_coef{};
    float m_fracture_health_k{};
    float m_fracture_pain_k{};
    float m_fracture_pain_rate{};
    float m_fracture_incarnation_v{};
    float m_fracture_move_pain_k{};
    float m_blood_parts_coef{1.f};
    float m_bleed_speed_k{1.f};
    float m_legs_min_power{0.4f};
    float m_legs_walk_k{0.5f};
    float m_legs_run_k{0.5f};
    float m_legs_sprint_k{0.5f};
    float m_legs_min_speed{0.1f};

    // one-shot guard: a part above m_guard_above when a hit window opens cannot be taken below its
    // floor by the hits of that window (m_guard_window_ms, so the pellets of one shot count as one)
    u32 m_guard_types{};
    float m_guard_above{2.f};
    float m_guard_floor_head{-1.f};
    float m_guard_floor_body{-1.f};
    u32 m_guard_window_ms{60};
    float m_guard_start[body_part::count]{};
    u32 m_guard_until[body_part::count]{};

    xr_map<shared_str, u8> m_bone_names;
    xr_vector<u8> m_bone_parts;
    const void* m_bones_visual{};
};
