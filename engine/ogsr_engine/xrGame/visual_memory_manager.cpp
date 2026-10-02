////////////////////////////////////////////////////////////////////////////
//	Module 		: visual_memory_manager.cpp
//	Created 	: 02.10.2001
//  Modified 	: 19.11.2003
//	Author		: Dmitriy Iassenev
//	Description : Visual memory manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "visual_memory_manager.h"
#include "ai/stalker/ai_stalker.h"
#include "memory_space_impl.h"
#include "../Include/xrRender/Kinematics.h"
#include "clsid_game.h"
#include "ai_object_location.h"
#include "level_graph.h"
#include "stalker_movement_manager.h"
#include "../xr_3da/gamemtllib.h"
#include "agent_manager.h"
#include "agent_member_manager.h"
#include "ai_space.h"

#include "actor.h"
#include "../xr_3da/camerabase.h"
#include "gamepersistent.h"
#include "actor_memory.h"
#include "client_spawn_manager.h"
#include "client_spawn_manager.h"
#include "memory_manager.h"
#include "alife_registry_wrappers.h"
#include "alife_simulator_header.h"
#include "holder_custom.h"
#include "inventory.h"
#include "torch.h"
#include "nlc_stealth.h" // NLC: stealth diagnostics and tuning

#ifndef MASTER_GOLD
#include "clsid_game.h"
#include "ai_debug.h"
#endif // MASTER_GOLD

struct SRemoveOfflinePredicate
{
    bool operator()(const CVisibleObject& object) const
    {
        return (!object.m_object || !!object.m_object->getDestroy() || object.m_object->H_Parent());
    }

    bool operator()(const CNotYetVisibleObject& object) const
    {
        return (!object.m_object || !!object.m_object->getDestroy() || object.m_object->H_Parent());
    }
};

struct CVisibleObjectPredicate
{
    u32 m_id;
    CVisibleObjectPredicate(u32 id) : m_id(id) {}

    bool operator()(const CObject* object) const
    {
        VERIFY(object);
        return (object->ID() == m_id);
    }
};

struct CNotYetVisibleObjectPredicate
{
    const CGameObject* m_game_object;

    IC CNotYetVisibleObjectPredicate(const CGameObject* game_object) { m_game_object = game_object; }

    IC bool operator()(const CNotYetVisibleObject& object) const { return (object.m_object->ID() == m_game_object->ID()); }
};

CVisualMemoryManager::CVisualMemoryManager(CCustomMonster* object)
{
    m_object = object;
    m_stalker = 0;
    m_client = 0;
    initialize();
}

CVisualMemoryManager::CVisualMemoryManager(CAI_Stalker* stalker)
{
    m_object = stalker;
    m_stalker = stalker;
    m_client = 0;
    initialize();
}

CVisualMemoryManager::CVisualMemoryManager(vision_client* client)
{
    m_object = 0;
    m_stalker = 0;
    m_client = client;
    initialize();

    m_objects = xr_new<VISIBLES>();
}

void CVisualMemoryManager::initialize()
{
    m_max_object_count = 1;
    m_adaptive_max_object_count = 0;
    m_enabled = true;
    m_objects = 0;
}

CVisualMemoryManager::~CVisualMemoryManager()
{
    clear_delayed_objects();

    if (!m_client)
        return;

    xr_delete(m_objects);
}

void CVisualMemoryManager::reinit()
{
    if (!m_client)
        m_objects = 0;
    else
    {
        VERIFY(m_objects);
        m_objects->clear();
    }

    m_visible_objects.clear();
    //	m_visible_objects.reserve			(100);

    m_not_yet_visible_objects.clear();
    //	m_not_yet_visible_objects.reserve	(100);

    // NLC: per-NPC stealth state; scripts re-apply flags on spawn
    m_nlc_point_time = 0;
    m_nlc_rate_k = 1.f;
    m_nlc_forced = 0.f;
    m_nlc_forced_until = 0;
    m_nlc_last_shot = 0;
    m_nlc_torch_time = 0;
    m_nlc_torch_on = false;
    m_nlc_nvd = false;

    if (m_object)
        m_object->feel_vision_clear();

    m_last_update_time = u32(-1);
}

void CVisualMemoryManager::reload(LPCSTR section)
{
    m_max_object_count = READ_IF_EXISTS(pSettings, r_s32, section, "DynamicObjectsCount", 1);
    m_adaptive_max_object_count = READ_IF_EXISTS(pSettings, r_u32, section, "DynamicObjectsCount_adaptive", 0u);

    if (m_stalker)
    {
        m_free.Load(pSettings->r_string(section, "vision_free_section"), true);
        m_danger.Load(pSettings->r_string(section, "vision_danger_section"), true);
    }
    else if (m_object)
    {
        m_free.Load(READ_IF_EXISTS(pSettings, r_string, section, "vision_free_section", section), !!m_client);
        m_danger.Load(READ_IF_EXISTS(pSettings, r_string, section, "vision_danger_section", section), !!m_client);
        nlc_stealth::load_monster_keys(*this, section); // NLC: monster senses
    }
    else
        m_free.Load(section, !!m_client);
}

