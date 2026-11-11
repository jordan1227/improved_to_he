////////////////////////////////////////////////////////////////////////////
// NLC: mutant polish pass, bloodsucker part (2026-11)
//
// Dodge budget, crosshair side, shot-timed steps, the back-off lunge, flank glance tolerance / sweep freeze /
// "being tracked" reversal, noise masking, and the group set: who counts as a group, the victim-as-shield
// approach, target weights and the timed break. Keys are read from the monster section (m_bloodsucker.ltx),
// every one defaults to the behaviour before this pass.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bloodsucker.h"
#include "../control_run_attack.h"
#include "../control_manager_custom.h"
#include "../state_manager.h"
#include "../state_defs.h"
#include "../../../actor.h"
#include "../../../ai_space.h"
#include "../../../ai_object_location.h"
#include "../../../entitycondition.h"
#include "../../../HudItem.h"
#include "../../../Weapon.h"
#include "../../../inventory.h"
#include "../../../inventory_item.h"
#include "../../../level.h"
#include "../../stalker/ai_stalker.h"
#include "level_graph.h"
#include "../anomaly_detector.h"
#include "../monster_home.h"
#include "../../../restricted_object.h"

#define P2_LOG(...) \
    do \
    { \
        if (nlc_evade_debug()) \
            Msg(__VA_ARGS__); \
    } while (0)

namespace
{
Fvector p2_rotate_y(const Fvector& v, float a)
{
    const float cs = _cos(a), sn = _sin(a);
    Fvector r;
    r.set(v.x * cs - v.z * sn, 0.f, v.x * sn + v.z * cs);
    return r;
}

Fvector p2_flat_dir(const Fvector& from, const Fvector& to)
{
    Fvector d;
    d.sub(to, from);
    d.y = 0.f;
    if (d.square_magnitude() > EPS_L)
        d.normalize();
    else
        d.set(0.f, 0.f, 0.f);
    return d;
}

// signed xz angle from a to b, in the sense of p2_rotate_y (b = rotate_y(a, angle))
float p2_signed(const Fvector& a, const Fvector& b) { return atan2f(a.x * b.z - a.z * b.x, a.x * b.x + a.z * b.z); }

CWeapon* p2_weapon(const CEntityAlive* e)
{
    const CInventoryOwner* owner = smart_cast<const CInventoryOwner*>(e);
    if (!owner)
        return nullptr;
    PIItem item = owner->inventory().ActiveItem();
    return item ? smart_cast<CWeapon*>(item) : nullptr;
}

bool p2_human(const CEntityAlive* e) { return e && (smart_cast<const CActor*>(e) || smart_cast<const CAI_Stalker*>(e)); }
} // namespace

