#pragma once

#include "../../../level.h"
#include "../telekinesis_hit_callback.h"

#define GOOD_DISTANCE_FOR_TELE 15.f
#define MAX_TIME_CHECK_FAILURE 6000

template <typename Object>
CStateBurerAttackTele<Object>::CStateBurerAttackTele(Object* obj) : inherited(obj)
{
    m_anim_end_tick = 0;
}

template <typename Object>
void CStateBurerAttackTele<Object>::initialize()
{
    inherited::initialize();

    m_action = ACTION_TELE_STARTED;
    selected_object = 0;

    SelectObjects();

    time_started = 0;
    m_anim_end_tick = 0;
    m_initial_health = object->conditions().GetHealth();
    m_end_tick = current_time() + object->m_tele_max_time;
    m_nlc_end_logged = false;

    object->set_script_capture(false);

    object->nlc_log("tele start objects=%u candidates_left=%u", object->CTelekinesis::get_objects_count(), u32(tele_objects.size()));
}

template <typename Object>
void CStateBurerAttackTele<Object>::execute()
{
    // 	if ( object->EnemyMan.see_enemy_now() )
    // 	{
    // 		m_last_saw_enemy_tick					=	current_time();
    // 	}

    switch (m_action)
    {
    case ACTION_TELE_STARTED:
        object->anim().set_override_animation(eAnimTelekinesis, 0);
        if (!time_started)
        {
            float const time = object->anim().get_animation_length(eAnimTelekinesis, 0);
            m_anim_end_tick = current_time() + TTime(time * 1000);
            time_started = Device.dwTimeGlobal;
        }
        else
        {
            if (current_time() > m_anim_end_tick)
            {
                m_action = ACTION_TELE_CONTINUE;
            }
        }
        break;

    case ACTION_TELE_CONTINUE:
        object->anim().set_override_animation(eAnimTelekinesis, 1);
        ExecuteTeleContinue();
        break;

    case ACTION_TELE_FIRE: {
        object->anim().set_override_animation(eAnimTeleFire, 0);
        ExecuteTeleFire();
        float const time = object->anim().get_animation_length(eAnimTeleFire, 0);
        m_anim_end_tick = current_time() + TTime(time * 1000);
        m_action = ACTION_WAIT_FIRE_END;
        break;
    }

    case ACTION_WAIT_FIRE_END:
        object->anim().set_override_animation(eAnimTeleFire, 0);
        if (current_time() > m_anim_end_tick)
        {
            if (IsActiveObjects())
            {
                m_action = ACTION_TELE_CONTINUE;
            }
            else
            {
                m_action = ACTION_COMPLETED;
            }
        }

    case ACTION_COMPLETED: break;
    }

    object->face_enemy();
}

template <typename Object>
void CStateBurerAttackTele<Object>::deactivate()
{
    tele_objects.clear();

    // grenades live in CBurer's own container (CBurer::nlc_grenade_update), not here
    for (u32 i = 0; i < object->CTelekinesis::get_objects_total_count(); ++i)
    {
        CPhysicsShellHolder* const cur_object = object->CTelekinesis::get_object_by_index(i).object;
        if (!cur_object || !cur_object->m_pPhysicsShell || !cur_object->m_pPhysicsShell->isActive())
        {
            continue;
        }

        object->StopTeleObjectParticle(cur_object);
    }

    // NLC: the end throws what is held at the enemy only if it is in sight; otherwise the objects are let go
    if (object->EnemyMan.get_enemy() && object->EnemyMan.see_enemy_now())
        FireAllToEnemy();
    else if (object->CTelekinesis::get_objects_count())
        object->nlc_log("tele end: %u objects let go (enemy out of sight)", object->CTelekinesis::get_objects_count());
    object->CTelekinesis::deactivate();
    object->set_script_capture(true);
}

template <typename Object>
void CStateBurerAttackTele<Object>::finalize()
{
    deactivate();
    object->m_nlc_tele_next = Device.dwTimeGlobal + u32(object->m_nlc_tele_cooldown * object->nlc_phase_k(1.f, 0.8f, 0.6f));
    inherited::finalize();
}