/*IC*/ const CVisionParameters& CVisualMemoryManager::current_state() const
{
    // NLC: debug profile override for watched observers (nlc_stealth force_profile, off by default)
    if (nlc_stealth::g_force_profile >= 0.f && m_object && nlc_stealth::watched(m_object->ID()))
        return (nlc_stealth::g_force_profile > 0.5f) ? m_danger : m_free;

    if (m_stalker)
    {
        return (m_stalker->movement().mental_state() == eMentalStateDanger) ? m_danger : m_free;
    }
    else if (m_object)
    {
        return m_object->is_base_monster_with_enemy() ? m_danger : m_free;
    }
    else
    {
        return m_free;
    }
}

u32 CVisualMemoryManager::visible_object_time_last_seen(const CObject* object) const
{
    if (Actor()->Holder() && smart_cast<const CActor*>(object))
        object = smart_cast<const CObject*>(Actor()->Holder());

    VISIBLES::iterator I = std::find(m_objects->begin(), m_objects->end(), object_id(object));
    if (I != m_objects->end())
        return (I->m_level_time);
    else
        return u32(-1);
}

bool CVisualMemoryManager::visible_right_now(const CGameObject* game_object) const
{
    if (Actor()->Holder() && smart_cast<const CActor*>(game_object))
        game_object = smart_cast<const CGameObject*>(Actor()->Holder());

    VISIBLES::const_iterator I = std::find(objects().begin(), objects().end(), object_id(game_object));
    if ((objects().end() == I))
        return (false);

    if (!(*I).visible(mask()))
        return (false);

    if ((*I).m_level_time < m_last_update_time)
        return (false);

    return (true);
}

bool CVisualMemoryManager::visible_now(const CGameObject* game_object) const
{
    if (Actor()->Holder() && smart_cast<const CActor*>(game_object))
        game_object = smart_cast<const CGameObject*>(Actor()->Holder());

    VISIBLES::const_iterator I = std::find(objects().begin(), objects().end(), object_id(game_object));
    return ((objects().end() != I) && (*I).visible(mask()));
}

void CVisualMemoryManager::enable(const CObject* object, bool enable)
{
    VISIBLES::iterator J = std::find(m_objects->begin(), m_objects->end(), object_id(object));
    if (J == m_objects->end())
        return;
    (*J).m_enabled = enable;
}

float CVisualMemoryManager::object_visible_distance(const CGameObject* game_object, float& object_distance) const
{
    Fvector eye_position = Fvector().set(0.f, 0.f, 0.f), eye_direction;
    Fmatrix eye_matrix;
    float object_range = flt_max, object_fov = flt_max;

    if (m_object)
    {
        eye_matrix = smart_cast<IKinematics*>(m_object->Visual())->LL_GetTransform(u16(m_object->eye_bone));

        Fvector temp;
        eye_matrix.transform_tiny(temp, eye_position);
        m_object->XFORM().transform_tiny(eye_position, temp);

        if (m_stalker)
        {
            eye_direction.setHP(-m_stalker->movement().m_head.current.yaw, -m_stalker->movement().m_head.current.pitch);
        }
        else
        { // if its a monster
            const MonsterSpace::SBoneRotation& head_orient = m_object->head_orientation();
            eye_direction.setHP(-head_orient.current.yaw, -head_orient.current.pitch);
        }
    }
    else
    {
        Fvector dummy;
        float _0, _1;
        m_client->camera(eye_position, eye_direction, dummy, object_fov, _0, _1, object_range);
    }

    Fvector object_direction;
    game_object->Center(object_direction);
    object_distance = object_direction.distance_to(eye_position);
    object_direction.sub(eye_position);
    object_direction.normalize_safe();

    if (m_object)
        m_object->update_range_fov(object_range, object_fov, m_object->eye_range, deg2rad(m_object->eye_fov));

    float fov = object_fov * .5f;
    float cos_alpha = eye_direction.dotproduct(object_direction);
    clamp(cos_alpha, -.99999f, .99999f);
    float alpha = acosf(cos_alpha);
    clamp(alpha, 0.f, fov);

    float max_view_distance = object_range, min_view_distance = object_range;
    max_view_distance *= current_state().m_max_view_distance;
    min_view_distance *= current_state().m_min_view_distance;

    float distance = (1.f - alpha / fov) * (max_view_distance - min_view_distance) + min_view_distance;
    clamp(distance, 0.f, GamePersistent().Environment().CurrentEnv->fog_far);

    return (distance);
}

