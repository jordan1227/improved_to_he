////////////////////////////////////////////////////////////////////////////
//	Module 		: base_monster_feel.cpp
//	Created 	: 26.05.2003
//  Modified 	: 26.05.2003
//	Author		: Serge Zhem
//	Description : Visibility and look for all the biting monsters
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "base_monster.h"
#include "../../../actor.h"
#include "../../../ActorEffector.h"
#include "../ai_monster_effector.h"
#include "../../../hudmanager.h"
#include "clsid_game.h"
#include "../../../../Include/xrRender/KinematicsAnimated.h"
#include "../../../sound_player.h"
#include "../../../level.h"
#include "../../../nlc_stealth.h" // NLC: stealth diagnostics
#include "../../../script_callback_ex.h"
#include "../../../script_game_object.h"
#include "../../../game_object_space.h"
#include "../../../ai_monster_space.h"
#include "../control_animation_base.h"
#include "../../../UIGameCustom.h"
#include "../../../UI/UIStatic.h"
#include "../../../ai_object_location.h"

#include "../../../ActorEffector.h"
#include "../../../../xr_3da/CameraBase.h"

void CBaseMonster::feel_sound_new(CObject* who, int eType, CSound_UserDataPtr user_data, const Fvector& Position, float power)
{
    if (!g_Alive())
        return;

    // ignore my sounds
    if (this == who)
        return;

    // NLC: the enemy fired a weapon: a zigzagging charge switches side (no-op unless a zigzag leg is running)
    if (who && m_nlc_evade.zz_on && EnemyMan.get_enemy() && who->ID() == EnemyMan.get_enemy()->ID() && (eType & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING)
        nlc_zz_on_fire();

    if (user_data)
        user_data->accept(sound_user_data_visitor());

    // ignore unknown sounds
    if (eType == 0xffffffff)
        return;

    // NLC M4: outfit noise, surface and rain masking of the actor's quiet sounds, walls (identity by default)
    if (nlc_stealth::g_actor_sound_factor && g_actor && who == g_actor)
        power *= nlc_stealth::actor_sound_factor(eType, this, Position);

    // ignore distant sounds
    Fvector center;
    Center(center);
    float dist = center.distance_to(Position);
    // NLC: stealth diagnostics for sounds owned by the actor (nlc_stealth; off unless a watch is set)
    const bool nlc_log = nlc_stealth::g_track && g_actor && who == g_actor;
    if (dist > db().m_max_hear_dist)
    {
        if (nlc_log)
            nlc_stealth::on_monster_sound(this, eType, dist, power, db().m_fSoundThreshold, false, false, "max_hear_dist");
        return;
    }

    // ignore sounds if not from enemies and not help sounds
    CEntityAlive* entity = smart_cast<CEntityAlive*>(who);

    // ignore sound if enemy drop a weapon on death
    if (!entity && ((eType & SOUND_TYPE_ITEM_HIDING) == SOUND_TYPE_ITEM_HIDING))
        return;

    if (entity && (!EnemyMan.is_enemy(entity)))
    {
        SoundMemory.check_help_sound(eType, entity->ai_location().level_vertex_id());
        return;
    }

    // if ((eType & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING) power = 1.f;

    // NLC: named for the diagnostics; the shooter range gate is a species key (near_hit_shooter_max_distance, off by default)
    const bool near_hit = ((eType & SOUND_TYPE_WEAPON_BULLET_HIT) == SOUND_TYPE_WEAPON_BULLET_HIT) && (dist < 2.f) && nlc_stealth::monster_near_hit_allowed(this, who);
    if (near_hit)
        HitMemory.add_hit(who, eSideFront);

    if (nlc_log) // NLC
        nlc_stealth::on_monster_sound(this, eType, dist, power, db().m_fSoundThreshold, power >= db().m_fSoundThreshold, near_hit, nullptr);

    // execute callback
    sound_callback(who, eType, Position, power);

    // NLC: a besieged enemy's sound renews the siege lock (faint shots only partly); before the faint-shot return
    if (m_nlc_siege.active && power >= db().m_fSoundThreshold)
        nlc_siege_on_noise(who, eType, Position, power);

    // NLC M3: a faint heard actor shot only sends the monster to look (nlc_stealth monster_shot_alert_pow / species shot_alert_pow)
    if (g_actor && who == g_actor && power >= db().m_fSoundThreshold && nlc_stealth::monster_faint_shot(this, eType, Position, power))
        return;

    // register in sound memory
    if (power >= db().m_fSoundThreshold)
    {
        SoundMemory.HearSound(who, eType, Position, power, Device.dwTimeGlobal);
    }
}
#define MAX_LOCK_TIME 2.f

void CBaseMonster::HitEntity(const CEntity* pEntity, float fDamage, float impulse, Fvector& dir, ALife::EHitType hit_type, bool draw_hit_marks)
{
    if (!g_Alive())
        return;
    if (!pEntity || pEntity->getDestroy())
        return;

    if (!EnemyMan.get_enemy())
        return;

    if (EnemyMan.get_enemy() == pEntity)
    {
        Fvector position_in_bone_space;
        position_in_bone_space.set(0.f, 0.f, 0.f);

        // перевод из локальных координат в мировые вектора направления импульса
        Fvector hit_dir;
        XFORM().transform_dir(hit_dir, dir);
        hit_dir.normalize();

        CEntity* pEntityNC = const_cast<CEntity*>(pEntity);
        VERIFY(pEntityNC);

        NET_Packet l_P;
        SHit HS;
        HS.GenHeader(GE_HIT, pEntityNC->ID()); //		u_EventGen	(l_P,GE_HIT, pEntityNC->ID());
        HS.whoID = (ID()); //		l_P.w_u16	(ID());
        HS.weaponID = (ID()); //		l_P.w_u16	(ID());
        HS.dir = (hit_dir); //		l_P.w_dir	(hit_dir);
        HS.power = (fDamage); //		l_P.w_float	(fDamage);
        HS.boneID = (smart_cast<IKinematics*>(pEntityNC->Visual())->LL_GetBoneRoot()); //		l_P.w_s16	(smart_cast<IKinematics*>(pEntityNC->Visual())->LL_GetBoneRoot());
        HS.p_in_bone_space = (position_in_bone_space); //		l_P.w_vec3	(position_in_bone_space);
        HS.impulse = (impulse); //		l_P.w_float	(impulse);
        HS.hit_type = hit_type; //		l_P.w_u16	( u16(ALife::eHitTypeWound) );
        HS.Write_Packet(l_P);
        u_EventSend(l_P);

        if (pEntityNC == Actor() && draw_hit_marks)
        {
            START_PROFILE("BaseMonster/Animation/HitEntity");
            SDrawStaticStruct* s = HUD().GetUI()->UIGame()->AddCustomStatic("monster_claws", false);
            s->m_endTime = Device.fTimeGlobal + 3.0f; // 3sec

            float h1, p1;
            Device.vCameraDirection.getHP(h1, p1);

            Fvector hd = hit_dir;
            hd.mul(-1);
            float d = -h1 + hd.getH();
            s->wnd()->SetHeading(d);
            s->wnd()->SetHeadingPivot(Fvector2().set(256, 512), Fvector2().set(0, 0), false);
            STOP_PROFILE;

            // SetAttackEffector			();

            float time_to_lock = fDamage * MAX_LOCK_TIME;
            clamp(time_to_lock, 0.f, MAX_LOCK_TIME);
            Actor()->lock_accel_for(int(time_to_lock * 1000));

            //////////////////////////////////////////////////////////////////////////
            //
            //////////////////////////////////////////////////////////////////////////

            CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effBigMonsterHit);
            if (!ce)
            {
                const shared_str& eff_sect = pSettings->r_string(cNameSect(), "actor_hit_effect");
                if (eff_sect.c_str())
                {
                    int id = -1;
                    Fvector cam_pos, cam_dir, cam_norm;
                    Actor()->cam_Active()->Get(cam_pos, cam_dir, cam_norm);
                    cam_dir.normalize_safe();
                    dir.normalize_safe();

                    float ang_diff = angle_difference(cam_dir.getH(), dir.getH());
                    Fvector cp;
                    cp.crossproduct(cam_dir, dir);
                    bool bUp = (cp.y > 0.0f);

                    Fvector cross;
                    cross.crossproduct(cam_dir, dir);
                    VERIFY(ang_diff >= 0.0f && ang_diff <= PI);

                    float _s1 = PI_DIV_8;
                    float _s2 = _s1 + PI_DIV_4;
                    float _s3 = _s2 + PI_DIV_4;
                    float _s4 = _s3 + PI_DIV_4;

                    if (ang_diff <= _s1)
                    {
                        id = 2;
                    }
                    else
                    {
                        if (ang_diff > _s1 && ang_diff <= _s2)
                        {
                            id = (bUp) ? 5 : 7;
                        }
                        else if (ang_diff > _s2 && ang_diff <= _s3)
                        {
                            id = (bUp) ? 3 : 1;
                        }
                        else if (ang_diff > _s3 && ang_diff <= _s4)
                        {
                            id = (bUp) ? 4 : 6;
                        }
                        else if (ang_diff > _s4)
                        {
                            id = 0;
                        }
                        else
                        {
                            VERIFY(0);
                        }
                    }

                    string64 sect_name;

                    sprintf_s(sect_name, "%s_%d", eff_sect.c_str(), id);
                    AddEffector(Actor(), effBigMonsterHit, sect_name, fDamage);
                }
            }
            //////////////////////////////////////////////////////////////////////////
        }

        Morale.on_attack_success();

        m_time_last_attack_success = Device.dwTimeGlobal;
    }
}

