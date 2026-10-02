#pragma once

#include "state_custom_action_look.h"
#include "state_move_to_point.h"

#define TEMPLATE_SPECIALIZATION template <typename _Object>

#define CStateMonsterHearInterestingSoundAbstract CStateMonsterHearInterestingSound<_Object>

class CBaseMonster;
namespace nlc_stealth
{
u32 monster_inv_style(CBaseMonster* monster);
u32 monster_notice_serial(CBaseMonster* monster);
}

TEMPLATE_SPECIALIZATION
CStateMonsterHearInterestingSoundAbstract::CStateMonsterHearInterestingSound(_Object* obj) : inherited(obj)
{
    add_state(eStateHearInterestingSound_MoveToDest, xr_new<CStateMonsterMoveToPoint<_Object>>(obj));
    add_state(eStateHearInterestingSound_LookAround, xr_new<CStateMonsterCustomActionLook<_Object>>(obj));
}

TEMPLATE_SPECIALIZATION
void CStateMonsterHearInterestingSoundAbstract::initialize()
{
    inherited::initialize();
    m_nlc_serial = nlc_stealth::monster_notice_serial(object);
}

// NLC M4: a new investigate impulse (near miss, shot, sight) restarts the walk toward the new point
TEMPLATE_SPECIALIZATION
void CStateMonsterHearInterestingSoundAbstract::check_force_state()
{
    const u32 serial = nlc_stealth::monster_notice_serial(object);
    if (serial == m_nlc_serial)
        return;
    m_nlc_serial = serial;
    if (current_substate != u32(-1))
    {
        get_state_current()->critical_finalize();
        current_substate = u32(-1);
    }
    prev_substate = u32(-1);
}

TEMPLATE_SPECIALIZATION
void CStateMonsterHearInterestingSoundAbstract::reselect_state()
{
    if (prev_substate == u32(-1))
    {
        // NLC M3: "hold" style watches the point without moving
        if (nlc_stealth::monster_inv_style(object) == 3)
        {
            select_state(eStateHearInterestingSound_LookAround);
            return;
        }
        if (get_state(eStateHearInterestingSound_MoveToDest)->check_start_conditions())
            select_state(eStateHearInterestingSound_MoveToDest);
        else
            select_state(eStateHearInterestingSound_LookAround);
        return;
    }

    select_state(eStateHearInterestingSound_LookAround);
}

TEMPLATE_SPECIALIZATION
void CStateMonsterHearInterestingSoundAbstract::setup_substates()
{
    state_ptr state = get_state_current();

    if (current_substate == eStateHearInterestingSound_MoveToDest)
    {
        SStateDataMoveToPoint data;
        data.point = get_target_position();
        data.vertex = u32(-1);
        data.action.action = ACT_WALK_FWD;
        data.accelerated = true;
        data.braking = false;
        data.accel_type = eAT_Calm;
        // NLC M3: investigate style (sneak, run); walk otherwise
        switch (nlc_stealth::monster_inv_style(object))
        {
        case 2: data.action.action = ACT_STEAL; break;
        case 4:
            data.action.action = ACT_RUN;
            data.accel_type = eAT_Aggressive;
            break;
        default: break;
        }
        data.completion_dist = 2.f;
        data.action.sound_type = MonsterSound::eMonsterSoundIdle;
        data.action.sound_delay = object->db().m_dwIdleSndDelay;

        state->fill_data_with(&data, sizeof(SStateDataMoveToPoint));

        return;
    }

    if (current_substate == eStateHearInterestingSound_LookAround)
    {
        SStateDataActionLook data;
        data.action = ACT_LOOK_AROUND;
        data.sound_type = MonsterSound::eMonsterSoundIdle;
        data.sound_delay = object->db().m_dwIdleSndDelay;

        Fvector dir;
        object->CoverMan->less_cover_direction(dir);
        data.point.mad(object->Position(), dir, 10.f);
        if (nlc_stealth::monster_inv_style(object) == 3) // NLC M3: hold and watch the sound
            data.point = object->SoundMemory.GetSound().position;

        state->fill_data_with(&data, sizeof(SStateDataActionLook));

        return;
    }
}

TEMPLATE_SPECIALIZATION
Fvector CStateMonsterHearInterestingSoundAbstract::get_target_position()
{
    Fvector snd_pos = object->SoundMemory.GetSound().position;
    if (!object->Home->has_home())
        return snd_pos;

    if (object->Home->at_home(snd_pos))
        return snd_pos;

    return ai().level_graph().vertex_position(object->Home->get_place());
}

#undef TEMPLATE_SPECIALIZATION
#undef CStateMonsterHearInterestingSoundAbstract
