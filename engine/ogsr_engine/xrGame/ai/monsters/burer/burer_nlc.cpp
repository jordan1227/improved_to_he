// NLC burer pass 2-4 (docs/BURER_REDESIGN.md 3.1, 3.2, 4): puppet gun, threat shield, camera wrench, shield strain,
// passive deflection, barricade selection, brain helpers. Pass 1 pieces (grenades, repulse, scan) stay in burer.cpp.
#include "stdafx.h"
#include "burer.h"
#include "../../../level.h"
#include "../../../actor.h"
#include "../../../ActorCondition.h"
#include "../../../Inventory.h"
#include "../../../WeaponMagazined.h"
#include "../../../WeaponMagazinedWGrenade.h"
#include "../../../WeaponKnife.h"
#include "../../../WeaponBinoculars.h"
#include "../../../WeaponRPG7.h"
#include "../../../WeaponRG6.h"
#include "../../../PhysicsShell.h"
#include "../../../PhysicObject.h"
#include "../../../memory_manager.h"
#include "../../../visual_memory_manager.h"
#include "../../../ActorEffector.h"
#include "../../../sound_player.h"
#include "../../../ai_space.h"
#include "../../../ai_object_location.h"
#include "../../../movement_manager.h"
#include "../../../restricted_object.h"
#include "../../../EntityCondition.h"
#include "../../../GamePersistent.h"
#include "../../../../xr_3da/CameraBase.h"
#include "level_graph.h"
#include "cover_point.h"
#include "../monster_cover_manager.h"
#include "../ai_monster_utils.h"
#include "../ai_monster_effector.h"
#include "../telekinesis_hit_callback.h"
#include "../../../nlc_stealth.h"
#include "../telekinetic_object.h"

namespace
{
void nlc_read_list(LPCSTR section, LPCSTR key, xr_vector<shared_str>& out)
{
    out.clear();
    if (!pSettings->line_exist(section, key))
        return;
    LPCSTR value = pSettings->r_string(section, key);
    const int count = _GetItemCount(value);
    string256 item;
    for (int i = 0; i < count; ++i)
    {
        LPCSTR name = _Trim(_GetItem(value, i, item));
        if (name && name[0])
            out.push_back(name);
    }
}

bool nlc_in_list(const xr_vector<shared_str>& list, const shared_str& value) { return std::find(list.begin(), list.end(), value) != list.end(); }

float nlc_smooth(float u)
{
    clamp(u, 0.f, 1.f);
    return u * u * (3.f - 2.f * u);
}
} // namespace

void CBurer::nlc_load_pass2(LPCSTR section)
{
    // defenses
    m_nlc_threat_gain = READ_IF_EXISTS(pSettings, r_float, section, "nlc_threat_gain", 2.5f);
    m_nlc_threat_loss = READ_IF_EXISTS(pSettings, r_float, section, "nlc_threat_loss", 1.5f);
    m_nlc_threat_radius = READ_IF_EXISTS(pSettings, r_float, section, "nlc_threat_radius", 0.7f);
    m_nlc_threat_shield = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_threat_shield", false);
    m_nlc_threat_shield_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_threat_shield_ms", 2500);
    m_nlc_shield_strain_hp = READ_IF_EXISTS(pSettings, r_float, section, "nlc_shield_strain_hp", 0.f);
    m_nlc_shield_leak_start = READ_IF_EXISTS(pSettings, r_float, section, "nlc_shield_leak_start", 0.5f);
    m_nlc_shield_leak_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_shield_leak_max", 1.f);
    m_nlc_shield_break_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_shield_break_cooldown", 8000);
    m_nlc_break_stamina = READ_IF_EXISTS(pSettings, r_float, section, "nlc_shield_break_stamina", 0.35f);
    m_nlc_break_shake = READ_IF_EXISTS(pSettings, r_float, section, "nlc_shield_break_shake", 20.f);
    m_nlc_deflect_chance = READ_IF_EXISTS(pSettings, r_float, section, "nlc_deflect_chance", 0.f);
    m_nlc_wrench = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_wrench", false);
    m_nlc_wrench_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_wrench_cooldown", 15000);
    m_nlc_wrench_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_wrench_min", 90.f);
    m_nlc_wrench_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_wrench_max", 180.f);
    m_nlc_wrench_pitch = READ_IF_EXISTS(pSettings, r_float, section, "nlc_wrench_pitch", 15.f);
    m_nlc_wrench_ms_min = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_wrench_ms_min", 250), u32(80));
    m_nlc_wrench_ms_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_wrench_ms_max", 450), m_nlc_wrench_ms_min);

    // barricade
    m_nlc_barricade = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_barricade", false);
    m_nlc_barricade_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_dist", 1.6f);
    m_nlc_barricade_height = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_height", 1.1f);
    m_nlc_barricade_min_size = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_min_size", 0.8f);
    m_nlc_barricade_min_volume = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_min_volume", 0.12f);
    m_nlc_barricade_mass_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_mass_min", 20.f);
    m_nlc_barricade_mass_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_mass_max", 400.f);
    m_nlc_barricade_mode = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_barricade_mode", 1);
    m_nlc_barricade_max_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_barricade_max_ms", 12000);
    m_nlc_barricade_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_barricade_cooldown", 4000);
    m_nlc_barricade_speed = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_speed", 9.f);
    m_nlc_barricade_leash = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_leash", 8.f);
    m_nlc_barricade_gain = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_gain", 5.f);
    m_nlc_barricade_accel = READ_IF_EXISTS(pSettings, r_float, section, "nlc_barricade_accel", 20.f);
    m_nlc_tele_weapon_speed_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_tele_weapon_speed_k", 1.6f);
    m_nlc_puppet_blind_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_blind_ms", 8000);
    nlc_read_list(section, "nlc_barricade_allow", m_nlc_barricade_allow);
    nlc_read_list(section, "nlc_barricade_deny", m_nlc_barricade_deny);
    nlc_read_list(section, "nlc_barricade_corpse_species", m_nlc_corpse_species);
    m_nlc_tele_vs_npc = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_tele_vs_npc", false);

    // puppet gun
    m_nlc_puppet_enabled = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_puppet", false);
    m_nlc_puppet_radius = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_radius", 20.f);
    m_nlc_puppet_max_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_max_dist", 30.f);
    m_nlc_puppet_seize_delay = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_seize_delay", 1500);
    m_nlc_puppet_settle_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_settle_ms", 900);
    m_nlc_puppet_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_cooldown", 4000);
    m_nlc_puppet_max_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_max_ms", 25000);
    m_nlc_puppet_hit_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_hit_k", 1.f);
    m_nlc_puppet_disp_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_disp_k", 2.f);
    m_nlc_puppet_blind_disp_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_blind_disp_k", 1.6f);
    m_nlc_puppet_fire_cone = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_fire_cone", 4.f);
    m_nlc_puppet_recoil = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_recoil", 0.06f);
    m_nlc_puppet_break_damage = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_break_damage", 0.05f);
    m_nlc_puppet_keep_away_chance = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_keep_away_chance", 0.5f);
    m_nlc_puppet_keep_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_keep_dist", 5.f);
    m_nlc_puppet_drift_speed = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_drift_speed", 3.f);
    m_nlc_puppet_hover_height = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_hover_height", 2.f);
    m_nlc_puppet_lock_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_lock_ms", 1500);
    m_nlc_puppet_burst_min = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_burst_min", 2), u32(1));
    m_nlc_puppet_burst_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_burst_max", 5), m_nlc_puppet_burst_min);
    m_nlc_puppet_head_aim = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_puppet_head_aim", false);
    m_nlc_puppet_bob = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_bob", 0.06f);
    m_nlc_puppet_tremor = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_tremor", 0.015f);
    m_nlc_puppet_jitter = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_jitter", 1.f);
    m_nlc_puppet_ubgl = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_puppet_ubgl", false);
    m_nlc_puppet_ubgl_min = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_ubgl_min_ms", 6000);
    m_nlc_puppet_ubgl_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_ubgl_max_ms", 8000), m_nlc_puppet_ubgl_min);
    m_nlc_puppet_ubgl_min_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_ubgl_min_dist", 10.f);
    m_nlc_puppet_jam_cap = READ_IF_EXISTS(pSettings, r_float, section, "nlc_puppet_jam_cap", 0.03f);
    m_nlc_puppet_unjam_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_unjam_ms", 3000);
    m_nlc_puppet_hover_commit = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_hover_commit", 2500);
    m_nlc_puppet_los_grace = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_puppet_los_grace", 600);

    // brain
    m_nlc_brain = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_brain", false);
    m_nlc_phase_wounded = READ_IF_EXISTS(pSettings, r_float, section, "nlc_phase_wounded", 0.6f);
    m_nlc_phase_desperate = READ_IF_EXISTS(pSettings, r_float, section, "nlc_phase_desperate", 0.3f);
    m_nlc_engage_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_engage_max", 25.f);
    m_nlc_cover_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_cover_min", 8.f);
    m_nlc_cover_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_cover_max", 22.f);
    m_nlc_cover_hold_min = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_cover_hold_min", 2000);
    m_nlc_cover_hold_max = _max(READ_IF_EXISTS(pSettings, r_u32, section, "nlc_cover_hold_max", 4000), m_nlc_cover_hold_min);
    m_nlc_cover_retry = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_cover_retry", 6000);
}

