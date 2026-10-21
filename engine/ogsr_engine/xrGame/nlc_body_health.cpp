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
static constexpr u8 BODY_SAVE_VERSION = 2; // NLC: 2 = green health per part
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

    // NLC: one-shot guard, read even when the system is off (it then guards the single health pool)
    m_guard_types = 0;
    if (sect && pSettings->section_exist(sect))
    {
        m_guard_above = READ_IF_EXISTS(pSettings, r_float, sect, "guard_above", 2.f);
        m_guard_floor_head = READ_IF_EXISTS(pSettings, r_float, sect, "guard_floor_head", -1.f);
        m_guard_floor_body = READ_IF_EXISTS(pSettings, r_float, sect, "guard_floor_body", -1.f);
        m_guard_window_ms = READ_IF_EXISTS(pSettings, r_u32, sect, "guard_window_ms", 60);
        LPCSTR types = READ_IF_EXISTS(pSettings, r_string, sect, "guard_hit_types", nullptr);
        const int n = types ? _GetItemCount(types) : 0;
        for (int i = 0; i < n; ++i)
        {
            string64 tmp;
            _GetItem(types, i, tmp);
            bool found = false;
            for (const xr_token* t = ALife::hit_types_token; t->name; ++t)
                if (!xr_strcmp(t->name, tmp))
                {
                    m_guard_types |= 1u << u32(t->id);
                    found = true;
                }
            if (!found)
                Msg("! [body_health] guard_hit_types: unknown hit type [%s]", tmp);
        }
    }

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
    m_green_base = READ_IF_EXISTS(pSettings, r_float, sect, "green_convert_base", 0.05f);
    m_green_factor = READ_IF_EXISTS(pSettings, r_float, sect, "green_convert_factor", 0.15f);
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
        m_green[i] = 0.f;
        m_fracture[i] = 0.f;
    }
    m_green_rate = 0.f;
    m_pain = 0.f;
    m_boosts.clear();
    m_use_part = automatic;
    m_rad_lost = 0.f;
    m_rad_deficit = 0.f;
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
        P.w_float(m_green[i]);
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
        m_green[i] = 0.f;
        m_fracture[i] = 0.f;
    }
    m_pain = 0.f;
    m_boosts.clear();
    m_rad_lost = 0.f;
    m_rad_deficit = 0.f;
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
        if (version >= 2)
            m_green[i] = P.r_float();
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
    Clamp();
    m_rad_deficit = std::max(0.f, std::min(PartHealth(all), m_owner->GetMaxHealth()) - total);
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

// HealPart's spread on plain arrays (also used by Predict): the part takes up to the budget, the rest goes
// evenly over the other parts with room
static void spread_heal(float* w, const float* g, const float* mx, u8 part, float budget)
{
    using namespace body_part;
    auto room = [&](u8 i) { return std::max(0.f, mx[i] - w[i] - g[i]); };
    if (part > all && part < count)
    {
        const float v = std::min(budget, room(part));
        w[part] += v;
        budget -= v;
    }
    // the rest (or an untargeted heal) evenly over the parts with room, a few passes for parts that fill up
    for (int pass = 0; pass < 6 && budget > 1e-6f; ++pass)
    {
        u8 open = 0;
        for (u8 i = head; i < count; ++i)
            if (i != part && room(i) > 1e-6f)
                ++open;
        if (!open)
            break;
        const float share = budget / open;
        for (u8 i = head; i < count; ++i)
        {
            if (i == part)
                continue;
            const float v = std::min(share, room(i));
            w[i] += v;
            budget -= v;
        }
    }
}

void CActorBodyHealth::HealPart(u8 part, float value)
{
    if (value <= 0.f)
        return;
    if (m_rad_deficit > 0.f)
    {
        const float paid = std::min(m_rad_deficit, value);
        m_rad_deficit -= paid;
        value -= paid;
    }
    float mx[count]{};
    for (u8 i = head; i < count; ++i)
        mx[i] = PartMaxHealth(i);
    spread_heal(m_health, m_green, mx, part, value * 6.f); // in part units: the total is the average of six parts
    Clamp();
}

