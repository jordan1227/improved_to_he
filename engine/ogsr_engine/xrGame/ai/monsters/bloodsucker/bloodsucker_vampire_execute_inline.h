#pragma once

#include "../../../../Include/xrRender/Kinematics.h"
#include "../../../Actor.h"
#include "../../../inventory.h"
#include "../../../../xr_3da/CameraBase.h"

#include "../../../HUDManager.h"

#define TEMPLATE_SPECIALIZATION template <typename _Object>

#define CStateBloodsuckerVampireExecuteAbstract CStateBloodsuckerVampireExecute<_Object>

//#define VAMPIRE_MIN_DIST		0.5f
//#define VAMPIRE_MAX_DIST		1.f

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::initialize()
{
    inherited::initialize();

    object->CControlledActor::install();

    // NLC: ambush grab: the camera may move slowly after the turn (a struggle), and one grab at a time over all bloodsuckers
    m_nlc_struggled = false;
    m_nlc_fired = false;
    if (object->m_nlc_vampire_ambush)
    {
        object->nlc_set_struggle_look(object->m_nlc_b.vampire_struggle_look);
        CAI_Bloodsucker::m_time_last_vampire = Device.dwTimeGlobal;
        object->nlc_backhit_grab_started(); // NLC: a back hit's rest of the damage waits for the grab's end
        // visible for the hold at once (the stock call below waits for the visibility delay); the triple animation
        // starts on the first execute, after this model swap
        object->nlc_set_cloak(CAI_Bloodsucker::full_visibility, true);
        if (CBaseMonster::nlc_evade_debug())
            Msg("~ [evade] [%s]: vampire: grab start", object->cName().c_str());
    }

    look_head();

    m_action = eActionPrepare;
    time_vampire_started = 0;

    object->set_visibility_state(CAI_Bloodsucker::full_visibility);

    object->m_hits_before_vampire = 0;
    object->m_sufficient_hits_before_vampire_random = -1 + (rand() % 3);

    HUD().GetUI()->HideGameIndicators();

    Actor()->inventory().SetSlotsBlocked(INV_STATE_BLOCK_ALL, true);

    // Actor()->set_inventory_disabled	(true);

    m_effector_activated = false;
    m_health_loss_activated = false;
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute()
{
    if (!object->CControlledActor::is_turning() && !m_effector_activated)
    {
        object->ActivateVampireEffector();
        m_effector_activated = true;
    }

    look_head();

    switch (m_action)
    {
    case eActionPrepare:
        execute_vampire_prepare();
        m_action = eActionContinue;
        break;

    case eActionContinue: execute_vampire_continue(); break;

    case eActionFire:
        execute_vampire_hit();
        m_action = eActionWaitTripleEnd;
        break;

    case eActionWaitTripleEnd:
        if (!object->com_man().ta_is_active())
        {
            m_action = eActionCompleted;
        }

    case eActionCompleted: break;
    }

    object->dir().face_target(object->EnemyMan.get_enemy());

    auto enemy_pos = object->EnemyMan.get_enemy()->Position();
    Fvector const enemy_to_self = enemy_pos.sub(object->Position());
    float const dist_to_enemy = enemy_to_self.magnitude();
    float const vampire_dist = object->get_vampire_distance();

    if (angle_between_vectors(object->Direction(), enemy_to_self) < deg2rad(20.f) && dist_to_enemy > vampire_dist)
    {
        object->set_action(ACT_RUN);
        object->anim().accel_activate(eAT_Aggressive);
        object->anim().accel_set_braking(false);

        u32 const target_vertex = object->EnemyMan.get_enemy()->ai_location().level_vertex_id();
        Fvector const target_pos = ai().level_graph().vertex_position(target_vertex);

        object->path().set_target_point(target_pos, target_vertex);
        object->path().set_rebuild_time(100);
        object->path().set_use_covers(false);
        object->path().set_distance_to_end(vampire_dist);
    }
    else
    {
        object->set_action(ACT_STAND_IDLE);
    }
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::show_hud()
{
    HUD().GetUI()->ShowGameIndicators();

    Actor()->inventory().SetSlotsBlocked(INV_STATE_BLOCK_ALL, false);
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::cleanup()
{
    // Actor()->set_inventory_disabled	(false);

    if (object->m_nlc_vampire_ambush && CBaseMonster::nlc_evade_debug())
        Msg("~ [evade] [%s]: vampire: grab end after %u ms (%s)", object->cName().c_str(), time_vampire_started ? Device.dwTimeGlobal - time_vampire_started : 0u,
            m_action == eActionCompleted ? "completed" : "interrupted");
    object->nlc_backhit_end(m_nlc_fired); // NLC: a broken back-hit grab lets the rest of the hit land

    if (object->com_man().ta_is_active())
        object->com_man().ta_deactivate();

    if (object->CControlledActor::is_controlling())
        object->CControlledActor::release();

    if (m_health_loss_activated)
    {
        const CEntityAlive* enemy = object->EnemyMan.get_enemy();
        if (enemy)
            enemy->conditions().GetChangeValues().m_fV_HealthRestore += object->m_vampire_loss_health_speed;
        m_health_loss_activated = false;
    }

    show_hud();
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::finalize()
{
    inherited::finalize();
    cleanup();
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::critical_finalize()
{
    inherited::critical_finalize();
    cleanup();
}

TEMPLATE_SPECIALIZATION
bool CStateBloodsuckerVampireExecuteAbstract::check_start_conditions()
{
    const CEntityAlive* enemy = object->EnemyMan.get_enemy();

    // NLC: vampire_ambush replaces the want / hit-count / facing gates with the ambush rules (cloaked, unseen, behind the
    // player, a chance per approach, one grab at a time); the stock actor, melee and line checks stay
    if (object->m_nlc_vampire_ambush)
    {
        if (!smart_cast<CActor const*>(enemy) || object->CControlledActor::is_controlling())
            return false;
        if (CAI_Bloodsucker::m_time_last_vampire && Device.dwTimeGlobal < CAI_Bloodsucker::m_time_last_vampire + object->m_vampire_min_delay)
            return false;
        // a hit that landed from behind (vampire_backhit_chance) skips the cloak, unseen and roll rules
        const bool backhit = object->nlc_backhit_ready();
        if (!backhit && !object->state_invisible)
            return false;
        if (smart_cast<CActor const*>(enemy)->input_external_handler_installed())
            return false;
        u32 const vertex_id = ai().level_graph().check_position_in_direction(object->ai_location().level_vertex_id(), object->Position(), enemy->Position());
        if (!ai().level_graph().valid_vertex_id(vertex_id))
            return false;
        if (!object->MeleeChecker.can_start_melee(enemy))
            return false;
        return backhit || object->nlc_vampire_ambush_ok(enemy);
    }

    // проверить дистанцию
    // 	float dist		= object->MeleeChecker.distance_to_enemy	(enemy);
    // 	if ((dist > VAMPIRE_MAX_DIST) || (dist < VAMPIRE_MIN_DIST))	return false;

    if (!object->done_enough_hits_before_vampire())
        return false;

    u32 const vertex_id = ai().level_graph().check_position_in_direction(object->ai_location().level_vertex_id(), object->Position(), enemy->Position());
    if (!ai().level_graph().valid_vertex_id(vertex_id))
        return false;

    if (!object->MeleeChecker.can_start_melee(enemy))
        return false;

    // проверить направление на врага
    if (!object->control().direction().is_face_target(enemy, PI_DIV_2))
        return false;

    if (!object->WantVampire())
        return false;

    // является ли враг актером
    if (!smart_cast<CActor const*>(enemy))
        return false;

    if (object->CControlledActor::is_controlling())
        return false;

    const CActor* actor = smart_cast<const CActor*>(enemy);

    VERIFY(actor);

    if (actor->input_external_handler_installed())
        return false;

    return true;
}

TEMPLATE_SPECIALIZATION
bool CStateBloodsuckerVampireExecuteAbstract::check_completion() { return (m_action == eActionCompleted); }

//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute_vampire_prepare()
{
    object->com_man().ta_activate(object->anim_triple_vampire);
    time_vampire_started = Device.dwTimeGlobal;

    object->sound().play(CAI_Bloodsucker::eVampireGrasp);
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute_vampire_continue()
{
    const CEntityAlive* enemy = object->EnemyMan.get_enemy();

    // if (object->Position().distance_to(Actor()->Position()) > 2.f) {
    // NLC: the ambush grab breaks only beyond vampire_break_dist (else MaxAttackDist): with the stock start distance
    // (MinAttackDist) a back-hit grab broke 220 ms after its start, the hit's knock-back carried the actor past it
    bool broken;
    if (object->m_nlc_vampire_ambush)
    {
        const float d = object->MeleeChecker.distance_to_enemy(enemy);
        const float limit = object->m_nlc_b.vampire_break_dist > 0.f ? object->m_nlc_b.vampire_break_dist : object->MeleeChecker.get_max_distance();
        broken = d > limit;
        if (broken && CBaseMonster::nlc_evade_debug())
            Msg("~ [evade] [%s]: vampire: grab broken after %u ms at %.1f m (limit %.1f)", object->cName().c_str(), Device.dwTimeGlobal - time_vampire_started, d, limit);
    }
    else
        broken = !object->MeleeChecker.can_start_melee(enemy);
    if (broken)
    {
        object->com_man().ta_deactivate();
        m_action = eActionCompleted;
        return;
    }

    object->sound().play(CAI_Bloodsucker::eVampireSucking);

    if (!m_health_loss_activated && !fis_zero(object->m_vampire_loss_health_speed))
    {
        enemy->conditions().GetChangeValues().m_fV_HealthRestore -= object->m_vampire_loss_health_speed;
        m_health_loss_activated = true;
    }

    // NLC: struggle: enough mouse travel ends the hold early (the grab still lands, with a smaller wound)
    if (object->m_nlc_vampire_ambush && !m_nlc_struggled && object->m_nlc_b.vampire_struggle_need > 0.f && object->nlc_struggle_sum() >= object->m_nlc_b.vampire_struggle_need)
    {
        m_nlc_struggled = true;
        m_action = eActionFire;
        if (CBaseMonster::nlc_evade_debug())
            Msg("~ [evade] [%s]: vampire: struggle: shaken off after %u ms, mouse travel %.0f", object->cName().c_str(), Device.dwTimeGlobal - time_vampire_started,
                object->nlc_struggle_sum());
        return;
    }

    if (time_vampire_started + object->m_vampire_hold_time < Device.dwTimeGlobal)
    {
        m_action = eActionFire;
    }
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute_vampire_hit()
{
    m_nlc_fired = true; // NLC
    object->com_man().ta_pointbreak();
    object->sound().play(CAI_Bloodsucker::eVampireHit);
    object->SatisfyVampire();

    const CEntityAlive* enemy = object->EnemyMan.get_enemy();

    if (m_health_loss_activated)
    {
        enemy->conditions().GetChangeValues().m_fV_HealthRestore += object->m_vampire_loss_health_speed;
        m_health_loss_activated = false;
    }

    if (smart_cast<CActor const*>(enemy) && !fis_zero(object->m_vampire_wound))
    {
        // NLC: a shaken-off grab wounds less (local copy, the member stays)
        const float wound = m_nlc_struggled ? object->m_vampire_wound * object->m_nlc_b.vampire_struggle_wound_k : object->m_vampire_wound;
        IKinematics* pK = smart_cast<IKinematics*>(const_cast<CEntityAlive*>(enemy)->Visual());
        enemy->conditions().AddWound(wound, ALife::eHitTypeWound, pK->LL_BoneID("bip01_head"));
    }
}

//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::look_head()
{
    IKinematics* pK = smart_cast<IKinematics*>(object->Visual());
    Fmatrix bone_transform;
    bone_transform = pK->LL_GetTransform(pK->LL_BoneID("bip01_head"));

    Fmatrix global_transform;
    global_transform.mul_43(object->XFORM(), bone_transform);

    object->CControlledActor::look_point(global_transform.c);
}

#undef TEMPLATE_SPECIALIZATION
#undef CStateBloodsuckerVampireExecuteAbstract