void CAI_Bloodsucker::nlc_p2_load(LPCSTR section)
{
    SNlcBs2Params& p = m_nlc_p2;
    p = SNlcBs2Params{};
    m_nlc_s2 = SNlcBs2State{};
    if (!nlc_evade_enabled())
        return;

    auto F = [&](LPCSTR key, float def) { return READ_IF_EXISTS(pSettings, r_float, section, key, def); };
    auto B = [&](LPCSTR key, bool def) { return !!READ_IF_EXISTS(pSettings, r_bool, section, key, def); };
    auto MS = [&](LPCSTR key, u32 def) { return u32(std::max(F(key, float(def)), 0.f)); };
    auto V2 = [&](LPCSTR key, Fvector2 def) {
        Fvector2 v = READ_IF_EXISTS(pSettings, r_fvector2, section, key, def);
        v.x = std::max(v.x, 0.f);
        v.y = std::max(v.y, v.x);
        return v;
    };

    p.dodge_budget = std::clamp(int(F("melee_dodge_budget", 0.f)), 0, 8);
    p.dodge_regen_ms = std::max(MS("melee_dodge_regen_ms", 2000), 200u);
    p.dodge_budget_cd = MS("melee_dodge_budget_cd", 700);
    p.dodge_gap = V2("melee_dodge_gap", Fvector2().set(0.f, 0.f));
    p.dodge_aim_side = B("melee_dodge_aim_side", false);
    p.dodge_sweep_rate = std::max(F("melee_dodge_sweep_rate", 0.8f), 0.05f);
    p.dodge_aim_offset = deg2rad(std::clamp(F("melee_dodge_aim_offset", 2.f), 0.1f, 30.f));
    p.dodge_on_fire = B("melee_dodge_on_fire", false);
    p.dodge_behind = std::clamp(F("melee_dodge_behind", 0.f), 0.f, 1.f);
    p.dodge_doubleback = std::clamp(F("melee_dodge_doubleback", 0.f), 0.f, 1.f);
    p.dodge_npc = B("melee_dodge_npc", false);

    p.backoff_chance = std::clamp(F("backoff_lunge_chance", 0.f), 0.f, 1.f);
    p.backoff_speed = std::max(F("backoff_lunge_speed", 1.8f), 0.3f);
    p.backoff_ms = MS("backoff_lunge_ms", 350);
    p.backoff_cd = MS("backoff_lunge_cd", 5000);
    p.backoff_window_ms = std::max(MS("backoff_lunge_window_ms", 1500), 200u);
    p.backoff_min_dist = std::clamp(F("backoff_lunge_min_dist", 2.f), 1.2f, 5.f);

    p.flank_glance_ms = MS("charge_flank_glance_ms", 0);
    p.flank_sweep_s = std::clamp(F("charge_flank_sweep_s", 0.f), 0.f, 2.f);
    p.flank_freeze_ms = MS("charge_flank_freeze_ms", 1200);
    p.flank_tracked_n = std::min(MS("charge_flank_tracked_n", 0), 4u);
    p.flank_tracked_window = std::max(MS("charge_flank_tracked_window", 12000), 1000u);
    p.flank_tracked_hold = MS("charge_flank_tracked_hold", 4000);

    p.mask_dist = std::max(F("noise_mask_dist", 0.f), 0.f);
    p.mask_quiet_speed = std::max(F("noise_mask_quiet_speed", 1.5f), 0.1f);
    p.mask_quiet_ms = MS("noise_mask_quiet_ms", 1000);
    p.mask_speed_k = std::clamp(F("noise_mask_speed_k", 0.6f), 0.2f, 1.f);
    p.mask_step_k = std::clamp(F("noise_mask_step_k", 0.4f), 0.f, 1.f);

    p.grp = B("grp_enabled", false);
    p.grp_radius = std::max(F("grp_radius", 18.f), 2.f);
    p.grp_hit_ms = MS("grp_hit_ms", 10000);
    p.grp_melee_ms = MS("grp_melee_ms", 2500);
    p.grp_break_ms = V2("grp_break_ms", Fvector2().set(3000.f, 5000.f));
    p.grp_recover_add = std::clamp(F("grp_recover_add", 0.15f), 0.f, 0.6f);
    p.grp_shield = B("grp_shield", false);
    p.grp_w_isolated = std::max(F("grp_w_isolated", 1.5f), 0.1f);
    p.grp_w_unaware = std::max(F("grp_w_unaware", 1.3f), 0.1f);
    p.grp_w_reload = std::max(F("grp_w_reload", 1.4f), 0.1f);
    p.grp_w_wounded = std::max(F("grp_w_wounded", 1.3f), 0.1f);
    p.grp_isolated_dist = std::max(F("grp_isolated_dist", 8.f), 1.f);

    p.npc_flank = B("npc_flank", false);
    p.npc_backhit_k = std::max(F("npc_backhit_k", 1.f), 1.f);
    p.npc_backhit_angle = deg2rad(std::clamp(F("npc_backhit_angle", 110.f), 45.f, 180.f));

    p.npc_flank_min_dist = std::max(F("npc_flank_min_dist", 0.f), 0.f);
    p.npc_flank_angle = deg2rad(std::clamp(F("npc_flank_angle", 0.f), 0.f, 180.f));
    p.npc_strike_cloaked = B("npc_strike_cloaked", false);
    p.npc_see_cloaked = std::max(F("npc_see_cloaked", 0.f), 0.f);
    p.npc_see_xray = std::max(F("npc_see_xray", 0.f), 0.f);
    p.grp_latch_ms = MS("grp_latch_ms", 5000);
    p.scar_heal_k = std::clamp(F("scar_heal_k", 1.f), 0.f, 1.f);
    p.dmg_cloak_actor = std::clamp(F("cloak_damage_k_actor", 0.5f), 0.05f, 1.f);
    p.dmg_xray_actor = std::clamp(F("xray_damage_k_actor", 0.5f), 0.05f, 1.f);
    p.dmg_cloak_npc = std::clamp(F("cloak_damage_k_npc", 0.5f), 0.05f, 1.f);
    p.dmg_xray_npc = std::clamp(F("xray_damage_k_npc", 0.5f), 0.05f, 1.f);
    p.scar_floor = std::clamp(F("scar_floor", 0.45f), 0.1f, 1.f);
    p.npc_grudge_ms = MS("npc_grudge_ms", 0);
    p.dodge_charge_range = std::max(F("melee_dodge_charge_range", 0.f), 0.f);
    p.lunge_still_speed = std::max(F("lunge_still_speed", 0.f), 0.f);
    p.obl_angle = deg2rad(std::clamp(F("watched_oblique_angle", 0.f), 0.f, 60.f));
    p.obl_close_k = std::clamp(F("watched_oblique_close_k", 0.7f), 0.3f, 1.f);
    p.obl_leg_ms = V2("watched_oblique_leg_ms", Fvector2().set(1000.f, 1500.f));
    p.orbit_ms = MS("watched_orbit_ms", 0);
    p.orbit_dist = V2("watched_orbit_dist", Fvector2().set(12.f, 16.f));
    p.orbit_cd = MS("watched_orbit_cd", 12000);

    m_nlc_s2.charges = float(p.dodge_budget);
}

