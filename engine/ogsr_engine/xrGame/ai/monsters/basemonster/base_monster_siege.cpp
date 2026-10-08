////////////////////////////////////////////////////////////////////////////
// NLC: siege of an elevated enemy (docs/DESIGN_monster_elevation_siege.md)
//
// An enemy out of reach because of height (roof, ledge, vehicle) is besieged instead of the stock
// home-point shuffle: the monster waits at a point the enemy cannot see, near the enemy's ground point.
// Noise or sight keeps the enemy remembered (memory pin) for a per-species patience window; a pack
// that loses too many members gives up; a monster shot at moves out of sight; coming down near a
// waiting monster gets an instant attack, repeated bait gets an escalating delay.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "base_monster.h"
#include "../ai_monster_squad.h"
#include "../ai_monster_squad_manager.h"
#include "../controlled_entity.h"
#include "../control_animation_base.h"
#include "../control_direction_base.h"
#include "../control_direction.h"
#include "../control_manager.h"
#include "../control_path_builder_base.h"
#include "../monster_cover_manager.h"
#include "../monster_home.h"
#include "../../../actor.h"
#include "../../../ai_space.h"
#include "../../../ai_object_location.h"
#include "../../../level.h"
#include "../../../memory_manager.h"
#include "../../../visual_memory_manager.h"
#include "../../../movement_manager.h"
#include "../../../restricted_object.h"
#include "../../../nlc_stealth.h"
#include "cover_point.h"
#include "level_graph.h"
#include "../state_manager.h"
#include "../state_defs.h"

bool CBaseMonster::s_nlc_siege_debug = false;

#define SIEGE_LOG(...) \
    do \
    { \
        if (s_nlc_siege_debug) \
            Msg(__VA_ARGS__); \
    } while (0)

#define EVADE_LOG(...) \
    do \
    { \
        if (nlc_evade_debug()) \
            Msg(__VA_ARGS__); \
    } while (0)

namespace
{
constexpr LPCSTR SIEGE_SECTION = "monster_elevation";

// [monster_elevation] value, overridden by the monster's own section
float siege_f(LPCSTR section, LPCSTR key, float def)
{
    const float shared = READ_IF_EXISTS(pSettings, r_float, SIEGE_SECTION, key, def);
    return READ_IF_EXISTS(pSettings, r_float, section, key, shared);
}

u32 siege_ms(LPCSTR section, LPCSTR key, u32 def) { return u32(std::max(siege_f(section, key, float(def)), 0.f)); }

bool siege_b(LPCSTR section, LPCSTR key, bool def)
{
    const bool shared = !!READ_IF_EXISTS(pSettings, r_bool, SIEGE_SECTION, key, def);
    return !!READ_IF_EXISTS(pSettings, r_bool, section, key, shared);
}

Fvector2 siege_v2(LPCSTR section, LPCSTR key, Fvector2 def)
{
    const Fvector2 shared = READ_IF_EXISTS(pSettings, r_fvector2, SIEGE_SECTION, key, def);
    Fvector2 v = READ_IF_EXISTS(pSettings, r_fvector2, section, key, shared);
    if (v.x < 0.f)
        v.x = 0.f;
    if (v.y < v.x)
        v.y = v.x;
    return v;
}

Fvector siege_v3(LPCSTR section, LPCSTR key, Fvector def)
{
    const Fvector shared = READ_IF_EXISTS(pSettings, r_fvector3, SIEGE_SECTION, key, def);
    return READ_IF_EXISTS(pSettings, r_fvector3, section, key, shared);
}

LPCSTR siege_s(LPCSTR section, LPCSTR key, LPCSTR def)
{
    LPCSTR shared = READ_IF_EXISTS(pSettings, r_string, SIEGE_SECTION, key, def);
    return READ_IF_EXISTS(pSettings, r_string, section, key, shared);
}

EAction gait_action(LPCSTR section, LPCSTR name)
{
    if (!xr_strcmp(name, "run"))
        return ACT_RUN;
    if (!xr_strcmp(name, "walk"))
        return ACT_WALK_FWD;
    if (!xr_strcmp(name, "steal"))
        return ACT_STEAL;
    Msg("! [siege] [%s]: unknown elev_gait [%s], using run", section, name);
    return ACT_RUN;
}

EAction rest_action(LPCSTR section, LPCSTR name)
{
    if (!xr_strcmp(name, "stand"))
        return ACT_STAND_IDLE;
    if (!xr_strcmp(name, "sit"))
        return ACT_SIT_IDLE;
    if (!xr_strcmp(name, "lie"))
        return ACT_LIE_IDLE;
    Msg("! [siege] [%s]: unknown elev_rest [%s], using stand", section, name);
    return ACT_STAND_IDLE;
}

u32 rand_ms(const Fvector2& range) { return u32(::Random.randF(range.x, range.y)); }

// at most this many point searches per frame over all monsters (a pack starting a siege together);
// the rest keep their state and search on a later frame. Retreats when shot are not budgeted.
constexpr u32 PICKS_PER_FRAME = 4;
u32 s_pick_frame = u32(-1);
u32 s_pick_count = 0;
bool pick_budget()
{
    if (s_pick_frame != Device.dwFrame)
    {
        s_pick_frame = Device.dwFrame;
        s_pick_count = 0;
    }
    return ++s_pick_count <= PICKS_PER_FRAME;
}

LPCSTR reason_name(u8 reason)
{
    switch (reason)
    {
    case CBaseMonster::eNlcInaccessibleHigh: return "high";
    case CBaseMonster::eNlcInaccessibleOffMap: return "off map";
    default: return "other";
    }
}
} // namespace

static LPCSTR nlc_siege_move_name(u8 move) // ENlcSiegeMove order
{
    switch (move)
    {
    case 1: return "goto";
    case 2: return "hold";
    case 3: return "peek";
    case 4: return "retreat";
    case 5: return "distract";
    case 6: return "flee";
    default: return "none";
    }
}

//////////////////////////////////////////////////////////////////////////
// helpers shared with the controller
//////////////////////////////////////////////////////////////////////////

