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
    void OnRadiation(float health_lost);
    void Update(float dt);

    u8 PartByBone(u16 bone);

    float PartHealth(u8 part) const;
    float PartMaxHealth(u8 part) const;
    void ChangePartHealth(u8 part, float value);

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
    void UpdatePain(float dt);
    void UpdateBoosts(float dt);
    void AddBoost(u8 type, u8 part, float value, float time);
    void ResolveBones();

    CActorCondition* m_owner;
    bool m_enabled{};
    bool m_user_enabled{true};

    float m_health[body_part::count]{};
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

    xr_map<shared_str, u8> m_bone_names;
    xr_vector<u8> m_bone_parts;
    const void* m_bones_visual{};
};