float CVisualMemoryManager::object_luminocity(const CGameObject* game_object) const
{
    const auto* pActor = smart_cast<const CActor*>(game_object);
    if (!pActor)
    {
        // NLC: light model for stalker targets of stalker observers (nlc_stealth npc_light_k; 0 = vanilla, always lit)
        if (nlc_stealth::g_npc_light_k > 0.f && m_stalker)
            if (const auto* target = smart_cast<const CAI_Stalker*>(game_object))
            {
                const float light = nlc_stealth::npc_target_light(m_object, target, m_nlc_nvd);
                const float factor = (nlc_stealth::g_lum_factor_override >= 0.f) ? nlc_stealth::g_lum_factor_override : current_state().m_luminocity_factor;
                return exp(log(light > .001f ? light : .001f) * factor);
            }
        return (1.f);
    }

    const auto* pTorch = smart_cast<const CTorch*>(pActor->GetCurrentTorch());
    if (pTorch && pTorch->torch_active())
    {
        float dist = m_object->Position().distance_to(game_object->Position());
        if (dist < pTorch->get_range())
            return 1.f;
    }

    float luminocity = const_cast<CGameObject*>(game_object)->ROS()->get_luminocity();
    // NLC: AI light terms for the actor (nlc_stealth sky_k, torch_k, flash_k; all 0 = off)
    if (m_object && nlc_stealth::g_actor_light_bonus)
    {
        if (m_stalker)
            luminocity += nlc_stealth::actor_light_parts(m_object, true, luminocity).total();
        else
        {
            // NLC: monsters take the sky and near-range terms by species (nlc_light_k), the muzzle flash always
            const nlc_stealth::LightParts parts = nlc_stealth::actor_light_parts(m_object, m_nlc_light_k > 0.f, luminocity);
            luminocity += m_nlc_light_k * (parts.sky + parts.near_light + parts.lamp) + parts.flash;
        }
        clamp(luminocity, 0.f, 1.f);
    }
    // NLC: night vision of nocturnal monsters (species nlc_dark_floor)
    if (!m_stalker && m_nlc_dark_floor > luminocity)
        luminocity = m_nlc_dark_floor;
    // NLC: night-vision device: light floor (nlc_stealth nvd_floor; flag set per NPC by script)
    if (m_nlc_nvd && nlc_stealth::g_nvd_floor > luminocity)
        luminocity = nlc_stealth::g_nvd_floor;
    // NLC: live override of the stalker luminocity_factor (nlc_stealth, identity by default)
    const float luminocity_factor = (m_stalker && nlc_stealth::g_lum_factor_override >= 0.f) ? nlc_stealth::g_lum_factor_override : current_state().m_luminocity_factor;
    float power = log(luminocity > .001f ? luminocity : .001f) * luminocity_factor;
    return (exp(power));
}

float CVisualMemoryManager::get_object_velocity(const CGameObject* game_object, const CNotYetVisibleObject& not_yet_visible_object) const
{
    if ((game_object->ps_Size() < 2) || (not_yet_visible_object.m_prev_time == game_object->ps_Element(game_object->ps_Size() - 2).dwTime))
        return (0.f);

    CObject::SavedPosition pos0 = game_object->ps_Element(game_object->ps_Size() - 2);
    CObject::SavedPosition pos1 = game_object->ps_Element(game_object->ps_Size() - 1);

    return (pos1.vPosition.distance_to(pos0.vPosition) / (float(pos1.dwTime) / 1000.f - float(pos0.dwTime) / 1000.f));
}

float CVisualMemoryManager::get_visible_value(float distance, float object_distance, float time_delta, float object_velocity, float luminocity, float trans) const
{
    float always_visible_distance = current_state().m_always_visible_distance;
    if (object_distance <= always_visible_distance)
        return current_state().m_visibility_threshold;
    if (distance <= always_visible_distance)
        distance = always_visible_distance + EPS_L;

    float fog_near = GamePersistent().Environment().CurrentEnv->fog_near;
    float fog_far = GamePersistent().Environment().CurrentEnv->fog_far;
    float fog_w = 1 / (fog_far - fog_near);
    float fog_x = -fog_near * fog_w;
    float fog = (object_distance * fog_w + fog_x) * current_state().m_fog_factor;
    clamp(fog, 0.f, 1.f);
    float fog_factor = 1.f - pow(fog, current_state().m_fog_pow);

    // NLC: live rate multiplier per profile (nlc_stealth, identity by default)
    const float rate_mult = m_object ? nlc_stealth::rate_mult(!!m_stalker, &current_state() == &m_danger) : 1.f;

    return (time_delta / current_state().m_time_quant * luminocity * (1.f + current_state().m_velocity_factor * object_velocity) * (distance - object_distance) /
            (distance - always_visible_distance) * fog_factor * trans * rate_mult);
}

CNotYetVisibleObject* CVisualMemoryManager::not_yet_visible_object(const CGameObject* game_object)
{
    START_PROFILE("Memory Manager/visuals/not_yet_visible_object")
    xr_vector<CNotYetVisibleObject>::iterator I = std::find_if(m_not_yet_visible_objects.begin(), m_not_yet_visible_objects.end(), CNotYetVisibleObjectPredicate(game_object));
    if (I == m_not_yet_visible_objects.end())
        return (0);
    return (&*I);
    STOP_PROFILE
}

void CVisualMemoryManager::add_not_yet_visible_object(const CNotYetVisibleObject& not_yet_visible_object) { m_not_yet_visible_objects.push_back(not_yet_visible_object); }

u32 CVisualMemoryManager::get_prev_time(const CGameObject* game_object) const
{
    if (!game_object->ps_Size())
        return (0);
    if (game_object->ps_Size() == 1)
        return (game_object->ps_Element(0).dwTime);
    return (game_object->ps_Element(game_object->ps_Size() - 2).dwTime);
}

