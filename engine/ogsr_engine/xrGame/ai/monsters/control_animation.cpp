#include "stdafx.h"
#include "control_animation.h"
#include "BaseMonster/base_monster.h"
#include "control_manager.h"
#include "control_animation_base.h" // NLC flytrap
#include "control_movement.h"
#include <typeinfo>


//#ifdef DEBUG
//#include "control_animation_base.h"
//#endif

void SAnimationPart::set_motion(MotionID const& m)
{
    VERIFY(m.valid());
    motion = m;
}

void CControlAnimation::reinit()
{
    inherited::reinit();

    m_skeleton_animated = smart_cast<IKinematicsAnimated*>(m_object->Visual());

    m_anim_events.clear();

    m_global_animation_end = false;
    m_legs_animation_end = false;
    m_torso_animation_end = false;

    m_freeze = false;
}

void CControlAnimation::reset_data()
{
    m_data.global.init();
    m_data.legs.init();
    m_data.torso.init();
    m_data.set_speed(-1.f);
}

void CControlAnimation::update_frame()
{
    if (m_freeze)
        return;

    // move to schedule update
    START_PROFILE("Base Monster/Animation/Update Tracks");
    m_skeleton_animated->UpdateTracks();
    STOP_PROFILE;

    START_PROFILE("Base Monster/Animation/Check callbacks");
    check_callbacks();
    STOP_PROFILE;

    START_PROFILE("Base Monster/Animation/Play");
    play();
    STOP_PROFILE;

    START_PROFILE("Base Monster/Animation/Check Events");
    check_events(m_data.global);
    check_events(m_data.torso);
    check_events(m_data.legs);
    STOP_PROFILE;
}

static void global_animation_end_callback(CBlend* B)
{
    CControlAnimation* controller = (CControlAnimation*)B->CallbackParam;
    controller->m_global_animation_end = true;
}
static void legs_animation_end_callback(CBlend* B)
{
    CControlAnimation* controller = (CControlAnimation*)B->CallbackParam;
    controller->m_legs_animation_end = true;
}
static void torso_animation_end_callback(CBlend* B)
{
    CControlAnimation* controller = (CControlAnimation*)B->CallbackParam;
    controller->m_torso_animation_end = true;
}

void CControlAnimation::play()
{
    if (!m_data.global.actual)
    {
        play_part(m_data.global, global_animation_end_callback);
        if (m_data.global.blend)
            m_saved_global_speed = m_data.global.blend->speed;
    }

    if (!m_data.legs.actual && m_data.legs.get_motion().valid())
        play_part(m_data.legs, legs_animation_end_callback);
    if (!m_data.torso.actual && m_data.torso.get_motion().valid())
        play_part(m_data.torso, torso_animation_end_callback);

    // speed only for global
    if (m_data.global.blend)
    {
        // NLC flytrap (docs/ANOMALY_DESIGN.md 20): the time factor is applied here, the last writer before playback.
        // An explicit speed from the base animation control (movement clips timed to the body, attacks, idles) is
        // given in the creature's own time; other controls that set one (jump glide) scale it themselves.
        const float tk = m_object->nlc_time_k();
        const bool log = Device.dwTimeGlobal < m_object->m_nlc_time_log_end && Device.dwTimeGlobal >= m_nlc_anim_log_next;
        // the capturer is only needed while slowed (or logging): no smart_cast per monster per frame outside a Flytrap
        CControl_Com* capturer = (tk < 0.999f || log) ? m_man->get_capturer(ControlCom::eControlAnimation) : nullptr;
        const bool base_owns = capturer && smart_cast<CControlAnimationBase*>(capturer);
        if (m_data.get_speed() > 0)
        {
            m_nlc_applied_tk = base_owns ? tk : 1.f;
            m_data.global.blend->speed = m_data.get_speed() * m_nlc_applied_tk; // TODO: make factor
        }
        else
        {
            // NLC flytrap: the clips of the other controls (jump, run attack, attack on the move) slow down too
            m_nlc_applied_tk = tk;
            m_data.global.blend->speed = m_saved_global_speed * tk;
        }

        if (log)
        {
            m_nlc_anim_log_next = Device.dwTimeGlobal + 500;
            Msg("~ [flytrap_anim] [%s] anim [%s] owner [%s] k %.2f move_k %.2f vel %.2f (path %.2f) set %.2f natural %.2f -> blend %.2f",
                m_object->cName().c_str(), m_object->anim().cur_anim_info().name.c_str(), capturer ? typeid(*capturer).name() : "none", tk,
                m_object->nlc_move_speed_k(), m_man->movement().real_velocity(), m_man->movement().velocity_current(), m_data.get_speed(),
                m_saved_global_speed, m_data.global.blend->speed);
        }
    }
}

