#include "stdafx.h"
#include "actorcondition.h"
#include "actor.h"
#include "actorEffector.h"
#include "inventory.h"
#include "level.h"
#include "sleepeffector.h"
#include "game_base_space.h"
#include "xrserver.h"
#include "ai_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "object_broker.h"
#include "weapon.h"
#include "PDA.h"
#include "ai/monsters/BaseMonster/base_monster.h"
#include "nlc_body_health.h"
#include "CustomOutfit.h" // NLC
#include "BoneProtections.h" // NLC
#include "nlc_npc_damage.h" // NLC

#define MAX_SATIETY 1.0f
#define START_SATIETY 0.5f

BOOL GodMode() { return psActorFlags.test(AF_GODMODE); }

CActorCondition::CActorCondition(CActor* object) : inherited(object)
{
    m_fJumpPower = 0.f;
    m_fStandPower = 0.f;
    m_fWalkPower = 0.f;
    m_fJumpWeightPower = 0.f;
    m_fWalkWeightPower = 0.f;
    m_fOverweightWalkK = 0.f;
    m_fOverweightJumpK = 0.f;
    m_fAccelK = 0.f;
    m_fSprintK = 0.f;
    m_fAlcohol = 0.f;
    m_fSatiety = 1.0f;
    m_fThirst = 1.0f;

    auto path_exists = [](LPCSTR alias) {
        const FS_Path* P = FS.get_path(alias);
        return P && GetFileAttributes(P->m_Path) != INVALID_FILE_ATTRIBUTES;
    };
    if (path_exists("$game_config$"))
        m_gamedata_flag_tmp += 1.f;
    if (path_exists("$game_scripts$"))
        m_gamedata_flag_tmp += 2.f;

    m_bJumpRequirePower = false;

    VERIFY(object);
    m_object = object;
    m_condition_flags.zero();

    m_f_time_affected = Device.fTimeGlobal;

    monsters_feel_touch = xr_new<Feel::Touch>();
    monsters_aura_radius = 0.f;

    m_body = xr_new<CActorBodyHealth>(this); // NLC
}

CActorCondition::~CActorCondition(void)
{
    xr_delete(monsters_feel_touch);
    xr_delete(m_body); // NLC
}

