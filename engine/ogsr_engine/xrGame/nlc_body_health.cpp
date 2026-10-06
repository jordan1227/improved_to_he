#include "stdafx.h"
#include "nlc_body_health.h"
#include "ActorCondition.h"
#include "Actor.h"
#include "Wound.h"
#include "Level.h"
#include "CharacterPhysicsSupport.h"
#include "PHMovementControl.h"
#include "../Include/xrRender/Kinematics.h"
#include "../xr_3da/bone.h"

namespace body_part
{
static LPCSTR s_names[count] = {"all", "head", "body", "left_arm", "right_arm", "left_leg", "right_leg"};

LPCSTR name(u8 part) { return part < count ? s_names[part] : "none"; }

u8 by_name(LPCSTR n)
{
    if (!n)
        return none;
    for (u8 i = 0; i < count; ++i)
        if (!xr_strcmp(n, s_names[i]))
            return i;
    if (!xr_strcmp(n, "torso"))
        return torso;
    return none;
}
} // namespace body_part

using namespace body_part;

static constexpr u32 BODY_SAVE_MAGIC = 0x48424C4E; // "NLBH"
static constexpr u8 BODY_SAVE_VERSION = 1;
static constexpr float PART_MIN_HEALTH = -0.01f;
static constexpr float FRACTURE_HEALTH_CAP = 0.9f;

CActorBodyHealth::CActorBodyHealth(CActorCondition* owner) : m_owner(owner)
{
    for (u8 i = 0; i < count; ++i)
    {
        m_health[i] = 1.f;
        m_hit_k[i] = 1.f;
        m_death_threshold[i] = -1.f;
    }
}

void CActorBodyHealth::Load(LPCSTR condition_section)
{
    LPCSTR sect = READ_IF_EXISTS(pSettings, r_string, condition_section, "body_health_sect", nullptr);
    m_enabled = sect && pSettings->section_exist(sect) && READ_IF_EXISTS(pSettings, r_bool, sect, "enabled", true);
    if (!m_enabled)
        return;

    for (u8 i = head; i < count; ++i)
    {
        string64 key;
        xr_sprintf(key, "hit_k_%s", name(i));
        m_hit_k[i] = READ_IF_EXISTS(pSettings, r_float, sect, key, 1.f);
        xr_sprintf(key, "death_%s", name(i));
        m_death_threshold[i] = READ_IF_EXISTS(pSettings, r_float, sect, key, -1.f);
    }

    m_spread_hit_k = READ_IF_EXISTS(pSettings, r_float, sect, "spread_hit_k", 1.f);
    m_pain_max = READ_IF_EXISTS(pSettings, r_float, sect, "pain_max", 1.f);
    m_pain_v = READ_IF_EXISTS(pSettings, r_float, sect, "pain_v", 0.00001f);
    m_pain_coef = READ_IF_EXISTS(pSettings, r_float, sect, "pain_coef", 0.1f);
    m_wound_pain_k = READ_IF_EXISTS(pSettings, r_float, sect, "wound_pain_k", 0.01f);
    m_wound_pain_rate = READ_IF_EXISTS(pSettings, r_float, sect, "wound_pain_rate", 0.0001f);
    m_strike_pain_k = READ_IF_EXISTS(pSettings, r_float, sect, "strike_pain_k", 0.01f);
    m_other_pain_k = READ_IF_EXISTS(pSettings, r_float, sect, "other_pain_k", 0.001f);
    m_fracture_threshold = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_threshold", 1.f);
    m_fracture_coef = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_coef", 0.f);
    m_fracture_health_k = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_health_k", 0.f);
    m_fracture_pain_k = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_pain_k", 0.01f);
    m_fracture_pain_rate = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_pain_rate", 0.0001f);
    m_fracture_incarnation_v = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_incarnation_v", 0.f);
    m_fracture_move_pain_k = READ_IF_EXISTS(pSettings, r_float, sect, "fracture_move_pain_k", 0.0001f);
    m_blood_parts_coef = READ_IF_EXISTS(pSettings, r_float, sect, "blood_parts_coef", 1.f);
    m_bleed_speed_k = READ_IF_EXISTS(pSettings, r_float, sect, "bleed_speed_k", 1.f);
    m_legs_min_power = READ_IF_EXISTS(pSettings, r_float, sect, "legs_min_power", 0.4f);
    m_legs_walk_k = READ_IF_EXISTS(pSettings, r_float, sect, "legs_walk_k", 0.5f);
    m_legs_run_k = READ_IF_EXISTS(pSettings, r_float, sect, "legs_run_k", 0.5f);
    m_legs_sprint_k = READ_IF_EXISTS(pSettings, r_float, sect, "legs_sprint_k", 0.5f);
    m_legs_min_speed = READ_IF_EXISTS(pSettings, r_float, sect, "legs_min_speed", 0.1f);
    if (fis_zero(m_blood_parts_coef))
        m_blood_parts_coef = 1.f;
    if (m_fracture_threshold > 0.999f)
        m_fracture_threshold = 0.999f;

    m_bone_names.clear();
    LPCSTR bones = READ_IF_EXISTS(pSettings, r_string, sect, "bones_sect", nullptr);
    if (bones && pSettings->section_exist(bones))
    {
        CInifile::Sect& S = pSettings->r_section(bones);
        for (const auto& item : S.Data)
        {
            const u8 part = by_name(item.second.c_str());
            if (part > all && part < count)
                m_bone_names[item.first] = part;
        }
    }
    m_bones_visual = nullptr;
}

