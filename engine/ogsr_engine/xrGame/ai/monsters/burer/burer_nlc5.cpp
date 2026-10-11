// NLC burer pass 5 (docs/BURER_REDESIGN.md 5.13): NPC disarm, grenade snatch, shield bullet catch, pull, elevation
// (overwatch, memory pin), gaze meter with the stare window, barricade grip. Pass 1 lives in burer.cpp, passes 2-4 in
// burer_nlc.cpp.
#include "stdafx.h"
#include "burer.h"
#include "script_engine.h"
#include "../../../ai_space.h"
#include "../../../level.h"
#include "../../../actor.h"
#include "../../../ActorCondition.h"
#include "../../../Inventory.h"
#include "../../../Weapon.h"
#include "../../../WeaponAmmo.h"
#include "../../../Grenade.h"
#include "../../../Level_Bullet_Manager.h"
#include "xrserver_objects_alife_items.h"
#include "../../../ParticlesObject.h"
#include "../../../GamePersistent.h"
#include "../../../PhysicsShell.h"
#include "../../../memory_manager.h"
#include "../../../enemy_manager.h"
#include "../../../visual_memory_manager.h"
#include "../../../ai_object_location.h"
#include "../../../movement_manager.h"
#include "../../../restricted_object.h"
#include "xrmessages.h"
#include "../../../sound_player.h"
#include "../../../../xr_3da/CameraBase.h"
#include "../../../../Include/xrRender/Kinematics.h"
#include "../../stalker/ai_stalker.h"
#include "level_graph.h"
#include "cover_point.h"
#include "../monster_cover_manager.h"
#include "../ai_monster_utils.h"
#include "../control_animation_base.h"
#include "../anti_aim_ability.h"

namespace
{
void nlc5_read_list(LPCSTR section, LPCSTR key, LPCSTR fallback, xr_vector<shared_str>& out)
{
    out.clear();
    LPCSTR value = pSettings->line_exist(section, key) ? pSettings->r_string(section, key) : fallback;
    if (!value)
        return;
    const int count = _GetItemCount(value);
    string256 item;
    for (int i = 0; i < count; ++i)
    {
        LPCSTR name = _Trim(_GetItem(value, i, item));
        if (name && name[0])
            out.push_back(name);
    }
}

// main weapons only: automatic (6), shotguns (7), sniper rifles (8), launchers (9), machine guns (10); knives (1) and
// pistols (5) stay in the hand
bool nlc5_primary(const CWeapon* weapon)
{
    if (!weapon)
        return false;
    const u32 type = weapon->ef_weapon_type();
    return type >= 6 && type <= 10 && type != 9; // 9: RPG / RG-6, which the puppet gun refuses
}

u32 nlc5_rand(u32 lo, u32 hi) { return hi > lo ? lo + u32(::Random.randI(int(hi - lo + 1))) : lo; }
} // namespace

