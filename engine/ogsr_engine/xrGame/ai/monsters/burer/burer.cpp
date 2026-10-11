#include "stdafx.h"
#include "script_engine.h"
#include "../../../ai_space.h"
#include "burer.h"
#include "../../../PhysicsShell.h"
#include "../../../characterphysicssupport.h"
#include "../../../actor.h"
#include "burer_state_manager.h"
#include "../../../../Include/xrRender/KinematicsAnimated.h"
#include "../../../sound_player.h"
#include "../../../level.h"
#include "../../../ai_monster_space.h"
#include "../../../level_debug.h"
#include "../monster_velocity_space.h"
#include "../../../GamePersistent.h"
#include "../control_animation_base.h"
#include "../control_movement_base.h"
#include "burer_fast_gravi.h"
#include "../anti_aim_ability.h"
#include "../anti_aim_ability.h"

#include "../../../Inventory.h"
#include "../../../ActorCondition.h"
#include "../../../xr_level_controller.h"
#include "../../../weapon.h"

#include "../control_direction_base.h"

#include "../../../grenade.h"
#include "../../../ExplosiveRocket.h"
#include "../../../WeaponMagazined.h"
#include "../../../inventory_item.h"
#include "../../../nlc_stealth.h"
#include "../../../memory_manager.h"
#include "../../../visual_memory_manager.h"
#include "../../../PHMovementControl.h"
#include "../../../ActorEffector.h"
#include "../ai_monster_utils.h"
#include "../ai_monster_effector.h"
#include "../../../PhysicObject.h"
#include "../../../../xr_3da/IGame_Persistent.h"
#include "../../../../Include/xrRender/Kinematics.h"

bool CBurer::can_scan = true;
xr_vector<CBurer*> CBurer::s_nlc_burers;

CBurer::CBurer()
{
    StateMan = xr_new<CStateManagerBurer>(this);

    m_fast_gravi = xr_new<CBurerFastGravi>();

    control().add(m_fast_gravi, ControlCom::eComCustom1);
}

CBurer::~CBurer()
{
    xr_delete(StateMan);
    xr_delete(m_fast_gravi);
}

void CBurer::reinit()
{
    inherited::reinit();

    DeactivateShield();

    time_last_scan = 0;

    nlc_reinit_pass2();
    nlc_reinit_pass5();
    if (std::find(s_nlc_burers.begin(), s_nlc_burers.end(), this) == s_nlc_burers.end())
        s_nlc_burers.push_back(this);
    m_nlc_pick_retry = 0;
    m_nlc_rocket_scan_next = 0;
    m_nlc_bar_ext_next = 0;
    m_nlc_repulse_next = 0;
    m_nlc_scan_next = 0;
    m_nlc_scan_prev_level = 0.f;
    m_nlc_gren_scan_next = 0;
}

void CBurer::net_Destroy()
{
    s_nlc_burers.erase(std::remove(s_nlc_burers.begin(), s_nlc_burers.end(), this), s_nlc_burers.end());
    // the gaze effect is switched off only by the engine: an offline switch / release mid-gaze left it on
    if (m_nlc_gaze_sent > 0.f && g_pGameLevel && !Level().is_removing_objects())
        nlc_gaze_send(0.f, false);
    m_nlc_gaze_sent = 0.f;
    nlc_grenade_release_all();
    nlc_puppet_release("burer destroyed", false);
    if (m_nlc_gun_tele.is_active())
        m_nlc_gun_tele.deactivate();
    nlc_barricade_drop("burer destroyed");
    if (m_nlc_bar_tele.is_active())
        m_nlc_bar_tele.deactivate();
    m_nlc_wr.active = false;
    nlc_catch_drop("burer destroyed");
    if (m_nlc_scan_playing)
    {
        can_scan = true;
        m_nlc_scan_playing = false;
    }
    inherited::net_Destroy();
}

void CBurer::reload(LPCSTR section)
{
    inherited::reload(section);

    if (!g_Alive())
        return;

    // add specific sounds
    sound().add_deferred(pSettings->r_string(section, "sound_gravi_attack"), DEFAULT_SAMPLE_COUNT,
        SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 2,
        u32(MonsterSound::eBaseChannel), eMonsterSoundGraviAttack, get_head_bone_name());

    sound().add_deferred(pSettings->r_string(section, "sound_tele_attack"), DEFAULT_SAMPLE_COUNT,
        SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 3,
        u32(MonsterSound::eBaseChannel), eMonsterSoundTeleAttack, get_head_bone_name());
}

void CBurer::ActivateShield() { m_shield_active = true; }

void CBurer::DeactivateShield() { m_shield_active = false; }

