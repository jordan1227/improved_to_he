////////////////////////////////////////////////////////////////////////////
// NLC: bloodsucker behaviour set (docs/DESIGN_monster_movement_under_fire.md 6, 8-10, 20)
//
// Cloak rules (ambush hold, lunge radius, melee reveal), faint steps, the edge sneak, the vampire grab as a
// true ambush, the pounce, and the tactic substate: hit and run (recover), stalk for openings, feint, bait
// and flank pair. Every feature has its own key in the monster section (m_bloodsucker.ltx) and defaults to
// the stock behaviour; evade_enabled = false switches all of it off.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bloodsucker.h"
#include "../ai_monster_squad.h"
#include "../ai_monster_squad_manager.h"
#include "../control_animation_base.h"
#include "../control_direction_base.h"
#include "../control_manager.h"
#include "../control_path_builder_base.h"
#include "../control_run_attack.h"
#include "../controlled_entity.h"
#include "../monster_home.h"
#include "../monster_velocity_space.h"
#include "../state_manager.h"
#include "../state_defs.h"
#include "../../../actor.h"
#include "../../../ActorEffector.h"
#include "../../../ai_space.h"
#include "../../../ai_object_location.h"
#include "../../../entitycondition.h"
#include "../../../HudItem.h"
#include "../../../inventory.h"
#include "../../../inventory_item.h"
#include "../../../level.h"
#include "../../../movement_manager.h"
#include "../../../nlc_stealth.h"
#include "../../../restricted_object.h"
#include "../../../sound_player.h"
#include "level_graph.h"

#define TAC_LOG(...) \
    do \
    { \
        if (nlc_evade_debug()) \
            Msg(__VA_ARGS__); \
    } while (0)

namespace
{
// at most this many point searches per frame over all bloodsuckers (as the siege's pick_budget)
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

// rotation around Y: x' = x cos - z sin, z' = x sin + z cos (as monster_state_attack_on_run_inline.h)
Fvector rotate_y(const Fvector& v, float a)
{
    const float cs = _cos(a), sn = _sin(a);
    Fvector r;
    r.set(v.x * cs - v.z * sn, 0.f, v.x * sn + v.z * cs);
    return r;
}

// unit xz direction from `from` to `to`; zero when they coincide
Fvector flat_dir(const Fvector& from, const Fvector& to)
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

LPCSTR mode_name(u8 mode)
{
    switch (mode)
    {
    case 1: return "recover";
    case 2: return "stalk";
    case 3: return "feint";
    case 4: return "bait";
    default: return "none";
    }
}
} // namespace

//////////////////////////////////////////////////////////////////////////
// config
//////////////////////////////////////////////////////////////////////////

void CAI_Bloodsucker::nlc_bs_load(LPCSTR section)
{
    SNlcBloodsuckerParams& b = m_nlc_b;
    b = SNlcBloodsuckerParams{};
    m_nlc_on = false;
    m_nlc_vampire_ambush = false;
    m_nlc_tac_any = false;
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

    b.run_attack_cfg = B("run_attack_cfg", false);
    b.run_attack_decloak = B("run_attack_decloak", false);
    b.cloak_lunge_radius = std::max(F("cloak_lunge_radius", 0.f), 0.f);
    b.cloak_step_volume = std::clamp(F("cloak_step_volume", 1.f), 0.f, 1.f);
    b.cloak_melee_reveal = B("cloak_melee_reveal", false);
    b.amb_cloak_hold = B("amb_cloak_hold", false);
    b.sneak_near_dist = std::max(F("sneak_near_dist", 0.f), 0.f);
    b.sneak_edge_cone = deg2rad(std::clamp(F("sneak_edge_cone", 75.f), 5.f, 170.f));
    b.amb_pounce = B("amb_pounce", false);
    b.pounce_min = std::max(F("jump_min_distance", 4.f), 1.f);
    b.pounce_max = std::max(F("jump_max_distance", 7.f), b.pounce_min);
    b.lunge_reveal = u8(std::clamp(int(F("lunge_reveal", 2.f)), 1, 2));
    b.lunge_reveal_ms = MS("lunge_reveal_ms", 0);
    b.vampire_intent_dist = std::max(F("vampire_intent_dist", 8.f), 0.f);
    b.strike_reveal_margin = std::max(F("strike_reveal_margin", 0.f), 0.f);
    b.strike_reveal_ms = MS("strike_reveal_ms", 300);
    b.cloak_cooldown_radius = std::max(F("cloak_cooldown_radius", 5.f), 0.f);
    b.pounce_chance = std::clamp(F("pounce_chance", 1.f), 0.f, 1.f);
    b.pounce_delay = MS("jump_delay", 3000);
    b.pounce_max_h = std::max(F("pounce_perch_max_h", F("jump_max_height", 2.5f)), 0.f);
    b.pounce_flat = B("pounce_flat", true);
    b.pounce_perch_chance = std::clamp(F("pounce_perch_chance", 1.f), 0.f, 1.f);
    b.pounce_perch_wait_ms = MS("pounce_perch_wait_ms", 4000);
    b.pounce_bounce_grace_ms = MS("pounce_bounce_grace_ms", 0);
    b.pounce_perch_block_ms = std::max(MS("pounce_perch_block_ms", 20000), 2000u); // 0 would give up and retake the spot every cycle
    b.strike_reveal_max_ms = MS("strike_reveal_max_ms", 0);
    b.strike_reveal_block_ms = MS("strike_reveal_block_ms", 3000);
    b.tac_stalk_flank = B("tac_stalk_flank", false);
    b.tac_stalk_flank_angle = deg2rad(std::clamp(F("tac_stalk_flank_angle", 35.f), 5.f, 90.f));
    b.charge_flank = B("charge_flank", false);
    b.charge_flank_angle = deg2rad(std::clamp(F("charge_flank_angle", 100.f), 45.f, 180.f));
    b.charge_flank_step = deg2rad(std::clamp(F("charge_flank_step", 40.f), 10.f, 90.f));
    b.charge_flank_dist = V2("charge_flank_dist", Fvector2().set(8.f, 30.f));
    b.tac_recover_min_time = MS("tac_recover_min_time", 0);
    b.tac_recover_break_health = std::clamp(F("tac_recover_break_health", 0.f), 0.f, 1.f);
    b.tac_feint_vis = u8(std::clamp(int(F("tac_feint_vis", 2.f)), 1, 2));
    b.tac_bait_vis = u8(std::clamp(int(F("tac_bait_vis", 2.f)), 1, 2));
    b.tac_feint_cloak_ms = MS("tac_feint_cloak_ms", 2000);

    b.vampire_ambush = B("vampire_ambush", false);
    b.vampire_ambush_chance = std::clamp(F("vampire_ambush_chance", 1.f), 0.f, 1.f);
    b.vampire_unseen_ms = MS("vampire_unseen_ms", 3000);
    b.vampire_behind_angle = deg2rad(std::clamp(F("vampire_behind_angle", 120.f), 30.f, 179.f));
    b.vampire_struggle_look = std::max(F("vampire_struggle_look", 8.f), 0.f);
    b.vampire_struggle_need = std::max(F("vampire_struggle_need", 1500.f), 0.f);
    b.vampire_struggle_wound_k = std::clamp(F("vampire_struggle_wound_k", 0.6f), 0.f, 1.f);
    b.vampire_backhit_chance = std::clamp(F("vampire_backhit_chance", 0.f), 0.f, 1.f);
    b.vampire_backhit_damage_k = std::clamp(F("vampire_backhit_damage_k", 0.2f), 0.f, 1.f);
    b.vampire_backhit_window_ms = MS("vampire_backhit_window_ms", 1500);
    b.vampire_break_dist = std::max(F("vampire_break_dist", 0.f), 0.f);

    b.tac_recover_enabled = B("tac_recover_enabled", false);
    b.tac_recover_health = std::clamp(F("tac_recover_health", 0.35f), 0.f, 1.f);
    b.tac_recover_until = std::clamp(F("tac_recover_until", 0.8f), b.tac_recover_health, 1.f);
    b.tac_recover_min_dist = std::max(F("tac_recover_min_dist", 4.f), 0.f);
    b.tac_recover_dist = V2("tac_recover_dist", Fvector2().set(18.f, 30.f));
    b.tac_recover_max_time = MS("tac_recover_max_time", 25000);
    b.tac_recover_cooldown = MS("tac_recover_cooldown", 45000);
    b.tac_return_angle = deg2rad(std::clamp(F("tac_return_angle", 90.f), 0.f, 180.f));

    b.tac_stalk_enabled = B("tac_stalk_enabled", false);
    b.tac_stalk_dist = V2("tac_stalk_dist", Fvector2().set(15.f, 25.f));
    b.tac_stalk_max_time = MS("tac_stalk_max_time", 40000);
    b.tac_open_reload = B("tac_open_reload", false);
    b.tac_open_switch = B("tac_open_switch", false);
    b.tac_open_away_angle = deg2rad(std::clamp(F("tac_open_away_angle", 100.f), 20.f, 180.f));
    b.tac_open_away_ms = MS("tac_open_away_ms", 1500);

    b.tac_feint_enabled = B("tac_feint_enabled", false);
    b.tac_feint_dist = V2("tac_feint_dist", Fvector2().set(8.f, 14.f));
    b.tac_feint_chance = std::clamp(F("tac_feint_chance", 0.4f), 0.f, 1.f);
    b.tac_feint_cooldown = MS("tac_feint_cooldown", 30000);
    b.tac_feint_show_ms = MS("tac_feint_show_ms", 1200);
    b.tac_feint_flank_angle = deg2rad(std::clamp(F("tac_feint_flank_angle", 100.f), 0.f, 180.f));

    b.tac_pair_enabled = B("tac_pair_enabled", false);
    b.tac_bait_dist = V2("tac_bait_dist", Fvector2().set(10.f, 14.f));
    b.tac_bait_max_time = MS("tac_bait_max_time", 15000);

    b.tac_night_brightness = std::max(F("tac_night_brightness", 0.15f), 0.f);
    b.tac_night_dist_k = std::max(F("tac_night_dist_k", 1.3f), 0.5f);
    b.tac_night_time_k = std::max(F("tac_night_time_k", 1.5f), 0.5f);
    b.tac_night_chance_add = std::clamp(F("tac_night_chance_add", 0.2f), 0.f, 1.f);

    m_nlc_tac_any = b.tac_recover_enabled || b.tac_stalk_enabled || b.tac_feint_enabled || b.tac_pair_enabled;
    m_nlc_vampire_ambush = b.vampire_ambush && m_vampire_enable;
    m_nlc_on = m_nlc_tac_any || m_nlc_vampire_ambush || b.run_attack_decloak || b.cloak_lunge_radius > 0.f || b.cloak_step_volume < 1.f || b.cloak_melee_reveal || b.amb_cloak_hold ||
        b.sneak_near_dist > 0.f || b.amb_pounce || b.lunge_reveal_ms > 0 || b.strike_reveal_margin > 0.f || b.charge_flank;

    // the run attack and jump abilities are added after control().load(), so their keys were never read for the bloodsucker
    if (b.run_attack_cfg && !pSettings->line_exist(section, "is_friendly"))
    {
        if (pSettings->line_exist(section, "Run_Attack_Dist") && pSettings->line_exist(section, "Run_Attack_Delay"))
            com_man().nlc_load_ability(ControlCom::eControlRunAttack, section);
        else
            Msg("! [evade] [%s]: run_attack_cfg needs Run_Attack_Dist and Run_Attack_Delay, keys not loaded", section);
    }
    if (b.amb_pounce)
        com_man().nlc_load_ability(ControlCom::eControlJump, section);
}