bool CVisualMemoryManager::visible(const CGameObject* game_object, float time_delta)
{
    VERIFY(game_object);

    if (game_object->getDestroy())
        return (false);


    float object_distance, distance = object_visible_distance(game_object, object_distance);

    CNotYetVisibleObject* object = not_yet_visible_object(game_object);

    // NLC: stealth diagnostics for the actor as target (nlc_stealth; off unless a watch or log_events is set)
    const bool nlc_log = nlc_stealth::g_track && m_object && g_actor && game_object == g_actor && nlc_stealth::tracked(m_object->ID());
    nlc_stealth::VisionSample nlc_sample{};
    if (nlc_log)
    {
        nlc_sample.object_distance = object_distance;
        nlc_sample.view_distance = distance;
        nlc_sample.threshold = current_state().m_visibility_threshold;
        nlc_sample.time_delta = time_delta;
        nlc_sample.danger_profile = &current_state() == &m_danger;
        nlc_sample.sum_before = object ? object->m_value : 0.f;
        nlc_sample.ray = visible_transparency_threshold(game_object);
        nlc_sample.trans = 1.f;
    }

    if (distance < object_distance)
    {
        if (object)
        {
            object->m_value -= current_state().m_decrease_value;
            if (object->m_value < 0.f)
                object->m_value = 0.f;
            else
                object->m_update_time = Device.dwTimeGlobal;
            if (nlc_log) // NLC
            {
                nlc_sample.out_of_range = true;
                nlc_sample.sum_after = object->m_value;
                nlc_stealth::on_vision(m_object, current_state(), nlc_sample);
            }
            return (object->m_value >= current_state().m_visibility_threshold);
        }
        return (false);
    }

    float luminocity = object_luminocity(game_object);
    float trans;
    // NLC: live override of the stalker transparency_factor (nlc_stealth, identity by default)
    const float transparency_factor =
        (m_stalker && nlc_stealth::g_transparency_factor_override >= 0.f) ? nlc_stealth::g_transparency_factor_override : current_state().m_transparency_factor;
    if (transparency_factor > 0.f && smart_cast<const CActor*>(game_object))
    {
        trans = visible_transparency_threshold(game_object);
        if (trans < 1.f)
            trans = trans < 0.f ? 1.f : (trans * transparency_factor);

        clamp(trans, 0.f, 1.f);
    }
    else
        trans = 1.f;

    if (!object)
    {
        CNotYetVisibleObject new_object;
        new_object.m_object = game_object;
        new_object.m_prev_time = 0;
        float velocity = get_object_velocity(game_object, new_object); // NLC: kept for the diagnostics
        if (nlc_stealth::g_vel_physics > 0.f && game_object == g_actor) // NLC: physics speed instead of the noisy position history
            velocity = nlc_stealth::actor_speed();
        new_object.m_value = get_visible_value(distance, object_distance, time_delta, velocity, luminocity, trans);
        // NLC: rank and stance factor for the actor as target (not inside always_visible_distance)
        if (nlc_stealth::g_actor_rate_factor && m_object && game_object == g_actor && object_distance > current_state().m_always_visible_distance)
            new_object.m_value *= nlc_stealth::actor_rate_factor(m_object);
        if (m_stalker && g_actor && game_object == g_actor) // NLC: suspicion rate factor and noticed point
        {
            if (m_nlc_rate_k != 1.f && object_distance > current_state().m_always_visible_distance)
                new_object.m_value *= m_nlc_rate_k;
            if (new_object.m_value > 0.f)
            {
                m_nlc_point = g_actor->Position();
                m_nlc_point_time = Device.dwTimeGlobal;
            }
        }
        if (nlc_log) // NLC
        {
            nlc_sample.increment = new_object.m_value;
            nlc_sample.velocity = velocity;
        }
        clamp(new_object.m_value, 0.f, current_state().m_visibility_threshold + EPS_L);
        new_object.m_update_time = Device.dwTimeGlobal;
        new_object.m_prev_time = get_prev_time(game_object);
        add_not_yet_visible_object(new_object);
        if (nlc_log) // NLC
        {
            nlc_sample.luminocity_used = luminocity;
            nlc_sample.trans = trans;
            nlc_sample.always_visible = object_distance <= current_state().m_always_visible_distance;
            nlc_sample.sum_after = new_object.m_value;
            nlc_stealth::on_vision(m_object, current_state(), nlc_sample);
        }
        return (new_object.m_value >= current_state().m_visibility_threshold);
    }

    object->m_update_time = Device.dwTimeGlobal;
    float velocity = get_object_velocity(game_object, *object); // NLC: kept for the diagnostics
    if (nlc_stealth::g_vel_physics > 0.f && game_object == g_actor) // NLC: physics speed instead of the noisy position history
        velocity = nlc_stealth::actor_speed();
    float increment = get_visible_value(distance, object_distance, time_delta, velocity, luminocity, trans);
    // NLC: rank and stance factor for the actor as target (not inside always_visible_distance)
    if (nlc_stealth::g_actor_rate_factor && m_object && game_object == g_actor && object_distance > current_state().m_always_visible_distance)
        increment *= nlc_stealth::actor_rate_factor(m_object);
    if (m_stalker && g_actor && game_object == g_actor) // NLC: suspicion rate factor and noticed point
    {
        if (m_nlc_rate_k != 1.f && object_distance > current_state().m_always_visible_distance)
            increment *= m_nlc_rate_k;
        if (increment > 0.f)
        {
            m_nlc_point = g_actor->Position();
            m_nlc_point_time = Device.dwTimeGlobal;
        }
    }
    object->m_value += increment;
    clamp(object->m_value, 0.f, current_state().m_visibility_threshold + EPS_L);
    object->m_prev_time = get_prev_time(game_object);
    // NLC: a monster half-noticing the actor goes to investigate (nlc_stealth monster_notice_v; off by default)
    if (nlc_stealth::g_monster_notice_v > 0.f && !m_stalker && m_object && g_actor && game_object == g_actor)
        nlc_stealth::monster_visual_notice(m_object, object->m_value / current_state().m_visibility_threshold);

    if (nlc_log) // NLC
    {
        nlc_sample.increment = increment;
        nlc_sample.velocity = velocity;
        nlc_sample.luminocity_used = luminocity;
        nlc_sample.trans = trans;
        nlc_sample.always_visible = object_distance <= current_state().m_always_visible_distance;
        nlc_sample.sum_after = object->m_value;
        nlc_stealth::on_vision(m_object, current_state(), nlc_sample);
    }

    return (object->m_value >= current_state().m_visibility_threshold);
}

