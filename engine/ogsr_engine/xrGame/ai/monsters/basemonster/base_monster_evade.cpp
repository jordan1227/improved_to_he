////////////////////////////////////////////////////////////////////////////
// NLC: evasion pass (docs/DESIGN_monster_movement_under_fire.md)
//
// The threat test (does the enemy watch, aim at or lock on this monster), the zigzag approach of charging
// species, the side-guard helper (moves toward or away from a watching enemy, never across its line of sight),
// the dodge speed factor and the lateral-run sampler. Every feature has its own key and defaults to off;
// [monster_evasion] in game_relations.ltx holds the shared values, a monster section overrides them.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "base_monster.h"
#include "../ai_monster_squad.h"
#include "../ai_monster_squad_manager.h"
#include "../control_manager.h"
#include "../control_movement_base.h"
#include "../control_path_builder.h"
#include "../monster_home.h"
#include "../monster_velocity_space.h"
#include "../../../actor.h"
#include "../../../ActorEffector.h"
#include "../../../ai_space.h"
#include "../../../ai_object_location.h"
#include "../../../hudmanager.h"
#include "../../../inventory.h"
#include "../../../level.h"
#include "../../../movement_manager.h"
#include "../../../restricted_object.h"
#include "../../../Weapon.h"
#include "../../../WeaponKnife.h"
#include "../../../WeaponBinoculars.h"
#include "../../../../Include/xrRender/Kinematics.h"
#include "level_graph.h"
#include "../state_manager.h"
#include "../state_defs.h"

bool CBaseMonster::s_nlc_evade_debug = false;

#define EVADE_LOG(...) \
    do \
    { \
        if (s_nlc_evade_debug) \
            Msg(__VA_ARGS__); \
    } while (0)

namespace
{
constexpr LPCSTR EVADE_SECTION = "monster_evasion";

// [monster_evasion] value, overridden by the monster's own section
float evade_f(LPCSTR section, LPCSTR key, float def)
{
    const float shared = READ_IF_EXISTS(pSettings, r_float, EVADE_SECTION, key, def);
    return READ_IF_EXISTS(pSettings, r_float, section, key, shared);
}

bool evade_b(LPCSTR section, LPCSTR key, bool def)
{
    const bool shared = !!READ_IF_EXISTS(pSettings, r_bool, EVADE_SECTION, key, def);
    return !!READ_IF_EXISTS(pSettings, r_bool, section, key, shared);
}

Fvector2 evade_v2(LPCSTR section, LPCSTR key, Fvector2 def)
{
    const Fvector2 shared = READ_IF_EXISTS(pSettings, r_fvector2, EVADE_SECTION, key, def);
    Fvector2 v = READ_IF_EXISTS(pSettings, r_fvector2, section, key, shared);
    if (v.x < 0.f)
        v.x = 0.f;
    if (v.y < v.x)
        v.y = v.x;
    return v;
}

LPCSTR threat_text(u8 t)
{
    switch (t & 15)
    {
    case 0: return "none";
    case CBaseMonster::eThreatWatched: return "watched";
    case CBaseMonster::eThreatWatched | CBaseMonster::eThreatArmed: return "watched, armed";
    case CBaseMonster::eThreatWatched | CBaseMonster::eThreatArmed | CBaseMonster::eThreatAimed: return "watched, aimed";
    case CBaseMonster::eThreatWatched | CBaseMonster::eThreatArmed | CBaseMonster::eThreatAimed | CBaseMonster::eThreatLocked: return "watched, aimed, locked";
    case CBaseMonster::eThreatWatched | CBaseMonster::eThreatLocked: return "watched, locked";
    case CBaseMonster::eThreatWatched | CBaseMonster::eThreatArmed | CBaseMonster::eThreatLocked: return "watched, armed, locked";
    case CBaseMonster::eThreatArmed: return "armed (not watched)";
    default: return "other";
    }
}
} // namespace

//////////////////////////////////////////////////////////////////////////
// config and lifecycle
//////////////////////////////////////////////////////////////////////////

