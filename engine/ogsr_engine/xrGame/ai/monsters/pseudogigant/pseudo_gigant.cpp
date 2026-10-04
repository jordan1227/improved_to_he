#include "stdafx.h"
#include "pseudo_gigant.h"
#include "pseudo_gigant_step_effector.h"
#include "../../../actor.h"
#include "../../../ActorEffector.h"
#include "../../../level.h"
#include "pseudogigant_state_manager.h"
#include "../monster_velocity_space.h"
#include "../control_animation_base.h"
#include "../control_movement_base.h"
#include "../ai_monster_effector.h"
#include "../../../../xr_3da/CameraBase.h"
#include "../../../xr_level_controller.h"
#include "../../../detail_path_manager_space.h"
#include "../../../detail_path_manager.h"
#include "../../../CharacterPhysicsSupport.h"
#include "../control_path_builder_base.h"
#include "../../../../xr_3da/IGame_Persistent.h"
#include "../../../PHMovementControl.h" // NLC: stomp splash
#include "../../../Grenade.h"
#include "../../stalker/ai_stalker.h"
#include "../../../stalker_animation_names.h"
#include "../../../ai_space.h" // NLC: charge locked point
#include "level_graph.h"
#include "../../../ai_object_location.h"

CPseudoGigant::CPseudoGigant()
{
    CControlled::init_external(this);

    StateMan = xr_new<CStateManagerGigant>(this);

    com_man().add_ability(ControlCom::eControlRunAttack);
    com_man().add_ability(ControlCom::eControlThreaten);
    // com_man().add_ability(ControlCom::eControlJump);
    com_man().add_ability(ControlCom::eControlRotationJump);
}

CPseudoGigant::~CPseudoGigant() { xr_delete(StateMan); }