void CBurer::nlc_load_pass5(LPCSTR section)
{
    // barricade fit
    m_nlc_barricade_tremor = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_tremor", 0.03f);
    m_nlc_barricade_turn = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_turn", 2.f);
    LPCSTR grip = READ_IF_EXISTS(pSettings, r_string, section, "nlc_barricade_grip", "whole");
    m_nlc_barricade_grip = !xr_strcmp(grip, "head") ? 1 : (!xr_strcmp(grip, "torso") ? 2 : 0);
    nlc5_read_list(section, "nlc_barricade_grip_head_bones", "bip01_head", m_nlc_grip_head_bones);
    nlc5_read_list(section, "nlc_barricade_grip_torso_bones", "bip01_spine1,bip01_spine,bip01_spine2,bip01_pelvis", m_nlc_grip_torso_bones);

    // NPC disarm
    m_nlc_npc_disarm = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_npc_disarm", false);
    m_nlc_npc_disarm_range = READ_IF_EXISTS(pSettings, r_float, section, "nlc_npc_disarm_range", 20.f);
    m_nlc_npc_disarm_aim_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_npc_disarm_aim_ms", 1500);
    m_nlc_npc_disarm_grace = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_npc_disarm_grace", 30000);
    m_nlc_npc_disarm_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_npc_disarm_cooldown", 8000);
    m_nlc_npc_disarm_mag_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_npc_disarm_mag_k", 0.33f);
    m_nlc_npc_drop_lock_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_npc_drop_lock_ms", 6000);
    m_nlc_npc_seize_delay = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_npc_seize_delay", 500);
    m_nlc_threat_npc_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_threat_npc_k", 0.6f);
    m_nlc_npc_disarm_need_sidearm = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_npc_disarm_need_sidearm", true);
    m_nlc_npc_spare = READ_IF_EXISTS(pSettings, r_string, section, "nlc_npc_disarm_spare", "");
    m_nlc_npc_spare_ammo = READ_IF_EXISTS(pSettings, r_string, section, "nlc_npc_disarm_spare_ammo", "");
    m_nlc_npc_spare_rounds = u16(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_npc_disarm_spare_rounds", 16));
    m_nlc_npc_spare_cond_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_npc_disarm_spare_cond_min", 0.4f);
    m_nlc_npc_spare_cond_max = _max(READ_IF_EXISTS(pSettings, r_float, section, "nlc_npc_disarm_spare_cond_max", 0.7f), m_nlc_npc_spare_cond_min);
    if (m_nlc_npc_spare.size() && !pSettings->section_exist(m_nlc_npc_spare))
    {
        Msg("! [burer] %s: nlc_npc_disarm_spare [%s] not found, no spare pistol", section, m_nlc_npc_spare.c_str());
        m_nlc_npc_spare = nullptr;
    }
    if (m_nlc_npc_spare_ammo.size() && !pSettings->section_exist(m_nlc_npc_spare_ammo))
    {
        Msg("! [burer] %s: nlc_npc_disarm_spare_ammo [%s] not found, no spare ammo", section, m_nlc_npc_spare_ammo.c_str());
        m_nlc_npc_spare_ammo = nullptr;
    }
    m_nlc_grudge_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_grudge_ms", 60000);

    // grenade snatch
    m_nlc_snatch = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_snatch", false);
    m_nlc_snatch_idle = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_snatch_selected", false);
    m_nlc_snatch_range = READ_IF_EXISTS(pSettings, r_float, section, "nlc_snatch_range", 14.f);
    m_nlc_snatch_chance = READ_IF_EXISTS(pSettings, r_float, section, "nlc_snatch_chance", 0.5f);
    m_nlc_snatch_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_snatch_cooldown", 20000);
    m_nlc_snatch_speed = READ_IF_EXISTS(pSettings, r_float, section, "nlc_snatch_speed", 4.f);

    // shield bullet catch
    m_nlc_catch_max = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_catch_max", 0);
    m_nlc_catch_return_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_catch_return_k", 0.7f);
    m_nlc_catch_disp = READ_IF_EXISTS(pSettings, r_float, section, "nlc_catch_disp", 1.5f);
    m_nlc_catch_delay_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_catch_delay_ms", 200);
    m_nlc_catch_interval_min = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_catch_interval_min", 40);
    m_nlc_catch_interval_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_catch_interval_max", 80), m_nlc_catch_interval_min);
    m_nlc_catch_drop_on_break = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_catch_drop_on_break", true);
    m_nlc_catch_particle = READ_IF_EXISTS(pSettings, r_string, section, "nlc_catch_particle", "");
    m_nlc_catch_glow = READ_IF_EXISTS(pSettings, r_string, section, "nlc_catch_glow", "");
    m_nlc_catch_glow_ms = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_catch_glow_ms", 1200), u32(200));
    m_nlc_catch_glow_fast_ms = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_catch_glow_fast_ms", 350), u32(100));
    m_nlc_catch_flare = READ_IF_EXISTS(pSettings, r_string, section, "nlc_catch_flare", "");

    // pull
    m_nlc_pull = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_pull", false);
    m_nlc_pull_range = READ_IF_EXISTS(pSettings, r_float, section, "nlc_pull_range", 20.f);
    m_nlc_pull_speed = READ_IF_EXISTS(pSettings, r_float, section, "nlc_pull_speed", 10.f);
    m_nlc_pull_up = READ_IF_EXISTS(pSettings, r_float, section, "nlc_pull_up", 3.5f);
    m_nlc_pull_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_pull_ms", 300);
    m_nlc_pull_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_pull_cooldown", 12000);
    m_nlc_pull_delay_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_pull_delay_ms", 500);
    m_nlc_pull_tug = READ_IF_EXISTS(pSettings, r_float, section, "nlc_pull_tug", 8.f);

    // elevation
    m_nlc_elev_enabled = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_elev", false);
    m_nlc_elev_min_height = READ_IF_EXISTS(pSettings, r_float, section, "nlc_elev_min_height", 1.8f);
    m_nlc_elev_patience = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_elev_patience", 90000);
    m_nlc_overwatch_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_overwatch_min", 10.f);
    m_nlc_overwatch_max = _max(READ_IF_EXISTS(pSettings, r_float, section, "nlc_overwatch_max", 20.f), m_nlc_overwatch_min);
    m_nlc_overwatch_hold_min = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_overwatch_hold_min", 4000);
    m_nlc_overwatch_hold_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_overwatch_hold_max", 8000), m_nlc_overwatch_hold_min);

    // gaze
    m_nlc_gaze = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_gaze", false);
    m_nlc_gaze_range = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_range", 25.f);
    m_nlc_gaze_min_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_min_dist", 4.f);
    m_nlc_gaze_cone = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_cone", 12.f);
    m_nlc_gaze_cone_full = _min(READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_cone_full", 3.f), m_nlc_gaze_cone - 0.5f);
    m_nlc_gaze_gain = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_gain", 0.6f);
    m_nlc_gaze_passive_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_passive_k", 0.25f);
    m_nlc_gaze_passive_cap = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_passive_cap", 0.5f);
    m_nlc_gaze_loss = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_loss", 0.5f);
    m_nlc_gaze_drain = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_drain", 0.15f);
    m_nlc_gaze_seize_stamina = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gaze_seize_stamina", 0.5f);
    m_nlc_gaze_window_min = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_gaze_window_min", 2000);
    m_nlc_gaze_window_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_gaze_window_max", 3000), m_nlc_gaze_window_min);
    m_nlc_gaze_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_gaze_cooldown", 8000);
    m_nlc_gaze_look_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_gaze_look_ms", 500);
    LPCSTR warn = READ_IF_EXISTS(pSettings, r_string, section, "nlc_gaze_warn_sound", READ_IF_EXISTS(pSettings, r_string, section, "scan_sound", ""));
    if (m_nlc_gaze && warn && warn[0])
        ::Sound->create(m_nlc_gaze_warn_sound, warn, st_Effect, SOUND_TYPE_WORLD);
}

void CBurer::nlc_reinit_pass5()
{
    m_nlc_npc_disarm_next = 0;
    m_nlc_npc_aim_id = u16(-1);
    m_nlc_npc_disarmed.clear();
    m_nlc_npc_drop_gun = u16(-1);
    m_nlc_npc_aiming = false;
    m_nlc_grudge_id = u16(-1);
    m_nlc_grudge_next = 0;
    m_nlc_grudge_unseen_since = 0;
    m_nlc_overwatch_settle_until = 0;
    m_nlc_shield_ends = 0;
    m_nlc_snatch_next = 0;
    m_nlc_snatch_id = u16(-1);
    m_nlc_pull_next = 0;
    m_nlc_elev = false;
    m_nlc_elev_change_since = 0;
    m_nlc_elev_unseen_since = 0;
    m_nlc_elev_pinned = false;
    m_nlc_gaze_level = 0.f;
    m_nlc_gaze_next = 0;
    m_nlc_gaze_look_since = 0;
    m_nlc_stare_until = 0;
    m_nlc_gaze_seize = false;
    m_nlc_gaze_force_until = 0;
    m_nlc_gaze_sent = 0.f;
    m_nlc_gravi_push_id = u16(-1);
    nlc_catch_drop("reinit");
}

void CBurer::nlc_pass5_update()
{
    nlc_elev_update();
    nlc_grudge_update();
    nlc_npc_track();
    nlc_snatch_update();
    nlc_catch_update();
    nlc_gaze_update();
}

//////////////////////////////////////////////////////////////////////////
// Timed casts (CStateBurerCast): the stare window, NPC disarm, pull

bool CBurer::nlc_cast_ready(u32 kind)
{
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !enemy->g_Alive())
        return false;
    const float dist = Position().distance_to(enemy->Position());

    switch (kind)
    {
    case eNlcCastStare:
        // never in the middle of a tactical move (a run to cover after a shield break)
        return m_nlc_gaze && enemy == Actor() && now >= m_nlc_gaze_next && !m_nlc_glass && !m_nlc_move_active && !get_force_anti_aim() && EnemyMan.see_enemy_now() &&
            dist >= m_nlc_gaze_min_dist && dist <= m_nlc_gaze_range && m_nlc_gaze_look_since && now >= m_nlc_gaze_look_since + m_nlc_gaze_look_ms;

    case eNlcCastNpcDisarm: {
        if (!m_nlc_npc_disarm || !get_anti_aim() || now < m_nlc_npc_disarm_next || m_nlc_npc_aim_id != enemy->ID() ||
            now < m_nlc_npc_aim_since + m_nlc_npc_disarm_aim_ms || dist > m_nlc_npc_disarm_range)
            return false;
        const auto it = m_nlc_npc_disarmed.find(enemy->ID());
        return (it == m_nlc_npc_disarmed.end() || now >= it->second) && nlc_npc_disarm_candidate(enemy);
    }

    case eNlcCastPull: {
        if (!m_nlc_pull || !m_nlc_elev || now < m_nlc_pull_next || dist > m_nlc_pull_range || !EnemyMan.see_enemy_now())
            return false;
        // the drop test casts rays: at most four times a second
        if (now >= m_nlc_pull_check_next)
        {
            m_nlc_pull_check_next = now + 250;
            m_nlc_pull_ok = nlc_pull_landing(enemy, m_nlc_pull_dir, m_nlc_pull_landing_dist);
        }
        return m_nlc_pull_ok;
    }
    }
    return false;
}

void CBurer::nlc_cast_begin(u32 kind)
{
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    m_nlc_cast_hit_done = false;
    m_nlc_cast_start = now;

    switch (kind)
    {
    case eNlcCastStare: {
        // it stops and stares: the warning sound, then the meter fills four times faster
        m_nlc_stare_until = now + nlc5_rand(m_nlc_gaze_window_min, m_nlc_gaze_window_max);
        m_nlc_gaze_next = m_nlc_stare_until + m_nlc_gaze_cooldown; // not ready again before the state is left
        m_nlc_cast_hit_done = true;
        m_nlc_cast_end = m_nlc_stare_until;
        if (m_nlc_gaze_warn_sound._handle() && Actor())
            ::Sound->play_at_pos(m_nlc_gaze_warn_sound, 0, Actor()->Position());
        nlc_log("stare window %u ms, gaze %.2f dist=%.1f", m_nlc_stare_until - now, m_nlc_gaze_level, enemy ? enemy->Position().distance_to(Position()) : -1.f);
        break;
    }
    case eNlcCastNpcDisarm: {
        const float length = anim().get_animation_length(eAnimAntiAimAbility, 0);
        float hit = anim().get_animation_hit_time(eAnimAntiAimAbility, 0);
        if (hit <= 0.f || hit >= length)
            hit = length * 0.5f;
        m_nlc_cast_hit = now + u32(hit * 1000.f);
        m_nlc_cast_end = now + u32(length * 1000.f);
        m_nlc_npc_target = enemy ? enemy->ID() : u16(-1);
        m_nlc_npc_disarm_next = now + m_nlc_npc_disarm_cooldown;
        nlc_log("npc disarm cast at %s dist=%.1f", enemy ? enemy->cName().c_str() : "-", enemy ? enemy->Position().distance_to(Position()) : -1.f);
        break;
    }
    case eNlcCastPull: {
        const float length = anim().get_animation_length(eAnimGraviFire, 0);
        m_nlc_cast_hit = now + m_nlc_pull_delay_ms;
        m_nlc_cast_end = _max(now + u32(length * 1000.f), m_nlc_cast_hit + 300);
        m_nlc_pull_next = now + m_nlc_pull_cooldown;
        sound().play(eMonsterSoundGraviAttack);
        nlc_log("pull cast: target %.1f m up, %.1f m away, a drop %.0f m out", m_nlc_elev_height, enemy ? enemy->Position().distance_to(Position()) : -1.f,
            m_nlc_pull_landing_dist);
        break;
    }
    }
}

void CBurer::nlc_cast_execute(u32 kind)
{
    face_enemy();
    if (kind == eNlcCastNpcDisarm)
        anim().set_override_animation(eAnimAntiAimAbility);
    else if (kind == eNlcCastPull)
        anim().set_override_animation(eAnimGraviFire);
    set_action(ACT_STAND_IDLE);

    if (!m_nlc_cast_hit_done && Device.dwTimeGlobal >= m_nlc_cast_hit)
    {
        m_nlc_cast_hit_done = true;
        if (kind == eNlcCastNpcDisarm)
            nlc_npc_disarm_fire();
        else if (kind == eNlcCastPull)
            nlc_pull_fire();
    }
}

bool CBurer::nlc_cast_done(u32 kind)
{
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return true;
    if (kind == eNlcCastStare)
    {
        // over: time, the seize (anti-aim takes it from here), another enemy, or the line of sight lost a moment
        if (now >= m_nlc_stare_until || get_force_anti_aim() || enemy != Actor())
            return true;
        return m_nlc_gaze_unseen_since && now >= m_nlc_gaze_unseen_since + 800;
    }
    return m_nlc_cast_hit_done && now >= m_nlc_cast_end;
}

void CBurer::nlc_cast_end(u32 kind, bool critical)
{
    if (kind == eNlcCastStare)
    {
        const u32 now = Device.dwTimeGlobal;
        if (m_nlc_stare_until)
        {
            nlc_log("stare window over after %u ms, gaze %.2f%s", now - m_nlc_cast_start, m_nlc_gaze_level, now < m_nlc_stare_until ? " (cut short)" : "");
            m_nlc_stare_until = 0;
        }
        if (m_nlc_gaze_next > now + m_nlc_gaze_cooldown)
            m_nlc_gaze_next = now + m_nlc_gaze_cooldown; // a window cut short: the cooldown counts from now
    }
}

//////////////////////////////////////////////////////////////////////////
// NPC disarm: a stalker that keeps a main weapon on the burer has it torn out of its hands

// a stalker with a gun in its hands that has the burer as its selected enemy and in sight
bool CBurer::nlc_npc_aiming_at_me(const CEntityAlive* enemy, bool primary_only) const
{
    CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(const_cast<CEntityAlive*>(enemy));
    if (!stalker || !stalker->g_Alive())
        return false;
    CWeapon* weapon = smart_cast<CWeapon*>(stalker->inventory().ActiveItem());
    if (!weapon || (primary_only && !nlc5_primary(weapon)))
        return false;
    return stalker->memory().enemy().selected() == this && stalker->memory().visual().visible_now(this);
}

// timing: a main weapon with at least nlc_npc_disarm_mag_k of its magazine and not being reloaded, so the torn-out
// gun can fire for the burer
bool CBurer::nlc_npc_disarm_candidate(const CEntityAlive* enemy) const
{
    if (!nlc_npc_aiming_at_me(enemy, true))
        return false;
    CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(const_cast<CEntityAlive*>(enemy));
    CWeapon* weapon = smart_cast<CWeapon*>(stalker->inventory().ActiveItem());
    if (weapon->GetState() == CWeapon::eReload)
        return false;
    const int need = _max(1, iCeil(float(weapon->GetAmmoMagSize()) * m_nlc_npc_disarm_mag_k));
    if (weapon->GetAmmoElapsed() < need)
        return false;

    // a stalker left with nothing usable wanders about unarmed: only one with a loaded or supplied sidearm (or other gun)
    // (with a spare pistol configured, everyone may be disarmed: the spare is given at the disarm)
    if (m_nlc_npc_disarm_need_sidearm && !m_nlc_npc_spare.size())
    {
        if (!nlc_npc_has_fallback(stalker, weapon))
        {
            if (m_nlc_npc_skip_logged != stalker->ID())
            {
                const_cast<CBurer*>(this)->m_nlc_npc_skip_logged = stalker->ID();
                const_cast<CBurer*>(this)->nlc_log("npc disarm skipped: %s has no usable sidearm", stalker->cName().c_str());
            }
            return false;
        }
    }
    return true;
}

bool CBurer::nlc_tele_skip(CPhysicsShellHolder* obj)
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    return enemy && enemy != Actor() && smart_cast<CWeapon*>(obj) != nullptr;
}