// stalker vision (CVisualMemoryManager::visible): the cloak hides it from stalkers beyond these distances
float CAI_Bloodsucker::nlc_npc_see_dist() const
{
    if (!m_nlc_on)
        return 0.f;
    switch (get_visibility_state())
    {
    case no_visibility: return m_nlc_p2.npc_see_cloaked;
    case partial_visibility: return m_nlc_p2.npc_see_xray;
    default: return 0.f;
    }
}

// the "watched" test (nlc_threat): a stalker that cannot see it through the cloak is not watching it (its body pointed
// this way ended every flank arc)
float CAI_Bloodsucker::nlc_watch_limit(const CEntityAlive* enemy) const { return nlc_enemy_stalker(enemy) ? nlc_npc_see_dist() : 0.f; }

bool CAI_Bloodsucker::nlc_npc_strike_cloaked(const CEntityAlive* enemy) const { return m_nlc_on && m_nlc_p2.npc_strike_cloaked && nlc_enemy_stalker(enemy); }

// share of a hit taken while cloaked or in x-ray (stock 0.5 for everyone): by who shoots
float CAI_Bloodsucker::nlc_cloak_damage_k(const CObject* who) const
{
    if (!m_nlc_on)
        return 0.5f;
    const bool actor = who && smart_cast<const CActor*>(who);
    if (get_visibility_state() == no_visibility)
        return actor ? m_nlc_p2.dmg_cloak_actor : m_nlc_p2.dmg_cloak_npc;
    return actor ? m_nlc_p2.dmg_xray_actor : m_nlc_p2.dmg_xray_npc;
}

// per frame (UpdateCL): dodge charges, the enemy's view sweep, its weapon fire, how quiet it is
void CAI_Bloodsucker::nlc_p2_frame()
{
    if (!m_nlc_on)
        return;
    const SNlcBs2Params& p = m_nlc_p2;
    SNlcBs2State& s = m_nlc_s2;
    const u32 now = Device.dwTimeGlobal;
    const float dt = std::min(Device.fTimeDelta, 0.2f);

    if (p.dodge_budget > 0)
        s.charges = std::min(float(p.dodge_budget), s.charges + dt * 1000.f / float(p.dodge_regen_ms));

    // a committed pounce has landed: the anomalies are avoided again (each once it is outside it)
    if (s.anom_susp && now > s.anom_susp + 500 && !com_man().is_jumping())
    {
        s.anom_susp = 0;
        anomaly_detector().nlc_resume();
    }

    // scars: regeneration stops at the ceiling earlier retreats left
    if (s.cap < 1.f)
    {
        const float top = s.cap * conditions().GetMaxHealth();
        if (conditions().GetHealth() > top)
            conditions().health() = top;
    }

    // grudge: a stalker that hit it stays in its memory (it forgot a stalker behind cover and never came back for it).
    // The memory has one pin: the tactics and the siege keep theirs, the grudge holds the rest of the time.
    if (p.npc_grudge_ms)
    {
        const TTime hit_time = HitMemory.get_last_hit_time();
        if (hit_time > s.grudge_seen)
        {
            s.grudge_seen = hit_time;
            const CAI_Stalker* who = smart_cast<const CAI_Stalker*>(HitMemory.get_last_hit_object());
            if (who && who->g_Alive() && now < hit_time + 3000)
            {
                if (s.grudge_id != who->ID() || now >= s.grudge_until)
                    P2_LOG("~ [evade] [%s]: grudge against [%s] for %u s", cName().c_str(), who->cName().c_str(), p.npc_grudge_ms / 1000);
                s.grudge_id = who->ID();
                s.grudge_until = now + p.npc_grudge_ms;
            }
        }
        if (now < s.grudge_until && m_nlc_tac.mode == eTacNone && !nlc_siege_active())
        {
            const CEntityAlive* g = smart_cast<const CEntityAlive*>(Level().Objects.net_Find(s.grudge_id));
            if (g && g->g_Alive() && !g->getDestroy())
                EnemyMemory.pin(g, s.grudge_until);
            else
                s.grudge_until = 0;
        }
    }

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
    {
        s.view_t = 0;
        s.sweep = 0.f;
        s.quiet_since = 0;
        return;
    }

    // view sweep: signed turn rate of the enemy's view (the actor's camera, an NPC's body)
    Fvector eye, view;
    if (nlc_enemy_eye(enemy, eye, view))
    {
        view.y = 0.f;
        if (view.square_magnitude() > EPS_L)
        {
            view.normalize();
            if (s.view_t && s.view_enemy == enemy->ID() && dt > EPS_L)
                s.sweep = 0.7f * s.sweep + 0.3f * (p2_signed(s.view_prev, view) / dt);
            else
                s.sweep = 0.f;
            s.view_prev = view;
            s.view_t = now;
            s.view_enemy = enemy->ID();
        }
    }

    // its weapon fires
    CWeapon* weapon = p2_weapon(enemy);
    if (weapon && weapon->GetState() == CHUDState::eFire)
    {
        s.fire_last = now;
        s.fire_interval = weapon->nlc_shot_interval();
    }

    // quiet: slow, not firing, not reloading (noise masking: it creeps then, and runs under the enemy's own noise)
    const bool reload = weapon && weapon->GetState() == CHUDState::eReload;
    const bool loud = s.enemy_speed > p.mask_quiet_speed || now < s.fire_last + 600 || reload;
    if (loud)
        s.quiet_since = 0;
    else if (!s.quiet_since)
        s.quiet_since = now;
}