bool CBaseMonster::nlc_point_on_map(const Fvector& wanted, Fvector& pos, u32& node) const
{
    const CRestrictedObject& restrictions = movement().restrictions();

    node = ai().level_graph().vertex_id(wanted);
    if (ai().level_graph().valid_vertex_id(node) && restrictions.accessible(wanted))
    {
        pos = wanted;
        return true;
    }

    // accessible_nearest requires an inaccessible point (VERIFY in CRestrictedObject::accessible_nearest);
    // called on an accessible one it returns some restrictor border vertex far away
    if (restrictions.accessible(wanted))
        return false; // accessible but off the ai-map

    node = restrictions.accessible_nearest(wanted, pos);
    return ai().level_graph().valid_vertex_id(node) && (pos.distance_to_xz(wanted) <= 6.f);
}

bool CBaseMonster::nlc_los_to(const Fvector& feet, const CEntityAlive* enemy, float eye_height) const
{
    Fvector from = feet;
    from.y += eye_height;
    Fvector to = enemy->Position();
    to.y += 1.4f;

    Fvector dir = Fvector().sub(to, from);
    const float range = dir.magnitude();
    if (range < EPS_L)
        return true;
    dir.div(range);

    collide::rq_result R;
    return !Level().ObjectSpace.RayPick(from, dir, range, collide::rqtStatic, R, const_cast<CBaseMonster*>(this));
}

//////////////////////////////////////////////////////////////////////////
// config and lifecycle
//////////////////////////////////////////////////////////////////////////

void CBaseMonster::nlc_siege_load(LPCSTR section)
{
    SNlcSiegeParams& p = m_nlc_siege_p;
    p.enabled = siege_b(section, "elev_enabled", false);
    p.vs_npc = siege_b(section, "elev_vs_npc", false);
    p.distractible = siege_b(section, "elev_distractible", false);
    p.grace_first = siege_ms(section, "elev_grace_first", 3000);
    p.grace_repeat = siege_ms(section, "elev_grace_repeat", 1000);
    p.reengage_close = std::max(siege_f(section, "elev_reengage_close", 10.f), 0.f);
    p.reengage_delay = siege_v3(section, "elev_reengage_delay", Fvector().set(3000.f, 6000.f, 10000.f));
    p.toggle_decay = std::max(siege_ms(section, "elev_toggle_decay", 60000), 1000u);
    p.min_episode = siege_ms(section, "elev_min_episode", 1000);
    p.reach_height = siege_f(section, "elev_reach_height", 1.6f);
    p.reach_timeout = siege_ms(section, "elev_reach_timeout", 3000);
    p.eye_height = std::clamp(siege_f(section, "elev_eye_height", 1.f), 0.2f, 4.f);
    p.ambush_dist = siege_v2(section, "elev_ambush_dist", Fvector2().set(8.f, 20.f));
    p.ambush_time = siege_ms(section, "elev_ambush_time", 45000);
    p.watch_dist = siege_v2(section, "elev_watch_dist", Fvector2().set(20.f, 35.f));
    p.watch_count = std::min(siege_ms(section, "elev_watch_count", 1), 4u);
    p.peek_interval = siege_v2(section, "elev_peek_interval", Fvector2().set(15000.f, 25000.f));
    p.peek_time = siege_ms(section, "elev_peek_time", 2500);
    p.peek_detect_k = std::clamp(siege_f(section, "elev_peek_detect_k", 1.5f), 1.f, 5.f);
    p.hold_time = siege_v2(section, "elev_hold_time", Fvector2().set(6000.f, 12000.f));
    p.growl_interval = siege_v2(section, "elev_growl_interval", Fvector2().set(8000.f, 15000.f));
    p.lock_radius = std::max(siege_f(section, "elev_lock_radius", 10.f), 1.f);
    p.patience = std::max(siege_ms(section, "elev_patience", 75000), 1000u);
    p.faint_patience = std::min(siege_ms(section, "elev_faint_patience", 30000), p.patience);
    p.distract_after = siege_ms(section, "elev_distract_after", 10000);
    p.retreat_dist = std::max(siege_f(section, "elev_retreat_dist", 28.f), 5.f);
    p.retreat_time = siege_ms(section, "elev_retreat_time", 7000);
    p.max_time = std::max(siege_ms(section, "elev_max_time", 600000), 10000u);
    // own key, else the stealth pass-5 key (pack losses before a hunt turns into a flight)
    p.flee_losses = u32(std::max(READ_IF_EXISTS(pSettings, r_float, section, "elev_flee_losses", READ_IF_EXISTS(pSettings, r_float, section, "hunt_flee_losses", 0.f)), 0.f));
    p.gait = gait_action(section, siege_s(section, "elev_gait", "run"));
    p.rest = rest_action(section, siege_s(section, "elev_rest", "stand"));
    p.pick_near_anchor = siege_b(section, "elev_pick_near_anchor", false); // NLC: evasion pass, ambush placement

    s_nlc_siege_debug = !!READ_IF_EXISTS(pSettings, r_bool, SIEGE_SECTION, "elev_debug_log", false);
}

void CBaseMonster::nlc_siege_reset()
{
    m_nlc_siege = SNlcSiege{};
    EnemyMemory.unpin();
}

//////////////////////////////////////////////////////////////////////////
// tracking (every UpdateCL): elevation episodes, the low-crate carve-out, re-engage, the gate
//////////////////////////////////////////////////////////////////////////