void CBurer::nlc_reinit_pass2()
{
    m_nlc_threat = 0.f;
    m_nlc_shield_next = 0;
    m_nlc_overlay_shield_until = 0;
    m_nlc_shield_strain = 0.f;
    m_nlc_wrench_next = 0;
    m_nlc_wr = NlcWrench();
    m_nlc_puppet = NlcPuppet();
    m_nlc_puppet_next = 0;
    m_nlc_puppet_candidate = u16(-1);
    m_nlc_logged_phase = u32(-1);
    m_nlc_move_next = 0;
    m_nlc_hold_until = 0;
    m_nlc_move_active = false;
    m_nlc_last_cover_move = 0;
    m_nlc_advance_straight_until = 0;
    m_nlc_force_cover = false;
    m_nlc_busy = false;
    m_nlc_bar_next = 0;
}

//////////////////////////////////////////////////////////////////////////
// Camera wrench: eased turn with a small overshoot, a pitch nudge and a decaying shake

void CBurer::nlc_start_wrench(float yaw_deg, float pitch_deg, u32 ms, float shake_deg, LPCSTR why)
{
    if (m_nlc_wr.active)
        return;
    m_nlc_wr = NlcWrench();
    m_nlc_wr.active = true;
    m_nlc_wr.start = Device.dwTimeGlobal;
    m_nlc_wr.dur = _max(ms, u32(80));
    m_nlc_wr.yaw_total = deg2rad(yaw_deg);
    m_nlc_wr.pitch_total = deg2rad(pitch_deg);
    m_nlc_wr.shake = deg2rad(shake_deg);
    nlc_log("camera wrench (%s) yaw=%.0f pitch=%.0f deg in %u ms", why, yaw_deg, pitch_deg, m_nlc_wr.dur);
}

void CBurer::nlc_wrench_update()
{
    if (!m_nlc_wr.active)
        return;
    CActor* actor = Actor();
    if (!actor || !actor->g_Alive())
    {
        m_nlc_wr.active = false;
        return;
    }

    const float settle = 0.35f; // share of the duration spent coming back from the overshoot
    const float overshoot = 0.08f;
    const float u = float(Device.dwTimeGlobal - m_nlc_wr.start) / float(m_nlc_wr.dur);
    const bool done = u >= 1.f + settle;
    float f;
    if (done)
        f = 1.f;
    else if (u < 1.f)
        f = nlc_smooth(u) * (1.f + overshoot);
    else
        f = (1.f + overshoot) - overshoot * nlc_smooth((u - 1.f) / settle);

    // a zero-mean shake that fades out with the turn
    const float shake_k = done ? 0.f : m_nlc_wr.shake * (1.f - _min(u / (1.f + settle), 1.f));
    const float shake_yaw = ::Random.randF(-1.f, 1.f) * shake_k;
    const float shake_pitch = ::Random.randF(-1.f, 1.f) * shake_k;

    const float yaw = f * m_nlc_wr.yaw_total;
    const float pitch = f * m_nlc_wr.pitch_total;
    CCameraBase* camera = actor->cam_Active();
    camera->yaw += (yaw - m_nlc_wr.yaw_applied) + (shake_yaw - m_nlc_wr.shake_yaw_prev);
    camera->pitch += (pitch - m_nlc_wr.pitch_applied) + (shake_pitch - m_nlc_wr.shake_pitch_prev);
    m_nlc_wr.yaw_applied = yaw;
    m_nlc_wr.pitch_applied = pitch;
    m_nlc_wr.shake_yaw_prev = shake_yaw;
    m_nlc_wr.shake_pitch_prev = shake_pitch;
    if (done)
        m_nlc_wr.active = false;
}

//////////////////////////////////////////////////////////////////////////
// Defenses

void CBurer::nlc_defense_update()
{
    const u32 now = Device.dwTimeGlobal;
    const float dt = Device.fTimeDelta;
    CActor* actor = Actor();

    nlc_wrench_update();

    // threat shield running out (time, or the actor reloads: the stock shield drops then too)
    if (m_nlc_overlay_shield_until)
    {
        if (now >= m_nlc_overlay_shield_until)
            nlc_overlay_shield_stop("time");
        else if (actor_is_reloading_weapon())
            nlc_overlay_shield_stop("actor reloads");
        else if (m_shield_keep_particle && now >= m_nlc_overlay_particle_next)
        {
            CParticlesPlayer::StartParticles(m_shield_keep_particle, Fvector().set(0, 1, 0), ID(), -1, true);
            m_nlc_overlay_particle_next = now + m_shield_keep_particle_period;
        }
    }

    // threat meter: the actor's crosshair on the burer with a loaded gun
    bool aimed = false;
    if (actor && actor->g_Alive() && EnemyMan.get_enemy() == actor)
    {
        CWeapon* weapon = smart_cast<CWeapon*>(actor->inventory().ActiveItem());
        if (weapon && !smart_cast<CWeaponKnife*>(weapon) && !smart_cast<CWeaponBinoculars*>(weapon) && weapon->GetAmmoElapsed() > 0)
        {
            const Fvector& eye = actor->cam_Active()->vPosition;
            const Fvector& look = actor->cam_Active()->vDirection;
            Fvector center;
            Center(center);
            Fvector to;
            to.sub(center, eye);
            const float along = to.dotproduct(look);
            if (along > 0.f && along < 80.f)
            {
                Fvector closest;
                closest.mad(eye, look, along);
                aimed = closest.distance_to(center) < m_nlc_threat_radius;
            }
        }
    }
    // a stalker with a gun on it fills the meter too (slower: NPCs aim all the time); nlc_npc_track samples it
    float gain = m_nlc_threat_gain * nlc_phase_k(1.f, 1.5f, 2.f);
    if (!aimed && m_nlc_npc_aiming && EnemyMan.get_enemy() != actor)
    {
        aimed = true;
        gain *= m_nlc_threat_npc_k;
    }
    m_nlc_threat += (aimed ? gain : -m_nlc_threat_loss) * dt;
    clamp(m_nlc_threat, 0.f, 1.2f);

    if (m_nlc_threat < 1.f || !EnemyMan.get_enemy())
        return;

    // full threat: the shield first, the strong burers' camera wrench while the shield recharges
    if (m_nlc_threat_shield && !m_shield_active && nlc_shield_allowed())
    {
        ActivateShield();
        m_nlc_shield_strain = 0.f;
        m_nlc_overlay_shield_until = now + m_nlc_threat_shield_ms;
        m_nlc_shield_ends = m_nlc_overlay_shield_until;
        m_nlc_overlay_particle_next = 0;
        m_nlc_shield_next = m_nlc_overlay_shield_until + m_shield_cooldown;
        m_nlc_threat = 0.4f;
        nlc_log("threat shield up for %u ms", m_nlc_threat_shield_ms);
        return;
    }

    if (m_nlc_wrench && actor && EnemyMan.get_enemy() == actor && now >= m_nlc_wrench_next && !m_nlc_wr.active && !m_shield_active)
    {
        m_nlc_wrench_next = now + m_nlc_wrench_cooldown;
        const float yaw = ::Random.randF(m_nlc_wrench_min, m_nlc_wrench_max) * (::Random.randI(2) ? 1.f : -1.f);
        const float pitch = ::Random.randF(0.6f, 1.f) * m_nlc_wrench_pitch * (::Random.randI(2) ? 1.f : -1.f);
        nlc_start_wrench(yaw, pitch, u32(::Random.randI(int(m_nlc_wrench_ms_min), int(m_nlc_wrench_ms_max) + 1)), 3.f, "threat");
        m_nlc_threat = 0.3f;
        if (m_nlc_scan_fx)
        {
            ::Sound->play_at_pos(sound_scan, 0, actor->Position());
            actor->Cameras().AddPPEffector(xr_new<CMonsterEffector>(m_nlc_scan_ppi, 1.5f, 0.1f, 0.8f));
        }
    }
}