//////////////////////////////////////////////////////////////////////////
// cloak
//////////////////////////////////////////////////////////////////////////

// a forced switch: bypass_delay skips the minimum delay once (the stock delay applies again afterwards)
void CAI_Bloodsucker::nlc_set_cloak(visibility_t state, bool bypass_delay)
{
    if (bypass_delay)
        m_visibility_state_last_changed_time = 0;
    set_visibility_state(state);
}

// does the lunge exist, is it off cooldown, and is the enemy seen beyond the lunge minimum
bool CAI_Bloodsucker::nlc_lunge_ready()
{
    if (!m_nlc_b.run_attack_decloak)
        return false;
    CControlRunAttack* ra = com_man().nlc_run_attack();
    if (!ra || ra->is_active() || Device.dwTimeGlobal < ra->time_next_attack())
        return false;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !EnemyMan.see_enemy_now())
        return false;
    return enemy->Position().distance_to(Position()) > ra->nlc_dist_min();
}

// the state a lunge or pounce reveals; the pounce never above x-ray (partial and no visibility share the predator visual,
// so no model swap happens while the jump plays)
CAI_Bloodsucker::visibility_t CAI_Bloodsucker::nlc_reveal_state(bool pounce) const
{
    if (pounce || m_nlc_b.lunge_reveal == 1)
        return partial_visibility;
    return full_visibility;
}

bool CAI_Bloodsucker::nlc_pounce_ready() const
{
    return m_nlc_b.amb_pounce && Device.dwTimeGlobal >= m_nlc_pounce_ready_at && !const_cast<CAI_Bloodsucker*>(this)->com_man().is_jumping();
}

void CAI_Bloodsucker::nlc_pounce_cooldown(u32 ms)
{
    m_nlc_pounce_ready_at = Device.dwTimeGlobal + ms;
    m_nlc_roll_flat = SNlcPounceRoll{};
    m_nlc_roll_perch = SNlcPounceRoll{};
}

// ready, and this cycle's roll allows a pounce at this kind of target
bool CAI_Bloodsucker::nlc_pounce_wanted(bool perch) const
{
    const SNlcPounceRoll& r = perch ? m_nlc_roll_perch : m_nlc_roll_flat;
    return r.rolled && r.go && nlc_pounce_ready();
}

// enemy feet above its ground point; false off the ai-map
bool CAI_Bloodsucker::nlc_enemy_perch_h(const CEntityAlive* enemy, float& h) const
{
    h = 0.f;
    if (!enemy)
        return false;
    const u32 ev = enemy->ai_location().level_vertex_id();
    if (!ai().level_graph().valid_vertex_id(ev))
        return false;
    h = enemy->Position().y - ai().level_graph().vertex_position(ev).y;
    return true;
}

// a bloodsucker besieging a player on a low perch goes to pounce distance and pounces instead of hiding; not again
// for pounce_perch_block_ms after a perch strike was given up
bool CAI_Bloodsucker::nlc_siege_perch_strike(const CEntityAlive* enemy, float& strike_dist)
{
    if (!m_nlc_on || !m_nlc_b.amb_pounce || m_nlc_b.pounce_perch_chance <= 0.f || !enemy || !smart_cast<const CActor*>(enemy))
        return false;
    if (Device.dwTimeGlobal < m_nlc_perch_block_until)
        return false;
    float h;
    if (!nlc_enemy_perch_h(enemy, h) || h < 0.4f || h > m_nlc_b.pounce_max_h)
        return false;
    strike_dist = 0.5f * (m_nlc_b.pounce_min + m_nlc_b.pounce_max);
    return true;
}

// shot at the perch-strike spot: the siege leaves it (retreat), and it is not taken again for pounce_perch_block_ms
void CAI_Bloodsucker::nlc_siege_perch_shot()
{
    m_nlc_perch_block_until = Device.dwTimeGlobal + m_nlc_b.pounce_perch_block_ms;
    m_nlc_perch_since = 0;
    TAC_LOG("~ [evade] [%s]: perch strike given up (shot), hiding %.0f s", cName().c_str(), float(m_nlc_b.pounce_perch_block_ms) / 1000.f);
}

void CAI_Bloodsucker::nlc_perch_give_up(const char* why)
{
    m_nlc_perch_block_until = Device.dwTimeGlobal + m_nlc_b.pounce_perch_block_ms;
    m_nlc_perch_since = 0;
    TAC_LOG("~ [evade] [%s]: perch strike given up (%s), hiding %.0f s", cName().c_str(), why, float(m_nlc_b.pounce_perch_block_ms) / 1000.f);
    nlc_siege_perch_abandon();
}

bool CAI_Bloodsucker::nlc_strike_tell_done() const
{
    if (m_nlc_b.strike_reveal_margin <= 0.f)
        return true;
    return m_nlc_full_since && Device.dwTimeGlobal >= m_nlc_full_since + m_nlc_b.strike_reveal_ms;
}

// a strike reveals the bloodsucker at once and holds that state for lunge_reveal_ms or until the first melee swing.
// The pounce only opens the window: the jump has already started, and a model swap under it is never allowed (the
// window's state applies after the landing; the visibility is frozen while the jump plays)
void CAI_Bloodsucker::nlc_strike_reveal(const char* what, bool pounce)
{
    m_nlc_strike_reveal_since = 0;
    if (m_nlc_b.lunge_reveal_ms)
        m_nlc_reveal_until = Device.dwTimeGlobal + m_nlc_b.lunge_reveal_ms;
    if (pounce)
        return;
    const visibility_t st = nlc_reveal_state(false);
    if (get_visibility_state() != st)
    {
        nlc_set_cloak(st, true);
        TAC_LOG("~ [evade] [%s]: %s, reveal %s", cName().c_str(), what, st == partial_visibility ? "x-ray" : "full");
    }
}