void CBaseMonster::nlc_siege_track()
{
    SNlcSiege& s = m_nlc_siege;
    const SNlcSiegeParams& p = m_nlc_siege_p;
    const u32 now = Device.dwTimeGlobal;

    s.want = false;
    s.owns = false;

    // the attack state was left without finalizing its substate: end the siege here (lock, ownership);
    // generous for monsters on a slow update schedule
    if (s.active && now > s.last_exec + 5000)
    {
        s.end_reason = "not executed";
        nlc_siege_end(false);
    }

    // debug: name the state that interrupted a siege ("state change")
    if (s.diag_until)
    {
        if (now >= s.diag_until || s.active)
            s.diag_until = 0;
        else if (StateMan && StateMan->get_state_type() != eStateAttack)
        {
            SIEGE_LOG("~ [siege] [%s]: interrupted by state %s, enemy %s", cName().c_str(), make_xrstr(StateMan->get_state_type()).c_str(),
                EnemyMan.get_enemy() ? "remembered" : "forgotten");
            s.diag_until = 0;
        }
    }

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!p.enabled || !m_nlc_siege_supported || !nlc_siege_capable() || !enemy || enemy->getDestroy() || (m_controlled && m_controlled->is_under_control()) ||
        (!smart_cast<const CActor*>(enemy) && !p.vs_npc))
    {
        s.elev_since = s.ground_since = s.reach_since = 0;
        if (!enemy)
            s.contact = 0;
        if (s.active)
            s.end_reason = enemy ? "not applicable" : "enemy lost";
        return;
    }

    if (enemy->ID() != s.enemy_id)
    {
        s.enemy_id = enemy->ID();
        s.toggles = 0;
        s.contact = 0;
        s.elev_since = s.ground_since = s.reach_since = s.reach_block_until = 0;
        if (s.active)
        {
            s.end_reason = "enemy changed";
            return;
        }
    }

    // B: one remembered bait episode fades per toggle_decay
    if (s.toggles && now >= s.toggle_time + p.toggle_decay)
    {
        --s.toggles;
        s.toggle_time = now;
    }

    const u32 vertex = enemy->ai_location().level_vertex_id();
    if (!ai().level_graph().valid_vertex_id(vertex))
    {
        if (s.active)
            s.end_reason = "enemy off the ai-map";
        return;
    }

    const Fvector ground = ai().level_graph().vertex_position(vertex);
    const float height = enemy->Position().y - ground.y;
    const u8 reason = m_nlc_inaccessible_reason;
    // above its ground point (a pit or a cellar below the ai-map stays stock)
    const bool geom = (reason == eNlcInaccessibleHigh || reason == eNlcInaccessibleOffMap) && height > -0.5f;

    // F: a low perch is attacked as reachable, until no bite was tried for reach_timeout, or the monster is shot
    // without biting; never during a siege (a perch change on the rock must not end it), not again for 15 s
    bool reach = false;
    if (geom && reason == eNlcInaccessibleOffMap && height <= p.reach_height && !s.active && now >= s.reach_block_until)
    {
        if (!s.reach_since)
        {
            s.reach_since = now;
            SIEGE_LOG("~ [siege] [%s]: [%s] on a low perch (%.1f m): attack", cName().c_str(), enemy->cName().c_str(), height);
        }
        const u32 last_bite = MeleeChecker.last_attempt_time();
        reach = now < std::max(s.reach_since, last_bite) + p.reach_timeout;
        LPCSTR why = "no bite attempt";
        if (reach && HitMemory.get_last_hit_time() > s.reach_since && HitMemory.get_last_hit_object() == enemy && now > last_bite + 2000)
        {
            reach = false;
            why = "shot without biting";
        }
        if (!reach)
        {
            s.reach_since = 0;
            s.reach_block_until = now + 15000;
            SIEGE_LOG("~ [siege] [%s]: low perch: %s, siege", cName().c_str(), why);
        }
    }
    else
        s.reach_since = 0;

    const bool elevated = geom && !reach;
    // a leashed monster (or an enemy outside the leash) keeps the stock home-point behaviour
    const bool home_ok = at_home() && Home->at_home(ground);
    s.owns = s.active || (geom && home_ok);

    if (elevated)
    {
        if (!s.elev_since)
            s.elev_since = now;
        s.ground_since = 0;
    }
    else
    {
        s.elev_since = 0;
        if (s.active && !s.ground_since)
            s.ground_since = now;
    }

    if (!s.active)
    {
        const u32 grace = s.toggles ? p.grace_repeat : p.grace_first;
        s.want = elevated && home_ok && now >= s.elev_since + grace;
        return;
    }

    if (!home_ok)
    {
        s.end_reason = "outside home";
        return;
    }

    if (!elevated)
    {
        const float dist = Position().distance_to(enemy->Position());
        if (dist < p.reengage_close)
        {
            s.end_reason = s.fled ? "cornered" : "reengage close";
            return;
        }
        if (!s.fled)
        {
            const float delay = (s.toggles == 0) ? p.reengage_delay.x : ((s.toggles == 1) ? p.reengage_delay.y : p.reengage_delay.z);
            if (now >= s.ground_since + u32(std::max(delay, 0.f)))
            {
                s.end_reason = "reengage delay";
                return;
            }
        }
    }

    s.want = true;
}

//////////////////////////////////////////////////////////////////////////
// the siege substate
//////////////////////////////////////////////////////////////////////////

void CBaseMonster::nlc_siege_begin()
{
    SNlcSiege& s = m_nlc_siege;
    const SNlcSiegeParams& p = m_nlc_siege_p;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    const u32 now = Device.dwTimeGlobal;

    s.active = true;
    s.start = now;
    s.last_exec = now;
    // a restart after an interruption keeps the patience running (no fresh 75 s per restart)
    s.contact = (enemy && s.contact && enemy->ID() == s.enemy_id) ? std::max(s.contact, EnemyMan.get_enemy_time_last_seen()) : now;
    s.watch = s.watcher = s.patience_over = s.max_logged = s.fled = false;
    s.pack_losses = 0;
    s.end_reason = nullptr;
    s.anchor = enemy ? enemy->Position() : Position();
    s.anchor_node = enemy ? enemy->ai_location().level_vertex_id() : ai_location().level_vertex_id();
    s.move = eNlcSiegeNone;
    s.reselect = true;
    s.target_node = s.locked_node = u32(-1);
    s.hold_until = s.move_until = s.next_los = s.next_watcher = s.next_near_miss = 0;
    s.next_peek = now + rand_ms(p.peek_interval);
    s.next_growl = now + rand_ms(p.growl_interval);
    s.hit_seen = HitMemory.get_last_hit_time();
    s.burned[0] = s.burned[1] = s.burned[2] = u32(-1);
    s.burned_next = 0;

    SIEGE_LOG("~ [siege] [%s]: start vs [%s] (%s, %.1f m up), toggles %u, patience %u s", cName().c_str(), enemy ? enemy->cName().c_str() : "?",
        reason_name(m_nlc_inaccessible_reason), enemy ? enemy->Position().y - ai().level_graph().vertex_position(s.anchor_node).y : 0.f, s.toggles, p.patience / 1000);
}

