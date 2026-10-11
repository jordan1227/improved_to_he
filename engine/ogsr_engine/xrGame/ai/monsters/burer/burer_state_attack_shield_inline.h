#pragma once

#include "../../stalker/ai_stalker.h"

template <class Object>
CStateBurerShield<Object>::CStateBurerShield(Object* obj) : inherited(obj)
{
    m_last_shield_started = 0;
    m_shield_start_anim_length_sec = 0;
    m_next_particle_allowed = 0;
}

template <class Object>
void CStateBurerShield<Object>::initialize()
{
    inherited::initialize();
    object->set_script_capture(false);
    m_last_shield_started = current_time();
    m_next_particle_allowed = 0;
    m_started = false;

    // NLC: one cooldown with the threat shield; the state takes over a running threat shield
    object->nlc_overlay_shield_stop("shield state");
    object->m_nlc_shield_next = Device.dwTimeGlobal + u32(object->m_shield_time + object->m_shield_cooldown);
    object->m_nlc_shield_strain = 0.f;
    object->m_nlc_shield_ends = Device.dwTimeGlobal + u32(object->m_shield_time);

    MotionID motion;
    object->anim().get_animation_info(eAnimShieldStart, 0, motion, m_shield_start_anim_length_sec);

    object->nlc_log("shield up hp=%.2f", object->conditions().GetHealth());
}

template <class Object>
void CStateBurerShield<Object>::execute()
{
    if (!m_started) // && current_time() > m_last_shield_started + TTime(m_shield_start_anim_length_sec*1000) )
    {
        m_started = true;
        object->ActivateShield();
    }

    if (m_started && object->m_shield_keep_particle != 0 && current_time() > m_next_particle_allowed)
    {
        object->CParticlesPlayer::StartParticles(object->m_shield_keep_particle, Fvector().set(0, 1, 0), object->ID(), -1, true);

        m_next_particle_allowed = current_time() + object->m_shield_keep_particle_period;
    }

    object->face_enemy();
    object->set_action(ACT_STAND_IDLE);

    object->anim().set_override_animation(m_started ? eAnimShieldContinue : eAnimShieldStart);
}

template <class Object>
void CStateBurerShield<Object>::finalize()
{
    object->nlc_log("shield down after %u ms reload=%d", u32(current_time() - m_last_shield_started), actor_is_reloading_weapon() ? 1 : 0);
    inherited::finalize();
    object->DeactivateShield();
    object->set_script_capture(true);
}

template <class Object>
void CStateBurerShield<Object>::critical_finalize()
{
    inherited::critical_finalize();
    object->DeactivateShield();
    object->set_script_capture(false);
}

template <class Object>
bool CStateBurerShield<Object>::check_start_conditions()
{
    if (current_time() < m_last_shield_started + object->m_shield_time + object->m_shield_cooldown)
        return false;
    if (!object->nlc_shield_allowed())
        return false;

    const CEntityAlive* enemy = object->EnemyMan.get_enemy();
    const CAI_Stalker* stalker = smart_cast<const CAI_Stalker*>(enemy);
    if (!enemy || !(stalker || enemy == Actor()))
        return false;

    if (!object->EnemyMan.enemy_see_me_now())
        return false;

    return true;
}

template <class Object>
bool CStateBurerShield<Object>::check_completion()
{
    if (current_time() > m_last_shield_started + object->m_shield_time)
        return true;

    // NLC: broken by strain (CBurer::nlc_on_shield_blocked)
    if (m_started && !object->m_shield_active)
        return true;

    CEntityAlive const* enemy = object->EnemyMan.get_enemy();
    if (!enemy)
        return true;

    if (enemy == Actor())
    {
        if (actor_is_reloading_weapon())
            return true;
    }

    if (!object->EnemyMan.get_enemy())
        return true;

    return false;
}
