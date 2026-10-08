#include "stdafx.h"
#include "controller.h"
#include "controller_state_manager.h"
#include "../controlled_entity.h"
#include "../../../Actor.h"
#include "../../../ActorEffector.h"
#include "../../../ActorCondition.h"
#include "../ai_monster_effector.h"
#include "../../../../Include/xrRender/KinematicsAnimated.h"
#include "../../../level.h"
#include "../../../sound_player.h"
#include "../../../ai_monster_space.h"
#include "../../../UIGameCustom.h"
#include "../../../ui/UIStatic.h"

#include "../monster_velocity_space.h"
#include "../../../level_debug.h"
#include "../../../game_object_space.h"
#include "../../../detail_path_manager.h"
#include "../../../ai_space.h"
#include "cover_point.h"
#include "../../../cover_manager.h"

#include "controller_animation.h"
#include "controller_direction.h"

#include "../control_direction_base.h"
#include "../control_movement_base.h"
#include "../control_path_builder_base.h"

#include "level_graph.h"
#include "../../../ai_object_location.h"

#include "../../../Inventory.h"
#include "../../../monster_community.h"
#include "../../../character_community.h"
#include "../../../InventoryOwner.h"
#include "character_info.h"

#include "controller_psy_hit.h"
#include "../monster_cover_manager.h"
#include "controller_psy_aura.h"
#include "../../../hudmanager.h"
#include "../../../memory_manager.h" // NLC: recruit visibility check
#include "../../../visual_memory_manager.h"
#include "../monster_home.h" // NLC: thrall home suspend/resume
#include "../../../alife_simulator.h" // NLC: Mirage decoy spawn
#include "xrServer_Objects_ALife.h"
#include "../../../ParticlesObject.h"

/*
#ifdef DEBUG
#	include <dinput.h>
#endif
*/

const u32 _pmt_psy_attack_delay = 2000;
const float _pmt_psy_attack_min_angle = deg(5);

namespace detail
{
namespace controller
{
// default settings for tube fire:
const u32 default_tube_condition_see_duration = 50;
const u32 default_tube_condition_min_delay = 10000;
const float default_tube_condition_min_distance = 10;
const float default_stamina_hit = 0.2f;

} // namespace controller
} // namespace detail

CController::CController()
{
    StateMan = xr_new<CStateManagerController>(this);
    time_control_hit_started = 0;

    m_psy_hit = xr_new<CControllerPsyHit>();

    control().add(m_psy_hit, ControlCom::eComCustom1);

    /*
    #ifdef DEBUG
        P1.set(0.f,0.f,0.f);
        P2.set(0.f,0.f,0.f);
    #endif
    */
}

CController::~CController()
{
    xr_delete(StateMan);
    xr_delete(m_psy_hit);
}

void CController::Load(LPCSTR section)
{
    inherited::Load(section);

    m_max_controlled_number = pSettings->r_u8(section, "Max_Controlled_Count");
    m_controlled_objects.reserve(m_max_controlled_number);

    anim().accel_load(section);

    ::Sound->create(control_start_sound, pSettings->r_string(section, "sound_control_start"), st_Effect, SOUND_TYPE_WORLD);
    ::Sound->create(control_hit_sound, pSettings->r_string(section, "sound_control_hit"), st_Effect, SOUND_TYPE_WORLD);

    anim().AddReplacedAnim(&m_bDamaged, eAnimStandIdle, eAnimStandDamaged);
    anim().AddReplacedAnim(&m_bDamaged, eAnimRun, eAnimRunDamaged);
    anim().AddReplacedAnim(&m_bDamaged, eAnimWalkFwd, eAnimWalkDamaged);
    // NLC: real run clip, registered in nlc_reinit only when the visual has it; the flag stays false otherwise
    anim().AddReplacedAnim(&m_nlc_running, eAnimRun, eAnimRunTurnLeft);

    // Load control postprocess --------------------------------------------------------
    LPCSTR ppi_section = pSettings->r_string(section, "control_effector");
    m_control_effector.ppi.duality.h = pSettings->r_float(ppi_section, "duality_h");
    m_control_effector.ppi.duality.v = pSettings->r_float(ppi_section, "duality_v");
    m_control_effector.ppi.gray = pSettings->r_float(ppi_section, "gray");
    m_control_effector.ppi.blur = pSettings->r_float(ppi_section, "blur");
    m_control_effector.ppi.noise.intensity = pSettings->r_float(ppi_section, "noise_intensity");
    m_control_effector.ppi.noise.grain = pSettings->r_float(ppi_section, "noise_grain");
    m_control_effector.ppi.noise.fps = pSettings->r_float(ppi_section, "noise_fps");
    VERIFY(!fis_zero(m_control_effector.ppi.noise.fps));

    sscanf(pSettings->r_string(ppi_section, "color_base"), "%f,%f,%f", &m_control_effector.ppi.color_base.r, &m_control_effector.ppi.color_base.g,
           &m_control_effector.ppi.color_base.b);
    sscanf(pSettings->r_string(ppi_section, "color_gray"), "%f,%f,%f", &m_control_effector.ppi.color_gray.r, &m_control_effector.ppi.color_gray.g,
           &m_control_effector.ppi.color_gray.b);
    sscanf(pSettings->r_string(ppi_section, "color_add"), "%f,%f,%f", &m_control_effector.ppi.color_add.r, &m_control_effector.ppi.color_add.g,
           &m_control_effector.ppi.color_add.b);

    m_control_effector.time = pSettings->r_float(ppi_section, "time");
    m_control_effector.time_attack = pSettings->r_float(ppi_section, "time_attack");
    m_control_effector.time_release = pSettings->r_float(ppi_section, "time_release");

    m_control_effector.ce_time = pSettings->r_float(ppi_section, "ce_time");
    m_control_effector.ce_amplitude = pSettings->r_float(ppi_section, "ce_amplitude");
    m_control_effector.ce_period_number = pSettings->r_float(ppi_section, "ce_period_number");
    m_control_effector.ce_power = pSettings->r_float(ppi_section, "ce_power");

    SVelocityParam& velocity_none = move().get_velocity(MonsterMovement::eVelocityParameterIdle);
    SVelocityParam& velocity_turn = move().get_velocity(MonsterMovement::eVelocityParameterStand);
    SVelocityParam& velocity_walk = move().get_velocity(MonsterMovement::eVelocityParameterWalkNormal);
    SVelocityParam& velocity_steal = move().get_velocity(MonsterMovement::eVelocityParameterSteal);

    anim().AddAnim(eAnimStandIdle, "stand_idle_", -1, &velocity_none, PS_STAND);
    anim().AddAnim(eAnimStandTurnLeft, "stand_turn_ls_", -1, &velocity_turn, PS_STAND);
    anim().AddAnim(eAnimStandTurnRight, "stand_turn_rs_", -1, &velocity_turn, PS_STAND);
    anim().AddAnim(eAnimStandDamaged, "stand_idle_dmg_", -1, &velocity_none, PS_STAND);
    anim().AddAnim(eAnimSitIdle, "sit_idle_", -1, &velocity_none, PS_SIT);
    anim().AddAnim(eAnimEat, "sit_eat_", -1, &velocity_none, PS_SIT);
    anim().AddAnim(eAnimWalkFwd, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND);
    anim().AddAnim(eAnimWalkDamaged, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND);
    anim().AddAnim(eAnimRun, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND);
    anim().AddAnim(eAnimRunDamaged, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND);
    anim().AddAnim(eAnimAttack, "stand_attack_", -1, &velocity_turn, PS_STAND);
    anim().AddAnim(eAnimSteal, "stand_steal_", -1, &velocity_steal, PS_STAND);
    anim().AddAnim(eAnimCheckCorpse, "stand_check_corpse_", -1, &velocity_none, PS_STAND);
    anim().AddAnim(eAnimDie, "stand_die_", -1, &velocity_none, PS_STAND);
    anim().AddAnim(eAnimStandSitDown, "stand_sit_down_", -1, &velocity_none, PS_STAND);
    anim().AddAnim(eAnimSitStandUp, "sit_stand_up_", -1, &velocity_none, PS_SIT);
    anim().AddAnim(eAnimSleep, "sit_sleep_", -1, &velocity_none, PS_SIT);

    anim().LinkAction(ACT_STAND_IDLE, eAnimStandIdle);
    anim().LinkAction(ACT_SIT_IDLE, eAnimSitIdle);
    anim().LinkAction(ACT_LIE_IDLE, eAnimSitIdle);
    anim().LinkAction(ACT_WALK_FWD, eAnimWalkFwd);
    anim().LinkAction(ACT_WALK_BKWD, eAnimWalkFwd);
    anim().LinkAction(ACT_RUN, eAnimRun);
    anim().LinkAction(ACT_EAT, eAnimEat);
    anim().LinkAction(ACT_SLEEP, eAnimSleep);
    anim().LinkAction(ACT_REST, eAnimSitIdle);
    anim().LinkAction(ACT_DRAG, eAnimStandIdle);
    anim().LinkAction(ACT_ATTACK, eAnimAttack);
    anim().LinkAction(ACT_STEAL, eAnimSteal);
    anim().LinkAction(ACT_LOOK_AROUND, eAnimStandIdle);

    anim().AddTransition(PS_STAND, PS_SIT, eAnimStandSitDown, false);
    anim().AddTransition(PS_SIT, PS_STAND, eAnimSitStandUp, false);

#ifdef DEBUG
    anim().accel_chain_test();
#endif

    m_velocity_move_fwd.Load(section, "Velocity_MoveFwd");
    m_velocity_move_bkwd.Load(section, "Velocity_MoveBkwd");

    load_friend_community_overrides(section);

    // load
    m_sound_hit_fx.create("affects\\tinnitus3a", st_Effect, sg_SourceType);

    m_sound_aura_left_channel.create("monsters\\controller\\controller_psy_aura_l", st_Effect, sg_SourceType);
    m_sound_aura_right_channel.create("monsters\\controller\\controller_psy_aura_r", st_Effect, sg_SourceType);
    m_sound_aura_hit_left_channel.create("monsters\\controller\\controller_psy_hit_l", st_Effect, sg_SourceType);
    m_sound_aura_hit_right_channel.create("monsters\\controller\\controller_psy_hit_l", st_Effect, sg_SourceType);

    m_sound_tube_start.create("monsters\\controller\\controller_first_hit", st_Effect, sg_SourceType);
    m_sound_tube_pull.create("monsters\\controller\\controller_whoosh", st_Effect, sg_SourceType);
    m_sound_tube_hit_left.create("monsters\\controller\\controller_final_hit_l", st_Effect, sg_SourceType);
    m_sound_tube_hit_right.create("monsters\\controller\\controller_final_hit_r", st_Effect, sg_SourceType);

    m_sound_tube_prepare.create("monsters\\controller\\controller_tube_prepare", st_Effect, sg_SourceType);

    particles_fire = pSettings->r_string(section, "Control_Hit");

    m_tube_damage = pSettings->r_float(section, "tube_damage");
    m_tube_at_once = !!pSettings->r_bool(section, "tube_at_once");

    LPCSTR tube_see_duration_line = "tube_condition_see_duration";
    LPCSTR tube_condition_min_delay_line = "tube_condition_min_delay";
    LPCSTR tube_condition_min_distance_line = "tube_condition_min_distance";

    using namespace detail::controller;
    m_tube_condition_see_duration =
        pSettings->line_exist(section, tube_see_duration_line) ? pSettings->r_u32(section, tube_see_duration_line) : default_tube_condition_see_duration;

    m_tube_condition_min_delay =
        pSettings->line_exist(section, tube_condition_min_delay_line) ? pSettings->r_u32(section, tube_condition_min_delay_line) : default_tube_condition_min_delay;

    m_tube_condition_min_distance =
        pSettings->line_exist(section, tube_condition_min_distance_line) ? pSettings->r_float(section, tube_condition_min_distance_line) : default_tube_condition_min_distance;

    m_stamina_hit = READ_IF_EXISTS(pSettings, r_float, section, "stamina_hit", default_stamina_hit);

    nlc_load(section); // NLC

    PostLoad(section);
}

