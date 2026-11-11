#include "stdafx.h"
#include "../../fl_hook.h"
#include "control_run_attack.h"
#include "BaseMonster/base_monster.h"
#include "monster_velocity_space.h"
#include "control_animation_base.h"
#include "control_direction_base.h"
#include "control_movement_base.h"
#include "anomaly_detector.h"

void CControlRunAttack::load(LPCSTR section)
{
    read_distance(section, "Run_Attack_Dist", m_min_dist, m_max_dist);
    read_delay(section, "Run_Attack_Delay", m_min_delay, m_max_delay);
    m_cooldown = READ_IF_EXISTS(pSettings, r_u32, section, "Run_Attack_Cooldown", 2200); // NLC
    m_overshoot = READ_IF_EXISTS(pSettings, r_float, section, "Run_Attack_Overshoot", -1.f); // NLC
}

void CControlRunAttack::reinit()
{
    CControl_ComCustom<>::reinit();

    m_time_next_attack = 0;
    m_nlc_force_until = 0;
    m_nlc_anomaly_commit = false;
    m_cycle_state = -1; // the visual may change on respawn
}

void CControlRunAttack::activate()
{
    // NLC: the cooldown is armed by a lunge that really starts. It used to be armed in check_start_conditions, before the
    // species gate (CBaseMonster::check_start_conditions) could refuse, so a refused lunge locked the control out for 2.2 s.
    // A charge that ends early must still not re-lock at once.
    m_time_next_attack = Device.dwTimeGlobal + m_cooldown;
    m_nlc_force_until = 0;

    // NLC: lunge speed (run_attack_haste_k, evasion pass; 1 = stock)
    if (m_object->nlc_run_attack_haste_k() > 1.f)
        m_object->nlc_set_dodge(m_object->nlc_run_attack_haste_k(), 2000);

    m_man->capture_pure(this);
    m_man->subscribe(this, ControlCom::eventAnimationEnd);
    m_man->subscribe(this, ControlCom::eventAnimationStart);

    m_man->path_stop(this);
    m_man->move_stop(this);

    //////////////////////////////////////////////////////////////////////////

    SControlDirectionData* ctrl_dir = (SControlDirectionData*)m_man->data(this, ControlCom::eControlDir);
    VERIFY(ctrl_dir);
    ctrl_dir->heading.target_speed = 3.f;
    ctrl_dir->heading.target_angle = m_man->direction().angle_to_target(m_object->EnemyMan.get_enemy()->Position());

    //////////////////////////////////////////////////////////////////////////

    SControlAnimationData* ctrl_anim = (SControlAnimationData*)m_man->data(this, ControlCom::eControlAnimation);
    VERIFY(ctrl_anim);

    ctrl_anim->global.set_motion(smart_cast<IKinematicsAnimated*>(m_object->Visual())->ID_Cycle_Safe("stand_attack_run_0"));
    ctrl_anim->global.actual = false;
}

void CControlRunAttack::on_release()
{
    if (m_nlc_anomaly_commit)
    {
        m_nlc_anomaly_commit = false;
        m_object->anomaly_detector().nlc_resume(); // NLC: each anomaly is avoided again once it is out of it
    }
    if (m_object->nlc_run_attack_haste_k() > 1.f)
        m_object->nlc_clear_dodge(); // NLC

    m_man->unlock_owned(this, ControlCom::eControlPath); // NLC: owner-safe
    m_man->release_pure_owned(this);
    m_man->unsubscribe(this, ControlCom::eventAnimationEnd);
    m_man->unsubscribe(this, ControlCom::eventAnimationStart);
}