void CActorCondition::LoadCondition(LPCSTR entity_section)
{
    inherited::LoadCondition(entity_section);

    LPCSTR section = READ_IF_EXISTS(pSettings, r_string, entity_section, "condition_sect", entity_section);

    m_fJumpPower = pSettings->r_float(section, "jump_power");
    m_fStandPower = pSettings->r_float(section, "stand_power");
    m_fWalkPower = pSettings->r_float(section, "walk_power");
    m_fJumpWeightPower = pSettings->r_float(section, "jump_weight_power");
    m_fWalkWeightPower = pSettings->r_float(section, "walk_weight_power");
    m_fOverweightWalkK = pSettings->r_float(section, "overweight_walk_k");
    m_fOverweightJumpK = pSettings->r_float(section, "overweight_jump_k");
    m_fAccelK = pSettings->r_float(section, "accel_k");
    m_fSprintK = pSettings->r_float(section, "sprint_k");
    g_up_speed_k = READ_IF_EXISTS(pSettings, r_float, section, "up_speed_k", 1.3f);

    m_bJumpRequirePower = READ_IF_EXISTS(pSettings, r_bool, section, "jump_require_power", false);

    //порог силы и здоровья меньше которого актер начинает хромать
    m_fLimpingHealthBegin = pSettings->r_float(section, "limping_health_begin");
    m_fLimpingHealthEnd = pSettings->r_float(section, "limping_health_end");
    R_ASSERT(m_fLimpingHealthBegin <= m_fLimpingHealthEnd);

    m_fLimpingPowerBegin = pSettings->r_float(section, "limping_power_begin");
    m_fLimpingPowerEnd = pSettings->r_float(section, "limping_power_end");
    R_ASSERT(m_fLimpingPowerBegin <= m_fLimpingPowerEnd);

    m_fCantWalkPowerBegin = pSettings->r_float(section, "cant_walk_power_begin");
    m_fCantWalkPowerEnd = pSettings->r_float(section, "cant_walk_power_end");
    R_ASSERT(m_fCantWalkPowerBegin <= m_fCantWalkPowerEnd);

    m_fCantSprintPowerBegin = pSettings->r_float(section, "cant_sprint_power_begin");
    m_fCantSprintPowerEnd = pSettings->r_float(section, "cant_sprint_power_end");
    R_ASSERT(m_fCantSprintPowerBegin <= m_fCantSprintPowerEnd);

    m_fPowerLeakSpeed = pSettings->r_float(section, "max_power_leak_speed");

    m_fV_Alcohol = pSettings->r_float(section, "alcohol_v");
    m_fV_Power = READ_IF_EXISTS(pSettings, r_float, section, "power_v", 0.0f);

    m_fV_Satiety = pSettings->r_float(section, "satiety_v");
    m_fV_SatietyPower = pSettings->r_float(section, "satiety_power_v");
    m_fV_SatietyHealth = pSettings->r_float(section, "satiety_health_v");

    m_fSatietyLightLimit = READ_IF_EXISTS(pSettings, r_float, section, "satiety_light_limit", 0.0f);
    clamp(m_fSatietyLightLimit, 0.0f, 1.0f);

    m_fSatietyCriticalLimit = READ_IF_EXISTS(pSettings, r_float, section, "satiety_critical_limit", 0.0f);
    clamp(m_fSatietyCriticalLimit, 0.0f, 1.0f);

    if (m_fSatietyCriticalLimit > m_fSatietyLightLimit)
    {
        m_fSatietyCriticalLimit = m_fSatietyLightLimit;
    }

    if (Core.Features.test(xrCore::Feature::actor_thirst))
    {
        m_fThirstLightLimit = READ_IF_EXISTS(pSettings, r_float, section, "thirst_light_limit", 0.0f);
        clamp(m_fThirstLightLimit, 0.0f, 1.0f);

        m_fThirstCriticalLimit = READ_IF_EXISTS(pSettings, r_float, section, "thirst_critical_limit", 0.0f);
        clamp(m_fThirstCriticalLimit, 0.0f, 1.0f);

        if (m_fThirstCriticalLimit > m_fThirstLightLimit)
        {
            m_fThirstCriticalLimit = m_fThirstLightLimit;
        }

        m_fV_Thirst = pSettings->r_float(section, "thirst_v");
        m_fV_ThirstPower = pSettings->r_float(section, "thirst_power_v");
        m_fV_ThirstHealth = pSettings->r_float(section, "thirst_health_v");
    }

    m_MaxWalkWeight = pSettings->r_float(section, "max_walk_weight");

    m_body->Load(section); // NLC
}

//вычисление параметров с ходом времени
#include "UI.h"
#include "HUDManager.h"

void CActorCondition::UpdateCondition()
{
    if (GodMode())
        return;
    if (!object().g_Alive())
        return;
    if (!object().Local() && m_object != Level().CurrentViewEntity())
        return;

    float weight = object().GetCarryWeight();
    float max_weight;
    if (Core.Features.test(xrCore::Feature::condition_jump_weight_mod))
        max_weight = object().inventory().GetMaxWeight() + object().ArtefactsAddWeight(false);
    else
        max_weight = object().MaxCarryWeight();

    float weight_coef = weight / max_weight;

    if ((object().mstate_real & mcAnyMove))
    {
        ConditionWalk(weight_coef, isActorAccelerated(object().mstate_real, object().IsZoomAimingMode()), (object().mstate_real & mcSprint) != 0);
    }
    else
    {
        ConditionStand(weight_coef);
    }

    if (m_fPowerLeakSpeed > 0.0f)
    {
        float k_max_power = 1.0f;

        if (true)
        {
            float base_w = object().MaxCarryWeight();

            k_max_power = 1.0f + _min(weight, base_w) / base_w + _max(0.0f, (weight - base_w) / 10.0f);
        }
        else
            k_max_power = 1.0f;

        SetMaxPower(GetMaxPower() - m_fPowerLeakSpeed * m_fDeltaTime * k_max_power);
    }    

    m_fAlcohol += m_fV_Alcohol * m_fDeltaTime;
    clamp(m_fAlcohol, 0.0f, 1.0f);

    CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effAlcohol);
    if ((m_fAlcohol > 0.0001f))
    {
        if (!ce)
        {
            AddEffector(m_object, effAlcohol, "effector_alcohol", fastdelegate::MakeDelegate(this, &CActorCondition::GetAlcohol));
        }
    }
    else
    {
        if (ce)
            RemoveEffector(m_object, effAlcohol);
    }

    CEffectorPP* ppe = object().Cameras().GetPPEffector((EEffectorPPType)effPsyHealth);

    string64 pp_sect_name;
    shared_str ln = Level().name();
    strconcat(sizeof(pp_sect_name), pp_sect_name, "effector_psy_health", "_", *ln);
    if (!pSettings->section_exist(pp_sect_name))
        strcpy_s(pp_sect_name, "effector_psy_health");

    if (!fsimilar(GetPsyHealth(), 1.0f, 0.05f))
    {
        if (!ppe)
        {
            AddEffector(m_object, effPsyHealth, pp_sect_name, fastdelegate::MakeDelegate(this, &CActorCondition::GetPsy));
        }
    }
    else
    {
        if (ppe)
            RemoveEffector(m_object, effPsyHealth);
    }

    if (fis_zero(GetPsyHealth()))
        health() = 0.0f;

    UpdateSatiety();

    if (Core.Features.test(xrCore::Feature::actor_thirst))
        UpdateThirst();

    if (m_fRadiation > 0.f && CanBeHarmed())
        m_body->OnRadiation(m_change_v.m_fV_RadiationHealth * m_fRadiation * m_fDeltaTime); // NLC

    inherited::UpdateCondition();

    m_body->Update(m_fDeltaTime); // NLC

    UpdateTutorialThresholds();

    AffectDamage_InjuriousMaterialAndMonstersInfluence();
}