void CController::load_friend_community_overrides(LPCSTR section)
{
    LPCSTR src = pSettings->r_string(section, "Friend_Community_Overrides");

    // parse src
    int item_count = _GetItemCount(src);
    m_friend_community_overrides.resize(item_count);
    for (int i = 0; i < item_count; i++)
    {
        string128 st;
        _GetItem(src, i, st);
        m_friend_community_overrides[i] = st;
    }
}

bool CController::is_community_friend_overrides(const CEntityAlive* entity_alive) const
{
    const CInventoryOwner* IO = smart_cast<const CInventoryOwner*>(entity_alive);
    if (!IO)
        return false;
    if (const_cast<CEntityAlive*>(entity_alive)->cast_base_monster())
        return false;

    return (std::find(m_friend_community_overrides.begin(), m_friend_community_overrides.end(), IO->CharacterInfo().Community().id()) != m_friend_community_overrides.end());
}

BOOL CController::net_Spawn(CSE_Abstract* DC)
{
    if (!inherited::net_Spawn(DC))
        return (FALSE);

    return (TRUE);
}

void CController::UpdateControlled()
{
    // если есть враг, проверить может ли быть враг взят под контроль
    if (EnemyMan.get_enemy())
    {
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(const_cast<CEntityAlive*>(EnemyMan.get_enemy()));
        if (entity)
        {
            // NLC: weight budget on top of Max_Controlled_Count; nlc_take also forgets the thrall as an enemy
            if (!entity->is_under_control() && nlc_can_take(EnemyMan.get_enemy()))
            {
                // взять под контроль
                nlc_take(const_cast<CEntityAlive*>(EnemyMan.get_enemy()), "enemy");
            }
        }
    }
}

void CController::set_controlled_task(u32 task)
{
    if (!HasUnderControl())
        return;

    const CEntity* object = ((((ETask)task) == eTaskNone) ? 0 : ((((ETask)task) == eTaskFollow) ? this : EnemyMan.get_enemy()));

    for (u32 i = 0; i < m_controlled_objects.size(); i++)
    {
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(m_controlled_objects[i]);
        entity->get_data().m_object = object;
        entity->get_data().m_task = (ETask)task;
    }
}

void CController::CheckSpecParams(u32 spec_params)
{
    if ((spec_params & ASP_CHECK_CORPSE) == ASP_CHECK_CORPSE)
    {
        com_man().seq_run(anim().get_motion_id(eAnimCheckCorpse));
    }
}

void CController::InitThink()
{
    for (u32 i = 0; i < m_controlled_objects.size(); i++)
    {
        CBaseMonster* base = smart_cast<CBaseMonster*>(m_controlled_objects[i]);
        if (!base)
            continue;
        if (base->EnemyMan.get_enemy())
            EnemyMemory.add_enemy(base->EnemyMan.get_enemy(), base->EnemyMan.get_enemy_position(), base->EnemyMan.get_enemy_vertex(), base->EnemyMan.get_enemy_time_last_seen());
    }
}

void CController::play_control_sound_start()
{
    Fvector pos = EnemyMan.get_enemy()->Position();
    pos.y += 1.5f;

    if (control_start_sound._feedback())
        control_start_sound.stop();
    control_start_sound.play_at_pos(const_cast<CEntityAlive*>(EnemyMan.get_enemy()), pos);
}

void CController::play_control_sound_hit()
{
    Fvector pos = EnemyMan.get_enemy()->Position();
    pos.y += 1.5f;

    if (control_hit_sound._feedback())
        control_hit_sound.stop();
    control_hit_sound.play_at_pos(const_cast<CEntityAlive*>(EnemyMan.get_enemy()), pos);
}

void CController::reload(LPCSTR section)
{
    inherited::reload(section);
    com_man().ta_fill_data(anim_triple_control, "stand_sit_down_attack_0", "control_attack_0", "sit_stand_up_attack_0", true, false);
}

void CController::reinit()
{
    // must be before inherited call because of its use in ControlAnimation com
    m_mental_state = eStateIdle;

    nlc_reinit(); // NLC: before inherited::reinit, which counts the registered animations

    CControlledActor::reinit();
    inherited::reinit();

    m_psy_fire_start_time = 0;
    m_psy_fire_delay = _pmt_psy_attack_delay;

    control().path_builder().detail().add_velocity(
        MonsterMovement::eControllerVelocityParameterMoveFwd,
        CDetailPathManager::STravelParams(m_velocity_move_fwd.velocity.linear, m_velocity_move_fwd.velocity.angular_path, m_velocity_move_fwd.velocity.angular_real));
    control().path_builder().detail().add_velocity(
        MonsterMovement::eControllerVelocityParameterMoveBkwd,
        CDetailPathManager::STravelParams(m_velocity_move_bkwd.velocity.linear, m_velocity_move_bkwd.velocity.angular_path, m_velocity_move_bkwd.velocity.angular_real));

    m_sndShockEffector = 0;
    active_control_fx = false;
}

void CController::control_hit()
{
    Hit_Psy(const_cast<CEntityAlive*>(EnemyMan.get_enemy()), 30.f);

    // start postprocess
    CActor* pA = const_cast<CActor*>(smart_cast<const CActor*>(EnemyMan.get_enemy()));

    if (!pA)
        return;

    Actor()->Cameras().AddCamEffector(
        xr_new<CMonsterEffectorHit>(m_control_effector.ce_time, m_control_effector.ce_amplitude, m_control_effector.ce_period_number, m_control_effector.ce_power));
    Actor()->Cameras().AddPPEffector(xr_new<CMonsterEffector>(m_control_effector.ppi, m_control_effector.time, m_control_effector.time_attack, m_control_effector.time_release));

    play_control_sound_hit();
}

static const float TEXTURE_SIZE_PERCENT = 2.f;

void CController::UpdateCL()
{
    inherited::UpdateCL();

    if (m_sndShockEffector)
    {
        m_sndShockEffector->Update();

        if (!m_sndShockEffector->InWork())
            xr_delete(m_sndShockEffector);
    }

    if (active_control_fx)
    {
        u32 time_to_show = 150;
        float percent = float((Device.dwTimeGlobal - time_control_hit_started)) / float(time_to_show);
        float percent2 = 1 - (percent - TEXTURE_SIZE_PERCENT) / 2;

        if (percent < TEXTURE_SIZE_PERCENT)
        {
            HUD().GetUI()->UIGame()->RemoveCustomStatic("controller_fx2");
            SDrawStaticStruct* s = HUD().GetUI()->UIGame()->AddCustomStatic("controller_fx", true);

            float x1 = Device.dwWidth / 2 - ((Device.dwWidth / 2) * percent);
            float y1 = Device.dwHeight / 2 - ((Device.dwHeight / 2) * percent);
            float x2 = Device.dwWidth / 2 + ((Device.dwWidth / 2) * percent);
            float y2 = Device.dwHeight / 2 + ((Device.dwHeight / 2) * percent);

            s->wnd()->SetWndRect(x1, y1, x2 - x1, y2 - y1);
        }
        else if (percent2 > 0)
        {
            HUD().GetUI()->UIGame()->RemoveCustomStatic("controller_fx");
            SDrawStaticStruct* s = HUD().GetUI()->UIGame()->AddCustomStatic("controller_fx2", true);

            float x1 = Device.dwWidth / 2 - ((Device.dwWidth / 2) * percent2);
            float y1 = Device.dwHeight / 2 - ((Device.dwHeight / 2) * percent2);
            float x2 = Device.dwWidth / 2 + ((Device.dwWidth / 2) * percent2);
            float y2 = Device.dwHeight / 2 + ((Device.dwHeight / 2) * percent2);

            s->wnd()->SetWndRect(x1, y1, x2 - x1, y2 - y1);
        }
        else
        {
            active_control_fx = false;
            HUD().GetUI()->UIGame()->RemoveCustomStatic("controller_fx");
            HUD().GetUI()->UIGame()->RemoveCustomStatic("controller_fx2");
        }
    }
}

void CController::shedule_Update(u32 dt)
{
    inherited::shedule_Update(dt);

    if (g_Alive())
    {
        if (m_nlc_is_decoy)
            nlc_decoy_self_update(); // NLC: a decoy has no tube, thralls or recruits
        else
        {
            UpdateControlled();
            nlc_update_recruit(); // NLC
            nlc_update_thrall_tasks(); // NLC
            nlc_decoy_update(); // NLC
            nlc_update_thrall_eyes(); // NLC
            if (can_tube_fire())
                tube_fire();
        }
    }

    // DEBUG
    test_covers();
}