BOOL CBaseMonster::feel_vision_isRelevant(CObject* O)
{
    if (!g_Alive())
        return FALSE;
    if (0 == smart_cast<CEntity*>(O))
        return FALSE;

    if ((O->spatial.type & STYPE_VISIBLEFORAI) != STYPE_VISIBLEFORAI)
        return FALSE;

    // если спит, то ничего не видит
    if (m_bSleep)
        return FALSE;

    // если не враг - не видит
    CEntityAlive* entity = smart_cast<CEntityAlive*>(O);
    if (entity && entity->g_Alive())
    {
        if (!EnemyMan.is_enemy(entity))
        {
            // если видит друга - проверить наличие у него врагов
            CBaseMonster* monster = smart_cast<CBaseMonster*>(entity);
            // NLC: pack sharing gate and visible-body exclusion (nlc_stealth pack_gate)
            if (monster && !m_skip_transfer_enemy && !nlc_stealth::monster_pack_share(this, monster))
                EnemyMan.transfer_enemy(monster);
            return FALSE;
        }
    }

    return TRUE;
}

void CBaseMonster::HitSignal(float amount, Fvector& vLocalDir, CObject* who, s16 element)
{
    if (!g_Alive())
        return;

    feel_sound_new(who, SOUND_TYPE_WEAPON_SHOOTING, 0, who->Position(), 1.f);
    if (g_Alive())
        sound().play(MonsterSound::eMonsterSoundTakeDamage);

    if (element < 0)
        return;

    // Определить направление хита (перед || зад || лево || право)
    float yaw, pitch;
    vLocalDir.getHP(yaw, pitch);

    yaw = angle_normalize(yaw);

    EHitSide hit_side = eSideFront;
    if ((yaw >= PI_DIV_4) && (yaw <= 3 * PI_DIV_4))
        hit_side = eSideLeft;
    else if ((yaw >= 3 * PI_DIV_4) && (yaw <= 5 * PI_DIV_4))
        hit_side = eSideBack;
    else if ((yaw >= 5 * PI_DIV_4) && (yaw <= 7 * PI_DIV_4))
        hit_side = eSideRight;

    anim().FX_Play(hit_side, 1.0f);

    HitMemory.add_hit(who, hit_side);

    // NLC: a hit by the enemy switches the zigzag side (feel_sound_new above already did for a shot sound; the call is idempotent)
    if (who && EnemyMan.get_enemy() && who->ID() == EnemyMan.get_enemy()->ID())
        nlc_zz_on_fire();

    // NLC: a hit in the back from close range provokes (threat only, no health), so a flanking
    // knife pulls aggro even when it barely hurts
    if (hit_side == eSideBack && who && who != this && EnemyMemory.back_hit_threat() > 0.f && who->Position().distance_to(Position()) <= EnemyMemory.back_hit_max_dist())
    {
        HitMemory.add_damage(who, EnemyMemory.back_hit_threat());
        if (CMonsterEnemyMemory::target_debug_log())
            Msg("~ [monster_target] [%s]: back hit by [%s]", cName().c_str(), who->cName().c_str());
    }

    Morale.on_hit();

    callback(GameObject::eHit)(lua_game_object(), amount, vLocalDir, smart_cast<const CGameObject*>(who)->lua_game_object(), element);

    // если нейтрал - добавить как врага
    CEntityAlive* obj = smart_cast<CEntityAlive*>(who);
    if (obj && (tfGetRelationType(obj) == ALife::eRelationTypeNeutral))
    {
        nlc_stealth::set_monster_add_source("hit_neutral"); // NLC: diagnostics tag
        EnemyMan.add_enemy(obj);
        nlc_stealth::set_monster_add_source(nullptr);
    }
}

