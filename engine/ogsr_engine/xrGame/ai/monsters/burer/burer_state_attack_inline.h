#pragma once

#include "burer_state_attack_tele.h"
#include "burer_state_attack_gravi.h"
#include "burer_state_attack_shield.h"
#include "burer_state_attack_melee.h"
#include "../states/state_look_point.h"
#include "../states/state_move_to_restrictor.h"
#include "burer_state_attack_run_around.h"
#include "burer_state_attack_antiaim.h"
#include "burer_state_attack_repulse.h"
#include "burer_state_attack_tactical.h"
#include "burer_state_attack_cast.h"

namespace burer
{
float const health_delta = 0.01f;

inline LPCSTR nlc_state_name(u32 state)
{
    switch (state)
    {
    case eStateBurerAttack_Tele: return "tele";
    case eStateBurerAttack_Gravi: return "gravi";
    case eStateBurerAttack_Melee: return "melee";
    case eStateBurerAttack_FaceEnemy: return "face_enemy";
    case eStateBurerAttack_RunAround: return "run_around";
    case eStateBurerAttack_Shield: return "shield";
    case eStateBurerAttack_AntiAim: return "anti_aim";
    case eStateBurerAttack_Repulse: return "repulse";
    case eStateAttack_Run: return "approach";
    case eStateCustomMoveToRestrictor: return "to_restrictor";
    case eStateBurerAttack_Tactical: return "tactical";
    case eStateBurerAttack_Stare: return "stare";
    case eStateBurerAttack_NpcDisarm: return "npc_disarm";
    case eStateBurerAttack_Pull: return "pull";
    default: return "none";
    }
}

} // namespace burer

template <typename Object>
CStateBurerAttack<Object>::CStateBurerAttack(Object* obj) : inherited(obj)
{
    add_state(eStateBurerAttack_Tele, xr_new<CStateBurerAttackTele<Object>>(obj));
    add_state(eStateBurerAttack_Gravi, xr_new<CStateBurerAttackGravi<Object>>(obj));
    add_state(eStateBurerAttack_Melee, xr_new<CStateBurerAttackMelee<Object>>(obj));
    add_state(eStateBurerAttack_FaceEnemy, xr_new<CStateMonsterLookToPoint<Object>>(obj));
    add_state(eStateBurerAttack_RunAround, xr_new<CStateBurerAttackRunAround<Object>>(obj));
    add_state(eStateBurerAttack_Shield, xr_new<CStateBurerShield<Object>>(obj));
    add_state(eStateBurerAttack_AntiAim, xr_new<CStateBurerAntiAim<Object>>(obj));
    add_state(eStateBurerAttack_Repulse, xr_new<CStateBurerRepulse<Object>>(obj));
    add_state(eStateBurerAttack_Tactical, xr_new<CStateBurerTactical<Object>>(obj));
    add_state(eStateBurerAttack_Stare, xr_new<CStateBurerCast<Object>>(obj, u32(CBurer::eNlcCastStare)));
    add_state(eStateBurerAttack_NpcDisarm, xr_new<CStateBurerCast<Object>>(obj, u32(CBurer::eNlcCastNpcDisarm)));
    add_state(eStateBurerAttack_Pull, xr_new<CStateBurerCast<Object>>(obj, u32(CBurer::eNlcCastPull)));
    add_state(eStateAttack_Run, xr_new<CStateMonsterAttackRun<Object>>(obj));
    add_state(eStateCustomMoveToRestrictor, xr_new<CStateMonsterMoveToRestrictor<Object>>(obj));

    m_allow_anti_aim = false;
    m_wait_state_end = false;
    m_lost_delta_health = false;
}

template <typename Object>
void CStateBurerAttack<Object>::initialize()
{
    inherited::initialize();

    m_last_health = object->conditions().GetHealth();
    m_lost_delta_health = false;
    m_next_runaway_allowed_tick = 0;
    m_allow_anti_aim = false;
    m_wait_state_end = false;
    m_nlc_logged_substate = u32(-1);

    const CEntityAlive* enemy = object->EnemyMan.get_enemy();
    object->nlc_log("attack start enemy=%s dist=%.1f hp=%.2f", enemy ? enemy->cName().c_str() : "-", enemy ? enemy->Position().distance_to(object->Position()) : -1.f,
        object->conditions().GetHealth());

    // 	CMonsterSquad *squad					=	monster_squad().get_squad(object);
    // 	if ( squad )
    // 	{
    // 		squad->InformSquadAboutEnemy			(object->EnemyMan.get_enemy());
    // 	}
}