void CController::Die(CObject* who)
{
    inherited::Die(who);
    nlc_release_all(m_nlc_release_staggers > 0); // NLC: stock FreeFromControl plus the release stagger
    if (m_nlc_eyes_thrall != u16(-1)) // NLC: a thrall-eyes wind-up dies with it
    {
        m_nlc_eyes_thrall = u16(-1);
        if (m_sound_tube_prepare._feedback() && !m_psy_hit->is_active())
            m_sound_tube_prepare.stop();
    }

    m_psy_hit->on_death();
}

void CController::net_Destroy()
{
    inherited::net_Destroy();

    FreeFromControl();
}

void CController::net_Relcase(CObject* O)
{
    inherited::net_Relcase(O);

    // NLC: thralls keep the controller's enemy as their task object; nothing else clears it when that object
    // is destroyed (offline switch, release), and the thrall states dereference it
    for (CEntity* thrall : m_controlled_objects)
    {
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall);
        if (!entity)
            continue;
        SControlledInfo& info = entity->get_data();
        if (info.m_object && static_cast<const CObject*>(info.m_object) == O)
        {
            info.m_task = eTaskFollow;
            info.m_object = this;
        }
    }
}

void CController::FreeFromControl()
{
    for (u32 i = 0; i < m_controlled_objects.size(); i++)
    {
        smart_cast<CControlledEntityBase*>(m_controlled_objects[i])->free_from_control();
        if (CBaseMonster* monster = smart_cast<CBaseMonster*>(m_controlled_objects[i])) // NLC
            monster->Home->nlc_resume();
    }
    m_controlled_objects.clear();
    m_nlc_thralls.clear(); // NLC
}

void CController::OnFreedFromControl(const CEntity* entity)
{
    m_nlc_thralls.erase(entity); // NLC

    for (u32 i = 0; i < m_controlled_objects.size(); i++)
        if (m_controlled_objects[i] == entity)
        {
            m_controlled_objects[i] = m_controlled_objects.back();
            m_controlled_objects.pop_back();
            return;
        }
}

//////////////////////////////////////////////////////////////////////////

void CController::draw_fire_particles()
{
    if (!EnemyMan.get_enemy())
        return;

    CEntityAlive* enemy = const_cast<CEntityAlive*>(EnemyMan.get_enemy());
    if (!EnemyMan.see_enemy_now())
        return;

    // вычислить позицию и направленность партикла
    Fvector my_head_pos;
    my_head_pos.set(get_head_position(this));

    Fvector position;
    position.set(get_head_position(enemy));
    position.y -= 0.5f;

    Fvector dir;
    dir.sub(position, my_head_pos);
    dir.normalize();

    PlayParticles(particles_fire, my_head_pos, dir);
    play_control_sound_hit();
}

void CController::psy_fire()
{
    if (!EnemyMan.get_enemy())
        return;

    draw_fire_particles();
}

bool CController::can_psy_fire()
{
    if (m_psy_fire_start_time + m_psy_fire_delay > time())
    {
        return false;
    }

    if (!EnemyMan.get_enemy())
    {
        return false;
    }

    if (!EnemyMan.see_enemy_now())
    {
        return false;
    }

    float cur_yaw = custom_dir().get_head_orientation().current.yaw;
    float dir_yaw = Fvector().sub(EnemyMan.get_enemy()->Position(), Position()).getH();
    dir_yaw = angle_normalize(-dir_yaw);

    if (angle_difference(cur_yaw, dir_yaw) > _pmt_psy_attack_min_angle)
    {
        return false;
    }

    m_psy_fire_start_time = time();
    return true;
}

void CController::set_psy_fire_delay_zero() { m_psy_fire_delay = 0; }
void CController::set_psy_fire_delay_default() { m_psy_fire_delay = _pmt_psy_attack_delay; }

//////////////////////////////////////////////////////////////////////////
// TUBE
//////////////////////////////////////////////////////////////////////////

void CController::tube_fire() { control().activate(ControlCom::eComCustom1); }

bool CController::can_tube_fire()
{
    using namespace detail::controller;

    if (0 && m_tube_at_once)
    {
        if (EnemyMan.get_enemy() && EnemyMan.see_enemy_now() && m_psy_hit->check_start_conditions())
        {
            return true;
        }

        return false;
    }

    if (!EnemyMan.get_enemy())
        return false;

    if (EnemyMan.see_enemy_duration() < m_tube_condition_see_duration && !nlc_psy_sense()) // NLC
        return false;

    if (!m_psy_hit->check_start_conditions())
        return false;

    if (EnemyMan.get_enemy()->Position().distance_to(Position()) < m_tube_condition_min_distance)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////

const MonsterSpace::SBoneRotation& CController::head_orientation() const { return m_custom_dir_base->get_head_orientation(); }

void CController::test_covers()
{
    //////////////////////////////////////////////////////////////////////////
    // update covers
    //////////////////////////////////////////////////////////////////////////
}

void CController::create_base_controls()
{
    m_custom_anim_base = xr_new<CControllerAnimation>();
    m_custom_dir_base = xr_new<CControllerDirection>();

    m_anim_base = m_custom_anim_base;
    m_dir_base = m_custom_dir_base;

    m_move_base = xr_new<CControlMovementBase>();
    m_path_base = xr_new<CControlPathBuilderBase>();
}

void CController::TranslateActionToPathParams()
{
    // if (m_mental_state == eStateIdle) {
    //	inherited::TranslateActionToPathParams();
    //	return;
    // }
    // custom_anim().set_path_params();

    // NLC: real run (eAnimRun replaced by the run clip) uses the run velocities
    if (m_nlc_running && !m_bDamaged && (anim().m_tAction == ACT_RUN))
    {
        inherited::TranslateActionToPathParams();
        return;
    }

    if ((anim().m_tAction != ACT_RUN) && (anim().m_tAction != ACT_WALK_FWD))
    {
        inherited::TranslateActionToPathParams();
        return;
    }

    u32 vel_mask = (m_bDamaged ? MonsterMovement::eVelocityParamsWalkDamaged : MonsterMovement::eVelocityParamsWalk);
    u32 des_mask = (m_bDamaged ? MonsterMovement::eVelocityParameterWalkDamaged : MonsterMovement::eVelocityParameterWalkNormal);

    if (m_force_real_speed)
        vel_mask = des_mask;

    path().set_velocity_mask(vel_mask);
    path().set_desirable_mask(des_mask);
    path().enable_path();
}

bool CController::is_relation_enemy(const CEntityAlive* tpEntityAlive) const
{
    //	MONSTER_COMMUNITY_ID
    if (xr_strcmp(*(tpEntityAlive->cNameSect()), "stalker_zombied") == 0)
        return false;
    if (is_community_friend_overrides(tpEntityAlive))
        return false;

    // NLC: game_relations.ltx still makes the species hostile; never target its own thralls
    if (std::find(m_controlled_objects.begin(), m_controlled_objects.end(), tpEntityAlive) != m_controlled_objects.end())
        return false;

    return inherited::is_relation_enemy(tpEntityAlive);
}

void CController::set_mental_state(EMentalState state)
{
    if (m_mental_state == state)
        return;

    m_mental_state = state;

    m_custom_anim_base->on_switch_controller();
}

void CController::HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type, bool draw_hit_marks)
{
    if (pEntity == Actor() && !GodMode())
    {
        Actor()->conditions().PowerHit(m_stamina_hit, false);
        if (Actor()->conditions().GetPower() < m_stamina_hit)
        {
            if (!Actor()->inventory().Action((u16)kDROP, CMD_STOP))
            {
                Actor()->g_PerformDrop();
            }
        }
    }

    inherited::HitEntity(pEntity, fDamage, impulse, dir, hit_type, draw_hit_marks);
}

bool CController::tube_ready() const { return m_psy_hit && m_psy_hit->tube_ready(); }

//////////////////////////////////////////////////////////////////////////
// NLC: controller redesign (docs/CONTROLLER_REDESIGN.md).
// Every key is optional; without them the controller behaves as in stock OGSR.
//////////////////////////////////////////////////////////////////////////

namespace
{
Fvector2 nlc_read_pair(LPCSTR section, LPCSTR key, float a, float b) { return READ_IF_EXISTS(pSettings, r_fvector2, section, key, Fvector2().set(a, b)); }

LPCSTR nlc_phase_name(u8 phase)
{
    static const char* names[] = {"off", "hide", "camp", "peek", "aim", "hold"};
    return phase < sizeof(names) / sizeof(names[0]) ? names[phase] : "?";
}

LPCSTR nlc_inaccessible_name(u8 reason)
{
    switch (reason)
    {
    case CBaseMonster::eNlcAccessible: return "accessible (same ai-map vertex or grace)";
    case CBaseMonster::eNlcInaccessibleHigh: return "high above/below its ai-map vertex";
    case CBaseMonster::eNlcInaccessibleOffMap: return "off the ai-map (crate, vehicle)";
    case CBaseMonster::eNlcInaccessibleHome: return "outside home restrictor";
    case CBaseMonster::eNlcInaccessibleRestrictor: return "no accessible point nearby (restrictor/anomaly)";
    case CBaseMonster::eNlcInaccessibleVertexPos: return "invalid vertex position";
    case CBaseMonster::eNlcInaccessibleVertexId: return "invalid vertex id";
    default: return "unknown";
    }
}

// thrall species: "species" key of the monster section
shared_str nlc_species(const CEntity* entity) { return READ_IF_EXISTS(pSettings, r_string, entity->cNameSect(), "species", ""); }
} // namespace