template <typename Object>
void CStateBurerAttackTele<Object>::critical_finalize()
{
    // NLC: CState::select_state ends the previous state with critical_finalize only; the cooldown has to be set here
    // too, or a tele cut short by damage restarted every few frames, throwing everything point-blank
    if (object->m_nlc_tele_next < Device.dwTimeGlobal + 100)
        object->m_nlc_tele_next = Device.dwTimeGlobal + u32(object->m_nlc_tele_cooldown * object->nlc_phase_k(1.f, 0.8f, 0.6f));
    deactivate();
    inherited::critical_finalize();
}

template <typename Object>
bool CStateBurerAttackTele<Object>::check_start_conditions()
{
    return CheckTeleStart();
}

template <typename Object>
bool CStateBurerAttackTele<Object>::check_completion()
{
    float dist = object->EnemyMan.get_enemy()->Position().distance_to(object->Position());

    LPCSTR reason = nullptr;
    if (dist < object->m_tele_min_distance)
        reason = "too_close";
    else if (dist > object->m_tele_max_distance)
        reason = "too_far";
    else if (object->conditions().GetHealth() < m_initial_health)
        reason = "damaged";
    else if (current_time() > m_end_tick)
        reason = "max_time";
    else if (m_action == ACTION_COMPLETED)
        reason = "completed";

    if (!reason)
        return false;

    if (!m_nlc_end_logged)
    {
        m_nlc_end_logged = true;
        object->m_nlc_tele_next = Device.dwTimeGlobal + u32(object->m_nlc_tele_cooldown * object->nlc_phase_k(1.f, 0.8f, 0.6f));
        object->nlc_log("tele end reason=%s dist=%.1f held=%u", reason, dist, object->CTelekinesis::get_objects_count());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

template <typename Object>
void CStateBurerAttackTele<Object>::FindFreeObjects(xr_vector<CObject*>& tpObjects, const Fvector& pos)
{
    Level().ObjectSpace.GetNearest(tpObjects, pos, object->m_tele_find_radius, NULL);

    for (u32 i = 0; i < tpObjects.size(); i++)
    {
        CPhysicsShellHolder* obj = smart_cast<CPhysicsShellHolder*>(tpObjects[i]);
        CCustomMonster* custom_monster = smart_cast<CCustomMonster*>(tpObjects[i]);
        CGrenade* grenade = smart_cast<CGrenade*>(tpObjects[i]);

        if (grenade || // grenades are handled by CBurer::nlc_grenade_update
            !obj || !obj->PPhysicsShell() || !obj->PPhysicsShell()->isActive() || custom_monster || (obj->spawn_ini() && obj->spawn_ini()->section_exist("ph_heavy")) ||
            (obj->m_pPhysicsShell->getMass() < object->m_tele_object_min_mass) || (obj->m_pPhysicsShell->getMass() > object->m_tele_object_max_mass) || (obj == object) ||
            CBurer::nlc_held_by_any(obj) || object->nlc_puppet_reserved(obj) || object->nlc_tele_skip(obj) ||
            (smart_cast<CInventoryItem*>(obj) && smart_cast<CInventoryItem*>(obj)->m_nlc_tele_held) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "ph_heavy") && pSettings->r_bool(obj->cNameSect().c_str(), "ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "quest_item") && pSettings->r_bool(obj->cNameSect().c_str(), "quest_item")) || obj->hasFixedBones() ||
            !obj->m_pPhysicsShell->get_ApplyByGravity())
            continue;

        tele_objects.push_back(obj);
    }
}

template <typename Object>
void CStateBurerAttackTele<Object>::FindObjects()
{
    const size_t res_size = tele_objects.size();
    tele_objects.clear();

    m_nearest.clear();
    m_nearest.reserve(res_size);
    FindFreeObjects(m_nearest, object->EnemyMan.get_enemy()->Position());

    FindFreeObjects(m_nearest, object->Position());

    float dist = object->EnemyMan.get_enemy()->Position().distance_to(object->Position());
    Fvector dir;
    dir.sub(object->EnemyMan.get_enemy()->Position(), object->Position());
    dir.normalize();

    Fvector pos;
    pos.mad(object->Position(), dir, dist / 2.f);
    FindFreeObjects(m_nearest, pos);

    tele_objects.erase(std::unique(tele_objects.begin(), tele_objects.end()), tele_objects.end());
}

template <typename Object>
void CStateBurerAttackTele<Object>::FireAllToEnemy()
{
    if (!object->CTelekinesis::is_active())
    {
        return;
    }

    if (!object->EnemyMan.get_enemy())
    {
        return;
    }

    for (u32 i = 0; i < object->CTelekinesis::get_objects_count(); ++i)
    {
        u32 const prev_num_objects = object->CTelekinesis::get_objects_count();

        CPhysicsShellHolder* const cur_object = object->CTelekinesis::get_object_by_index(i).object;
        if (!cur_object)
        {
            continue;
        }

        FireObject(cur_object);

        u32 const new_num_objects = object->CTelekinesis::get_objects_count();
        if (new_num_objects < prev_num_objects)
        {
            VERIFY(new_num_objects == prev_num_objects - 1);
            --i;
        }
    }

    // object->CTelekinesis::fire_all(enemy_pos);

    object->sound().play(CBurer::eMonsterSoundTeleAttack);
}

template <typename Object>
void CStateBurerAttackTele<Object>::ExecuteTeleContinue()
{
    if (time_started + object->m_tele_time_to_hold > Device.dwTimeGlobal)
        return;

    if (!object->EnemyMan.see_enemy_now())
        return;

    bool object_found = false;
    CTelekineticObject tele_object;

    u32 i = 0;
    while (i < object->CTelekinesis::get_objects_count())
    {
        tele_object = object->CTelekinesis::get_object_by_index(i);

        if ((tele_object.get_state() == TS_Keep) && (tele_object.time_keep_started + 1500 < Device.dwTimeGlobal) &&
            IsPathClear(tele_object.get_object()))
        {
            object_found = true;
            break;
        }
        else
            i++;
    }

    if (object_found)
    {
        m_action = ACTION_TELE_FIRE;
        selected_object = tele_object.get_object();
    }
    else
    {
        if (!IsActiveObjects() || (time_started + MAX_TIME_CHECK_FAILURE < Device.dwTimeGlobal))
        {
            m_action = ACTION_COMPLETED;
        }
    }
}

#define HEAD_OFFSET_INDOOR 1.f
#define HEAD_OFFSET_OUTDOOR 5.f

template <typename Object>
void CStateBurerAttackTele<Object>::ExecuteTeleFire()
{
    FireObject(selected_object);

    object->StopTeleObjectParticle(selected_object);
    object->sound().play(CBurer::eMonsterSoundTeleAttack);
}

template <typename Object>
void CStateBurerAttackTele<Object>::FireObject(CPhysicsShellHolder* obj)
{
    const CEntityAlive* enemy = object->EnemyMan.get_enemy();
    if (!obj || !enemy)
        return;

    // NLC: a gun lifted before the enemy changed: dropped, not thrown at a stalker
    if (object->nlc_tele_skip(obj))
    {
        object->StopTeleObjectParticle(obj);
        object->CTelekinesis::deactivate(obj);
        object->nlc_log("tele drops %s (no guns thrown at stalkers)", obj->cNameSect().c_str());
        return;
    }

    Fvector aim = get_head_position(const_cast<CEntityAlive*>(enemy));
    // guns are heavy and slow at the prop speed: thrown faster and flatter
    const float speed = object->nlc_throw_speed(obj);
    float fire_time = obj->Position().distance_to(aim) / speed;
    aim = object->nlc_lead_point(enemy, aim, fire_time, object->m_nlc_tele_lead);
    fire_time = obj->Position().distance_to(aim) / speed;

    // a missed throw stops being dangerous when the telekinetic fire phase ends (FIRE_TIME 3 s)
    const float damage = object->nlc_tele_damage(obj);
    const bool armed = nlc_tele_set_hit_callback(obj, damage, Device.dwTimeGlobal + 3000);
    object->CTelekinesis::fire_t(obj, aim, fire_time);

    const Fvector head = get_head_position(const_cast<CEntityAlive*>(enemy));
    object->nlc_log("tele throw %s mass=%.0f damage=%.2f armed=%d dist=%.1f flight=%.2f lead=%.1f", obj->cNameSect().c_str(),
        obj->m_pPhysicsShell ? obj->m_pPhysicsShell->getMass() : 0.f, damage, armed ? 1 : 0, obj->Position().distance_to(aim), fire_time, head.distance_to(aim));
}

template <typename Object>
bool CStateBurerAttackTele<Object>::IsPathClear(CPhysicsShellHolder* obj)
{
    const CEntityAlive* enemy = object->EnemyMan.get_enemy();
    if (!obj || !enemy)
        return false;

    Fvector from;
    obj->Center(from);
    return object->nlc_clear_path(from, get_head_position(const_cast<CEntityAlive*>(enemy)), obj, enemy);
}

template <typename Object>
bool CStateBurerAttackTele<Object>::IsActiveObjects()
{
    return (object->CTelekinesis::get_objects_count() > 0);
}

template <typename Object>
bool CStateBurerAttackTele<Object>::CheckTeleStart()
{
    if (IsActiveObjects())
        return false;

    if (Device.dwTimeGlobal < object->m_nlc_tele_next)
        return false;

    // one concentration: the puppet gun is the ranged attack while it works
    if (object->nlc_puppet_active())
        return false;

    CEntityAlive const* enemy = object->EnemyMan.get_enemy();
    if (!enemy || (enemy != Actor() && !object->m_nlc_tele_vs_npc))
        return false;

    float dist = object->Position().distance_to(enemy->Position());
    if (dist < object->m_tele_min_distance)
        return false;
    if (dist > object->m_tele_max_distance)
        return false;

    if (Device.dwTimeGlobal < m_nlc_find_next)
        return false;

    FindObjects();

    if (tele_objects.empty())
    {
        m_nlc_find_next = Device.dwTimeGlobal + 300;
        return false;
    }

    return true;
}

class best_object_predicate
{
    Fvector enemy_pos;
    Fvector monster_pos;

public:
    best_object_predicate(const Fvector& m_pos, const Fvector& pos)
    {
        monster_pos = m_pos;
        enemy_pos = pos;
    }

    bool operator()(const CGameObject* tpObject1, const CGameObject* tpObject2) const
    {
        float dist1 = monster_pos.distance_to(tpObject1->Position());
        float dist2 = enemy_pos.distance_to(tpObject2->Position());
        float dist3 = enemy_pos.distance_to(monster_pos);

        return ((dist1 < dist3) && (dist2 > dist3));
    };
};

class best_object_predicate2
{
    Fvector enemy_pos;
    Fvector monster_pos;

public:
    best_object_predicate2(const Fvector& m_pos, const Fvector& pos)
    {
        monster_pos = m_pos;
        enemy_pos = pos;
    }

    bool operator()(const CGameObject* tpObject1, const CGameObject* tpObject2) const
    {
        float dist1 = enemy_pos.distance_to(tpObject1->Position());
        float dist2 = enemy_pos.distance_to(tpObject2->Position());

        return dist1 < dist2;
    }
};

template <typename Object>
void CStateBurerAttackTele<Object>::SelectObjects()
{
    std::sort(tele_objects.begin(), tele_objects.end(), best_object_predicate2(object->Position(), object->EnemyMan.get_enemy()->Position()));

    for (u32 i = 0; i < tele_objects.size(); ++i)
    {
        CPhysicsShellHolder* obj = tele_objects[i];

        float height = object->m_tele_object_height;

        if (object->m_monster_type == CBaseMonster::eMonsterTypeIndoor)
        {
            height *= 0.7f;
        }

        bool const rotate = object->m_monster_type != CBaseMonster::eMonsterTypeIndoor;

        CTelekineticObject* tele_obj = object->CTelekinesis::activate(obj, object->m_tele_raise_speed, height, 10000, rotate);

        tele_obj->set_sound(object->sound_tele_hold, object->sound_tele_throw);

        object->StartTeleObjectParticle(obj);

        tele_objects[i] = tele_objects[tele_objects.size() - 1];
        tele_objects.pop_back();

        if (object->CTelekinesis::get_objects_count() + (object->m_nlc_bar_obj ? 1u : 0u) >= object->m_tele_max_handled_objects)
        {
            break;
        }
    }
}