bool CControlRunAttack::check_start_conditions()
{
    // Stock is too strict for OGSR boars: the path controller owns pure capture during the chase,
    // the face cone is ~30 deg and the speed must match run speed within 2. The ram has to start
    // while charging toward the enemy without taking over standing melee (paws).
    if (is_active())
    {
        fl_hook::boar_result(fl_hook::eBoarActive);
        return false;
    }

    // NLC: never steal the controls from another special control (a jump / pounce, the vampire grab, a stagger):
    // capture_pure takes them without asking (nlc-3.589.32 crash: a lunge during a pounce). The chase's base path
    // builder and animation are base controllers and do not count as captured.
    if (m_object->com_man().is_jumping() || m_man->is_captured(ControlCom::eControlAnimation))
    {
        fl_hook::boar_result(fl_hook::eBoarActive);
        return false;
    }

    // NLC: some models (e.g. the pseudogiant) have no run-attack cycle; LL_PlayCycle then returns a null blend
    if (m_cycle_state < 0)
    {
        IKinematicsAnimated* ka = smart_cast<IKinematicsAnimated*>(m_object->Visual());
        m_cycle_state = (ka && ka->ID_Cycle_Safe("stand_attack_run_0").valid()) ? 1 : 0;
        if (!m_cycle_state)
            Msg("! [run_attack] [%s]: no stand_attack_run_0 in the model, run attack disabled", m_object->cName().c_str());
    }
    if (!m_cycle_state)
        return false;

    const CEntityAlive* enemy = m_object->EnemyMan.get_enemy();
    if (!enemy)
    {
        fl_hook::boar_result(fl_hook::eBoarNoEnemy);
        return false;
    }

    const u32 now = Device.dwTimeGlobal;
    const bool forced = now < m_nlc_force_until; // NLC polish pass (bloodsucker back-off lunge)
    if (m_time_next_attack > now && !forced)
    {
        fl_hook::boar_result(fl_hook::eBoarCooldown);
        return false;
    }

    Fvector to_enemy;
    to_enemy.sub(enemy->Position(), m_object->Position());
    const float dist = to_enemy.magnitude();
    fl_hook::boar_last_dist = dist;

    // Run_Attack_Dist as configured, a floor only for a broken config and a hard cap
    float dmin = m_min_dist;
    float dmax = m_max_dist;
    if (dmin < 2.5f)
        dmin = 2.5f;
    if (dmax < dmin + 0.5f)
        dmax = dmin + 3.0f;
    if (dmax > 10.0f)
        dmax = 10.0f;
    if (forced)
        dmin = std::min(dmin, m_nlc_force_min);
    if (dist < dmin || dist > dmax)
    {
        fl_hook::boar_result(fl_hook::eBoarDistance);
        return false;
    }

    // ~55 deg face cone on XZ from the object's forward vector
    const Fvector& fwd = m_object->XFORM().k;
    const float fl2 = fwd.x * fwd.x + fwd.z * fwd.z;
    const float tl2 = to_enemy.x * to_enemy.x + to_enemy.z * to_enemy.z;
    const float dot = fwd.x * to_enemy.x + fwd.z * to_enemy.z;
    if (fl2 < 1e-8f || tl2 < 1e-8f || dot <= 0.f || dot * dot < 0.329f * fl2 * tl2)
    {
        fl_hook::boar_result(fl_hook::eBoarNotFacing);
        return false;
    }

    // real movement required, so standing combat stays on eAnimAttack (paws)
    const float vcur = m_man->movement().velocity_current();
    fl_hook::boar_last_vel = vcur;
    if (vcur < 2.0f && !forced)
    {
        fl_hook::boar_result(fl_hook::eBoarStanding);
        return false;
    }

    // NLC: the 2.2 s cooldown is armed in activate() (see there)

    fl_hook::boar_result(fl_hook::eBoarOk);
    return true;
}