void CActorCondition::AffectDamage_InjuriousMaterialAndMonstersInfluence()
{
    float one = 0.1f;
    float tg = Device.fTimeGlobal;
    if (m_f_time_affected + one > tg)
    {
        return;
    }

    clamp(m_f_time_affected, tg - (one * 3), tg);

    float psy_influence = 0;
    float fire_influence = 0;
    float radiation_influence = GetInjuriousMaterialDamage(); // Get Radiation from Material

    // Add Radiation and Psy Level from Monsters
    monsters_feel_touch->feel_touch_update(object().Position(), monsters_aura_radius, {}, [&](const auto O) -> bool {
        const auto monster = smart_cast<CBaseMonster*>(O);
        return monster ? true : false;
    });
    for (const auto& it : monsters_feel_touch->feel_touch)
    {
        const auto monster = smart_cast<CBaseMonster*>(it);
        if (!monster)
            continue;
        psy_influence += monster->get_psy_influence();
        radiation_influence += monster->get_radiation_influence();
        fire_influence += monster->get_fire_influence();
    }
    /*
        CPda* const pda						=	m_object->GetPDA();

        if ( pda )
        {
            using monsters = xr_vector<CObject*>;

            for ( monsters::const_iterator	it	=	pda->feel_touch.begin();
                                            it	!=	pda->feel_touch.end();
                                            ++it )
            {
                CBaseMonster* const	monster		=	smart_cast<CBaseMonster*>(*it);
                if ( !monster || !monster->g_Alive() ) continue;

                psy_influence					+=	monster->get_psy_influence();
                radiation_influence				+=	monster->get_radiation_influence();
                fire_influence					+=	monster->get_fire_influence();
            }
        }
    */

    struct
    {
        ALife::EHitType type;
        float value;

    } hits[] = {{ALife::eHitTypeRadiation, radiation_influence * one}, {ALife::eHitTypeTelepatic, psy_influence * one}, {ALife::eHitTypeBurn, fire_influence * one}};

    NET_Packet np;

    while (m_f_time_affected + one < tg)
    {
        m_f_time_affected += one;

        for (auto& hit : hits)
        {
            float damage = hit.value;
            ALife::EHitType type = hit.type;

            if (damage > EPS)
            {
                SHit HDS = SHit(damage,
                                //.								0.0f,
                                Fvector().set(0, 1, 0), nullptr, BI_NONE, Fvector().set(0, 0, 0), 0.0f, type, 0.0f, false);

                HDS.GenHeader(GE_HIT, m_object->ID());
                HDS.Write_Packet(np);
                CGameObject::u_EventSend(np);
            }

        } // for

    } // while
}

#include "characterphysicssupport.h"
float CActorCondition::GetInjuriousMaterialDamage()
{
    /*
        u16 mat_injurios = m_object->character_physics_support()->movement()->injurious_material_idx();

        if(mat_injurios!=GAMEMTL_NONE_IDX)
        {
            const SGameMtl* mtl		= GMLib.GetMaterialByIdx(mat_injurios);
            return					mtl->fInjuriousSpeed;
        }else
    */
    return 0.0f;
}