void CVisualMemoryManager::add_visible_object(const CObject* object, float time_delta, bool fictitious)
{
    if (object && object->getDestroy())
        return;

#ifndef MASTER_GOLD
    if (object && (object->CLS_ID == CLSID_OBJECT_ACTOR) && psAI_Flags.test(aiIgnoreActor))
        return;
#endif // MASTER_GOLD

    //	START_PROFILE("Memory Manager/visuals/update/add_visibles/visible")
    auto game_object = smart_cast<const CGameObject*>(object);
    if (!game_object || (!fictitious && !visible(game_object, time_delta)))
        return;
    //	STOP_PROFILE

    //	START_PROFILE("Memory Manager/visuals/update/add_visibles/find_object_by_id")
    const CGameObject* self = m_object;
    auto J = std::find(m_objects->begin(), m_objects->end(), object_id(game_object));
    //	STOP_PROFILE

    //	START_PROFILE("Memory Manager/visuals/update/add_visibles/fill")
    if (m_objects->end() == J)
    {
        CVisibleObject visible_object;

        visible_object.fill(game_object, self, mask(), mask());
#ifdef USE_FIRST_GAME_TIME
        visible_object.m_first_game_time = Level().GetGameTime();
#endif
#ifdef USE_LEVEL_TIME // USE_FIRST_LEVEL_TIME
        visible_object.m_first_level_time = Device.dwTimeGlobal;
#endif

        if (m_objects->size() >= m_max_object_count)
        {
            auto I = std::min_element(m_objects->begin(), m_objects->end(), SLevelTimePredicate<CGameObject>());
            VERIFY(m_objects->end() != I);
            if (!m_adaptive_max_object_count || I->m_level_time + m_adaptive_max_object_count < Device.dwTimeGlobal)
                m_objects->erase(I);
        }
        m_objects->push_front(visible_object);
    }
    else
    {
        if (!fictitious)
            (*J).fill(game_object, self, (*J).m_squad_mask.get() | mask(), (*J).m_visible.get() | mask());
        else
        {
            (*J).m_visible.assign((*J).m_visible.get() | mask());
            (*J).m_squad_mask.assign((*J).m_squad_mask.get() | mask());
            (*J).m_enabled = true;
        }
    }
    //	STOP_PROFILE
}

void CVisualMemoryManager::add_visible_object(CVisibleObject visible_object)
{
#ifndef MASTER_GOLD
    if (visible_object.m_object && (visible_object.m_object->CLS_ID == CLSID_OBJECT_ACTOR) && psAI_Flags.test(aiIgnoreActor))
        return;
#endif // MASTER_GOLD

    VERIFY(m_objects);
    auto J = std::find(m_objects->begin(), m_objects->end(), object_id(visible_object.m_object));
    if (m_objects->end() != J)
        *J = visible_object;
    else
    {
#ifdef USE_LEVEL_TIME // USE_FIRST_LEVEL_TIME
        visible_object.m_first_level_time = Device.dwTimeGlobal;
#endif
        if (m_objects->size() >= m_max_object_count)
        {
            auto I = std::min_element(m_objects->begin(), m_objects->end(), SLevelTimePredicate<CGameObject>());
            VERIFY(m_objects->end() != I);
            if (!m_adaptive_max_object_count || I->m_level_time + m_adaptive_max_object_count < Device.dwTimeGlobal)
                m_objects->erase(I);
        }
        m_objects->push_front(visible_object);
    }
}

#ifdef DEBUG
void CVisualMemoryManager::check_visibles() const
{
    squad_mask_type mask = this->mask();
    auto I = m_objects->begin();
    auto E = m_objects->end();
    for (; I != E; ++I)
    {
        if (!(*I).visible(mask))
            continue;

        xr_vector<Feel::Vision::feel_visible_Item>::iterator i = m_object->feel_visible.begin();
        xr_vector<Feel::Vision::feel_visible_Item>::iterator e = m_object->feel_visible.end();
        for (; i != e; ++i)
            if (i->O->ID() == (*I).m_object->ID())
            {
                VERIFY(i->fuzzy > 0.f);
                break;
            }
    }
}
#endif

bool CVisualMemoryManager::visible(u32 _level_vertex_id, float yaw, float eye_fov) const
{
    Fvector direction;
    direction.sub(ai().level_graph().vertex_position(_level_vertex_id), m_object->Position());
    direction.normalize_safe();
    float y, p;
    direction.getHP(y, p);
    if (angle_difference(yaw, y) <= eye_fov * PI / 180.f / 2.f)
        return (ai().level_graph().check_vertex_in_direction(m_object->ai_location().level_vertex_id(), m_object->Position(), _level_vertex_id));
    else
        return (false);
}

float CVisualMemoryManager::feel_vision_mtl_transp(CObject* O, u32 element)
{
    float vis = 1.f;
    if (O)
    {
        IKinematics* V = smart_cast<IKinematics*>(O->Visual());
        if (0 != V)
        {
            CBoneData& B = V->LL_GetData((u16)element);
            vis = GMLib.GetMaterialByIdx(B.game_mtl_idx)->fVisTransparencyFactor;
        }
    }
    else
    {
        CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + element;
        vis = GMLib.GetMaterialByIdx(T->material)->fVisTransparencyFactor;
    }
    return vis;
}

