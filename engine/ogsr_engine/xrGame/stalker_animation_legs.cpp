////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_animation_legs.cpp
//	Created 	: 25.02.2003
//  Modified 	: 19.11.2004
//	Author		: Dmitriy Iassenev
//	Description : Stalker animation manager : legs animations
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stalker_animation_manager.h"
#include "ai/stalker/ai_stalker.h"
#include "sight_manager.h"
#include "stalker_movement_manager.h"
#include "stalker_animation_data.h"

#include "weapon.h"
#include "missile.h"
#include "inventory.h"
#include "stalker_animation_manager_impl.h"

const float right_forward_angle = PI_DIV_4;
const float left_forward_angle = PI_DIV_4;
const float standing_turn_angle = PI_DIV_6;
const float epsilon = EPS_L;

const u32 direction_switch_interval = 500;

const u32 need_look_back_time_delay = 0;

const float direction_angles[] = {
    0.f, //	eMovementDirectionForward
    PI, //	eMovementDirectionBackward
    PI_DIV_2, //	eMovementDirectionLeft
    -PI_DIV_2 //	eMovementDirectionRight
};

void CStalkerAnimationManager::legs_play_callback(CBlend* blend)
{
    CAI_Stalker* object = (CAI_Stalker*)blend->CallbackParam;
    VERIFY(object);

    CStalkerAnimationPair& pair = object->animation().legs();
    pair.on_animation_end();
}

IC float CStalkerAnimationManager::legs_switch_factor() const
{
    if ((m_target_direction == eMovementDirectionForward) && (m_current_direction == eMovementDirectionBackward))
        return (0.f);

    if ((m_target_direction == eMovementDirectionBackward) && (m_current_direction == eMovementDirectionForward))
        return (0.f);

    if ((m_target_direction == eMovementDirectionLeft) && (m_current_direction == eMovementDirectionRight))
        return (0.f);

    if ((m_target_direction == eMovementDirectionRight) && (m_current_direction == eMovementDirectionLeft))
        return (0.f);

    return (1.f);
}

bool CStalkerAnimationManager::need_look_back() const
{
    if (m_looking_back)
        return (true);

    if (m_previous_speed_direction != eMovementDirectionBackward)
        return (false);

    if ((m_change_direction_time + need_look_back_time_delay) > Device.dwTimeGlobal)
        return (false);

    m_looking_back = ::Random.randI(2) + 1;
    return (true);
}

void CStalkerAnimationManager::legs_assign_direction(float switch_factor, const EMovementDirection& direction)
{
    if (m_current_direction == direction)
    {
        m_direction_start = Device.dwTimeGlobal;
        return;
    }

    if (m_target_direction != direction)
    {
        m_direction_start = Device.dwTimeGlobal;
        m_target_direction = direction;
        return;
    }

    VERIFY(m_direction_start <= Device.dwTimeGlobal);
    if ((Device.dwTimeGlobal - m_direction_start) <= (u32)iFloor(switch_factor * direction_switch_interval))
        return;

    m_direction_start = Device.dwTimeGlobal;
    m_current_direction = direction;
}

void CStalkerAnimationManager::legs_process_direction(float yaw)
{
    float switch_factor = legs_switch_factor();
    CStalkerMovementManager& movement = object().movement();
    float head_current = movement.head_orientation().current.yaw;
    float left = left_angle(yaw, head_current);
    float test_angle_forward = right_forward_angle;
    float test_angle_backward = left_forward_angle;
    if (left)
    {
        test_angle_forward = left_forward_angle;
        test_angle_backward = right_forward_angle;
    }
    test_angle_backward = PI - test_angle_backward;

    float difference = angle_difference(yaw, head_current);

    if (difference <= test_angle_forward)
        legs_assign_direction(switch_factor, eMovementDirectionForward);
    else
    {
        if (difference > test_angle_backward)
            legs_assign_direction(switch_factor, eMovementDirectionBackward);
        else if (left)
            legs_assign_direction(switch_factor, eMovementDirectionLeft);
        else
            legs_assign_direction(switch_factor, eMovementDirectionRight);
    }

    movement.m_body.target.yaw = yaw + direction_angles[m_current_direction];
}

// NLC: wounded gait. Below [nlc_limp] health (default 0.4, 0 = off) a standing stalker walks and runs with the damaged
// clip set (state 2, "dmg_norm_"); the torso keeps its normal set, so every weapon pose still works. Hysteresis 0.05.
static u32 g_nlc_limp_mode = u32(-1); // -1: not read from [nlc_limp] yet

u32 nlc_limp_mode()
{
    if (g_nlc_limp_mode == u32(-1))
    {
        g_nlc_limp_mode = (READ_IF_EXISTS(pSettings, r_bool, "nlc_limp", "legs", true) ? 1u : 0u) | (READ_IF_EXISTS(pSettings, r_bool, "nlc_limp", "torso_calm", true) ? 2u : 0u) |
            (READ_IF_EXISTS(pSettings, r_bool, "nlc_limp", "torso_alert", true) ? 4u : 0u) | (READ_IF_EXISTS(pSettings, r_bool, "nlc_limp", "torso_panic", true) ? 8u : 0u);
    }
    return g_nlc_limp_mode;
}