void CActorCondition::UpdateSatiety()
{
    if (m_fSatiety > 0)
    {
        m_fSatiety -= m_fV_Satiety * m_fDeltaTime;
        clamp(m_fSatiety, 0.0f, 1.0f);
    }

    float satiety_health_koef = 1;
    float satiety_power_koef = 1;

    if (m_fSatietyLightLimit > 0)
    {
        if (m_fSatiety < m_fSatietyLightLimit)
        {
            satiety_power_koef = m_fSatiety / m_fSatietyLightLimit;

            const float critical_k = m_fSatietyCriticalLimit / m_fSatietyLightLimit;
            satiety_health_koef = (m_fSatiety / m_fSatietyLightLimit - critical_k) / (m_fSatiety >= m_fSatietyCriticalLimit ? 1 - critical_k : critical_k);
        }
    }
    else
    {
        if (fis_zero(m_fSatiety))
        {
            satiety_health_koef = -1;
        }
    }

    if (m_bIsBleeding && satiety_health_koef > 0)
    {
        satiety_health_koef = 0;
    }

    m_fDeltaHealth += m_fV_SatietyHealth * satiety_health_koef * m_fDeltaTime;
    m_fDeltaPower += m_fV_SatietyPower * satiety_power_koef * m_fDeltaTime;
}

void CActorCondition::UpdateThirst()
{
    if (m_fThirst > 0)
    {
        m_fThirst -= m_fV_Thirst * m_fDeltaTime;
        clamp(m_fThirst, 0.0f, 1.0f);
    }

    float thirst_health_koef = 1;
    float thirst_power_koef = 0;

    if (m_fThirstLightLimit > 0)
    {
        if (m_fThirst < m_fThirstLightLimit)
        {
            thirst_power_koef = (1 - m_fThirst / m_fThirstLightLimit) * -1;

            const float critical_k = m_fThirstCriticalLimit / m_fThirstLightLimit;
            thirst_health_koef = (m_fThirst / m_fThirstLightLimit - critical_k) / (m_fThirst >= m_fThirstCriticalLimit ? 1 - critical_k : critical_k);
        }
    }
    else
    {
        if (fis_zero(m_fThirst))
        {
            thirst_health_koef = -1;
        }
    }

    if (m_bIsBleeding && thirst_health_koef > 0)
    {
        thirst_health_koef = 0;
    }

    m_fDeltaHealth += m_fV_ThirstHealth * thirst_health_koef * m_fDeltaTime;
    m_fDeltaPower += m_fV_ThirstPower * thirst_power_koef * m_fDeltaTime;
}

void CActorCondition::UpdatePower()
{
    m_fPower += m_fV_Power * m_fDeltaTime;
    clamp(m_fPower, 0.0f, 1.0f);
}

CWound* CActorCondition::ConditionHit(SHit* pHDS)
{
    if (GodMode())
        return NULL;

    // NLC: last-hit record for scripts (A2); HitOutfitEffect fills post / k_eff / armour
    m_nlc_last_hit = SNlcLastHit{};
    m_nlc_last_hit.time = Device.dwTimeGlobal;
    m_nlc_last_hit.type = u32(pHDS->hit_type);
    m_nlc_last_hit.bone = pHDS->boneID;
    m_nlc_last_hit.raw = pHDS->damage();
    m_nlc_last_hit.post = m_nlc_last_hit.raw;
    m_nlc_hit_weapon = pHDS->weaponID; // NLC: ammo nlc_blunt_k for the bullet model

    // NLC: bullets use their own head scale (A5); the damage table keeps serving mutant hits
    const SNlcActorDamage& nlc = nlc_actor_damage();
    if (pHDS->hit_type == ALife::eHitTypeFireWound && nlc.enabled && nlc.bullet_head_scale >= 0.f && pHDS->boneID != BI_NONE &&
        m_body->PartByBone(u16(pHDS->boneID)) == body_part::head)
        m_fHitBoneScale = nlc.bullet_head_scale;

    // NLC: health lost by a hit goes to the body part, the total is their average
    const float delta_before = m_fDeltaHealth;
    CWound* wound = inherited::ConditionHit(pHDS);
    const float lost = delta_before - m_fDeltaHealth;
    m_nlc_last_hit.dealt = lost;
    if (m_body->Enabled())
    {
        m_fDeltaHealth = delta_before;
        m_body->OnHit(pHDS, lost);
    }
    else if (lost > 0.f && m_body->GuardHitType(u32(pHDS->hit_type)))
    {
        // NLC: one-shot guard on the single health pool (body health option off)
        const float before = GetHealth() + delta_before;
        const float after = m_body->Guard(body_part::all, before, before - lost);
        m_fDeltaHealth = delta_before - (before - after);
    }

    if (nlc.log && pHDS->hit_type == ALife::eHitTypeFireWound)
        Msg("* [nlc_actor_damage] bone=%d raw=%.3f post=%.3f K=%.3f A=%.3f dealt=%.3f model=%d guard=%d head_cov=%.2f", pHDS->boneID, m_nlc_last_hit.raw,
            m_nlc_last_hit.post, m_nlc_last_hit.k_eff, m_nlc_last_hit.armour, m_nlc_last_hit.dealt, m_nlc_last_hit.bullet_model, m_nlc_last_hit.guarded,
            m_nlc_head_cov);
    return wound;
}