// first match wins; false = the stock distance rule
bool CAI_Bloodsucker::nlc_cloak_override(visibility_t& state)
{
    if (!m_nlc_on)
        return false;
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;

    // 0. never swap the visual while a jump or a lunge plays (the swap replaces the model and restarts its animations)
    CControlRunAttack* ra = com_man().nlc_run_attack();
    if (com_man().is_jumping() || (ra && ra->is_active()))
    {
        state = m_visibility_state;
        return state != unset;
    }

    // 0b. fully visible while its grab holds the actor (a back-hit grab can start inside a strike's x-ray window; a
    // swap to x-ray would restart the animations under the grab)
    if (CControlledActor::is_controlling())
    {
        state = full_visibility;
        return true;
    }

    // 1. the first melee swing decloaks (at once) and ends the strike reveal
    if (b.cloak_melee_reveal && StateMan && StateMan->get_state_type() == eStateAttack_Melee)
    {
        if (!m_nlc_melee_logged)
        {
            m_nlc_melee_logged = true;
            m_nlc_reveal_until = 0;
            if (get_visibility_state() != full_visibility)
                m_visibility_state_last_changed_time = 0;
            TAC_LOG("~ [evade] [%s]: melee, decloak", cName().c_str());
        }
        state = full_visibility;
        return true;
    }
    m_nlc_melee_logged = false;

    // 2. a grab from behind is planned: stay cloaked
    if (m_nlc_grab_intent && !CControlledActor::is_controlling())
    {
        state = no_visibility;
        return true;
    }

    // 2b. the cloak window after a feint (vanish and charge on unseen)
    if (now < m_nlc_cloak_until)
    {
        state = no_visibility;
        return true;
    }

    // 3. after a lunge or pounce, until the first melee swing (lunge_reveal_ms)
    if (now < m_nlc_reveal_until)
    {
        state = nlc_reveal_state(false);
        return true;
    }

    // 4. a tactic wants a fixed state (feint, bait, recover, stalk)
    if (m_nlc_tac.force_vis != unset)
    {
        state = m_nlc_tac.force_vis;
        return true;
    }

    // 5. cloaked at the siege hold point even right under the enemy
    if (b.amb_cloak_hold && nlc_siege_hidden_hold())
    {
        state = no_visibility;
        return true;
    }

    // 6a. reveal before a strike: fully visible within (start distance of a ready lunge or pounce + strike_reveal_margin),
    // else within cloak_cooldown_radius; the strike waits strike_reveal_ms fully visible (the tell)
    if (b.strike_reveal_margin > 0.f)
    {
        const CEntityAlive* enemy = EnemyMan.get_enemy();
        if (!enemy)
            return false;
        float reveal = b.cloak_cooldown_radius;
        float strike_reveal = 0.f; // > 0: a ready strike reveals within this
        float h = 0.f;
        const bool perch_enemy = nlc_enemy_perch_h(enemy, h) && h >= 0.4f;
        const bool blocked = now < m_nlc_strike_reveal_block_until;
        // only while actually charging (attack run, facing the enemy within 45 degrees, seeing it) or at a perch-strike
        // spot: with "any ready strike" one was nearly always ready and the bloodsucker stayed visible within 10 m
        const bool perch = nlc_siege_perch();
        bool charging = perch;
        if (!charging && StateMan && StateMan->get_state_type() == eStateAttack_Run && EnemyMan.see_enemy_now())
        {
            Fvector fwd = Direction();
            fwd.y = 0.f;
            Fvector to;
            to.sub(enemy->Position(), Position());
            to.y = 0.f;
            charging = fwd.square_magnitude() > EPS_L && to.square_magnitude() > EPS_L && angle_between_vectors(fwd, to) < deg2rad(45.f);
        }
        if (charging && !blocked)
        {
            // no lunge at a perched enemy (it cannot reach it) nor during a siege
            if (!perch && !perch_enemy && b.run_attack_decloak && ra && !ra->is_active() && now >= ra->time_next_attack())
                strike_reveal = std::max(strike_reveal, ra->nlc_dist_max() + b.strike_reveal_margin);
            if (nlc_pounce_wanted(perch_enemy) && h <= b.pounce_max_h)
                strike_reveal = std::max(strike_reveal, b.pounce_max + b.strike_reveal_margin);
        }
        const float d = enemy->Position().distance_to(Position());
        const bool strike_shown = strike_reveal > 0.f && d <= strike_reveal && d > reveal;
        // a strike reveal that brings no strike ends after strike_reveal_max_ms (it stood revealed for seconds)
        if (strike_shown && b.strike_reveal_max_ms)
        {
            if (!m_nlc_strike_reveal_since)
                m_nlc_strike_reveal_since = now;
            else if (now >= m_nlc_strike_reveal_since + b.strike_reveal_max_ms)
            {
                m_nlc_strike_reveal_since = 0;
                m_nlc_strike_reveal_block_until = now + b.strike_reveal_block_ms;
                TAC_LOG("~ [evade] [%s]: strike reveal without a strike for %u ms, cloaking %u ms", cName().c_str(), b.strike_reveal_max_ms, b.strike_reveal_block_ms);
                strike_reveal = 0.f;
            }
        }
        else
            m_nlc_strike_reveal_since = 0;
        reveal = std::max(reveal, strike_reveal);
        if (d <= reveal)
            state = full_visibility;
        else if (perch)
            state = no_visibility; // waiting at the perch-strike spot: cloaked until the pounce is ready
        else if (d <= get_partial_visibility_radius())
            state = partial_visibility;
        else
            state = no_visibility;
        return true;
    }

    // 6. the stock distance rule with the lunge radius while the lunge is ready
    if (b.cloak_lunge_radius > 0.f)
    {
        if (const CEntityAlive* enemy = EnemyMan.get_enemy())
        {
            if (nlc_lunge_ready())
            {
                const float d = enemy->Position().distance_to(Position());
                if (d <= b.cloak_lunge_radius)
                    state = full_visibility;
                else if (d <= get_partial_visibility_radius())
                    state = partial_visibility;
                else
                    state = no_visibility;
                return true;
            }
        }
    }
    return false;
}

// faint steps while cloaked (the actor's step callback keeps the original power)
float CAI_Bloodsucker::step_volume_k() { return (m_nlc_on && state_invisible) ? m_nlc_b.cloak_step_volume : 1.f; }

// the slow sneak gait while cloaked: close to an unseen enemy, near the edge of its view
bool CAI_Bloodsucker::nlc_cloak_keeps_gait()
{
    const SNlcBloodsuckerParams& b = m_nlc_b;
    if (!m_nlc_on || b.sneak_near_dist <= 0.f)
        return false;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || enemy->Position().distance_to_xz(Position()) >= b.sneak_near_dist)
        return false;
    if (nlc_threat() & eThreatWatched)
        return false;
    Fvector eye, view;
    if (!nlc_enemy_eye(enemy, eye, view))
        return false;
    Fvector c;
    Center(c);
    return angle_between_vectors(view, Fvector().sub(c, eye)) < b.sneak_edge_cone;
}

bool CAI_Bloodsucker::nlc_tac_night()
{
    float bright = 1.f;
    nlc_stealth::sky_light(this, nullptr, &bright);
    return bright < m_nlc_b.tac_night_brightness;
}

//////////////////////////////////////////////////////////////////////////
// vampire grab as a true ambush (20.7)
//////////////////////////////////////////////////////////////////////////

// cloaked, unseen for vampire_unseen_ms, the actor neither jumping, climbing nor in a vehicle, the monster behind it,
// and one chance roll per approach (re-rolled after it was seen or after a lunge)
bool CAI_Bloodsucker::nlc_vampire_ambush_ok(const CEntityAlive* enemy) { return nlc_grab_possible(enemy, true); }

// the grab rules without the distance (the execute state checks melee range; the intent uses vampire_intent_dist);
// roll = false: everything except the chance roll
bool CAI_Bloodsucker::nlc_grab_possible(const CEntityAlive* enemy, bool roll)
{
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;
    if (!m_nlc_vampire_ambush || !state_invisible || !enemy)
        return false;
    if (CAI_Bloodsucker::m_time_last_vampire && now < CAI_Bloodsucker::m_time_last_vampire + m_vampire_min_delay)
        return false; // one grab at a time over all bloodsuckers
    // not mid-strike: a grab started during a lunge was carried past the player and ended at once (camera twitch,
    // nlc-3.589.32 log); not in the x-ray window after a strike (it just attacked openly)
    CControlRunAttack* ra = com_man().nlc_run_attack();
    if (com_man().is_jumping() || (ra && ra->is_active()) || now < m_nlc_reveal_until)
        return false;

    nlc_threat(); // refreshes last_watched
    if (now - nlc_last_watched() < b.vampire_unseen_ms)
        return false;

    CActor* actor = smart_cast<CActor*>(const_cast<CEntityAlive*>(enemy));
    if (!actor || actor->is_jump() || (actor->MovingState() & mcClimb) || actor->Holder())
        return false;

    Fvector to_me;
    to_me.sub(Position(), actor->Position());
    if (angle_between_vectors(actor->Cameras().Direction(), to_me) <= b.vampire_behind_angle)
        return false;

    SNlcTactic& t = m_nlc_tac;
    if (!roll)
        return true;
    if (!t.rolled_grab)
    {
        float chance = b.vampire_ambush_chance + (t.night ? b.tac_night_chance_add : 0.f);
        chance = std::clamp(chance, 0.f, 1.f);
        t.grab_ok = ::Random.randF() < chance;
        t.rolled_grab = true;
        TAC_LOG("~ [evade] [%s]: vampire: roll %.2f%s -> %s", cName().c_str(), chance, t.night ? " (night)" : "", t.grab_ok ? "grab" : "no grab");
    }
    return t.grab_ok;
}

// a melee, lunge or pounce hit that lands from behind (vampire_behind_angle) may turn into the grab: only
// vampire_backhit_damage_k of it is dealt now, the rest only when no grab starts within vampire_backhit_window_ms
// (hit + grab is not double damage); the global grab cooldown and the actor checks stay
bool CAI_Bloodsucker::nlc_backhit_try(const CEntity* entity, float damage, float impulse, Fvector& dir, ALife::EHitType hit_type, bool draw_hit_marks)
{
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;
    if (!m_nlc_on || !m_nlc_vampire_ambush || b.vampire_backhit_chance <= 0.f || m_nlc_backhit || !g_Alive())
        return false;
    if (!entity || entity != EnemyMan.get_enemy() || !entity->g_Alive())
        return false;
    CActor* actor = smart_cast<CActor*>(const_cast<CEntity*>(entity));
    if (!actor || CControlledActor::is_controlling() || actor->input_external_handler_installed())
        return false;
    if (CAI_Bloodsucker::m_time_last_vampire && now < CAI_Bloodsucker::m_time_last_vampire + m_vampire_min_delay)
        return false;
    if (!nlc_backhit_actor_ok(actor))
        return false;
    // a grab must be possible from here (as the execute state checks it): else the hit stays whole
    if (!MeleeChecker.can_start_melee(actor) ||
        !ai().level_graph().valid_vertex_id(ai().level_graph().check_position_in_direction(ai_location().level_vertex_id(), Position(), actor->Position())))
        return false;
    if (::Random.randF() >= b.vampire_backhit_chance)
    {
        TAC_LOG("~ [evade] [%s]: back hit: roll %.2f -> no grab", cName().c_str(), b.vampire_backhit_chance);
        return false;
    }

    const float now_dmg = damage * b.vampire_backhit_damage_k;
    if (now_dmg > 0.f)
        inherited::HitEntity(entity, now_dmg, impulse * b.vampire_backhit_damage_k, dir, hit_type, draw_hit_marks);
    m_nlc_backhit = true;
    m_nlc_backhit_in_grab = false;
    m_nlc_backhit_until = now + b.vampire_backhit_window_ms;
    m_nlc_backhit_target = entity->ID();
    m_nlc_backhit_rest = damage - now_dmg;
    m_nlc_backhit_impulse = impulse * (1.f - b.vampire_backhit_damage_k);
    XFORM().transform_dir(m_nlc_backhit_dir, dir); // world space: the monster turns before the rest may land
    m_nlc_backhit_type = hit_type;
    TAC_LOG("~ [evade] [%s]: back hit: grab (%.2f of %.2f damage now)", cName().c_str(), now_dmg, damage);
    return true;
}