void CController::nlc_load(LPCSTR section)
{
    m_nlc_debug = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_debug", false);

    // thralls
    m_nlc_thrall_attack = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_thrall_attack", false);
    m_nlc_thrall_memory = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_thrall_memory", 15000);
    m_nlc_budget = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_thrall_budget", -1.f);
    m_nlc_thrall_table = READ_IF_EXISTS(pSettings, r_string, section, "nlc_ctrl_thrall_table", "");
    if (m_nlc_thrall_table.size() && !pSettings->section_exist(m_nlc_thrall_table.c_str()))
        m_nlc_thrall_table = "";

    m_nlc_guard_species.clear();
    if (LPCSTR guards = READ_IF_EXISTS(pSettings, r_string, section, "nlc_ctrl_guard_species", nullptr))
    {
        string128 item;
        for (int i = 0, n = _GetItemCount(guards); i < n; ++i)
            m_nlc_guard_species.push_back(_Trim(_GetItem(guards, i, item)));
    }
    m_nlc_guard_max = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_guard_max", 0);
    m_nlc_guard_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_guard_dist", 4.f);
    m_nlc_flank_offset = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_flank_offset", 9.f);
    m_nlc_leash = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_leash_dist", 0.f);
    Fvector2 v = nlc_read_pair(section, "nlc_ctrl_release_stagger", 0.f, 1.f);
    m_nlc_release_staggers = u32(std::max(v.x, 0.f));
    m_nlc_release_stagger_speed = std::clamp(v.y, 0.3f, 3.f);
    v = nlc_read_pair(section, "nlc_ctrl_take_stagger", 0.f, 1.f);
    m_nlc_take_staggers = u32(std::max(v.x, 0.f));
    m_nlc_take_stagger_speed = std::clamp(v.y, 0.3f, 3.f);
    v = nlc_read_pair(section, "nlc_ctrl_release_slow", 0.f, 0.f);
    m_nlc_release_slow_k = std::clamp(v.x, 0.f, 0.9f);
    m_nlc_release_slow_time = u32(std::max(v.y, 0.f));

    // active recruitment
    m_nlc_recruit_radius = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_recruit_radius", 0.f);
    m_nlc_recruit_time = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_recruit_time", 1500);
    m_nlc_recruit_delay = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_recruit_delay", 12000);
    m_nlc_recruit_idle = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_recruit_idle", false);
    m_nlc_recruit_anim = READ_IF_EXISTS(pSettings, r_string, section, "nlc_ctrl_recruit_anim", "");

    // ranged behavior
    m_nlc_reposition = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_reposition", false);
    m_nlc_cover_loop = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_cover_loop", false);
    m_nlc_close_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_close_dist", 5.f);
    m_nlc_lost_time = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_lost_time", 10000);
    v = nlc_read_pair(section, "nlc_ctrl_cover_dist", 12.f, 30.f);
    m_nlc_cover_min = std::max(v.x, 1.f);
    m_nlc_cover_max = std::max(v.y, m_nlc_cover_min + 1.f);
    v = nlc_read_pair(section, "nlc_ctrl_range_dist", 12.f, 20.f);
    m_nlc_range_min = std::max(v.x, 1.f);
    m_nlc_range_max = std::max(v.y, m_nlc_range_min);
    m_nlc_peek_radius = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_peek_radius", 8.f);
    v = nlc_read_pair(section, "nlc_ctrl_hold_time", 2500.f, 5000.f);
    m_nlc_hold_min = u32(std::max(v.x, 500.f));
    m_nlc_hold_max = std::max(u32(v.y), m_nlc_hold_min);
    m_nlc_aim_time = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_aim_time", 5000);
    m_nlc_move_timeout = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_move_timeout", 12000);
    m_nlc_run_anim = READ_IF_EXISTS(pSettings, r_string, section, "nlc_ctrl_run_anim", "");

    // flushed reaction and thrall eyes
    m_nlc_flush_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_flush_dist", 0.f);
    m_nlc_flush_psy = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_flush_psy", 0.2f);
    m_nlc_flush_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_flush_cooldown", 8000);
    m_nlc_thrall_eyes = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_thrall_eyes", false);
    m_nlc_eyes_power = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_thrall_eyes_power", 0.08f);
    m_nlc_eyes_range = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_thrall_eyes_range", 30.f);
    m_nlc_eyes_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_thrall_eyes_cooldown", 6000);
    m_nlc_eyes_windup = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_thrall_eyes_windup", 450);

    // Mirage decoy
    m_nlc_is_decoy = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_ctrl_decoy", false);
    m_nlc_decoy_life = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_decoy_life", 12000);
    m_nlc_decoy_section = m_nlc_is_decoy ? "" : READ_IF_EXISTS(pSettings, r_string, section, "nlc_ctrl_decoy_section", "");
    if (m_nlc_decoy_section.size() && !pSettings->section_exist(m_nlc_decoy_section.c_str()))
        m_nlc_decoy_section = "";
    m_nlc_decoy_chance = READ_IF_EXISTS(pSettings, r_float, section, "nlc_ctrl_decoy_chance", 0.f);
    m_nlc_decoy_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_ctrl_decoy_cooldown", 30000);
}

void CController::nlc_reinit()
{
    m_nlc_phase = eNlcPhaseOff;
    m_nlc_running = false;
    m_nlc_channel_target = u16(-1);
    m_nlc_decoy_id = u16(-1);
    m_nlc_decoy_next = 0;
    m_nlc_flush_next = 0;
    m_nlc_eyes_next = 0;
    m_nlc_eyes_thrall = u16(-1);
    m_nlc_eyes_strike = 0;
    m_nlc_herd_until = 0;
    m_nlc_decoy_die_at = 0;
    m_nlc_decoy_vanishing = false;
    m_nlc_recruit_next = 0;
    m_nlc_thralls.clear();

    IKinematicsAnimated* skel = smart_cast<IKinematicsAnimated*>(Visual());

    // a missing clip in the animation storage is fatal (UpdateAnimCount), so register the run only
    // when this visual has it (kontroler_st is unverified)
    m_nlc_run_ok = false;
    if (skel && m_nlc_run_anim.size())
    {
        string128 first;
        xr_strconcat(first, m_nlc_run_anim.c_str(), "0");
        if (skel->ID_Cycle_Safe(first).valid())
        {
            if (!m_nlc_run_registered)
            {
                anim().AddAnim(eAnimRunTurnLeft, m_nlc_run_anim.c_str(), -1, &move().get_velocity(MonsterMovement::eVelocityParameterRunNormal), PS_STAND);
                m_nlc_run_registered = true;
            }
            m_nlc_run_ok = true;
        }
        else if (m_nlc_debug)
            Msg("~ [controller] [%s]: visual has no [%s], keeps walking", cName().c_str(), first);
    }

    // recruit channel: the stock control triple (sit down, control, stand up) when the visual has all three
    // clips, else the first clip of nlc_ctrl_recruit_anim that exists, else no animation
    m_nlc_triple_ok = skel && skel->ID_Cycle_Safe("stand_sit_down_attack_0").valid() && skel->ID_Cycle_Safe("control_attack_0").valid() &&
        skel->ID_Cycle_Safe("sit_stand_up_attack_0").valid();

    m_nlc_recruit_motion = MotionID();
    if (skel && m_nlc_recruit_anim.size())
    {
        string64 item;
        for (int i = 0, n = _GetItemCount(m_nlc_recruit_anim.c_str()); i < n && !m_nlc_recruit_motion.valid(); ++i)
            m_nlc_recruit_motion = skel->ID_Cycle_Safe(_Trim(_GetItem(m_nlc_recruit_anim.c_str(), i, item)));
    }

    if (m_nlc_debug && m_nlc_recruit_radius > 0.f)
        Msg("~ [controller] [%s]: recruit channel animation: %s, run clip: %s", cName().c_str(),
            m_nlc_triple_ok ? "control triple" : (m_nlc_recruit_motion.valid() ? "clip" : "none"), m_nlc_run_ok ? "yes" : "no");
}

//////////////////////////////////////////////////////////////////////////
// Thralls
//////////////////////////////////////////////////////////////////////////

float CController::nlc_thrall_weight(const CEntity* entity) const
{
    if (!m_nlc_thrall_table.size())
        return 1.f;

    const shared_str species = nlc_species(entity);
    if (!species.size() || !pSettings->line_exist(m_nlc_thrall_table.c_str(), species.c_str()))
        return 1.f;

    string64 item;
    return std::max(0.f, float(atof(_GetItem(pSettings->r_string(m_nlc_thrall_table.c_str(), species.c_str()), 0, item))));
}

float CController::nlc_thralls_weight() const
{
    float sum = 0.f;
    for (const auto& it : m_nlc_thralls)
        sum += it.second.weight;
    return sum;
}

bool CController::nlc_can_take(const CEntityAlive* entity) const
{
    if (m_controlled_objects.size() >= m_max_controlled_number)
        return false;
    if (m_nlc_budget < 0.f)
        return true;
    return nlc_thralls_weight() + nlc_thrall_weight(entity) <= m_nlc_budget + EPS_L;
}

void CController::nlc_take(CEntityAlive* thrall, LPCSTR how, bool stagger)
{
    CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall);
    if (!entity || entity->is_under_control())
        return;

    entity->set_under_control(this);
    entity->set_task_follow(this);
    m_controlled_objects.push_back(thrall);

    // its home would pull it back (the attack state returns home when the enemy is outside it)
    CBaseMonster* thrall_monster = smart_cast<CBaseMonster*>(thrall);
    if (thrall_monster)
    {
        thrall_monster->Home->nlc_suspend();
        if (m_nlc_take_staggers && stagger)
            thrall_monster->nlc_stagger_repeat(m_nlc_take_staggers, m_nlc_take_stagger_speed);
    }

    // forget it as an enemy right away (remembered entries and the "who just hit me" path would let target
    // selection pick the thrall again). This can clear EnemyMan's current enemy.
    EnemyMemory.remove_links(thrall);
    HitMemory.remove_hit_info(thrall);

    SNlcThrall info;
    info.weight = nlc_thrall_weight(thrall);
    info.side = m_nlc_next_side;
    m_nlc_next_side = -m_nlc_next_side;

    const shared_str species = nlc_species(thrall);
    if (m_nlc_thrall_table.size() && species.size() && pSettings->line_exist(m_nlc_thrall_table.c_str(), species.c_str()))
    {
        string64 role;
        _GetItem(pSettings->r_string(m_nlc_thrall_table.c_str(), species.c_str()), 1, role);
        _Trim(role);
        if (!xr_strcmp(role, "flank"))
            info.role = eNlcRoleFlank;
        else if (!xr_strcmp(role, "guard"))
            info.role = eNlcRoleGuard;
    }

    if (species.size() && std::find(m_nlc_guard_species.begin(), m_nlc_guard_species.end(), species) != m_nlc_guard_species.end())
    {
        // one or two meat shields stay with the controller; the rest pressure the enemy (flank, hunt alternating)
        info.guard_capable = true;
        u32 guards = 0;
        for (const auto& it : m_nlc_thralls)
            guards += (it.second.role == eNlcRoleGuard) ? 1 : 0;
        if (guards < m_nlc_guard_max)
            info.role = eNlcRoleGuard;
        else
        {
            info.role = m_nlc_overflow_flank ? eNlcRoleFlank : eNlcRoleHunt;
            m_nlc_overflow_flank = !m_nlc_overflow_flank;
        }
    }

    m_nlc_thralls[thrall] = info;

    if (m_nlc_debug)
    {
        static const char* roles[] = {"hunt", "flank", "guard"};
        Msg("~ [controller] [%s]: took [%s] under control (%s, role %s, weight %.1f, %u/%u, budget %.1f/%.1f)", cName().c_str(), thrall->cName().c_str(), how,
            roles[info.role], info.weight, u32(m_controlled_objects.size()), u32(m_max_controlled_number), nlc_thralls_weight(), m_nlc_budget);
        if (thrall_monster)
        {
            const shared_str out_r = thrall_monster->movement().restrictions().out_restrictions();
            const shared_str in_r = thrall_monster->movement().restrictions().in_restrictions();
            if (out_r.size() || in_r.size())
                Msg("~ [controller] [%s]: thrall [%s] keeps restrictions out [%s] in [%s]", cName().c_str(), thrall->cName().c_str(), out_r.c_str(), in_r.c_str());
        }
    }
}