struct CVisibleObjectPredicateEx
{
    const CObject* m_object;

    CVisibleObjectPredicateEx(const CObject* object) : m_object(object) {}

    bool operator()(const MemorySpace::CVisibleObject& visible_object) const
    {
        if (!m_object)
            return (!visible_object.m_object);
        if (!visible_object.m_object)
            return (false);
        return (m_object->ID() == visible_object.m_object->ID());
    }

    bool operator()(const MemorySpace::CNotYetVisibleObject& not_yet_visible_object) const
    {
        if (!m_object)
            return (!not_yet_visible_object.m_object);
        if (!not_yet_visible_object.m_object)
            return (false);
        return (m_object->ID() == not_yet_visible_object.m_object->ID());
    }
};

// NLC: per-NPC vision profile (nlc_stealth_set_vision); same Load flags as reload()
bool CVisualMemoryManager::nlc_set_vision_sections(LPCSTR free_section, LPCSTR danger_section)
{
    if (!m_object)
        return false;
    const bool not_a_stalker = m_stalker ? true : !!m_client;
    if (free_section && *free_section)
    {
        if (!pSettings->section_exist(free_section))
            return false;
        m_free.Load(free_section, not_a_stalker);
    }
    if (danger_section && *danger_section)
    {
        if (!pSettings->section_exist(danger_section))
            return false;
        m_danger.Load(danger_section, not_a_stalker);
    }
    return true;
}

// NLC: actor sum / threshold (0..1) for suspicion stages; a harness-forced value counts until it expires
float CVisualMemoryManager::nlc_suspicion()
{
    float value = 0.f;
    if (g_actor)
        if (const CNotYetVisibleObject* entry = not_yet_visible_object(g_actor))
        {
            const float threshold = visibility_threshold();
            if (threshold > 0.f)
                value = entry->m_value / threshold;
        }
    if (m_nlc_forced_until && Device.dwTimeGlobal < m_nlc_forced_until)
        value = _max(value, m_nlc_forced);
    if (Device.dwTimeGlobal < m_nlc_heard_until) // NLC M3: heard concern (faint shot, near miss)
        value = _max(value, m_nlc_heard);
    clamp(value, 0.f, 1.f);
    return value;
}

void CVisualMemoryManager::remove_links(CObject* object)
{
    {
        VERIFY(m_objects);
        VISIBLES::iterator I = std::find_if(m_objects->begin(), m_objects->end(), CVisibleObjectPredicateEx(object));
        if (I != m_objects->end())
            m_objects->erase(I);
    }
    {
        NOT_YET_VISIBLES::iterator I = std::find_if(m_not_yet_visible_objects.begin(), m_not_yet_visible_objects.end(), CVisibleObjectPredicateEx(object));
        if (I != m_not_yet_visible_objects.end())
            m_not_yet_visible_objects.erase(I);
    }
}

CVisibleObject* CVisualMemoryManager::visible_object(const CGameObject* game_object)
{
    VISIBLES::iterator I = std::find_if(m_objects->begin(), m_objects->end(), CVisibleObjectPredicateEx(game_object));
    if (I == m_objects->end())
        return (0);
    return (&*I);
}

squad_mask_type CVisualMemoryManager::mask() const
{
    if (!m_stalker)
        return (squad_mask_type(-1));

    return (m_stalker->agent_manager().member().mask(m_stalker));
}