void CBurer::nlc_npc_track()
{
    if (!m_nlc_npc_disarm && m_nlc_threat_npc_k <= 0.f)
        return;
    const u32 now = Device.dwTimeGlobal;
    if (now < m_nlc_npc_track_next)
        return;
    m_nlc_npc_track_next = now + 250;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    m_nlc_npc_aiming = enemy && enemy != Actor() && nlc_npc_aiming_at_me(enemy, false);
    if (enemy && EnemyMan.see_enemy_now() && Position().distance_to(enemy->Position()) <= m_nlc_npc_disarm_range && nlc_npc_disarm_candidate(enemy))
    {
        if (m_nlc_npc_aim_id != enemy->ID())
        {
            m_nlc_npc_aim_id = enemy->ID();
            m_nlc_npc_aim_since = now;
        }
    }
    else
        m_nlc_npc_aim_id = u16(-1);

    // forget old graces now and then
    if (m_nlc_npc_disarmed.size() > 16)
    {
        for (auto it = m_nlc_npc_disarmed.begin(); it != m_nlc_npc_disarmed.end();)
            it = now >= it->second ? m_nlc_npc_disarmed.erase(it) : std::next(it);
    }
}

void CBurer::nlc_npc_disarm_fire()
{
    CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(Level().Objects.net_Find(m_nlc_npc_target));
    if (!stalker || !stalker->g_Alive() || stalker->getDestroy())
    {
        nlc_log("npc disarm: the target is gone");
        return;
    }
    CWeapon* weapon = smart_cast<CWeapon*>(stalker->inventory().ActiveItem());
    if (!nlc5_primary(weapon))
    {
        nlc_log("npc disarm: %s has no main weapon in its hands now", stalker->cName().c_str());
        return;
    }

    // flung out of its hands toward the burer (away from the stalker), nobody may take it for a while: the burer seizes it
    Fvector v;
    v.sub(Position(), stalker->Position());
    v.y = 0.f;
    v.normalize_safe();
    v.y = 0.6f;
    v.normalize_safe();
    v.mul(m_weapon_drop_velocity);
    weapon->SetActivationSpeedOverride(v);
    weapon->m_nlc_tele_lock_until = Device.dwTimeGlobal + m_nlc_npc_drop_lock_ms;
    m_nlc_npc_drop_gun = weapon->ID();
    NET_Packet P;
    CGameObject::u_EventGen(P, GE_OWNERSHIP_REJECT, stalker->ID());
    P.w_u16(weapon->ID());
    CGameObject::u_EventSend(P);

    m_nlc_npc_disarmed[stalker->ID()] = Device.dwTimeGlobal + m_nlc_npc_disarm_grace;
    if (m_nlc_npc_spare.size() && !nlc_npc_has_fallback(stalker, weapon))
        nlc_npc_give_spare(stalker);
    m_nlc_npc_aim_id = u16(-1);
    nlc_log("npc disarm %s: %s(%u) torn from its hands, ammo %d/%d, dist=%.1f", stalker->cName().c_str(), weapon->cNameSect().c_str(), weapon->ID(), weapon->GetAmmoElapsed(),
        weapon->GetAmmoMagSize(), stalker->Position().distance_to(Position()));

    // what it has left (does it fall back to a sidearm?)
    string512 left = "";
    for (CInventoryItem* item : stalker->inventory().m_all)
    {
        CWeapon* other = smart_cast<CWeapon*>(item);
        if (!other || other == weapon)
            continue;
        string128 one;
        xr_sprintf(one, "%s%s(mag %d, usable %d)", left[0] ? ", " : "", other->cNameSect().c_str(), other->GetAmmoElapsed(), other->can_kill() ? 1 : 0);
        xr_strcat(left, one);
    }
    nlc_log("npc disarm %s: weapons left: %s", stalker->cName().c_str(), left[0] ? left : "none");
}