void CActorBodyHealth::SetUserEnabled(bool on)
{
    // the state is kept while the system is off; on return Update() spreads the change of the
    // total health made meanwhile over all parts, like any hit without a bone
    m_user_enabled = on;
}

void CActorBodyHealth::Reinit()
{
    for (u8 i = 0; i < count; ++i)
    {
        m_health[i] = 1.f;
        m_fracture[i] = 0.f;
    }
    m_pain = 0.f;
    m_boosts.clear();
    m_use_part = automatic;
    m_last_total = m_owner->GetHealth();
}

void CActorBodyHealth::save(NET_Packet& P)
{
    P.w_u32(BODY_SAVE_MAGIC);
    P.w_u8(BODY_SAVE_VERSION);
    for (u8 i = head; i < count; ++i)
    {
        P.w_float(m_health[i]);
        P.w_float(m_fracture[i]);
    }
    P.w_float(m_pain);
    P.w_u16(u16(m_boosts.size()));
    for (const SBoost& b : m_boosts)
    {
        P.w_u8(b.type);
        P.w_u8(b.part);
        P.w_float(b.value);
        P.w_float(b.time);
    }
}

void CActorBodyHealth::load(IReader& P)
{
    const float total = m_owner->GetHealth();
    for (u8 i = 0; i < count; ++i)
    {
        m_health[i] = total;
        m_fracture[i] = 0.f;
    }
    m_pain = 0.f;
    m_boosts.clear();
    m_last_total = total;

    if (P.elapsed() < int(sizeof(u32) + sizeof(u8)))
        return;
    const auto pos = P.tell();
    if (P.r_u32() != BODY_SAVE_MAGIC)
    {
        P.seek(pos);
        return;
    }
    const u8 version = P.r_u8();
    for (u8 i = head; i < count; ++i)
    {
        m_health[i] = P.r_float();
        m_fracture[i] = P.r_float();
    }
    m_pain = P.r_float();
    const u16 n = P.r_u16();
    for (u16 i = 0; i < n; ++i)
    {
        SBoost b;
        b.type = P.r_u8();
        b.part = P.r_u8();
        b.value = P.r_float();
        b.time = P.r_float();
        m_boosts.push_back(b);
    }
    (void)version;
    Clamp();
}

void CActorBodyHealth::ResolveBones()
{
    IKinematics* K = m_owner->object().Visual() ? smart_cast<IKinematics*>(m_owner->object().Visual()) : nullptr;
    if (!K)
    {
        m_bone_parts.clear();
        m_bones_visual = nullptr;
        return;
    }
    if (m_bones_visual == K && !m_bone_parts.empty())
        return;

    m_bones_visual = K;
    const u16 n = K->LL_BoneCount();
    m_bone_parts.assign(n, torso);
    for (u16 i = 0; i < n; ++i)
    {
        u16 id = i;
        while (id != BI_NONE && id < n)
        {
            const auto it = m_bone_names.find(K->LL_BoneName(id));
            if (it != m_bone_names.end())
            {
                m_bone_parts[i] = it->second;
                break;
            }
            id = K->GetBoneData(id).GetParentID();
        }
    }
}

u8 CActorBodyHealth::PartByBone(u16 bone)
{
    if (bone == BI_NONE)
        return none;
    ResolveBones();
    return bone < m_bone_parts.size() ? m_bone_parts[bone] : torso;
}

