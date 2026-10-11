#pragma once

template <typename Object>
CStateBurerRepulse<Object>::CStateBurerRepulse(Object* obj) : inherited(obj)
{
}

template <typename Object>
void CStateBurerRepulse<Object>::initialize()
{
    inherited::initialize();

    // the cast clip is the gravi one (stand_power_attack_0 on the shipped visuals)
    const float length = object->anim().get_animation_length(eAnimGraviFire, 0);
    const TTime now = current_time();
    m_blast_tick = now + TTime(length * object->m_nlc_repulse.hit_time * 1000.f);
    m_end_tick = now + TTime(length * 1000.f);
    m_blast_done = false;

    object->m_nlc_repulse_next = Device.dwTimeGlobal + u32(object->m_nlc_repulse.cooldown * object->nlc_phase_k(1.f, 1.f, 0.6f));
    object->set_script_capture(false);
    object->sound().play(CBurer::eMonsterSoundGraviAttack);
}

template <typename Object>
void CStateBurerRepulse<Object>::execute()
{
    if (!m_blast_done && current_time() >= m_blast_tick)
    {
        m_blast_done = true;
        object->nlc_repulse_blast();
    }

    object->face_enemy();
    object->anim().set_override_animation(eAnimGraviFire);
    object->set_action(ACT_STAND_IDLE);
}

template <typename Object>
void CStateBurerRepulse<Object>::finalize()
{
    inherited::finalize();
    object->set_script_capture(true);
}

template <typename Object>
void CStateBurerRepulse<Object>::critical_finalize()
{
    inherited::critical_finalize();
    object->set_script_capture(false);
}

template <typename Object>
bool CStateBurerRepulse<Object>::check_start_conditions()
{
    return object->nlc_repulse_ready();
}

template <typename Object>
bool CStateBurerRepulse<Object>::check_completion()
{
    return m_blast_done && current_time() > m_end_tick;
}