void CBaseMonster::nlc_siege_end(bool clear_anim)
{
    SNlcSiege& s = m_nlc_siege;
    if (!s.active)
        return;

    const SNlcSiegeParams& p = m_nlc_siege_p;
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    LPCSTR why = s.end_reason ? s.end_reason : (enemy ? "state change" : "enemy lost");
    const bool reengage = s.end_reason && !strncmp(s.end_reason, "reengage", 8);

    // interrupted (hit reaction, a sound state, ...) with the enemy still up there: the lock (pin) stays until it
    // runs out, so a besieger is not forgotten by the interruption; any other end releases it
    const bool keep_lock = !s.end_reason && enemy && enemy->ID() == s.enemy_id && !s.fled;
    nlc_siege_unlock();
    if (!keep_lock)
        EnemyMemory.unpin();
    if (clear_anim)
        anim().clear_override_animation();
    if (s.peek_boost)
    {
        memory().visual().m_nlc_rate_k = 1.f;
        s.peek_boost = false;
    }
    s.diag_until = keep_lock ? now + 3000 : 0;

    if (reengage)
    {
        // B: the enemy came down after a siege; repeated bait raises the next re-engage delay
        if (now >= s.start + p.min_episode)
        {
            ++s.toggles;
            s.toggle_time = now;
        }
        // the stock inaccessible ticks would send the monster to its home point for up to 3 s
        m_first_tick_enemy_inaccessible = 0;
        m_last_tick_enemy_inaccessible = 0;
    }

    // lost by silence: the stealth "lost the trail" alert (sharper senses; nlc_stealth::monster_hunt_update ends it)
    if (!enemy && s.patience_over && !s.fled)
    {
        CVisualMemoryManager& v = memory().visual();
        v.m_nlc_alert_until = now + u32(std::max(v.m_nlc_hunt_alert_ms, 0.f));
        v.m_nlc_rate_k = std::max(1.f, v.m_nlc_hunt_detect_k);
        SIEGE_LOG("~ [siege] [%s]: lost the trail, alert %.0f s", cName().c_str(), v.m_nlc_hunt_alert_ms / 1000.f);
    }

    SIEGE_LOG("~ [siege] [%s]: end (%s) after %.1f s, toggles %u, enemy sensed %.0f s ago%s", cName().c_str(), why, float(now - s.start) / 1000.f, s.toggles,
        enemy ? float(now - std::min(now, EnemyMan.get_enemy_time_last_seen())) / 1000.f : -1.f, keep_lock ? ", lock kept" : "");

    s.active = false;
    s.watcher = false;
    s.move = eNlcSiegeNone;
    s.end_reason = nullptr;
}