void CBaseMonster::nlc_evade_load(LPCSTR section)
{
    SNlcEvadeParams& p = m_nlc_evade_p;
    p = SNlcEvadeParams{};
    p.enabled = evade_b(section, "evade_enabled", true);
    p.watch_cone = deg2rad(std::clamp(evade_f(section, "evade_watch_cone", 55.f), 5.f, 170.f));
    p.aim_angle = deg2rad(std::clamp(evade_f(section, "evade_aim_angle", 12.f), 1.f, 90.f));
    p.zz_enabled = evade_b(section, "zz_enabled", false);
    p.zz_need_gun = std::clamp(int(evade_f(section, "zz_need_gun", 0.f)), 0, 3);
    p.zz_react_ms = u32(std::clamp(evade_f(section, "zz_react_ms", 0.f), 0.f, 5000.f));
    p.zz_dist = evade_v2(section, "zz_dist", Fvector2().set(8.f, 40.f));
    p.zz_speed_k = std::clamp(evade_f(section, "zz_speed_k", 1.2f), 1.f, 1.6f);
    p.zz_max_angle = deg2rad(std::clamp(evade_f(section, "zz_max_angle", 35.f), 0.f, 60.f));
    p.zz_leg_time = evade_v2(section, "zz_leg_time", Fvector2().set(500.f, 900.f));
    p.zz_leg_time.x = std::max(p.zz_leg_time.x, 200.f);
    p.zz_leg_time.y = std::max(p.zz_leg_time.y, p.zz_leg_time.x);
    p.zz_aim_leg_k = std::clamp(evade_f(section, "zz_aim_leg_k", 0.7f), 0.2f, 1.f);
    p.zz_lock_leg_k = std::clamp(evade_f(section, "zz_lock_leg_k", 0.5f), 0.2f, 1.f);
    p.zz_same_side = std::clamp(evade_f(section, "zz_same_side", 0.25f), 0.f, 1.f);
    p.zz_fire_switch = evade_b(section, "zz_fire_switch", true);
    p.zz_vs_npc = evade_b(section, "zz_vs_npc", false);
    p.zz_seen_grace = u32(std::clamp(evade_f(section, "zz_seen_grace", 800.f), 0.f, 5000.f));
    p.zz_chance = std::clamp(evade_f(section, "zz_chance", 1.f), 0.f, 1.f);
    p.zz_burst = evade_v2(section, "zz_burst", Fvector2().set(0.f, 0.f));
    p.zz_rest = evade_v2(section, "zz_rest", Fvector2().set(0.f, 0.f));
    p.zz_turn_share = std::clamp(evade_f(section, "zz_turn_share", 0.5f), 0.1f, 1.f);
    p.zz_min_leg_k = std::clamp(evade_f(section, "zz_min_leg_k", 1.5f), 0.f, 5.f);
    p.zz_heading_gate = deg2rad(std::clamp(evade_f(section, "zz_heading_gate", 60.f), 10.f, 180.f));
    p.side_guard = evade_b(section, "side_guard", false);
    p.side_guard_angle = deg2rad(std::clamp(evade_f(section, "side_guard_angle", 30.f), 5.f, 85.f));
    p.side_guard_short = std::max(evade_f(section, "side_guard_short", 3.f), 0.f);
    p.run_attack_haste_k = std::clamp(evade_f(section, "run_attack_haste_k", 1.f), 1.f, 2.f);

    s_nlc_evade_debug = !!READ_IF_EXISTS(pSettings, r_bool, EVADE_SECTION, "evade_debug_log", false);
}

void CBaseMonster::nlc_evade_reset() { m_nlc_evade = SNlcEvade{}; }