void nlc_limp_mode_set(u32 mode)
{
    g_nlc_limp_mode = mode & 15u;
    Msg("~ [nlc_limp] mode = %u (1 legs, 2 torso calm, 4 torso alert, 8 torso panic run)", g_nlc_limp_mode);
}

static float g_nlc_limp_enter = -1.f; // -1: not read from [nlc_limp] yet
static float g_nlc_limp_exit = -1.f;

static void nlc_limp_health_read()
{
    if (g_nlc_limp_enter >= 0.f)
        return;
    g_nlc_limp_enter = READ_IF_EXISTS(pSettings, r_float, "nlc_limp", "health", 0.4f);
    g_nlc_limp_exit = READ_IF_EXISTS(pSettings, r_float, "nlc_limp", "exit", 0.55f);
    clamp(g_nlc_limp_enter, 0.f, 1.f);
    clamp(g_nlc_limp_exit, g_nlc_limp_enter + 0.01f, 1.f);
}

// the limp starts below `enter` and ends at `exit` or above (Lua level.nlc_limp_health)
void nlc_limp_health_set(float enter, float exit)
{
    clamp(enter, 0.f, 1.f);
    clamp(exit, enter + 0.01f, 1.f);
    g_nlc_limp_enter = enter;
    g_nlc_limp_exit = exit;
    Msg("~ [nlc_limp] health enter = %.2f, exit = %.2f", enter, exit);
}

bool CStalkerAnimationManager::nlc_limp() const
{
    nlc_limp_health_read();
    if (g_nlc_limp_enter <= 0.f)
        return false;
    const float h = object().GetfHealth();
    if (!m_nlc_limp && h < g_nlc_limp_enter)
        m_nlc_limp = true;
    else if (m_nlc_limp && h >= g_nlc_limp_exit - 0.001f)
        m_nlc_limp = false;
    return m_nlc_limp && !object().critically_wounded();
}

static float g_nlc_limp_run = -1.f; // -1: not read from [nlc_limp] yet
static float g_nlc_limp_walk = -1.f;

static void nlc_limp_speed_read()
{
    if (g_nlc_limp_run >= 0.f)
        return;
    g_nlc_limp_run = READ_IF_EXISTS(pSettings, r_float, "nlc_limp", "speed_run", 0.7f);
    g_nlc_limp_walk = READ_IF_EXISTS(pSettings, r_float, "nlc_limp", "speed_walk", 0.85f);
    clamp(g_nlc_limp_run, 0.2f, 1.5f);
    clamp(g_nlc_limp_walk, 0.2f, 1.5f);
}

void nlc_limp_speed_set(float run, float walk)
{
    clamp(run, 0.2f, 1.5f);
    clamp(walk, 0.2f, 1.5f);
    g_nlc_limp_run = run;
    g_nlc_limp_walk = walk;
    Msg("~ [nlc_limp] speed run = %.2f, walk = %.2f", run, walk);
}

float CStalkerAnimationManager::nlc_limp_speed_k() const
{
    if (legs_state() != 2)
        return 1.f;
    nlc_limp_speed_read();
    return object().movement().movement_type() == eMovementTypeRun ? g_nlc_limp_run : g_nlc_limp_walk;
}

u32 CStalkerAnimationManager::legs_state() const
{
    const u32 st = body_state();
    if (st != eBodyStateStand)
        return st;
    if (!nlc_limp() || !(nlc_limp_mode() & 1u))
        return st;
    return 2;
}

// the damaged torso clip of a weapon slot (state 2, "dmg_norm_torso_<slot>_<set>_<index>"), an invalid id when the visual has none
MotionID CStalkerAnimationManager::nlc_dmg_clip(u32 slot, u32 set, u32 index) const
{
    const auto& parts = m_data_storage->m_part_animations.A;
    if (parts.size() <= 2)
        return MotionID();
    const auto& dmg = parts[2].m_torso.A;
    if (slot >= dmg.size() || set >= dmg[slot].A.size() || index >= dmg[slot].A[set].A.size())
        return MotionID();
    return dmg[slot].A[set].A[index];
}

// torso pose with the weapon not up (free mental state): idle_1, walk_1, run_1 of the slot
MotionID CStalkerAnimationManager::nlc_torso_calm(u32 slot, const EBodyState& body_state, u32 set, u32 index) const
{
    if (body_state == eBodyStateStand && (nlc_limp_mode() & 2u) && nlc_limp())
    {
        const MotionID m = nlc_dmg_clip(slot, set, index);
        if (m.valid())
            return m;
    }
    return m_data_storage->m_part_animations.A[body_state].m_torso.A[slot].A[set].A[index];
}