// freed thrall: home back, then the repeated (slowed) stagger and the optional slow
void CController::nlc_release_effects(CEntity* thrall)
{
    CBaseMonster* monster = smart_cast<CBaseMonster*>(thrall);
    if (!monster)
        return;

    monster->Home->nlc_resume();

    if (!monster->g_Alive())
        return;
    if (m_nlc_release_staggers)
        monster->nlc_stagger_repeat(m_nlc_release_staggers, m_nlc_release_stagger_speed);
    monster->nlc_apply_move_slow(m_nlc_release_slow_k, m_nlc_release_slow_time);
}

void CController::nlc_release(CEntity* thrall, bool stagger, LPCSTR why)
{
    auto it = std::find(m_controlled_objects.begin(), m_controlled_objects.end(), thrall);
    if (it == m_controlled_objects.end())
        return;

    if (CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall))
        entity->free_from_control();

    *it = m_controlled_objects.back();
    m_controlled_objects.pop_back();
    m_nlc_thralls.erase(thrall);

    if (stagger)
        nlc_release_effects(thrall);
    else if (CBaseMonster* monster = smart_cast<CBaseMonster*>(thrall))
        monster->Home->nlc_resume();

    if (m_nlc_debug)
        Msg("~ [controller] [%s]: released [%s] (%s)", cName().c_str(), thrall->cName().c_str(), why);
}

void CController::nlc_release_all(bool stagger)
{
    // free first: the stagger must run in the monster's own state machine, not the controlled one
    for (CEntity* thrall : m_controlled_objects)
    {
        if (CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall))
            entity->free_from_control();

        if (stagger)
            nlc_release_effects(thrall);
        else if (CBaseMonster* monster = smart_cast<CBaseMonster*>(thrall))
            monster->Home->nlc_resume();

        if (m_nlc_debug)
            Msg("~ [controller] [%s]: released [%s] (controller died)", cName().c_str(), thrall->cName().c_str());
    }

    m_controlled_objects.clear();
    m_nlc_thralls.clear();
}

// Thralls get a role-based task: hunt attacks the controller's enemy, flank first runs to a point beside it,
// guard holds a point between the controller and the enemy. Stock code never called set_controlled_task,
// so thralls always followed and sat down next to the controller.
void CController::nlc_update_thrall_tasks()
{
    if (!m_nlc_thrall_attack || m_controlled_objects.empty())
        return;

    const u32 now = time();

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (enemy && (enemy->getDestroy() || !enemy->g_Alive() || now > EnemyMan.get_enemy_time_last_seen() + m_nlc_thrall_memory))
        enemy = nullptr;

    // right after a capture the controller's enemy can still be the new thrall itself
    if (enemy && std::find(m_controlled_objects.begin(), m_controlled_objects.end(), enemy) != m_controlled_objects.end())
        enemy = nullptr;

    // leash first (releasing changes the vector)
    if (m_nlc_leash > 0.f)
    {
        for (u32 i = 0; i < m_controlled_objects.size();)
        {
            CEntity* thrall = m_controlled_objects[i];
            if (thrall->Position().distance_to(Position()) > m_nlc_leash)
                nlc_release(thrall, m_nlc_release_staggers > 0, "out of leash range");
            else
                ++i;
        }
    }

    Fvector to_enemy{};
    if (enemy)
    {
        to_enemy.sub(enemy->Position(), Position());
        to_enemy.y = 0.f;
        to_enemy.normalize_safe();
    }

    // refill guard posts (a guard died or was released) from guard-capable thralls
    if (m_nlc_guard_max)
    {
        u32 guards = 0;
        for (const auto& it : m_nlc_thralls)
            guards += (it.second.role == eNlcRoleGuard) ? 1 : 0;
        for (auto& it : m_nlc_thralls)
        {
            if (guards >= m_nlc_guard_max)
                break;
            if (it.second.guard_capable && it.second.role != eNlcRoleGuard)
            {
                it.second.role = eNlcRoleGuard;
                it.second.guard_engaged = false;
                ++guards;
                if (m_nlc_debug)
                    Msg("~ [controller] [%s]: thrall [%s] promoted to guard", cName().c_str(), it.first->cName().c_str());
            }
        }
    }

    u32 guard_index = 0;
    for (CEntity* thrall : m_controlled_objects)
    {
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall);
        CBaseMonster* monster = smart_cast<CBaseMonster*>(thrall);
        if (!entity || !monster)
            continue;

        SNlcThrall& t = m_nlc_thralls[thrall];
        SControlledInfo& info = entity->get_data();

        ETask task = eTaskFollow;
        const CEntity* target = this;
        Fvector point{};
        u32 node = u32(-1);

        if (enemy)
        {
            task = eTaskAttack;
            target = enemy;

            const float thrall_to_enemy = thrall->Position().distance_to(enemy->Position());

            if (t.role == eNlcRoleFlank && !t.flank_done)
            {
                if (!t.flank_until)
                {
                    // a point beside the enemy, on this thrall's side, halfway between it and the enemy
                    Fvector v = Fvector().sub(thrall->Position(), enemy->Position());
                    v.y = 0.f;
                    const float d = v.magnitude();
                    v.normalize_safe();
                    Fvector side = Fvector().set(-v.z, 0.f, v.x).mul(float(t.side));
                    t.flank_point.mad(enemy->Position(), v, d * 0.5f).mad(side, m_nlc_flank_offset);
                    const Fvector wanted = t.flank_point;
                    if (!nlc_resolve_point(monster, wanted, t.flank_point, t.flank_node))
                    {
                        t.flank_node = u32(-1); // no reachable point there: just attack
                        if (m_nlc_debug)
                            Msg("~ [controller] [%s]: thrall [%s] flank point unreachable, attacking directly", cName().c_str(), thrall->cName().c_str());
                    }
                    t.flank_until = now + 8000;
                }

                if (!ai().level_graph().valid_vertex_id(t.flank_node) || now > t.flank_until || thrall->Position().distance_to_xz(t.flank_point) < 3.f ||
                    thrall_to_enemy < 6.f)
                    t.flank_done = true;
                else
                {
                    task = eTaskMove;
                    point = t.flank_point;
                    node = t.flank_node;
                }
            }
            else if (t.role == eNlcRoleGuard)
            {
                // holds its post; bites when the enemy comes within 4 m of it near the post, returns once the
                // enemy is 7 m away or the chase has taken it 10 m past the post (it must not hunt far off)
                const float from_controller = thrall->Position().distance_to(Position());
                if (t.guard_engaged)
                    t.guard_engaged = (thrall_to_enemy < 7.f) && (from_controller < m_nlc_guard_dist + 10.f);
                else
                    t.guard_engaged = (thrall_to_enemy < 4.f) && (from_controller < m_nlc_guard_dist + 6.f);
                if (!t.guard_engaged)
                {
                    // spread guards sideways: 0, +1.8, -1.8, +3.6 ...
                    const float lateral = (guard_index == 0) ? 0.f : 1.8f * float((guard_index + 1) / 2) * ((guard_index & 1) ? 1.f : -1.f);
                    ++guard_index;

                    // the exact post is often just off the ai-map (walls, slopes): try it, then closer to the
                    // controller and without the sideways spread, before falling back to following
                    Fvector side = Fvector().set(-to_enemy.z, 0.f, to_enemy.x);
                    bool found = false;
                    for (const float k : {1.f, 0.6f, 0.3f})
                    {
                        Fvector wanted;
                        wanted.mad(Position(), to_enemy, m_nlc_guard_dist * k).mad(side, lateral * k);
                        if (nlc_resolve_point(monster, wanted, point, node))
                        {
                            found = true;
                            break;
                        }
                    }

                    if (found)
                        task = eTaskMove;
                    else
                    {
                        task = eTaskFollow; // post unreachable: stay with the controller, never chase
                        target = this;
                    }
                }
            }

            // herd call: the whole herd charges, posts and flanks are dropped
            if (now < m_nlc_herd_until)
            {
                task = eTaskAttack;
                target = enemy;
            }
        }
        else
        {
            t.flank_done = false;
            t.flank_until = 0;
            t.guard_engaged = false;
        }

        const bool changed = (info.m_task != task) || (info.m_object != target);

        info.m_task = task;
        info.m_object = target;
        if (task == eTaskMove)
        {
            info.m_position = point;
            info.m_node = node;
            info.m_radius = (t.role == eNlcRoleGuard) ? 1.f : 1.5f; // guards hold the line more tightly
        }

        if (changed && m_nlc_debug)
        {
            static const char* tasks[] = {"follow", "attack", "move"};
            Msg("~ [controller] [%s]: thrall [%s] -> %s [%s]", cName().c_str(), thrall->cName().c_str(), tasks[task], target->cName().c_str());
        }
    }

    if (m_nlc_debug && now >= m_nlc_next_status_log)
    {
        m_nlc_next_status_log = now + 3000;
        nlc_log_thralls(enemy);
    }
}

// debug: one line per thrall every 3 s (role, task, distances), so stuck or runaway thralls show in the log
void CController::nlc_log_thralls(const CEntityAlive* enemy)
{
    static const char* roles[] = {"hunt", "flank", "guard"};
    static const char* tasks[] = {"follow", "attack", "move"};
    for (CEntity* thrall : m_controlled_objects)
    {
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall);
        if (!entity)
            continue;
        const SNlcThrall& t = m_nlc_thralls[thrall];
        const SControlledInfo& info = entity->get_data();
        const u32 task = u32(info.m_task);
        string64 post = "";
        if (task == eTaskMove)
            xr_sprintf(post, ", post %.1f m away", thrall->Position().distance_to_xz(info.m_position));
        Msg("~ [controller] [%s]: status [%s] role %s%s task %s, %.1f m from controller, %.1f m from enemy%s", cName().c_str(), thrall->cName().c_str(), roles[t.role],
            t.guard_engaged ? " (engaged)" : "", task < 3 ? tasks[task] : "?", thrall->Position().distance_to(Position()),
            enemy ? thrall->Position().distance_to(enemy->Position()) : -1.f, post);
    }
}