// the enemy changed (or appeared): drop the zigzag, the threat cache and the dodge factor
bool CBaseMonster::nlc_evade_sync(const CEntityAlive* enemy)
{
    if (!enemy)
        return false;
    if (enemy->ID() != m_nlc_evade.enemy_id)
    {
        const bool was_on = m_nlc_evade.zz_on;
        nlc_evade_reset();
        m_nlc_evade.enemy_id = enemy->ID();
        if (was_on)
            EVADE_LOG("~ [evade] [%s]: zigzag stop (enemy changed)", cName().c_str());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// A: threat test
//////////////////////////////////////////////////////////////////////////

bool CBaseMonster::nlc_enemy_eye(const CEntityAlive* enemy, Fvector& eye, Fvector& view) const
{
    if (!enemy)
        return false;

    if (const CActor* actor = smart_cast<const CActor*>(enemy))
    {
        CActor* a = const_cast<CActor*>(actor);
        eye = a->Cameras().Position();
        view = a->Cameras().Direction();
        return true;
    }

    // NPC: head position (as anti_aim_ability), body direction; no skeleton or head bone: eye height above the feet
    CEntityAlive* e = const_cast<CEntityAlive*>(enemy);
    pcstr bone = "bip01_head";
    if (CBaseMonster* monster = smart_cast<CBaseMonster*>(e))
        bone = monster->get_head_bone_name();
    IKinematics* K = e->Visual() ? smart_cast<IKinematics*>(e->Visual()) : nullptr;
    if (K && K->LL_BoneID(bone) != BI_NONE)
        eye = get_head_position(e);
    else
    {
        eye = enemy->Position();
        eye.y += 1.6f;
    }
    view = enemy->Direction();
    return true;
}

bool CBaseMonster::nlc_enemy_armed(const CEntityAlive* enemy) const
{
    const CInventoryOwner* owner = smart_cast<const CInventoryOwner*>(enemy);
    if (!owner)
        return false;
    PIItem item = owner->inventory().ActiveItem();
    if (!item)
        return false;
    return smart_cast<CWeapon*>(item) && !smart_cast<CWeaponKnife*>(item) && !smart_cast<CWeaponBinoculars*>(item);
}

u8 CBaseMonster::nlc_threat()
{
    SNlcEvade& e = m_nlc_evade;
    const SNlcEvadeParams& p = m_nlc_evade_p;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !p.enabled)
    {
        e.threat = 0;
        return 0;
    }
    nlc_evade_sync(enemy);

    const u32 now = Device.dwTimeGlobal;
    if (now < e.threat_next)
        return e.threat;
    e.threat_next = now + 200;

    u8 t = 0;
    Fvector eye, view;
    if (nlc_enemy_eye(enemy, eye, view))
    {
        const bool armed = nlc_enemy_armed(enemy);
        Fvector c;
        Center(c);
        Fvector to;
        to.sub(c, eye);
        const float range = to.magnitude();
        const float ang = angle_between_vectors(view, to);
        if (ang < p.watch_cone)
        {
            bool clear = range < EPS_L;
            if (!clear)
            {
                Fvector dir = to;
                dir.div(range);
                collide::rq_result R;
                clear = !Level().ObjectSpace.RayPick(eye, dir, range, collide::rqtStatic, R, this);
            }
            if (clear)
            {
                t |= eThreatWatched;
                e.last_watched = now;
                if (armed && ang < p.aim_angle)
                    t |= eThreatAimed;
                if (smart_cast<const CActor*>(enemy) && HUD().GetCurrentRayQuery().O == this)
                    t |= eThreatLocked;
            }
        }
        if (armed)
            t |= eThreatArmed; // reported even when the monster is out of view
    }

    if (t != e.threat)
        EVADE_LOG("~ [evade] [%s]: threat %s -> %s", cName().c_str(), threat_text(e.threat), threat_text(t));
    e.threat = t;
    return t;
}

bool CBaseMonster::nlc_side_guard_watched() { return nlc_side_guard_on() && (nlc_threat() & eThreatWatched); }

//////////////////////////////////////////////////////////////////////////
// B: side guard
//////////////////////////////////////////////////////////////////////////

// a move from here to `pt` is lateral when it is longer than side_guard_short, deviates more than
// side_guard_angle from straight toward or straight away from the enemy, or needs a bent path
bool CBaseMonster::nlc_guard_lateral(const Fvector& pt, const CEntityAlive* enemy) const
{
    const SNlcEvadeParams& p = m_nlc_evade_p;
    const Fvector me = Position();

    Fvector m;
    m.sub(pt, me);
    m.y = 0.f;
    if (m.magnitude() <= p.side_guard_short)
        return false;

    Fvector r;
    r.sub(me, enemy->Position());
    r.y = 0.f;
    if (r.square_magnitude() < EPS_L)
        return false;

    const float a = angle_between_vectors(m, r);
    if (std::min(a, PI - a) > p.side_guard_angle)
        return true;

    const u32 mine = ai_location().level_vertex_id();
    if (ai().level_graph().valid_vertex_id(mine) && !ai().level_graph().valid_vertex_id(ai().level_graph().check_position_in_direction(mine, me, pt)))
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////
// C: zigzag approach
//////////////////////////////////////////////////////////////////////////

void CBaseMonster::nlc_zz_stop(const char* reason)
{
    SNlcEvade& e = m_nlc_evade;
    e.zz_leg = false;
    if (e.zz_legs_left == u32(-1))
        e.zz_legs_left = 0; // a continuous zigzag rolls again next time; a burst keeps its count and rest
    if (!e.zz_on)
        return;
    e.zz_on = false;
    nlc_clear_dodge();
    EVADE_LOG("~ [evade] [%s]: zigzag stop (%s)", cName().c_str(), reason);
}

bool CBaseMonster::nlc_zz_target(Fvector& pos, u32& node)
{
    const SNlcEvadeParams& p = m_nlc_evade_p;
    if (!p.enabled || !p.zz_enabled)
        return false; // every species without the key leaves here

    SNlcEvade& e = m_nlc_evade;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !enemy->g_Alive())
    {
        nlc_zz_stop("no enemy");
        return false;
    }
    nlc_evade_sync(enemy);

    const u32 now = Device.dwTimeGlobal;
    const Fvector me = Position();
    Fvector dir;
    dir.sub(enemy->Position(), me);
    dir.y = 0.f;
    const float dist = dir.magnitude();

    // gate; the monster's own "sees the enemy now" flickers between vision updates, so it holds for zz_seen_grace
    if (EnemyMan.see_enemy_now())
        e.zz_seen_until = now + p.zz_seen_grace;
    const char* why = nullptr;
    u8 th = 0;
    if (nlc_siege_active() || m_nlc_siege.want)
        why = "siege";
    else if (is_jumping())
        why = "jumping";
    else if (!smart_cast<const CActor*>(enemy) && !p.zz_vs_npc)
        why = "npc enemy";
    else if (now >= e.zz_seen_until)
        why = "enemy not seen";
    else if (dist < p.zz_dist.x)
        why = "close";
    else if (dist > p.zz_dist.y)
        why = "far";
    else
    {
        th = nlc_threat();
        bool ok;
        switch (p.zz_need_gun)
        {
        case 3:
        {
            // aiming down sights at it (watched, armed, the weapon zoomed) or the crosshair ray on it
            bool ads = false;
            if ((th & eThreatWatched) && (th & eThreatArmed))
                if (const CInventoryOwner* owner = smart_cast<const CInventoryOwner*>(enemy))
                    if (CWeapon* w = smart_cast<CWeapon*>(owner->inventory().ActiveItem()))
                        ads = w->IsZoomed();
            ok = ads || (th & eThreatLocked);
            break;
        }
        case 2: ok = (th & eThreatAimed) != 0; break;
        case 1: ok = (th & eThreatWatched) && (th & eThreatArmed); break;
        default: ok = (th & eThreatWatched) != 0; break;
        }
        if (ok)
        {
            if (!e.zz_ok_since)
                e.zz_ok_since = now;
            // reaction time: the first leg only after the trigger held zz_react_ms
            if (!e.zz_on && now < e.zz_ok_since + p.zz_react_ms)
                why = "reacting";
            else
                e.zz_threat_until = now + 1000;
        }
        else
        {
            e.zz_ok_since = 0;
            if (!(e.zz_on && now < e.zz_threat_until))
                why = "not threatened";
        }
    }
    if (why)
    {
        if (strcmp(why, "reacting"))
            e.zz_ok_since = 0; // the reaction time starts again after any other stop (enemy out of sight, far, ...)
        nlc_zz_stop(why);
        return false;
    }

    // a new leg: none yet, the time is up, the target is reached, or a fire switch asked for one
    if (!e.zz_leg || now >= e.zz_leg_end || me.distance_to_xz(e.zz_target) < 1.5f)
    {
        // bursts (zz_burst, zz_rest, zz_chance): a straight run between bursts; a failed roll rests too
        if (e.zz_legs_left == 0)
        {
            if (now < e.zz_rest_until)
            {
                e.zz_leg = false;
                nlc_clear_dodge();
                return false;
            }
            if (p.zz_chance < 1.f && ::Random.randF() >= p.zz_chance)
            {
                e.zz_leg = false;
                nlc_clear_dodge();
                e.zz_rest_until = now + u32(std::max(::Random.randF(p.zz_rest.x, p.zz_rest.y), 1000.f));
                EVADE_LOG("~ [evade] [%s]: zigzag burst skipped (chance %.2f), straight %.1f s", cName().c_str(), p.zz_chance, float(e.zz_rest_until - now) / 1000.f);
                return false;
            }
            e.zz_legs_left = (p.zz_burst.y > 0.f) ? u32(std::max(::Random.randI(int(p.zz_burst.x), int(p.zz_burst.y) + 1), 1)) : u32(-1);
            if (p.zz_burst.y > 0.f)
                EVADE_LOG("~ [evade] [%s]: zigzag burst of %u legs", cName().c_str(), e.zz_legs_left);
        }

        // heading gate: a leg started while the body faces away became a U-turn on the run turn radius (cats circled away)
        {
            Fvector fwd = Direction();
            fwd.y = 0.f;
            if (fwd.square_magnitude() > EPS_L && angle_between_vectors(fwd, dir) > p.zz_heading_gate)
            {
                e.zz_leg = false;
                nlc_clear_dodge();
                if (now >= e.zz_log_next)
                {
                    e.zz_log_next = now + 1000;
                    EVADE_LOG("~ [evade] [%s]: zigzag waits: heading %.0f deg off the enemy", cName().c_str(), rad2deg(angle_between_vectors(fwd, dir)));
                }
                return false;
            }
        }

        float t = std::min(std::acos(1.f / p.zz_speed_k), p.zz_max_angle);
        if (p.side_guard)
            t = std::min(t, std::max(p.side_guard_angle - deg2rad(10.f), 0.f));
        if (t < deg2rad(2.f))
        {
            nlc_zz_stop("leg angle too small");
            return false;
        }

        s8 side;
        const bool flipped = e.zz_flip;
        if (!e.zz_on || e.zz_side == 0)
            side = s8(::Random.randI(2) ? 1 : -1);
        else if (flipped)
            side = s8(-e.zz_side);
        else
            side = (::Random.randF() < p.zz_same_side) ? e.zz_side : s8(-e.zz_side);

        float ms = ::Random.randF(p.zz_leg_time.x, p.zz_leg_time.y);
        if (th & eThreatAimed)
            ms *= p.zz_aim_leg_k;
        if (th & eThreatLocked)
            ms *= p.zz_lock_leg_k;
        ms = std::max(ms, 200.f);

        const SVelocityParam& run_v = move().get_velocity(MonsterMovement::eVelocityParameterRunNormal);
        const float v_run = run_v.velocity.linear;
        // turn rate: the side switch (2 t) has to fit in zz_turn_share of the leg at the run heading speed (boosted by k
        // like the speed, see SelectVelocities); slow-turning species get a smaller angle
        const float w_run = std::max(run_v.velocity.angular_real, 0.1f);
        t = std::min(t, 0.5f * p.zz_turn_share * w_run * p.zz_speed_k * ms / 1000.f);
        if (t < deg2rad(5.f))
        {
            e.zz_leg = false;
            nlc_clear_dodge();
            return false; // too slow a turn for this leg time: straight
        }

        float len = v_run * m_nlc_speed_base * m_nlc_speed_bonus * p.zz_speed_k * ms / 1000.f;
        len = std::min(len, dist - p.zz_dist.x); // never closer than zz_dist.x to the enemy
        // a leg much shorter than the turn radius forces curves too tight to run: the path drops to walk speed
        const float min_len = std::max(1.5f, p.zz_min_leg_k * v_run / w_run);
        if (len < min_len)
        {
            if (len < 1.5f)
            {
                nlc_zz_stop("close");
                return false;
            }
            e.zz_leg = false;
            nlc_clear_dodge();
            if (now >= e.zz_log_next)
            {
                e.zz_log_next = now + 1000;
                EVADE_LOG("~ [evade] [%s]: zigzag leg too short (%.1f m < %.1f m), straight", cName().c_str(), len, min_len);
            }
            return false;
        }

        const u32 mine = ai_location().level_vertex_id();
        Fvector d = dir;
        d.div(dist);
        Fvector cand{};
        u32 cnode = u32(-1);
        auto leg_ok = [&](s8 sd) -> bool {
            const float a = float(sd) * t;
            const float cs = _cos(a), sn = _sin(a);
            Fvector ld;
            ld.set(d.x * cs - d.z * sn, 0.f, d.x * sn + d.z * cs);
            cand.mad(me, ld, len);
            if (!ai().level_graph().valid_vertex_position(cand))
                return false;
            cnode = ai().level_graph().vertex_id(cand);
            if (!ai().level_graph().valid_vertex_id(cnode))
                return false;
            cand = ai().level_graph().vertex_position(cnode);
            if (_abs(cand.y - me.y) > 3.f)
                return false;
            if (!movement().restrictions().accessible(cand) || !Home->at_home(cand))
                return false;
            if (!ai().level_graph().valid_vertex_id(mine))
                return false;
            return ai().level_graph().valid_vertex_id(ai().level_graph().check_position_in_direction(mine, me, cand));
        };

        bool changed_side = false;
        bool found = leg_ok(side);
        if (!found)
        {
            side = s8(-side);
            changed_side = true;
            found = leg_ok(side);
        }
        if (!found)
        {
            e.zz_leg = false;
            e.zz_flip = false;
            if (now >= e.zz_log_next)
            {
                e.zz_log_next = now + 1000;
                EVADE_LOG("~ [evade] [%s]: zigzag leg blocked (%.1f m, %.0f deg), straight", cName().c_str(), len, rad2deg(t));
            }
            return false;
        }

        e.zz_side = side;
        e.zz_flip = false;
        e.zz_target = cand;
        e.zz_node = cnode;
        e.zz_leg_start = now;
        e.zz_leg_end = now + u32(ms) + 300;
        e.zz_leg = true;
        if (e.zz_legs_left != u32(-1) && e.zz_legs_left > 0 && --e.zz_legs_left == 0)
            e.zz_rest_until = e.zz_leg_end + u32(::Random.randF(p.zz_rest.x, p.zz_rest.y)); // the rest starts after this leg
        if (!e.zz_on)
        {
            e.zz_on = true;
            EVADE_LOG("~ [evade] [%s]: zigzag start vs [%s] at %.1f m, threat %s", cName().c_str(), enemy->cName().c_str(), dist, threat_text(th));
        }
        EVADE_LOG("~ [evade] [%s]: zigzag leg %s %.0f deg, %.0f ms, %.1f m, k %.2f%s%s", cName().c_str(), side > 0 ? "right" : "left", rad2deg(t), ms, len, p.zz_speed_k,
            changed_side ? " (side flipped: blocked)" : (flipped ? " (fire switch)" : ""), (th & eThreatLocked) ? " (locked)" : ((th & eThreatAimed) ? " (aimed)" : ""));
    }

    if (!e.zz_leg)
        return false;

    nlc_set_dodge(p.zz_speed_k, 400); // refreshed every call; lapses 400 ms after the last one
    pos = e.zz_target;
    node = e.zz_node;
    return true;
}

void CBaseMonster::nlc_zz_on_fire()
{
    SNlcEvade& e = m_nlc_evade;
    if (!e.zz_on || !m_nlc_evade_p.zz_fire_switch || e.zz_flip)
        return;
    if (Device.dwTimeGlobal <= e.zz_leg_start + 300)
        return;
    e.zz_flip = true;
    e.zz_leg_end = 0;
    EVADE_LOG("~ [evade] [%s]: zigzag fire switch", cName().c_str());
}

//////////////////////////////////////////////////////////////////////////
// B5: lateral sampler (diagnostics)
//////////////////////////////////////////////////////////////////////////

void CBaseMonster::nlc_evade_update()
{
    if (!s_nlc_evade_debug || !nlc_side_guard_on())
        return;

    SNlcEvade& e = m_nlc_evade;
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();

    auto end_run = [&]() {
        if (e.lateral_since && now - e.lateral_since > 500)
            Msg("~ [evade] [%s]: lateral %.1f s, max %.0f deg, state %s", cName().c_str(), float(now - e.lateral_since) / 1000.f, e.lateral_max,
                StateMan ? make_xrstr(StateMan->get_state_type()).c_str() : "?");
        e.lateral_since = 0;
        e.lateral_max = 0.f;
    };

    if (!enemy)
    {
        end_run();
        return;
    }
    if (now < e.next_sample)
        return;
    e.next_sample = now + 500;
    nlc_evade_sync(enemy);

    bool lateral = false;
    float deg = 0.f;
    if (control().path_builder().is_moving_on_path() && (nlc_threat() & eThreatWatched))
    {
        Fvector d = Direction();
        d.y = 0.f;
        Fvector to;
        to.sub(enemy->Position(), Position());
        to.y = 0.f;
        if (d.square_magnitude() > EPS_L && to.square_magnitude() > EPS_L)
        {
            float a = angle_between_vectors(d, to);
            a = std::min(a, PI - a); // toward or away: 0, across: 90
            deg = rad2deg(a);
            lateral = a > m_nlc_evade_p.side_guard_angle;
        }
    }

    if (lateral)
    {
        if (!e.lateral_since)
        {
            e.lateral_since = now;
            e.lateral_max = 0.f;
        }
        e.lateral_max = std::max(e.lateral_max, deg);
    }
    else
        end_run();
}