bool CAI_Bloodsucker::nlc_enemy_stalker(const CEntityAlive* enemy) const { return enemy && smart_cast<const CAI_Stalker*>(enemy) != nullptr; }
bool CAI_Bloodsucker::nlc_dodge_target_npc(const CEntityAlive* enemy) const { return m_nlc_p2.dodge_npc && enemy && smart_cast<const CAI_Stalker*>(enemy) != nullptr; }

// charges for a burst (budget on): how many steps it may take now; 0 = none
int CAI_Bloodsucker::nlc_dodge_burst_size(int steps) const
{
    if (m_nlc_p2.dodge_budget <= 0)
        return steps;
    return std::min(steps, int(m_nlc_s2.charges + 0.001f));
}

// +1 / -1: the step side (in the sense of rotate_y around the enemy) that takes it off the crosshair; 0 = no opinion
int CAI_Bloodsucker::nlc_dodge_aim_side(const CEntityAlive* enemy, const Fvector& r, const char*& why) const
{
    const SNlcBs2Params& p = m_nlc_p2;
    const SNlcBs2State& s = m_nlc_s2;
    if (!p.dodge_aim_side || !enemy || !s.view_t || s.view_enemy != enemy->ID())
        return 0;
    // the sign a +1 step turns the bearing from the enemy by
    const float c = p2_signed(r, p2_rotate_y(r, 0.3f)) >= 0.f ? 1.f : -1.f;
    if (_abs(s.sweep) > p.dodge_sweep_rate)
    {
        why = ", against the sweep";
        return (s.sweep > 0.f ? -1.f : 1.f) * c > 0.f ? 1 : -1;
    }
    const float off = p2_signed(r, s.view_prev); // where the view points, relative to the line to the monster
    if (_abs(off) > p.dodge_aim_offset)
    {
        why = ", off the crosshair";
        return (off > 0.f ? -1.f : 1.f) * c > 0.f ? 1 : -1;
    }
    return 0;
}

// the enemy's weapon fired a moment ago while pointed this way (a missed hip shot too)
bool CAI_Bloodsucker::nlc_dodge_fire_trigger(u8 threat) const
{
    return m_nlc_p2.dodge_on_fire && (threat & eThreatAimed) && Device.dwTimeGlobal < m_nlc_s2.fire_last + 250;
}

// a slow gun (pump, bolt) between its shots: an opening like a reload
bool CAI_Bloodsucker::nlc_dodge_cycle_opening() const
{
    const SNlcBs2State& s = m_nlc_s2;
    if (!m_nlc_p2.dodge_on_fire || !s.fire_last || s.fire_interval < 0.4f)
        return false;
    const u32 now = Device.dwTimeGlobal;
    return now > s.fire_last + 120 && now < s.fire_last + u32(std::min(s.fire_interval, 1.2f) * 1000.f);
}