void CBaseMonster::nlc_siege_execute()
{
    SNlcSiege& s = m_nlc_siege;
    const SNlcSiegeParams& p = m_nlc_siege_p;
    const CEntityAlive* enemy = EnemyMan.get_enemy();

    anim().clear_override_animation();

    if (!s.active || !enemy)
    {
        set_action(ACT_STAND_IDLE);
        return;
    }

    const u32 now = Device.dwTimeGlobal;
    s.last_exec = now;

    // contact: the enemy was sensed (sight, feel, hit, its sound in sound memory, pack sharing)
    const u32 sensed = EnemyMan.get_enemy_time_last_seen();
    if (sensed > s.contact)
    {
        s.contact = sensed;
        const Fvector pos = EnemyMan.get_enemy_position();
        const bool seen = EnemyMan.see_enemy_now();
        const float moved = pos.distance_to(s.anchor);
        // sight re-anchors anywhere; a sound only within the lock radius (a shot from the next roof keeps the siege here)
        if (seen || moved <= p.lock_radius)
        {
            s.anchor = pos;
            if (pos.distance_to(enemy->Position()) < 3.f)
                s.anchor_node = enemy->ai_location().level_vertex_id();
            if (moved > 4.f)
            {
                if (s.move == eNlcSiegeHold || s.move == eNlcSiegeGoto)
                    s.reselect = true;
                SIEGE_LOG("~ [siege] [%s]: anchor moved %.1f m (%s)", cName().c_str(), moved, seen ? "sight" : "contact");
            }
        }
        if (s.patience_over)
        {
            s.patience_over = false;
            SIEGE_LOG("~ [siege] [%s]: contact again, lock renewed", cName().c_str());
        }
    }

    // C: the lock (memory pin) holds for patience after the last contact, then the stock memory time
    if (!s.fled)
    {
        const u32 patience_end = s.contact + p.patience;
        const u32 lock_end = std::min(patience_end, s.start + p.max_time);
        EnemyMemory.pin(enemy, lock_end + EnemyMemory.memory_time());
        if (now >= patience_end && !s.patience_over)
        {
            s.patience_over = true;
            SIEGE_LOG("~ [siege] [%s]: patience over (%u s without noise or sight), forgetting in %u s", cName().c_str(), p.patience / 1000,
                EnemyMemory.memory_time() / 1000);
        }
        if (!s.max_logged && now >= s.start + p.max_time)
        {
            s.max_logged = true;
            SIEGE_LOG("~ [siege] [%s]: fail-safe, lock no longer renewed after %u s", cName().c_str(), p.max_time / 1000);
        }
    }

    // pack losses: abandon the siege and flee (elev_flee_losses, else hunt_flee_losses)
    if (!s.fled && p.flee_losses && s.pack_losses >= p.flee_losses)
    {
        s.fled = true;
        EnemyMemory.unpin();
        SIEGE_LOG("~ [siege] [%s]: pack lost %u: abandon and flee", cName().c_str(), s.pack_losses);
        if (!nlc_siege_pick(eNlcSiegeFlee, "flee"))
            nlc_siege_hold(5000);
    }

    // Ambush -> Watch
    if (!s.watch && now >= s.start + p.ambush_time)
    {
        s.watch = true;
        if (s.move == eNlcSiegeHold || s.move == eNlcSiegeGoto)
            s.reselect = true;
        SIEGE_LOG("~ [siege] [%s]: watch phase", cName().c_str());
    }
    nlc_siege_update_watcher();

    // D: shot by the enemy: out of sight, farther away
    const u32 hit_time = HitMemory.get_last_hit_time();
    if (hit_time > s.hit_seen)
    {
        s.hit_seen = hit_time;
        if (!s.fled && s.move != eNlcSiegeRetreat && HitMemory.get_last_hit_object() == enemy)
        {
            nlc_siege_burn_target();
            // NLC: shot at a perch-strike spot: give it up, or the failed retreat re-picked the same spot every shot
            if (s.perch)
            {
                nlc_siege_perch_shot();
                s.perch = false;
            }
            if (!nlc_siege_pick(eNlcSiegeRetreat, "shot"))
                s.reselect = true;
        }
    }

    // movement progress
    const bool moving = s.move != eNlcSiegeNone && s.move != eNlcSiegeHold && !s.hold_until;
    if (moving)
    {
        const bool arrived = ai_location().level_vertex_id() == s.target_node || Position().distance_to_xz(s.target) < 1.5f;
        if (arrived || now >= s.move_until)
        {
            SIEGE_LOG("~ [siege] [%s]: %s %s", cName().c_str(), nlc_siege_move_name(s.move), arrived ? "reached" : "timed out");
            if (s.escape)
            {
                // NLC: side guard radial escape: now out of sight, so the next pick may move laterally
                s.escape = false;
                s.reselect = true;
                nlc_siege_hold(500);
            }
            else
                switch (s.move)
                {
                case eNlcSiegePeek: nlc_siege_hold(p.peek_time); break;
                case eNlcSiegeDistract: nlc_siege_hold(u32(::Random.randI(3000, 5000))); break;
                case eNlcSiegeRetreat: nlc_siege_hold(u32(::Random.randI(2000, 4000))); break;
                case eNlcSiegeFlee: nlc_siege_hold(10000); break;
                default: nlc_siege_hold(rand_ms(p.hold_time)); break;
                }
        }
    }
    else if (s.hold_until)
    {
        if (now >= s.hold_until)
        {
            if (s.move == eNlcSiegeFlee)
                s.hold_until = now + 10000;
            else if (s.move == eNlcSiegeHold && !s.fallback && nlc_siege_in_ring(Position()) && !nlc_siege_visible(Position(), enemy))
                s.hold_until = now + rand_ms(p.hold_time); // still a good spot: stay
            else
                s.reselect = true; // voluntary move, or the end of a peek / distraction / retreat pause
        }
        else if (s.move == eNlcSiegeHold && !s.fallback && now >= s.next_los)
        {
            // the enemy moved on the roof and can see this spot
            s.next_los = now + 400 + u32(::Random.randI(200)); // staggered over a pack
            if (nlc_siege_visible(Position(), enemy))
            {
                SIEGE_LOG("~ [siege] [%s]: exposed at the hold point", cName().c_str());
                nlc_siege_burn_target();
                s.reselect = true;
            }
        }
    }

    // watcher peek (Watch phase): a real sighting refreshes memory and spreads through pack sharing
    if (s.watcher && s.watch && !s.fled && s.move == eNlcSiegeHold && !s.reselect && now >= s.next_peek && pick_budget())
    {
        s.next_peek = now + rand_ms(p.peek_interval);
        if (!nlc_siege_pick(eNlcSiegePeek, "peek"))
            SIEGE_LOG("~ [siege] [%s]: no peek point", cName().c_str());
    }

    if ((s.reselect || s.move == eNlcSiegeNone) && pick_budget())
    {
        s.reselect = false;
        if (s.fled)
        {
            if (!nlc_siege_pick(eNlcSiegeFlee, "flee"))
                nlc_siege_hold(5000);
        }
        else if (!nlc_siege_pick(eNlcSiegeGoto, "hide"))
            nlc_siege_hold(rand_ms(p.hold_time));
    }

    // growl (0, 0 = silent species)
    if (p.growl_interval.y > 0.f && now >= s.next_growl)
    {
        s.next_growl = now + rand_ms(p.growl_interval);
        if (!s.fled)
            set_state_sound(MonsterSound::eMonsterSoundAggressive, true);
    }

    // apply
    if (s.move != eNlcSiegeNone && s.move != eNlcSiegeHold && !s.hold_until)
    {
        // the species gait (sneak, walk) only for a move that starts out of sight; exposed monsters run
        EAction act = ACT_RUN;
        if (s.move == eNlcSiegePeek || s.move == eNlcSiegeDistract)
            act = ACT_WALK_FWD;
        else if (s.move == eNlcSiegeGoto && s.sneak)
            act = p.gait;
        set_action(act);
        if (act == ACT_RUN)
        {
            anim().accel_activate(eAT_Aggressive);
            anim().accel_set_braking(false);
        }
        else
            anim().accel_deactivate();
        path().set_target_point(s.target, s.target_node);
        path().set_rebuild_time(250);
        path().set_distance_to_end(1.f);
        path().set_use_covers(false);
        path().set_use_dest_orient(false);
        return;
    }

    // holding: rest in the Watch phase (not the watchers), else stand and face
    EAction act = ACT_STAND_IDLE;
    if (s.move == eNlcSiegeHold && s.fallback)
        act = nlc_side_guard_on() ? ACT_STAND_IDLE : ACT_LIE_IDLE; // visible: keep a low profile; NLC: a side-guard species stands facing the enemy
    else if ((s.move == eNlcSiegeHold && s.watch && !s.watcher) || s.move == eNlcSiegeFlee)
        act = p.rest;
    set_action(act);
    if (act == ACT_STAND_IDLE)
        nlc_siege_face(s.move == eNlcSiegeDistract ? s.target : s.anchor);
}

//////////////////////////////////////////////////////////////////////////
// points
//////////////////////////////////////////////////////////////////////////