// NLC: actor bullet model (docs/damage_model/DESIGN_actor_damage_model.md 3, A1); other hit types and suits
// without bullet_protection_sect keep the original outfit path
float CActorCondition::HitOutfitEffect(float hit_power, ALife::EHitType hit_type, s16 element, float AP)
{
    if (hit_type == ALife::eHitTypeFireWound && nlc_actor_damage().enabled)
    {
        CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(object().inventory().ItemFromSlot(OUTFIT_SLOT));
        if (outfit && outfit->bullet_protection())
        {
            const float result = nlc_bullet_outfit(hit_power, element, AP, outfit);
            if (!m_nlc_dry)
                outfit->Hit(hit_power, hit_type); // wear from the incoming power, as the original path
            m_nlc_last_hit.post = result;
            m_nlc_last_hit.bullet_model = true;
            return result;
        }
    }
    float result;
    if (m_nlc_dry)
    {
        // same as the original path, without outfit wear
        CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(object().inventory().ItemFromSlot(OUTFIT_SLOT));
        if (!outfit)
            result = hit_power;
        else if (hit_type == ALife::eHitTypeFireWound)
            result = outfit->HitThruArmour(hit_power, element, nlc_npc_damage().apply_ap_actor ? AP : 0.f);
        else
            result = hit_power * outfit->GetHitTypeProtection(hit_type, element);
    }
    else
        result = inherited::HitOutfitEffect(hit_power, hit_type, element, AP);
    m_nlc_last_hit.post = result;
    return result;
}

void CActorCondition::nlc_dry_hit(SHit* pHDS, float hit_scale)
{
    m_nlc_last_hit = SNlcLastHit{};
    m_nlc_last_hit.time = Device.dwTimeGlobal;
    m_nlc_last_hit.type = u32(pHDS->hit_type);
    m_nlc_last_hit.bone = pHDS->boneID;
    m_nlc_last_hit.raw = pHDS->damage();
    m_nlc_last_hit.post = m_nlc_last_hit.raw;
    m_nlc_hit_weapon = pHDS->weaponID;

    const SNlcActorDamage& nlc = nlc_actor_damage();
    const bool head = pHDS->boneID != BI_NONE && m_body->PartByBone(u16(pHDS->boneID)) == body_part::head;
    if (pHDS->hit_type == ALife::eHitTypeFireWound && nlc.enabled && nlc.bullet_head_scale >= 0.f && head)
        hit_scale = nlc.bullet_head_scale;

    m_nlc_dry = true;
    const float post = HitOutfitEffect(pHDS->damage(), pHDS->hit_type, pHDS->boneID, pHDS->ap);
    m_nlc_dry = false;

    const ALife::EHitType t = pHDS->hit_type;
    float dealt = 0.f;
    if (t == ALife::eHitTypeFireWound || t == ALife::eHitTypeWound || t == ALife::eHitTypeBurn)
        dealt = post * m_HitTypeK[t] * m_fHealthHitPart[t] * hit_scale;
    else if (t == ALife::eHitTypeShock || t == ALife::eHitTypeStrike || t == ALife::eHitTypeExplosion || t == ALife::eHitTypePhysicStrike)
        dealt = post * m_HitTypeK[t] * m_fHealthHitPart[t];
    m_nlc_last_hit.dealt = dealt;

    // would the one-shot guard have caught it (against the current part health)
    if (dealt > 0.f && m_body->GuardHitType(u32(t)))
    {
        const u8 part = m_body->Enabled() ? (pHDS->boneID == BI_NONE ? body_part::none : m_body->PartByBone(u16(pHDS->boneID))) : body_part::all;
        if (part != body_part::none)
        {
            const float before = part == body_part::all ? GetHealth() : m_body->PartHealth(part);
            m_nlc_last_hit.guarded = m_body->GuardWould(part, before, before - dealt);
        }
    }

    if (nlc.log && t == ALife::eHitTypeFireWound)
        Msg("* [nlc_actor_damage] (god) bone=%d raw=%.3f post=%.3f K=%.3f A=%.3f dealt=%.3f model=%d guard=%d head_cov=%.2f", pHDS->boneID, m_nlc_last_hit.raw,
            m_nlc_last_hit.post, m_nlc_last_hit.k_eff, m_nlc_last_hit.armour, m_nlc_last_hit.dealt, m_nlc_last_hit.bullet_model, m_nlc_last_hit.guarded,
            m_nlc_head_cov);
}