float CActorBodyHealth::PartMaxHealth(u8 part) const
{
    if (part == all)
    {
        float s = 0.f;
        for (u8 i = head; i < count; ++i)
            s += PartMaxHealth(i);
        return s / 6.f;
    }
    if (part >= count)
        return 1.f;
    return 1.f - std::min(FRACTURE_HEALTH_CAP, m_fracture[part] * m_fracture_health_k);
}

float CActorBodyHealth::PartHealth(u8 part) const
{
    if (part == all)
    {
        float s = 0.f;
        for (u8 i = head; i < count; ++i)
            s += m_health[i];
        return s / 6.f;
    }
    return part < count ? m_health[part] : 0.f;
}

void CActorBodyHealth::ChangePartHealth(u8 part, float value)
{
    if (part == all)
    {
        for (u8 i = head; i < count; ++i)
            m_health[i] += value / 6.f;
    }
    else if (part < count)
        m_health[part] += value;
    Clamp();
}

void CActorBodyHealth::Clamp()
{
    for (u8 i = head; i < count; ++i)
        clamp(m_health[i], PART_MIN_HEALTH, PartMaxHealth(i));
    clamp(m_pain, 0.f, m_pain_max);
}

float CActorBodyHealth::Fracture(u8 part) const
{
    if (part == all)
    {
        float s = 0.f;
        for (u8 i = head; i < count; ++i)
            s += m_fracture[i];
        return std::min(s, 1.f);
    }
    return part < count ? m_fracture[part] : 0.f;
}

void CActorBodyHealth::ChangeFracture(u8 part, float value)
{
    auto heal = [&](u8 i, float v) {
        m_fracture[i] -= v;
        if (m_fracture[i] < m_owner->m_fMinWoundSize)
            m_fracture[i] = 0.f;
        clamp(m_fracture[i], 0.f, 1.f);
    };
    if (part == all)
    {
        for (u8 i = head; i < count; ++i)
            if (m_fracture[i] > 0.f)
                heal(i, value / m_blood_parts_coef);
    }
    else if (part < count)
        heal(part, value);
    Clamp();
}

void CActorBodyHealth::AddFracture(u8 part, float power)
{
    if (part <= all || part >= count)
        return;
    m_fracture[part] += power * m_fracture_coef;
    clamp(m_fracture[part], 0.f, 1.f);
    ChangePain(power * m_fracture_pain_k);
    Clamp();
}

float CActorBodyHealth::Bleeding(u8 part) const
{
    float s = 0.f;
    for (CWound* w : m_owner->wounds())
    {
        if (part != all && const_cast<CActorBodyHealth*>(this)->PartByBone(w->GetBoneNum()) != part)
            continue;
        s += w->TotalSize();
    }
    return s * m_bleed_speed_k;
}

void CActorBodyHealth::ChangeBleeding(u8 part, float percent)
{
    const float v = part == all ? percent / m_blood_parts_coef : percent;
    for (CWound* w : m_owner->wounds())
    {
        if (part != all && PartByBone(w->GetBoneNum()) != part)
            continue;
        w->Incarnation(v, m_owner->m_fMinWoundSize);
        if (0 == w->TotalSize())
            w->SetDestroy(true);
    }
}

void CActorBodyHealth::ChangePain(float value)
{
    m_pain += value;
    clamp(m_pain, 0.f, m_pain_max);
}

void CActorBodyHealth::OnHit(SHit* hit, float lost)
{
    if (lost <= 0.f)
        return;

    const bool strike = hit->hit_type == ALife::eHitTypeStrike || hit->hit_type == ALife::eHitTypePhysicStrike;
    const bool wound = hit->hit_type == ALife::eHitTypeWound || hit->hit_type == ALife::eHitTypeFireWound || hit->hit_type == ALife::eHitTypeExplosion;

    u8 part;
    if (hit->boneID == 0 && strike)
    {
        const int r = ::Random.randI(1, 11);
        if (r == 1)
            part = left_arm;
        else if (r == 2)
            part = right_arm;
        else if (r <= 4)
            part = ::Random.randI(2) ? head : torso;
        else if (r <= 7)
            part = left_leg;
        else
            part = right_leg;
    }
    else
        part = PartByBone(hit->boneID);

    if (part == none)
    {
        for (u8 i = head; i < count; ++i)
            m_health[i] -= lost * m_spread_hit_k;
    }
    else
        m_health[part] -= lost * m_hit_k[part];

    ChangePain(lost * (wound ? m_wound_pain_k : strike ? m_strike_pain_k : m_other_pain_k));

    if (part != none && (wound || strike) && lost > m_fracture_threshold)
    {
        const float chance = (lost - m_fracture_threshold) / (1.f - m_fracture_threshold);
        if (chance > ::Random.randF())
            AddFracture(part, lost);
    }
    Clamp();
}