//////////////////////////////////////////////////////////////////////////
// Grenade snatch: a grenade in the actor's hand is yanked out (dropped, no throw animation), its pin pulled, held and
// thrown back like a caught one

void CBurer::nlc_snatch_update()
{
    if (!m_nlc_snatch || !m_nlc_gren.enabled)
        return;
    const u32 now = Device.dwTimeGlobal;

    // the yanked grenade reaches the world: the pin goes, the burer holds it in front of itself
    if (m_nlc_snatch_id != u16(-1))
    {
        CGrenade* grenade = smart_cast<CGrenade*>(Level().Objects.net_Find(m_nlc_snatch_id));
        if (!grenade || grenade->getDestroy() || now > m_nlc_snatch_until)
        {
            nlc_log("snatch lost the grenade (%s)", grenade ? "it never left the hand" : "gone");
            m_nlc_snatch_id = u16(-1);
            return;
        }
        if (grenade->H_Parent() || !grenade->PPhysicsShell() || !grenade->PPhysicsShell()->isActive())
            return;
        m_nlc_snatch_id = u16(-1);
        grenade->nlc_snatch_arm(ID());
        CActor* actor = Actor();
        if (!nlc_grenade_catch(grenade, actor ? actor->ID() : u16(-1)))
        {
            nlc_log("snatch: %s(%u) armed but not held (no tele slot), it lies there", grenade->cNameSect().c_str(), grenade->ID());
            return;
        }
        if (CTelekineticObject* tele = m_nlc_gren_tele.find_object(grenade))
        {
            Fvector hold;
            hold.mad(Position(), Direction(), 1.5f);
            hold.y = Position().y + 1.6f;
            tele->m_nlc_steer = true;
            tele->m_nlc_target = hold;
            tele->m_nlc_max_speed = 9.f;
            tele->m_nlc_max_accel = 20.f;
        }
        nlc_log("snatch: %s(%u) pin pulled, held", grenade->cNameSect().c_str(), grenade->ID());
        return;
    }

    if (now < m_nlc_snatch_next || now < m_nlc_snatch_check_next)
        return;
    m_nlc_snatch_check_next = now + 500;

    CActor* actor = Actor();
    if (!actor || !actor->g_Alive() || EnemyMan.get_enemy() != actor || !EnemyMan.see_enemy_now())
        return;
    if (m_nlc_held.size() + m_nlc_held_rockets.size() >= m_nlc_gren.max_held)
        return;
    CGrenade* grenade = smart_cast<CGrenade*>(actor->inventory().ActiveItem());
    if (!grenade || grenade->getDestroy())
        return;
    const u32 state = grenade->GetState();
    const bool primed = state == CMissile::eThrowStart || state == CMissile::eReady;
    if (!primed && !(m_nlc_snatch_idle && state == CMissile::eIdle))
        return;
    const float dist = actor->Position().distance_to(Position());
    if (dist > m_nlc_snatch_range || ::Random.randF(0.f, 1.f) >= m_nlc_snatch_chance)
        return;

    // out of the hand toward the burer
    Fvector v;
    v.sub(Position(), actor->Position());
    v.y = 0.f;
    v.normalize_safe();
    v.y = 0.5f;
    v.normalize_safe();
    v.mul(m_nlc_snatch_speed);
    grenade->SetActivationSpeedOverride(v);
    NET_Packet P;
    CGameObject::u_EventGen(P, GE_OWNERSHIP_REJECT, actor->ID());
    P.w_u16(grenade->ID());
    CGameObject::u_EventSend(P);

    m_nlc_snatch_id = grenade->ID();
    m_nlc_snatch_until = now + 2000;
    m_nlc_snatch_next = now + m_nlc_snatch_cooldown;
    nlc_log("snatch %s(%u) out of the actor's hand (%s) dist=%.1f", grenade->cNameSect().c_str(), grenade->ID(), primed ? "primed" : "selected", dist);
}

