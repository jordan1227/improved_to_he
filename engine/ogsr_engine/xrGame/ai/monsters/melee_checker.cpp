#include "stdafx.h"
#include "melee_checker.h"
#include "basemonster/base_monster.h"
#include "../../../Include\xrRender\Kinematics.h"
#include "../../../xr_3da/xr_collide_form.h"

#define MAX_TRACE_ENEMY_RANGE 6.f

bool CMeleeChecker::s_debug_log = false;
bool CMeleeChecker::s_trace_ignore_objects = false;
float CMeleeChecker::s_close_yaw_dist = 0.f;
float CMeleeChecker::s_close_yaw_half = 0.f;

// NLC: distance from the attacker's centre to the enemy's collision surface (bone shapes) along the
// centre-to-centre line, plus m_surface_offset, never above the centre distance. Human-sized targets keep
// roughly the centre distance (offset ~ their half-width); large targets (pseudogiant) get much closer values.
float CMeleeChecker::surface_distance(const CEntity* enemy, float centre)
{
    if (m_cache_frame == Device.dwFrame && m_cache_enemy == enemy->ID())
        return m_cache_value;

    float result = centre;
    if (enemy->CFORM())
    {
        Fvector from, to, dir;
        m_object->Center(from);
        enemy->Center(to);
        dir.sub(to, from);
        const float range = dir.magnitude();
        if (range > EPS_L)
        {
            dir.div(range);
            collide::rq_results results;
            collide::ray_defs query(from, dir, range, CDB::OPT_ONLYNEAREST, collide::rqtObject);
            if (enemy->CFORM()->RayQuery(results, query) && results.r_count())
                result = std::min(centre, results.r_begin()->range + m_surface_offset);
        }
    }

    m_cache_frame = Device.dwFrame;
    m_cache_enemy = enemy->ID();
    m_cache_value = result;
    return result;
}

float CMeleeChecker::distance_to_enemy(const CEntity* enemy)
{
    float dist = enemy->Position().distance_to(m_object->Position());

    if (m_surface_mode) // NLC
        return (dist > MAX_TRACE_ENEMY_RANGE) ? dist : surface_distance(enemy, dist);

    if (dist > MAX_TRACE_ENEMY_RANGE)
        return dist;

    Fvector enemy_center;
    enemy->Center(enemy_center);

    Fvector my_head_pos = get_head_position(m_object);

    Fvector dir;
    dir.sub(enemy_center, my_head_pos);
    dir.normalize_safe();

    collide::ray_defs r_query(my_head_pos, dir, MAX_TRACE_ENEMY_RANGE, CDB::OPT_CULL | CDB::OPT_ONLYNEAREST, collide::rqtObject);
    r_res.r_clear();

    if (m_object->CFORM() && m_object->CFORM()->RayQuery(r_res, r_query))
    {
        if (r_res.r_begin()->O == enemy)
            dist = r_res.r_begin()->range;
    }

    return (dist);
}

void CMeleeChecker::on_hit_attempt(bool hit_success)
{
    // добавить новый элемент в стек
    for (u32 i = HIT_STACK_SIZE - 1; i > 0; i--)
        m_hit_stack[i] = m_hit_stack[i - 1];
    m_hit_stack[0] = hit_success;

    // проверить однородность стека
    bool stack_similar = true;
    for (u32 i = 1; i < HIT_STACK_SIZE; i++)
        if (m_hit_stack[i] != hit_success)
        {
            stack_similar = false;
            break;
        }

    if (!stack_similar)
        return;

    // обновить m_current_min_distance
    if (hit_success)
    {
        if (m_current_min_distance + m_as_step < m_min_attack_distance)
            m_current_min_distance += m_as_step;
        else
            m_current_min_distance = m_min_attack_distance;
    }
    else
    {
        if (m_current_min_distance > m_as_min_dist + m_as_step)
            m_current_min_distance -= m_as_step;
        else
            m_current_min_distance = m_as_min_dist;
    }
}