void CControlRunAttack::on_event(ControlCom::EEventType type, ControlCom::IEventData* dat)
{
    switch (type)
    {
    case ControlCom::eventAnimationEnd:
        m_time_next_attack = time() + Random.randI(m_min_delay, m_max_delay);
        m_man->notify(ControlCom::eventRunAttackEnd, 0);
        break;
    case ControlCom::eventAnimationStart: // handle blend params
    {
        // set animation speed
        VERIFY((SControlAnimationData*)m_man->data(this, ControlCom::eControlAnimation));

        CBlend* blend = m_man->animation().current_blend();
        VERIFY(blend);
        if (!blend || !m_object->EnemyMan.get_enemy())
        {
            if (CBaseMonster::nlc_evade_debug())
                Msg("~ [run_attack] [%s]: lunge ends at its start: %s", m_object->cName().c_str(), blend ? "no enemy" : "no animation blend");
            m_man->notify(ControlCom::eventRunAttackEnd, 0);
            break;
        }

        // animation time
        float anim_time = blend->timeTotal / blend->speed;

        // run velocity
        u32 velocity_mask = MonsterMovement::eVelocityParameterRunNormal;
        SVelocityParam& velocity = m_object->move().get_velocity(velocity_mask);

        // distance
        float path_dist = anim_time * velocity.velocity.linear;
        if (m_object->nlc_run_attack_haste_k() > 1.f)
            path_dist *= m_object->nlc_move_speed_k(); // NLC: the line matches the boosted lunge speed

        Fvector dir;
        dir.sub(m_object->EnemyMan.get_enemy()->Position(), m_object->Position());
        dir.normalize_safe();

        // NLC: Run_Attack_Overshoot: stop shortly past the enemy instead of running the whole clip on (the long
        // loop back afterwards looked like circling away)
        if (m_overshoot >= 0.f)
            path_dist = std::min(path_dist, std::max(m_object->EnemyMan.get_enemy()->Position().distance_to(m_object->Position()) + m_overshoot, 1.f));

        // NLC polish pass 4 / 8: a refused line (an obstacle, a restriction or the ai-map edge past the enemy) ended the
        // lunge while its clip went on playing in place. The line is flat with its height following the slope to the
        // enemy (along the 3D direction its end lay 2-3 m under or over the ground on a slope), and it is shortened in
        // steps: the longest line that fits is taken, so the run matches the clip as far as the ground allows.
        const u32 line_mask = velocity_mask | MonsterMovement::eVelocityParameterStand;
        const Fvector me = m_object->Position();
        const Fvector ep = m_object->EnemyMan.get_enemy()->Position();
        Fvector flat = dir;
        flat.y = 0.f;
        if (flat.square_magnitude() > EPS_L)
            flat.normalize();
        else
            flat = dir;
        const float to_enemy = me.distance_to_xz(ep);
        auto line_end = [&](float d) {
            Fvector pt;
            pt.mad(me, flat, d);
            pt.y = me.y + (ep.y - me.y) * (to_enemy > 0.1f ? std::min(d / to_enemy, 1.f) : 1.f);
            return pt;
        };
        const float full = path_dist;
        float lens[12];
        u32 n = 0;
        lens[n++] = full;
        for (float d = full - 1.5f; d > to_enemy + 1.f && n < 9; d -= 1.5f)
            lens[n++] = d;
        if (to_enemy < full - 0.1f)
            lens[n++] = to_enemy;
        lens[n++] = 0.5f * std::min(full, to_enemy);
        // NLC polish pass 9: a committed lunge. With the way to the target clear of the anomalies it has noticed, it
        // may ignore them for the rest of the line (lunge_anomaly_commit): a sidestep sends it on into one. Only past
        // the target: an anomaly between it and the target still stops the lunge short. (Pass 10: "clear" is tested
        // against the anomaly volumes, not by a path line: a target at the edge of one is inside its restriction margin.)
        const u32 avoided = m_object->anomaly_detector().nlc_avoided();
        if (avoided && !m_nlc_anomaly_commit && m_object->nlc_lunge_anomaly_commit() > 0.f && to_enemy >= 0.7f && to_enemy < full &&
            m_object->anomaly_detector().nlc_segment_clear(me, line_end(to_enemy)) && ::Random.randF() < m_object->nlc_lunge_anomaly_commit())
        {
            const bool blocked = !m_man->build_path_line(this, line_end(full), u32(-1), line_mask);
            m_object->anomaly_detector().nlc_suspend();
            m_nlc_anomaly_commit = true;
            if (CBaseMonster::nlc_evade_debug())
                Msg("~ [run_attack] [%s]: committed lunge: %u noticed anomalies ignored past the target (the full line was %s)", m_object->cName().c_str(), avoided,
                    blocked ? m_man->path_builder().nlc_special_fail_text() : "clear anyway");
        }
        bool built = false;
        const char* first_fail = "";
        for (u32 i = 0; i < n && !built; ++i)
        {
            if (lens[i] < 0.7f)
                break;
            built = m_man->build_path_line(this, line_end(lens[i]), u32(-1), line_mask);
            if (!built && i == 0)
                first_fail = m_man->path_builder().nlc_special_fail_text();
            if (built && CBaseMonster::nlc_evade_debug())
            {
                if (i == 0)
                    Msg("~ [run_attack] [%s]: lunge line %.1f m (full), enemy at %.1f m", m_object->cName().c_str(), full, to_enemy);
                else
                    Msg("~ [run_attack] [%s]: lunge line %.1f m of %.1f m (try %u), enemy at %.1f m; the full line: %s", m_object->cName().c_str(), lens[i], full, i + 1,
                        to_enemy, first_fail);
            }
        }
        if (!built && CBaseMonster::nlc_evade_debug())
        {
            Fvector fwd = m_object->Direction();
            fwd.y = 0.f;
            Msg("~ [run_attack] [%s]: every lunge line refused (%.1f m: %s; shortest: %s); heading %.0f deg off the line, speed %.1f m/s, enemy at %.1f m, %.1f m %s",
                m_object->cName().c_str(), full, first_fail, m_man->path_builder().nlc_special_fail_text(), rad2deg(angle_between_vectors(fwd, flat)),
                m_man->movement().velocity_current(), to_enemy, _abs(ep.y - me.y), ep.y > me.y ? "up" : "down");
        }
        if (!built)
        {
            if (CBaseMonster::nlc_evade_debug())
                Msg("~ [run_attack] [%s]: no lunge line at all, the lunge ends", m_object->cName().c_str());
            m_man->notify(ControlCom::eventRunAttackEnd, 0);
        }
        else
        {
            // enable path
            SControlPathBuilderData* ctrl_path = (SControlPathBuilderData*)m_man->data(this, ControlCom::eControlPath);
            VERIFY(ctrl_path);
            ctrl_path->enable = true;

            m_man->lock(this, ControlCom::eControlPath);

            SControlMovementData* ctrl_move = (SControlMovementData*)m_man->data(this, ControlCom::eControlMovement);
            VERIFY(ctrl_move);
            ctrl_move->velocity_target = velocity.velocity.linear;
            ctrl_move->acc = flt_max;
        }
    }
    break;
    }
}