void CBaseMonster::SetAttackEffector()
{
    CActor* pA = smart_cast<CActor*>(Level().CurrentEntity());
    if (pA)
    {
        Actor()->Cameras().AddCamEffector(xr_new<CMonsterEffectorHit>(db().m_attack_effector.ce_time, db().m_attack_effector.ce_amplitude, db().m_attack_effector.ce_period_number,
                                                                      db().m_attack_effector.ce_power));
        Actor()->Cameras().AddPPEffector(
            xr_new<CMonsterEffector>(db().m_attack_effector.ppi, db().m_attack_effector.time, db().m_attack_effector.time_attack, db().m_attack_effector.time_release));
    }
}

void CBaseMonster::Hit_Psy(CObject* object, float value)
{
    NET_Packet P;
    SHit HS;
    HS.GenHeader(GE_HIT, object->ID()); //					//	u_EventGen		(P,GE_HIT, object->ID());				//
    HS.whoID = (ID()); // own		//	P.w_u16			(ID());									// own
    HS.weaponID = (ID()); // own		//	P.w_u16			(ID());									// own
    HS.dir = (Fvector().set(0.f, 1.f, 0.f)); // direction	//	P.w_dir			(Fvector().set(0.f,1.f,0.f));			// direction
    HS.power = (value); // hit value	//	P.w_float		(value);								// hit value
    HS.boneID = (BI_NONE); // bone		//	P.w_s16			(BI_NONE);								// bone
    HS.p_in_bone_space = (Fvector().set(0.f, 0.f, 0.f)); //	P.w_vec3		(Fvector().set(0.f,0.f,0.f));
    HS.impulse = (0.f); //	P.w_float		(0.f);
    HS.hit_type = (ALife::eHitTypeTelepatic); //	P.w_u16			(u16(ALife::eHitTypeTelepatic));
    HS.Write_Packet(P);
    u_EventSend(P);
}