void CVisualMemoryManager::update(float time_delta)
{
    START_PROFILE("Memory Manager/visuals/update")

    clear_delayed_objects();

    if (!enabled())
        return;

    m_last_update_time = Device.dwTimeGlobal;

    squad_mask_type mask = this->mask();
    VERIFY(m_objects);
    m_visible_objects.clear();

    START_PROFILE("Memory Manager/visuals/update/feel_vision_get")
    if (m_object)
        m_object->feel_vision_get(m_visible_objects);
    else
    {
        VERIFY(m_client);
        m_client->feel_vision_get(m_visible_objects);
    }
    STOP_PROFILE

    START_PROFILE("Memory Manager/visuals/update/make_invisible")
    {
        auto I = m_objects->begin();
        auto E = m_objects->end();
        for (; I != E; ++I)
            if ((*I).m_level_time + current_state().m_still_visible_time < Device.dwTimeGlobal)
                (*I).visible(mask, false);
    }
    STOP_PROFILE

    START_PROFILE("Memory Manager/visuals/update/add_visibles")
    {
        xr_vector<CObject*>::const_iterator I = m_visible_objects.begin();
        xr_vector<CObject*>::const_iterator E = m_visible_objects.end();
        for (; I != E; ++I)
            add_visible_object(*I, time_delta);

        m_visible_objects.clear();
    }
    STOP_PROFILE

    // NLC: stealth diagnostics - the actor was not evaluated this update, so the loop below resets (or decays) its sum
    const CNotYetVisibleObject* nlc_actor_entry = nullptr;
    float nlc_sum_before = 0.f;
    bool nlc_skipped = false;
    if (nlc_stealth::g_track && m_object && g_actor && nlc_stealth::tracked(m_object->ID()))
    {
        nlc_actor_entry = not_yet_visible_object(g_actor);
        nlc_skipped = !nlc_actor_entry || nlc_actor_entry->m_update_time < Device.dwTimeGlobal;
        nlc_sum_before = nlc_actor_entry ? nlc_actor_entry->m_value : 0.f;
    }

    START_PROFILE("Memory Manager/visuals/update/make_not_yet_visible")
    {
        xr_vector<CNotYetVisibleObject>::iterator I = m_not_yet_visible_objects.begin();
        xr_vector<CNotYetVisibleObject>::iterator E = m_not_yet_visible_objects.end();
        for (; I != E; ++I)
            if ((*I).m_update_time < Device.dwTimeGlobal)
            {
                // NLC: stalkers keep a fading actor sum instead of the instant reset (nlc_stealth mem_hold_ms, mem_decay_s; off by default)
                if (nlc_stealth::g_memory_decay && m_stalker && g_actor && (*I).m_object == g_actor)
                    (*I).m_value = nlc_stealth::forget_actor_sum((*I).m_value, current_state().m_visibility_threshold, Device.dwTimeGlobal - (*I).m_update_time, time_delta);
                else if (nlc_stealth::g_monster_memory_decay && !m_stalker && m_object && g_actor && (*I).m_object == g_actor) // NLC: monsters
                    (*I).m_value =
                        nlc_stealth::forget_actor_sum((*I).m_value, current_state().m_visibility_threshold, Device.dwTimeGlobal - (*I).m_update_time, time_delta, true);
                else
                    (*I).m_value = 0.f;
            }
    }
    STOP_PROFILE

    if (nlc_skipped) // NLC
        nlc_stealth::on_vision_skipped(m_object, visible_transparency_threshold(g_actor), transparency_threshold(), nlc_sum_before,
                                       nlc_actor_entry ? nlc_actor_entry->m_value : 0.f);

    START_PROFILE("Memory Manager/visuals/update/removing_offline")
    // verifying if object is online
    {
        m_objects->erase(std::remove_if(m_objects->begin(), m_objects->end(), SRemoveOfflinePredicate()), m_objects->end());
    }

    if (m_adaptive_max_object_count && m_objects->size() > m_max_object_count)
    {
        m_objects->erase(std::remove_if(m_objects->begin() + m_max_object_count, m_objects->end(),
                                        [&](const auto& it) -> bool { return it.m_level_time + m_adaptive_max_object_count < Device.dwTimeGlobal; }),
                         m_objects->end());
    }

    // verifying if object is online
    {
        m_not_yet_visible_objects.erase(std::remove_if(m_not_yet_visible_objects.begin(), m_not_yet_visible_objects.end(), SRemoveOfflinePredicate()),
                                        m_not_yet_visible_objects.end());
    }
    STOP_PROFILE

#if 0 // def DEBUG
	if (m_stalker) {
		CAgentMemberManager::MEMBER_STORAGE::const_iterator	I = m_stalker->agent_manager().member().members().begin();
		CAgentMemberManager::MEMBER_STORAGE::const_iterator	E = m_stalker->agent_manager().member().members().end();
		for ( ; I != E; ++I)
			(*I)->object().memory().visual().check_visibles();
	}
#endif

    if (m_object && g_actor)
    {
        if (m_object->is_relation_enemy(Actor()))
        {
            xr_vector<CNotYetVisibleObject>::iterator I = std::find_if(m_not_yet_visible_objects.begin(), m_not_yet_visible_objects.end(), CNotYetVisibleObjectPredicate(Actor()));
            if (I != m_not_yet_visible_objects.end())
            {
                Actor()->SetActorVisibility(m_object->ID(), clampr((*I).m_value / visibility_threshold(), 0.f, 1.f));
            }
            else
                Actor()->SetActorVisibility(m_object->ID(), 0.f);
        }
        else
            Actor()->SetActorVisibility(m_object->ID(), 0.f);
    }

    STOP_PROFILE
}

void CVisualMemoryManager::save(NET_Packet& packet) const
{
    if (m_client)
        return;

    if (!m_object->g_Alive())
        return;

    packet.w_u8((u8)objects().size());

    VISIBLES::const_iterator I = objects().begin();
    VISIBLES::const_iterator E = objects().end();
    for (; I != E; ++I)
    {
        VERIFY((*I).m_object);
        packet.w_u16((*I).m_object->ID());
        // object params
        packet.w_u32((*I).m_object_params.m_level_vertex_id);
        packet.w_vec3((*I).m_object_params.m_position);
#ifdef USE_ORIENTATION
        packet.w_float((*I).m_object_params.m_orientation.yaw);
        packet.w_float((*I).m_object_params.m_orientation.pitch);
        packet.w_float((*I).m_object_params.m_orientation.roll);
#endif // USE_ORIENTATION
       // self params
        packet.w_u32((*I).m_self_params.m_level_vertex_id);
        packet.w_vec3((*I).m_self_params.m_position);
#ifdef USE_ORIENTATION
        packet.w_float((*I).m_self_params.m_orientation.yaw);
        packet.w_float((*I).m_self_params.m_orientation.pitch);
        packet.w_float((*I).m_self_params.m_orientation.roll);
#endif // USE_ORIENTATION
#ifdef USE_LEVEL_TIME
        packet.w_u32((Device.dwTimeGlobal >= (*I).m_level_time) ? (Device.dwTimeGlobal - (*I).m_level_time) : 0);
#endif // USE_LAST_LEVEL_TIME
#ifdef USE_LEVEL_TIME
        packet.w_u32((Device.dwTimeGlobal >= (*I).m_level_time) ? (Device.dwTimeGlobal - (*I).m_last_level_time) : 0);
#endif // USE_LAST_LEVEL_TIME
#ifdef USE_FIRST_LEVEL_TIME
        packet.w_u32((Device.dwTimeGlobal >= (*I).m_level_time) ? (Device.dwTimeGlobal - (*I).m_first_level_time) : 0);
#endif // USE_FIRST_LEVEL_TIME
        packet.w_u64((*I).m_visible.flags);
    }
}