void CPseudoGigant::Load(LPCSTR section)
{
    inherited::Load(section);

    anim().AddReplacedAnim(&m_bDamaged, eAnimRun, eAnimRunDamaged);
    anim().AddReplacedAnim(&m_bDamaged, eAnimWalkFwd, eAnimWalkDamaged);
    // anim().AddReplacedAnim(&m_bRunTurnLeft,		eAnimRun,		eAnimRunTurnLeft);
    // anim().AddReplacedAnim(&m_bRunTurnRight,	eAnimRun,		eAnimRunTurnRight);

    anim().accel_load(section);
    // anim().accel_chain_add		(eAnimWalkFwd,		eAnimRun);
    // anim().accel_chain_add		(eAnimWalkFwd,		eAnimRunTurnLeft);
    // anim().accel_chain_add		(eAnimWalkFwd,		eAnimRunTurnRight);
    // anim().accel_chain_add		(eAnimWalkDamaged,	eAnimRunDamaged);

    step_effector.time = pSettings->r_float(section, "step_effector_time");
    step_effector.amplitude = pSettings->r_float(section, "step_effector_amplitude");
    step_effector.period_number = pSettings->r_float(section, "step_effector_period_number");

    SVelocityParam& velocity_none = move().get_velocity(MonsterMovement::eVelocityParameterIdle);
    SVelocityParam& velocity_turn = move().get_velocity(MonsterMovement::eVelocityParameterStand);
    SVelocityParam& velocity_walk = move().get_velocity(MonsterMovement::eVelocityParameterWalkNormal);
    //	SVelocityParam &velocity_run		= move().get_velocity(MonsterMovement::eVelocityParameterRunNormal);
    SVelocityParam& velocity_walk_dmg = move().get_velocity(MonsterMovement::eVelocityParameterWalkDamaged);
    //	SVelocityParam &velocity_run_dmg	= move().get_velocity(MonsterMovement::eVelocityParameterRunDamaged);
    SVelocityParam& velocity_steal = move().get_velocity(MonsterMovement::eVelocityParameterSteal);

    anim().AddAnim(eAnimStandIdle, "stand_idle_", -1, &velocity_none, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimStandTurnLeft, "stand_turn_ls_", -1, &velocity_turn, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimStandTurnRight, "stand_turn_rs_", -1, &velocity_turn, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimLieIdle, "stand_sleep_", -1, &velocity_none, PS_LIE, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimSitIdle, "sit_idle_", -1, &velocity_none, PS_SIT, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimSleep, "stand_sleep_", -1, &velocity_none, PS_LIE, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimWalkFwd, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimWalkDamaged, "stand_walk_fwd_dmg_", -1, &velocity_walk_dmg, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimRun, "stand_walk_fwd_", -1, &velocity_walk, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimRunDamaged, "stand_walk_fwd_dmg_", -1, &velocity_walk_dmg, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimEat, "stand_eat_", -1, &velocity_none, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimAttack, "stand_attack_", -1, &velocity_none, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimLookAround, "stand_idle_", -1, &velocity_none, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimSteal, "stand_steal_", -1, &velocity_steal, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimDie, "stand_idle_", -1, &velocity_none, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimStandLieDown, "stand_lie_down_", -1, &velocity_none, PS_STAND, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
    anim().AddAnim(eAnimLieToSleep, "lie_to_sleep_", -1, &velocity_none, PS_LIE, "fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    // anim().AddAnim(eAnimThreaten,		"stand_kick_",			-1, &velocity_none,		PS_STAND,	"fx_stand_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

    // 	anim().AddAnim(eAnimRunTurnLeft,	"stand_run_left_",		-1, &velocity_run,		PS_STAND);
    // 	anim().AddAnim(eAnimRunTurnRight,	"stand_run_right_",		-1, &velocity_run,		PS_STAND);

    anim().LinkAction(ACT_STAND_IDLE, eAnimStandIdle);
    anim().LinkAction(ACT_SIT_IDLE, eAnimSitIdle);
    anim().LinkAction(ACT_LIE_IDLE, eAnimLieIdle);
    anim().LinkAction(ACT_WALK_FWD, eAnimWalkFwd);
    anim().LinkAction(ACT_WALK_BKWD, eAnimWalkFwd);
    anim().LinkAction(ACT_RUN, eAnimRun);
    anim().LinkAction(ACT_EAT, eAnimEat);
    anim().LinkAction(ACT_SLEEP, eAnimSleep);
    anim().LinkAction(ACT_REST, eAnimSleep);
    anim().LinkAction(ACT_DRAG, eAnimWalkFwd);
    anim().LinkAction(ACT_ATTACK, eAnimAttack);
    anim().LinkAction(ACT_STEAL, eAnimSteal);
    anim().LinkAction(ACT_LOOK_AROUND, eAnimStandIdle);

    // define transitions
    anim().AddTransition(eAnimStandLieDown, eAnimSleep, eAnimLieToSleep, false);
    anim().AddTransition(PS_STAND, eAnimSleep, eAnimStandLieDown, true);
    anim().AddTransition(PS_STAND, PS_LIE, eAnimStandLieDown, false);

#ifdef DEBUG
    anim().accel_chain_test();
#endif

    // Load psi postprocess --------------------------------------------------------
    LPCSTR ppi_section = pSettings->r_string(section, "threaten_effector");
    m_threaten_effector.ppi.duality.h = pSettings->r_float(ppi_section, "duality_h");
    m_threaten_effector.ppi.duality.v = pSettings->r_float(ppi_section, "duality_v");
    m_threaten_effector.ppi.gray = pSettings->r_float(ppi_section, "gray");
    m_threaten_effector.ppi.blur = pSettings->r_float(ppi_section, "blur");
    m_threaten_effector.ppi.noise.intensity = pSettings->r_float(ppi_section, "noise_intensity");
    m_threaten_effector.ppi.noise.grain = pSettings->r_float(ppi_section, "noise_grain");
    m_threaten_effector.ppi.noise.fps = pSettings->r_float(ppi_section, "noise_fps");
    VERIFY(!fis_zero(m_threaten_effector.ppi.noise.fps));

    sscanf(pSettings->r_string(ppi_section, "color_base"), "%f,%f,%f", &m_threaten_effector.ppi.color_base.r, &m_threaten_effector.ppi.color_base.g,
           &m_threaten_effector.ppi.color_base.b);
    sscanf(pSettings->r_string(ppi_section, "color_gray"), "%f,%f,%f", &m_threaten_effector.ppi.color_gray.r, &m_threaten_effector.ppi.color_gray.g,
           &m_threaten_effector.ppi.color_gray.b);
    sscanf(pSettings->r_string(ppi_section, "color_add"), "%f,%f,%f", &m_threaten_effector.ppi.color_add.r, &m_threaten_effector.ppi.color_add.g,
           &m_threaten_effector.ppi.color_add.b);

    m_threaten_effector.time = pSettings->r_float(ppi_section, "time");
    m_threaten_effector.time_attack = pSettings->r_float(ppi_section, "time_attack");
    m_threaten_effector.time_release = pSettings->r_float(ppi_section, "time_release");

    m_threaten_effector.ce_time = pSettings->r_float(ppi_section, "ce_time");
    m_threaten_effector.ce_amplitude = pSettings->r_float(ppi_section, "ce_amplitude");
    m_threaten_effector.ce_period_number = pSettings->r_float(ppi_section, "ce_period_number");
    m_threaten_effector.ce_power = pSettings->r_float(ppi_section, "ce_power");

    // --------------------------------------------------------------------------------

    ::Sound->create(m_sound_threaten_hit, pSettings->r_string(section, "sound_threaten_hit"), st_Effect, SOUND_TYPE_WORLD);
    ::Sound->create(m_sound_start_threaten, pSettings->r_string(section, "sound_threaten_start"), st_Effect, SOUND_TYPE_MONSTER_ATTACKING);

    m_kick_damage = pSettings->r_float(section, "HugeKick_Damage");
    m_kick_particles = pSettings->r_string(section, "HugeKick_Particles");
    read_distance(section, "HugeKick_MinMaxDist", m_threaten_dist_min, m_threaten_dist_max);
    read_delay(section, "HugeKick_MinMaxDelay", m_threaten_delay_min, m_threaten_delay_max);

    m_time_kick_actor_slow_down = pSettings->r_u32(section, "HugeKick_Time_SlowDown");
    m_kick_hit_jumping_actor = READ_IF_EXISTS(pSettings, r_bool, section, "HugeKick_Hit_Jumping_Actor", false);

    load_stomp_params(section); // NLC
    load_moves_params(section);

    PostLoad(section);
}

void CPseudoGigant::reinit()
{
    inherited::reinit();

    m_time_next_threaten = 0;
    m_time_next_grenade_stomp = 0; // NLC
    m_time_next_grenade_scan = 0;
    m_grenade_seen = false;
    m_stomp_mode = eStompNormal;
    m_rage = false;
    m_rage_stomp_pending = false;
    m_recover_pending = false;
    m_recent_damage.clear();
    m_time_next_recover = 0;
    m_time_next_charge = 0;
    m_charge_until = 0;
    m_charge_strike_until = 0;
    m_charge_rush_start = 0;
    m_charge_locked = false;
    m_time_next_moves_update = 0;
    nlc_set_speed_bonus(1.f);
    nlc_clear_haste();

    if (CCustomMonster::use_simplified_visual())
        return;

    move().load_velocity(*cNameSect(), "Velocity_JumpPrepare", MonsterMovement::eGiantVelocityParameterJumpPrepare);
    move().load_velocity(*cNameSect(), "Velocity_JumpGround", MonsterMovement::eGiantVelocityParameterJumpGround);

    // com_man().load_jump_data(0,"jump_attack_0", "jump_attack_1", "jump_attack_2", MonsterMovement::eGiantVelocityParameterJumpPrepare,
    // MonsterMovement::eGiantVelocityParameterJumpGround,0);
    com_man().add_rotation_jump_data("1", "2", "3", "4", PI_DIV_2);

    com_man().set_threaten_data("stand_kick_0", 0.43f);
}

#define MAX_STEP_RADIUS 60.f

void CPseudoGigant::event_on_step()
{
    //////////////////////////////////////////////////////////////////////////
    // Earthquake Effector	//////////////
    CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());
    if (pActor)
    {
        float dist_to_actor = pActor->Position().distance_to(Position());
        float max_dist = MAX_STEP_RADIUS;
        if (dist_to_actor < max_dist)
            Actor()->Cameras().AddCamEffector(
                xr_new<CPseudogigantStepEffector>(step_effector.time, step_effector.amplitude, step_effector.period_number, (max_dist - dist_to_actor) / (1.2f * max_dist)));
    }
    //////////////////////////////////
}

bool CPseudoGigant::check_start_conditions(ControlCom::EControlType type)
{
    if (!inherited::check_start_conditions(type))
        return false;

    if (type == ControlCom::eControlRunAttack)
        return true;

    if (type == ControlCom::eControlThreaten)
    {
        if (!EnemyMan.get_enemy())
            return false;

        // NLC: a thrown grenade nearby -> quick deflection stomp on its own cooldown
        // (the sighting is only used while fresh: the scan runs every 250 ms while the stomp could start)
        if (m_grenade_seen && time() < m_time_grenade_seen + 500)
        {
            m_stomp_mode = eStompGrenade;
            return true;
        }

        // NLC: burst damage -> immediate recovery stomp (own cooldown)
        if (m_recover_pending)
        {
            m_stomp_mode = eStompRecover;
            return true;
        }

        // NLC: entering rage -> one immediate stomp
        if (m_rage_stomp_pending)
        {
            m_stomp_mode = eStompRage;
            return true;
        }

        if (m_time_next_threaten > time())
            return false;

        // check distance to enemy
        float dist = EnemyMan.get_enemy()->Position().distance_to(Position());

        if ((dist <= m_threaten_dist_max) && (dist >= m_threaten_dist_min))
        {
            m_stomp_mode = eStompNormal;
            return true;
        }

        // NLC: crowd stomp - enough enemies around, regardless of the target distance window
        if (stomp_crowd_min() && count_crowd() >= stomp_crowd_min())
        {
            m_stomp_mode = eStompCrowd;
            return true;
        }

        return false;
    }

    return true;
}

void CPseudoGigant::on_activate_control(ControlCom::EControlType type)
{
    if (type == ControlCom::eControlThreaten)
    {
        m_sound_start_threaten.play_at_pos(this, get_head_position(this));

        switch (m_stomp_mode)
        {
        case eStompGrenade:
            m_grenade_seen = false;
            m_time_next_grenade_stomp = time() + m_grenade_delay;
            // no regular stomp right after the deflection
            m_time_next_threaten = std::max(m_time_next_threaten, time() + 5000);
            break;
        case eStompRecover:
            m_recover_pending = false;
            m_recent_damage.clear();
            m_time_next_recover = time() + m_recover_delay;
            m_time_next_threaten = std::max(m_time_next_threaten, time() + 5000);
            break;
        case eStompRage:
            m_rage_stomp_pending = false;
            m_time_next_threaten = time() + u32(Random.randI(m_threaten_delay_min, m_threaten_delay_max) * m_rage_delay_k);
            break;
        default: {
            const float delay_k = m_rage ? m_rage_delay_k : 1.f;
            m_time_next_threaten = time() + u32(Random.randI(m_threaten_delay_min, m_threaten_delay_max) * delay_k);
        }
        break;
        }

        // a stomp interrupts a charge
        m_charge_until = 0;
        nlc_clear_haste();

        if (CMonsterEnemyMemory::target_debug_log())
        {
            static const char* names[] = {"normal", "crowd", "grenade", "recover", "rage"};
            Msg("~ [giant_stomp] [%s]: start, mode %s%s", cName().c_str(), names[m_stomp_mode], m_rage ? " (rage)" : "");
        }
    }
}

bool CPseudoGigant::threaten_skip_facing()
{
    scan_for_grenades();
    return (m_grenade_seen && time() < m_time_grenade_seen + 500) || m_recover_pending || m_rage_stomp_pending;
}

float CPseudoGigant::threaten_anim_speed_k() { return (m_stomp_mode == eStompGrenade) ? m_grenade_anim_speed : 1.f; }

void CPseudoGigant::load_stomp_params(LPCSTR section)
{
    m_crowd_radius = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Crowd_Radius", 0.f);
    m_crowd_min = READ_IF_EXISTS(pSettings, r_u32, section, "HugeKick_Crowd_Min", 0);
    m_height_gate = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Height_Gate", 3.f);
    m_splash_radius = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Splash_Radius", 0.f);
    m_splash_hit_neutrals = !!READ_IF_EXISTS(pSettings, r_bool, section, "HugeKick_Splash_Hit_Neutrals", false);
    m_damage_k_monster = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Damage_K_Monster", 1.f);
    m_damage_k_stalker = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Damage_K_Stalker", 1.f);

    // only strike or explosion make sense for a shockwave; anything else falls back to strike
    auto read_hit_type = [section](LPCSTR key) {
        LPCSTR value = READ_IF_EXISTS(pSettings, r_string, section, key, "strike");
        return (value && !_stricmp(value, "explosion")) ? ALife::eHitTypeExplosion : ALife::eHitTypeStrike;
    };
    m_hit_type_monster = read_hit_type("HugeKick_Hit_Type_Monster");
    m_hit_type_stalker = read_hit_type("HugeKick_Hit_Type_Stalker");
    m_hit_bone_monster = READ_IF_EXISTS(pSettings, r_string, section, "HugeKick_Hit_Bone_Monster", "");
    m_impulse = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Impulse", 0.f);
    m_stagger_min = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Stagger_Min", 0.f);
    m_slow_k = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Slow_K", 0.f);
    m_slow_time = READ_IF_EXISTS(pSettings, r_u32, section, "HugeKick_Slow_Time", 0);
    m_hit_actor_in_splash = !!READ_IF_EXISTS(pSettings, r_bool, section, "HugeKick_Hit_Actor_In_Splash", false);
    m_dodge_height = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Dodge_Height", 0.f);

    m_grenade_radius = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Grenade_Radius", 0.f);
    m_grenade_delay = READ_IF_EXISTS(pSettings, r_u32, section, "HugeKick_Grenade_Delay", 30000);
    m_grenade_damage_k = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Grenade_Damage_K", 1.f);
    m_grenade_anim_speed = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Grenade_Anim_Speed", 1.f);
    clamp(m_grenade_anim_speed, 0.5f, 3.f);
}

void CPseudoGigant::load_moves_params(LPCSTR section)
{
    m_rage_enabled = !!READ_IF_EXISTS(pSettings, r_bool, section, "HugeKick_Rage_Enabled", false);
    m_rage_health = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Rage_Health", 0.35f);
    m_rage_delay_k = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Rage_Delay_K", 1.f);
    m_rage_crowd_min = READ_IF_EXISTS(pSettings, r_u32, section, "HugeKick_Rage_Crowd_Min", 0);
    m_rage_splash_k = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Rage_Splash_K", 1.f);
    m_rage_speed_k = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Rage_Speed_K", 1.f);
    m_rage_entry_stomp = !!READ_IF_EXISTS(pSettings, r_bool, section, "HugeKick_Rage_Entry_Stomp", false);
    clamp(m_rage_delay_k, 0.1f, 2.f);
    clamp(m_rage_speed_k, 0.5f, 2.f);

    m_recover_enabled = !!READ_IF_EXISTS(pSettings, r_bool, section, "HugeKick_Recover_Enabled", false);
    m_recover_damage = READ_IF_EXISTS(pSettings, r_float, section, "HugeKick_Recover_Damage", 0.12f);
    m_recover_window = READ_IF_EXISTS(pSettings, r_u32, section, "HugeKick_Recover_Window", 3000);
    m_recover_delay = READ_IF_EXISTS(pSettings, r_u32, section, "HugeKick_Recover_Delay", 20000);

    m_charge_enabled = !!READ_IF_EXISTS(pSettings, r_bool, section, "Charge_Enabled", false);
    if (pSettings->line_exist(section, "Charge_Dist"))
        read_distance(section, "Charge_Dist", m_charge_dist_min, m_charge_dist_max);
    else
        m_charge_dist_min = 6.f, m_charge_dist_max = 15.f;
    if (pSettings->line_exist(section, "Charge_Delay"))
        read_delay(section, "Charge_Delay", m_charge_delay_min, m_charge_delay_max);
    else
        m_charge_delay_min = 15000, m_charge_delay_max = 25000;
    m_charge_max_height = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Max_Height", 2.f);
    m_charge_speed_k = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Speed_K", 1.6f);
    m_charge_max_time = READ_IF_EXISTS(pSettings, r_u32, section, "Charge_Max_Time", 3000);
    m_charge_end_dist = READ_IF_EXISTS(pSettings, r_float, section, "Charge_End_Dist", 3.3f);
    m_charge_strike_window = READ_IF_EXISTS(pSettings, r_u32, section, "Charge_Strike_Window", 1500);
    m_charge_damage_k = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Damage_K", 1.5f);
    m_charge_impulse_k = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Impulse_K", 2.f);
    m_charge_stumble_k = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Stumble_K", 0.f);
    m_charge_stumble_time = READ_IF_EXISTS(pSettings, r_u32, section, "Charge_Stumble_Time", 0);
    m_charge_contact_dist = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Contact_Dist", 0.f);
    m_charge_contact_yaw = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Contact_Yaw", 1.0f);
    m_charge_contact_power = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Contact_Power", 0.75f);
    m_charge_contact_impulse = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Contact_Impulse", 600.f);
    m_charge_windup = READ_IF_EXISTS(pSettings, r_u32, section, "Charge_Windup", 0);
    m_charge_max_turn = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Max_Turn", 0.f);
    m_charge_overshoot = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Overshoot", 3.f);
    m_charge_face_yaw = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Face_Yaw", 0.5f);
    m_charge_windup_anim = READ_IF_EXISTS(pSettings, r_string, section, "Charge_Windup_Anim", "");
    m_charge_windup_anim_speed = READ_IF_EXISTS(pSettings, r_float, section, "Charge_Windup_Anim_Speed", 1.f);
    clamp(m_charge_windup_anim_speed, 0.5f, 3.f);
    clamp(m_charge_speed_k, 1.f, 3.f);
}

void CPseudoGigant::shedule_Update(u32 dt)
{
    inherited::shedule_Update(dt);

    if (g_Alive() && !getDestroy())
        update_moves();
}

void CPseudoGigant::update_moves()
{
    if (time() < m_time_next_moves_update)
        return;
    m_time_next_moves_update = time() + (m_charge_until ? 50 : 100); // finer during the rush (contact check)

    // rage: below HugeKick_Rage_Health; leaves again if healed 10% above it
    if (m_rage_enabled)
    {
        const float health = conditions().GetHealth() / std::max(conditions().GetMaxHealth(), EPS_L);
        if (!m_rage && health < m_rage_health)
        {
            m_rage = true;
            nlc_set_speed_bonus(m_rage_speed_k);
            m_rage_stomp_pending = m_rage_entry_stomp;
            if (CMonsterEnemyMemory::target_debug_log())
                Msg("~ [giant_moves] [%s]: rage at %.0f%% health", cName().c_str(), health * 100.f);
        }
        else if (m_rage && health > m_rage_health + 0.1f)
        {
            m_rage = false;
            m_rage_stomp_pending = false;
            nlc_set_speed_bonus(1.f);
        }
    }

    // recovery window bookkeeping
    if (!m_recent_damage.empty())
    {
        const u32 now = time();
        m_recent_damage.erase(std::remove_if(m_recent_damage.begin(), m_recent_damage.end(),
                                             [this, now](const std::pair<u32, float>& d) { return d.first + m_recover_window < now; }),
                              m_recent_damage.end());
    }

    update_charge();
}

void CPseudoGigant::nlc_on_health_lost(const CObject* who, float fraction)
{
    if (!m_recover_enabled || m_recover_pending || time() < m_time_next_recover)
        return;

    m_recent_damage.emplace_back(time(), fraction);

    float sum = 0.f;
    for (const auto& d : m_recent_damage)
        if (d.first + m_recover_window >= time())
            sum += d.second;

    if (sum >= m_recover_damage)
    {
        m_recover_pending = true;
        if (CMonsterEnemyMemory::target_debug_log())
            Msg("~ [giant_moves] [%s]: recovery stomp, %.0f%% health lost in %u ms", cName().c_str(), sum * 100.f, m_recover_window);
    }
}

// charge: wind-up (in-place clip + sound), then a rush to a point locked past the target's spot.
// Ram on touch while the target is in front; reaching the locked point without contact = dodged -> limp (stumble).
void CPseudoGigant::update_charge()
{
    if (!m_charge_enabled)
        return;

    const u32 now = time();
    const CEntityAlive* enemy = EnemyMan.get_enemy();

    auto end_charge = [this]() {
        if (m_charge_rush_start) // still in the wind-up: stop our clip
            com_man().seq_stop();
        m_charge_until = 0;
        m_charge_rush_start = 0;
        m_charge_locked = false;
        nlc_clear_haste();
    };
    auto stumble = [this](LPCSTR why) {
        if (m_charge_stumble_time)
            nlc_force_damaged(m_charge_stumble_time);
        if (CMonsterEnemyMemory::target_debug_log())
            Msg("~ [giant_moves] [%s]: charge %s -> stumble", cName().c_str(), why);
    };

    // strike-window variant (Charge_Contact_Dist = 0): window ran out without a hit -> stumble
    if (m_charge_strike_until && now >= m_charge_strike_until)
    {
        m_charge_strike_until = 0;
        stumble("strike missed");
    }

    if (m_charge_until)
    {
        const bool lost = !enemy || !enemy->g_Alive();
        if (lost || now >= m_charge_until)
        {
            end_charge();
            if (CMonsterEnemyMemory::target_debug_log())
                Msg("~ [giant_moves] [%s]: charge ended (%s)", cName().c_str(), lost ? "enemy lost" : "timeout");
            return;
        }

        Fvector to_enemy;
        to_enemy.sub(enemy->Position(), Position());
        to_enemy.y = 0.f;
        const float dist = to_enemy.magnitude();
        to_enemy.normalize_safe();

        // wind-up: the clip plays in place (sequencer); the line is taken from the facing at its end
        if (m_charge_rush_start && now < m_charge_rush_start)
            return;

        if (m_charge_rush_start)
        {
            m_charge_rush_start = 0;
            com_man().seq_stop();

            // lock a point past the target's current spot along the line giant -> target
            m_charge_dir = to_enemy;
            Fvector point;
            point.mad(enemy->Position(), m_charge_dir, m_charge_overshoot);
            u32 vertex = ai().level_graph().vertex_id(point);
            if (!ai().level_graph().valid_vertex_id(vertex))
            {
                point.set(enemy->Position());
                vertex = enemy->ai_location().level_vertex_id();
            }
            m_charge_point = ai().level_graph().vertex_position(vertex);
            m_charge_vertex = vertex;
            m_charge_locked = true;

            nlc_set_haste(m_charge_speed_k, m_charge_until - now);
            if (CMonsterEnemyMemory::target_debug_log())
                Msg("~ [giant_moves] [%s]: charge rush at %.1f m, locked point %.1f m away", cName().c_str(), dist, m_charge_point.distance_to_xz(Position()));
        }

        if (m_charge_contact_dist > 0.f)
        {
            const float off_line = acosf(std::clamp(Direction().dotproduct(to_enemy) / std::max(Direction().magnitude(), EPS_L), -1.f, 1.f));

            if (dist <= m_charge_contact_dist && off_line <= m_charge_contact_yaw)
            {
                end_charge();
                Fvector local_dir;
                local_dir.set(0.f, 0.4f, 1.f).normalize();
                if (CMonsterEnemyMemory::target_debug_log())
                    Msg("~ [giant_moves] [%s]: charge ram on [%s] at %.1f m, power %.2f, impulse %.0f", cName().c_str(), enemy->cName().c_str(), dist,
                        m_charge_contact_power, m_charge_contact_impulse);
                inherited::HitEntity(enemy, m_charge_contact_power, m_charge_contact_impulse, local_dir);
                return;
            }

            // passed the target's spot without contact
            if (m_charge_locked && m_charge_point.distance_to_xz(Position()) <= 1.5f)
            {
                end_charge();
                stumble("dodged (reached the locked point)");
            }
            return;
        }

        if (dist <= m_charge_end_dist)
        {
            end_charge();
            m_charge_strike_until = now + m_charge_strike_window;
            if (CMonsterEnemyMemory::target_debug_log())
                Msg("~ [giant_moves] [%s]: charge arrived at %.1f m, strike window open", cName().c_str(), dist);
        }
        return;
    }

    if (m_charge_strike_until || now < m_time_next_charge || !enemy || !enemy->g_Alive())
        return;
    if (is_under_control() || control().is_captured_pure() || !EnemyMan.see_enemy_now())
        return;

    const float dist = enemy->Position().distance_to_xz(Position());
    if (dist < m_charge_dist_min || dist > m_charge_dist_max)
        return;
    if (_abs(enemy->Position().y - Position().y) > m_charge_max_height)
        return;

    // must already face the target (the wind-up clip stops rotation)
    Fvector to_enemy;
    to_enemy.sub(enemy->Position(), Position());
    to_enemy.y = 0.f;
    to_enemy.normalize_safe();
    Fvector forward = Direction();
    forward.y = 0.f;
    forward.normalize_safe();
    if (acosf(std::clamp(forward.dotproduct(to_enemy), -1.f, 1.f)) > m_charge_face_yaw)
        return;

    // clear straight line (level geometry only) at chest height
    Fvector from, to, dir;
    from.set(Position()).y += 1.f;
    to.set(enemy->Position()).y += 1.f;
    dir.sub(to, from);
    const float range = dir.magnitude();
    if (range < EPS_L)
        return;
    dir.div(range);
    collide::rq_result R;
    if (Level().ObjectSpace.RayPick(from, dir, range, collide::rqtStatic, R, this))
        return;

    m_time_next_charge = now + Random.randI(m_charge_delay_min, m_charge_delay_max);
    m_charge_until = now + m_charge_windup + m_charge_max_time;
    m_charge_rush_start = now + m_charge_windup;
    m_charge_locked = false;
    m_charge_dir = to_enemy;
    m_sound_start_threaten.play_at_pos(this, get_head_position(this));

    if (m_charge_windup && m_charge_windup_anim.size())
    {
        IKinematicsAnimated* skel = smart_cast<IKinematicsAnimated*>(Visual());
        const MotionID motion = skel ? skel->ID_Cycle_Safe(m_charge_windup_anim) : MotionID();
        if (motion.valid())
            com_man().seq_run(motion, m_charge_windup_anim_speed);
    }
    if (!m_charge_windup)
        m_charge_rush_start = now; // rush starts on the next update

    if (CMonsterEnemyMemory::target_debug_log())
        Msg("~ [giant_moves] [%s]: charge at [%s], %.1f m (wind-up %u ms, %s)", cName().c_str(), enemy->cName().c_str(), dist, m_charge_windup,
            com_man().seq_active() ? m_charge_windup_anim.c_str() : "no clip");
}

bool CPseudoGigant::nlc_run_target_override(Fvector& position, u32& vertex)
{
    if (!m_charge_until || !m_charge_locked)
        return false;

    position = m_charge_point;
    vertex = m_charge_vertex;
    return true;
}

void CPseudoGigant::HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type, bool draw_hit_marks)
{
    // NLC: the first melee strike after a charge carries its momentum
    if (m_charge_strike_until && time() < m_charge_strike_until && pEntity == EnemyMan.get_enemy())
    {
        m_charge_strike_until = 0;
        fDamage *= m_charge_damage_k;
        impulse *= m_charge_impulse_k;
        if (CMonsterEnemyMemory::target_debug_log())
            Msg("~ [giant_moves] [%s]: charge strike on [%s], damage %.2f, impulse %.0f", cName().c_str(), pEntity->cName().c_str(), fDamage, impulse);
    }

    inherited::HitEntity(pEntity, fDamage, impulse, dir, hit_type, draw_hit_marks);
}