//////////////////////////////////////////////////////////////////////////
// Shield bullet catch: blocked bullets stop in the air and go back when the shield drops (fall when it breaks)

void CBurer::nlc_catch_bullet(const SHit* hit)
{
    if (!m_nlc_catch_max || m_nlc_caught.size() >= m_nlc_catch_max)
        return;
    // one shotgun shell is one catch
    if (Device.dwFrame == m_nlc_catch_frame && hit->weaponID == m_nlc_catch_weapon)
        return;
    m_nlc_catch_frame = Device.dwFrame;
    m_nlc_catch_weapon = hit->weaponID;

    CWeapon* weapon = smart_cast<CWeapon*>(Level().Objects.net_Find(hit->weaponID));
    if (!weapon || weapon->m_ammoTypes.empty())
        return;
    const u32 type = weapon->m_ammoType < weapon->m_ammoTypes.size() ? weapon->m_ammoType : 0;

    NlcCaught c;
    c.ammo = weapon->m_ammoTypes[type];
    c.ammo_type = u8(type);
    c.power = hit->power;
    c.impulse = hit->impulse;
    c.speed = _max(weapon->nlc_bullet_speed(), 300.f);
    c.shooter = hit->whoID;

    Fmatrix xf;
    CParticlesPlayer::MakeXFORM(this, hit->bone(), hit->dir, hit->p_in_bone_space, xf);
    Fvector point = xf.c;
    point.mad(hit->dir, -0.35f); // stopped just short of the body
    c.offset.sub(point, Position());
    if (m_nlc_catch_particle.size())
    {
        c.ps = CParticlesObject::Create(m_nlc_catch_particle.c_str(), FALSE);
        Fmatrix m;
        m.identity();
        m.c = point;
        c.ps->UpdateParent(m, Fvector().set(0.f, 0.f, 0.f));
        c.ps->Play(false);
    }
    m_nlc_caught.push_back(c);
    nlc_log("caught bullet %u/%u %s power=%.2f", u32(m_nlc_caught.size()), m_nlc_catch_max, c.ammo.c_str(), c.power);
}

void CBurer::nlc_catch_drop(LPCSTR why)
{
    if (m_nlc_caught.empty())
        return;
    for (NlcCaught& c : m_nlc_caught)
    {
        if (c.ps)
        {
            c.ps->Stop(FALSE);
            CParticlesObject::Destroy(c.ps);
        }
    }
    nlc_log("caught bullets fall (%s): %u", why, u32(m_nlc_caught.size()));
    m_nlc_caught.clear();
    m_nlc_catch_returning = false;
    m_nlc_catch_broken = false;
}

void CBurer::nlc_catch_update()
{
    if (m_nlc_caught.empty())
    {
        if (m_nlc_catch_returning)
            nlc_log("caught bullets returned: %u", m_nlc_catch_returned);
        m_nlc_catch_returning = false;
        m_nlc_catch_broken = false;
        return;
    }

    // the bullets ride with the burer; the glow (the shield's own bullet flash) is replayed at each one
    const u32 glow_now = Device.dwTimeGlobal;
    const bool ending = m_shield_active && m_nlc_shield_ends && glow_now + 1000 >= m_nlc_shield_ends;
    for (NlcCaught& c : m_nlc_caught)
    {
        Fmatrix m;
        m.identity();
        m.c.add(Position(), c.offset);
        if (c.ps)
            c.ps->UpdateParent(m, Fvector().set(0.f, 0.f, 0.f));
        if (m_nlc_catch_glow.size() && glow_now >= c.glow_next)
        {
            c.glow_next = glow_now + (ending ? m_nlc_catch_glow_fast_ms : m_nlc_catch_glow_ms);
            CParticlesObject* glow = CParticlesObject::Create(m_nlc_catch_glow.c_str(), TRUE);
            glow->UpdateParent(m, Fvector().set(0.f, 0.f, 0.f));
            GamePersistent().ps_needtoplay.push_back(glow);
        }
    }

    if (m_shield_active)
        return;
    if (m_nlc_catch_broken && m_nlc_catch_drop_on_break)
    {
        nlc_catch_drop("shield broken");
        return;
    }

    const u32 now = Device.dwTimeGlobal;
    if (!m_nlc_catch_returning)
    {
        m_nlc_catch_returning = true;
        m_nlc_catch_returned = 0;
        m_nlc_catch_next_shot = now + m_nlc_catch_delay_ms;
        // the tell: every held bullet flares and the hold sound plays, then the volley
        for (NlcCaught& c : m_nlc_caught)
        {
            if (!m_nlc_catch_flare.size())
                break;
            Fmatrix m;
            m.identity();
            m.c.add(Position(), c.offset);
            CParticlesObject* flare = CParticlesObject::Create(m_nlc_catch_flare.c_str(), TRUE);
            flare->UpdateParent(m, Fvector().set(0.f, 0.f, 0.f));
            GamePersistent().ps_needtoplay.push_back(flare);
        }
        if (sound_tele_hold._handle())
            ::Sound->play_at_pos(sound_tele_hold, this, get_head_position(this));
        nlc_log("caught bullets: returning %u in %u ms", u32(m_nlc_caught.size()), m_nlc_catch_delay_ms);
        return;
    }
    if (now < m_nlc_catch_next_shot)
        return;

    // the current enemy if it is in sight, else the one who fired the bullet
    NlcCaught c = m_nlc_caught.back();
    const CEntityAlive* target = nullptr;
    if (EnemyMan.get_enemy() && EnemyMan.see_enemy_now())
        target = EnemyMan.get_enemy();
    else
    {
        const CEntityAlive* shooter = smart_cast<const CEntityAlive*>(Level().Objects.net_Find(c.shooter));
        if (shooter && shooter != this && shooter->g_Alive() && memory().visual().visible_now(shooter))
            target = shooter;
    }
    if (!target)
    {
        nlc_catch_drop("no target");
        return;
    }

    m_nlc_caught.pop_back();
    Fvector from;
    from.add(Position(), c.offset);
    Fvector aim;
    target->Center(aim);
    aim = nlc_lead_point(target, aim, from.distance_to(aim) / c.speed, 1.f);
    Fvector dir;
    dir.sub(aim, from);
    dir.normalize_safe();
    Fvector shot;
    shot.random_dir(dir, deg2rad(m_nlc_catch_disp));

    CCartridge cartridge;
    cartridge.Load(c.ammo.c_str(), c.ammo_type);
    const float power = c.power / _max(cartridge.m_kHit, 0.01f) * m_nlc_catch_return_k;
    Level().BulletManager().AddBullet(from, shot, c.speed, power, c.impulse, ID(), ID(), ALife::eHitTypeFireWound, 150.f, cartridge, true, false);
    if (!m_nlc_catch_returned)
        sound().play(eMonsterSoundTeleAttack);
    ++m_nlc_catch_returned;

    if (c.ps)
    {
        c.ps->Stop(FALSE);
        CParticlesObject::Destroy(c.ps);
    }
    m_nlc_catch_next_shot = now + nlc5_rand(m_nlc_catch_interval_min, m_nlc_catch_interval_max);
}