// the enemy backs straight off out of swing range: a roll for the lunge (the long double strike along a line),
// sidestepping is the answer. Called from the dodge update with the filtered speeds.
void CAI_Bloodsucker::nlc_backoff_update(const CEntityAlive* enemy, float dist, bool in_melee, bool busy)
{
    const SNlcBs2Params& p = m_nlc_p2;
    SNlcBs2State& s = m_nlc_s2;
    const u32 now = Device.dwTimeGlobal;
    if (p.backoff_chance <= 0.f || !enemy || !smart_cast<const CActor*>(enemy) || !in_melee || busy || m_nlc_grab_intent || nlc_siege_active())
    {
        s.backoff_since = 0;
        return;
    }
    CControlRunAttack* ra = com_man().nlc_run_attack();
    if (!ra || now < s.backoff_cd || now < s.backoff_until)
    {
        s.backoff_since = 0;
        return;
    }
    const bool backing = s.away_v > p.backoff_speed && _abs(m_nlc_enemy_lat_v) < s.away_v && dist >= p.backoff_min_dist && dist <= ra->nlc_dist_max();
    if (!backing)
    {
        s.backoff_since = 0;
        return;
    }
    if (!s.backoff_since)
        s.backoff_since = now;
    if (now < s.backoff_since + p.backoff_ms)
        return;

    s.backoff_since = 0;
    s.backoff_cd = now + p.backoff_cd;
    const bool go = ::Random.randF() < p.backoff_chance;
    P2_LOG("~ [evade] [%s]: back-off: the enemy backs away at %.1f m/s (sideways %.1f), %.1f m: roll %.2f -> %s", cName().c_str(), s.away_v, m_nlc_enemy_lat_v, dist,
        p.backoff_chance, go ? "lunge" : "no");
    if (!go)
        return;
    s.backoff_until = now + p.backoff_window_ms;
    ra->nlc_force(s.backoff_until, p.backoff_min_dist);
    // the burst gives way to the lunge
    m_nlc_dodge_left = 0;
}

//////////////////////////////////////////////////////////////////////////
// flank: glance tolerance, sweep freeze, "being tracked"
//////////////////////////////////////////////////////////////////////////

// 0 = go on with the arc, 1 = watched for real (straight in), 2 = freeze (stand still, cloaked)
int CAI_Bloodsucker::nlc_flank_watch(const CEntityAlive* enemy, float dist)
{
    const SNlcBs2Params& p = m_nlc_p2;
    SNlcBs2State& s = m_nlc_s2;
    const u32 now = Device.dwTimeGlobal;
    const bool watched = (nlc_threat() & eThreatWatched) != 0;
    const bool cloaked = get_visibility_state() == no_visibility;

    if (watched)
    {
        if (!s.watch_since)
        {
            s.watch_since = now;
            // a look that finds it while cloaked and not close: the enemy reads its approach (sound, shimmer)
            if (p.flank_tracked_n && cloaked && dist > 5.f)
            {
                s.looks[s.looks_i % 4] = now;
                ++s.looks_i;
                u32 n = 0;
                for (u32 t : s.looks)
                    if (t && now < t + p.flank_tracked_window)
                        ++n;
                if (n >= p.flank_tracked_n)
                {
                    for (u32& t : s.looks)
                        t = 0;
                    m_nlc_flank_side = m_nlc_flank_side > 0 ? -1 : 1;
                    m_nlc_flank_side_until = now + p.flank_tracked_hold;
                    s.freeze_until = now + p.flank_freeze_ms;
                    s.freeze_block = s.freeze_until + 2000;
                    s.side_forced_until = m_nlc_flank_side_until;
                    P2_LOG("~ [evade] [%s]: flank: tracked (%u looks in %u s), freezes %u ms, then the other way round", cName().c_str(), n, p.flank_tracked_window / 1000,
                        p.flank_freeze_ms);
                }
            }
        }
    }
    else
        s.watch_since = 0;

    if (now < s.freeze_until && cloaked)
        return 2;

    if (watched)
    {
        if (p.flank_glance_ms && now < s.watch_since + p.flank_glance_ms)
            return 0; // a glance: the arc goes on
        return 1;
    }

    // the view sweeps toward it and gets here soon: stand still (a standing cloaked bloodsucker makes no sound)
    // (polish pass 5: not for a stalker beyond its sight limit for the cloak: its body turning froze the arc)
    const float limit = nlc_watch_limit(enemy);
    if (p.flank_sweep_s > 0.f && cloaked && dist > 6.f && now >= s.freeze_block && s.view_t && s.view_enemy == enemy->ID() && !(limit > 0.f && dist > limit))
    {
        Fvector eye, view;
        if (nlc_enemy_eye(enemy, eye, view))
        {
            Fvector c;
            Center(c);
            const Fvector to = p2_flat_dir(eye, c);
            const float off = p2_signed(s.view_prev, to); // from the view to the monster
            const float rate = s.sweep * (off > 0.f ? 1.f : -1.f); // > 0: closing on it
            if (rate > 0.3f && _abs(off) - rate * p.flank_sweep_s < nlc_watch_cone() && _abs(off) > nlc_watch_cone())
            {
                s.freeze_until = now + p.flank_freeze_ms;
                s.freeze_block = s.freeze_until + 2000;
                P2_LOG("~ [evade] [%s]: flank: the view sweeps this way (%.0f deg off, %.0f deg/s), freezes %u ms", cName().c_str(), rad2deg(_abs(off)), rad2deg(rate),
                    p.flank_freeze_ms);
                return 2;
            }
        }
    }
    return 0;
}