// enemies (or neutrals if enabled) that the stomp may hit; friends and self never
bool CPseudoGigant::is_stomp_victim(const CEntityAlive* entity) const
{
    if (!entity || entity == this || !entity->g_Alive() || entity->getDestroy())
        return false;

    switch (tfGetRelationType(entity))
    {
    case ALife::eRelationTypeEnemy:
    case ALife::eRelationTypeWorstEnemy: return true;
    case ALife::eRelationTypeNeutral: return m_splash_hit_neutrals;
    default: return false;
    }
}

u32 CPseudoGigant::count_crowd() const
{
    u32 count = 0;
    for (const auto& it : const_cast<CPseudoGigant*>(this)->EnemyMemory.get_memory())
    {
        const CEntityAlive* enemy = it.first;
        if (!enemy || !enemy->g_Alive() || enemy->getDestroy())
            continue;
        if (_abs(enemy->Position().y - Position().y) >= m_height_gate)
            continue;
        if (enemy->Position().distance_to_xz(Position()) <= m_crowd_radius)
            ++count;
    }
    return count;
}

// a thrown grenade (no parent, fuse running) in front of the giant within m_grenade_radius
void CPseudoGigant::scan_for_grenades()
{
    if (m_grenade_radius <= 0.f || m_time_next_grenade_stomp > time() || m_time_next_grenade_scan > time())
        return;

    m_time_next_grenade_scan = time() + 250;
    m_grenade_seen = false;

    xr_vector<CObject*> nearest;
    Level().ObjectSpace.GetNearest(nearest, Position(), m_grenade_radius, NULL);
    for (CObject* obj : nearest)
    {
        CGrenade* grenade = smart_cast<CGrenade*>(obj);
        if (!grenade || grenade->H_Parent() || grenade->destroy_time() == 0xffffffff)
            continue;

        Fvector to_grenade;
        to_grenade.sub(grenade->Position(), Position());
        to_grenade.y = 0.f;
        if (to_grenade.magnitude() > EPS_L)
        {
            to_grenade.normalize();
            Fvector dir = Direction();
            dir.y = 0.f;
            dir.normalize_safe();
            if (dir.dotproduct(to_grenade) < 0.f) // behind: the giant does not see it
                continue;
        }

        m_grenade_seen = true;
        m_time_grenade_seen = time();
        if (CMonsterEnemyMemory::target_debug_log())
            Msg("~ [giant_stomp] [%s]: grenade [%s] seen at %.1f m", cName().c_str(), grenade->cName().c_str(), grenade->Position().distance_to(Position()));
        break;
    }
}