void CBaseMonster::Hit_Wound(CObject* object, float value, const Fvector& dir, float impulse)
{
    NET_Packet P;
    SHit HS;
    HS.GenHeader(GE_HIT, object->ID()); //	u_EventGen	(P,GE_HIT, object->ID());
    HS.whoID = (ID()); //	P.w_u16		(ID());
    HS.weaponID = (ID()); //	P.w_u16		(ID());
    HS.dir = (dir); //	P.w_dir		(dir);
    HS.power = (value); //	P.w_float	(value);
    HS.boneID = (smart_cast<IKinematics*>(object->Visual())->LL_GetBoneRoot()); //	P.w_s16		(smart_cast<IKinematics*>(object->Visual())->LL_GetBoneRoot());
    HS.p_in_bone_space = (Fvector().set(0.f, 0.f, 0.f)); //	P.w_vec3	(Fvector().set(0.f,0.f,0.f));
    HS.impulse = (impulse); //	P.w_float	(impulse);
    HS.hit_type = (ALife::eHitTypeWound); //	P.w_u16		(u16(ALife::eHitTypeWound));
    HS.Write_Packet(P);
    u_EventSend(P);
}

bool CBaseMonster::critical_wound_external_conditions_suitable()
{
    if (!control().check_start_conditions(ControlCom::eControlSequencer))
        return false;

    if (!anim().IsStandCurAnim())
        return false;

    return true;
}

void CBaseMonster::critical_wounded_state_start()
{
    VERIFY(m_critical_wound_type != u32(-1));

    LPCSTR anim = 0;
    switch (m_critical_wound_type)
    {
    case critical_wound_type_head: anim = m_critical_wound_anim_head; break;
    case critical_wound_type_torso: anim = m_critical_wound_anim_torso; break;
    case critical_wound_type_legs: anim = m_critical_wound_anim_legs; break;
    }

    VERIFY(anim);
    com_man().critical_wound(anim);
}