// polish pass 5: the watched approach against the player. Straight in was the only answer to being watched (an open
// field: always). Now: oblique legs (a point watched_oblique_angle around the player at watched_oblique_close_k of
// the distance, about 40 degrees off the direct line), the side switching on a timer or a hit; and inside
// watched_orbit_dist, cloaked, it first circles for up to watched_orbit_ms waiting for a reload or a slow gun's cycle.
bool CAI_Bloodsucker::nlc_watched_approach(const CEntityAlive* enemy, const Fvector& r, float dist, Fvector& position, u32& vertex)
{
    const SNlcBs2Params& p = m_nlc_p2;
    SNlcBs2State& s = m_nlc_s2;
    if (p.obl_angle <= 0.f)
        return false;
    const u32 now = Device.dwTimeGlobal;
    const bool fresh = now > s.obl_last + 500;
    const u32 hit = u32(HitMemory.get_last_hit_time());
    const bool shot = hit > s.obl_hit_seen;
    s.obl_hit_seen = std::max(s.obl_hit_seen, hit);

    // orbit
    bool orbit = false;
    // (pass 6: 3 m of slack on the outer edge once it circles; no "opening" exit in the first second)
    const bool in_ring = p.orbit_ms && dist >= p.orbit_dist.x && dist <= p.orbit_dist.y + (s.orbit_until ? 3.f : 0.f) && get_visibility_state() == no_visibility;
    if (in_ring && !s.orbit_until && now >= s.orbit_cd)
    {
        s.orbit_until = now + p.orbit_ms;
        P2_LOG("~ [evade] [%s]: watched at %.1f m: circles for up to %u ms, waiting for an opening", cName().c_str(), dist, p.orbit_ms);
    }
    if (s.orbit_until)
    {
        CWeapon* w = p2_weapon(enemy);
        const bool opening = now + p.orbit_ms >= s.orbit_until + 1000 && ((w && w->GetState() == CHUDState::eReload) || nlc_dodge_cycle_opening());
        if (!in_ring || opening || now >= s.orbit_until)
        {
            P2_LOG("~ [evade] [%s]: circling over (%s) at %.1f m, goes in", cName().c_str(), opening ? "opening" : (in_ring ? "time" : "left the ring or revealed"), dist);
            s.orbit_until = 0;
            s.orbit_cd = now + p.orbit_cd;
        }
        else
            orbit = true;
    }

    if (fresh)
    {
        s.obl_side = ::Random.randI(2) ? 1 : -1;
        s.obl_switch = now + u32(::Random.randF(p.obl_leg_ms.x, p.obl_leg_ms.y));
        P2_LOG("~ [evade] [%s]: watched approach: oblique legs from %.1f m, %s first", cName().c_str(), dist, s.obl_side > 0 ? "right" : "left");
    }
    else if ((!orbit && now >= s.obl_switch) || (shot && now >= s.obl_shot_block))
    {
        s.obl_side = -s.obl_side;
        s.obl_switch = now + u32(::Random.randF(p.obl_leg_ms.x, p.obl_leg_ms.y));
        s.obl_shot_block = now + 500;
        P2_LOG("~ [evade] [%s]: watched approach: %s leg at %.1f m%s", cName().c_str(), s.obl_side > 0 ? "right" : "left", dist, shot ? " (hit)" : "");
    }

    const float ang = orbit ? deg2rad(30.f) : p.obl_angle;
    const float radius = orbit ? dist : std::max(dist * p.obl_close_k, 3.f);
    const Fvector ep = enemy->Position();
    for (int k = 0; k < 2; ++k)
    {
        const int side = k == 0 ? s.obl_side : -s.obl_side;
        const Fvector w = p2_rotate_y(r, float(side) * ang);
        Fvector pt;
        pt.set(ep.x + w.x * radius, Position().y, ep.z + w.z * radius);
        if (!ai().level_graph().valid_vertex_position(pt))
            continue;
        const u32 n = ai().level_graph().vertex_id(pt);
        if (!ai().level_graph().valid_vertex_id(n))
            continue;
        pt = ai().level_graph().vertex_position(n);
        if (_abs(pt.y - Position().y) > 3.f || !movement().restrictions().accessible(pt) || !Home->at_home(pt))
            continue;
        if (side != s.obl_side)
        {
            s.obl_side = side; // that side is blocked: the other one, for a full leg
            s.obl_switch = now + u32(::Random.randF(p.obl_leg_ms.x, p.obl_leg_ms.y));
        }
        s.obl_last = now;
        position = pt;
        vertex = n;
        return true;
    }
    if (!fresh && now > s.still_log + 2000)
    {
        s.still_log = now;
        P2_LOG("~ [evade] [%s]: watched approach: no leg point on either side at %.1f m, straight in", cName().c_str(), dist);
    }
    return false;
}