void CActorBodyHealth::AddBoost(u8 type, u8 part, float value, float time)
{
    if (fis_zero(value) || time <= 0.f)
        return;
    m_boosts.push_back({type, part, value, time});
}

void CActorBodyHealth::UpdateBoosts(float dt)
{
    for (SBoost& b : m_boosts)
    {
        const float t = std::min(dt, b.time);
        b.time -= dt;
        switch (b.type)
        {
        case boost_health: ChangePartHealth(b.part, b.value * t); break;
        case boost_bleeding: ChangeBleeding(b.part, b.value * t); break;
        case boost_fracture: ChangeFracture(b.part, b.value * t); break;
        case boost_pain: ChangePain(-b.value * t); break;
        }
    }
    m_boosts.erase(std::remove_if(m_boosts.begin(), m_boosts.end(), [](const SBoost& b) { return b.time <= 0.f; }), m_boosts.end());
}

void CActorBodyHealth::UpdatePain(float dt)
{
    if (m_owner->CanBeHarmed())
    {
        if (Bleeding(all) * m_wound_pain_k > m_pain)
            ChangePain(m_wound_pain_rate * m_wound_pain_k);
        if (Fracture(all) * m_fracture_pain_k > m_pain)
            ChangePain(m_fracture_pain_rate * m_fracture_pain_k);

        const float frac = Fracture(all);
        if (frac > 0.1f)
        {
            CActor& A = m_owner->object();
            if (A.character_physics_support() && A.character_physics_support()->movement())
                ChangePain(A.character_physics_support()->movement()->GetVelocityActual() * m_fracture_move_pain_k * frac);
        }
    }
    ChangePain(-m_pain_v * dt);
}

void CActorBodyHealth::Update(float dt)
{
    if (!Enabled())
        return;

    float& total = m_owner->health();
    if (total <= 0.f)
    {
        m_last_total = total;
        return;
    }

    float diff = total - m_last_total;
    if (diff > 0.f)
        diff = std::max(0.f, diff - m_pain * m_pain_coef * dt);
    if (!fis_zero(diff))
        for (u8 i = head; i < count; ++i)
            m_health[i] += diff;

    UpdateBoosts(dt);

    if (m_fracture_incarnation_v > 0.f)
        for (u8 i = head; i < count; ++i)
            if (m_fracture[i] > 0.f)
                ChangeFracture(i, m_fracture_incarnation_v * dt);

    UpdatePain(dt);
    Clamp();

    bool dead = false;
    for (u8 i = head; i < count; ++i)
        if (m_health[i] <= m_death_threshold[i])
            dead = true;

    const float avg = PartHealth(all);
    if (dead || avg <= 0.f)
        total = 0.f;
    else
        total = std::min(avg, m_owner->GetMaxHealth());
    m_last_total = total;

    const float legs = (m_health[left_leg] + m_health[right_leg]) * 0.5f;
    const float cap = m_owner->GetMaxPower() * std::max(m_legs_min_power, std::min(1.f, legs));
    if (m_owner->m_fPower > cap)
        m_owner->m_fPower = cap;
}

float CActorBodyHealth::LegsSpeedK(bool run, bool sprint) const
{
    if (!Enabled())
        return 1.f;
    const float lost = 1.f - std::clamp((m_health[left_leg] + m_health[right_leg]) * 0.5f, 0.f, 1.f);
    float k = 1.f - lost * m_legs_walk_k;
    if (run)
        k *= 1.f - lost * m_legs_run_k;
    if (sprint)
        k *= 1.f - lost * m_legs_sprint_k;
    return std::max(k, m_legs_min_speed);
}

static u8 parse_heal_mask(LPCSTR list)
{
    u8 mask = 0;
    const int n = _GetItemCount(list);
    string64 tmp;
    for (int i = 0; i < n; ++i)
    {
        _GetItem(list, i, tmp);
        if (!xr_strcmp(tmp, "health"))
            mask |= heal_health;
        else if (!xr_strcmp(tmp, "bleeding"))
            mask |= heal_bleeding;
        else if (!xr_strcmp(tmp, "fracture"))
            mask |= heal_fracture;
    }
    return mask;
}