bool CBaseMonster::nlc_stagger(float speed_k)
{
    if (!g_Alive() || critically_wounded())
        return false;

    if (m_nlc_stagger_state < 0)
    {
        // read the keys directly: species with critical_wound_threshold < 0 never load them
        m_nlc_stagger_state = 0;
        IKinematicsAnimated* skel = smart_cast<IKinematicsAnimated*>(Visual());
        for (LPCSTR key : {"critical_wound_anim_torso", "critical_wound_anim_legs", "critical_wound_anim_head"})
        {
            LPCSTR anim = READ_IF_EXISTS(pSettings, r_string, cNameSect(), key, nullptr);
            if (anim && skel && skel->ID_Cycle_Safe(anim).valid())
            {
                m_nlc_stagger_anim = anim;
                m_nlc_stagger_state = 1;
                break;
            }
        }
    }

    if (m_nlc_stagger_state != 1)
        return false;

    if (!critical_wound_external_conditions_suitable() || !control().check_start_conditions(ControlCom::eComCriticalWound))
        return false;

    // marks the monster as critically wounded until CControlCriticalWound::on_release clears it
    m_critical_wound_type = critical_wound_type_torso;
    com_man().critical_wound(m_nlc_stagger_anim, speed_k);
    return true;
}

void CBaseMonster::nlc_stagger_repeat(u32 count, float speed_k)
{
    m_nlc_stagger_left = count;
    m_nlc_stagger_speed = std::clamp(speed_k, 0.3f, 3.f);
    m_nlc_stagger_until = Device.dwTimeGlobal + 1500;
    nlc_update_stagger_repeat();
}

// schedule update: start the next stagger once the previous one has ended
void CBaseMonster::nlc_update_stagger_repeat()
{
    if (!m_nlc_stagger_left)
        return;
    if (!g_Alive())
    {
        m_nlc_stagger_left = 0;
        return;
    }
    if (critically_wounded())
    {
        m_nlc_stagger_until = Device.dwTimeGlobal + 1500; // the previous stagger is still playing
        return;
    }
    if (nlc_stagger(m_nlc_stagger_speed))
    {
        --m_nlc_stagger_left;
        m_nlc_stagger_until = Device.dwTimeGlobal + 1500;
    }
    else if (m_nlc_stagger_state == 0 || Device.dwTimeGlobal > m_nlc_stagger_until)
        m_nlc_stagger_left = 0; // no usable clip, or it could not start in time (a late stagger looks random)
}

void CBaseMonster::nlc_apply_move_slow(float k, u32 time_ms)
{
    clamp(k, 0.f, 0.9f);
    if (fis_zero(k) || !time_ms)
        return;

    // keep whichever slow is stronger right now (slow part only, without bonus/haste)
    const u32 now = Device.dwTimeGlobal;
    if (now < m_nlc_slow_end && m_nlc_slow_end > m_nlc_slow_start)
    {
        const float left = float(m_nlc_slow_end - now) / float(m_nlc_slow_end - m_nlc_slow_start);
        if (m_nlc_slow_k * std::min(1.f, left / 0.25f) >= k)
            return;
    }

    m_nlc_slow_k = k;
    m_nlc_slow_start = Device.dwTimeGlobal;
    m_nlc_slow_end = Device.dwTimeGlobal + time_ms;
}

float CBaseMonster::nlc_move_speed_k() const
{
    const u32 now = Device.dwTimeGlobal;
    float k = m_nlc_speed_base * m_nlc_speed_bonus;
    if (now < m_nlc_haste_end)
        k *= m_nlc_haste_k;
    if (now < m_nlc_evade.dodge_end)
        k *= m_nlc_evade.dodge_k; // NLC: zigzag legs, lunge

    if (now >= m_nlc_slow_end || m_nlc_slow_end <= m_nlc_slow_start)
        return k;

    const float left = float(m_nlc_slow_end - now) / float(m_nlc_slow_end - m_nlc_slow_start);
    return k * (1.f - m_nlc_slow_k * std::min(1.f, left / 0.25f));
}