bool CController::nlc_resolve_point(CBaseMonster* monster, const Fvector& wanted, Fvector& pos, u32& node) const
{
    return monster->nlc_point_on_map(wanted, pos, node); // NLC: moved to CBaseMonster (siege uses it too)
}

// Elder/Shepherd: dominate an eligible mutant in sight. The channel plays a clip and is broken by any hit.
void CController::nlc_update_recruit()
{
    if (m_nlc_recruit_radius <= 0.f)
        return;

    const u32 now = time();

    if (m_nlc_channel_target != u16(-1))
    {
        CEntityAlive* target = smart_cast<CEntityAlive*>(Level().Objects.net_Find(m_nlc_channel_target));
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(target);

        LPCSTR abort = nullptr;
        if (!target || target->getDestroy() || !target->g_Alive() || !entity || entity->is_under_control() || !nlc_can_take(target))
            abort = "target lost";
        else if (HitMemory.is_hit() && HitMemory.get_last_hit_time() > m_nlc_channel_start)
            abort = "interrupted by a hit";

        if (abort)
        {
            if (m_nlc_channel_triple && com_man().ta_is_active())
                com_man().ta_deactivate();
            else if (!m_nlc_channel_triple && com_man().seq_active())
                com_man().seq_stop();
            m_nlc_channel_target = u16(-1);
            m_nlc_recruit_next = now + m_nlc_recruit_delay / 2;
            if (m_nlc_debug)
                Msg("~ [controller] [%s]: recruit channel %s", cName().c_str(), abort);
            return;
        }

        if (now < m_nlc_channel_end)
            return;

        if (m_nlc_channel_triple && com_man().ta_is_active())
            com_man().ta_pointbreak(); // stand up
        else if (!m_nlc_channel_triple && com_man().seq_active())
            com_man().seq_stop();
        m_nlc_channel_target = u16(-1);
        m_nlc_recruit_next = now + m_nlc_recruit_delay;
        play_control_sound_start_at(target);
        nlc_take(target, "recruited");
        return;
    }

    if (now < m_nlc_recruit_next)
        return;
    m_nlc_recruit_next = now + 1000; // scan rate

    if (!EnemyMan.get_enemy() && !m_nlc_recruit_idle)
        return;
    if (m_controlled_objects.size() >= m_max_controlled_number)
        return;
    if (control().is_captured_pure() || com_man().seq_active() || com_man().ta_is_active() || m_psy_hit->is_active())
        return;

    m_nlc_nearest.clear();
    Level().ObjectSpace.GetNearest(m_nlc_nearest, Position(), m_nlc_recruit_radius, this);

    CBaseMonster* best = nullptr;
    float best_dist = flt_max;
    for (CObject* O : m_nlc_nearest)
    {
        CBaseMonster* monster = smart_cast<CBaseMonster*>(O);
        if (!monster || monster == this || monster->getDestroy() || !monster->g_Alive())
            continue;

        // scripted psy phantoms (sivol_psy_phantoms) are flagged invisible for zones
        if (!monster->m_visible_for_zones)
            continue;

        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(monster);
        if (!entity || entity->is_under_control() || !nlc_can_take(monster))
            continue;

        if (!memory().visual().visible_now(monster))
            continue;

        const float dist = monster->Position().distance_to(Position());
        if (dist < best_dist)
        {
            best_dist = dist;
            best = monster;
        }
    }

    if (!best)
        return;

    m_nlc_channel_target = best->ID();
    m_nlc_channel_start = now;
    m_nlc_channel_end = now + m_nlc_recruit_time;
    m_nlc_channel_triple = m_nlc_triple_ok;
    if (m_nlc_channel_triple)
        com_man().ta_activate(anim_triple_control);
    else if (m_nlc_recruit_motion.valid())
        com_man().seq_run(m_nlc_recruit_motion);
    dir().face_target(best);

    if (m_nlc_debug)
        Msg("~ [controller] [%s]: recruit channel on [%s], %.1f m (%s)", cName().c_str(), best->cName().c_str(), best_dist,
            m_nlc_channel_triple ? "control triple" : (m_nlc_recruit_motion.valid() ? "clip" : "no clip"));
}

void CController::play_control_sound_start_at(const CEntityAlive* target)
{
    Fvector pos = target->Position();
    pos.y += 1.f;

    if (control_start_sound._feedback())
        control_start_sound.stop();
    control_start_sound.play_at_pos(const_cast<CEntityAlive*>(target), pos);
}

//////////////////////////////////////////////////////////////////////////
// Ranged behavior: hide in cover while the tube recharges, peek out to a point with line of sight
// when it is ready, stock melee when the enemy is close, stock hunt when it was lost for long.
//////////////////////////////////////////////////////////////////////////

bool CController::nlc_has_los(const Fvector& feet, const CEntityAlive* enemy) const
{
    return nlc_los_to(feet, enemy, 1.6f); // NLC: moved to CBaseMonster (siege uses it too)
}

bool CController::nlc_set_target(Fvector pos, u32 node, ENlcPhase phase, LPCSTR kind)
{
    if (!ai().level_graph().valid_vertex_id(node) || !movement().restrictions().accessible(pos))
    {
        // rejects far-off nearest vertices (logged a 345 m "range" pick)
        const Fvector wanted = pos;
        if (!nlc_resolve_point(this, wanted, pos, node))
            return false;
    }

    // hide moves run beyond 5 m; peeks walk, except long approaches into tube range
    m_nlc_run_this_move = m_nlc_run_ok && (Position().distance_to_xz(pos) > ((phase == eNlcPhaseHide) ? 5.f : 15.f));

    const u32 now = time();
    m_nlc_target_pos = pos;
    m_nlc_target_node = node;
    m_nlc_phase = phase;
    m_nlc_phase_start = now;
    m_nlc_phase_until = now + m_nlc_move_timeout;
    path().prepare_builder();

    if (m_nlc_debug && EnemyMan.get_enemy())
        Msg("~ [controller] [%s]: %s -> %s, %.1f m away, %.1f m from enemy (tube %s)", cName().c_str(), nlc_phase_name(phase), kind, Position().distance_to(pos),
            pos.distance_to(EnemyMan.get_enemy_position()), tube_ready() ? "ready" : "recharging");
    return true;
}

void CController::nlc_set_phase_hold(ENlcPhase phase, u32 time_ms)
{
    const u32 now = time();
    m_nlc_phase = phase;
    m_nlc_phase_start = now;
    m_nlc_phase_until = now + time_ms;
}

bool CController::nlc_select_cover()
{
    const Fvector enemy_pos = EnemyMan.get_enemy_position();
    const CCoverPoint* point = CoverMan->find_cover(enemy_pos, m_nlc_cover_min, m_nlc_cover_max);
    if (!point || point->position().distance_to_xz(Position()) < 2.f)
        return false;
    return nlc_set_target(point->position(), point->level_vertex_id(), eNlcPhaseHide, "cover");
}

// a point in tube range with line of sight to the enemy, as close to the controller as possible
bool CController::nlc_select_peek()
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return false;

    auto in_range = [&](const Fvector& p) {
        const float d = p.distance_to(enemy->Position());
        return d >= m_nlc_range_min && d <= m_nlc_range_max + 5.f;
    };

    // already fine where it stands
    if (in_range(Position()) && nlc_has_los(Position(), enemy))
    {
        nlc_set_phase_hold(eNlcPhaseAim, m_nlc_aim_time);
        if (m_nlc_debug)
            Msg("~ [controller] [%s]: aim from here, %.1f m from enemy", cName().c_str(), Position().distance_to(enemy->Position()));
        return true;
    }

    const float start = ::Random.randF(0.f, PI_MUL_2);
    for (float radius : {m_nlc_peek_radius * 0.5f, m_nlc_peek_radius})
    {
        for (int i = 0; i < 8; ++i)
        {
            const float angle = start + float(i) * PI_DIV_4;
            Fvector p = Fvector().set(Position().x + _cos(angle) * radius, Position().y, Position().z + _sin(angle) * radius);
            const u32 node = ai().level_graph().vertex_id(p);
            if (!ai().level_graph().valid_vertex_id(node))
                continue;
            p = ai().level_graph().vertex_position(node);
            if (!movement().restrictions().accessible(p) || !in_range(p) || !nlc_has_los(p, enemy))
                continue;
            return nlc_set_target(p, node, eNlcPhasePeek, "peek");
        }
    }

    return nlc_select_range();
}

// tube range on the controller's side of the enemy (line of sight not checked)
bool CController::nlc_select_range()
{
    const Fvector enemy_pos = EnemyMan.get_enemy_position();

    Fvector dir = Fvector().sub(Position(), enemy_pos);
    dir.y = 0.f;
    if (dir.square_magnitude() < EPS_L)
    {
        dir = Direction();
        dir.invert();
        dir.y = 0.f;
    }
    dir.normalize_safe();

    Fvector pos = Fvector().mad(enemy_pos, dir, ::Random.randF(m_nlc_range_min, m_nlc_range_max));
    pos.y = Position().y;
    if (pos.distance_to_xz(Position()) < 2.f)
        return false;

    return nlc_set_target(pos, ai().level_graph().vertex_id(pos), eNlcPhasePeek, "range");
}

// running to cover: melee only when the enemy is right on top of it (melee kept interrupting the run)
bool CController::nlc_keep_ranged()
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    return enemy && (m_nlc_phase == eNlcPhaseHide) && m_nlc_run_this_move && (Position().distance_to(enemy->Position()) > 2.f);
}