void CActorBodyHealth::Predict(LPCSTR section, u8 part, float total_heal, float* white, float* green) const
{
    if (part >= count)
        part = all;
    float w[count]{}, g[count]{}, mx[count]{};
    for (u8 i = head; i < count; ++i)
    {
        w[i] = m_health[i];
        g[i] = m_green[i];
        mx[i] = PartMaxHealth(i);
    }
    float deficit = m_rad_deficit;
    auto clamp_all = [&] {
        for (u8 i = head; i < count; ++i)
        {
            clamp(w[i], PART_MIN_HEALTH, mx[i]);
            clamp(g[i], 0.f, std::max(0.f, mx[i] - w[i]));
        }
    };
    // ChangePartHealth
    auto change = [&](u8 p, float v) {
        if (v > 0.f && deficit > 0.f)
        {
            const float paid = std::min(deficit, v / 6.f);
            deficit -= paid;
            v -= paid * 6.f;
        }
        for (u8 i = head; i < count; ++i)
            if (p == all)
                w[i] += v / 6.f;
            else if (p == i)
                w[i] += v;
        clamp_all();
    };

    if (section && pSettings->section_exist(section))
    {
        float eat_health = READ_IF_EXISTS(pSettings, r_float, section, "eat_health", 0.f);
        if (part != all)
        {
            static LPCSTR part_keys[count] = {"", "eat_headhealth", "eat_bodyhealth", "eat_lhhealth", "eat_rhhealth", "eat_llhealth", "eat_rlhealth"};
            eat_health = READ_IF_EXISTS(pSettings, r_float, section, part_keys[part], eat_health);
        }
        if (!fis_zero(eat_health))
            change(part, eat_health);

        const float eat_green = READ_IF_EXISTS(pSettings, r_float, section, "eat_green", 0.f);
        if (eat_green > 0.f)
        {
            u8 gp = part;
            if (gp == all)
            {
                float worst = 0.f;
                for (u8 i = head; i < count; ++i)
                    if (mx[i] - w[i] - g[i] > worst)
                    {
                        worst = mx[i] - w[i] - g[i];
                        gp = i;
                    }
            }
            if (gp != all)
                g[gp] += eat_green;
            clamp_all();
        }

        const float conv = READ_IF_EXISTS(pSettings, r_float, section, "green_convert", 0.f);
        if (conv > 0.f)
        {
            for (u8 i = head; i < count; ++i)
            {
                const float v = g[i] * std::min(conv, 1.f);
                g[i] -= v;
                w[i] += v;
            }
            clamp_all();
        }

        float time = READ_IF_EXISTS(pSettings, r_float, section, "boost_time", 0.f);
        const float restore = READ_IF_EXISTS(pSettings, r_float, section, "boost_health_restore", 0.f);
        if (time > 0.f && !fis_zero(restore))
        {
            if (!READ_IF_EXISTS(pSettings, r_bool, section, "real_time_boost", false))
                time *= Level().GetGameTimeFactor();
            change(part, restore * time);
        }
    }

    float v = total_heal;
    if (v > 0.f)
    {
        const float paid = std::min(deficit, v);
        deficit -= paid;
        v -= paid;
        spread_heal(w, g, mx, part, v * 6.f);
        clamp_all();
    }
    for (u8 i = head; i < count; ++i)
    {
        white[i] = w[i];
        green[i] = g[i];
    }
}

float CActorBodyHealth::PartGreen(u8 part) const
{
    if (part == all)
    {
        float s = 0.f;
        for (u8 i = head; i < count; ++i)
            s += m_green[i];
        return s / 6.f;
    }
    return part < count ? m_green[part] : 0.f;
}

float CActorBodyHealth::PartTotal(u8 part) const { return PartHealth(part) + PartGreen(part); }

float CActorBodyHealth::DisplayHealth(float total) const
{
    if (!Enabled())
        return total;
    return std::min(total, std::min(PartTotal(head), PartTotal(torso)));
}