float CActorCondition::nlc_bullet_absorb(u16 bone, float power, float ap)
{
    if (!nlc_actor_damage().enabled || bone == BI_NONE || power <= 0.f)
        return -1.f;
    CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(object().inventory().ItemFromSlot(OUTFIT_SLOT));
    if (!outfit || !outfit->bullet_protection())
        return -1.f;
    const SNlcLastHit saved = m_nlc_last_hit;
    const u16 saved_weapon = m_nlc_hit_weapon;
    m_nlc_hit_weapon = u16(-1);
    const float post = nlc_bullet_outfit(power, s16(bone), ap, outfit);
    m_nlc_last_hit = saved;
    m_nlc_hit_weapon = saved_weapon;
    return std::clamp(1.f - post / power, 0.f, 1.f);
}

float CActorCondition::nlc_bullet_outfit(float hit_power, s16 element, float AP, CCustomOutfit* outfit)
{
    const SNlcActorDamage& nlc = nlc_actor_damage();
    SBoneProtections* bp = outfit->bullet_protection();
    const float cf = std::min(1.f, outfit->GetCondition() / nlc.cond_full);
    const float ap = std::clamp(AP, 0.f, 1.f);

    float cov, armour, blunt;
    bool head_bone = false;
    if (u16(element) != BI_NONE && m_body->PartByBone(u16(element)) == body_part::head)
    {
        // the head is covered by the worn mask only (its own condition is in the coverage)
        cov = m_nlc_head_cov;
        armour = m_nlc_head_armour;
        blunt = nlc.blunt_head * m_nlc_head_cov;
        head_bone = true;
    }
    else
    {
        cov = bp->getBoneProtection(element);
        armour = bp->getBoneArmour(element) * cf;
        blunt = bp->getBoneBlunt(element);
    }

    const float pen = std::max(hit_power - armour * (1.f - ap), hit_power * bp->m_fHitFrac);
    const float F = std::clamp(outfit->nlc_fire_wound_protection() * cf, -0.5f, 0.95f);
    const float absorb = 1.f - powf(1.f - F, nlc.fw_protection_exp);
    float k = 1.f - cov * absorb;
    if (k < 1.f)
        k += (1.f - k) * ap * nlc.ap_koeff_bypass;
    k = std::max(k, 0.f);

    m_nlc_last_hit.k_eff = k;
    m_nlc_last_hit.armour = armour;
    // NLC: blunt-impact minimum (docs/damage_model/DESIGN_npc_damage_model.md 21), same as for NPCs
    const float result = std::min(pen, hit_power) * k;
    return blunt > 0.f ? std::max(result, hit_power * blunt * nlc_hit_blunt_k(m_nlc_hit_weapon, head_bone)) : result;
}

void CActorCondition::PowerHit(float power, bool apply_outfit)
{
    m_fPower -= apply_outfit ? HitPowerEffect(power) : power;
    clamp(m_fPower, 0.f, 1.f);
}

// weight - "удельный" вес от 0..1
void CActorCondition::ConditionJump(float weight)
{
    float power = m_fJumpPower;
    power += m_fJumpWeightPower * weight * (weight > 1.f ? m_fOverweightJumpK : 1.f);
    m_fPower -= HitPowerEffect(power);
}
void CActorCondition::ConditionWalk(float weight, bool accel, bool sprint)
{
    static float last_y = 0.f;

    const float max_carry = object().MaxCarryWeight();
    float power = m_fWalkWeightPower * weight;
    const float overweight = max_carry * weight - max_carry;
    if (overweight > 0.f)
        power += m_fOverweightWalkK * 0.0000001f * (overweight * overweight);
    power += m_fWalkPower;
    power *= (accel ? (sprint ? m_fSprintK : m_fAccelK) : 1.f) * m_fDeltaTime;

    float up = 0.f;
    const float y = g_actor->Position().y;
    if (last_y != 0.f && y != last_y)
        up = (y - last_y) * 0.3f;
    power *= 0.5f;
    last_y = y;

    if (up > 0.f)
        power += up * g_up_speed_k * m_fJumpWeightPower * weight * (weight > 1.f ? m_fOverweightJumpK : 1.f);

    m_fPower -= HitPowerEffect(power);
}