void CBurer::nlc_overlay_shield_stop(LPCSTR why)
{
    if (!m_nlc_overlay_shield_until)
        return;
    m_nlc_overlay_shield_until = 0;
    DeactivateShield();
    nlc_log("threat shield down (%s)", why);
}

// strain = the health the blocked hits would have cost (damage x immunity x health part x bone scale), against
// nlc_shield_strain_hp x max health; past nlc_shield_leak_start a growing share leaks through; full = broken
float CBurer::nlc_on_shield_blocked(const SHit* hit)
{
    if (m_nlc_shield_strain_hp <= 0.f)
        return 0.f;

    float hit_scale = 1.f, wound_scale = 1.f;
    CDamageManager::HitScale(hit->boneID, hit_scale, wound_scale, hit->aim_bullet);
    const ALife::EHitType type = hit->hit_type;
    float cost = hit->damage() * conditions().GetHitImmunity(type) * conditions().nlc_health_hit_part(type);
    if (type == ALife::eHitTypeFireWound || type == ALife::eHitTypeWound || type == ALife::eHitTypeBurn)
        cost *= hit_scale;
    m_nlc_shield_strain += cost;

    const float capacity = m_nlc_shield_strain_hp * conditions().GetMaxHealth();
    const float ratio = capacity > EPS ? m_nlc_shield_strain / capacity : 1.f;
    if (ratio >= 1.f)
    {
        nlc_log("shield broken: absorbed %.3f health (capacity %.3f)", m_nlc_shield_strain, capacity);
        m_nlc_catch_broken = true;
        m_nlc_shield_strain = 0.f;
        m_nlc_overlay_shield_until = 0;
        DeactivateShield();
        m_nlc_shield_next = Device.dwTimeGlobal + m_nlc_shield_break_cooldown;
        nlc_shield_break_reaction();
        return 0.f;
    }

    float leak = 0.f;
    if (ratio > m_nlc_shield_leak_start && m_nlc_shield_leak_start < 1.f)
        leak = (ratio - m_nlc_shield_leak_start) / (1.f - m_nlc_shield_leak_start) * m_nlc_shield_leak_max;
    if (leak > 0.f && Device.dwTimeGlobal >= m_nlc_shield_crackle_next)
    {
        m_nlc_shield_crackle_next = Device.dwTimeGlobal + 400;
        if (m_shield_keep_particle)
            CParticlesPlayer::StartParticles(m_shield_keep_particle, Fvector().set(0, 1, 0), ID(), -1, true);
        nlc_log("shield straining %.0f %% (leaks %.0f %%)", ratio * 100.f, leak * 100.f);
    }
    return leak;
}

// a broken shield buys the burer a moment: psi pulse (stamina, camera shake), straight to cover, strong ones wrench
void CBurer::nlc_shield_break_reaction()
{
    m_nlc_force_cover = true;
    nlc_barricade_drop("shield broken");
    CActor* actor = Actor();
    if (!actor || !actor->g_Alive() || EnemyMan.get_enemy() != actor || actor->Position().distance_to(Position()) > 40.f)
        return;

    if (m_nlc_break_stamina > 0.f)
        actor->conditions().PowerHit(m_nlc_break_stamina, false);
    if (m_nlc_scan_fx)
        actor->Cameras().AddPPEffector(xr_new<CMonsterEffector>(m_nlc_scan_ppi, 1.2f, 0.05f, 0.8f));

    if (m_nlc_wrench && Device.dwTimeGlobal >= m_nlc_wrench_next)
    {
        m_nlc_wrench_next = Device.dwTimeGlobal + m_nlc_wrench_cooldown;
        const float yaw = ::Random.randF(m_nlc_wrench_min, m_nlc_wrench_max) * (::Random.randI(2) ? 1.f : -1.f);
        const float pitch = ::Random.randF(0.6f, 1.f) * m_nlc_wrench_pitch * (::Random.randI(2) ? 1.f : -1.f);
        nlc_start_wrench(yaw, pitch, u32(::Random.randI(int(m_nlc_wrench_ms_min), int(m_nlc_wrench_ms_max) + 1)), 3.f, "shield break");
    }
    else if (m_nlc_break_shake > 0.f)
    {
        const float yaw = ::Random.randF(0.5f, 1.f) * m_nlc_break_shake * (::Random.randI(2) ? 1.f : -1.f);
        const float pitch = ::Random.randF(0.3f, 0.6f) * m_nlc_break_shake * (::Random.randI(2) ? 1.f : -1.f);
        nlc_start_wrench(yaw, pitch, 220, 4.f, "psi pulse");
    }
}