// a rise of the total health worth `value` (6 x value in part units), the most damaged part first (design 34):
// each step fills the part with the most room up to the next one, so regeneration lands where it matters;
// the radiation deficit is paid first
void CActorBodyHealth::AddGreen(float value)
{
    if (value <= 0.f)
        return;
    if (m_rad_deficit > 0.f)
    {
        const float paid = std::min(m_rad_deficit, value);
        m_rad_deficit -= paid;
        value -= paid;
    }
    float budget = value * 6.f;
    auto room = [&](u8 i) { return std::max(0.f, PartMaxHealth(i) - m_health[i] - m_green[i]); };
    for (int pass = 0; pass < 8 && budget > 1e-7f; ++pass)
    {
        // the parts with the most room share the step that brings them down to the next largest room
        float top = 0.f, next = 0.f;
        for (u8 i = head; i < count; ++i)
            top = std::max(top, room(i));
        if (top <= 1e-7f)
            break;
        u8 n = 0;
        for (u8 i = head; i < count; ++i)
        {
            const float r = room(i);
            if (r >= top - 1e-6f)
                ++n;
            else
                next = std::max(next, r);
        }
        const float step = std::min(budget / n, top - next);
        for (u8 i = head; i < count; ++i)
            if (room(i) >= top - 1e-6f)
                m_green[i] += step;
        budget -= step * n;
    }
    Clamp();
}

void CActorBodyHealth::AddGreenPart(u8 part, float value)
{
    if (value <= 0.f)
        return;
    for (u8 i = head; i < count; ++i)
        if (part == all || part == i)
            m_green[i] += value;
    Clamp(); // caps green at the part's room (max - white)
}

void CActorBodyHealth::ConvertGreen(u8 part, float fraction)
{
    clamp(fraction, 0.f, 1.f);
    for (u8 i = head; i < count; ++i)
    {
        if (part != all && part != i)
            continue;
        const float v = m_green[i] * fraction;
        m_green[i] -= v;
        m_health[i] += v;
    }
    Clamp();
}

// green turns white at (base + factor x regen per game hour) per game hour; dt is in game seconds
void CActorBodyHealth::UpdateGreen(float dt)
{
    const float step = (m_green_base + m_green_factor * m_green_rate * 3600.f) / 3600.f * dt;
    if (step <= 0.f)
        return;
    for (u8 i = head; i < count; ++i)
    {
        const float v = std::min(m_green[i], step);
        m_green[i] -= v;
        m_health[i] += v;
    }
}

void CActorBodyHealth::SetInjury(u8 zone, u8 level)
{
    if (zone < 3)
        m_injury[zone] = level;
}

u8 CActorBodyHealth::Injury(u8 part) const
{
    if (part == left_arm || part == right_arm)
        return m_injury[0];
    if (part == left_leg || part == right_leg)
        return m_injury[1];
    if (part == head)
        return m_injury[2];
    return 0;
}