// the actor can be grabbed: not jumping, climbing or in a vehicle, the monster behind it
bool CAI_Bloodsucker::nlc_backhit_actor_ok(CActor* actor)
{
    if (!actor || actor->is_jump() || (actor->MovingState() & mcClimb) || actor->Holder())
        return false;
    Fvector to_me;
    to_me.sub(Position(), actor->Position());
    return angle_between_vectors(actor->Cameras().Direction(), to_me) > m_nlc_b.vampire_behind_angle;
}

// the grab may start now: a back hit is pending, no lunge or jump plays (a grab started mid-lunge ended at once), and
// the actor is still grabbable (it may have jumped or turned around since the hit)
bool CAI_Bloodsucker::nlc_backhit_ready()
{
    if (!m_nlc_backhit || m_nlc_backhit_in_grab || Device.dwTimeGlobal >= m_nlc_backhit_until)
        return false;
    CControlRunAttack* ra = com_man().nlc_run_attack();
    if (com_man().is_jumping() || (ra && ra->is_active()))
        return false;
    return nlc_backhit_actor_ok(smart_cast<CActor*>(const_cast<CEntityAlive*>(EnemyMan.get_enemy())));
}

// the grab started: the window stops; the rest of the hit is settled when the grab ends (nlc_backhit_end)
void CAI_Bloodsucker::nlc_backhit_grab_started()
{
    if (m_nlc_backhit)
        m_nlc_backhit_in_grab = true;
}

// the grab ended: landed (its wound replaces the rest of the hit) or broke early (the rest lands now)
void CAI_Bloodsucker::nlc_backhit_end(bool landed)
{
    if (!m_nlc_backhit || !m_nlc_backhit_in_grab)
        return;
    if (landed)
    {
        m_nlc_backhit = false;
        return;
    }
    m_nlc_backhit_until = 0; // nlc_backhit_update delivers it
    m_nlc_backhit_in_grab = false;
}

// no grab within the window, or the grab broke: the rest of the hit lands after all (it did connect)
void CAI_Bloodsucker::nlc_backhit_update()
{
    if (!m_nlc_backhit || m_nlc_backhit_in_grab || Device.dwTimeGlobal < m_nlc_backhit_until)
        return;
    m_nlc_backhit = false;
    CEntity* target = smart_cast<CEntity*>(Level().Objects.net_Find(m_nlc_backhit_target));
    TAC_LOG("~ [evade] [%s]: back hit: no grab, rest of the damage %.2f", cName().c_str(), m_nlc_backhit_rest);
    if (target && !target->getDestroy() && target->g_Alive() && target == EnemyMan.get_enemy() && m_nlc_backhit_rest > 0.f)
    {
        // HitEntity takes a monster-local direction
        Fmatrix inv;
        inv.invert(XFORM());
        Fvector local;
        inv.transform_dir(local, m_nlc_backhit_dir);
        inherited::HitEntity(target, m_nlc_backhit_rest, m_nlc_backhit_impulse, local, m_nlc_backhit_type, true);
    }
}

//////////////////////////////////////////////////////////////////////////
// flank charge: while unwatched, the charge curves toward the player's flank or back
//////////////////////////////////////////////////////////////////////////

bool CAI_Bloodsucker::nlc_run_target_override(Fvector& position, u32& vertex)
{
    const SNlcBloodsuckerParams& b = m_nlc_b;
    if (!m_nlc_on || !b.charge_flank)
        return false;
    auto stop = [&](const char* why) {
        if (m_nlc_flank_on)
            TAC_LOG("~ [evade] [%s]: flank charge end (%s)", cName().c_str(), why);
        m_nlc_flank_on = false;
        return false;
    };
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !smart_cast<const CActor*>(enemy) || nlc_siege_active() || nlc_siege_wanted() || m_nlc_grab_intent)
        return stop("no target");
    if (nlc_threat() & eThreatWatched)
        return stop("watched"); // watched: straight in (side guard)
    Fvector eye, view;
    if (!nlc_enemy_eye(enemy, eye, view))
        return stop("no view");
    view.y = 0.f;
    if (view.square_magnitude() < EPS_L)
        return stop("no view");
    view.normalize();

    const Fvector ep = enemy->Position();
    const Fvector r = flat_dir(ep, Position()); // player -> monster
    const float dist = ep.distance_to_xz(Position());
    if (r.square_magnitude() < EPS_L || dist < b.charge_flank_dist.x || dist > b.charge_flank_dist.y)
        return stop("distance");
    const float phi = angle_between_vectors(view, r); // 0 in front of the player, 180 behind
    if (phi >= b.charge_flank_angle - deg2rad(10.f))
        return stop("at the flank");

    // the side the monster is on (held 2 s, a turning player would flip it every frame)
    const u32 now = Device.dwTimeGlobal;
    if (!m_nlc_flank_side || now >= m_nlc_flank_side_until)
    {
        m_nlc_flank_side = (view.x * r.z - view.z * r.x) >= 0.f ? 1 : -1;
        m_nlc_flank_side_until = now + 2000;
    }
    // a waypoint on a ring around the player, charge_flank_step farther around toward the flank, a bit closer each time
    const float step = std::min(b.charge_flank_step, b.charge_flank_angle - phi);
    const Fvector w = rotate_y(r, float(m_nlc_flank_side) * step);
    const float radius = std::max(dist * 0.85f, b.charge_flank_dist.x);
    Fvector pt;
    pt.set(ep.x + w.x * radius, Position().y, ep.z + w.z * radius);
    if (!ai().level_graph().valid_vertex_position(pt))
        return stop("waypoint off the map");
    const u32 n = ai().level_graph().vertex_id(pt);
    if (!ai().level_graph().valid_vertex_id(n))
        return stop("waypoint off the map");
    pt = ai().level_graph().vertex_position(n);
    if (_abs(pt.y - Position().y) > 3.f || !movement().restrictions().accessible(pt) || !Home->at_home(pt))
        return stop("waypoint blocked");
    if (!m_nlc_flank_on)
    {
        m_nlc_flank_on = true;
        TAC_LOG("~ [evade] [%s]: flank charge %s: %.0f deg off the view, %.1f m", cName().c_str(), m_nlc_flank_side > 0 ? "right" : "left", rad2deg(phi), dist);
    }
    position = pt;
    vertex = n;
    return true;
}

//////////////////////////////////////////////////////////////////////////
// pounce (experimental)
//////////////////////////////////////////////////////////////////////////