void CControlAnimation::play_part(SAnimationPart& part, PlayCallback callback)
{
    VERIFY(part.get_motion().valid());

    u16 bone_or_part = m_skeleton_animated->LL_GetMotionDef(part.get_motion())->bone_or_part;
    if (bone_or_part == u16(-1))
        bone_or_part = m_skeleton_animated->LL_PartID("default");

    // initialize synchronization of prev and current animation
    float pos = -1.f;
    if (part.blend && !part.blend->stop_at_end)
        pos = fmod(part.blend->timeCurrent, part.blend->timeTotal) / part.blend->timeTotal;
#ifdef DEBUG
        // IKinematicsAnimated * K = m_object->Visual()->dcast_PKinematicsAnimated();
        // Msg				("%6d Playing animation : %s , %s , Object %s",Device.dwTimeGlobal, K->LL_MotionDefName_dbg(part.motion).first,K->LL_MotionDefName_dbg(part.motion).second,
        // *(m_object->cName()));
#endif

    part.blend = m_skeleton_animated->LL_PlayCycle(bone_or_part, part.get_motion(), TRUE, callback, this);

    ///////////////////////////////////////////////////////////////////////////////
    //#ifdef DEBUG
    //	Msg("Monster[%s] Time[%u] Anim[%s]",*(m_object->cName()), Device.dwTimeGlobal,*(m_object->anim().GetAnimTranslation(part.motion)));
    //#endif
    ///////////////////////////////////////////////////////////////////////////////

    // synchronize prev and current animations
    if ((pos > 0) && part.blend && !part.blend->stop_at_end)
    {
        part.blend->timeCurrent = part.blend->timeTotal * pos;
    }

    part.time_started = Device.dwTimeGlobal;
    part.actual = true;

    m_man->notify(ControlCom::eventAnimationStart, 0);

    if ((part.get_motion() != m_data.torso.get_motion()) && part.blend)
        m_object->CStepManager::on_animation_start(part.get_motion(), part.blend);

    auto it = m_anim_events.find(part.get_motion());
    if (it != m_anim_events.end())
    {
        for (auto event_it = it->second.begin(); event_it != it->second.end(); ++event_it)
        {
            event_it->handled = false;
        }
    }
}

void CControlAnimation::add_anim_event(MotionID motion, float time_perc, u32 id)
{
    // if there is already event with exact timing - return
    auto it = m_anim_events.find(motion);
    if (it != m_anim_events.end())
    {
        ANIMATION_EVENT_VEC& anim_vec = it->second;

        for (auto I = anim_vec.begin(); I != anim_vec.end(); ++I)
        {
            if (fsimilar(I->time_perc, time_perc))
                return;
        }
    }

    SAnimationEvent event;
    event.time_perc = time_perc;
    event.event_id = id;

    m_anim_events[motion].push_back(event);
}

void CControlAnimation::check_events(SAnimationPart& part)
{
    if (part.get_motion().valid() && part.actual && part.blend)
    {
        auto it = m_anim_events.find(part.get_motion());
        if (it != m_anim_events.end())
        {
            float cur_perc = float(Device.dwTimeGlobal - part.time_started) / ((part.blend->timeTotal / part.blend->speed) * 1000);

            for (auto event_it = it->second.begin(); event_it != it->second.end(); ++event_it)
            {
                SAnimationEvent& event = *event_it;
                if (!event.handled && (event.time_perc < cur_perc))
                {
                    event.handled = true;

                    // gen event
                    SAnimationSignalEventData anim_event(part.get_motion(), event.time_perc, event.event_id);
                    m_man->notify(ControlCom::eventAnimationSignal, &anim_event);
                }
            }
        }
    }
}

void CControlAnimation::check_callbacks()
{
    if (m_global_animation_end)
    {
        m_man->notify(ControlCom::eventAnimationEnd, 0);
        m_global_animation_end = false;
    }

    if (m_legs_animation_end)
    {
        m_man->notify(ControlCom::eventLegsAnimationEnd, 0);
        m_legs_animation_end = false;
    }

    if (m_torso_animation_end)
    {
        m_man->notify(ControlCom::eventTorsoAnimationEnd, 0);
        m_torso_animation_end = false;
    }
}

void CControlAnimation::restart(SAnimationPart& part, PlayCallback callback)
{
    VERIFY(part.get_motion().valid());
    VERIFY(part.blend);

    u16 bone_or_part = m_skeleton_animated->LL_GetMotionDef(part.get_motion())->bone_or_part;
    if (bone_or_part == u16(-1))
        bone_or_part = m_skeleton_animated->LL_PartID("default");

    // save
    float time_saved = part.blend->timeCurrent;

    // start
    part.blend = m_skeleton_animated->LL_PlayCycle(bone_or_part, part.get_motion(), TRUE, callback, this);

    // restore
    part.blend->timeCurrent = time_saved;
}

void CControlAnimation::restart()
{
    m_skeleton_animated = smart_cast<IKinematicsAnimated*>(m_object->Visual());

    if (m_data.global.blend)
        restart(m_data.global, global_animation_end_callback);
    if (m_data.legs.blend)
        restart(m_data.legs, legs_animation_end_callback);
    if (m_data.torso.blend)
        restart(m_data.torso, torso_animation_end_callback);
}
void CControlAnimation::freeze()
{
    if (m_freeze)
        return;
    m_freeze = true;

    if (m_data.global.blend)
    {
        // NLC flytrap: saved in the creature's own time; play() applies the time factor again (it was k^2 after unfreeze)
        m_saved_global_speed = m_data.global.blend->speed / std::max(m_nlc_applied_tk, 0.01f);
        m_data.global.blend->speed = 0.f;
    }
    if (m_data.legs.blend)
    {
        m_saved_legs_speed = m_data.legs.blend->speed;
        m_data.legs.blend->speed = 0.f;
    }
    if (m_data.torso.blend)
    {
        m_saved_torso_speed = m_data.torso.blend->speed;
        m_data.torso.blend->speed = 0.f;
    }
}

void CControlAnimation::unfreeze()
{
    if (!m_freeze)
        return;
    m_freeze = false;

    if (m_data.global.blend)
    {
        m_data.global.blend->speed = m_saved_global_speed;
    }
    if (m_data.legs.blend)
    {
        m_data.legs.blend->speed = m_saved_legs_speed;
    }
    if (m_data.torso.blend)
    {
        m_data.torso.blend->speed = m_saved_torso_speed;
    }
}