CController::ENlcRanged CController::nlc_ranged_execute(bool enemy_inaccessible)
{
    m_nlc_running = false;
    if (m_nlc_phase != eNlcPhaseAim)
        m_nlc_aim_los = false;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
    {
        nlc_reposition_stop("no enemy");
        return eNlcRangedStock;
    }

    const u32 now = time();
    const float dist = Position().distance_to(enemy->Position());

    if (!enemy_inaccessible)
    {
        if (!m_nlc_cover_loop)
        {
            nlc_reposition_stop("cover loop off");
            return eNlcRangedStock;
        }

        // the tube only ever targets the actor: against stalkers and monsters the loop would peek and aim
        // forever without attacking, so they get the stock fight (the psy pressure works on stalkers anyway)
        if (!smart_cast<const CActor*>(enemy))
        {
            nlc_reposition_stop("enemy is not the actor");
            return eNlcRangedStock;
        }

        // close range: stock melee, held for 3 s (or until 3 m further) so it does not flip back every update;
        // a controller already running to cover keeps running unless the enemy is right on top of it
        const bool fleeing = (m_nlc_phase == eNlcPhaseHide) && m_nlc_run_this_move;
        if (dist < (fleeing ? 2.f : m_nlc_close_dist))
            m_nlc_stock_until = now + 3000;
        if (now < m_nlc_stock_until && dist < m_nlc_close_dist + 3.f)
        {
            nlc_reposition_stop("enemy close");
            return eNlcRangedStock;
        }

        // stock hunt when the enemy was out of sight for long
        if (now > EnemyMan.get_enemy_time_last_seen() + m_nlc_lost_time)
        {
            nlc_reposition_stop("enemy lost");
            return eNlcRangedStock;
        }
    }
    else
    {
        if (!m_nlc_reposition && !m_nlc_cover_loop)
            return eNlcRangedStock;

        if (m_nlc_debug && m_nlc_logged_reason != nlc_inaccessible_reason())
        {
            m_nlc_logged_reason = nlc_inaccessible_reason();
            Msg("~ [controller] [%s]: enemy [%s] inaccessible: %s, %.1f m", cName().c_str(), enemy->cName().c_str(), nlc_inaccessible_name(m_nlc_logged_reason), dist);
        }
    }

    const bool hit_recently = HitMemory.is_hit() && (HitMemory.get_last_hit_time() + 3000 > now);
    bool reselect = false;

    switch (m_nlc_phase)
    {
    case eNlcPhaseOff: reselect = true; break;

    case eNlcPhaseHide:
    case eNlcPhasePeek: {
        // peeking: stop at the first spot with line of sight in tube range instead of walking on to the point
        if (m_nlc_phase == eNlcPhasePeek && tube_ready() && now >= m_nlc_next_los_check)
        {
            m_nlc_next_los_check = now + 250;
            if (dist >= m_nlc_range_min && dist <= m_nlc_range_max + 5.f && nlc_has_los(Position(), enemy))
            {
                if (m_nlc_debug)
                    Msg("~ [controller] [%s]: peek has line of sight on the way, %.1f m from enemy", cName().c_str(), dist);
                nlc_set_phase_hold(eNlcPhaseAim, m_nlc_aim_time);
                break;
            }
        }

        const bool arrived = (ai_location().level_vertex_id() == m_nlc_target_node) || (Position().distance_to_xz(m_nlc_target_pos) < 1.f);
        if (arrived || now >= m_nlc_phase_until)
        {
            if (m_nlc_debug)
                Msg("~ [controller] [%s]: %s %s", cName().c_str(), nlc_phase_name(m_nlc_phase), arrived ? "reached" : "timed out");

            if (m_nlc_phase == eNlcPhaseHide)
            {
                nlc_set_phase_hold(eNlcPhaseCamp, u32(::Random.randI(int(m_nlc_hold_min), int(m_nlc_hold_max) + 1)));
                nlc_try_decoy(enemy);
            }
            else
                nlc_set_phase_hold(eNlcPhaseAim, m_nlc_aim_time);
        }
        break;
    }

    case eNlcPhaseCamp:
        // flushed: the enemy walked into this cover
        if (nlc_try_flush(enemy, dist))
            break;
        // bad cover: still taking hits after settling in
        reselect = (!nlc_decoy_active() && now >= m_nlc_phase_until) || (hit_recently && HitMemory.get_last_hit_time() > m_nlc_phase_start + 1000);
        break;

    case eNlcPhaseAim:
        // the tube fired (recharging now), took too long, or the controller is being shot
        reselect = !tube_ready() || (now >= m_nlc_phase_until) || (hit_recently && HitMemory.get_last_hit_time() > m_nlc_phase_start);
        if (!reselect && now >= m_nlc_next_los_check)
        {
            m_nlc_next_los_check = now + 250;
            m_nlc_aim_los = smart_cast<const CActor*>(enemy) && (now < EnemyMan.get_enemy_time_last_seen() + 3000) && nlc_has_los(Position(), enemy);
        }
        break;

    case eNlcPhaseHold: reselect = (now >= m_nlc_phase_until); break;
    }

    if (reselect)
    {
        const bool want_peek = tube_ready() && !hit_recently;
        bool ok = want_peek ? nlc_select_peek() : nlc_select_cover();
        if (!ok)
            ok = want_peek ? nlc_select_cover() : nlc_select_range();
        if (!ok)
            nlc_set_phase_hold(eNlcPhaseHold, u32(::Random.randI(int(m_nlc_hold_min), int(m_nlc_hold_max) + 1)));
    }

    if (m_nlc_phase != eNlcPhaseHide && m_nlc_phase != eNlcPhasePeek)
        return eNlcRangedHold;

    // a move decides once whether it runs (nlc_set_target)
    m_nlc_running = m_nlc_run_this_move && !m_bDamaged;

    anim().accel_activate(eAT_Aggressive);
    anim().accel_set_braking(false);

    path().set_target_point(m_nlc_target_pos, m_nlc_target_node);
    path().set_rebuild_time(0);
    path().set_distance_to_end(0.f);
    path().set_use_covers(false);

    set_action(m_nlc_running ? ACT_RUN : ACT_WALK_FWD);
    set_state_sound(MonsterSound::eMonsterSoundAggressive);
    return eNlcRangedMove;
}

//////////////////////////////////////////////////////////////////////////
// Mirage decoy: a controller-shaped illusion steps out of cover while the real one stays hidden
//////////////////////////////////////////////////////////////////////////

void CController::Hit(SHit* pHDS)
{
    if (m_nlc_is_decoy)
    {
        if (g_Alive())
            nlc_decoy_vanish("hit");
        return;
    }

    inherited::Hit(pHDS);
}

void CController::nlc_decoy_self_update()
{
    const u32 now = time();
    if (!m_nlc_decoy_die_at)
        m_nlc_decoy_die_at = now + m_nlc_decoy_life;
    else if (now >= m_nlc_decoy_die_at)
        nlc_decoy_vanish("timeout");
}

void CController::nlc_decoy_vanish(LPCSTR why)
{
    if (m_nlc_decoy_vanishing)
        return;
    m_nlc_decoy_vanishing = true;

    // destroyed while alive: no corpse, no loot, no kill statistics
    Fvector pos = Position();
    pos.y += 1.f;
    CParticlesObject* fx = CParticlesObject::Create("monsters\\phantom_death", TRUE);
    fx->play_at_pos(pos);

    if (m_nlc_debug)
        Msg("~ [controller] [%s]: decoy vanished (%s)", cName().c_str(), why);

    DestroyObject();
}

void CController::nlc_try_decoy(const CEntityAlive* enemy)
{
    if (!m_nlc_decoy_section.size() || nlc_decoy_active() || !ai().get_alife() || !smart_cast<const CActor*>(enemy))
        return;

    const u32 now = time();
    if (now < m_nlc_decoy_next || ::Random.randF() >= m_nlc_decoy_chance)
        return;

    // step out beside the controller, toward the enemy's side
    Fvector to_enemy = Fvector().sub(enemy->Position(), Position());
    to_enemy.y = 0.f;
    to_enemy.normalize_safe();
    Fvector pos = Fvector().mad(Position(), Fvector().set(-to_enemy.z, 0.f, to_enemy.x), ::Random.randI(2) ? 1.2f : -1.2f);
    u32 node = ai().level_graph().vertex_id(pos);
    if (!ai().level_graph().valid_vertex_id(node))
    {
        pos = Position();
        node = ai_location().level_vertex_id();
    }

    // same as the script alife():create (alife_simulator_script.cpp)
    CALifeSimulator* sim = const_cast<CALifeSimulator*>(ai().get_alife());
    CSE_Abstract* se = sim->spawn_item(m_nlc_decoy_section.c_str(), pos, node, ai_location().game_vertex_id(), 0xffff);
    if (!se)
        return;
    if (CSE_ALifeObject* alife_object = smart_cast<CSE_ALifeObject*>(se))
        alife_object->can_switch_offline(false);

    m_nlc_decoy_id = se->ID;
    m_nlc_decoy_spawned_at = now;
    m_nlc_decoy_fed = false;
    m_nlc_decoy_next = now + m_nlc_decoy_cooldown;

    if (m_nlc_debug)
        Msg("~ [controller] [%s]: decoy [%s] sent out (id %u)", cName().c_str(), m_nlc_decoy_section.c_str(), u32(se->ID));
}

// gives the decoy this controller's enemy once it is online; forgets it once it is gone
void CController::nlc_decoy_update()
{
    if (!nlc_decoy_active())
        return;

    CController* decoy = smart_cast<CController*>(Level().Objects.net_Find(m_nlc_decoy_id));
    if (!decoy || decoy->getDestroy() || !decoy->g_Alive())
    {
        // not online yet right after the spawn; gone once it was online or after 5 s
        if (m_nlc_decoy_fed || decoy || time() > m_nlc_decoy_spawned_at + 5000)
        {
            if (m_nlc_debug)
                Msg("~ [controller] [%s]: decoy gone", cName().c_str());
            m_nlc_decoy_id = u16(-1);
        }
        return;
    }

    if (!m_nlc_decoy_fed && EnemyMan.get_enemy())
    {
        decoy->EnemyMemory.add_enemy(EnemyMan.get_enemy(), EnemyMan.get_enemy_position(), EnemyMan.get_enemy_vertex(), EnemyMan.get_enemy_time_last_seen());
        m_nlc_decoy_fed = true;
    }
}

//////////////////////////////////////////////////////////////////////////
// Flushed reaction, thrall eyes, script access
//////////////////////////////////////////////////////////////////////////