bool CActorBodyHealth::IsBodyItem(LPCSTR section) const { return Enabled() && section && pSettings->line_exist(section, "target_parts"); }

u8 CActorBodyHealth::ItemHealMask(LPCSTR section) const
{
    return parse_heal_mask(READ_IF_EXISTS(pSettings, r_string, section, "heal_type", "health"));
}

bool CActorBodyHealth::ItemTargetsPart(LPCSTR section, u8 part) const
{
    if (!IsBodyItem(section))
        return false;
    if (part == all)
        return READ_IF_EXISTS(pSettings, r_bool, section, "show_heal_all", false);
    LPCSTR list = pSettings->r_string(section, "target_parts");
    const int n = _GetItemCount(list);
    string64 tmp;
    for (int i = 0; i < n; ++i)
    {
        _GetItem(list, i, tmp);
        if (by_name(tmp) == part)
            return true;
    }
    return false;
}

float CActorBodyHealth::PartDamage(u8 part, u8 mask) const
{
    float d = 0.f;
    if (mask & heal_health)
        d = std::max(d, PartMaxHealth(part) - PartHealth(part));
    if (mask & heal_bleeding)
        d = std::max(d, std::min(1.f, Bleeding(part)));
    if (mask & heal_fracture)
        d = std::max(d, Fracture(part));
    return d;
}

u8 CActorBodyHealth::ResolveUsePart(LPCSTR section, u8 part) const
{
    if (part != automatic)
        return part;
    const u8 mask = ItemHealMask(section);
    u8 best = all;
    float best_d = 0.f;
    for (u8 i = head; i < count; ++i)
    {
        if (!ItemTargetsPart(section, i))
            continue;
        const float d = PartDamage(i, mask);
        if (d > best_d)
        {
            best_d = d;
            best = i;
        }
    }
    return best;
}

void CActorBodyHealth::UseItem(LPCSTR section, u8 part)
{
    if (part >= count)
        part = all;

    float eat_health = READ_IF_EXISTS(pSettings, r_float, section, "eat_health", 0.f);
    if (part != all)
    {
        static LPCSTR part_keys[count] = {"", "eat_headhealth", "eat_bodyhealth", "eat_lhhealth", "eat_rhhealth", "eat_llhealth", "eat_rlhealth"};
        eat_health = READ_IF_EXISTS(pSettings, r_float, section, part_keys[part], eat_health);
    }
    if (!fis_zero(eat_health))
        ChangePartHealth(part, eat_health);

    const float wounds = READ_IF_EXISTS(pSettings, r_float, section, "wounds_heal_perc", 0.f);
    if (wounds > 0.f)
        ChangeBleeding(part, wounds);

    const float fracture = READ_IF_EXISTS(pSettings, r_float, section, "fracture_heal_perc", 0.f);
    if (fracture > 0.f)
        ChangeFracture(part, fracture);

    const float pain = READ_IF_EXISTS(pSettings, r_float, section, "eat_pain", 0.f);
    if (!fis_zero(pain))
        ChangePain(-pain);

    float time = READ_IF_EXISTS(pSettings, r_float, section, "boost_time", 0.f);
    if (time > 0.f)
    {
        if (!READ_IF_EXISTS(pSettings, r_bool, section, "real_time_boost", false))
            time *= Level().GetGameTimeFactor();
        AddBoost(boost_health, part, READ_IF_EXISTS(pSettings, r_float, section, "boost_health_restore", 0.f), time);
        AddBoost(boost_bleeding, part, READ_IF_EXISTS(pSettings, r_float, section, "boost_bleeding_restore", 0.f), time);
        AddBoost(boost_fracture, part, READ_IF_EXISTS(pSettings, r_float, section, "boost_fracture_restore", 0.f), time);
        AddBoost(boost_pain, part, READ_IF_EXISTS(pSettings, r_float, section, "boost_pain", 0.f), time);
    }
    Clamp();
}

// ---------------------------------------------------------------------------
// Lua: body_health.*
// ---------------------------------------------------------------------------

#include "Inventory.h"
#include "InventoryOwner.h"
#include "inventory_item.h"
#include "script_engine.h"