void CAI_Bloodsucker::nlc_pounce_update()
{
    if (!m_nlc_on || !m_nlc_b.amb_pounce)
        return;
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;

    // the rolls happen before any reveal (a refused roll used to reveal it for nothing), each only while the enemy is
    // on that kind of ground
    float eh = 0.f;
    if (nlc_pounce_ready() && nlc_enemy_perch_h(EnemyMan.get_enemy(), eh))
    {
        auto roll = [&](SNlcPounceRoll& r, float chance, const char* what) {
            if (chance <= 0.f || (r.rolled && (r.go || now < r.next)))
                return;
            r.rolled = true;
            r.go = ::Random.randF() < chance;
            r.next = now + b.pounce_delay;
            if (!r.go)
                TAC_LOG("~ [evade] [%s]: pounce: %s roll %.2f -> no, again in %u ms", cName().c_str(), what, chance, b.pounce_delay);
        };
        if (eh >= 0.4f)
            roll(m_nlc_roll_perch, b.pounce_perch_chance, "perch");
        else
            roll(m_nlc_roll_flat, b.pounce_flat ? b.pounce_chance : 0.f, "flat");
    }

    // the siege perch-strike spot: no pounce soon after arriving (cooldown, refused roll, no jump possible) -> hide
    if (nlc_siege_perch_holding())
    {
        if (!m_nlc_perch_since)
            m_nlc_perch_since = now;
        if (m_nlc_roll_perch.rolled && !m_nlc_roll_perch.go)
            nlc_perch_give_up("pounce roll refused");
        else if (now >= m_nlc_perch_since + b.pounce_perch_wait_ms)
            nlc_perch_give_up("no pounce in time");
    }
    else
        m_nlc_perch_since = 0;

    // with strike_reveal_margin the pounce comes from full visibility after the tell; else (legacy) from the cloak
    if (b.strike_reveal_margin > 0.f ? !nlc_strike_tell_done() : !state_invisible)
        return;
    if (now < m_nlc_next_pounce || !nlc_pounce_ready())
        return;
    m_nlc_next_pounce = now + 200;

    // diagnostics: at the perch-strike spot with the pounce ready, say why it does not start (once a second)
    const bool perch_diag = nlc_siege_perch_holding() && nlc_evade_debug();
    auto blocked = [&](const char* why) {
        if (perch_diag && now >= m_nlc_perch_block_log)
        {
            m_nlc_perch_block_log = now + 1000;
            Msg("~ [evade] [%s]: perch pounce waits: %s", cName().c_str(), why);
        }
    };
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return;
    if (!EnemyMan.see_enemy_now())
        return blocked("enemy not seen");
    if (CControlledActor::is_controlling() || com_man().is_jumping())
        return blocked("grab or jump running");
    // not while a grab from behind is planned
    if (m_nlc_grab_intent)
        return blocked("grab intent");
    // a perched enemy (feet 0.4 m or more above its ground point, up to pounce_perch_max_h) uses the perch roll, flat
    // ground the flat roll (pounce_flat); in a siege only the perch
    float h = 0.f;
    if (!nlc_enemy_perch_h(enemy, h) || h > b.pounce_max_h)
        return blocked("enemy too high or off the ai-map");
    const bool perch = h >= 0.4f;
    if (!perch && (nlc_siege_active() || nlc_siege_wanted()))
        return blocked("enemy no longer perched");
    if (!nlc_pounce_wanted(perch))
        return blocked("roll");
    CControlRunAttack* ra = com_man().nlc_run_attack();
    if (ra && ra->is_active())
        return blocked("lunge running");
    if (StateMan && StateMan->get_state_type() == eStateVampire_Execute)
        return;

    // the range as CControlJump::can_jump measures it: 3D, the feet to the enemy's centre (a perch adds its height)
    Fvector target;
    enemy->Center(target);
    const float dist = target.distance_to(Position());
    if (dist < m_nlc_b.pounce_min || dist > m_nlc_b.pounce_max)
    {
        string64 why;
        xr_sprintf(why, "distance %.1f m", dist);
        return blocked(why);
    }
    Fvector face = Direction();
    face.y = 0.f;
    Fvector to;
    to.sub(enemy->Position(), Position());
    to.y = 0.f;
    if (face.square_magnitude() < EPS_L || to.square_magnitude() < EPS_L || angle_between_vectors(face, to) > deg2rad(20.f))
    {
        string64 why;
        xr_sprintf(why, "facing %.0f deg off", (face.square_magnitude() < EPS_L || to.square_magnitude() < EPS_L) ? 0.f : rad2deg(angle_between_vectors(face, to)));
        return blocked(why);
    }

    m_nlc_allow_jump = true;
    const bool ok = com_man().jump_if_possible(target, const_cast<CEntityAlive*>(enemy), true, true, true);
    m_nlc_allow_jump = false;
    if (ok)
    {
        // x-ray only: partial and no visibility share the predator visual, so the model is not swapped mid-jump
        // (a full decloak here replaced the model under the starting glide: crash in CControlJump::on_event, nlc-3.589.31)
        nlc_strike_reveal("pounce", true);
        m_nlc_tac.rolled_grab = false;
        m_nlc_perch_since = 0;
        nlc_pounce_cooldown(b.pounce_delay);
        TAC_LOG("~ [evade] [%s]: pounce at %.1f m%s", cName().c_str(), dist, perch ? " (perch)" : "");
    }
    else if (now >= m_nlc_pounce_fail_log)
    {
        // CControlJump::can_jump refused (cooldown, distance, heading, height vs jump_max_height, restrictor)
        m_nlc_pounce_fail_log = now + 2000;
        TAC_LOG("~ [evade] [%s]: pounce: jump not possible (%.1f m, target %.1f m above the feet%s)", cName().c_str(), Position().distance_to(target), target.y - Position().y,
            perch ? ", perch" : "");
    }
}

//////////////////////////////////////////////////////////////////////////
// tactic: helpers
//////////////////////////////////////////////////////////////////////////

void CAI_Bloodsucker::nlc_tac_finish(const char* reason)
{
    SNlcTactic& t = m_nlc_tac;
    if (t.mode != eTacNone)
        TAC_LOG("~ [evade] [%s]: tactic %s end (%s) after %.1f s", cName().c_str(), mode_name(t.mode), reason, float(Device.dwTimeGlobal - t.mode_start) / 1000.f);
    t.mode = eTacNone;
    t.want = false;
    t.moving = false;
    t.have_target = false;
    t.force_vis = unset;
    t.feint_phase = 0;
    t.away_since = 0;
}

// leave the tactic to strike: the stock charge (with the lunge) or the grab takes over
void CAI_Bloodsucker::nlc_tac_commit(const char* reason)
{
    SNlcTactic& t = m_nlc_tac;
    const u32 now = Device.dwTimeGlobal;
    t.stalk_block = now + 15000;
    t.committed_until = now + 20000;
    nlc_tac_finish(reason);
}

void CAI_Bloodsucker::nlc_tac_move(const Fvector& target, u32 node, EAction act)
{
    set_action(act);
    if (act == ACT_RUN)
    {
        anim().accel_activate(eAT_Aggressive);
        anim().accel_set_braking(false);
    }
    else
        anim().accel_deactivate();
    path().set_target_point(target, node);
    path().set_rebuild_time(250);
    path().set_distance_to_end(1.f);
    path().set_use_covers(false);
    path().set_use_dest_orient(false);
}

void CAI_Bloodsucker::nlc_tac_hold(const CEntityAlive* enemy)
{
    set_action(ACT_STAND_IDLE);
    nlc_face_point(enemy->Position());
}

// a point on the line toward the enemy's position from this monster, tac_bait_dist away from the enemy
bool CAI_Bloodsucker::nlc_tac_bait_point(const CEntityAlive* enemy, Fvector& pos, u32& node)
{
    const Fvector dir = flat_dir(enemy->Position(), Position()); // enemy -> monster
    if (dir.square_magnitude() < EPS_L)
        return false;
    const float d = ::Random.randF(m_nlc_b.tac_bait_dist.x, m_nlc_b.tac_bait_dist.y);
    Fvector wanted;
    wanted.mad(enemy->Position(), dir, d);
    return nlc_point_on_map(wanted, pos, node) && Home->at_home(pos);
}

// hidden / unwatched point around `center` (3 radii x 8 angles, the siege loop shape): the nearest to the monster
int CAI_Bloodsucker::nlc_tac_pick(const Fvector& center, Fvector2 ring, float bias_angle, const Fvector* bias_dir, bool need_hidden, bool prefer_hidden, bool need_unwatched,
                                  Fvector& pos, u32& node)
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return eTacPickNone;
    if (!pick_budget())
        return eTacPickDeferred;

    const SNlcTactic& t = m_nlc_tac;
    const u32 enemy_node = enemy->ai_location().level_vertex_id();
    const float ground_y = ai().level_graph().valid_vertex_id(enemy_node) ? ai().level_graph().vertex_position(enemy_node).y : enemy->Position().y;
    const bool guard = nlc_side_guard_on() && (nlc_threat() & eThreatWatched);

    Fvector eye{}, view{};
    if (need_unwatched && !nlc_enemy_eye(enemy, eye, view))
        need_unwatched = false;

    // pass 0: hidden (when wanted or preferred), pass 1: relaxed to unwatched only
    const int passes = (need_hidden || !prefer_hidden) ? 1 : 2;
    for (int pass = 0; pass < passes; ++pass)
    {
        const bool hidden = need_hidden || (prefer_hidden && pass == 0);
        float best = flt_max;
        bool found = false;
        const float start = ::Random.randF(0.f, PI_MUL_2);
        for (float k : {0.15f, 0.55f, 0.95f})
        {
            const float r = ring.x + (ring.y - ring.x) * k;
            for (int i = 0; i < 8; ++i)
            {
                const float a = start + float(i) * PI_DIV_4 + k;
                Fvector pt;
                pt.set(center.x + _cos(a) * r, ground_y, center.z + _sin(a) * r);
                if (!ai().level_graph().valid_vertex_position(pt))
                    continue;
                const u32 n = ai().level_graph().vertex_id(pt);
                if (!ai().level_graph().valid_vertex_id(n) || n == t.burned)
                    continue;
                pt = ai().level_graph().vertex_position(n);
                if (_abs(pt.y - ground_y) > 4.f || !movement().restrictions().accessible(pt) || !Home->at_home(pt))
                    continue;
                if (guard && nlc_guard_lateral(pt, enemy))
                    continue;
                if (bias_dir)
                {
                    Fvector d;
                    d.set(pt.x - center.x, 0.f, pt.z - center.z);
                    if (angle_between_vectors(d, *bias_dir) > bias_angle)
                        continue;
                }
                const float mine = pt.distance_to(Position());
                if (mine >= best)
                    continue;
                if (need_unwatched)
                {
                    Fvector to;
                    to.set(pt.x - eye.x, pt.y + 1.f - eye.y, pt.z - eye.z);
                    if (angle_between_vectors(view, to) <= nlc_watch_cone())
                        continue;
                }
                if (hidden && nlc_hidden_test_visible(pt, enemy))
                    continue;
                best = mine;
                pos = pt;
                node = n;
                found = true;
            }
        }
        if (found)
            return eTacPickFound;
    }
    return eTacPickNone;
}