//////////////////////////////////////////////////////////////////////////
// Pull: a target up on a roof or ledge is yanked toward the burer, over the edge

bool CBurer::nlc_pull_landing(const CEntityAlive* enemy, Fvector& dir, float& landing) const
{
    dir.sub(Position(), enemy->Position());
    dir.y = 0.f;
    const float len = dir.magnitude();
    if (len < 1.f)
        return false;
    dir.div(len);

    // nothing solid right in front of the target at chest height (a parapet or a wall stops the pull)
    const Fvector from = enemy->Position();
    Fvector chest = from;
    chest.y += 1.f;
    collide::rq_result rq;
    if (Level().ObjectSpace.RayPick(chest, dir, 1.5f, collide::rqtStatic, rq, nullptr))
        return false;

    // walkable ground well below the target 2-6 m out toward the burer: a drop, not more roof
    const CLevelGraph& graph = ai().level_graph();
    for (float d = 2.f; d <= 6.f; d += 1.f)
    {
        Fvector p;
        p.mad(from, dir, d);
        const u32 node = graph.vertex_id(p);
        if (!graph.valid_vertex_id(node) || graph.vertex_position(node).distance_to_xz(p) > 1.2f)
            continue;
        if (from.y - graph.vertex_plane_y(node, p.x, p.z) < 1.5f)
            continue;
        landing = d;
        return true;
    }
    return false;
}

void CBurer::nlc_pull_fire()
{
    CEntityAlive* enemy = const_cast<CEntityAlive*>(EnemyMan.get_enemy());
    if (!enemy || !enemy->g_Alive() || !EnemyMan.see_enemy_now())
    {
        nlc_log("pull lost the target");
        return;
    }
    Fvector dir;
    float landing = 0.f;
    if (!nlc_pull_landing(enemy, dir, landing))
    {
        nlc_log("pull cancelled: no drop in front of the target any more");
        return;
    }
    nlc_push(enemy, dir, m_nlc_pull_speed, m_nlc_pull_up, m_nlc_pull_ms, "pull");
    if (enemy == Actor() && m_nlc_pull_tug > 0.f)
        nlc_start_wrench(::Random.randF(-0.5f, 0.5f) * m_nlc_pull_tug, -m_nlc_pull_tug, 260, 2.f, "pull");
    nlc_log("pull %s: %.1f m up, %.1f m away, drop %.0f m out", enemy->cName().c_str(), m_nlc_elev_height, Position().distance_to(enemy->Position()), landing);
}

//////////////////////////////////////////////////////////////////////////
// Elevation: an enemy up on a roof is pinned in memory (never given up while there is contact) and watched from a
// point with a line of sight

void CBurer::nlc_elev_update()
{
    if (!m_nlc_elev_enabled)
        return;
    const u32 now = Device.dwTimeGlobal;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (enemy && m_nlc_elev)
    {
        if (EnemyMan.see_enemy_now())
            m_nlc_elev_unseen_since = 0;
        else if (!m_nlc_elev_unseen_since)
            m_nlc_elev_unseen_since = now;
    }

    if (now < m_nlc_elev_next)
        return;
    m_nlc_elev_next = now + 200;

    bool up = false;
    float height = 0.f;
    if (enemy && enemy->g_Alive() && !enemy->getDestroy())
    {
        const u32 vertex = enemy->ai_location().level_vertex_id();
        if (ai().level_graph().valid_vertex_id(vertex))
        {
            const Fvector& p = enemy->Position();
            height = p.y - ai().level_graph().vertex_plane_y(vertex, p.x, p.z);
            up = height >= m_nlc_elev_min_height;
        }
    }
    m_nlc_elev_height = height;

    // a second either way before it counts (jumps, stairs)
    if (up != m_nlc_elev)
    {
        if (!m_nlc_elev_change_since)
            m_nlc_elev_change_since = now;
        else if (now >= m_nlc_elev_change_since + 1000)
        {
            m_nlc_elev = up;
            m_nlc_elev_change_since = 0;
            m_nlc_elev_unseen_since = 0;
            if (up)
                nlc_log("enemy elevated: %.1f m above the ground under it", height);
            else
            {
                nlc_log("enemy %s", enemy ? "back on the ground" : "gone");
                if (m_nlc_elev_pinned)
                {
                    EnemyMemory.unpin();
                    m_nlc_elev_pinned = false;
                }
            }
        }
    }
    else
        m_nlc_elev_change_since = 0;

    if (!m_nlc_elev || !enemy)
        return;

    // never gives up while it still has contact: sight, sound, a hit or the psy sense renew the pin
    TTime contact = EnemyMan.get_enemy_time_last_seen();
    if (enemy == Actor())
    {
        const CVisualMemoryManager::NlcSense& sense = memory().visual().m_nlc_sense;
        if (sense.notice > 0.f && sense.level >= sense.notice)
            contact = now;
    }
    EnemyMemory.pin(enemy, contact + m_nlc_elev_patience);
    m_nlc_elev_pinned = true;

    // holding an overwatch point that lost the line of sight: look for another
    if (m_nlc_elev_unseen_since && now >= m_nlc_elev_unseen_since + 2000 && now >= m_nlc_overwatch_settle_until && now < m_nlc_hold_until &&
        !xr_strcmp(m_nlc_move_kind, "overwatch"))
    {
        m_nlc_hold_until = 0;
        nlc_log("overwatch lost the line of sight");
    }
}