// a telekinetic field turns some bullets away while the burer is not busy (casting, channelling, puppeting)
bool CBurer::nlc_try_deflect(const SHit* hit)
{
    if (m_nlc_deflect_chance <= 0.f || hit->hit_type != ALife::eHitTypeFireWound || !g_Alive() || !EnemyMan.get_enemy() || m_nlc_busy || nlc_puppet_active())
        return false;

    // only shots from the front half (it faces what it deflects)
    Fvector from_shooter = hit->dir;
    from_shooter.invert();
    if (from_shooter.dotproduct(Direction()) < 0.2f)
        return false;
    if (::Random.randF(0.f, 1.f) >= m_nlc_deflect_chance)
        return false;

    Fmatrix pos;
    CParticlesPlayer::MakeXFORM(this, hit->bone(), hit->dir, hit->p_in_bone_space, pos);
    CParticlesObject* ps = CParticlesObject::Create(particle_fire_shield, TRUE);
    ps->UpdateParent(pos, Fvector().set(0.f, 0.f, 0.f));
    GamePersistent().ps_needtoplay.push_back(ps);

    ++m_nlc_deflect_count;
    if (Device.dwTimeGlobal >= m_nlc_deflect_log_next)
    {
        m_nlc_deflect_log_next = Device.dwTimeGlobal + 2000;
        nlc_log("deflected bullets: %u so far", m_nlc_deflect_count);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Barricade

bool CBurer::nlc_barricade_ok(CPhysicsShellHolder* obj) const
{
    if (!m_nlc_barricade || !obj || !obj->m_pPhysicsShell)
        return false;
    const shared_str& section = obj->cNameSect();
    if (nlc_in_list(m_nlc_barricade_deny, section))
        return false;
    if (nlc_in_list(m_nlc_barricade_allow, section))
        return true;
    if (smart_cast<CInventoryItem*>(obj))
        return false; // guns, grenades and loot are no cover

    const float mass = obj->m_pPhysicsShell->getMass();
    if (mass < m_nlc_barricade_mass_min || mass > m_nlc_barricade_mass_max)
        return false;

    // size, not mass: bone pieces of a crashed skeleton weigh 10-30 kg but hide nothing
    Fvector size;
    obj->BoundingBox().getsize(size);
    const float largest = _max(size.x, _max(size.y, size.z));
    return largest >= m_nlc_barricade_min_size && size.x * size.y * size.z >= m_nlc_barricade_min_volume;
}

CPhysicsShellHolder* CBurer::nlc_find_corpse_barricade()
{
    if (!m_nlc_barricade || m_nlc_corpse_species.empty())
        return nullptr;

    CPhysicsShellHolder* best = nullptr;
    float best_dist = flt_max;
    m_nlc_nearest.clear();
    Level().ObjectSpace.GetNearest(m_nlc_nearest, Position(), m_tele_find_radius, this);
    for (CObject* O : m_nlc_nearest)
    {
        CBaseMonster* corpse = smart_cast<CBaseMonster*>(O);
        if (!corpse || corpse->g_Alive() || corpse->getDestroy() || !corpse->m_pPhysicsShell || !corpse->m_pPhysicsShell->isActive() ||
            CTelekinesis::is_active_object(corpse) || nlc_stealth::is_visible_body(corpse))
            continue;
        if (!pSettings->line_exist(corpse->cNameSect(), "species"))
            continue;
        if (!nlc_in_list(m_nlc_corpse_species, pSettings->r_string(corpse->cNameSect(), "species")))
            continue;
        const float dist = corpse->Position().distance_to(Position());
        if (dist < best_dist)
        {
            best_dist = dist;
            best = corpse;
        }
    }
    return best;
}

//////////////////////////////////////////////////////////////////////////
// Puppet gun

static bool nlc_is_launcher(CWeaponMagazined* gun) { return smart_cast<CWeaponRPG7*>(gun) || smart_cast<CWeaponRG6*>(gun); }

// something to fire: rifle rounds, or a launcher round ready (either mode of a rifle with an underbarrel launcher)
static bool nlc_puppet_has_ammo(CWeaponMagazined* gun)
{
    if (CWeaponMagazinedWGrenade* rifle_gl = smart_cast<CWeaponMagazinedWGrenade*>(gun))
        return rifle_gl->nlc_rifle_ammo() || rifle_gl->nlc_ubgl_ready();
    return gun->nlc_ammo_elapsed() > 0;
}

bool CBurer::nlc_puppet_reserved(CPhysicsShellHolder* obj) const
{
    if (!m_nlc_puppet_enabled)
        return false;
    CWeaponMagazined* gun = smart_cast<CWeaponMagazined*>(obj);
    return gun && !nlc_is_launcher(gun) && !gun->IsMisfire() && nlc_puppet_has_ammo(gun);
}

// rifles with an underbarrel launcher (CWeaponMagazinedWGrenade is also a CRocketLauncher) qualify in either mode;
// RPG / RG-6 do not
static bool nlc_puppet_gun_ok(CWeaponMagazined* gun)
{
    return gun && !gun->getDestroy() && !gun->H_Parent() && !gun->m_nlc_tele_held && gun->PPhysicsShell() && gun->PPhysicsShell()->isActive() &&
        !gun->IsMisfire() && !nlc_is_launcher(gun) && nlc_puppet_has_ammo(gun);
}

bool CBurer::nlc_puppet_hover_ok(const Fvector& p) const
{
    const CLevelGraph& graph = ai().level_graph();
    const u32 node = graph.vertex_id(p);
    if (!graph.valid_vertex_id(node))
        return false;
    if (graph.vertex_position(node).distance_to_xz(p) > 1.5f)
        return false;
    const float above = p.y - graph.vertex_plane_y(node, p.x, p.z);
    // against an enemy up on a roof it may rise to his height (pass 5)
    float max_above = 3.5f;
    const CEntityAlive* enemy = m_nlc_elev ? const_cast<CBurer*>(this)->EnemyMan.get_enemy() : nullptr;
    if (enemy)
        max_above = _max(max_above, enemy->Position().y + 1.f - graph.vertex_plane_y(node, p.x, p.z));
    if (above < 0.8f || above > max_above)
        return false;

    // headroom: no ceiling right above, nothing solid between the gun and the floor below it
    collide::rq_result rq;
    if (Level().ObjectSpace.RayPick(p, Fvector().set(0.f, 1.f, 0.f), 0.4f, collide::rqtStatic, rq, nullptr))
        return false;
    if (Level().ObjectSpace.RayPick(p, Fvector().set(0.f, -1.f, 0.f), above - 0.3f, collide::rqtStatic, rq, nullptr))
        return false;
    return true;
}

void CBurer::nlc_puppet_pick_hover(const CEntityAlive* enemy, const Fvector& aim, bool crowded)
{
    NlcPuppet& p = m_nlc_puppet;
    const Fvector current = p.gun->Position();
    const CLevelGraph& graph = ai().level_graph();

    auto at_height = [&](Fvector c, Fvector& out) {
        const u32 node = graph.vertex_id(c);
        if (!graph.valid_vertex_id(node))
            return false;
        c.y = graph.vertex_plane_y(node, c.x, c.z) + m_nlc_puppet_hover_height;
        if (!nlc_puppet_hover_ok(c) || c.distance_to(Position()) > m_nlc_puppet_max_dist)
            return false;
        out = c;
        return true;
    };

    // keep-away: drift back from an approaching enemy (slowly; rolled once per seize)
    if (crowded)
    {
        Fvector away;
        away.sub(current, enemy->Position());
        away.y = 0.f;
        away.normalize_safe();
        Fvector c;
        if (at_height(Fvector().mad(current, away, 3.f), c))
            p.hover = c;
        return;
    }

    // rise over the cover first (0.6 / 1.2 m up; 2.5 / 4 m more against an elevated enemy), sideways only when that fails
    for (float rise : {0.6f, 1.2f, 2.5f, 4.f})
    {
        if (rise > 2.f && !m_nlc_elev)
            break;
        Fvector c = current;
        c.y += rise;
        if (nlc_puppet_hover_ok(c) && c.distance_to(Position()) <= m_nlc_puppet_max_dist && nlc_clear_path(c, aim, p.gun, enemy))
        {
            p.hover = c;
            return;
        }
    }

    // the nearest point around the gun with a free line to the target: drifts round cover for a crossfire
    float best = flt_max;
    Fvector best_point = p.hover;
    const float start = ::Random.randF(0.f, PI_MUL_2);
    for (float radius : {2.5f, 4.5f})
    {
        for (int i = 0; i < 8; ++i)
        {
            const float angle = start + float(i) * PI_DIV_4;
            Fvector c;
            if (!at_height(Fvector().set(current.x + _cos(angle) * radius, current.y, current.z + _sin(angle) * radius), c))
                continue;
            if (!nlc_clear_path(c, aim, p.gun, enemy))
                continue;
            const float d = c.distance_to(current);
            if (d < best)
            {
                best = d;
                best_point = c;
            }
        }
        if (best < flt_max)
            break;
    }
    p.hover = best_point;
}

void CBurer::nlc_puppet_release(LPCSTR why, bool throw_at_enemy)
{
    NlcPuppet& p = m_nlc_puppet;
    CWeaponMagazined* gun = p.gun;
    if (!gun)
        return;

    const u32 now = Device.dwTimeGlobal;
    gun->m_nlc_tele_held = false;
    gun->m_nlc_tele_lock_until = now + m_nlc_puppet_lock_ms;
    StopTeleObjectParticle(gun);

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    bool thrown = false;
    CTelekineticObject* tele = m_nlc_gun_tele.find_object(gun);
    if (tele)
    {
        tele->m_nlc_steer = false;
        tele->m_nlc_spin.set(0.f, 0.f, 0.f);
    }
    // thrown only when empty, at the actor, in sight: a loaded gun or one landing at a stalker's feet arms the enemy
    const bool empty = !nlc_puppet_has_ammo(gun);
    const bool may_throw = enemy && enemy == Actor() && empty && EnemyMan.see_enemy_now();
    if (throw_at_enemy && !may_throw)
        nlc_log("puppet release: not thrown (%s)", !empty ? "rounds left" : (!enemy || enemy != Actor() ? "enemy is not the actor" : "enemy out of sight"));
    if (throw_at_enemy && may_throw && tele)
    {
        Fvector aim = get_head_position(const_cast<CEntityAlive*>(enemy));
        const float speed = _max(m_tele_fly_velocity * m_nlc_tele_weapon_speed_k, 1.f);
        float flight = gun->Position().distance_to(aim) / speed;
        aim = nlc_lead_point(enemy, aim, flight, m_nlc_tele_lead);
        flight = gun->Position().distance_to(aim) / speed;
        nlc_tele_set_hit_callback(gun, nlc_tele_damage(gun), now + 3000);
        m_nlc_gun_tele.fire_t(gun, aim, flight);
        sound().play(eMonsterSoundTeleAttack);
        thrown = true;
    }
    else if (tele)
        m_nlc_gun_tele.deactivate(gun);

    nlc_log("puppet release %s(%u) reason=%s shots=%u ammo_left=%d %s", gun->cNameSect().c_str(), gun->ID(), why, p.shots, gun->nlc_ammo_elapsed(),
        thrown ? "thrown at the enemy" : "dropped");
    m_nlc_puppet = NlcPuppet();
    m_nlc_puppet_next = now + m_nlc_puppet_cooldown;
}

void CBurer::nlc_puppet_update()
{
    if (!m_nlc_puppet_enabled)
        return;

    NlcPuppet& p = m_nlc_puppet;
    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();

    if (!p.gun)
    {
        if (!enemy || now < m_nlc_puppet_next)
            return;
        m_nlc_puppet_next = now + 400; // scan rate

        // the nearest usable gun around the burer or the enemy (a disarmed gun lands next to the player)
        CWeaponMagazined* best = nullptr;
        float best_dist = flt_max;
        auto scan = [&](const Fvector& center, float radius) {
            m_nlc_nearest.clear();
            Level().ObjectSpace.GetNearest(m_nlc_nearest, center, radius, this);
            for (CObject* O : m_nlc_nearest)
            {
                CWeaponMagazined* gun = smart_cast<CWeaponMagazined*>(O);
                if (!nlc_puppet_gun_ok(gun) || nlc_own_held(gun))
                    continue;
                const float dist = gun->Position().distance_to(Position());
                if (dist > m_nlc_puppet_max_dist)
                    continue;
                if (dist < best_dist)
                {
                    best_dist = dist;
                    best = gun;
                }
            }
        };
        scan(Position(), m_nlc_puppet_radius);
        scan(enemy->Position(), m_nlc_puppet_radius * 0.5f);
        if (!best)
        {
            m_nlc_puppet_candidate = u16(-1);
            return;
        }

        // a gun has to lie for a moment first: time for the player to grab it back
        if (best->ID() != m_nlc_puppet_candidate)
        {
            m_nlc_puppet_candidate = best->ID();
            m_nlc_puppet_candidate_since = now;
            return;
        }
        if (now < m_nlc_puppet_candidate_since + (best->ID() == m_nlc_npc_drop_gun ? m_nlc_npc_seize_delay : m_nlc_puppet_seize_delay))
            return;

        CTelekineticObject* tele = m_nlc_gun_tele.activate(best, 3.f, 0.3f, 600000, false);
        if (!tele)
            return;

        // one flow whatever mode the gun was dropped in: the rifle magazine is the gun's state from here on
        CWeaponMagazinedWGrenade* rifle_gl = smart_cast<CWeaponMagazinedWGrenade*>(best);
        const bool was_launcher_mode = rifle_gl && rifle_gl->IsGrenadeMode();
        if (was_launcher_mode)
            rifle_gl->nlc_switch_to_rifle();

        p = NlcPuppet();
        p.gun = best;
        p.phase = 1;
        p.phase_start = now;
        p.seized = now;
        p.hover_since = now;
        p.end_by = now + m_nlc_puppet_max_ms;
        p.hp_at_start = conditions().GetHealth();
        p.keep_away = ::Random.randF(0.f, 1.f) < m_nlc_puppet_keep_away_chance;
        p.burst_left = u32(::Random.randI(int(m_nlc_puppet_burst_min), int(m_nlc_puppet_burst_max) + 1));
        p.ubgl_at = now + u32(::Random.randI(int(m_nlc_puppet_ubgl_min), int(m_nlc_puppet_ubgl_max) + 1));
        if (rifle_gl && !rifle_gl->nlc_rifle_ammo())
            p.ubgl_at = now; // only the launcher round left: it goes first
        p.seed = float(best->ID() % 97) * 0.731f;
        p.hover = best->Position();
        p.hover.y += m_nlc_puppet_hover_height;
        tele->m_nlc_steer = true;
        tele->m_nlc_target = p.hover;
        tele->m_nlc_max_speed = 4.f;
        tele->m_nlc_max_accel = 8.f;
        // lifted loosely: a slow random tumble until it takes aim
        tele->m_nlc_spin.set(::Random.randF(-1.2f, 1.2f), ::Random.randF(-1.2f, 1.2f), ::Random.randF(-1.2f, 1.2f));
        tele->set_sound(sound_tele_hold, sound_tele_throw);
        best->m_nlc_tele_held = true;
        StartTeleObjectParticle(best);
        m_nlc_puppet_candidate = u16(-1);
        nlc_log("puppet seize %s(%u) rifle=%d ubgl=%d (launcher rounds=%d attached=%u)%s dist=%.1f keep_away=%d", best->cNameSect().c_str(), best->ID(),
            rifle_gl ? (rifle_gl->nlc_rifle_ammo() ? best->nlc_ammo_elapsed() : 0) : best->nlc_ammo_elapsed(), rifle_gl && rifle_gl->nlc_ubgl_ready() ? 1 : 0,
            rifle_gl ? rifle_gl->nlc_launcher_rounds() : 0, rifle_gl ? rifle_gl->nlc_attached_rounds() : 0u, was_launcher_mode ? " (was in launcher mode)" : "",
            best_dist, p.keep_away ? 1 : 0);
        return;
    }

    CTelekineticObject* tele = m_nlc_gun_tele.find_object(p.gun);
    if (!tele || p.gun->getDestroy() || p.gun->H_Parent())
    {
        nlc_log("puppet lost the gun (%s%s)", p.gun->H_Parent() ? "taken by " : "gone", p.gun->H_Parent() ? p.gun->H_Parent()->cName().c_str() : "");
        if (!p.gun->getDestroy())
            p.gun->m_nlc_tele_held = false;
        m_nlc_puppet = NlcPuppet();
        m_nlc_puppet_next = now + m_nlc_puppet_cooldown;
        return;
    }
    const float hp_lost = p.hp_at_start - conditions().GetHealth();
    if (hp_lost > m_nlc_puppet_break_damage)
    {
        nlc_puppet_release("concentration broken", false);
        return;
    }
    if (now > p.end_by)
    {
        nlc_puppet_release("max time", true);
        return;
    }
    if (!enemy)
    {
        nlc_puppet_release("no enemy", false);
        return;
    }
    // held for nothing: no shot and no line of fire for a while (the enemy hides): put it down, seize it again later
    if (p.los_lost_since && now >= p.los_lost_since + m_nlc_puppet_blind_ms && now >= p.seized + m_nlc_puppet_blind_ms)
    {
        nlc_puppet_release("no line of fire", false);
        return;
    }

    // aim: chest (head for configured ranks), led by the bullet's flight time
    Fvector aim;
    if (m_nlc_puppet_head_aim)
        aim = get_head_position(const_cast<CEntityAlive*>(enemy));
    else
        enemy->Center(aim);
    const Fvector muzzle = p.gun->get_LastFP();
    const float bullet_speed = _max(p.gun->nlc_bullet_speed(), 100.f);
    aim = nlc_lead_point(enemy, aim, muzzle.distance_to(aim) / bullet_speed, 1.f);
    Fvector desired;
    desired.sub(aim, muzzle);
    desired.normalize_safe();

    // effort: the hold looks harder far from the burer, after damage, when weakened, and while it clears a jam
    const bool unjamming = now < p.unjam_until;
    const float t = float(now) * 0.001f;
    float effort = 1.f + p.gun->Position().distance_to(Position()) / 20.f + _max(hp_lost, 0.f) / _max(m_nlc_puppet_break_damage, 0.01f) +
        0.5f * float(nlc_phase());
    if (unjamming)
    {
        effort *= 2.5f;
        if (!p.unjam_sound_done && now + 400 >= p.unjam_until)
        {
            p.unjam_sound_done = true;
            p.gun->nlc_puppet_sound(true);
        }
    }

    // recoil, overshoot and sag decay; the steering pulls the barrel back onto the target
    const float decay = expf(-Device.fTimeDelta * 6.f);
    p.kick_pitch *= decay;
    p.kick_yaw *= decay;
    const float jitter = deg2rad(m_nlc_puppet_jitter) * effort;
    Fvector barrel_goal = desired;
    float heading, pitch;
    barrel_goal.getHP(heading, pitch);
    float shake_h = 0.f, shake_p = 0.f;
    if (unjamming) // rattled while the jam is worked out
    {
        shake_h = ::Random.randF(-0.12f, 0.12f);
        shake_p = ::Random.randF(-0.12f, 0.12f);
    }
    barrel_goal.setHP(heading + p.kick_yaw + _sin(t * 9.1f + p.seed) * jitter + shake_h, pitch + p.kick_pitch + _sin(t * 8.3f + p.seed * 2.f) * jitter + shake_p);

    // hover point: kept for a while; moved only when the line has been lost for a moment, or to keep away
    if (now >= p.next_hover)
    {
        p.next_hover = now + 300;
        p.los = nlc_clear_path(muzzle, aim, p.gun, enemy);
        if (p.los)
            p.los_lost_since = 0;
        else if (!p.los_lost_since)
            p.los_lost_since = now;
        const bool lost = p.los_lost_since && now >= p.los_lost_since + m_nlc_puppet_los_grace;
        const bool committed = now < p.hover_since + m_nlc_puppet_hover_commit;
        const bool crowded = p.keep_away && enemy->Position().distance_to_xz(p.gun->Position()) < m_nlc_puppet_keep_dist && now >= p.hover_since + 1000;
        if (crowded || (lost && !committed))
        {
            const Fvector old = p.hover;
            nlc_puppet_pick_hover(enemy, aim, crowded);
            if (old.distance_to(p.hover) > 0.3f)
            {
                p.hover_since = now;
                nlc_log("puppet moves %.1f m (%s)", old.distance_to(p.hover), crowded ? "keep-away" : "no line of fire");
            }
        }
        tele->m_nlc_max_speed = p.phase == 1 ? 4.f : m_nlc_puppet_drift_speed;
    }

    // hover point + two slow bob waves + a tremor that grows with effort + shot bounce; a lean before a burst
    p.lift_kick *= expf(-Device.fTimeDelta * 5.f);
    Fvector target = p.hover;
    target.y += _sin(t * PI_MUL_2 * 0.5f + p.seed) * m_nlc_puppet_bob + _sin(t * PI_MUL_2 * 0.17f + p.seed * 5.f) * m_nlc_puppet_bob * 0.6f + p.lift_kick;
    const float tremor = m_nlc_puppet_tremor * effort;
    target.x += _sin(t * 7.3f + p.seed) * tremor;
    target.y += _sin(t * 6.7f + p.seed * 2.f) * tremor;
    target.z += _cos(t * 7.9f + p.seed * 3.f) * tremor;
    if (p.phase == 3 && p.next_shot > now && p.next_shot - now < 250 && p.burst_start && !unjamming)
        target.mad(desired, 0.25f * (1.f - float(p.next_shot - now) / 250.f));
    tele->m_nlc_target = target;
    tele->m_nlc_orient = p.phase >= 2;
    tele->m_nlc_dir = barrel_goal;

    if (p.phase == 1 && now >= p.phase_start + 700)
    {
        p.phase = 2;
        p.phase_start = now;
        tele->m_nlc_spin.set(0.f, 0.f, 0.f);
        // snaps toward the target with a small overshoot
        p.kick_yaw += ::Random.randF(0.12f, 0.2f) * (::Random.randI(2) ? 1.f : -1.f);
        p.kick_pitch += 0.05f;
    }
    if (p.phase == 2 && now >= p.phase_start + m_nlc_puppet_settle_ms)
    {
        p.phase = 3;
        p.phase_start = now;
        p.burst_start = true;
        nlc_log("puppet firing %s(%u) dist=%.1f los=%d", p.gun->cNameSect().c_str(), p.gun->ID(), muzzle.distance_to(aim), p.los ? 1 : 0);
    }
    if (p.phase != 3 || now < p.next_shot || !p.los || unjamming)
        return;

    const float off = acosf(clampr(p.gun->XFORM().k.dotproduct(desired), -1.f, 1.f));
    if (off > deg2rad(m_nlc_puppet_fire_cone))
        return;

    // the burer must see the target or sense the actor (puppeting from cover, with a penalty)
    const bool sees = EnemyMan.see_enemy_now();
    const CVisualMemoryManager::NlcSense& sense = memory().visual().m_nlc_sense;
    const bool sensed = enemy == Actor() && sense.notice > 0.f && sense.level >= sense.notice;
    if (!sees && !sensed)
        return;

    // no friendly fire: a living thing other than the target in the line holds the burst
    collide::rq_result rq;
    if (Level().ObjectSpace.RayPick(muzzle, desired, muzzle.distance_to(aim), collide::rqtObject, rq, p.gun) && rq.O && rq.O != enemy)
    {
        CEntityAlive* blocker = smart_cast<CEntityAlive*>(rq.O);
        if (blocker && blocker->g_Alive() && !nlc_stealth::is_visible_body(blocker))
        {
            p.next_shot = now + 300;
            return;
        }
    }

    // the underbarrel round once, 6-8 s into the hold (at once when it is all the gun has), at the feet of a target
    // far enough away; retried until it is
    CWeaponMagazinedWGrenade* rifle_gl = smart_cast<CWeaponMagazinedWGrenade*>(p.gun);
    const bool rifle_ammo = rifle_gl ? rifle_gl->nlc_rifle_ammo() : p.gun->nlc_ammo_elapsed() > 0;
    if (m_nlc_puppet_ubgl && rifle_gl && !p.ubgl_done && now >= p.ubgl_at && rifle_gl->nlc_ubgl_ready())
    {
        const float dist = muzzle.distance_to(enemy->Position());
        if (dist >= m_nlc_puppet_ubgl_min_dist)
        {
            p.ubgl_done = true;
            const int result = rifle_gl->nlc_puppet_launch(ID(), enemy->Position());
            nlc_log("puppet ubgl %s at %.1f m: %s", p.gun->cNameSect().c_str(), dist, result == 0 ? "launched" : "not ready");
            if (result == 0)
            {
                p.kick_pitch += 0.15f;
                p.lift_kick += 0.25f;
                p.next_shot = now + 900;
                return;
            }
        }
        else
        {
            if (now >= p.ubgl_log_next)
            {
                p.ubgl_log_next = now + 2000;
                nlc_log("puppet ubgl waits: target %.1f m < %.1f m", dist, m_nlc_puppet_ubgl_min_dist);
            }
            if (!rifle_ammo)
            {
                if (now >= p.ubgl_at + 4000)
                    nlc_puppet_release("launcher round, target stayed too close", true);
                return;
            }
        }
    }
    if (!rifle_ammo)
    {
        nlc_puppet_release("empty", true);
        return;
    }

    const float disp_k = m_nlc_puppet_disp_k * (1.f + p.gun->Position().distance_to(Position()) / 30.f) * (sees ? 1.f : m_nlc_puppet_blind_disp_k);
    const int result = p.gun->nlc_puppet_shot(ID(), disp_k, m_nlc_puppet_hit_k, m_nlc_puppet_jam_cap);
    if (result == 1)
    {
        // jammed: rattled for a few seconds while the burer clears it, then it carries on
        p.unjam_until = now + m_nlc_puppet_unjam_ms;
        p.unjam_sound_done = false;
        p.next_shot = p.unjam_until + 200;
        p.burst_start = true;
        nlc_log("puppet %s(%u) jammed after %u shots, clearing it for %u ms", p.gun->cNameSect().c_str(), p.gun->ID(), p.shots, m_nlc_puppet_unjam_ms);
        return;
    }
    if (result == 2)
    {
        nlc_puppet_release("empty", true);
        return;
    }

    ++p.shots;
    p.burst_start = false;
    p.kick_pitch += m_nlc_puppet_recoil;
    p.lift_kick += 0.04f; // each shot bounces it up a little
    p.kick_yaw += ::Random.randF(-0.4f, 0.4f) * m_nlc_puppet_recoil;
    const u32 interval = _max(u32(p.gun->nlc_shot_interval() * 1000.f), u32(60));
    if (p.gun->SingleShotMode())
    {
        p.next_shot = now + _max(interval, u32(350 + ::Random.randI(250)));
        p.burst_start = true;
    }
    else if (--p.burst_left == 0)
    {
        p.burst_left = u32(::Random.randI(int(m_nlc_puppet_burst_min), int(m_nlc_puppet_burst_max) + 1));
        p.next_shot = now + u32(600 + ::Random.randI(600));
        p.kick_pitch -= 0.05f; // sags between bursts, then corrects
        p.burst_start = true;
    }
    else
        p.next_shot = now + interval;

    nlc_log("puppet shot %u off=%.1f deg disp_k=%.2f dist=%.1f sees=%d effort=%.2f ammo_left=%d", p.shots, rad2deg(off), disp_k, muzzle.distance_to(aim), sees ? 1 : 0,
        effort, p.gun->nlc_ammo_elapsed());

    const bool more = rifle_gl ? (rifle_gl->nlc_rifle_ammo() || (m_nlc_puppet_ubgl && !p.ubgl_done && rifle_gl->nlc_ubgl_ready())) : p.gun->nlc_ammo_elapsed() > 0;
    if (!more)
        nlc_puppet_release("empty", true);
}

//////////////////////////////////////////////////////////////////////////
// Barricade (free action)

bool CBurer::nlc_own_held(CPhysicsShellHolder* obj)
{
    return obj && (CTelekinesis::is_active_object(obj) || m_nlc_gren_tele.is_active_object(obj) || m_nlc_gun_tele.is_active_object(obj) || m_nlc_bar_tele.is_active_object(obj));
}

float CBurer::nlc_throw_speed(CPhysicsShellHolder* obj) const
{
    return _max(m_tele_fly_velocity * (smart_cast<CWeapon*>(obj) ? m_nlc_tele_weapon_speed_k : 1.f), 1.f);
}

// two burers grabbed the same round / barricade and steered it against each other
bool CBurer::nlc_held_by_any(CPhysicsShellHolder* obj)
{
    if (!obj)
        return false;
    for (CBurer* burer : s_nlc_burers)
        if (burer->nlc_own_held(obj))
            return true;
    return false;
}

void CBurer::nlc_barricade_drop(LPCSTR why)
{
    CPhysicsShellHolder* obj = m_nlc_bar_obj;
    if (!obj)
        return;
    m_nlc_bar_obj = nullptr;
    m_nlc_bar_next = Device.dwTimeGlobal + m_nlc_barricade_cooldown;
    if (CTelekineticObject* tele = m_nlc_bar_tele.find_object(obj))
        tele->m_nlc_steer = false;
    if (m_nlc_bar_tele.is_active_object(obj))
        m_nlc_bar_tele.deactivate(obj);
    StopTeleObjectParticle(obj);
    nlc_log("barricade dropped %s (%s)", obj->cNameSect().c_str(), why);
}

void CBurer::nlc_barricade_update()
{
    if (!m_nlc_barricade || m_nlc_barricade_mode == 0)
        return;

    const u32 now = Device.dwTimeGlobal;
    const CEntityAlive* enemy = EnemyMan.get_enemy();

    if (m_nlc_bar_obj)
    {
        CTelekineticObject* tele = m_nlc_bar_tele.find_object(m_nlc_bar_obj);
        if (!tele || m_nlc_bar_obj->getDestroy())
        {
            m_nlc_bar_obj = nullptr;
            m_nlc_bar_next = now + m_nlc_barricade_cooldown;
            return;
        }
        if (!enemy)
        {
            nlc_barricade_drop("no enemy");
            return;
        }
        if (now >= m_nlc_bar_until)
        {
            nlc_barricade_drop("time");
            return;
        }
        const Fvector post = tele->m_nlc_target;
        if (m_nlc_bar_obj->Position().distance_to(post) > m_nlc_barricade_leash)
        {
            if (!m_nlc_bar_far_since)
                m_nlc_bar_far_since = now;
            else if (now >= m_nlc_bar_far_since + 2000)
            {
                nlc_barricade_drop("left behind");
                return;
            }
        }
        else
            m_nlc_bar_far_since = 0;

        // between the burer and the enemy: the box centre at chest height above the ground under the post, the
        // lowest point at least 0.2 m up, the broad side to the enemy, a slow wobble and a tremor that grows with the
        // effort (distance, damage taken while holding, phase)
        Fvector dir;
        dir.sub(enemy->Position(), Position());
        dir.y = 0.f;
        if (dir.square_magnitude() < EPS_L)
            dir = Direction();
        dir.normalize_safe();
        const float t = float(now) * 0.001f;
        const float seed = float(m_nlc_bar_obj->ID() % 89) * 0.53f;
        Fvector center;
        center.mad(Position(), dir, m_nlc_barricade_dist);
        float ground = Position().y;
        const CLevelGraph& graph = ai().level_graph();
        const u32 node = graph.vertex_id(center);
        if (graph.valid_vertex_id(node) && graph.vertex_position(node).distance_to_xz(center) < 2.f)
            ground = graph.vertex_plane_y(node, center.x, center.z);
        // the shell's real extents (world axes): a ragdoll's visual box is its bind pose and far too large
        CPhysicsShell* shell = m_nlc_bar_obj->m_pPhysicsShell;
        float* ext = m_nlc_bar_ext;
        if (now >= m_nlc_bar_ext_next)
        {
            m_nlc_bar_ext_next = now + 100;
            shell->get_Extensions(Fvector().set(1.f, 0.f, 0.f), 0.f, ext[0], ext[1]);
            shell->get_Extensions(Fvector().set(0.f, 1.f, 0.f), 0.f, ext[2], ext[3]);
            shell->get_Extensions(Fvector().set(0.f, 0.f, 1.f), 0.f, ext[4], ext[5]);
        }
        const float lo_x = ext[0], hi_x = ext[1], lo_y = ext[2], hi_y = ext[3], lo_z = ext[4], hi_z = ext[5];
        Fvector mid;
        mid.set((lo_x + hi_x) * 0.5f, (lo_y + hi_y) * 0.5f, (lo_z + hi_z) * 0.5f);
        Fvector held = mid; // the steered point: the shell's middle, or the gripped element
        if (tele->m_nlc_grip)
        {
            Fmatrix m;
            tele->m_nlc_grip->GetGlobalTransformDynamic(&m);
            held = m.c;
            // the element high enough for the body hanging below it to clear the ground
            center.y = ground + _max(m_nlc_barricade_height, 0.2f + _max(held.y - lo_y, 0.f));
        }
        else
            center.y = ground + _max(m_nlc_barricade_height, 0.2f + (hi_y - lo_y) * 0.5f);
        const float effort = 1.f + m_nlc_bar_obj->Position().distance_to(Position()) / 20.f + _max(m_nlc_bar_hp - conditions().GetHealth(), 0.f) / 0.05f +
            0.5f * float(nlc_phase());
        const float tremor = m_nlc_barricade_tremor * effort;
        center.y += _sin(t * PI_MUL_2 * 0.4f + seed) * 0.06f + _sin(t * 6.7f + seed * 2.f) * tremor;
        center.x += _sin(t * 7.3f + seed) * tremor;
        center.z += _cos(t * 7.9f + seed * 3.f) * tremor;

        // the steering moves the origin (or the gripped element): offset by where the shell's middle sits now
        Fvector target = center;
        if (!tele->m_nlc_grip)
            target.sub(Fvector().sub(mid, m_nlc_bar_obj->Position()));
        tele->m_nlc_target = target;
        if (now >= m_nlc_bar_log_next)
        {
            m_nlc_bar_log_next = now + 2000;
            nlc_log("barricade hold: bottom %.2f m, top %.2f m over the ground (burer's head %.2f m)", lo_y - ground, hi_y - ground,
                get_head_position(this).y - Position().y);
        }
        tele->m_nlc_orient = !m_nlc_bar_corpse;
        tele->m_nlc_dir = dir;
        return;
    }

    if (!enemy || now < m_nlc_bar_next)
        return;
    m_nlc_bar_next = now + 500; // scan rate

    const bool hit_recently = HitMemory.is_hit() && HitMemory.get_last_hit_time() + 3000 > now;
    const bool wanted = m_nlc_barricade_mode == 2 || m_nlc_threat >= 0.5f || hit_recently || nlc_phase() >= 1;
    if (!wanted)
        return;

    // the largest suitable object around it (size, not mass), else an allowed corpse
    CPhysicsShellHolder* pick = nullptr;
    float best_volume = 0.f;
    m_nlc_nearest.clear();
    Level().ObjectSpace.GetNearest(m_nlc_nearest, Position(), m_tele_find_radius, this);
    for (CObject* O : m_nlc_nearest)
    {
        CPhysicsShellHolder* obj = smart_cast<CPhysicsShellHolder*>(O);
        if (!obj || obj->getDestroy() || smart_cast<CEntityAlive*>(O) || !obj->m_pPhysicsShell || !obj->m_pPhysicsShell->isActive() || nlc_held_by_any(obj) ||
            (obj->spawn_ini() && obj->spawn_ini()->section_exist("ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "ph_heavy") && pSettings->r_bool(obj->cNameSect().c_str(), "ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "quest_item") && pSettings->r_bool(obj->cNameSect().c_str(), "quest_item")) || obj->hasFixedBones() ||
            !obj->m_pPhysicsShell->get_ApplyByGravity() || !nlc_barricade_ok(obj))
            continue;
        Fvector size;
        obj->BoundingBox().getsize(size);
        const float volume = size.x * size.y * size.z;
        if (volume > best_volume)
        {
            best_volume = volume;
            pick = obj;
        }
    }
    LPCSTR kind = "object";
    if (!pick)
    {
        pick = nlc_find_corpse_barricade();
        kind = "corpse";
    }
    if (!pick)
        return;

    CTelekineticObject* tele = m_nlc_bar_tele.activate(pick, 3.f, 0.5f, 600000, false);
    if (!tele)
        return;
    tele->m_nlc_steer = true;
    tele->m_nlc_max_speed = m_nlc_barricade_speed;
    tele->m_nlc_max_accel = m_nlc_barricade_accel;
    tele->m_nlc_gain = m_nlc_barricade_gain;
    m_nlc_bar_far_since = 0;
    m_nlc_bar_hp = conditions().GetHealth();
    m_nlc_bar_ext_next = 0;
    m_nlc_bar_corpse = smart_cast<CEntityAlive*>(pick) != nullptr;

    // the broad side (the face across the thinnest axis of the box) turns toward the enemy, slowly
    Fvector box_size;
    pick->BoundingBox().getsize(box_size);
    tele->m_nlc_axis = box_size.x <= box_size.y && box_size.x <= box_size.z ? 0 : (box_size.y <= box_size.z ? 1 : 2);
    tele->m_nlc_axis_twoway = true;
    tele->m_nlc_turn_k = 3.f;
    tele->m_nlc_turn_max = m_nlc_barricade_turn;

    // experimental: a corpse held by its head or torso, the rest hanging
    LPCSTR grip_name = "whole";
    if (m_nlc_bar_corpse && m_nlc_barricade_grip)
    {
        CPhysicsElement* element = nlc_grip_element(pick, m_nlc_barricade_grip == 1 ? m_nlc_grip_head_bones : m_nlc_grip_torso_bones);
        if (element)
        {
            pick->m_pPhysicsShell->set_ApplyByGravity(TRUE);
            element->set_ApplyByGravity(FALSE);
            tele->m_nlc_grip = element;
            m_nlc_bar_hang = _max(box_size.x, _max(box_size.y, box_size.z)) * 0.85f;
            grip_name = m_nlc_barricade_grip == 1 ? "head" : "torso";
        }
        else
            grip_name = "whole (no grip bone)";
    }
    tele->m_nlc_target = pick->Position();
    tele->m_nlc_target.y += 0.5f;
    tele->set_sound(sound_tele_hold, sound_tele_throw);
    StartTeleObjectParticle(pick);
    m_nlc_bar_obj = pick;
    m_nlc_bar_until = now + m_nlc_barricade_max_ms;

    Fvector size;
    pick->BoundingBox().getsize(size);
    nlc_log("barricade up: %s %s mass=%.0f size=%.1fx%.1fx%.1f grip=%s (%s)", kind, pick->cNameSect().c_str(), pick->m_pPhysicsShell ? pick->m_pPhysicsShell->getMass() : 0.f,
        size.x, size.y, size.z, grip_name, m_nlc_barricade_mode == 2 ? "always" : (hit_recently ? "hit" : (m_nlc_threat >= 0.5f ? "threat" : "wounded")));
}

//////////////////////////////////////////////////////////////////////////
// Brain

u32 CBurer::nlc_phase()
{
    const float hp = conditions().GetHealth();
    const u32 phase = hp > m_nlc_phase_wounded ? 0 : (hp > m_nlc_phase_desperate ? 1 : 2);
    if (phase != m_nlc_logged_phase)
    {
        m_nlc_logged_phase = phase;
        nlc_log("phase %s hp=%.2f", phase == 0 ? "healthy" : (phase == 1 ? "wounded" : "desperate"), hp);
    }
    return phase;
}

bool CBurer::nlc_gravi_path_clear()
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return false;
    Fvector from = Position();
    from.y += 0.6f;
    Fvector to;
    enemy->Center(to);
    // static geometry only: its own barricade (and props it shoves anyway) must not stop the wave
    Fvector dir;
    dir.sub(to, from);
    const float len = dir.magnitude();
    if (len < EPS_L)
        return true;
    dir.div(len);
    collide::rq_result rq;
    return !Level().ObjectSpace.RayPick(from, dir, len, collide::rqtStatic, rq, nullptr) || rq.range >= len - 0.3f;
}

bool CBurer::nlc_set_move(const Fvector& wanted, u32 node, bool run, LPCSTR kind)
{
    Fvector pos = wanted;
    if (!ai().level_graph().valid_vertex_id(node) || !movement().restrictions().accessible(pos))
    {
        if (!nlc_point_on_map(wanted, pos, node))
            return false;
    }
    if (pos.distance_to_xz(Position()) < 2.f)
        return false;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    m_nlc_move_target = pos;
    m_nlc_move_node = node;
    m_nlc_move_run = run;
    m_nlc_move_kind = kind;
    m_nlc_move_until = Device.dwTimeGlobal + 8000;
    m_nlc_move_active = true;
    m_nlc_move_start_dist = enemy ? Position().distance_to(enemy->Position()) : 0.f;
    nlc_log("move %s %.1f m away, %.1f m from the enemy (%s)", kind, Position().distance_to(pos), enemy ? pos.distance_to(enemy->Position()) : -1.f,
        run ? "run" : "walk");
    return true;
}

// arrival bookkeeping, called when the tactical move completes (also from its finalize)
void CBurer::nlc_on_move_end()
{
    if (!m_nlc_move_active)
        return;
    m_nlc_move_active = false;

    const u32 now = Device.dwTimeGlobal;
    const bool arrived = Position().distance_to_xz(m_nlc_move_target) < 1.5f;
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    const float dist = enemy ? Position().distance_to(enemy->Position()) : 0.f;
    const bool cover = !xr_strcmp(m_nlc_move_kind, "cover") || !xr_strcmp(m_nlc_move_kind, "advance cover");
    const bool overwatch = !xr_strcmp(m_nlc_move_kind, "overwatch");
    const bool siege_hide = !xr_strcmp(m_nlc_move_kind, "siege hide");

    // in cover: hold there a moment (the puppet gun, the threat shield and the abilities keep working)
    if (cover)
        m_nlc_hold_until = now + u32(::Random.randI(int(m_nlc_cover_hold_min), int(m_nlc_cover_hold_max) + 1));
    // overwatch: hold while it sees the enemy (nlc_elev_update ends it when the line is lost); hidden: wait longer
    if (overwatch)
    {
        m_nlc_overwatch_settle_until = now + 2500;
        m_nlc_elev_unseen_since = 0;
    }
    if (overwatch || siege_hide)
        m_nlc_hold_until = now + u32(float(::Random.randI(int(m_nlc_overwatch_hold_min), int(m_nlc_overwatch_hold_max) + 1)) * (siege_hide ? 1.5f : 1.f));
    // an advance by cover that gained less than 3 m: straight advances for a while
    if (!xr_strcmp(m_nlc_move_kind, "advance cover") && m_nlc_move_start_dist - dist < 3.f)
        m_nlc_advance_straight_until = now + 15000;

    nlc_log("move %s %s, %.1f m from the enemy (was %.1f)%s", m_nlc_move_kind, arrived ? "reached" : "ended", dist, m_nlc_move_start_dist,
        cover || overwatch || siege_hide ? ", holding" : "");
}

bool CBurer::nlc_brain_pick_move(bool want_cover, bool hurt, bool ready)
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !CoverMan)
        return false;
    const u32 now = Device.dwTimeGlobal;
    const Fvector enemy_pos = enemy->Position();
    const float dist = Position().distance_to(enemy_pos);

    // cover: when hurt at once, otherwise not again right after the last cover move
    if (want_cover && (hurt || now >= m_nlc_last_cover_move + m_nlc_cover_retry))
    {
        const CCoverPoint* point = CoverMan->find_cover(enemy_pos, m_nlc_cover_min, m_nlc_cover_max);
        if (point && nlc_set_move(point->position(), point->level_vertex_id(), true, "cover"))
        {
            m_nlc_last_cover_move = now;
            return true;
        }
    }

    // an enemy up on a roof: a point with a line of sight (or a hidden spot nearby); no advance under the wall
    if (m_nlc_elev)
    {
        if (nlc_overwatch_pick())
            return true;
    }
    else if (dist > m_nlc_engage_max)
    {
        // advance: a cover point clearly closer to the enemy (unless that just failed), else straight into engage range
        if (now >= m_nlc_advance_straight_until)
        {
            const CCoverPoint* point = CoverMan->find_cover(enemy_pos, m_nlc_cover_min, _max(m_nlc_cover_min + 1.f, dist - 6.f));
            if (point && point->position().distance_to(enemy_pos) < dist - 4.f && nlc_set_move(point->position(), point->level_vertex_id(), true, "advance cover"))
                return true;
        }
        Fvector dir;
        dir.sub(Position(), enemy_pos);
        dir.y = 0.f;
        dir.normalize_safe();
        Fvector wanted;
        wanted.mad(enemy_pos, dir, m_nlc_engage_max - 5.f);
        wanted.y = Position().y;
        return nlc_set_move(wanted, ai().level_graph().vertex_id(wanted), true, "advance");
    }

    // in range but no line of sight, and something to use from there: peek to a nearby point that sees the enemy
    if (ready && !EnemyMan.see_enemy_now())
    {
        const float start = ::Random.randF(0.f, PI_MUL_2);
        for (float radius : {4.f, 7.f})
        {
            for (int i = 0; i < 8; ++i)
            {
                const float angle = start + float(i) * PI_DIV_4;
                Fvector c = Fvector().set(Position().x + _cos(angle) * radius, Position().y, Position().z + _sin(angle) * radius);
                const u32 node = ai().level_graph().vertex_id(c);
                if (!ai().level_graph().valid_vertex_id(node))
                    continue;
                c = ai().level_graph().vertex_position(node);
                if (!movement().restrictions().accessible(c) || !nlc_los_to(c, enemy, 1.5f))
                    continue;
                if (nlc_set_move(c, node, false, "peek"))
                    return true;
            }
        }
        // grudge hunt with no peek point nearby (behind a big rock): walk toward the last known position, stopping
        // short at casting range, then peek again from there
        if (nlc_grudge_hunt())
        {
            const Fvector last = EnemyMan.get_enemy_position();
            Fvector dir;
            dir.sub(Position(), last);
            dir.y = 0.f;
            const float d = dir.magnitude();
            if (d > 10.f)
            {
                dir.div(d);
                Fvector wanted;
                wanted.mad(last, dir, 8.f);
                wanted.y = last.y;
                if (nlc_set_move(wanted, ai().level_graph().vertex_id(wanted), false, "search"))
                    return true;
            }
        }
    }
    return false;
}