bool CBaseMonster::nlc_siege_pick(ENlcSiegeMove kind, LPCSTR why)
{
    SNlcSiege& s = m_nlc_siege;
    const SNlcSiegeParams& p = m_nlc_siege_p;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !ai().level_graph().valid_vertex_id(s.anchor_node))
        return false;

    const Fvector ground = ai().level_graph().vertex_position(s.anchor_node);

    // NLC: perch strike: a species that can jump up at the enemy (bloodsucker pounce) goes to its strike distance on
    // its own side instead of hiding or holding a far visible spot (a strong one stood 87 s 7.9 m from a 1.4 m rock)
    float strike_d = 0.f;
    if (kind == eNlcSiegeGoto && nlc_siege_perch_strike(enemy, strike_d))
    {
        Fvector side;
        side.set(Position().x - ground.x, 0.f, Position().z - ground.z);
        if (side.square_magnitude() < EPS_L)
            side.set(0.f, 0.f, 1.f);
        side.normalize();
        Fvector wanted;
        wanted.mad(ground, side, strike_d);
        Fvector pos;
        u32 node;
        if (nlc_point_on_map(wanted, pos, node) && Home->at_home(pos))
        {
            nlc_siege_set_target(pos, node, eNlcSiegeGoto, 6000 + u32(pos.distance_to(Position()) * 300.f));
            s.perch = true;
            s.fallback = true; // visible on purpose: stand facing the enemy, no exposure reselect
            SIEGE_LOG("~ [siege] [%s]: %s -> perch strike spot %.1f m from the enemy's ground point", cName().c_str(), why, pos.distance_to_xz(ground));
            return true;
        }
        SIEGE_LOG("~ [siege] [%s]: %s: perch strike: no spot on the ai-map", cName().c_str(), why);
    }

    Fvector2 ring;
    bool want_los = false;
    switch (kind)
    {
    case eNlcSiegePeek:
        ring.set(p.ambush_dist.x, p.watch_dist.y);
        want_los = true;
        break;
    case eNlcSiegeRetreat: ring.set(p.retreat_dist * 0.8f, p.retreat_dist * 1.2f); break;
    case eNlcSiegeFlee: ring.set(p.retreat_dist * 1.5f, p.retreat_dist * 2.2f); break;
    default: ring = s.watch ? p.watch_dist : p.ambush_dist; break;
    }

    // NLC: is the monster exposed at its current spot (one 3-ray test, shared with the sneak rule below)
    int exposed_cache = -1;
    auto exposed_here = [&]() {
        if (exposed_cache < 0)
            exposed_cache = nlc_siege_visible(Position(), enemy) ? 1 : 0;
        return exposed_cache == 1;
    };
    // NLC: side guard (evasion pass): an exposed monster does not cross the line of sight of a watching enemy
    const bool guard = nlc_side_guard_on() && (kind == eNlcSiegeGoto || kind == eNlcSiegeRetreat || kind == eNlcSiegeFlee) && exposed_here();
    // NLC: ambush placement: rank hidden points by their distance to the enemy's ground point
    const bool near_anchor = kind == eNlcSiegeGoto && !s.watch && p.pick_near_anchor;
    auto rank_of = [&](const Fvector& pt) { return near_anchor ? pt.distance_to_xz(ground) : pt.distance_to(Position()); };

    CMonsterSquad* squad = monster_squad().get_squad(this);
    auto usable = [&](const Fvector& pt, u32 node) {
        if (!ai().level_graph().valid_vertex_id(node) || !movement().restrictions().accessible(pt) || !Home->at_home(pt))
            return false;
        if (squad && node != s.locked_node && squad->is_locked_cover(node))
            return false;
        for (u32 b : s.burned)
            if (b == node)
                return false;
        const float d = pt.distance_to_xz(ground);
        if (d < ring.x * 0.9f || d > ring.y * 1.1f)
            return false;
        return !(guard && nlc_guard_lateral(pt, enemy)); // NLC
    };

    Fvector best_pos{};
    u32 best_node = u32(-1);
    float best_dist = flt_max;
    LPCSTR best_kind = nullptr;
    Fvector fall_pos{};
    u32 fall_node = u32(-1);
    float fall_r = -1.f;
    u32 rays = 0;

    // 1. a cover point graded against the elevated position (hidden kinds)
    if (!want_los)
    {
        if (const CCoverPoint* cp = CoverMan->find_cover(ground, s.anchor, ring.x, ring.y))
        {
            const Fvector cpos = cp->position();
            const u32 cnode = cp->level_vertex_id();
            if (usable(cpos, cnode))
            {
                ++rays;
                if (!nlc_siege_visible(cpos, enemy))
                {
                    best_pos = cpos;
                    best_node = cnode;
                    best_dist = rank_of(cpos); // NLC
                    best_kind = "cover";
                }
            }
        }
    }

    // 2. samples on rings around the enemy's ground point; the nearest one with the wanted visibility; hidden kinds
    // try a farther ring before settling for a visible point (open ground)
    const Fvector2 phase_ring = ring;
    for (int pass = 0; pass < (want_los ? 1 : 2) && best_node == u32(-1); ++pass)
    {
        if (pass == 1)
            ring.set(phase_ring.y, phase_ring.y * 1.7f);
        const float start = ::Random.randF(0.f, PI_MUL_2);
        for (float k : {0.15f, 0.55f, 0.95f})
        {
            const float r = ring.x + (ring.y - ring.x) * k;
            for (int i = 0; i < 8; ++i)
            {
                const float a = start + float(i) * PI_DIV_4 + k;
                Fvector pt;
                pt.set(ground.x + _cos(a) * r, ground.y, ground.z + _sin(a) * r);
                const u32 node = ai().level_graph().vertex_id(pt);
                if (!ai().level_graph().valid_vertex_id(node))
                    continue;
                pt = ai().level_graph().vertex_position(node);
                if (_abs(pt.y - ground.y) > 6.f || !usable(pt, node))
                    continue; // other floors, cliffs
                if (!want_los && pass == 0 && r > fall_r)
                {
                    fall_r = r;
                    fall_pos = pt;
                    fall_node = node;
                }
                const float my = rank_of(pt); // NLC
                if (my >= best_dist)
                    continue;
                ++rays;
                if (nlc_siege_visible(pt, enemy) != want_los)
                    continue;
                best_pos = pt;
                best_node = node;
                best_dist = my;
                best_kind = "ring";
            }
        }
    }

    // NLC: side guard: no usable hidden point without crossing the line of sight: back straight off (radial escape)
    if (best_node == u32(-1) && guard)
    {
        Fvector away;
        away.sub(Position(), enemy->Position());
        away.y = 0.f;
        const u32 mine = ai_location().level_vertex_id();
        if (away.square_magnitude() > EPS_L && ai().level_graph().valid_vertex_id(mine))
        {
            away.normalize();
            for (float d : {4.f, 7.f, 10.f, 14.f})
            {
                const float j = deg2rad(::Random.randF(-10.f, 10.f));
                const float cs = _cos(j), sn = _sin(j);
                Fvector pt;
                pt.set(Position().x + (away.x * cs - away.z * sn) * d, Position().y, Position().z + (away.x * sn + away.z * cs) * d);
                if (!ai().level_graph().valid_vertex_position(pt))
                    continue;
                const u32 node = ai().level_graph().vertex_id(pt);
                if (!ai().level_graph().valid_vertex_id(node))
                    continue;
                pt = ai().level_graph().vertex_position(node);
                if (_abs(pt.y - Position().y) > 3.f || !movement().restrictions().accessible(pt) || !Home->at_home(pt))
                    continue;
                bool burned = false;
                for (u32 b : s.burned)
                    burned = burned || b == node;
                if (burned || !ai().level_graph().valid_vertex_id(ai().level_graph().check_position_in_direction(mine, Position(), pt)))
                    continue;
                ++rays;
                if (nlc_siege_visible(pt, enemy))
                    continue;
                nlc_siege_set_target(pt, node, eNlcSiegeGoto, 6000 + u32(d * 300.f));
                s.escape = true;
                EVADE_LOG("~ [evade] [%s]: %s: side guard: radial escape %.0f m, %u checks", cName().c_str(), why, d, rays);
                return true;
            }
        }
        EVADE_LOG("~ [evade] [%s]: %s: side guard: no radial move (%u checks)", cName().c_str(), why, rays);
    }

    // NLC: a side-guard species (bloodsucker) does not hold a visible spot when a hidden one exists farther out: the
    // retreat ring first (a low rock gave a visible fallback 7 m away, held side-on for seconds)
    if (best_node == u32(-1) && kind == eNlcSiegeGoto && nlc_side_guard_on())
    {
        SIEGE_LOG("~ [siege] [%s]: %s: no close hidden point, trying the retreat ring", cName().c_str(), why);
        if (nlc_siege_pick(eNlcSiegeRetreat, "hide far"))
            return true;
    }

    if (best_node == u32(-1))
    {
        if (want_los || fall_node == u32(-1))
        {
            SIEGE_LOG("~ [siege] [%s]: %s: no %s point (%.0f-%.0f m, %u rays)", cName().c_str(), why, nlc_siege_move_name(kind), ring.x, ring.y, rays);
            return false;
        }
        // no hidden point: the farthest usable one of the phase ring, lying low there
        best_pos = fall_pos;
        best_node = fall_node;
        best_kind = "fallback (visible)";
    }

    const bool sneak = kind == eNlcSiegeGoto && p.gait != ACT_RUN && !exposed_here();
    const float dist = best_pos.distance_to(Position());
    const float speed_k = sneak ? 900.f : 300.f;
    nlc_siege_set_target(best_pos, best_node, kind, 6000 + u32(dist * speed_k));
    s.sneak = sneak;
    s.fallback = best_kind && !xr_strcmp(best_kind, "fallback (visible)");
    // a watcher peeking looks hard (the stealth hunt factor): a short peek at 30 m would rarely see anything
    if (kind == eNlcSiegePeek)
    {
        CVisualMemoryManager& v = memory().visual();
        v.m_nlc_rate_k = std::max(v.m_nlc_rate_k, p.peek_detect_k);
        s.peek_boost = true;
    }
    SIEGE_LOG("~ [siege] [%s]: %s -> %s %s, %.1f m away, %.1f m from the enemy's ground point, %u checks%s", cName().c_str(), why, nlc_siege_move_name(kind),
        best_kind, dist, best_pos.distance_to_xz(ground), rays, sneak ? ", sneaking" : "");
    return true;
}