// a hidden point straight away from the enemy at min_d..max_d (4 samples, +-10 degrees)
bool CAI_Bloodsucker::nlc_tac_radial(const CEntityAlive* enemy, float min_d, float max_d, bool need_hidden, Fvector& pos, u32& node)
{
    const Fvector away = flat_dir(enemy->Position(), Position());
    const u32 mine = ai_location().level_vertex_id();
    if (away.square_magnitude() < EPS_L || !ai().level_graph().valid_vertex_id(mine))
        return false;
    for (int i = 0; i < 4; ++i)
    {
        const float d = min_d + (max_d - min_d) * float(i) / 3.f;
        const Fvector dir = rotate_y(away, deg2rad(::Random.randF(-10.f, 10.f)));
        Fvector pt;
        pt.set(Position().x + dir.x * d, Position().y, Position().z + dir.z * d);
        if (!ai().level_graph().valid_vertex_position(pt))
            continue;
        const u32 n = ai().level_graph().vertex_id(pt);
        if (!ai().level_graph().valid_vertex_id(n))
            continue;
        pt = ai().level_graph().vertex_position(n);
        if (_abs(pt.y - Position().y) > 3.f || !movement().restrictions().accessible(pt) || !Home->at_home(pt))
            continue;
        if (!ai().level_graph().valid_vertex_id(ai().level_graph().check_position_in_direction(mine, Position(), pt)))
            continue;
        if (need_hidden && nlc_hidden_test_visible(pt, enemy))
            continue;
        pos = pt;
        node = n;
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// tactic: decisions (every 200 ms from UpdateCL)
//////////////////////////////////////////////////////////////////////////

void CAI_Bloodsucker::nlc_tactic_update()
{
    SNlcTactic& t = m_nlc_tac;
    if (!m_nlc_on)
        return;
    if (!g_Alive() || CCustomMonster::use_simplified_visual())
    {
        m_nlc_grab_intent = false;
        m_nlc_reveal_until = 0;
        if (t.want || t.mode != eTacNone)
            nlc_tac_finish("dead");
        return;
    }

    const u32 now = Device.dwTimeGlobal;
    if (now < t.next_update)
        return;
    t.next_update = now + 200;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    const u16 eid = enemy ? enemy->ID() : u16(-1);
    if (eid != t.enemy_id)
    {
        if (t.mode != eTacNone)
            nlc_tac_finish(enemy ? "enemy changed" : "enemy lost");
        t.enemy_id = eid;
        t.rolled_grab = false;
        t.role = eRoleNone;
        t.committed_until = 0;
        t.bias_set = false;
        t.burned = u32(-1);
        t.bias_stage = 0;
    }
    if (!enemy)
    {
        m_nlc_grab_intent = false;
        return;
    }

    t.night = nlc_tac_night();
    if (nlc_threat() & eThreatWatched)
        t.rolled_grab = false; // seen: the next approach rolls again

    // grab intent: behind the player, unseen, cloaked, close: no lunge or pounce, stay cloaked until the grab range
    {
        const bool was = m_nlc_grab_intent;
        m_nlc_grab_intent = false;
        if (m_nlc_vampire_ambush && !CControlledActor::is_controlling() && !nlc_siege_active() && smart_cast<const CActor*>(enemy) &&
            enemy->Position().distance_to(Position()) <= m_nlc_b.vampire_intent_dist)
            m_nlc_grab_intent = nlc_grab_possible(enemy, true);
        if (m_nlc_grab_intent != was)
            TAC_LOG("~ [evade] [%s]: grab intent %s at %.1f m", cName().c_str(), m_nlc_grab_intent ? "on" : "off", enemy->Position().distance_to(Position()));
    }

    // stalk, hit and run, feint: keep the enemy in memory while hiding from it (the bloodsucker's memory is 40 s,
    // a night stalk can last 60 s); the pin lapses 1.5 s after the mode ends
    if (t.mode == eTacStalk || t.mode == eTacRecover || t.mode == eTacFeint)
        EnemyMemory.pin(enemy, now + 1500);

    if (!m_nlc_tac_any)
        return;

    if (nlc_siege_active() || nlc_siege_wanted() || (m_controlled && m_controlled->is_under_control()) || CControlledActor::is_controlling())
    {
        if (t.mode != eTacNone)
            nlc_tac_finish("siege, control or grab");
        return;
    }

    const bool actor_enemy = smart_cast<const CActor*>(enemy) != nullptr;
    nlc_tac_update_role(enemy);
    if (t.mode == eTacNone)
        nlc_tac_try_enter(enemy, actor_enemy);
    else
        nlc_tac_continue(enemy, actor_enemy);
}

// bait = the lowest ID of the bloodsuckers of this squad with the same enemy (re-ranked every 2 s), the rest flank
void CAI_Bloodsucker::nlc_tac_update_role(const CEntityAlive* enemy)
{
    SNlcTactic& t = m_nlc_tac;
    if (!m_nlc_b.tac_pair_enabled)
    {
        t.role = eRoleNone;
        return;
    }
    const u32 now = Device.dwTimeGlobal;
    if (now < t.next_rank)
        return;
    t.next_rank = now + 2000;

    ENlcTacRole role = eRoleNone;
    if (CMonsterSquad* squad = monster_squad().get_squad(this))
    {
        xr_vector<CEntity*> mates;
        squad->nlc_members(mates);
        bool any = false, lowest = true;
        for (CEntity* e : mates)
        {
            CAI_Bloodsucker* m = smart_cast<CAI_Bloodsucker*>(e);
            if (!m || m == this || !m->g_Alive() || m->getDestroy() || !m->m_nlc_b.tac_pair_enabled)
                continue;
            const CEntityAlive* me = m->EnemyMan.get_enemy();
            if (!me || me->ID() != enemy->ID())
                continue;
            any = true;
            if (m->ID() < ID())
                lowest = false;
        }
        if (any)
            role = lowest ? eRoleBait : eRoleFlanker;
    }
    if (role != t.role)
    {
        TAC_LOG("~ [evade] [%s]: pair role %s", cName().c_str(), role == eRoleBait ? "bait" : (role == eRoleFlanker ? "flanker" : "none"));
        t.role = role;
    }
}

bool CAI_Bloodsucker::nlc_tac_flanker_struck(const CEntityAlive* enemy)
{
    CMonsterSquad* squad = monster_squad().get_squad(this);
    if (!squad || !enemy)
        return false;
    const u32 now = Device.dwTimeGlobal;
    xr_vector<CEntity*> mates;
    squad->nlc_members(mates);
    for (CEntity* e : mates)
    {
        CAI_Bloodsucker* m = smart_cast<CAI_Bloodsucker*>(e);
        if (!m || m == this || !m->g_Alive() || m->m_nlc_tac.role != eRoleFlanker)
            continue;
        const CEntityAlive* me = m->EnemyMan.get_enemy();
        if (me && me->ID() == enemy->ID() && now < m->m_nlc_tac.committed_until)
            return true;
    }
    return false;
}

void CAI_Bloodsucker::nlc_tac_enter(ENlcTacMode mode, const Fvector& target, u32 node)
{
    SNlcTactic& t = m_nlc_tac;
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    const float dist = enemy ? enemy->Position().distance_to_xz(Position()) : 0.f;

    t.mode = mode;
    t.want = true;
    t.mode_start = now;
    t.moving = false;
    t.have_target = false;
    t.away_since = 0;
    t.hit_seen = HitMemory.get_last_hit_time();

    switch (mode)
    {
    case eTacRecover:
        t.target = target;
        t.node = node;
        t.have_target = t.moving = true;
        t.move_until = now + 8000 + u32(Position().distance_to(target) * 400.f);
        t.force_vis = no_visibility;
        TAC_LOG("~ [evade] [%s]: tactic recover: health %.2f, enemy %.1f m, hiding %.1f m away", cName().c_str(), conditions().GetHealth() / std::max(conditions().GetMaxHealth(), EPS),
            dist, Position().distance_to(target));
        break;
    case eTacStalk:
        t.force_vis = no_visibility;
        t.bias_stage = 0;
        t.next_pick = now;
        t.next_repick_ok = now + 1500;
        TAC_LOG("~ [evade] [%s]: tactic stalk: enemy %.1f m%s%s", cName().c_str(), dist, t.night ? ", night" : "", t.bias_set ? ", from another side" : "");
        break;
    case eTacFeint:
        t.feint_phase = 0;
        t.feint_until = now + m_nlc_b.tac_feint_show_ms;
        t.force_vis = m_nlc_b.tac_feint_vis == 1 ? partial_visibility : full_visibility; // NLC: tac_feint_vis
        nlc_set_cloak(t.force_vis, true);
        sound().play(CAI_Bloodsucker::eGrowl);
        TAC_LOG("~ [evade] [%s]: tactic feint: showing itself at %.1f m for %u ms", cName().c_str(), dist, m_nlc_b.tac_feint_show_ms);
        break;
    case eTacBait:
        t.target = target;
        t.node = node;
        t.have_target = t.moving = true;
        t.move_until = now + 8000 + u32(Position().distance_to(target) * 400.f);
        t.force_vis = m_nlc_b.tac_bait_vis == 1 ? partial_visibility : full_visibility; // NLC: tac_bait_vis
        t.next_pick = now + 1000;
        t.next_growl = now + u32(::Random.randI(4000, 7000));
        nlc_set_cloak(t.force_vis, true);
        TAC_LOG("~ [evade] [%s]: tactic bait: enemy %.1f m, going to %.1f m from it", cName().c_str(), dist, enemy ? target.distance_to_xz(enemy->Position()) : 0.f);
        break;
    default: break;
    }
}

void CAI_Bloodsucker::nlc_tac_try_enter(const CEntityAlive* enemy, bool actor_enemy)
{
    SNlcTactic& t = m_nlc_tac;
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;
    const float dist = enemy->Position().distance_to_xz(Position());

    // 1. recover (hit and run): hurt, the enemy not at arm's length
    if (b.tac_recover_enabled && now >= t.recover_cd)
    {
        const float max_h = conditions().GetMaxHealth();
        const float hp = max_h > 0.f ? conditions().GetHealth() / max_h : 1.f;
        // cornered (closer than tac_recover_min_dist) it fights, unless below tac_recover_break_health (it kept fighting
        // at point blank down to 10-20% health)
        if (hp < b.tac_recover_health && (dist > b.tac_recover_min_dist || hp < b.tac_recover_break_health))
        {
            const Fvector away = flat_dir(EnemyMan.get_enemy_position(), Position());
            Fvector pos{};
            u32 node = u32(-1);
            // a hidden point away from the enemy; else one out of its view; else straight away (open ground had none)
            int r = nlc_tac_pick(EnemyMan.get_enemy_position(), /* NLC: last known position */ b.tac_recover_dist, deg2rad(60.f), &away, true, false, false, pos, node);
            if (r == eTacPickNone)
                r = nlc_tac_pick(EnemyMan.get_enemy_position(), b.tac_recover_dist, deg2rad(75.f), &away, false, true, true, pos, node);
            if (r == eTacPickDeferred)
                return;
            if (r == eTacPickNone && nlc_tac_radial(enemy, 10.f, 18.f, false, pos, node))
                r = eTacPickFound;
            if (r == eTacPickNone)
            {
                t.recover_cd = now + 3000;
                TAC_LOG("~ [evade] [%s]: recover: no point to break away to", cName().c_str());
                return;
            }
            nlc_tac_enter(eTacRecover, pos, node);
            return;
        }
    }

    if (!actor_enemy)
        return; // NPC enemies: only recover

    // 2. bait of a pair
    if (b.tac_pair_enabled && t.role == eRoleBait && now >= t.stalk_block && EnemyMan.see_enemy_now() && !nlc_tac_flanker_struck(enemy))
    {
        Fvector pos{};
        u32 node = u32(-1);
        if (nlc_tac_bait_point(enemy, pos, node))
        {
            nlc_tac_enter(eTacBait, pos, node);
            return;
        }
    }

    // 3. feint: watched at the feint distance, one roll per cooldown
    if (b.tac_feint_enabled && now >= t.feint_cd && dist >= b.tac_feint_dist.x && dist <= b.tac_feint_dist.y && (nlc_threat() & eThreatWatched))
    {
        t.feint_cd = now + b.tac_feint_cooldown;
        const float chance = std::clamp(b.tac_feint_chance + (t.night ? b.tac_night_chance_add : 0.f), 0.f, 1.f);
        const bool go = ::Random.randF() < chance;
        TAC_LOG("~ [evade] [%s]: feint: roll %.2f%s -> %s", cName().c_str(), chance, t.night ? " (night)" : "", go ? "yes" : "no");
        if (go)
        {
            nlc_tac_enter(eTacFeint, Position(), ai_location().level_vertex_id());
            return;
        }
    }

    // 4. stalk: a far bloodsucker shadows the enemy instead of charging (a close one just attacks)
    if (b.tac_stalk_enabled && now >= t.stalk_block && dist > b.tac_stalk_dist.x * (t.night ? b.tac_night_dist_k : 1.f) && EnemyMan.see_enemy_now())
        nlc_tac_enter(eTacStalk, Position(), ai_location().level_vertex_id());
}

void CAI_Bloodsucker::nlc_tac_continue(const CEntityAlive* enemy, bool actor_enemy)
{
    SNlcTactic& t = m_nlc_tac;
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;
    const float dist = enemy->Position().distance_to_xz(Position());

    if (!actor_enemy && t.mode != eTacRecover)
    {
        nlc_tac_finish("npc enemy");
        return;
    }

    // recover -> stalk from another side (or just leave)
    auto to_stalk = [&](const Fvector& from_dir, float angle, const char* why) {
        t.moving = false;
        t.have_target = false;
        if (b.tac_stalk_enabled && actor_enemy)
        {
            const float side = ::Random.randI(2) ? 1.f : -1.f;
            t.bias = rotate_y(from_dir, side * angle);
            t.bias_set = t.bias.square_magnitude() > EPS_L;
            t.mode = eTacStalk;
            t.mode_start = now;
            t.force_vis = no_visibility;
            t.next_pick = now;
            t.next_repick_ok = now + 1500;
            t.feint_phase = 0;
            TAC_LOG("~ [evade] [%s]: tactic stalk (%s): returning %.0f deg around the player", cName().c_str(), why, rad2deg(angle));
        }
        else
            nlc_tac_finish(why);
    };

    switch (t.mode)
    {
    case eTacRecover:
    {
        // shot while moving: burn the point and look for another
        const u32 hit = HitMemory.get_last_hit_time();
        if (hit > t.hit_seen)
        {
            t.hit_seen = hit;
            if (t.moving && HitMemory.get_last_hit_object() == enemy)
            {
                t.burned = t.node;
                const Fvector away = flat_dir(EnemyMan.get_enemy_position(), Position());
                Fvector pos{};
                u32 node = u32(-1);
                const int r = nlc_tac_pick(EnemyMan.get_enemy_position(), /* NLC: last known position */ b.tac_recover_dist, deg2rad(60.f), &away, true, false, false, pos, node);
                if (r == eTacPickFound)
                {
                    t.target = pos;
                    t.node = node;
                    t.move_until = now + 8000 + u32(Position().distance_to(pos) * 400.f);
                    TAC_LOG("~ [evade] [%s]: recover: shot, new hiding point %.1f m away", cName().c_str(), Position().distance_to(pos));
                }
                else
                    TAC_LOG("~ [evade] [%s]: recover: shot, no other hiding point", cName().c_str());
            }
        }
        const float max_h = conditions().GetMaxHealth();
        // NLC: tac_recover_min_time: with fast regeneration the hide would last only seconds
        const bool healed = max_h > 0.f && conditions().GetHealth() / max_h >= b.tac_recover_until && now >= t.mode_start + b.tac_recover_min_time;
        if (healed || now >= t.mode_start + b.tac_recover_max_time)
        {
            t.left_dir = flat_dir(enemy->Position(), Position());
            t.recover_cd = now + b.tac_recover_cooldown;
            to_stalk(t.left_dir, b.tac_return_angle, healed ? "recovered" : "recover time");
        }
        break;
    }

    case eTacStalk:
    {
        const float dk = t.night ? b.tac_night_dist_k : 1.f;
        const float tk = t.night ? b.tac_night_time_k : 1.f;

        // watched at the current spot: shadow from another point
        if (now >= t.next_repick_ok && (nlc_threat() & eThreatWatched))
        {
            t.next_repick_ok = now + 1500;
            t.next_pick = now;
            TAC_LOG("~ [evade] [%s]: stalk: watched here, new point", cName().c_str());
        }

        if (dist < std::max(6.f, b.tac_stalk_dist.x * dk * 0.5f))
        {
            nlc_tac_commit("stalk: close, strike");
            break;
        }
        if (now >= t.mode_start + u32(float(b.tac_stalk_max_time) * tk))
        {
            nlc_tac_commit("stalk: time, strike");
            break;
        }

        CActor* actor = smart_cast<CActor*>(const_cast<CEntityAlive*>(enemy));
        if (!actor)
            break;

        // openings: reload, weapon switch
        if (b.tac_open_reload || b.tac_open_switch)
        {
            PIItem item = actor->inventory().ActiveItem();
            CHudItem* hud_item = item ? item->cast_hud_item() : nullptr;
            if (hud_item)
            {
                const u32 st = hud_item->GetState();
                if (b.tac_open_reload && st == CHUDState::eReload)
                {
                    nlc_tac_commit("opening: reload");
                    break;
                }
                if (b.tac_open_switch && (st == CHUDState::eHiding || st == CHUDState::eShowing))
                {
                    nlc_tac_commit("opening: weapon switch");
                    break;
                }
            }
        }

        // opening: looked away for tac_open_away_ms
        Fvector eye{}, view{};
        if (nlc_enemy_eye(enemy, eye, view))
        {
            Fvector to_me;
            to_me.sub(Position(), actor->Position());
            if (angle_between_vectors(view, to_me) > b.tac_open_away_angle)
            {
                if (!t.away_since)
                    t.away_since = now;
                else if (now - t.away_since >= b.tac_open_away_ms)
                {
                    nlc_tac_commit("opening: looked away");
                    break;
                }
            }
            else
                t.away_since = 0;

            // opening: the player is watching the bait
            if (t.role == eRoleFlanker)
            {
                if (CMonsterSquad* squad = monster_squad().get_squad(this))
                {
                    xr_vector<CEntity*> mates;
                    squad->nlc_members(mates);
                    for (CEntity* e : mates)
                    {
                        CAI_Bloodsucker* m = smart_cast<CAI_Bloodsucker*>(e);
                        if (!m || m == this || !m->g_Alive() || m->m_nlc_tac.role != eRoleBait)
                            continue;
                        const CEntityAlive* me = m->EnemyMan.get_enemy();
                        if (!me || me->ID() != enemy->ID())
                            continue;
                        if (angle_between_vectors(view, Fvector().sub(m->Position(), eye)) < deg2rad(20.f))
                        {
                            nlc_tac_commit("opening: the player watches the bait");
                            break;
                        }
                    }
                }
            }
        }
        break;
    }

    case eTacFeint:
    {
        if (t.feint_phase == 0)
        {
            if (now < t.feint_until)
                break;
            // vanish (normal switch delay) and break the line of sight radially
            t.feint_phase = 1;
            t.force_vis = no_visibility;
            m_nlc_cloak_until = now + b.tac_feint_cloak_ms; // stays cloaked after force_vis ends (tactic end, strike reveal)
            Fvector pos{};
            u32 node = u32(-1);
            if (nlc_tac_radial(enemy, 6.f, 14.f, true, pos, node))
            {
                t.target = pos;
                t.node = node;
                t.have_target = t.moving = true;
                t.move_until = now + 6000;
                TAC_LOG("~ [evade] [%s]: feint: vanishing %.1f m away", cName().c_str(), Position().distance_to(pos));
            }
            else
            {
                // no hidden point: charge on cloaked instead of standing (it stood in view: free shots)
                m_nlc_cloak_until = now + b.tac_feint_cloak_ms;
                TAC_LOG("~ [evade] [%s]: feint: no radial hidden point, charging cloaked", cName().c_str());
                nlc_tac_commit("feint: charge cloaked");
            }
        }
        else if (!t.moving)
            to_stalk(flat_dir(enemy->Position(), Position()), b.tac_feint_flank_angle, "feint done");
        break;
    }

    case eTacBait:
    {
        if (t.role != eRoleBait)
        {
            nlc_tac_finish("no longer the bait");
            break;
        }
        // a flanker struck, the bait waited long enough, or the player came close
        if (nlc_tac_flanker_struck(enemy))
        {
            nlc_tac_commit("bait: a flanker struck, charge");
            break;
        }
        if (now >= t.mode_start + b.tac_bait_max_time)
        {
            nlc_tac_commit("bait: time, charge");
            break;
        }
        if (dist < b.tac_bait_dist.x * 0.6f)
        {
            nlc_tac_commit("bait: close, charge");
            break;
        }

        // shot: back off straight away from the player, never sideways
        const u32 hit = HitMemory.get_last_hit_time();
        if (hit > t.hit_seen)
        {
            t.hit_seen = hit;
            if (HitMemory.get_last_hit_object() == enemy)
            {
                Fvector pos{};
                u32 node = u32(-1);
                if (nlc_tac_radial(enemy, 4.f, 7.f, false, pos, node))
                {
                    t.target = pos;
                    t.node = node;
                    t.have_target = t.moving = true;
                    t.move_until = now + 4000;
                    TAC_LOG("~ [evade] [%s]: bait: shot, backing off %.1f m", cName().c_str(), Position().distance_to(pos));
                }
                else
                    TAC_LOG("~ [evade] [%s]: bait: shot, no radial move", cName().c_str());
            }
        }
        // back to the bait distance after a retreat or when the player walks away
        if (!t.moving && now >= t.next_pick && dist > b.tac_bait_dist.y + 1.5f)
        {
            t.next_pick = now + 1000;
            Fvector pos{};
            u32 node = u32(-1);
            if (nlc_tac_bait_point(enemy, pos, node))
            {
                t.target = pos;
                t.node = node;
                t.have_target = t.moving = true;
                t.move_until = now + 6000 + u32(Position().distance_to(pos) * 300.f);
            }
        }
        break;
    }

    default: break;
    }
}

//////////////////////////////////////////////////////////////////////////
// tactic: the attack substate (every frame while selected)
//////////////////////////////////////////////////////////////////////////

void CAI_Bloodsucker::nlc_tactic_begin()
{
    anim().clear_override_animation();
    TAC_LOG("~ [evade] [%s]: tactic %s begin", cName().c_str(), mode_name(m_nlc_tac.mode));
}

void CAI_Bloodsucker::nlc_tactic_end()
{
    anim().clear_override_animation();
    SNlcTactic& t = m_nlc_tac;
    t.force_vis = unset;
    if (t.mode != eTacNone)
    {
        // left by the state machine (a higher-priority state), not by the tactic itself
        TAC_LOG("~ [evade] [%s]: tactic %s interrupted by state %s", cName().c_str(), mode_name(t.mode), StateMan ? make_xrstr(StateMan->get_state_type()).c_str() : "?");
        t.mode = eTacNone;
        t.want = false;
        t.moving = false;
        t.have_target = false;
    }
}

void CAI_Bloodsucker::nlc_tactic_execute()
{
    anim().clear_override_animation();
    SNlcTactic& t = m_nlc_tac;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!t.want || t.mode == eTacNone || !enemy)
    {
        set_action(ACT_STAND_IDLE);
        return;
    }
    const SNlcBloodsuckerParams& b = m_nlc_b;
    const u32 now = Device.dwTimeGlobal;

    // arrival
    if (t.moving)
    {
        const bool arrived = ai_location().level_vertex_id() == t.node || Position().distance_to_xz(t.target) < 1.5f;
        if (arrived || now >= t.move_until)
        {
            t.moving = false;
            TAC_LOG("~ [evade] [%s]: %s %s", cName().c_str(), mode_name(t.mode), arrived ? "reached the point" : "move timed out");
            if (t.mode == eTacStalk)
                t.next_pick = now + u32(::Random.randI(3000, 6000)); // shadow here a while, then move on
        }
    }

    // stalk: pick the next shadow point
    if (t.mode == eTacStalk && now >= t.next_pick)
    {
        const float dk = t.night ? b.tac_night_dist_k : 1.f;
        Fvector2 ring;
        ring.set(b.tac_stalk_dist.x * dk, b.tac_stalk_dist.y * dk);

        const Fvector* bias = nullptr;
        Fvector bias_dir{};
        float bias_angle = deg2rad(50.f);
        if (t.bias_set)
        {
            bias_dir = t.bias;
            bias = &bias_dir;
        }
        else if (t.role == eRoleFlanker && t.bias_stage < 2)
        {
            Fvector eye{}, view{};
            if (nlc_enemy_eye(enemy, eye, view))
            {
                bias_dir.set(-view.x, 0.f, -view.z); // the player's back
                if (bias_dir.square_magnitude() > EPS_L)
                {
                    bias_dir.normalize();
                    bias = &bias_dir;
                    bias_angle = deg2rad(60.f);
                }
            }
        }
        else
        {
            if (t.bias_stage == 0 && !b.tac_stalk_flank)
                t.bias_stage = 1;
            const Fvector own = flat_dir(EnemyMan.get_enemy_position(), Position());
            Fvector eye{}, view{};
            if (t.bias_stage == 0 && nlc_enemy_eye(enemy, eye, view))
                view.y = 0.f;
            if (t.bias_stage == 0 && own.square_magnitude() > EPS_L && view.square_magnitude() > EPS_L)
            {
                // the player's flank on its own side (perpendicular to the view): a side approach, temples away
                view.normalize();
                const float side = (view.x * own.z - view.z * own.x) >= 0.f ? 1.f : -1.f;
                bias_dir = rotate_y(view, side * PI_DIV_2);
                bias = &bias_dir;
                bias_angle = b.tac_stalk_flank_angle;
            }
            else if (t.bias_stage <= 1 && own.square_magnitude() > EPS_L)
            {
                // its own side of the player: the route to a point across passed within the strike range and ended the stalk
                t.bias_stage = 1;
                bias_dir = own;
                bias = &bias_dir;
                bias_angle = deg2rad(70.f);
            }
        }

        Fvector pos{};
        u32 node = u32(-1);
        const int r = nlc_tac_pick(EnemyMan.get_enemy_position(), /* NLC: last known position */ ring, bias_angle, bias, false, true, true, pos, node);
        if (r == eTacPickFound)
        {
            t.target = pos;
            t.node = node;
            t.have_target = t.moving = true;
            t.move_until = now + 8000 + u32(Position().distance_to(pos) * 400.f);
            t.next_pick = now + 120000; // set again on arrival
            TAC_LOG("~ [evade] [%s]: stalk: new point %.1f m away, %.1f m from the player%s", cName().c_str(), Position().distance_to(pos), pos.distance_to_xz(enemy->Position()),
                !bias ? "" : (t.bias_set ? " (return side)" : (t.role == eRoleFlanker ? " (behind)" : (t.bias_stage == 0 ? " (flank)" : " (own side)"))));
            t.bias_set = false;
            t.bias_stage = 0;
        }
        else if (r == eTacPickNone)
        {
            if (bias)
            {
                if (!t.bias_set)
                    t.bias_stage = u8(std::min(t.bias_stage + 1, 2)); // flank -> own side -> none
                t.bias_set = false; // retry with the next preference next frame
                t.next_pick = now;
                TAC_LOG("~ [evade] [%s]: stalk: no point on the preferred side", cName().c_str());
            }
            else
            {
                TAC_LOG("~ [evade] [%s]: stalk: no unwatched point", cName().c_str());
                nlc_tac_commit("stalk: no point, strike");
                set_action(ACT_STAND_IDLE);
                return;
            }
        }
    }

    // the feint shows itself while it keeps charging (it used to stop and stand for the show)
    if (t.mode == eTacFeint && t.feint_phase == 0)
    {
        const u32 ev = enemy->ai_location().level_vertex_id();
        if (ai().level_graph().valid_vertex_id(ev))
        {
            nlc_tac_move(ai().level_graph().vertex_position(ev), ev, ACT_RUN);
            return;
        }
    }

    if (t.moving && t.have_target)
    {
        // the slow gait only in the edge case (cloaked, close, near the edge of the view); else the cloaked run
        const EAction act = (t.mode == eTacStalk && nlc_cloak_keeps_gait()) ? ACT_STEAL : ACT_RUN;
        nlc_tac_move(t.target, t.node, act);
        return;
    }

    nlc_tac_hold(enemy);
    if (t.mode == eTacBait && now >= t.next_growl)
    {
        t.next_growl = now + u32(::Random.randI(4000, 7000));
        sound().play(CAI_Bloodsucker::eGrowl);
    }
}
