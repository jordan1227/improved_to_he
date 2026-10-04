#include "stdafx.h"
#include "control_sequencer.h"
#include "control_manager.h"
#include "BaseMonster/base_monster.h" // NLC: speed_k (visual, motion defs)
#include "../../../Include/xrRender/KinematicsAnimated.h"

void CAnimationSequencer::reset_data()
{
    m_data.motions.clear();
    m_data.speed_k = 1.f; // NLC
}

void CAnimationSequencer::on_capture()
{
    m_man->capture_pure(this);
    m_man->subscribe(this, ControlCom::eventAnimationEnd);

    m_man->path_stop(this);
    m_man->move_stop(this);
    m_man->dir_stop(this);
}

void CAnimationSequencer::activate()
{
    m_index = 0;
    play_selected();
}

void CAnimationSequencer::on_event(ControlCom::EEventType type, ControlCom::IEventData* data)
{
    if (type == ControlCom::eventAnimationEnd)
    {
        if (m_index + 1 < m_data.motions.size())
        {
            m_index++;
            play_selected();
        }
        else
        {
            m_man->notify(ControlCom::eventSequenceEnd, 0);
        }
        return;
    }
}

void CAnimationSequencer::on_release()
{
    // NLC: drop the speed override before giving the animation back
    if (SControlAnimationData* ctrl_data = (SControlAnimationData*)m_man->data(this, ControlCom::eControlAnimation))
        ctrl_data->set_speed(-1.f);

    m_man->release_pure(this);
    m_man->unsubscribe(this, ControlCom::eventAnimationEnd);
}

void CAnimationSequencer::play_selected()
{
    // start new animation
    SControlAnimationData* ctrl_data = (SControlAnimationData*)m_man->data(this, ControlCom::eControlAnimation);
    VERIFY(ctrl_data);

    ctrl_data->global.set_motion(m_data.motions[m_index]);
    ctrl_data->global.actual = false;

    // NLC: optional faster/slower playback (absolute blend speed, as in CControlThreaten)
    IKinematicsAnimated* skel = smart_cast<IKinematicsAnimated*>(m_object->Visual());
    if (!fsimilar(m_data.speed_k, 1.f) && skel && m_data.motions[m_index].valid())
        ctrl_data->set_speed(skel->LL_GetMotionDef(m_data.motions[m_index])->Speed() * m_data.speed_k);
    else
        ctrl_data->set_speed(-1.f);
}

bool CAnimationSequencer::check_start_conditions()
{
    if (is_active())
        return false;
    if (m_man->is_captured_pure())
        return false;

    return true;
}