void CBaseMonster::nlc_siege_set_target(const Fvector& pos, u32 node, ENlcSiegeMove kind, u32 timeout)
{
    SNlcSiege& s = m_nlc_siege;
    nlc_siege_unlock();

    s.move = kind;
    s.target = pos;
    s.target_node = node;
    s.hold_until = 0;
    s.sneak = false;
    s.escape = false; // NLC: set again by the side-guard escape after this call
    s.perch = false; // NLC: set again by the perch strike after this call
    s.fallback = false;
    if (s.peek_boost && kind != eNlcSiegePeek)
    {
        memory().visual().m_nlc_rate_k = 1.f;
        s.peek_boost = false;
    }
    s.move_until = Device.dwTimeGlobal + timeout;

    // waiting spots are spread over the squad (the home-point state's cover locks)
    if (kind == eNlcSiegeGoto || kind == eNlcSiegeRetreat || kind == eNlcSiegeFlee)
    {
        if (CMonsterSquad* squad = monster_squad().get_squad(this))
        {
            squad->lock_cover(node);
            s.locked_node = node;
        }
    }

    path().prepare_builder();
}

// NLC: a bloodsucker's ambush hold (cloak hold, evasion pass): holding a hidden point, or moving there unseen
bool CBaseMonster::nlc_siege_hidden_hold() const
{
    const SNlcSiege& s = m_nlc_siege;
    if (!s.active || s.fled)
        return false;
    // NLC: every hold or move of the siege except a perch-strike spot (a bloodsucker held a visible fallback fully
    // visible for 87 s); a cloaked bloodsucker takes half damage there
    return !s.perch;
}

void CBaseMonster::nlc_siege_perch_abandon()
{
    SNlcSiege& s = m_nlc_siege;
    if (!s.active || !s.perch)
        return;
    s.perch = false;
    s.fallback = false;
    s.reselect = true;
}

void CBaseMonster::nlc_siege_hold(u32 time_ms)
{
    SNlcSiege& s = m_nlc_siege;
    if (s.move == eNlcSiegeGoto || s.move == eNlcSiegeNone)
        s.move = eNlcSiegeHold;
    s.hold_until = Device.dwTimeGlobal + std::max(time_ms, 500u);
}

void CBaseMonster::nlc_siege_burn_target()
{
    SNlcSiege& s = m_nlc_siege;
    if (s.target_node == u32(-1))
        return;
    s.burned[s.burned_next % 3] = s.target_node;
    s.burned_next = u8((s.burned_next + 1) % 3);
}

void CBaseMonster::nlc_siege_unlock()
{
    SNlcSiege& s = m_nlc_siege;
    if (s.locked_node == u32(-1))
        return;
    if (CMonsterSquad* squad = monster_squad().get_squad(this))
        squad->unlock_cover(s.locked_node);
    s.locked_node = u32(-1);
}