bool CAI_Bloodsucker::nlc_run_override_exact() const { return m_nlc_dodge_active || Device.dwTimeGlobal < m_nlc_s2.obl_last + 300; }

// noise masking: cloaked and unwatched near a quiet enemy it creeps; under the enemy's own noise it runs
bool CAI_Bloodsucker::nlc_mask_creep(const CEntityAlive* enemy, float dist)
{
    const SNlcBs2Params& p = m_nlc_p2;
    SNlcBs2State& s = m_nlc_s2;
    const u32 now = Device.dwTimeGlobal;
    const bool creep = p.mask_dist > 0.f && enemy && smart_cast<const CActor*>(enemy) && get_visibility_state() == no_visibility && dist < p.mask_dist && dist > 5.f &&
        !(nlc_threat() & eThreatWatched) && s.quiet_since && now >= s.quiet_since + p.mask_quiet_ms;
    if (creep != s.creeping)
    {
        s.creeping = creep;
        P2_LOG("~ [evade] [%s]: noise mask: %s at %.1f m (enemy speed %.1f m/s)", cName().c_str(), creep ? "the enemy is quiet, creeps" : "runs", dist, s.enemy_speed);
    }
    if (creep)
    {
        nlc_set_dodge(p.mask_speed_k, 300);
        s.creep_until = now + 300;
    }
    return creep;
}

float CAI_Bloodsucker::nlc_mask_step_k() const { return Device.dwTimeGlobal < m_nlc_s2.creep_until ? m_nlc_p2.mask_step_k : 1.f; }

//////////////////////////////////////////////////////////////////////////
// group: armed humans around the target (the actor is a member like any other)
//////////////////////////////////////////////////////////////////////////

void CAI_Bloodsucker::nlc_grp_update(const CEntityAlive* enemy)
{
    const SNlcBs2Params& p = m_nlc_p2;
    SNlcBs2State& s = m_nlc_s2;
    const u32 now = Device.dwTimeGlobal;
    if (!p.grp || !enemy)
    {
        s.grp_n = 0;
        s.melee_ms = 0;
        return;
    }

    // time spent in the standing melee (the break rule)
    {
        const u32 dt = s.grp_t ? std::min(now - s.grp_t, 500u) : 0u;
        s.grp_t = now;
        const u32 st = StateMan ? u32(StateMan->get_state_type()) : u32(eStateUnknown);
        if (st == eStateAttack_Melee)
            s.melee_ms += dt;
    }

    if (now < s.grp_next)
        return;
    s.grp_next = now + 500;

    const u8 was = s.grp_n;
    const SNlcBs2State before = s;
    s.grp_n = 0;
    Fvector sum{};
    u32 others = 0;
    for (const auto& it : EnemyMemory.get_memory())
    {
        const CEntityAlive* e = it.first;
        if (!e || !e->g_Alive() || e->getDestroy() || !p2_human(e))
            continue;
        const bool target = e == enemy;
        if (!target)
        {
            if (!nlc_enemy_armed(e))
                continue;
            const TTime hit = HitMemory.get_last_hit_time(e);
            const bool hit_recent = hit && now < hit + p.grp_hit_ms;
            if (!hit_recent && e->Position().distance_to(enemy->Position()) > p.grp_radius)
                continue;
        }
        if (s.grp_n < 6)
            s.grp_ids[s.grp_n++] = e->ID();
        if (!target)
        {
            sum.add(e->Position());
            ++others;
        }
    }
    s.grp_have_others = others > 0;
    if (others)
        s.grp_others.set(sum.x / float(others), sum.y / float(others), sum.z / float(others));
    // a group stays a group for grp_latch_ms after its count drops (members flickered in and out of the enemy memory)
    if (s.grp_n >= 2)
        s.grp_latch = now + p.grp_latch_ms;
    else if (was >= 2 && now < s.grp_latch)
    {
        s.grp_n = before.grp_n;
        for (u8 i = 0; i < 6; ++i)
            s.grp_ids[i] = before.grp_ids[i];
        s.grp_others = before.grp_others;
        s.grp_have_others = before.grp_have_others;
    }
    if ((was >= 2) != (s.grp_n >= 2))
        P2_LOG("~ [evade] [%s]: group: %s (%u armed humans with [%s])", cName().c_str(), s.grp_n >= 2 ? "outnumbered, group tactics" : "solo target", u32(s.grp_n),
            enemy->cName().c_str());
}

