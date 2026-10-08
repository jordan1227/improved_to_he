#include "stdafx.h"
#include "control_critical_wound.h"
#include "BaseMonster/base_monster.h"
#include "control_animation_base.h"
#include "control_direction_base.h"
#include "control_movement_base.h"

void CControlCriticalWound::activate()
{
    m_man->capture_pure(this);
    m_man->subscribe(this, ControlCom::eventAnimationEnd);

    m_man->path_stop(this);
    m_man->move_stop(this);
    m_man->dir_stop(this);

    IKinematicsAnimated* skel = smart_cast<IKinematicsAnimated*>(m_object->Visual());

    SControlAnimationData* ctrl_anim = (SControlAnimationData*)m_man->data(this, ControlCom::eControlAnimation);
    VERIFY(ctrl_anim);
    const MotionID motion = skel->ID_Cycle_Safe(m_data.animation);
    ctrl_anim->global.set_motion(motion);
    ctrl_anim->global.actual = false;

    // NLC: optional slower/faster playback (absolute blend speed, as in CAnimationSequencer)
    if (!fsimilar(m_data.speed_k, 1.f) && motion.valid())
        ctrl_anim->set_speed(skel->LL_GetMotionDef(motion)->Speed() * m_data.speed_k);
    else
        ctrl_anim->set_speed(-1.f);
}

void CControlCriticalWound::on_release()
{
    // NLC: drop the speed override before giving the animation back
    if (SControlAnimationData* ctrl_anim = (SControlAnimationData*)m_man->data(this, ControlCom::eControlAnimation))
        ctrl_anim->set_speed(-1.f);

    m_man->release_pure(this);
    m_man->unsubscribe(this, ControlCom::eventAnimationEnd);

    m_object->critical_wounded_state_stop();
}

bool CControlCriticalWound::check_start_conditions()
{
    if (is_active())
        return false;
    if (m_man->is_captured_pure())
        return false;

    return true;
}

void CControlCriticalWound::on_event(ControlCom::EEventType type, ControlCom::IEventData* dat)
{
    switch (type)
    {
    case ControlCom::eventAnimationEnd: m_man->notify(ControlCom::eventCriticalWoundEnd, 0); break;
    }
}