// torso pose with the weapon up but not firing (danger): aim_0 / aim_2 / aim_3 of the slot; the panic run falls back to run_0 when there is no aim_3
MotionID CStalkerAnimationManager::nlc_torso_alert(u32 slot, const EBodyState& body_state, u32 set, u32 index) const
{
    if (body_state == eBodyStateStand && nlc_limp())
    {
        const u32 mode = nlc_limp_mode();
        if (mode & 4u)
        {
            const MotionID m = nlc_dmg_clip(slot, set, index);
            if (m.valid())
                return m;
        }
        if ((mode & 8u) && index == 3 && object().movement().mental_state() == MonsterSpace::eMentalStatePanic)
        {
            const MotionID m = nlc_dmg_clip(slot, 8, 0);
            if (m.valid())
                return m;
        }
    }
    return m_data_storage->m_part_animations.A[body_state].m_torso.A[slot].A[set].A[index];
}

MotionID CStalkerAnimationManager::nlc_leg_motion(u32 movement_type, u32 direction, u32 variant)
{
    const u32 st = legs_state();
    MotionID m = m_data_storage->m_part_animations.A[st].m_movement.A[movement_type].A[direction].A[variant];
    if (!m.valid() && st != u32(body_state())) // the visual has no damaged set: the normal one
        m = m_data_storage->m_part_animations.A[body_state()].m_movement.A[movement_type].A[direction].A[variant];
    return m;
}

MotionID CStalkerAnimationManager::legs_move_animation()
{
    m_no_move_actual = false;

    CStalkerMovementManager& movement = object().movement();

    VERIFY((movement.body_state() == eBodyStateStand) || (movement.mental_state() != eMentalStateFree));

    if (eMentalStateDanger != movement.mental_state())
    {
        m_current_speed = movement.speed(eMovementDirectionForward);

        return (nlc_leg_motion(movement.movement_type(), eMovementDirectionForward, 1));
    }

    float yaw, pitch;
    object().sight().GetDirectionAngles(yaw, pitch);

    yaw = angle_normalize_signed(-yaw);
    ;
    legs_process_direction(yaw);

    float body_current = movement.body_orientation().current.yaw;
    bool left = left_angle(yaw, body_current);
    float test_angle_forward = right_forward_angle;
    float test_angle_backward = left_forward_angle;
    if (left)
    {
        test_angle_forward = left_forward_angle;
        test_angle_backward = right_forward_angle;
    }
    test_angle_backward = PI - test_angle_backward;

    EMovementDirection speed_direction;
    float difference = angle_difference(yaw, body_current);

    if (difference <= test_angle_forward)
        speed_direction = eMovementDirectionForward;
    else
    {
        if (difference > test_angle_backward)
            speed_direction = eMovementDirectionBackward;
        else
        {
            if (left)
                speed_direction = eMovementDirectionLeft;
            else
                speed_direction = eMovementDirectionRight;
        }
    }

    if (m_previous_speed_direction != speed_direction)
    {
        if (m_change_direction_time < Device.dwTimeGlobal)
            m_change_direction_time = Device.dwTimeGlobal;

        if (!legs_switch_factor())
        {
            m_previous_speed = 0.f;
            m_current_speed = 0.f;
        }

        m_previous_speed_direction = speed_direction;
    }

    m_current_speed = movement.speed(speed_direction);

    return (nlc_leg_motion(movement.movement_type(), speed_direction, 0));
}

MotionID CStalkerAnimationManager::legs_no_move_animation()
{
    m_previous_speed = 0.f;
    m_current_speed = 0.f;

    if (!m_no_move_actual)
    {
        m_no_move_actual = true;
        if (m_crouch_state_config == -1)
            m_crouch_state = ::Random.randI(2);
        else
            m_crouch_state = m_crouch_state_config;
    }

    m_change_direction_time = Device.dwTimeGlobal;
    m_current_speed = 0.f;

    EBodyState body_state = this->body_state();
    const xr_vector<MotionID>& animation = m_data_storage->m_part_animations.A[body_state].m_in_place->A;

    CStalkerMovementManager& movement = object().movement();
    const SBoneRotation& body_orientation = movement.body_orientation();
    float current = body_orientation.current.yaw;
    float target = body_orientation.target.yaw;
    if (angle_difference(target, current) < EPS_L)
    {
        float head_current = movement.head_orientation().current.yaw;
        if ((movement.mental_state() != eMentalStateFree) || (!object().sight().turning_in_place() && (angle_difference(current, head_current) <= standing_turn_angle)))
        {
            if (movement.mental_state() == eMentalStateFree)
                return (animation[1]);

            if (body_state == eBodyStateCrouch)
                return (animation[m_crouch_state]);

            return (animation[0]);
        }

        movement.m_body.target.yaw = movement.head_orientation().target.yaw;
        target = movement.m_body.target.yaw;
    }

    if (left_angle(current, target))
    {
        if (movement.mental_state() == eMentalStateFree)
            return (animation[4]);

        return (animation[2]);
    }

    if (movement.mental_state() == eMentalStateFree)
        return (animation[5]);

    return (animation[3]);
}

MotionID CStalkerAnimationManager::assign_legs_animation()
{
    if (standing())
        return (legs_no_move_animation());

    return (legs_move_animation());
}