bool CAI_Bloodsucker::nlc_grp_active() const { return m_nlc_p2.grp && m_nlc_s2.grp_n >= 2; }

// the bearing (from the target) that puts the target between it and the other guns
bool CAI_Bloodsucker::nlc_grp_shield_dir(const CEntityAlive* enemy, Fvector& dir) const
{
    const SNlcBs2State& s = m_nlc_s2;
    if (!m_nlc_p2.grp_shield || !nlc_grp_active() || !s.grp_have_others || !enemy)
        return false;
    dir = p2_flat_dir(s.grp_others, enemy->Position());
    return dir.square_magnitude() > EPS_L;
}

// target weights in a group: the isolated, the one looking away, the one reloading, the wounded
float CAI_Bloodsucker::nlc_enemy_danger_k(const CEntityAlive* e)
{
    float k = inherited::nlc_enemy_danger_k(e);
    const SNlcBs2Params& p = m_nlc_p2;
    const SNlcBs2State& s = m_nlc_s2;
    if (!m_nlc_on || !nlc_grp_active() || !e || !p2_human(e))
        return k;
    bool member = false;
    float nearest = flt_max;
    for (u8 i = 0; i < s.grp_n; ++i)
    {
        if (s.grp_ids[i] == e->ID())
        {
            member = true;
            continue;
        }
        if (const CObject* o = Level().Objects.net_Find(s.grp_ids[i]))
            nearest = std::min(nearest, o->Position().distance_to(e->Position()));
    }
    if (!member)
        return k;
    if (nearest > p.grp_isolated_dist)
        k *= p.grp_w_isolated;
    Fvector to_me = p2_flat_dir(e->Position(), Position());
    Fvector eye, view;
    if (nlc_enemy_eye(e, eye, view))
    {
        view.y = 0.f;
        if (view.square_magnitude() > EPS_L && to_me.square_magnitude() > EPS_L && angle_between_vectors(view, to_me) > PI_DIV_2)
            k *= p.grp_w_unaware;
    }
    if (CWeapon* w = p2_weapon(e))
        if (w->GetState() == CHUDState::eReload)
            k *= p.grp_w_reload;
    if (e->conditions().GetHealth() < 0.5f * e->conditions().GetMaxHealth())
        k *= p.grp_w_wounded;
    return k;
}

// a group break is due: it has stood in melee long enough among several guns
bool CAI_Bloodsucker::nlc_grp_break_due() const
{
    const SNlcBs2Params& p = m_nlc_p2;
    const SNlcBs2State& s = m_nlc_s2;
    return nlc_grp_active() && p.grp_melee_ms > 0 && s.melee_ms >= p.grp_melee_ms && Device.dwTimeGlobal >= s.break_cd;
}

void CAI_Bloodsucker::nlc_grp_break_begin()
{
    SNlcBs2State& s = m_nlc_s2;
    const Fvector2& b = m_nlc_p2.grp_break_ms;
    s.breaking = true;
    s.break_until = Device.dwTimeGlobal + u32(::Random.randF(b.x, b.y));
    s.melee_ms = 0;
    P2_LOG("~ [evade] [%s]: group: breaks off for %u ms (%u in the group)", cName().c_str(), s.break_until - Device.dwTimeGlobal, u32(s.grp_n));
}

bool CAI_Bloodsucker::nlc_grp_break_over()
{
    SNlcBs2State& s = m_nlc_s2;
    if (!s.breaking || Device.dwTimeGlobal < s.break_until)
        return false;
    nlc_grp_break_end();
    return true;
}

void CAI_Bloodsucker::nlc_grp_break_end()
{
    SNlcBs2State& s = m_nlc_s2;
    if (!s.breaking)
        return;
    s.breaking = false;
    s.break_cd = Device.dwTimeGlobal + 4000;
    s.melee_ms = 0;
}

// a strike into a stalker's back hurts more (the grab stays actor-only: no NPC animations)
float CAI_Bloodsucker::nlc_npc_backhit_k(const CEntity* entity) const
{
    const SNlcBs2Params& p = m_nlc_p2;
    if (!m_nlc_on || p.npc_backhit_k <= 1.f || !entity || !smart_cast<const CAI_Stalker*>(entity))
        return 1.f;
    Fvector face = entity->Direction();
    face.y = 0.f;
    const Fvector to_me = p2_flat_dir(entity->Position(), Position());
    if (face.square_magnitude() < EPS_L || to_me.square_magnitude() < EPS_L || angle_between_vectors(face, to_me) < p.npc_backhit_angle)
        return 1.f;
    P2_LOG("~ [evade] [%s]: back hit on [%s]: damage x%.2f", cName().c_str(), entity->cName().c_str(), p.npc_backhit_k);
    return p.npc_backhit_k;
}