void CPseudoGigant::stomp_hit(CEntityAlive* victim, float power, float falloff, ALife::EHitType hit_type, LPCSTR bone)
{
    // push away from the giant and upwards: knockback when alive, ragdoll throw when killed
    Fvector dir;
    dir.sub(victim->Position(), Position());
    dir.y = 0.f;
    if (dir.magnitude() < EPS_L)
        dir.set(Direction());
    dir.normalize_safe();
    dir.y = 1.f;
    dir.normalize();

    float mass = 70.f;
    if (CCharacterPhysicsSupport* phys = victim->character_physics_support())
        if (phys->movement())
            mass = phys->movement()->GetMass();

    IKinematics* kinematics = smart_cast<IKinematics*>(victim->Visual());
    if (!kinematics)
        return;

    NET_Packet l_P;
    SHit HS;
    HS.GenHeader(GE_HIT, victim->ID());
    HS.whoID = ID();
    HS.weaponID = ID();
    HS.dir = dir;
    HS.power = power;
    // aim a body bone when configured (the root bone uses the damage table's default scale)
    u16 bone_id = BI_NONE;
    if (bone && bone[0])
        bone_id = kinematics->LL_BoneID(bone);
    if (bone_id == BI_NONE)
        bone_id = kinematics->LL_GetBoneRoot();

    if (CMonsterEnemyMemory::target_debug_log())
        Msg("~ [giant_stomp] [%s]: hit [%s] power %.2f (%s, bone %s), falloff %.2f", cName().c_str(), victim->cName().c_str(), power,
            hit_type == ALife::eHitTypeExplosion ? "explosion" : "strike", kinematics->LL_BoneName(bone_id), falloff);

    HS.boneID = bone_id;
    HS.p_in_bone_space = Fvector().set(0.f, 0.f, 0.f);
    HS.impulse = m_impulse * falloff * mass;
    HS.hit_type = hit_type;
    HS.Write_Packet(l_P);
    u_EventSend(l_P);
}