void CActorCondition::ConditionStand(float weight)
{
    float power = m_fStandPower;
    power *= m_fDeltaTime;
    m_fPower -= power;
}

bool CActorCondition::IsCantWalk()
{
    if (m_fPower < m_fCantWalkPowerBegin)
        m_condition_flags.set(eCantWalk, TRUE);
    else if (m_fPower > m_fCantWalkPowerEnd)
        m_condition_flags.set(eCantWalk, FALSE);
    return m_condition_flags.test(eCantWalk);
}

#include "CustomOutfit.h"

bool CActorCondition::IsCantWalkWeight()
{
    if (!GodMode())
    {
        float max_w = m_MaxWalkWeight + object().ArtefactsAddWeight();

        CCustomOutfit* outfit = m_object->GetOutfit();
        if (outfit)
            max_w += outfit->m_additional_weight;

        if (object().GetCarryWeight() > max_w)
            return true;
    }
    return false;
}

bool CActorCondition::IsCantSprint()
{
    if (m_fPower < m_fCantSprintPowerBegin)
        m_condition_flags.set(eCantSprint, TRUE);
    else if (m_fPower > m_fCantSprintPowerEnd)
        m_condition_flags.set(eCantSprint, FALSE);
    return m_condition_flags.test(eCantSprint);
}

bool CActorCondition::IsCantJump(float weight)
{
    if (!m_bJumpRequirePower || GodMode())
    {
        return false;
    }

    float power = m_fJumpPower;
    power += m_fJumpWeightPower * weight * (weight > 1.f ? m_fOverweightJumpK : 1.f);
    return m_fPower < HitPowerEffect(power);
}

bool CActorCondition::IsLimping()
{
    if (m_fPower < m_fLimpingPowerBegin || GetHealth() < m_fLimpingHealthBegin)
        m_condition_flags.set(eLimping, TRUE);
    else if (m_fPower > m_fLimpingPowerEnd && GetHealth() > m_fLimpingHealthEnd)
        m_condition_flags.set(eLimping, FALSE);
    return m_condition_flags.test(eLimping);
}
extern bool g_bShowHudInfo;

void CActorCondition::save(NET_Packet& output_packet)
{
    inherited::save(output_packet);
    save_data(m_fAlcohol, output_packet);
    save_data(m_condition_flags, output_packet);
    save_data(m_fSatiety, output_packet);
    save_data(m_fThirst, output_packet);
    if (m_gamedata_flag_tmp > m_gamedata_flag)
        m_gamedata_flag = m_gamedata_flag_tmp;
    save_data(m_gamedata_flag, output_packet);
    m_body->save(output_packet); // NLC
}

#include "alife_registry_wrappers.h"
#include "alife_simulator_header.h"

void CActorCondition::load(IReader& input_packet)
{
    inherited::load(input_packet);
    load_data(m_fAlcohol, input_packet);
    load_data(m_condition_flags, input_packet);
    load_data(m_fSatiety, input_packet);
    if (ai().get_alife()->header().version() > 8)
    {
        load_data(m_fThirst, input_packet);
        load_data(m_gamedata_flag, input_packet);
        Msg("Gamedata flags: [%f]", m_gamedata_flag);
    }
    m_body->load(input_packet); // NLC
}

void CActorCondition::reinit()
{
    inherited::reinit();
    m_condition_flags.set(eLimping, FALSE);
    m_condition_flags.set(eCantWalk, FALSE);
    m_condition_flags.set(eCantSprint, FALSE);
    m_fSatiety = 1.f;
    m_fAlcohol = 0.f;
    m_fThirst = 1.f;
    m_body->Reinit(); // NLC
}

void CActorCondition::ChangeAlcohol(float value) { m_fAlcohol += value; }

void CActorCondition::ChangeSatiety(float value)
{
    m_fSatiety += value;
    clamp(m_fSatiety, 0.0f, 1.0f);
}

void CActorCondition::ChangeThirst(float value)
{
    m_fThirst += value;
    clamp(m_fThirst, 0.0f, 1.0f);
}