namespace nlc_body_health
{
static CActorBodyHealth* body()
{
    CActor* A = Actor();
    return A ? &A->conditions().body() : nullptr;
}

static u8 arg_part(lua_State* L, int i) { return lua_isnumber(L, i) ? u8(lua_tointeger(L, i)) : body_part::by_name(lua_tostring(L, i)); }

static int ret_num(lua_State* L, float v)
{
    lua_pushnumber(L, v);
    return 1;
}

static int l_enabled(lua_State* L)
{
    CActorBodyHealth* b = body();
    lua_pushboolean(L, b && b->Enabled() ? 1 : 0);
    return 1;
}
static int l_set_enabled(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->SetUserEnabled(lua_toboolean(L, 1) != 0);
    return 0;
}
static int l_part_health(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->PartHealth(arg_part(L, 1))) : 0;
}
static int l_part_max_health(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->PartMaxHealth(arg_part(L, 1))) : 0;
}
static int l_change_part_health(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->ChangePartHealth(arg_part(L, 1), float(lua_tonumber(L, 2)));
    return 0;
}
static int l_fracture(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->Fracture(arg_part(L, 1))) : 0;
}
static int l_change_fracture(lua_State* L)
{
    if (CActorBodyHealth* b = body())
    {
        const float v = float(lua_tonumber(L, 2));
        if (v < 0.f)
            b->AddFracture(arg_part(L, 1), -v);
        else
            b->ChangeFracture(arg_part(L, 1), v);
    }
    return 0;
}
static int l_bleeding(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->Bleeding(arg_part(L, 1))) : 0;
}
static int l_change_bleeding(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->ChangeBleeding(arg_part(L, 1), float(lua_tonumber(L, 2)));
    return 0;
}
static int l_pain(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->Pain()) : 0;
}
static int l_pain_max(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->PainMax()) : 0;
}
static int l_change_pain(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->ChangePain(float(lua_tonumber(L, 1)));
    return 0;
}
static int l_use_part(lua_State* L)
{
    CActorBodyHealth* b = body();
    lua_pushinteger(L, b ? b->UsePart() : body_part::automatic);
    return 1;
}
static int l_part_name(lua_State* L)
{
    lua_pushstring(L, body_part::name(arg_part(L, 1)));
    return 1;
}
// apply a body item now (scripts that run the use animation); returns the resolved part
static int l_use_item(lua_State* L)
{
    CActorBodyHealth* b = body();
    LPCSTR sect = lua_tostring(L, 1);
    if (!b || !sect || !b->IsBodyItem(sect))
        return 0;
    const u8 part = b->ResolveUsePart(sect, lua_isnoneornil(L, 2) ? body_part::automatic : arg_part(L, 2));
    b->UseItem(sect, part);
    lua_pushinteger(L, part);
    return 1;
}
static int l_is_body_item(lua_State* L)
{
    CActorBodyHealth* b = body();
    LPCSTR sect = lua_tostring(L, 1);
    lua_pushboolean(L, b && sect && b->IsBodyItem(sect) ? 1 : 0);
    return 1;
}
static int l_eat_on_part(lua_State* L)
{
    CActor* A = Actor();
    CObject* O = A ? Level().Objects.net_Find(u16(lua_tointeger(L, 1))) : nullptr;
    PIItem item = smart_cast<PIItem>(O);
    bool ok = false;
    if (item && item->m_pCurrentInventory == &A->inventory())
        ok = A->inventory().Eat(item, arg_part(L, 2));
    lua_pushboolean(L, ok ? 1 : 0);
    return 1;
}

void script_register(lua_State* L)
{
    static const luaL_Reg funcs[] = {
        {"enabled", l_enabled},
        {"set_enabled", l_set_enabled},
        {"part_health", l_part_health},
        {"part_max_health", l_part_max_health},
        {"change_part_health", l_change_part_health},
        {"fracture", l_fracture},
        {"change_fracture", l_change_fracture},
        {"bleeding", l_bleeding},
        {"change_bleeding", l_change_bleeding},
        {"pain", l_pain},
        {"pain_max", l_pain_max},
        {"change_pain", l_change_pain},
        {"use_part", l_use_part},
        {"part_name", l_part_name},
        {"eat_on_part", l_eat_on_part},
        {"use_item", l_use_item},
        {"is_body_item", l_is_body_item},
        {nullptr, nullptr},
    };
    luaL_register(L, "body_health", funcs);
    lua_pop(L, 1);
}
} // namespace nlc_body_health