void CPseudoGigant::stomp_hit_actor(float falloff, float mode_k)
{
    CActor* pA = Actor();

    if (m_dodge_height > 0.f)
    {
        // NLC: precise dodge - the feet must be high enough above whatever is below at the impact frame
        collide::rq_result R;
        Fvector start = pA->Position();
        start.y += 0.1f;
        float clearance = 3.f;
        if (Level().ObjectSpace.RayPick(start, Fvector().set(0.f, -1.f, 0.f), 3.1f, collide::rqtBoth, R, pA))
            clearance = R.range - 0.1f;

        const bool dodged = !m_kick_hit_jumping_actor && clearance >= m_dodge_height;
        if (CMonsterEnemyMemory::target_debug_log())
            Msg("~ [giant_stomp] [%s]: actor clearance %.2f m (need %.2f) -> %s", cName().c_str(), clearance, m_dodge_height, dodged ? "dodged" : "hit");
        if (dodged)
        {
            // feedback only: a light shake, no damage or slowdown
            Actor()->Cameras().AddCamEffector(xr_new<CMonsterEffectorHit>(m_threaten_effector.ce_time, m_threaten_effector.ce_amplitude * 0.25f,
                                                                          m_threaten_effector.ce_period_number, m_threaten_effector.ce_power * 0.25f));
            return;
        }
    }
    else if (pA->is_jump() && !m_kick_hit_jumping_actor)
        return;

    float hit_value = m_kick_damage * falloff;
    clamp(hit_value, 0.f, 1.f);
    hit_value *= mode_k;
    if (fis_zero(hit_value))
        return;

    // запустить эффектор
    Actor()->Cameras().AddCamEffector(xr_new<CMonsterEffectorHit>(m_threaten_effector.ce_time, m_threaten_effector.ce_amplitude * hit_value, m_threaten_effector.ce_period_number,
                                                                  m_threaten_effector.ce_power * hit_value));
    Actor()->Cameras().AddPPEffector(
        xr_new<CMonsterEffector>(m_threaten_effector.ppi, m_threaten_effector.time, m_threaten_effector.time_attack, m_threaten_effector.time_release, hit_value));

    // развернуть камеру
    if (pA->cam_Active())
    {
        pA->cam_Active()->Move(Random.randI(2) ? kRIGHT : kLEFT, Random.randF(0.3f * hit_value));
        pA->cam_Active()->Move(Random.randI(2) ? kUP : kDOWN, Random.randF(0.3f * hit_value));
    }

    Actor()->lock_accel_for(m_time_kick_actor_slow_down);

    // Нанести хит
    NET_Packet l_P;
    SHit HS;

    HS.GenHeader(GE_HIT, pA->ID()); //	u_EventGen	(l_P,GE_HIT, pA->ID());
    HS.whoID = (ID()); //	l_P.w_u16	(ID());
    HS.weaponID = (ID()); //	l_P.w_u16	(ID());
    HS.dir = (Fvector().set(0.f, 1.f, 0.f)); //	l_P.w_dir	(Fvector().set(0.f,1.f,0.f));
    HS.power = (hit_value); //	l_P.w_float	(m_kick_damage);
    HS.boneID = (smart_cast<IKinematics*>(pA->Visual())->LL_GetBoneRoot()); //	l_P.w_s16	(smart_cast<IKinematics*>(pA->Visual())->LL_GetBoneRoot());
    HS.p_in_bone_space = (Fvector().set(0.f, 0.f, 0.f)); //	l_P.w_vec3	(Fvector().set(0.f,0.f,0.f));
    HS.impulse = (80 * pA->character_physics_support()->movement()->GetMass()); //	l_P.w_float	(20 * pA->movement_control()->GetMass());
    HS.hit_type = (ALife::eHitTypeStrike); //	l_P.w_u16	( u16(ALife::eHitTypeWound) );
    HS.Write_Packet(l_P);
    u_EventSend(l_P);
}