void CBurer::Load(LPCSTR section)
{
    inherited::Load(section);

    // anim().AddReplacedAnim		(&m_bDamaged, eAnimStandIdle,	eAnimStandDamaged);
    // anim().AddReplacedAnim		(&m_bDamaged, eAnimRun,			eAnimRunDamaged);
    // anim().AddReplacedAnim		(&m_bDamaged, eAnimWalkFwd,		eAnimWalkDamaged);

    anim().accel_load(section);
    anim().accel_chain_add(eAnimWalkFwd, eAnimRun);

    anim().AddReplacedAnim(&m_bRunTurnLeft, eAnimRun, eAnimRunTurnLeft);
    anim().AddReplacedAnim(&m_bRunTurnRight, eAnimRun, eAnimRunTurnRight);

    particle_gravi_wave = pSettings->r_string(section, "Particle_Gravi_Wave");
    particle_gravi_prepare = pSettings->r_string(section, "Particle_Gravi_Prepare");
    particle_tele_object = pSettings->r_string(section, "Particle_Tele_Object");

    ::Sound->create(sound_gravi_wave, pSettings->r_string(section, "sound_gravi_wave"), st_Effect, SOUND_TYPE_WORLD);
    ::Sound->create(sound_tele_hold, pSettings->r_string(section, "sound_tele_hold"), st_Effect, SOUND_TYPE_WORLD);
    ::Sound->create(sound_tele_throw, pSettings->r_string(section, "sound_tele_throw"), st_Effect, SOUND_TYPE_WORLD);

    m_gravi.cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "Gravi_Cooldown", 5000);
    m_gravi.min_dist = READ_IF_EXISTS(pSettings, r_float, section, "Gravi_MinDist", 1.f);
    m_gravi.max_dist = READ_IF_EXISTS(pSettings, r_float, section, "Gravi_MaxDist", 15.f);
    m_gravi.speed = pSettings->r_float(section, "Gravi_Speed");
    m_gravi.step = pSettings->r_float(section, "Gravi_Step");
    m_gravi.time_to_hold = pSettings->r_u32(section, "Gravi_Time_To_Hold");
    m_gravi.radius = pSettings->r_float(section, "Gravi_Radius");
    m_gravi.impulse_to_objects = pSettings->r_float(section, "Gravi_Impulse_To_Objects");
    m_gravi.impulse_to_enemy = pSettings->r_float(section, "Gravi_Impulse_To_Enemy");
    m_gravi.hit_power = pSettings->r_float(section, "Gravi_Hit_Power");

    m_weight_to_stamina_hit = READ_IF_EXISTS(pSettings, r_float, section, "weight_to_stamina_hit", 0.02f);
    m_weapon_drop_stamina_k = READ_IF_EXISTS(pSettings, r_float, section, "weapon_drop_stamina_k", 3.f);

    m_runaway_distance = READ_IF_EXISTS(pSettings, r_float, section, "runaway_distance", 6.f);
    m_normal_distance = READ_IF_EXISTS(pSettings, r_float, section, "normal_distance", 12.f);
    m_max_runaway_time = READ_IF_EXISTS(pSettings, r_u32, section, "max_runaway_time", 5000);

    m_weapon_drop_velocity = READ_IF_EXISTS(pSettings, r_float, section, "weapon_drop_velocity", 8);

    m_shield_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "shield_cooldown", 4000);
    m_shield_time = READ_IF_EXISTS(pSettings, r_u32, section, "shield_time", 3000);
    m_shield_keep_particle = READ_IF_EXISTS(pSettings, r_string, section, "shield_keep_particle", 0);
    m_shield_keep_particle_period = READ_IF_EXISTS(pSettings, r_u32, section, "shield_keep_particle_period", 1000);

    m_tele_max_handled_objects = pSettings->r_u32(section, "Tele_Max_Handled_Objects");
    m_tele_max_time = READ_IF_EXISTS(pSettings, r_u32, section, "Tele_Max_Time", 10000);
    m_tele_time_to_hold = pSettings->r_u32(section, "Tele_Time_To_Hold");
    m_tele_object_min_mass = pSettings->r_float(section, "Tele_Object_Min_Mass");
    m_tele_object_max_mass = pSettings->r_float(section, "Tele_Object_Max_Mass");
    m_tele_find_radius = pSettings->r_float(section, "Tele_Find_Radius");
    m_tele_min_distance = READ_IF_EXISTS(pSettings, r_float, section, "tele_min_distance", 8);
    m_tele_max_distance = READ_IF_EXISTS(pSettings, r_float, section, "tele_max_distance", 30);
    m_tele_raise_speed = READ_IF_EXISTS(pSettings, r_float, section, "tele_raise_speed", 5.f);
    m_tele_fly_velocity = READ_IF_EXISTS(pSettings, r_float, section, "tele_fly_velocity", 30.f);
    m_tele_object_height = READ_IF_EXISTS(pSettings, r_float, section, "tele_object_height", 2.f);

    particle_fire_shield = pSettings->r_string(section, "Particle_Shield");

    SVelocityParam& velocity_none = move().get_velocity(MonsterMovement::eVelocityParameterIdle);
    SVelocityParam& velocity_turn = move().get_velocity(MonsterMovement::eVelocityParameterStand);
    SVelocityParam& velocity_walk = move().get_velocity(MonsterMovement::eVelocityParameterWalkNormal);
    SVelocityParam& velocity_run = move().get_velocity(MonsterMovement::eVelocityParameterRunNormal);
    // SVelocityParam &velocity_walk_dmg	= 	move().get_velocity(MonsterMovement::eVelocityParameterWalkDamaged);
    // SVelocityParam &velocity_run_dmg	= 	move().get_velocity(MonsterMovement::eVelocityParameterRunDamaged);
    // SVelocityParam &velocity_steal		= 	move().get_velocity(MonsterMovement::eVelocityParameterSteal);
    //		SVelocityParam &velocity_drag		= move().get_velocity(MonsterMovement::eVelocityParameterDrag);

    anim().AddAnim(eAnimStandIdle, "stand_idle_", -1, &velocity_none, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimStandTurnLeft, "stand_turn_ls_", -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimStandTurnRight, "stand_turn_rs_", -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    //	anim().AddAnim(eAnimStandDamaged,	"stand_idle_dmg_",		-1, &velocity_none,		PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    anim().AddAnim(eAnimWalkFwd, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // anim().AddAnim(eAnimWalkDamaged,	"stand_walk_fwd_dmg_",	-1, &velocity_walk_dmg,	PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimRun, "stand_run_fwd_", -1, &velocity_run, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // anim().AddAnim(eAnimRunDamaged,		"stand_run_dmg_",		-1,	&velocity_run_dmg,	PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    anim().AddAnim(eAnimAttack, "stand_attack_", -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    anim().AddAnim(eAnimDie, "stand_die_", -1, &velocity_none, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    pcstr shield_anim_start = READ_IF_EXISTS(pSettings, r_string, section, "shield_animation_start", "stand_tele_" /*"stand_shield_"*/);
    anim().AddAnim(eAnimShieldStart, shield_anim_start, -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    pcstr shield_anim_cont = READ_IF_EXISTS(pSettings, r_string, section, "shield_animation_cont", "stand_tele_" /*"stand_shield_idle_"*/);
    anim().AddAnim(eAnimShieldContinue, shield_anim_cont, -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    pcstr tele_anim_fire = READ_IF_EXISTS(pSettings, r_string, section, "tele_animation_fire", "stand_tele_" /*"stand_power_attack_"*/);
    anim().AddAnim(eAnimTeleFire, tele_anim_fire, -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    pcstr tele_anim_tele = READ_IF_EXISTS(pSettings, r_string, section, "tele_animation_tele", "stand_tele_" /*"telekinesis_"*/);
    anim().AddAnim(eAnimTelekinesis, tele_anim_tele, -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    pcstr gravi_anim_fire = READ_IF_EXISTS(pSettings, r_string, section, "gravi_animation_fire", "stand_gravi_" /*"stand_power_attack_"*/);
    anim().AddAnim(eAnimGraviFire, gravi_anim_fire, -1, &velocity_turn, PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    pcstr run_anim_tl = READ_IF_EXISTS(pSettings, r_string, section, "run_animation_tl", "stand_run_fwd_" /*"stand_run_fwd_turn_left_"*/);
    anim().AddAnim(eAnimRunTurnLeft, run_anim_tl, -1, &velocity_run, PS_STAND);
    pcstr run_anim_tr = READ_IF_EXISTS(pSettings, r_string, section, "run_animation_tr", "stand_run_fwd_" /*"stand_run_fwd_turn_right_"*/);
    anim().AddAnim(eAnimRunTurnRight, run_anim_tr, -1, &velocity_run, PS_STAND);

    // 	anim().AddAnim(eAnimScared,			"stand_scared_",		-1, &velocity_none,		PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // 	anim().AddAnim(eAnimSteal,			"stand_steal_",			-1, &velocity_steal,	PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // 	anim().AddAnim(eAnimEat,			"sit_eat_",				-1, &velocity_none,		PS_SIT); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    //
    // 	anim().AddAnim(eAnimSitIdle,		"sit_idle_",			-1, &velocity_none,		PS_SIT); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // 	anim().AddAnim(eAnimCheckCorpse,	"sit_check_corpse_",	-1, &velocity_none,		PS_SIT); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // 	anim().AddAnim(eAnimSitStandUp,		"sit_stand_up_",		-1, &velocity_none,		PS_SIT); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    // 	anim().AddAnim(eAnimStandSitDown,	"stand_sit_down_",		-1, &velocity_none,		PS_STAND); //, 	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    //	anim().AddTransition(PS_SIT,		PS_STAND,		eAnimSitStandUp,	false);
    //	anim().AddTransition(PS_STAND,		PS_SIT,			eAnimStandSitDown,	false);

    anim().LinkAction(ACT_STAND_IDLE, eAnimStandIdle);
    //	anim().LinkAction		(ACT_SIT_IDLE,		eAnimSitIdle);
    //	anim().LinkAction		(ACT_LIE_IDLE,		eAnimSitIdle);
    anim().LinkAction(ACT_WALK_FWD, eAnimWalkFwd);
    anim().LinkAction(ACT_WALK_BKWD, eAnimWalkFwd);
    anim().LinkAction(ACT_RUN, eAnimRun);
    // anim().LinkAction(ACT_EAT,			eAnimEat);
    anim().LinkAction(ACT_SLEEP, eAnimStandIdle); // eAnimSitIdle);
    anim().LinkAction(ACT_REST, eAnimStandIdle); // eAnimSitIdle);
    anim().LinkAction(ACT_DRAG, eAnimWalkFwd);
    anim().LinkAction(ACT_ATTACK, eAnimAttack);
    // anim().LinkAction(ACT_STEAL,		eAnimSteal);
    // anim().LinkAction(ACT_LOOK_AROUND,	eAnimScared);

#ifdef DEBUG
    anim().accel_chain_test();
#endif

    m_force_gravi_attack = false;

    nlc_load(section);

    PostLoad(section);
}

void CBurer::PostLoad(LPCSTR section)
{
    inherited::PostLoad(section);
    if (m_anti_aim)
    {
        m_anti_aim->set_callback(fastdelegate::MakeDelegate(this, &CBurer::StaminaHit));
    }
}

void CBurer::shedule_Update(u32 dt)
{
    inherited::shedule_Update(dt);

    CTelekinesis::schedule_update();
    m_nlc_gren_tele.schedule_update();
    m_nlc_gun_tele.schedule_update();
    m_nlc_bar_tele.schedule_update();

    nlc_scan_update();
}

void CBurer::CheckSpecParams(u32 spec_params) {}

void CBurer::StaminaHit()
{
    if (GodMode())
    {
        m_nlc_gaze_seize = false;
        nlc_log("anti-aim hit skipped: god mode");
        return;
    }

    CWeapon* const active_weapon = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());
    if (!active_weapon)
    {
        return;
    }

    luabind::functor<bool> func_check;
    if (ai().script_engine().functor("bind_monster.handle_glass_new_on_belt", func_check) && func_check())
    {
        nlc_log("anti-aim blocked by af_glass_new");
        return;
    }

    // NLC: the gaze's seize (docs/BURER_REDESIGN.md 5.13) hits stamina first, so a long stare drops the gun
    if (m_nlc_gaze_seize)
    {
        m_nlc_gaze_seize = false;
        m_nlc_gaze_force_until = 0;
        Actor()->conditions().PowerHit(m_nlc_gaze_seize_stamina, false);
        luabind::functor<void> func_seize;
        if (ai().script_engine().functor("bind_monster.burer_on_gaze_seize", func_seize))
            func_seize(ID());
    }

    float const weight = active_weapon->Weight();
    float const stamina_hit = weight * m_weight_to_stamina_hit;

    const float power_before = Actor()->conditions().GetPower();
    bool do_weapon_drop = power_before < stamina_hit * m_weapon_drop_stamina_k;

    Actor()->conditions().PowerHit(stamina_hit, false);

    // NLC: one disarm grace period shared by every burer and the script stare (docs/BURER_REDESIGN.md 5.2)
    luabind::functor<bool> func_disarm_allowed;
    if (do_weapon_drop && ai().script_engine().functor("bind_monster.burer_disarm_allowed", func_disarm_allowed) && !func_disarm_allowed())
    {
        do_weapon_drop = false;
        nlc_log("anti-aim drop blocked by the disarm grace");
    }
    nlc_log("anti-aim stamina hit weapon=%s weight=%.1f power=%.2f hit=%.2f drop=%d", active_weapon->cNameSect().c_str(), weight, power_before, stamina_hit, do_weapon_drop ? 1 : 0);

    if (do_weapon_drop)
    {
        Fvector dir = Actor()->Direction();
        if (dir.y < 0.f)
        {
            dir.y = -dir.y;
        }
        auto item = smart_cast<CPhysicsShellHolder*>(Actor()->inventory().ActiveItem());
        item->SetActivationSpeedOverride(normalize(dir) * m_weapon_drop_velocity);

        if (!Actor()->inventory().Action((u16)kDROP, CMD_STOP))
        {
            Actor()->g_PerformDrop();
        }

        luabind::functor<void> func_on_disarm;
        if (ai().script_engine().functor("bind_monster.burer_on_disarm", func_on_disarm))
            func_on_disarm(m_nlc_disarm_grace, "anti_aim");
    }
}

void CBurer::UpdateGraviObject()
{
    if (!m_gravi_object.active)
    {
        return;
    }

    if (!m_gravi_object.enemy || (m_gravi_object.enemy && m_gravi_object.enemy->getDestroy()))
    {
        m_gravi_object.deactivate();
        return;
    }

    if (m_gravi_object.from_pos.distance_to(m_gravi_object.cur_pos) > m_gravi_object.from_pos.distance_to(m_gravi_object.target_pos))
    {
        m_gravi_object.deactivate();
        return;
    }

    float dt = float(Device.dwTimeGlobal - m_gravi_object.time_last_update);
    float dist = dt * float(m_gravi.speed) / 1000.f;

    if (dist < m_gravi.step)
        return;

    Fvector new_pos;
    Fvector dir;
    dir.sub(m_gravi_object.target_pos, m_gravi_object.cur_pos);
    dir.normalize();

    new_pos.mad(m_gravi_object.cur_pos, dir, dist);

    // Trace to enemy
    Fvector enemy_center;
    m_gravi_object.enemy->Center(enemy_center);
    dir.sub(enemy_center, new_pos);
    dir.normalize();

    float trace_dist = float(m_gravi.step);

    collide::rq_result l_rq;
    if (Level().ObjectSpace.RayPick(new_pos, dir, trace_dist, collide::rqtBoth, l_rq, NULL))
    {
        const CObject* enemy = smart_cast<const CObject*>(m_gravi_object.enemy);
        if ((l_rq.O == enemy) && (l_rq.range < trace_dist))
        {
            // check for visibility
            bool b_enemy_visible = false;
            xr_vector<CObject*> visible_objects;
            feel_vision_get(visible_objects);

            // find object
            for (u32 i = 0; i < visible_objects.size(); i++)
            {
                if (visible_objects[i] == enemy)
                {
                    b_enemy_visible = true;
                    break;
                }
            }

            if (b_enemy_visible)
            {
                Fvector impulse_dir;

                impulse_dir.set(0.0f, 0.0f, 1.0f);
                impulse_dir.normalize();

                HitEntity(m_gravi_object.enemy, m_gravi.hit_power, m_gravi.impulse_to_enemy, impulse_dir, ALife::eHitTypeStrike, false);
                // NLC: a strike hit never moves a character (ApplyHit: explosion / wound only); the push comes the
                // frame after the hit (the hit is processed first), horizontally away from the burer
                if (m_nlc_gravi_push_speed > 0.f)
                {
                    Fvector away;
                    away.sub(m_gravi_object.enemy->Position(), Position());
                    away.y = 0.f;
                    if (away.square_magnitude() < EPS_L)
                        away = Direction();
                    away.normalize_safe();
                    m_nlc_gravi_push_id = m_gravi_object.enemy->ID();
                    m_nlc_gravi_push_frame = Device.dwFrame;
                    m_nlc_gravi_push_dir = away;
                }
                nlc_log("gravi hit %s power=%.2f travelled=%.1f", m_gravi_object.enemy->cName().c_str(), m_gravi.hit_power, m_gravi_object.from_pos.distance_to(new_pos));
                m_gravi_object.deactivate();
                return;
            }
        }
    }

    m_gravi_object.cur_pos = new_pos;
    m_gravi_object.time_last_update = Device.dwTimeGlobal;

    // ---------------------------------------------------------------------
    // draw particle
    CParticlesObject* ps = CParticlesObject::Create(particle_gravi_wave, TRUE);

    Fmatrix pos;
    pos.identity();
    pos.k.set(dir);
    Fvector::generate_orthonormal_basis_normalized(pos.k, pos.j, pos.i);
    pos.translate_over(m_gravi_object.cur_pos);

    ps->UpdateParent(pos, {});
    ps->Play(false);

    // hit objects
    m_nearest.clear();
    Level().ObjectSpace.GetNearest(m_nearest, m_gravi_object.cur_pos, m_gravi.radius, NULL);
    // xr_vector<CObject*> &m_nearest = Level().ObjectSpace.q_nearest;

    for (u32 i = 0; i < m_nearest.size(); i++)
    {
        CPhysicsShellHolder* obj = smart_cast<CPhysicsShellHolder*>(m_nearest[i]);
        if (
		  !obj ||
		  !obj->m_pPhysicsShell ||
		  smart_cast<CGrenade*>(obj) || // NLC: the 500 x mass shove launched thrown grenades out of sight
		  nlc_own_held(obj) || // NLC: its own wave passes through its barricade / puppet gun
		  (obj->spawn_ini() && obj->spawn_ini()->section_exist( "ph_heavy" )) || 
          (pSettings->line_exist( obj->cNameSect().c_str(), "ph_heavy" ) && pSettings->r_bool( obj->cNameSect().c_str(), "ph_heavy" )) ||
		  (pSettings->line_exist( obj->cNameSect().c_str(), "quest_item" ) && pSettings->r_bool( obj->cNameSect().c_str(), "quest_item" )) ||
		  obj->hasFixedBones() ||
		  !obj->m_pPhysicsShell->get_ApplyByGravity()
		) continue;

        Fvector dir;
        dir.sub(obj->Position(), m_gravi_object.cur_pos);
        dir.normalize();
        // NLC: a disarmed gun was flung out of sight by 500 x mass; inventory items get their own impulse
        const float impulse_k = smart_cast<CInventoryItem*>(obj) ? m_nlc_gravi_item_impulse : m_gravi.impulse_to_objects;
        obj->m_pPhysicsShell->applyImpulse(dir, impulse_k * obj->m_pPhysicsShell->getMass());
    }

    Fvector snd_pos = m_gravi_object.cur_pos;
    snd_pos.y += 0.5f;
    if (sound_gravi_wave._feedback())
    {
        sound_gravi_wave.set_position(snd_pos);
    }
    else
        ::Sound->play_at_pos(sound_gravi_wave, 0, snd_pos);
}

void CBurer::UpdateCL()
{
    inherited::UpdateCL();

    UpdateGraviObject();

    if (g_Alive())
    {
        nlc_grenade_update();
        nlc_defense_update();
        nlc_puppet_update();
        nlc_barricade_update();
        nlc_pass5_update();
    }
    nlc_push_update();
    // if (m_fast_gravi->check_start_conditions())
    //	control().activate(ControlCom::eComCustom1);
}

void CBurer::StartGraviPrepare()
{
    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy)
        return;

    CActor* pA = const_cast<CActor*>(smart_cast<const CActor*>(enemy));
    if (!pA)
        return;

    pA->CParticlesPlayer::StartParticles(particle_gravi_prepare, Fvector().set(0.0f, 0.1f, 0.0f), pA->ID());
}
void CBurer::StopGraviPrepare()
{
    CActor* pA = Actor();
    if (!pA)
        return;
    pA->CParticlesPlayer::StopParticles(particle_gravi_prepare, BI_NONE, true);
}

void CBurer::StartTeleObjectParticle(CGameObject* pO)
{
    CParticlesPlayer* PP = smart_cast<CParticlesPlayer*>(pO);
    if (!PP)
        return;
    PP->StartParticles(particle_tele_object, Fvector().set(0.0f, 0.1f, 0.0f), pO->ID());
}
void CBurer::StopTeleObjectParticle(CGameObject* pO)
{
    CParticlesPlayer* PP = smart_cast<CParticlesPlayer*>(pO);
    if (!PP)
        return;
    PP->StopParticles(particle_tele_object, BI_NONE, true);
}

void CBurer::Hit(SHit* pHDS)
{
    // NLC: the passive field turns some bullets away while the burer is not busy
    if (!m_shield_active && nlc_try_deflect(pHDS))
    {
        last_hit_frame = Device.dwFrame;
        return;
    }

    if (m_shield_active && pHDS->hit_type == ALife::eHitTypeFireWound && Device.dwFrame != last_hit_frame)
    {
        Fmatrix pos;
        // CParticlesPlayer::MakeXFORM(this,element,Fvector().set(0.f,0.f,1.f),p_in_object_space,pos);
        CParticlesPlayer::MakeXFORM(this, pHDS->bone(), pHDS->dir, pHDS->p_in_bone_space, pos);

        CParticlesObject* ps = CParticlesObject::Create(particle_fire_shield, TRUE);

        ps->UpdateParent(pos, Fvector().set(0.f, 0.f, 0.f));
        GamePersistent().ps_needtoplay.push_back(ps);
    }
    else if (!m_shield_active)
    {
        inherited::Hit(pHDS);
    }

    // NLC: every blocked hit strains the shield (nlc_shield_strain_hp); a strained shield leaks part of the hit
    if (m_shield_active)
    {
        const float leak = nlc_on_shield_blocked(pHDS);
        if (leak > 0.f && m_shield_active)
        {
            SHit leaked = *pHDS;
            leaked.power *= leak;
            leaked.impulse *= leak;
            inherited::Hit(&leaked);
        }
        else if (m_shield_active && pHDS->hit_type == ALife::eHitTypeFireWound)
            nlc_catch_bullet(pHDS); // NLC: stopped in the air, returned when the shield drops
    }

    last_hit_frame = Device.dwFrame;
}

void CBurer::Die(CObject* who)
{
    inherited::Die(who);

    nlc_grenade_release_all();
    nlc_puppet_release("burer died", false);
    nlc_barricade_drop("burer died");
    nlc_overlay_shield_stop("burer died");
    m_nlc_wr.active = false;
    nlc_catch_drop("burer died");
    if (m_nlc_gaze_sent > 0.f)
        nlc_gaze_send(0.f, false);
    m_nlc_stare_until = 0;

    if (com_man().ta_is_active())
    {
        com_man().ta_deactivate();
    }

    CTelekinesis::Deactivate();
}

void CBurer::net_Relcase(CObject* O)
{
    inherited::net_Relcase(O);

    TTelekinesis::remove_links(O);

    nlc_grenade_forget(O);
    m_nlc_gren_tele.remove_links(O);
    if (m_nlc_puppet.gun == O)
        m_nlc_puppet = NlcPuppet();
    m_nlc_gun_tele.remove_links(O);
    if (m_nlc_bar_obj == O)
        m_nlc_bar_obj = nullptr;
    m_nlc_bar_tele.remove_links(O);
}

/*
#ifdef DEBUG
CBaseMonster::SDebugInfo CBurer::show_debug_info()
{
    CBaseMonster::SDebugInfo info = inherited::show_debug_info();
    if (!info.active) return CBaseMonster::SDebugInfo();

    string128 text;
    DBG().text(this).add_item(text, info.x, info.y+=info.delta_y, info.color);
    DBG().text(this).add_item("---------------------------------------", info.x, info.y+=info.delta_y, info.delimiter_color);

    return CBaseMonster::SDebugInfo();
}
#endif
*/

void CBurer::face_enemy()
{
    if (!EnemyMan.get_enemy())
    {
        return;
    }
    Fvector const enemy_pos = EnemyMan.get_enemy()->Position();
    Fvector const self_pos = Position();
    Fvector const self2enemy = enemy_pos - self_pos;
    bool const good_aiming = angle_between_vectors(self2enemy, Direction()) < deg2rad(20.f);

    if (!good_aiming)
    {
        dir().face_target(enemy_pos);
    }

    set_action(ACT_STAND_IDLE);
}

bool actor_is_reloading_weapon()
{
    if (!Actor())
    {
        return false;
    }

    CWeapon* const active_weapon = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());
    if (active_weapon && active_weapon->GetState() == CWeapon::eReload)
    {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
// NLC burer pass 1 (docs/BURER_REDESIGN.md 5.2)
//////////////////////////////////////////////////////////////////////////

void CBurer::nlc_log(LPCSTR format, ...)
{
    if (!m_nlc_debug)
        return;

    string1024 buffer;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = 0;
    Msg("~ [burer] t=%u %s(%u) %s", Device.dwTimeGlobal, cName().c_str(), ID(), buffer);
}

void CBurer::nlc_load(LPCSTR section)
{
    m_nlc_debug = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_burer_debug", false);

    m_nlc_tele_dmg_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_tele_damage_min", 0.f);
    m_nlc_tele_dmg_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_tele_damage_max", m_nlc_tele_dmg_min);
    m_nlc_tele_dmg_mass_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_tele_damage_mass_min", 10.f);
    m_nlc_tele_dmg_mass_max = READ_IF_EXISTS(pSettings, r_float, section, "nlc_tele_damage_mass_max", 150.f);
    m_nlc_tele_lead = READ_IF_EXISTS(pSettings, r_float, section, "nlc_tele_lead", 0.f);
    clamp(m_nlc_tele_lead, 0.f, 1.f);

    m_nlc_gren.enabled = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_grenade_catch", false);
    m_nlc_gren.catch_radius = READ_IF_EXISTS(pSettings, r_float, section, "nlc_grenade_catch_radius", 10.f);
    m_nlc_gren.max_held = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_grenade_max_held", 2);
    m_nlc_gren.min_hold_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_grenade_min_hold_ms", 700);
    m_nlc_gren.max_hold_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_grenade_max_hold_ms", 10000);
    m_nlc_gren.safe_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_grenade_safe_dist", 7.f);
    m_nlc_gren.throw_speed = _max(READ_IF_EXISTS(pSettings, r_float, section, "nlc_grenade_throw_speed", 14.f), 1.f);
    m_nlc_gren.airburst = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_grenade_airburst", true);
    m_nlc_gren.fuse_extra_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_grenade_fuse_extra_ms", 1500);
    m_nlc_gren.late_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_grenade_late_ms", 4000);
    m_nlc_gren.late_safe_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_grenade_late_safe_dist", 5.f);

    m_nlc_repulse.enabled = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_repulse_enabled", false);
    m_nlc_repulse.trigger_dist = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_trigger_dist", 4.f);
    m_nlc_repulse.radius = _max(READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_radius", 6.f), 1.f);
    m_nlc_repulse.cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_repulse_cooldown", 9000);
    m_nlc_repulse.hit = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_hit", 0.12f);
    m_nlc_repulse.push_speed = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_push_speed", 9.f);
    m_nlc_repulse.push_up = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_push_up", 4.5f);
    m_nlc_repulse.push_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_repulse_push_ms", 250);
    m_nlc_repulse.falloff_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_falloff_min", 0.7f);
    m_nlc_gravi_push_speed = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gravi_push_speed", 0.f);
    m_nlc_gravi_push_up = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gravi_push_up", 0.f);
    m_nlc_gravi_push_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_gravi_push_ms", 300);
    m_nlc_tele_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_tele_cooldown", 5000);
    m_nlc_repulse.impulse_objects = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_impulse_objects", 8.f);
    m_nlc_repulse.hit_time = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_hit_time", 0.35f);
    clamp(m_nlc_repulse.hit_time, 0.f, 1.f);
    m_nlc_repulse.cam_inert = READ_IF_EXISTS(pSettings, r_float, section, "nlc_repulse_cam_inert", 0.f);
    m_nlc_repulse.particle = READ_IF_EXISTS(pSettings, r_string, section, "nlc_repulse_particle", "");
    pcstr repulse_sound = READ_IF_EXISTS(pSettings, r_string, section, "nlc_repulse_sound", "");
    if (repulse_sound && repulse_sound[0])
        ::Sound->create(m_nlc_repulse.sound, repulse_sound, st_Effect, SOUND_TYPE_WORLD);

    m_nlc_disarm_grace = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_disarm_grace", 20000);
    m_nlc_gravi_item_impulse = READ_IF_EXISTS(pSettings, r_float, section, "nlc_gravi_item_impulse", 6.f);
    m_nlc_gravi_after_disarm_ms = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_gravi_after_disarm_ms", 2500);

    // scan feedback: the old scanner's sound and post-process (CScanningAbility::load) on the M2 psy sense
    nlc_load_pass2(section);
    nlc_load_pass5(section);

    m_nlc_scan_fx = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_scan_fx", false) && pSettings->line_exist(section, "scan_sound") &&
        pSettings->line_exist(section, "scan_effector_section");
    m_nlc_scan_fx_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "nlc_scan_fx_cooldown", 30000);
    if (m_nlc_scan_fx)
    {
        LPCSTR ppi_section = pSettings->r_string(section, "scan_effector_section");
        if (!pSettings->section_exist(ppi_section))
        {
            Msg("! [burer] %s: scan_effector_section [%s] not found, scan feedback off", section, ppi_section);
            m_nlc_scan_fx = false;
        }
        else
        {
            ::Sound->create(sound_scan, pSettings->r_string(section, "scan_sound"), st_Effect, SOUND_TYPE_WORLD);
            m_nlc_scan_ppi.duality.h = pSettings->r_float(ppi_section, "duality_h");
            m_nlc_scan_ppi.duality.v = pSettings->r_float(ppi_section, "duality_v");
            m_nlc_scan_ppi.gray = pSettings->r_float(ppi_section, "gray");
            m_nlc_scan_ppi.blur = pSettings->r_float(ppi_section, "blur");
            m_nlc_scan_ppi.noise.intensity = pSettings->r_float(ppi_section, "noise_intensity");
            m_nlc_scan_ppi.noise.grain = pSettings->r_float(ppi_section, "noise_grain");
            m_nlc_scan_ppi.noise.fps = pSettings->r_float(ppi_section, "noise_fps");
            sscanf(pSettings->r_string(ppi_section, "color_base"), "%f,%f,%f", &m_nlc_scan_ppi.color_base.r, &m_nlc_scan_ppi.color_base.g, &m_nlc_scan_ppi.color_base.b);
            sscanf(pSettings->r_string(ppi_section, "color_gray"), "%f,%f,%f", &m_nlc_scan_ppi.color_gray.r, &m_nlc_scan_ppi.color_gray.g, &m_nlc_scan_ppi.color_gray.b);
            sscanf(pSettings->r_string(ppi_section, "color_add"), "%f,%f,%f", &m_nlc_scan_ppi.color_add.r, &m_nlc_scan_ppi.color_add.g, &m_nlc_scan_ppi.color_add.b);
            m_nlc_scan_time = pSettings->r_float(ppi_section, "time");
            m_nlc_scan_attack = pSettings->r_float(ppi_section, "time_attack");
            m_nlc_scan_release = pSettings->r_float(ppi_section, "time_release");
        }
    }
}

float CBurer::nlc_tele_damage(CPhysicsShellHolder* obj) const
{
    if (!obj || !obj->m_pPhysicsShell || m_nlc_tele_dmg_max <= 0.f)
        return 0.f;

    const float span = m_nlc_tele_dmg_mass_max - m_nlc_tele_dmg_mass_min;
    float k = span > EPS ? (obj->m_pPhysicsShell->getMass() - m_nlc_tele_dmg_mass_min) / span : 1.f;
    clamp(k, 0.f, 1.f);
    return m_nlc_tele_dmg_min + (m_nlc_tele_dmg_max - m_nlc_tele_dmg_min) * k;
}

Fvector CBurer::nlc_lead_point(const CEntityAlive* target, const Fvector& aim, float flight_time, float lead_k) const
{
    Fvector res = aim;
    if (!target || lead_k <= 0.f || flight_time <= 0.f)
        return res;

    // NLC pass 5: any moving target (stalkers, monsters), not only the actor
    CCharacterPhysicsSupport* cps = const_cast<CEntityAlive*>(target)->character_physics_support();
    if (!cps || !cps->movement())
        return res;

    Fvector offset = cps->movement()->GetVelocity();
    offset.y = 0.f;
    offset.mul(flight_time * lead_k);
    const float len = offset.magnitude();
    if (len > 4.f)
        offset.mul(4.f / len);
    res.add(offset);
    return res;
}

bool CBurer::nlc_clear_path(const Fvector& from, const Fvector& to, CObject* ignore, const CObject* target) const
{
    Fvector dir;
    dir.sub(to, from);
    const float len = dir.magnitude();
    if (len < EPS_L)
        return true;
    dir.div(len);

    collide::rq_result rq;
    if (!Level().ObjectSpace.RayPick(from, dir, len, collide::rqtBoth, rq, ignore))
        return true;
    if (target && rq.O == target)
        return true;
    return rq.range >= len - 0.3f;
}

//////////////////////////////////////////////////////////////////////////
// Repulse

bool CBurer::nlc_gravi_blocked_by_disarm()
{
    if (m_nlc_gravi_after_disarm_ms == 0)
        return false;

    // the stare disarm lives in the script; ask at most every 200 ms
    const u32 now = Device.dwTimeGlobal;
    if (now >= m_nlc_disarm_query_next)
    {
        m_nlc_disarm_query_next = now + 200;
        luabind::functor<float> func_disarm_age;
        m_nlc_disarm_recent = ai().script_engine().functor("bind_monster.burer_disarm_age", func_disarm_age) && func_disarm_age() < float(m_nlc_gravi_after_disarm_ms);
    }
    return m_nlc_disarm_recent;
}

void CBurer::nlc_push(CEntityAlive* entity, const Fvector& dir, float speed, float up, u32 hold_ms, LPCSTR kind)
{
    CCharacterPhysicsSupport* cps = entity ? entity->character_physics_support() : nullptr;
    if (!cps || !cps->movement())
        return;
    Fvector velocity;
    velocity.mul(dir, speed);
    velocity.y = up;
    cps->movement()->EnableCharacter(); // a body at rest sleeps: a velocity set on it never moved a standing actor
    cps->movement()->SetVelocity(velocity);
    if (entity == Actor())
    {
        const u32 now = Device.dwTimeGlobal;
        m_nlc_push_frame = Device.dwFrame;
        m_nlc_push_first_logged = false;
        m_nlc_push_vel = velocity;
        m_nlc_push_until = now + hold_ms;
        m_nlc_push_origin = entity->Position();
        m_nlc_push_log_at = now + 500;
        m_nlc_push_kind = kind;
    }
}

void CBurer::nlc_push_update()
{
    // the gravi hit's push, one frame after the hit
    if (m_nlc_gravi_push_id != u16(-1) && Device.dwFrame > m_nlc_gravi_push_frame)
    {
        CEntityAlive* target = smart_cast<CEntityAlive*>(Level().Objects.net_Find(m_nlc_gravi_push_id));
        m_nlc_gravi_push_id = u16(-1);
        if (target && target->g_Alive() && !target->getDestroy())
            nlc_push(target, m_nlc_gravi_push_dir, m_nlc_gravi_push_speed, m_nlc_gravi_push_up, m_nlc_gravi_push_ms, "gravi");
    }

    // the actor's velocity the frame after a push and at the end of the hold
    if (!m_nlc_push_first_logged && Device.dwFrame > m_nlc_push_frame)
    {
        m_nlc_push_first_logged = true;
        CActor* pushed = Actor();
        CCharacterPhysicsSupport* cps = pushed ? pushed->character_physics_support() : nullptr;
        if (cps && cps->movement())
        {
            Fvector v;
            cps->movement()->GetCharacterVelocity(v);
            nlc_log("%s push: next frame velocity %.1f m/s (horizontal %.1f, up %.1f; set %.1f)", m_nlc_push_kind, v.magnitude(), _sqrt(v.x * v.x + v.z * v.z), v.y,
                m_nlc_push_vel.magnitude());
        }
    }

    // how far the push actually moved the actor (blocked by walls / slopes shows here)
    if (m_nlc_push_log_at && Device.dwTimeGlobal >= m_nlc_push_log_at)
    {
        m_nlc_push_log_at = 0;
        if (CActor* pushed = Actor())
            nlc_log("%s push moved the actor %.1f m in 0.5 s (set %.1f m/s, up %.1f)", m_nlc_push_kind, pushed->Position().distance_to_xz(m_nlc_push_origin),
                m_nlc_push_vel.magnitude(), m_nlc_push_vel.y);
    }
    if (!m_nlc_push_until)
        return;
    CActor* actor = Actor();
    if (Device.dwTimeGlobal >= m_nlc_push_until || !actor || !actor->g_Alive())
    {
        m_nlc_push_until = 0;
        CCharacterPhysicsSupport* end_cps = actor ? actor->character_physics_support() : nullptr;
        if (end_cps && end_cps->movement())
        {
            Fvector v;
            end_cps->movement()->GetCharacterVelocity(v);
            nlc_log("%s push: hold over, velocity %.1f m/s (horizontal %.1f, up %.1f)", m_nlc_push_kind, v.magnitude(), _sqrt(v.x * v.x + v.z * v.z), v.y);
        }
        return;
    }

    // hold the horizontal push against the character controller; gravity keeps the vertical part
    CCharacterPhysicsSupport* cps = actor->character_physics_support();
    if (!cps || !cps->movement())
        return;
    Fvector velocity;
    cps->movement()->GetCharacterVelocity(velocity);
    velocity.x = m_nlc_push_vel.x;
    velocity.z = m_nlc_push_vel.z;
    cps->movement()->EnableCharacter();
    cps->movement()->SetVelocity(velocity);
}

bool CBurer::nlc_repulse_ready()
{
    if (!m_nlc_repulse.enabled || m_shield_active || Device.dwTimeGlobal < m_nlc_repulse_next)
        return false;

    const CEntityAlive* enemy = EnemyMan.get_enemy();
    if (!enemy || !EnemyMan.see_enemy_now())
        return false;

    return Position().distance_to(enemy->Position()) <= m_nlc_repulse.trigger_dist;
}

void CBurer::nlc_repulse_blast()
{
    const Fvector pos = Position();
    Fvector center = pos;
    center.y += 1.f;
    const float radius = m_nlc_repulse.radius;

    if (m_nlc_repulse.particle.size())
    {
        CParticlesObject* ps = CParticlesObject::Create(m_nlc_repulse.particle.c_str(), TRUE);
        Fmatrix xform;
        xform.identity();
        xform.c.set(pos);
        ps->UpdateParent(xform, {});
        ps->Play(false);
    }
    if (m_nlc_repulse.sound._handle())
        m_nlc_repulse.sound.play_at_pos(this, center);
    g_pGamePersistent->GrassBendersAddExplosion(ID(), pos, Direction(), 1.33f, 3.0f, ps_ssfx_grass_interactive.w, radius);

    u32 entities = 0, objects = 0;
    bool actor_hit = false;

    m_nlc_nearest.clear();
    Level().ObjectSpace.GetNearest(m_nlc_nearest, center, radius, this);
    for (CObject* O : m_nlc_nearest)
    {
        if (!O || O == this || O->getDestroy())
            continue;

        Fvector dir;
        dir.sub(O->Position(), pos);
        dir.y = 0.f;
        const float dist = dir.magnitude();
        if (dist > EPS_L)
            dir.div(dist);
        else
            dir.set(Direction());
        const float k = 1.f - (1.f - m_nlc_repulse.falloff_min) * _min(dist / radius, 1.f); // 100 % at the burer, falloff_min at the edge

        CEntityAlive* entity = smart_cast<CEntityAlive*>(O);
        if (entity && entity->g_Alive())
        {
            if (smart_cast<CBurer*>(entity) || nlc_stealth::is_visible_body(entity))
                continue;

            Fvector push = dir;
            push.y = 1.f;
            push.normalize();

            // strike damage; the push goes through the movement control because ApplyHit only moves characters
            // for explosion / wound hits
            NET_Packet packet;
            SHit hit;
            hit.GenHeader(GE_HIT, entity->ID());
            hit.whoID = ID();
            hit.weaponID = ID();
            hit.dir = push;
            hit.power = m_nlc_repulse.hit * k;
            IKinematics* kinematics = smart_cast<IKinematics*>(entity->Visual());
            hit.boneID = kinematics ? kinematics->LL_GetBoneRoot() : 0;
            hit.p_in_bone_space.set(0.f, 0.f, 0.f);
            hit.impulse = 0.f;
            hit.hit_type = ALife::eHitTypeStrike;
            hit.Write_Packet(packet);
            u_EventSend(packet);

            // velocity, not ApplyImpulse: CPHSimpleCharacter turns an impulse on a jumping / not-in-control character
            // straight down and the ground controller cancels the rest within a few steps (two test rounds: 20-30 cm)
            if (m_nlc_repulse.push_speed > 0.f)
                nlc_push(entity, dir, m_nlc_repulse.push_speed * k, m_nlc_repulse.push_up * k, m_nlc_repulse.push_ms, "repulse");
            nlc_log("repulse hit %s dist=%.1f k=%.2f power=%.3f push=%.1f up=%.1f", entity->cName().c_str(), dist, k, m_nlc_repulse.hit * k,
                m_nlc_repulse.push_speed * k, m_nlc_repulse.push_up * k);

            ++entities;
            if (entity == Actor())
            {
                actor_hit = true;
                luabind::functor<void> func_repulse_actor;
                if (m_nlc_repulse.cam_inert > 0.f && ai().script_engine().functor("bind_monster.burer_repulse_actor", func_repulse_actor))
                    func_repulse_actor(m_nlc_repulse.cam_inert, k);
            }
            continue;
        }

        CPhysicsShellHolder* obj = smart_cast<CPhysicsShellHolder*>(O);
        if (!obj || !obj->m_pPhysicsShell || !obj->m_pPhysicsShell->isActive() || smart_cast<CGrenade*>(O) || nlc_own_held(obj) ||
            (obj->spawn_ini() && obj->spawn_ini()->section_exist("ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "ph_heavy") && pSettings->r_bool(obj->cNameSect().c_str(), "ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "quest_item") && pSettings->r_bool(obj->cNameSect().c_str(), "quest_item")) || obj->hasFixedBones() ||
            !obj->m_pPhysicsShell->get_ApplyByGravity())
            continue;

        Fvector push = dir;
        push.y = 0.3f;
        push.normalize();
        obj->m_pPhysicsShell->applyImpulse(push, m_nlc_repulse.impulse_objects * obj->m_pPhysicsShell->getMass() * k);
        ++objects;
    }

    nlc_log("repulse blast radius=%.1f entities=%u objects=%u actor=%d", radius, entities, objects, actor_hit ? 1 : 0);
}

//////////////////////////////////////////////////////////////////////////
// Grenade catch

void CBurer::nlc_grenade_update()
{
    if (!m_nlc_gren.enabled)
        return;

    const u32 now = Device.dwTimeGlobal;

    nlc_rocket_update(get_head_position(this));

    // catch live grenades in flight or on the ground
    if (now >= m_nlc_gren_scan_next && m_nlc_held.size() < m_nlc_gren.max_held)
    {
        m_nlc_gren_scan_next = now + 150;
        const Fvector eye = get_head_position(this);
        m_nlc_nearest.clear();
        Level().ObjectSpace.GetNearest(m_nlc_nearest, Position(), m_nlc_gren.catch_radius, this);
        for (CObject* O : m_nlc_nearest)
        {
            CGrenade* grenade = smart_cast<CGrenade*>(O);
            if (!grenade || grenade->getDestroy() || grenade->H_Parent() || grenade->m_nlc_tele_held || !grenade->PPhysicsShell() || !grenade->PPhysicsShell()->isActive() ||
                !grenade->CExplosive::Useful() || grenade->destroy_time() == 0xffffffff)
                continue;
            if (grenade->nlc_fuse_remaining() < 150)
                continue;

            const u16 sender = grenade->Initiator();
            if (smart_cast<CBurer*>(Level().Objects.net_Find(sender)))
                continue; // a burer threw it: no ping-pong between burers

            Fvector center;
            grenade->Center(center);
            if (!nlc_clear_path(eye, center, this, grenade))
                continue;

            if (nlc_grenade_catch(grenade, sender) && m_nlc_held.size() >= m_nlc_gren.max_held)
                break;
        }
    }

    // throw back
    for (u32 i = 0; i < m_nlc_held.size(); ++i)
    {
        NlcHeldGrenade& held = m_nlc_held[i];
        CGrenade* grenade = held.grenade;
        if (now < held.caught + m_nlc_gren.min_hold_ms)
            continue;

        // the thrower if he is alive and visible, else the current enemy if visible
        const CEntityAlive* target = nullptr;
        if (held.sender != u16(-1) && held.sender != grenade->ID())
        {
            const CEntityAlive* sender = smart_cast<const CEntityAlive*>(Level().Objects.net_Find(held.sender));
            if (sender && sender != this && sender->g_Alive() && memory().visual().visible_now(sender))
                target = sender;
        }
        if (!target && EnemyMan.get_enemy() && EnemyMan.see_enemy_now())
            target = EnemyMan.get_enemy();

        // held a while (e.g. carried into cover): a closer target is fine too
        const float safe = now >= held.caught + m_nlc_gren.late_ms ? _min(m_nlc_gren.safe_dist, m_nlc_gren.late_safe_dist) : m_nlc_gren.safe_dist;
        if (target && target->Position().distance_to(Position()) >= safe)
        {
            Fvector aim;
            target->Center(aim);
            const float flight = clampr(grenade->Position().distance_to(aim) / m_nlc_gren.throw_speed, 0.5f, 2.f);
            aim = nlc_lead_point(target, aim, flight, m_nlc_tele_lead);
            if (nlc_clear_path(grenade->Position(), aim, grenade, target))
            {
                nlc_grenade_throw(i, aim, target == EnemyMan.get_enemy() ? "enemy" : "sender");
                --i;
                continue;
            }
        }

        if (now >= held.caught + m_nlc_gren.max_hold_ms)
        {
            // no valid target for too long: away from itself along its forward axis
            Fvector away;
            away.mad(Position(), Direction(), 12.f);
            away.y = Position().y + 0.5f;
            nlc_grenade_throw(i, away, "timeout");
            --i;
        }
    }
}

bool CBurer::nlc_grenade_catch(CGrenade* grenade, u16 sender)
{
    const u32 remaining = grenade->nlc_fuse_pause();
    CTelekineticObject* tele_obj = m_nlc_gren_tele.activate(grenade, 3.f, 2.f, m_nlc_gren.max_hold_ms + 5000, false);
    if (!tele_obj)
    {
        grenade->nlc_fuse_resume(remaining);
        return false;
    }

    // caught in flight: stop it dead (telekinesis only switches gravity off), then it rises and hovers
    grenade->PPhysicsShell()->set_LinearVel(Fvector().set(0.f, 0.f, 0.f));
    grenade->PPhysicsShell()->set_AngularVel(Fvector().set(0.f, 0.f, 0.f));

    tele_obj->set_sound(sound_tele_hold, sound_tele_throw);
    StartTeleObjectParticle(grenade);
    grenade->m_nlc_tele_held = true;
    grenade->set_destroy_callback(fastdelegate::MakeDelegate(this, &CBurer::nlc_on_grenade_destroyed));
    m_nlc_held.push_back({grenade, remaining, Device.dwTimeGlobal, sender});

    nlc_log("grenade catch %s(%u) fuse_left=%u sender=%u dist=%.1f", grenade->cNameSect().c_str(), grenade->ID(), remaining, sender, grenade->Position().distance_to(Position()));
    return true;
}

void CBurer::nlc_grenade_throw(u32 index, const Fvector& target, LPCSTR reason)
{
    const NlcHeldGrenade held = m_nlc_held[index];
    m_nlc_held.erase(m_nlc_held.begin() + index);
    CGrenade* grenade = held.grenade;

    const float dist = grenade->Position().distance_to(target);
    const float flight = clampr(dist / m_nlc_gren.throw_speed, 0.5f, 2.f);
    const u32 flight_ms = u32(flight * 1000.f);
    // airburst: lands, then runs for fuse_extra_ms; otherwise the grenade's own remaining fuse (at least the flight)
    const u32 fuse = m_nlc_gren.airburst ? flight_ms + 100 + m_nlc_gren.fuse_extra_ms : _max(held.remaining, flight_ms + 300);

    grenade->set_destroy_callback(CGrenade::destroy_callback());
    grenade->m_nlc_tele_held = false;
    if (CTelekineticObject* tele = m_nlc_gren_tele.find_object(grenade))
        tele->m_nlc_steer = false; // a snatched grenade is steered while held
    grenade->SetInitiator(ID()); // a grenade thrown back kills on the burer's account
    grenade->nlc_fuse_resume(fuse);
    StopTeleObjectParticle(grenade);
    m_nlc_gren_tele.fire_t(grenade, target, flight);
    sound().play(eMonsterSoundTeleAttack);

    nlc_log("grenade throw %u reason=%s dist=%.1f flight=%.2f fuse=%u held_ms=%u", grenade->ID(), reason, dist, flight, fuse, Device.dwTimeGlobal - held.caught);
}

void CBurer::nlc_grenade_release_all()
{
    for (NlcHeldRocket& held : m_nlc_held_rockets)
    {
        if (CTelekineticObject* tele = m_nlc_gren_tele.find_object(held.rocket))
            tele->m_nlc_steer = false;
        held.rocket->m_nlc_tele_held = false;
        held.rocket->nlc_fuse_resume(_max(held.fuse_left, 0.5f)); // the burer lets go: the fuse runs on
        StopTeleObjectParticle(held.rocket);
    }
    m_nlc_held_rockets.clear();

    for (NlcHeldGrenade& held : m_nlc_held)
    {
        CGrenade* grenade = held.grenade;
        grenade->set_destroy_callback(CGrenade::destroy_callback());
        grenade->m_nlc_tele_held = false;
        grenade->nlc_fuse_resume(held.remaining); // the burer lets go: the fuse runs on
        StopTeleObjectParticle(grenade);
    }
    if (!m_nlc_held.empty())
        nlc_log("grenade release count=%u", u32(m_nlc_held.size()));
    m_nlc_held.clear();

    // grenades already thrown have no destroy callback; drop whatever the container still holds
    if (m_nlc_gren_tele.is_active())
        m_nlc_gren_tele.deactivate();
}

void CBurer::nlc_on_grenade_destroyed(CGrenade* grenade)
{
    nlc_log("grenade %u exploded while held", grenade->ID());
    nlc_grenade_forget(grenade);
    m_nlc_gren_tele.remove_links(grenade);
}

void CBurer::nlc_rocket_update(const Fvector& eye)
{
    const u32 now = Device.dwTimeGlobal;

    if (m_nlc_held.size() + m_nlc_held_rockets.size() < m_nlc_gren.max_held && now >= m_nlc_rocket_scan_next)
    {
        m_nlc_rocket_scan_next = now + 150;
        m_nlc_nearest.clear();
        Level().ObjectSpace.GetNearest(m_nlc_nearest, Position(), m_nlc_gren.catch_radius, this);
        for (CObject* O : m_nlc_nearest)
        {
            CExplosiveRocket* rocket = smart_cast<CExplosiveRocket*>(O);
            if (!rocket || rocket->getDestroy() || rocket->H_Parent() || !rocket->nlc_in_flight() || !rocket->CExplosive::Useful() || !rocket->PPhysicsShell() ||
                !rocket->PPhysicsShell()->isActive() || rocket->m_nlc_tele_held || nlc_held_by_any(rocket))
                continue;
            const u16 sender = rocket->Initiator();
            if (smart_cast<CBurer*>(Level().Objects.net_Find(sender)))
                continue;
            Fvector center;
            rocket->Center(center);
            if (!nlc_clear_path(eye, center, this, rocket))
                continue;

            CTelekineticObject* tele = m_nlc_gren_tele.activate(rocket, 3.f, 0.3f, m_nlc_gren.max_hold_ms + 5000, false);
            if (!tele)
                continue;
            const float fuse = rocket->nlc_fuse_pause();
            rocket->nlc_set_owner(this); // touching the burer no longer detonates it
            rocket->nlc_clear_safe_dist();
            rocket->PPhysicsShell()->set_LinearVel(Fvector().set(0.f, 0.f, 0.f));
            rocket->PPhysicsShell()->set_AngularVel(Fvector().set(0.f, 0.f, 0.f));
            tele->m_nlc_steer = true;
            tele->m_nlc_target = rocket->Position();
            tele->m_nlc_target.y += 0.4f;
            tele->m_nlc_max_speed = 3.f;
            tele->set_sound(sound_tele_hold, sound_tele_throw);
            StartTeleObjectParticle(rocket);
            rocket->m_nlc_tele_held = true;
            m_nlc_held_rockets.push_back({rocket, fuse, now, sender});
            nlc_log("round catch %s(%u) fuse_left=%.2f sender=%u dist=%.1f", rocket->cNameSect().c_str(), rocket->ID(), fuse, sender, rocket->Position().distance_to(Position()));
            break;
        }
    }

    for (u32 i = 0; i < m_nlc_held_rockets.size(); ++i)
    {
        NlcHeldRocket& held = m_nlc_held_rockets[i];
        if (now < held.caught + m_nlc_gren.min_hold_ms)
            continue;

        const CEntityAlive* target = nullptr;
        const CEntityAlive* sender = smart_cast<const CEntityAlive*>(Level().Objects.net_Find(held.sender));
        if (sender && sender != this && sender->g_Alive() && memory().visual().visible_now(sender))
            target = sender;
        if (!target && EnemyMan.get_enemy() && EnemyMan.see_enemy_now())
            target = EnemyMan.get_enemy();

        if (target && target->Position().distance_to(Position()) >= m_nlc_gren.safe_dist)
        {
            Fvector aim;
            target->Center(aim);
            const float flight = clampr(held.rocket->Position().distance_to(aim) / m_nlc_gren.throw_speed, 0.5f, 2.f);
            aim = nlc_lead_point(target, aim, flight, m_nlc_tele_lead);
            if (nlc_clear_path(held.rocket->Position(), aim, held.rocket, target))
            {
                nlc_rocket_throw(i, aim, target == EnemyMan.get_enemy() ? "enemy" : "sender");
                --i;
                continue;
            }
        }
        if (now >= held.caught + m_nlc_gren.max_hold_ms)
        {
            Fvector away;
            away.mad(Position(), Direction(), 12.f);
            away.y = Position().y + 0.5f;
            nlc_rocket_throw(i, away, "timeout");
            --i;
        }
    }
}

void CBurer::nlc_rocket_throw(u32 index, const Fvector& target, LPCSTR reason)
{
    const NlcHeldRocket held = m_nlc_held_rockets[index];
    m_nlc_held_rockets.erase(m_nlc_held_rockets.begin() + index);
    CExplosiveRocket* rocket = held.rocket;

    const float dist = rocket->Position().distance_to(target);
    const float flight = clampr(dist / m_nlc_gren.throw_speed, 0.5f, 2.f);
    if (CTelekineticObject* tele = m_nlc_gren_tele.find_object(rocket))
        tele->m_nlc_steer = false;
    rocket->m_nlc_tele_held = false;
    rocket->SetInitiator(ID()); // a returned round kills on the burer's account
    rocket->nlc_fuse_resume(flight + 1.f); // contact detonates it on arrival; the fuse is a fallback
    StopTeleObjectParticle(rocket);
    m_nlc_gren_tele.fire_t(rocket, target, flight);
    sound().play(eMonsterSoundTeleAttack);
    nlc_log("round throw %u reason=%s dist=%.1f flight=%.2f", rocket->ID(), reason, dist, flight);
}

void CBurer::nlc_grenade_forget(CObject* O)
{
    for (u32 i = 0; i < m_nlc_held_rockets.size(); ++i)
    {
        if (m_nlc_held_rockets[i].rocket == O)
        {
            m_nlc_held_rockets.erase(m_nlc_held_rockets.begin() + i);
            break;
        }
    }
    for (u32 i = 0; i < m_nlc_held.size(); ++i)
    {
        if (m_nlc_held[i].grenade == O)
        {
            m_nlc_held.erase(m_nlc_held.begin() + i);
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Scan feedback

void CBurer::nlc_scan_update()
{
    if (m_nlc_scan_playing && !sound_scan._feedback())
    {
        can_scan = true;
        m_nlc_scan_playing = false;
    }

    if (!m_nlc_scan_fx || !g_Alive())
        return;

    CActor* actor = Actor();
    if (!actor || !actor->g_Alive())
        return;

    const CVisualMemoryManager::NlcSense& sense = memory().visual().m_nlc_sense;
    const float level = sense.level;
    const bool crossed = sense.notice > 0.f && m_nlc_scan_prev_level < sense.notice && level >= sense.notice;
    m_nlc_scan_prev_level = level;
    if (!crossed || EnemyMan.get_enemy() == actor)
        return;

    const u32 now = Device.dwTimeGlobal;
    if (now < m_nlc_scan_next || !can_scan)
        return;
    m_nlc_scan_next = now + m_nlc_scan_fx_cooldown;

    ::Sound->play_at_pos(sound_scan, 0, actor->Position());
    actor->Cameras().AddPPEffector(xr_new<CMonsterEffector>(m_nlc_scan_ppi, m_nlc_scan_time, m_nlc_scan_attack, m_nlc_scan_release));
    can_scan = false;
    m_nlc_scan_playing = true;

    nlc_log("scan fx sense=%.2f notice=%.2f dist=%.1f", level, sense.notice, Position().distance_to(actor->Position()));
}