template <typename Object>
void CStateBurerAttack<Object>::execute()
{
    CEntityAlive* enemy = const_cast<CEntityAlive*>(object->EnemyMan.get_enemy());

    // NLC: casting / channelling / shielding: no passive deflection then (CBurer::nlc_try_deflect)
    object->m_nlc_busy = current_substate == eStateBurerAttack_Tele || current_substate == eStateBurerAttack_Gravi ||
        current_substate == eStateBurerAttack_Repulse || current_substate == eStateBurerAttack_AntiAim || current_substate == eStateBurerAttack_Shield ||
        current_substate == eStateBurerAttack_Stare || current_substate == eStateBurerAttack_NpcDisarm || current_substate == eStateBurerAttack_Pull;

    if (current_substate != m_nlc_logged_substate)
    {
        m_nlc_logged_substate = current_substate;
        object->nlc_log("state %s dist=%.1f hp=%.2f see=%d", burer::nlc_state_name(current_substate), enemy ? enemy->Position().distance_to(object->Position()) : -1.f,
            object->conditions().GetHealth(), object->EnemyMan.see_enemy_now() ? 1 : 0);
    }

    // Notify squad
    CMonsterSquad* squad = monster_squad().get_squad(object);
    if (squad)
    {
        SMemberGoal goal;
        goal.type = MG_AttackEnemy;
        goal.entity = enemy;
        squad->UpdateGoal(object, goal);
    }

    if (object->anim().has_override_animation())
    {
        object->anim().clear_override_animation();
    }

    if (object->conditions().GetHealth() <= m_last_health - burer::health_delta)
    {
        m_last_health = object->conditions().GetHealth();
        m_lost_delta_health = true;
    }

    if (m_wait_state_end)
    {
        if (get_state_current()->check_completion())
        {
            m_wait_state_end = false;
        }
        else
        {
            get_state_current()->execute();
            prev_substate = current_substate;
            return;
        }
    }

    m_allow_anti_aim = true;
    bool const anti_aim_ready = get_state(eStateBurerAttack_AntiAim)->check_start_conditions();
    m_allow_anti_aim = false;

    bool const repulse_ready = get_state(eStateBurerAttack_Repulse)->check_start_conditions();
    bool const pull_ready = get_state(eStateBurerAttack_Pull)->check_start_conditions();
    bool const gravi_ready = get_state(eStateBurerAttack_Gravi)->check_start_conditions();
    bool const shield_ready = get_state(eStateBurerAttack_Shield)->check_start_conditions();
    bool const npc_disarm_ready = get_state(eStateBurerAttack_NpcDisarm)->check_start_conditions();
    bool const stare_ready = get_state(eStateBurerAttack_Stare)->check_start_conditions();
    bool const tele_ready = get_state(eStateBurerAttack_Tele)->check_start_conditions();

    bool selected_state = true;

    if (repulse_ready)
    {
        select_state(eStateBurerAttack_Repulse);
    }
    else if (pull_ready)
    {
        select_state(eStateBurerAttack_Pull);
    }
    else if (gravi_ready)
    {
        select_state(eStateBurerAttack_Gravi);
    }
    else if (m_lost_delta_health && shield_ready)
    {
        m_lost_delta_health = false;
        select_state(eStateBurerAttack_Shield);
    }
    else if (npc_disarm_ready)
    {
        select_state(eStateBurerAttack_NpcDisarm);
    }
    else if (anti_aim_ready)
    {
        select_state(eStateBurerAttack_AntiAim);
    }
    else if (stare_ready)
    {
        select_state(eStateBurerAttack_Stare);
    }
    else if (tele_ready && current_substate != eStateBurerAttack_RunAround)
    {
        select_state(eStateBurerAttack_Tele);
    }
    else
    {
        selected_state = false;
    }

    if (selected_state)
    {
        get_state_current()->execute();
        m_wait_state_end = true;
        prev_substate = current_substate;
        return;
    }

    Fvector const enemy_pos = enemy->Position();
    Fvector const self_pos = object->Position();
    Fvector const self2enemy = enemy_pos - self_pos;
    float const self2enemy_dist = magnitude(self2enemy);

    bool const in_runaway_range = self2enemy_dist < object->m_runaway_distance;
    bool const in_normal_range = self2enemy_dist < object->m_normal_distance;

    if (current_substate == eStateCustomMoveToRestrictor)
    {
        if (!get_state_current()->check_completion())
        {
            get_state_current()->execute();
            prev_substate = current_substate;
            return;
        }
    }

    if (get_state(eStateCustomMoveToRestrictor)->check_start_conditions())
    {
        select_state(eStateCustomMoveToRestrictor);
        get_state_current()->execute();
        prev_substate = current_substate;
        return;
    }

    // NLC brain: cover when hurt (or wounded with nothing ready), cover hops when far, a peek when blind
    if (current_substate == eStateBurerAttack_Tactical)
    {
        if (!get_state_current()->check_completion())
        {
            get_state_current()->execute();
            prev_substate = current_substate;
            return;
        }
        object->nlc_on_move_end(); // the state is re-selected below: its finalize would not run
    }
    if (object->m_nlc_brain && !in_runaway_range)
    {
        const u32 now = Device.dwTimeGlobal;
        const bool hurt = m_lost_delta_health || object->m_nlc_force_cover;
        const bool hit_recently = object->HitMemory.is_hit() && object->HitMemory.get_last_hit_time() + 2500 > now;
        const bool ready = gravi_ready || tele_ready || object->nlc_puppet_active() || object->nlc_grudge_hunt();
        const bool holding = now < object->m_nlc_hold_until;
        const bool want_cover = hurt || (object->nlc_phase() >= 1 && hit_recently) || (object->nlc_phase() >= 1 && !ready && !holding && object->EnemyMan.see_enemy_now());
        if ((now >= object->m_nlc_move_next || hurt) && (!holding || hurt) && now >= object->m_nlc_pick_retry)
        {
            if (object->nlc_brain_pick_move(want_cover, hurt, ready))
            {
                m_lost_delta_health = false;
                object->m_nlc_force_cover = false;
                object->m_nlc_move_next = now + 3000;
                select_state(eStateBurerAttack_Tactical);
                get_state_current()->execute();
                prev_substate = current_substate;
                return;
            }
            // nothing found (cover, overwatch, peek searches cost rays): not every frame
            object->m_nlc_pick_retry = now + 750;
        }
    }

    if (current_substate == eStateBurerAttack_RunAround)
    {
        if (get_state_current()->check_completion())
        {
            if (in_runaway_range)
            {
                m_next_runaway_allowed_tick = current_time() + 5000;
            }
        }
        else
        {
            get_state_current()->execute();
            prev_substate = current_substate;
            return;
        }
    }

    if (m_lost_delta_health || (in_runaway_range && current_time() > m_next_runaway_allowed_tick))
    {
        m_lost_delta_health = false;
        select_state(eStateBurerAttack_RunAround);
    }
    else if (!in_normal_range && !(object->m_nlc_brain && Device.dwTimeGlobal < object->m_nlc_hold_until) && !object->m_nlc_elev)
    {
        select_state(eStateAttack_Run);
    }
    else
    {
        Fvector const self2enemy = enemy_pos - self_pos;
        bool const good_aiming = angle_between_vectors(self2enemy, object->Direction()) < deg2rad(20.f);

        select_state(eStateBurerAttack_FaceEnemy);

        if (!good_aiming)
        {
            bool const rotate_right = object->control().direction().is_from_right(enemy_pos);
            object->anim().set_override_animation(rotate_right ? eAnimStandTurnRight : eAnimStandTurnLeft, 0);
            object->dir().face_target(enemy_pos);
        }

        object->set_action(ACT_STAND_IDLE);
        return;
    }

    get_state_current()->execute();
    prev_substate = current_substate;
}

template <typename Object>
void CStateBurerAttack<Object>::finalize()
{
    object->m_nlc_busy = false;
    object->nlc_log("attack end hp=%.2f", object->conditions().GetHealth());
    if (object->anim().has_override_animation())
    {
        object->anim().clear_override_animation();
    }

    inherited::finalize();
}

template <typename Object>
void CStateBurerAttack<Object>::critical_finalize()
{
    object->m_nlc_busy = false; // panic / hit reaction: the passive deflection works again
    if (object->anim().has_override_animation())
    {
        object->anim().clear_override_animation();
    }

    inherited::critical_finalize();
}

template <typename Object>
bool CStateBurerAttack<Object>::check_control_start_conditions(ControlCom::EControlType type)
{
    if (type == ControlCom::eAntiAim)
    {
        return m_allow_anti_aim;
    }

    return true;
}