// watchers: the lowest-ID besiegers of this enemy in the squad (elev_watch_count)
void CBaseMonster::nlc_siege_update_watcher()
{
    SNlcSiege& s = m_nlc_siege;
    const u32 now = Device.dwTimeGlobal;
    if (now < s.next_watcher)
        return;
    s.next_watcher = now + 2000;

    u32 rank = 0;
    if (CMonsterSquad* squad = monster_squad().get_squad(this))
    {
        xr_vector<CEntity*> mates;
        squad->nlc_members(mates);
        for (CEntity* e : mates)
        {
            CBaseMonster* m = smart_cast<CBaseMonster*>(e);
            if (!m || m == this || !m->g_Alive() || m->getDestroy())
                continue;
            const SNlcSiege& ms = m->m_nlc_siege;
            if (ms.active && !ms.fled && ms.enemy_id == s.enemy_id && m->ID() < ID())
                ++rank;
        }
    }

    const bool watcher = !s.fled && rank < m_nlc_siege_p.watch_count;
    if (watcher != s.watcher)
    {
        s.watcher = watcher;
        SIEGE_LOG("~ [siege] [%s]: %s", cName().c_str(), watcher ? "watcher" : "not a watcher");
    }
}

// rays from the enemy's eyes to the monster's head and to both ends of its body (sideways to the line of sight);
// static geometry only (bushes do not hide)
bool CBaseMonster::nlc_siege_visible(const Fvector& feet, const CEntityAlive* enemy) const
{
    Fvector eye = enemy->Position();
    eye.y += 1.7f;
    Fvector side = Fvector().sub(feet, eye);
    side.y = 0.f;
    if (side.square_magnitude() < EPS_L)
        return true;
    side.normalize();
    side.set(-side.z, 0.f, side.x);

    const float h = m_nlc_siege_p.eye_height;
    Fvector points[3];
    points[0].set(feet.x, feet.y + h + 0.1f, feet.z);
    points[1].mad(feet, side, 0.6f);
    points[1].y = feet.y + h * 0.7f;
    points[2].mad(feet, side, -0.6f);
    points[2].y = feet.y + h * 0.7f;

    for (const Fvector& to : points)
    {
        Fvector dir = Fvector().sub(to, eye);
        const float range = dir.magnitude();
        if (range < EPS_L)
            return true;
        dir.div(range);
        collide::rq_result R;
        if (!Level().ObjectSpace.RayPick(eye, dir, range, collide::rqtStatic, R, const_cast<CBaseMonster*>(this)))
            return true;
    }
    return false;
}

bool CBaseMonster::nlc_siege_in_ring(const Fvector& pos) const
{
    const SNlcSiege& s = m_nlc_siege;
    if (!ai().level_graph().valid_vertex_id(s.anchor_node))
        return false;
    const Fvector2& ring = s.watch ? m_nlc_siege_p.watch_dist : m_nlc_siege_p.ambush_dist;
    const float d = pos.distance_to_xz(ai().level_graph().vertex_position(s.anchor_node));
    return d >= ring.x * 0.9f && d <= ring.y * 1.1f;
}

void CBaseMonster::nlc_siege_face(const Fvector& point)
{
    Fvector dir_xz = Direction();
    dir_xz.y = 0.f;
    Fvector to = Fvector().sub(point, Position());
    to.y = 0.f;
    if (to.square_magnitude() < 0.01f || dir_xz.square_magnitude() < EPS_L)
        return;

    if (_abs(angle_between_vectors(dir_xz, to)) > deg2rad(30.f))
    {
        const bool rotate_right = control().direction().is_from_right(point);
        anim().set_override_animation(rotate_right ? eAnimStandTurnRight : eAnimStandTurnLeft, 0);
        dir().face_target(point);
    }
}

//////////////////////////////////////////////////////////////////////////
// hooks
//////////////////////////////////////////////////////////////////////////

// a sound of the besieged enemy (feel_sound_new, before the stealth faint-shot return). Sounds that reach sound
// memory already renew the lock through the enemy's last-sensed time; a faint (suppressed outer-tier) shot does
// not, so it renews here, but only up to faint_patience.
void CBaseMonster::nlc_siege_on_noise(const CObject* who, int sound_type, const Fvector& position, float power)
{
    SNlcSiege& s = m_nlc_siege;
    const SNlcSiegeParams& p = m_nlc_siege_p;
    if (!s.active || s.fled || !who || who->ID() != s.enemy_id || who != Actor())
        return;
    if (!nlc_stealth::monster_shot_is_faint(this, sound_type, power))
        return;

    const u32 now = Device.dwTimeGlobal;
    const u32 contact = (p.patience > p.faint_patience && now > p.patience - p.faint_patience) ? now - (p.patience - p.faint_patience) : now;
    if (contact > s.contact)
    {
        s.contact = contact;
        SIEGE_LOG("~ [siege] [%s]: faint shot, %.1f m from the anchor: lock at least %u s", cName().c_str(), position.distance_to(s.anchor), p.faint_patience / 1000);
    }
}

void CBaseMonster::nlc_siege_on_near_miss(const Fvector& origin)
{
    SNlcSiege& s = m_nlc_siege;
    if (!s.active || s.fled || s.move == eNlcSiegeRetreat)
        return;
    const u32 now = Device.dwTimeGlobal;
    if (now < s.next_near_miss)
        return;
    s.next_near_miss = now + 2000;
    SIEGE_LOG("~ [siege] [%s]: near miss, spot exposed", cName().c_str());
    nlc_siege_burn_target();
    s.reselect = true;
}

bool CBaseMonster::nlc_siege_on_bolt(const Fvector& position)
{
    SNlcSiege& s = m_nlc_siege;
    const SNlcSiegeParams& p = m_nlc_siege_p;
    if (!s.active || !p.distractible || s.fled || s.watcher || (s.move != eNlcSiegeHold && s.move != eNlcSiegeGoto))
        return false;
    const u32 now = Device.dwTimeGlobal;
    if (now < s.contact + p.distract_after || position.distance_to(s.anchor) <= p.lock_radius)
        return false;

    Fvector pos;
    u32 node;
    if (!nlc_point_on_map(position, pos, node) || !Home->at_home(pos))
        return false;

    nlc_siege_set_target(pos, node, eNlcSiegeDistract, 15000);
    SIEGE_LOG("~ [siege] [%s]: bolt %.1f m from the anchor: going to look", cName().c_str(), position.distance_to(s.anchor));
    return true;
}

void CBaseMonster::nlc_siege_on_pack_loss()
{
    SNlcSiege& s = m_nlc_siege;
    if (!s.active || s.fled)
        return;
    ++s.pack_losses;
    SIEGE_LOG("~ [siege] [%s]: pack loss %u of %u", cName().c_str(), s.pack_losses, m_nlc_siege_p.flee_losses);
}