void CActorCondition::UpdateTutorialThresholds()
{
    string256 cb_name;
    static float _cPowerThr = pSettings->r_float("tutorial_conditions_thresholds", "power");
    static float _cPowerMaxThr = pSettings->r_float("tutorial_conditions_thresholds", "max_power");
    static float _cBleeding = pSettings->r_float("tutorial_conditions_thresholds", "bleeding");
    static float _cSatiety = pSettings->r_float("tutorial_conditions_thresholds", "satiety");
    static float _cRadiation = pSettings->r_float("tutorial_conditions_thresholds", "radiation");
    static float _cWpnCondition = pSettings->r_float("tutorial_conditions_thresholds", "weapon_jammed");
    static float _cPsyHealthThr = pSettings->r_float("tutorial_conditions_thresholds", "psy_health");
    static float _cThirst = 0.0f;

    if (Core.Features.test(xrCore::Feature::actor_thirst))
    {
        _cThirst = pSettings->r_float("tutorial_conditions_thresholds", "thirst");
    }

    bool b = true;
    if (b && !m_condition_flags.test(eCriticalPowerReached) && GetPower() < _cPowerThr)
    {
        m_condition_flags.set(eCriticalPowerReached, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_critical_power");
    }

    if (b && !m_condition_flags.test(eCriticalMaxPowerReached) && GetMaxPower() < _cPowerMaxThr)
    {
        m_condition_flags.set(eCriticalMaxPowerReached, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_critical_max_power");
    }

    if (b && !m_condition_flags.test(eCriticalBleedingSpeed) && BleedingSpeed() > _cBleeding)
    {
        m_condition_flags.set(eCriticalBleedingSpeed, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_bleeding");
    }

    if (b && !m_condition_flags.test(eCriticalSatietyReached) && GetSatiety() < _cSatiety)
    {
        m_condition_flags.set(eCriticalSatietyReached, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_satiety");
    }

    if (Core.Features.test(xrCore::Feature::actor_thirst))
    {
        if (b && !m_condition_flags.test(eCriticalThirstReached) && GetThirst() < _cThirst)
        {
            m_condition_flags.set(eCriticalThirstReached, true);
            b = false;
            xr_strcpy(cb_name, "_G.on_actor_thirst");
        }
    }

    if (b && !m_condition_flags.test(eCriticalRadiationReached) && GetRadiation() > _cRadiation)
    {
        m_condition_flags.set(eCriticalRadiationReached, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_radiation");
    }

    if (b && !m_condition_flags.test(ePhyHealthMinReached) && GetPsyHealth() < _cPsyHealthThr)
    {
        m_condition_flags.set(ePhyHealthMinReached, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_psy");
    }

    if (b && !m_condition_flags.test(eCantWalkWeight) && IsCantWalkWeight())
    {
        m_condition_flags.set(eCantWalkWeight, TRUE);
        b = false;
        strcpy_s(cb_name, "_G.on_actor_cant_walk_weight");
    }

    if (b && !m_condition_flags.test(eWeaponJammedReached) && m_object->inventory().GetActiveSlot() != NO_ACTIVE_SLOT)
    {
        PIItem item = m_object->inventory().ItemFromSlot(m_object->inventory().GetActiveSlot());
        CWeapon* pWeapon = smart_cast<CWeapon*>(item);
        if (pWeapon && pWeapon->GetCondition() < _cWpnCondition)
        {
            m_condition_flags.set(eWeaponJammedReached, TRUE);
            b = false;
            strcpy_s(cb_name, "_G.on_actor_weapon_jammed");
        }
    }

    if (!b)
    {
        luabind::functor<LPCSTR> fl;
        R_ASSERT(ai().script_engine().functor<LPCSTR>(cb_name, fl));
        fl();
    }
}

bool CActorCondition::DisableSprint(SHit* pHDS)
{
    return (pHDS->hit_type != ALife::eHitTypeTelepatic) && (pHDS->hit_type != ALife::eHitTypeChemicalBurn) && (pHDS->hit_type != ALife::eHitTypeBurn) &&
        (pHDS->hit_type != ALife::eHitTypeRadiation);
}

float CActorCondition::HitSlowmo(SHit* pHDS)
{
    float ret;
    if (pHDS->hit_type == ALife::eHitTypeWound || pHDS->hit_type == ALife::eHitTypeStrike)
    {
        ret = pHDS->damage();
        clamp(ret, 0.0f, 1.f);
    }
    else
        ret = 0.0f;

    return ret;
}

void CActorCondition::net_Relcase(CObject* O)
{
    if (Level().is_removing_objects())
        return;
    monsters_feel_touch->feel_touch_relcase(O);
}