void CVisualMemoryManager::load(IReader& packet)
{
    if (m_client)
        return;

    if (!m_object->g_Alive())
        return;

    auto callback = fastdelegate::MakeDelegate(&m_object->memory(), &CMemoryManager::on_requested_spawn);

    int count = packet.r_u8();
    for (int i = 0; i < count; ++i)
    {
        CDelayedVisibleObject delayed_object;
        delayed_object.m_object_id = packet.r_u16();

        CVisibleObject& object = delayed_object.m_visible_object;
        object.m_object = smart_cast<CGameObject*>(Level().Objects.net_Find(delayed_object.m_object_id));
        // object params
        object.m_object_params.m_level_vertex_id = packet.r_u32();
        packet.r_fvector3(object.m_object_params.m_position);
#ifdef USE_ORIENTATION
        packet.r_float(object.m_object_params.m_orientation.yaw);
        packet.r_float(object.m_object_params.m_orientation.pitch);
        packet.r_float(object.m_object_params.m_orientation.roll);
#endif
        // self params
        object.m_self_params.m_level_vertex_id = packet.r_u32();
        packet.r_fvector3(object.m_self_params.m_position);
#ifdef USE_ORIENTATION
        packet.r_float(object.m_self_params.m_orientation.yaw);
        packet.r_float(object.m_self_params.m_orientation.pitch);
        packet.r_float(object.m_self_params.m_orientation.roll);
#endif
#ifdef USE_LEVEL_TIME
        VERIFY(Device.dwTimeGlobal >= object.m_level_time);
        object.m_level_time = packet.r_u32();
        object.m_level_time = Device.dwTimeGlobal >= object.m_level_time ? Device.dwTimeGlobal - object.m_level_time : 0;
        object.m_first_level_time = Device.dwTimeGlobal;
#endif // USE_LEVEL_TIME
#ifdef USE_LAST_LEVEL_TIME
        VERIFY(Device.dwTimeGlobal >= object.m_last_level_time);
        object.m_last_level_time = packet.r_u32();
        object.m_last_level_time = Device.dwTimeGlobal >= object.m_last_level_time ? Device.dwTimeGlobal - object.m_last_level_time : 0;
#endif // USE_LAST_LEVEL_TIME
#ifdef USE_FIRST_LEVEL_TIME
        VERIFY(Device.dwTimeGlobal >= (*I).m_first_level_time);
        object.m_first_level_time = packet.r_u32();
        object.m_first_level_time = Device.dwTimeGlobal >= object.m_first_level_time ? Device.dwTimeGlobal - (*I).m_first_level_time : 0;
#endif // USE_FIRST_LEVEL_TIME
        object.m_visible.assign(ai().get_alife()->header().version() < 8 ? (squad_mask_type)packet.r_u32() : packet.r_u64());

        if (object.m_object)
        {
            add_visible_object(object);
            continue;
        }

        m_delayed_objects.push_back(delayed_object);

        const CClientSpawnManager::CSpawnCallback* spawn_callback = Level().client_spawn_manager().callback(delayed_object.m_object_id, m_object->ID());
        if (!spawn_callback || !spawn_callback->m_object_callback)
            Level().client_spawn_manager().add(delayed_object.m_object_id, m_object->ID(), callback);
#ifdef DEBUG
        else
        {
            if (spawn_callback && spawn_callback->m_object_callback)
            {
                VERIFY(spawn_callback->m_object_callback == callback);
            }
        }
#endif // DEBUG
    }
}

void CVisualMemoryManager::clear_delayed_objects()
{
    if (m_client)
        return;

    if (m_delayed_objects.empty())
        return;

    CClientSpawnManager& manager = Level().client_spawn_manager();
    DELAYED_VISIBLE_OBJECTS::const_iterator I = m_delayed_objects.begin();
    DELAYED_VISIBLE_OBJECTS::const_iterator E = m_delayed_objects.end();
    for (; I != E; ++I)
        if (manager.callback((*I).m_object_id, m_object->ID()))
            manager.remove((*I).m_object_id, m_object->ID());

    m_delayed_objects.clear();
}

void CVisualMemoryManager::on_requested_spawn(CObject* object)
{
    DELAYED_VISIBLE_OBJECTS::iterator I = m_delayed_objects.begin();
    DELAYED_VISIBLE_OBJECTS::iterator E = m_delayed_objects.end();
    for (; I != E; ++I)
    {
        if ((*I).m_object_id != object->ID())
            continue;

        if (m_object->g_Alive())
        {
            (*I).m_visible_object.m_object = smart_cast<CGameObject*>(object);
            VERIFY((*I).m_visible_object.m_object);
            add_visible_object((*I).m_visible_object);
        }

        m_delayed_objects.erase(I);
        return;
    }
}

float CVisualMemoryManager::visible_transparency_threshold(const CGameObject* game_object) const
{
    if (m_object)
        return m_object->feel_vision_get_transparency(game_object);
    else
    {
        VERIFY(m_client);
        return m_client->feel_vision_get_transparency(game_object);
    }
}