// short psy jolt on the actor: psy hit plus a lighter version of the control effectors
void CController::nlc_psy_jolt(const CEntityAlive* target, float power, float amplitude_k)
{
    CActor* actor = smart_cast<CActor*>(const_cast<CEntityAlive*>(target));
    if (!actor || !actor->g_Alive() || power <= 0.f)
        return;

    Hit_Psy(actor, power);
    actor->Cameras().AddCamEffector(xr_new<CMonsterEffectorHit>(m_control_effector.ce_time, m_control_effector.ce_amplitude * amplitude_k,
                                                               m_control_effector.ce_period_number, m_control_effector.ce_power));
    actor->Cameras().AddPPEffector(xr_new<CMonsterEffector>(m_control_effector.ppi, m_control_effector.time, m_control_effector.time_attack, m_control_effector.time_release));

    Fvector pos = actor->Position();
    pos.y += 1.5f;
    if (control_hit_sound._feedback())
        control_hit_sound.stop();
    control_hit_sound.play_at_pos(actor, pos);
}

// the enemy walked into the cover: a close-range psy jolt, then a short reposition nearby instead of
// fleeing across the map (stock melee still takes over below nlc_ctrl_close_dist)
bool CController::nlc_try_flush(const CEntityAlive* enemy, float dist)
{
    const u32 now = time();
    if (m_nlc_flush_dist <= 0.f || dist > m_nlc_flush_dist || now < m_nlc_flush_next || now < m_nlc_phase_start + 500 || !EnemyMan.see_enemy_now())
        return false;

    m_nlc_flush_next = now + m_nlc_flush_cooldown;
    nlc_psy_jolt(enemy, m_nlc_flush_psy, 0.6f);

    LPCSTR kind = "hold";
    const CCoverPoint* point = CoverMan->find_cover(enemy->Position(), std::max(m_nlc_cover_min, dist + 4.f), dist + 14.f);
    if (point && point->position().distance_to_xz(Position()) >= 2.f && nlc_set_target(point->position(), point->level_vertex_id(), eNlcPhaseHide, "flushed cover"))
        kind = "near cover";
    else if (nlc_select_peek())
        kind = "peek";
    else
        nlc_set_phase_hold(eNlcPhaseHold, m_nlc_hold_min);

    if (m_nlc_debug)
        Msg("~ [controller] [%s]: flushed at %.1f m: psy jolt, then %s", cName().c_str(), dist, kind);
    return true;
}

void CController::nlc_update_thrall_eyes()
{
    if (!m_nlc_thrall_eyes)
        return;

    const u32 now = time();
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    CActor* actor = smart_cast<CActor*>(const_cast<CEntityAlive*>(enemy));

    // a wind-up in progress: strike when it is due, if the thrall still sees the actor
    if (m_nlc_eyes_thrall != u16(-1))
    {
        if (now < m_nlc_eyes_strike)
            return;

        CCustomMonster* monster = nullptr;
        for (CEntity* thrall : m_controlled_objects)
            if (thrall->ID() == m_nlc_eyes_thrall)
                monster = smart_cast<CCustomMonster*>(thrall);
        m_nlc_eyes_thrall = u16(-1);
        if (m_sound_tube_prepare._feedback() && !m_psy_hit->is_active())
            m_sound_tube_prepare.stop();

        if (!monster || !monster->g_Alive() || !actor || !actor->g_Alive() || m_psy_hit->is_active() ||
            !monster->memory().visual().visible_now(smart_cast<const CGameObject*>(enemy)))
        {
            if (m_nlc_debug)
                Msg("~ [controller] [%s]: thrall eyes wind-up lost its target", cName().c_str());
            return;
        }

        m_sound_tube_hit_left.play_at_pos(actor, Fvector().set(-1.f, 0.f, 1.f), sm_2D);
        m_sound_tube_hit_right.play_at_pos(actor, Fvector().set(1.f, 0.f, 1.f), sm_2D);
        nlc_psy_jolt(enemy, m_nlc_eyes_power, 0.5f);
        if (m_nlc_debug)
            Msg("~ [controller] [%s]: thrall eyes strike through [%s], %.1f m", cName().c_str(), monster->cName().c_str(), monster->Position().distance_to(enemy->Position()));
        return;
    }

    if (m_controlled_objects.empty() || now < m_nlc_eyes_next || !actor || EnemyMan.see_enemy_now() || m_psy_hit->is_active())
        return;

    for (CEntity* thrall : m_controlled_objects)
    {
        CCustomMonster* monster = smart_cast<CCustomMonster*>(thrall);
        if (!monster || !monster->g_Alive() || monster->Position().distance_to(enemy->Position()) > m_nlc_eyes_range)
            continue;
        if (!monster->memory().visual().visible_now(smart_cast<const CGameObject*>(enemy)))
            continue;

        m_nlc_eyes_next = now + m_nlc_eyes_cooldown;
        m_nlc_eyes_thrall = thrall->ID();
        m_nlc_eyes_strike = now + m_nlc_eyes_windup;

        // the tube's own particles, from the thrall's head toward the actor
        Fvector head = get_head_position(monster);
        Fvector dir = Fvector().sub(get_head_position(actor), head);
        dir.normalize_safe();
        PlayParticles(particles_fire, head, dir);
        if (m_sound_tube_prepare._feedback())
            m_sound_tube_prepare.stop();
        m_sound_tube_prepare.play_at_pos(actor, Fvector().set(0.f, 0.f, 0.f), sm_2D);

        if (m_nlc_debug)
            Msg("~ [controller] [%s]: thrall eyes through [%s], %.1f m (wind-up %u ms)", cName().c_str(), thrall->cName().c_str(),
                monster->Position().distance_to(enemy->Position()), m_nlc_eyes_windup);
        return;
    }
}

LPCSTR CController::nlc_script_phase() const { return nlc_phase_name(m_nlc_phase); }

// herd call (bind_monster): every thrall attacks the controller's enemy, guards and flankers included
u32 CController::nlc_herd_call(u32 duration_ms)
{
    if (!g_Alive() || m_nlc_is_decoy || !m_nlc_thrall_attack || m_controlled_objects.empty() || !EnemyMan.get_enemy())
        return 0;
    m_nlc_herd_until = time() + duration_ms;
    if (m_nlc_debug)
        Msg("~ [controller] [%s]: herd call, %u thralls for %u ms", cName().c_str(), u32(m_controlled_objects.size()), duration_ms);
    return u32(m_controlled_objects.size());
}

void CController::nlc_thrall_ids(xr_string& out) const
{
    out.clear();
    for (const CEntity* thrall : m_controlled_objects)
    {
        if (!out.empty())
            out += ",";
        out += std::to_string(thrall->ID()).c_str();
    }
}

bool CController::nlc_script_take(CEntityAlive* thrall)
{
    if (!thrall || !thrall->g_Alive() || !g_Alive() || m_nlc_is_decoy)
        return false;
    CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(thrall);
    if (!entity || entity->is_under_control() || !nlc_can_take(thrall))
        return false;
    nlc_take(thrall, "restored after load", false); // every thrall staggering at once on each load looked broken
    return true;
}

void CController::nlc_reposition_stop(LPCSTR why)
{
    m_nlc_running = false;
    m_nlc_run_this_move = false;
    m_nlc_aim_los = false;

    if (m_nlc_phase == eNlcPhaseOff)
        return;

    if (m_nlc_debug)
        Msg("~ [controller] [%s]: ranged mode ended (%s)", cName().c_str(), why);

    m_nlc_phase = eNlcPhaseOff;
    m_nlc_logged_reason = eNlcAccessible;
}

/*
#ifdef DEBUG
CBaseMonster::SDebugInfo CController::show_debug_info()
{
    CBaseMonster::SDebugInfo info = inherited::show_debug_info();
    if (!info.active) return CBaseMonster::SDebugInfo();


    // Draw Controlled Lines
    DBG().level_info(this).clear();

    Fvector my_pos = Position();
    my_pos.y += 1.5f;


    for (u32 i=0; i < m_controlled_objects.size(); i++)
    {
        Fvector enemy_pos = m_controlled_objects[i]->Position();

        Fvector dir;
        dir.sub(enemy_pos, Position());
        dir.div(2.f);
        Fvector new_pos;
        new_pos.add(Position(),dir);
        new_pos.y += 10.f;

        enemy_pos.y += 1.0f;

        DBG().level_info(this).add_item(my_pos,	new_pos, D3DCOLOR_XRGB(0,255,255));
        DBG().level_info(this).add_item(enemy_pos, new_pos, D3DCOLOR_XRGB(0,255,255));
    }

    return CBaseMonster::SDebugInfo();
}
#endif

#ifdef DEBUG
void CController::debug_on_key(int key)
{
    switch (key){
    case DIK_MINUS:
        //m_sound_aura_left_channel.play_at_pos(Level().CurrentEntity(), Fvector().set(-1.f, 0.f, 1.f), sm_2D);
        //m_sound_aura_right_channel.play_at_pos(Level().CurrentEntity(), Fvector().set(1.f, 0.f, 1.f), sm_2D);

        if (m_psy_hit->check_start_conditions()) {
            control().activate(ControlCom::eComCustom1);
        }
        //P1.set		(Actor()->Position());
        //
        //DBG().level_info(this).remove_item	(u32(0));
        //DBG().level_info(this).add_item(P1,0.5f,COLOR_BLUE,0);


        //if (!fsimilar(P1.square_magnitude(),0.f) &&
        //	!fsimilar(P2.square_magnitude(),0.f)) {
        //	const CCoverPoint *cover = CoverMan->find_cover(P1,P2,10.f,40.f);
        //	if (cover) {
        //		DBG().level_info(this).remove_item	(3);
        //		DBG().level_info(this).add_item		(cover->position(),0.8f,COLOR_RED,3);
        //	}
        //}


        break;
    case DIK_EQUALS:
        P2.set		(Actor()->Position());
        DBG().level_info(this).remove_item	(1);
        DBG().level_info(this).add_item(P2,0.5f,COLOR_GREEN,1);

        if (!fsimilar(P1.square_magnitude(),0.f) &&
            !fsimilar(P2.square_magnitude(),0.f)) {
            const CCoverPoint *cover = CoverMan->find_cover(P1,P2,10.f,40.f);
            if (cover) {
                DBG().level_info(this).remove_item	(3);
                DBG().level_info(this).add_item		(cover->position(),0.8f,COLOR_RED,3);
            }
        }

        //m_sound_aura_hit_left_channel.play_at_pos(Level().CurrentEntity(), Fvector().set(-1.f, 0.f, 1.f), sm_2D);
        //m_sound_aura_hit_right_channel.play_at_pos(Level().CurrentEntity(), Fvector().set(1.f, 0.f, 1.f), sm_2D);
        break;
    }
}
#endif
*/