void CPseudoGigant::on_threaten_execute()
{
    const float mode_k = (m_stomp_mode == eStompGrenade) ? m_grenade_damage_k : 1.f;

    // разбросить объекты (NLC: also the splash victims, so the query covers the splash radius)
    m_nearest.clear();
    Level().ObjectSpace.GetNearest(m_nearest, Position(), std::max(15.f, stomp_splash_radius()), NULL);
    for (u32 i = 0; i < m_nearest.size(); i++)
    {
        CPhysicsShellHolder* obj = smart_cast<CPhysicsShellHolder*>(m_nearest[i]);
        if (!obj || !obj->PPhysicsShell() || (obj->spawn_ini() && obj->spawn_ini()->section_exist("ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "ph_heavy") && pSettings->r_bool(obj->cNameSect().c_str(), "ph_heavy")) ||
            (pSettings->line_exist(obj->cNameSect().c_str(), "quest_item") && pSettings->r_bool(obj->cNameSect().c_str(), "quest_item")) || obj->hasFixedBones())
            continue;
        if (obj->Position().distance_to(Position()) > 15.f)
            continue;

        Fvector dir;
        Fvector pos;
        pos.set(obj->Position());
        pos.y += 2.f;
        dir.sub(pos, Position());
        dir.normalize();
        obj->m_pPhysicsShell->applyImpulse(dir, 20 * obj->m_pPhysicsShell->getMass());
    }

    // играть звук
    Fvector pos;
    pos.set(Position());
    pos.y += 0.1f;
    m_sound_threaten_hit.play_at_pos(this, pos);

    g_pGamePersistent->GrassBendersAddExplosion(ID(), pos, {0.f, -99.f, 0.f}, 1.33f, 5.0f, ps_ssfx_grass_interactive.w, 20);

    // играть партиклы
    PlayParticles(m_kick_particles, pos, Direction());

    // NLC: splash on other creatures (enemies, optionally neutrals) within m_splash_radius
    u32 victims = 0;
    if (m_splash_radius > 0.f)
    {
        for (CObject* object : m_nearest)
        {
            CEntityAlive* victim = smart_cast<CEntityAlive*>(object);
            if (!victim || victim == Actor() || !is_stomp_victim(victim))
                continue;
            if (_abs(victim->Position().y - Position().y) >= m_height_gate)
                continue;

            const float dist = victim->Position().distance_to_xz(Position());
            if (dist > stomp_splash_radius())
                continue;

            const float falloff = 1.f - dist / stomp_splash_radius();
            float base = m_kick_damage * falloff;
            clamp(base, 0.f, 1.f);
            base *= mode_k;
            if (fis_zero(base))
                continue;

            CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(victim);
            CBaseMonster* monster = smart_cast<CBaseMonster*>(victim);
            const float power = base * (stalker ? m_damage_k_stalker : m_damage_k_monster);

            if (stalker)
                stomp_hit(victim, power, falloff, m_hit_type_stalker, nullptr);
            else
                stomp_hit(victim, power, falloff, m_hit_type_monster, m_hit_bone_monster.c_str());
            ++victims;

            if (m_stagger_min > 0.f && base >= m_stagger_min)
            {
                if (stalker)
                    stalker->force_critical_wound(Random.randI(2) ? critical_wound_type_leg_left : critical_wound_type_leg_right);
                else if (monster)
                    monster->nlc_stagger();
            }

            if (monster && m_slow_time)
                monster->nlc_apply_move_slow(m_slow_k * falloff, m_slow_time);
        }
    }

    if (CMonsterEnemyMemory::target_debug_log())
        Msg("~ [giant_stomp] [%s]: execute, %u splash victim(s)", cName().c_str(), victims);

    // the actor: as the target (original falloff over HugeKick_MinMaxDist), or inside the splash
    CActor* pA = Actor();
    if (!pA || !pA->g_Alive())
        return;
    if (!m_hit_actor_in_splash && pA != EnemyMan.get_enemy())
        return;
    if (!is_relation_enemy(pA))
        return;
    if (m_splash_radius > 0.f && _abs(pA->Position().y - Position().y) >= m_height_gate)
        return;

    const float dist_to_actor = pA->Position().distance_to(Position());
    float falloff = 1.f - dist_to_actor / m_threaten_dist_max;
    if (pA != EnemyMan.get_enemy())
        falloff = (m_splash_radius > 0.f) ? 1.f - pA->Position().distance_to_xz(Position()) / stomp_splash_radius() : 0.f;
    if (falloff <= 0.f)
        return;

    stomp_hit_actor(falloff, mode_k);
}

void CPseudoGigant::HitEntityInJump(const CEntity* pEntity)
{
    SAAParam& params = anim().AA_GetParams("jump_attack_1");
    HitEntity(pEntity, params.hit_power, params.impulse, params.impulse_dir);
}

void CPseudoGigant::TranslateActionToPathParams()
{
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