void CActorBodyHealth::ChangePartHealth(u8 part, float value)
{
    if (value > 0.f && m_rad_deficit > 0.f)
    {
        const float paid = std::min(m_rad_deficit, value / 6.f);
        m_rad_deficit -= paid;
        value -= paid * 6.f;
    }
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
    {
        clamp(m_health[i], PART_MIN_HEALTH, PartMaxHealth(i));
        clamp(m_green[i], 0.f, std::max(0.f, PartMaxHealth(i) - m_health[i]));
    }
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

// radiation lowers only the total health, the parts stay (A.R.E.A. UpdateRadiation)
void CActorBodyHealth::OnRadiation(float lost)
{
    if (Enabled() && lost > 0.f)
        m_rad_lost += lost;
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

    const bool guard = GuardHitType(hit->hit_type);
    if (part == none)
    {
        for (u8 i = head; i < count; ++i)
            Damage(i, lost * m_spread_hit_k, guard);
    }
    else
        Damage(part, lost * m_hit_k[part], guard);

    ChangePain(lost * (wound ? m_wound_pain_k : strike ? m_strike_pain_k : m_other_pain_k));

    if (part != none && (wound || strike) && lost > m_fracture_threshold)
    {
        const float chance = (lost - m_fracture_threshold) / (1.f - m_fracture_threshold);
        if (chance > ::Random.randF())
            AddFracture(part, lost);
    }
    Clamp();
}

// the one-shot guard works on white + green; the damage takes green first
void CActorBodyHealth::Damage(u8 part, float amount, bool guard)
{
    const float before = m_health[part] + m_green[part];
    const float after = guard ? Guard(part, before, before - amount) : before - amount;
    const float d = before - after;
    const float g = std::min(m_green[part], std::max(0.f, d));
    m_green[part] -= g;
    m_health[part] -= d - g;
}

float CActorBodyHealth::Guard(u8 part, float before, float after)
{
    // the head dies at death_head, the body (and the single pool) at death_body / 0; limbs never kill
    const float floor = part == head ? m_guard_floor_head : (part == torso || part == all) ? m_guard_floor_body : -1.f;
    if (floor < 0.f || part >= count)
        return after;

    const u32 now = Device.dwTimeGlobal;
    if (now >= m_guard_until[part])
    {
        m_guard_start[part] = before;
        m_guard_until[part] = now + m_guard_window_ms;
    }
    if (m_guard_start[part] > m_guard_above && after < floor)
    {
        const float guarded = std::max(after, std::min(before, floor));
        if (guarded > after)
            m_owner->m_nlc_last_hit.guarded = true;
        return guarded;
    }
    return after;
}

bool CActorBodyHealth::GuardWould(u8 part, float before, float after) const
{
    const float floor = part == head ? m_guard_floor_head : (part == torso || part == all) ? m_guard_floor_body : -1.f;
    return floor >= 0.f && part < count && before > m_guard_above && after < floor;
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
        m_rad_lost = 0.f;
        return;
    }

    float diff = total - m_last_total + m_rad_lost;
    m_rad_deficit += m_rad_lost;
    m_rad_lost = 0.f;
    if (diff > 0.f)
    {
        diff = std::max(0.f, diff - m_pain * m_pain_coef * dt);
        m_rad_deficit = std::max(0.f, m_rad_deficit - diff);
    }
    if (diff > 0.f)
        for (u8 i = head; i < count; ++i)
            m_health[i] += diff;
    else if (diff < 0.f) // bleeding, hunger...: green first, as hits
        for (u8 i = head; i < count; ++i)
        {
            const float g = std::min(m_green[i], -diff);
            m_green[i] -= g;
            m_health[i] += diff + g;
        }

    UpdateBoosts(dt);
    UpdateGreen(dt);

    if (m_fracture_incarnation_v > 0.f)
        for (u8 i = head; i < count; ++i)
            if (m_fracture[i] > 0.f)
                ChangeFracture(i, m_fracture_incarnation_v * dt);

    UpdatePain(dt);
    Clamp();

    bool dead = false;
    for (u8 i = head; i < count; ++i)
        if (m_health[i] + m_green[i] <= m_death_threshold[i])
            dead = true;

    const float avg = PartTotal(all);
    const float top = std::min(avg, m_owner->GetMaxHealth());
    m_rad_deficit = std::min(m_rad_deficit, std::max(0.f, top));
    if (dead || avg <= 0.f || top - m_rad_deficit <= 0.f)
        total = 0.f;
    else
        total = top - m_rad_deficit;
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

    // NLC: a patch (bandage) holds the part together as green health (DESIGN_recovery_and_body_status.md 30)
    const float eat_green = READ_IF_EXISTS(pSettings, r_float, section, "eat_green", 0.f);
    if (eat_green > 0.f)
    {
        // no part chosen and nothing bleeding: the most damaged part, never every part at once
        u8 gp = part;
        if (gp == all)
        {
            float worst = 0.f;
            for (u8 i = head; i < count; ++i)
            {
                const float room = PartMaxHealth(i) - m_health[i] - m_green[i];
                if (room > worst)
                {
                    worst = room;
                    gp = i;
                }
            }
        }
        if (gp != all)
            AddGreenPart(gp, eat_green);
    }

    const float wounds = READ_IF_EXISTS(pSettings, r_float, section, "wounds_heal_perc", 0.f);
    if (wounds > 0.f)
        ChangeBleeding(part, wounds);

    const float fracture = READ_IF_EXISTS(pSettings, r_float, section, "fracture_heal_perc", 0.f);
    if (fracture > 0.f)
        ChangeFracture(part, fracture);

    // NLC: a medkit turns this share of the green health of every part white
    const float green = READ_IF_EXISTS(pSettings, r_float, section, "green_convert", 0.f);
    if (green > 0.f)
        ConvertGreen(all, green);

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
// part_armor(part [, power, ap]): share of a bullet the outfit absorbs on the part (reference AK 5.45 FMJ:
// power 0.9, ap 0.2), -1 without a bullet-model outfit (inventory hover, design 35)
static int l_part_armor(lua_State* L)
{
    static LPCSTR bones[body_part::count] = {"", "bip01_head", "bip01_spine1", "bip01_l_upperarm", "bip01_r_upperarm", "bip01_l_thigh", "bip01_r_thigh"};
    CActor* A = Actor();
    const u8 part = arg_part(L, 1);
    if (!A || part <= body_part::all || part >= body_part::count)
        return ret_num(L, -1.f);
    IKinematics* K = smart_cast<IKinematics*>(A->Visual());
    const u16 bone = K ? K->LL_BoneID(bones[part]) : BI_NONE;
    const float power = lua_isnumber(L, 2) ? float(lua_tonumber(L, 2)) : 0.9f;
    const float ap = lua_isnumber(L, 3) ? float(lua_tonumber(L, 3)) : 0.2f;
    return ret_num(L, A->conditions().nlc_bullet_absorb(bone, power, ap));
}
static int l_change_part_health(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->ChangePartHealth(arg_part(L, 1), float(lua_tonumber(L, 2)));
    return 0;
}
// heal_part(part or "all", value): a heal worth value of total health, aimed at the part (HealPart)
static int l_heal_part(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->HealPart(arg_part(L, 1), float(lua_tonumber(L, 2)));
    return 0;
}
static int l_green(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->PartGreen(arg_part(L, 1))) : 0;
}
static int l_total(lua_State* L)
{
    CActorBodyHealth* b = body();
    return b ? ret_num(L, b->PartTotal(arg_part(L, 1))) : 0;
}
// convert_green(part or "all", fraction 0..1, default 1)
static int l_convert_green(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->ConvertGreen(arg_part(L, 1), lua_isnumber(L, 2) ? float(lua_tonumber(L, 2)) : 1.f);
    return 0;
}
// set_injury("arm" | "leg" | "head", level): sivol_actor_crit injuries for the inventory window
static int l_set_injury(lua_State* L)
{
    if (CActorBodyHealth* b = body())
    {
        LPCSTR z = lua_tostring(L, 1);
        const u8 zone = !z ? 3 : !xr_strcmp(z, "arm") ? 0 : !xr_strcmp(z, "leg") ? 1 : !xr_strcmp(z, "head") ? 2 : 3;
        b->SetInjury(zone, u8(std::clamp(int(lua_tointeger(L, 2)), 0, 3))); // 3 = splinted
    }
    return 0;
}
static int l_set_injury_system(lua_State* L)
{
    if (CActorBodyHealth* b = body())
        b->SetInjurySystem(lua_toboolean(L, 1) != 0);
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
        {"part_armor", l_part_armor},
        {"change_part_health", l_change_part_health},
        {"heal_part", l_heal_part},
        {"green", l_green},
        {"total", l_total},
        {"convert_green", l_convert_green},
        {"set_injury", l_set_injury},
        {"set_injury_system", l_set_injury_system},
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