bool CBurer::nlc_overwatch_pick()
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return false;
    const u32 now = Device.dwTimeGlobal;
    // in sight: it stays and casts (pull, tele, puppet gun); out of sight a moment: a point with a line of sight
    if (EnemyMan.see_enemy_now() || !m_nlc_elev_unseen_since || now < m_nlc_elev_unseen_since + 1500)
        return false;

    const CLevelGraph& graph = ai().level_graph();
    const u32 enemy_vertex = enemy->ai_location().level_vertex_id();
    if (!graph.valid_vertex_id(enemy_vertex))
        return false;
    // the ring is around the enemy himself (on a roof his ai-map vertex can be 20 m away), at ground height
    Fvector anchor = enemy->Position();
    anchor.y = graph.vertex_position(enemy_vertex).y;

    float best = flt_max;
    Fvector best_pos{};
    u32 best_node = u32(-1);
    u32 tried = 0, seeing = 0;
    const float start = ::Random.randF(0.f, PI_MUL_2);
    const float step = (m_nlc_overwatch_max - m_nlc_overwatch_min) * 0.5f;
    for (int ring = 0; ring < 3; ++ring)
    {
        const float radius = m_nlc_overwatch_min + step * float(ring);
        for (int i = 0; i < 12; ++i)
        {
            const float angle = start + float(i) * (PI_MUL_2 / 12.f);
            Fvector c;
            c.set(anchor.x + _cos(angle) * radius, anchor.y, anchor.z + _sin(angle) * radius);
            const u32 node = graph.vertex_id(c);
            if (!graph.valid_vertex_id(node))
                continue;
            c = graph.vertex_position(node);
            const float ring = c.distance_to_xz(enemy->Position());
            if (_abs(c.y - anchor.y) > 6.f || ring < m_nlc_overwatch_min - 2.f || ring > m_nlc_overwatch_max + 2.f || !movement().restrictions().accessible(c))
                continue; // another floor, off the ring, or out of bounds
            ++tried;
            if (!nlc_los_to(c, enemy, 1.7f))
                continue;
            ++seeing;
            const float d = c.distance_to(Position());
            if (d < 3.f)
                continue; // a step to the side changes nothing: it turns instead
            if (d < best)
            {
                best = d;
                best_pos = c;
                best_node = node;
            }
        }
    }

    if (best_node != u32(-1))
    {
        nlc_log("overwatch: %u of %u points see the enemy (%.1f m up), the nearest is %.1f m away", seeing, tried, m_nlc_elev_height, best);
        return nlc_set_move(best_pos, best_node, true, "overwatch");
    }

    // no line of sight anywhere: wait hidden near the building and keep sensing
    nlc_log("overwatch: none of %u points sees the enemy (%.1f m up), hiding nearby", tried, m_nlc_elev_height);
    if (CoverMan)
    {
        const CCoverPoint* point = CoverMan->find_cover(enemy->Position(), 6.f, 16.f);
        if (point && nlc_set_move(point->position(), point->level_vertex_id(), false, "siege hide"))
            return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// Gaze: mutual sight fills a meter; in the stare window (CStateBurerCast) four times faster; full in the window = the
// stamina attack (anti-aim animation, StaminaHit) or a stamina hit without a weapon in the hands

void CBurer::nlc_gaze_update()
{
    if (!m_nlc_gaze)
        return;
    const u32 now = Device.dwTimeGlobal;
    const float dt = Device.fTimeDelta;
    CActor* actor = Actor();

    // af_glass_new blocks it (the script owns the check)
    if (now >= m_nlc_glass_check_next)
    {
        m_nlc_glass_check_next = now + 1000;
        luabind::functor<bool> func_glass;
        m_nlc_glass = ai().script_engine().functor("bind_monster.handle_glass_new_on_belt", func_glass) && func_glass();
    }

    // a forced stamina attack that never started (state changed, enemy lost): drop it
    if (m_nlc_gaze_force_until && now >= m_nlc_gaze_force_until)
    {
        m_nlc_gaze_force_until = 0;
        if (get_force_anti_aim())
        {
            set_force_anti_aim(false);
            m_nlc_gaze_seize = false;
            nlc_log("gaze seize expired: the stamina attack did not start");
        }
    }

    bool contact = false, visible = false;
    float k = 0.f;
    if (actor && actor->g_Alive() && EnemyMan.get_enemy() == actor && !m_nlc_glass)
    {
        visible = memory().visual().visible_now(actor);
        const float dist = Position().distance_to(actor->Position());
        if (visible && dist <= m_nlc_gaze_range)
        {
            // the actor looks at its head ...
            const Fvector& eye = actor->cam_Active()->vPosition;
            const Fvector& look = actor->cam_Active()->vDirection;
            Fvector to;
            to.sub(get_head_position(this), eye);
            to.normalize_safe();
            const float angle = rad2deg(acosf(clampr(to.dotproduct(look), -1.f, 1.f)));
            float angle_k = angle <= m_nlc_gaze_cone_full ? 1.f : 1.f - (angle - m_nlc_gaze_cone_full) / (m_nlc_gaze_cone - m_nlc_gaze_cone_full);
            clamp(angle_k, 0.f, 1.f);
            // ... and it looks back
            Fvector self2actor;
            self2actor.sub(actor->Position(), Position());
            const bool facing = angle_between_vectors(self2actor, Direction()) < deg2rad(60.f);
            if (angle_k > 0.5f)
            {
                if (!m_nlc_gaze_look_since)
                    m_nlc_gaze_look_since = now;
            }
            else
                m_nlc_gaze_look_since = 0;
            if (angle_k > 0.f && facing)
            {
                contact = true;
                const float dist_k = dist <= 10.f ? 1.f : 1.f - 0.5f * _min((dist - 10.f) / _max(m_nlc_gaze_range - 10.f, 1.f), 1.f);
                k = angle_k * dist_k;
            }
        }
        else
            m_nlc_gaze_look_since = 0;
    }
    else
        m_nlc_gaze_look_since = 0;

    if (visible)
        m_nlc_gaze_unseen_since = 0;
    else if (!m_nlc_gaze_unseen_since)
        m_nlc_gaze_unseen_since = now;

    const bool window = now < m_nlc_stare_until;
    const float prev = m_nlc_gaze_level;
    if (contact)
    {
        m_nlc_gaze_level += m_nlc_gaze_gain * k * nlc_phase_k(1.f, 1.25f, 1.5f) * (window ? 1.f : m_nlc_gaze_passive_k) * dt;
        // outside the window aiming costs almost nothing: it only primes the meter
        if (!window && m_nlc_gaze_level > m_nlc_gaze_passive_cap)
            m_nlc_gaze_level = _max(m_nlc_gaze_passive_cap, prev - m_nlc_gaze_loss * dt);
    }
    else
        m_nlc_gaze_level -= m_nlc_gaze_loss * (visible ? 1.f : 2.f) * dt;
    clamp(m_nlc_gaze_level, 0.f, 1.f);

    // logged when it crosses a quarter
    const float band = floorf(m_nlc_gaze_level * 4.f) * 0.25f;
    if (!fsimilar(band, m_nlc_gaze_logged_band))
    {
        if (band > m_nlc_gaze_logged_band)
            nlc_log("gaze %.2f%s", m_nlc_gaze_level, window ? " (stare window)" : "");
        m_nlc_gaze_logged_band = band;
    }

    // a gentle stamina drain past the half
    if (actor && actor->g_Alive() && m_nlc_gaze_level > 0.5f && m_nlc_gaze_drain > 0.f)
        actor->conditions().PowerHit(m_nlc_gaze_drain * (m_nlc_gaze_level - 0.5f) * 2.f * dt, false);

    if (window && m_nlc_gaze_level >= 1.f)
        nlc_gaze_seize();

    // the script turns the meter into the screen effect and the whispers (bind_monster.burer_gaze)
    if (now >= m_nlc_gaze_script_next && (m_nlc_gaze_level > 0.005f || m_nlc_gaze_sent > 0.f))
    {
        m_nlc_gaze_script_next = now + 100;
        nlc_gaze_send(m_nlc_gaze_level, window);
    }
}

void CBurer::nlc_gaze_send(float level, bool window)
{
    luabind::functor<void> func_gaze;
    if (ai().script_engine().functor("bind_monster.burer_gaze", func_gaze))
        func_gaze(ID(), level, window);
    m_nlc_gaze_sent = level > 0.005f ? level : 0.f;
}

void CBurer::nlc_gaze_seize()
{
    const u32 now = Device.dwTimeGlobal;
    CActor* actor = Actor();
    m_nlc_gaze_level = 0.3f;
    m_nlc_gaze_logged_band = 0.25f;
    m_nlc_gaze_next = now + m_nlc_gaze_cooldown;
    m_nlc_stare_until = 0;

    CWeapon* weapon = actor ? smart_cast<CWeapon*>(actor->inventory().ActiveItem()) : nullptr;
    if (weapon && get_anti_aim())
    {
        // the stock stamina attack (animation, camera effector, StaminaHit with the gaze's extra stamina hit)
        m_nlc_gaze_seize = true;
        m_nlc_gaze_force_until = now + 2000;
        set_force_anti_aim(true);
        nlc_log("gaze seize: stamina attack, %s in the hands", weapon->cNameSect().c_str());
        return;
    }
    if (actor)
    {
        actor->conditions().PowerHit(m_nlc_gaze_seize_stamina, false);
        luabind::functor<void> func_seize;
        if (ai().script_engine().functor("bind_monster.burer_on_gaze_seize", func_seize))
            func_seize(ID());
    }
    nlc_log("gaze seize: nothing to drop, stamina only");
}

//////////////////////////////////////////////////////////////////////////
// Barricade grip: a corpse held by one bone (experimental)

CPhysicsElement* CBurer::nlc_grip_element(CPhysicsShellHolder* obj, const xr_vector<shared_str>& bones)
{
    IKinematics* kinematics = smart_cast<IKinematics*>(obj->Visual());
    if (!kinematics || !obj->m_pPhysicsShell)
        return nullptr;
    for (const shared_str& name : bones)
    {
        u16 bone = kinematics->LL_BoneID(name);
        // bones without an own element belong to their parent's
        while (bone != BI_NONE)
        {
            if (CPhysicsElement* element = obj->m_pPhysicsShell->get_Element(bone))
                return element;
            bone = kinematics->GetBoneData(bone).GetParentID();
        }
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Grudge: an enemy that hurt it in the last nlc_grudge_ms stays in memory (no rest / eating while it hides); unseen for
// 3 s the brain peeks for a line of sight even with nothing ready

void CBurer::nlc_grudge_update()
{
    if (!m_nlc_grudge_ms)
        return;
    const u32 now = Device.dwTimeGlobal;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (enemy && enemy->ID() == m_nlc_grudge_id)
    {
        if (EnemyMan.see_enemy_now())
            m_nlc_grudge_unseen_since = 0;
        else if (!m_nlc_grudge_unseen_since)
            m_nlc_grudge_unseen_since = now;
    }

    if (now < m_nlc_grudge_next)
        return;
    m_nlc_grudge_next = now + 500;

    if (m_nlc_elev_pinned || !HitMemory.is_hit())
        return; // the elevation pin owns the memory lock
    const CEntityAlive* who = smart_cast<const CEntityAlive*>(HitMemory.get_last_hit_object());
    const TTime hit_time = HitMemory.get_last_hit_time();
    if (!who || who == this || !who->g_Alive() || who->getDestroy() || now >= hit_time + m_nlc_grudge_ms)
    {
        if (m_nlc_grudge_id != u16(-1))
        {
            nlc_log("grudge over");
            m_nlc_grudge_id = u16(-1);
            m_nlc_grudge_unseen_since = 0;
        }
        return;
    }
    // only an enemy it already remembers (a hit from a friend or a stray never makes one)
    if (EnemyMemory.get_danger(who) < 0.f)
        return;
    EnemyMemory.pin(who, hit_time + m_nlc_grudge_ms);
    if (m_nlc_grudge_id != who->ID())
    {
        m_nlc_grudge_id = who->ID();
        m_nlc_grudge_unseen_since = 0;
        nlc_log("grudge against %s for %u s", who->cName().c_str(), m_nlc_grudge_ms / 1000);
    }
}

bool CBurer::nlc_grudge_hunt() const
{
    if (m_nlc_grudge_id != u16(-1) && m_nlc_grudge_unseen_since && Device.dwTimeGlobal >= m_nlc_grudge_unseen_since + 3000 &&
        Device.dwTimeGlobal >= m_nlc_grudge_log_next)
    {
        CBurer* self = const_cast<CBurer*>(this);
        self->m_nlc_grudge_log_next = Device.dwTimeGlobal + 10000;
        self->nlc_log("grudge: hunting, enemy unseen %.1f s", float(Device.dwTimeGlobal - m_nlc_grudge_unseen_since) * 0.001f);
    }
    return m_nlc_grudge_id != u16(-1) && m_nlc_grudge_unseen_since && Device.dwTimeGlobal >= m_nlc_grudge_unseen_since + 3000;
}

//////////////////////////////////////////////////////////////////////////
// Spare pistol: a stalker disarmed without another gun it can fire gets a worn pistol and a box (docs/BURER_REDESIGN.md 5.19)

bool CBurer::nlc_npc_has_fallback(const CEntityAlive* entity, const CWeapon* except) const
{
    CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(const_cast<CEntityAlive*>(entity));
    if (!stalker)
        return false;
    for (CInventoryItem* item : stalker->inventory().m_all)
    {
        CWeapon* other = smart_cast<CWeapon*>(item);
        if (!other || other == except)
            continue;
        const u32 type = other->ef_weapon_type();
        if (type >= 5 && type <= 10 && other->can_kill())
            return true;
    }
    return false;
}

void CBurer::nlc_npc_give_spare(CEntityAlive* stalker)
{
    const u32 vertex = stalker->ai_location().level_vertex_id();
    CSE_Abstract* abstract = Level().spawn_item(m_nlc_npc_spare.c_str(), stalker->Position(), vertex, stalker->ID(), true);
    if (!abstract)
        return;
    const float condition = ::Random.randF(m_nlc_npc_spare_cond_min, m_nlc_npc_spare_cond_max);
    if (CSE_ALifeInventoryItem* item = smart_cast<CSE_ALifeInventoryItem*>(abstract))
        item->m_fCondition = condition;
    NET_Packet P;
    abstract->Spawn_Write(P, TRUE);
    Level().Send(P, net_flags(TRUE));
    F_entity_Destroy(abstract);

    if (m_nlc_npc_spare_ammo.size() && m_nlc_npc_spare_rounds)
    {
        CSE_Abstract* box = Level().spawn_item(m_nlc_npc_spare_ammo.c_str(), stalker->Position(), vertex, stalker->ID(), true);
        if (box)
        {
            if (CSE_ALifeItemAmmo* ammo = smart_cast<CSE_ALifeItemAmmo*>(box))
                ammo->m_boxSize = m_nlc_npc_spare_rounds;
            NET_Packet A;
            box->Spawn_Write(A, TRUE);
            Level().Send(A, net_flags(TRUE));
            F_entity_Destroy(box);
        }
    }
    nlc_log("npc disarm: spare %s (condition %.2f, +%u %s) for %s", m_nlc_npc_spare.c_str(), condition, u32(m_nlc_npc_spare_rounds),
        m_nlc_npc_spare_ammo.size() ? m_nlc_npc_spare_ammo.c_str() : "-", stalker->cName().c_str());
}
