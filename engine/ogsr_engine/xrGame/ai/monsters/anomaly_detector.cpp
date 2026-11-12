#include "stdafx.h"
#include "anomaly_detector.h"
#include "../../CustomMonster.h"
#include "../../restricted_object.h"
#include "../../level.h"
#include "../../space_restriction_manager.h"
#include "../../movement_manager.h"
#include "../../patrol_path_manager.h"
#include "patrol_path.h"
#include "ai_monster_utils.h"
#include "basemonster/base_monster.h"

CAnomalyDetector::CAnomalyDetector(CCustomMonster* monster) : m_object(monster) {}

CAnomalyDetector::~CAnomalyDetector() {}

void CAnomalyDetector::load(LPCSTR section)
{
    m_radius = READ_IF_EXISTS(pSettings, r_float, section, "Anomaly_Detect_Radius", 15.f);
    m_time_to_rememeber = READ_IF_EXISTS(pSettings, r_u32, section, "Anomaly_Detect_Time_Remember", 30000);
    m_detect_probability = READ_IF_EXISTS(pSettings, r_float, section, "Anomaly_Detect_Probability", 1.f);

    m_ignore_clsids.clear();
    if (pSettings->line_exist(section, "Anomaly_Detect_Ignore"))
    {
        LPCSTR ignore = pSettings->r_string(section, "Anomaly_Detect_Ignore");
        string64 temp;
        for (u32 i = 0, n = _GetItemCount(ignore); i < n; ++i)
            m_ignore_clsids.push_back(TEXT2CLSID(_GetItem(ignore, i, temp)));
    }
}

void CAnomalyDetector::reinit()
{
    m_storage.clear();

    m_active = false;
    m_forced = false;
    m_nlc_suspended = false;
}

void CAnomalyDetector::update_schedule()
{
    if (m_active)
        feel_touch_update(m_object->Position(), m_radius);

    if (m_storage.empty())
        return;

    xr_vector<u16> temp_out_restrictors;
    xr_vector<u16> temp_in_restrictors;

    temp_in_restrictors.reserve(m_storage.size());

    // NLC flytrap: the enemy (monsters only) for the zones entered while it is inside them
    const CEntityAlive* nlc_enemy = nullptr;
    if (CBaseMonster* monster = smart_cast<CBaseMonster*>(m_object))
        nlc_enemy = monster->EnemyMan.get_enemy();

    // add new restrictions
    for (ANOMALY_INFO_VEC_IT it = m_storage.begin(); it != m_storage.end(); it++)
    {
        // NLC flytrap: a zone with monster_follow_enemy is not avoided while the enemy stands in it
        bool enemy_inside = false;
        if (nlc_enemy && !it->ignored)
        {
            CCustomZone* zone = smart_cast<CCustomZone*>(Level().Objects.net_Find(it->id));
            if (zone && !zone->getDestroy() && zone->nlc_follow_enemy())
            {
                Fsphere sphere;
                sphere.set(nlc_enemy->Position(), 0.5f);
                enemy_inside = !!zone->inside(sphere);
            }
        }
        if (enemy_inside && !it->held)
        {
            if (it->time_registered != 0)
            {
                xr_vector<u16> lifted{it->id};
                m_object->movement().restrictions().remove_restrictions(temp_out_restrictors, lifted);
            }
            it->held = true;
            it->follow = true;
            it->time_registered = time();
            if (CBaseMonster::nlc_evade_debug())
                Msg("~ [evade] [%s]: anomaly [%u] entered after the enemy", m_object->cName().c_str(), it->id);
            continue;
        }
        if (enemy_inside && it->held)
            it->follow = true; // learned or suspended meanwhile: it stays lifted for the enemy's sake
        if (it->follow)
        {
            if (enemy_inside)
            {
                it->time_registered = time();
                continue;
            }
            it->follow = false; // back to the ordinary held entry: avoided again once the monster is outside it
        }

        if (it->time_registered == 0)
        {
            it->time_registered = time();
            if (m_nlc_suspended)
                it->held = true; // NLC: noticed during a committed lunge: avoided from its end on
            else
                temp_in_restrictors.push_back(it->id);
        }
        else if (it->held && !m_nlc_suspended)
        {
            // NLC: the lunge is over: avoided again once the monster is outside it
            CCustomZone* zone = smart_cast<CCustomZone*>(Level().Objects.net_Find(it->id));
            Fsphere sphere;
            sphere.set(m_object->Position(), 1.f);
            if (zone && !zone->getDestroy() && zone->inside(sphere))
            {
                it->time_registered = time(); // still in it: stays remembered
                continue;
            }
            it->held = false;
            it->time_registered = time();
            if (zone && !zone->getDestroy())
            {
                temp_in_restrictors.push_back(it->id);
                if (CBaseMonster::nlc_evade_debug())
                    Msg("~ [evade] [%s]: anomaly [%s] avoided again", m_object->cName().c_str(), zone->cName().c_str());
            }
            else
                it->ignored = true; // gone: forgotten by the timer, nothing to remove
        }
    }
    m_object->movement().restrictions().add_restrictions(temp_out_restrictors, temp_in_restrictors);

    // remove old restrictions
    temp_in_restrictors.clear();
    for (ANOMALY_INFO_VEC_IT it = m_storage.begin(); it != m_storage.end(); it++)
    {
        if (it->time_registered + m_time_to_rememeber < time() && !it->ignored && !it->held)
        {
            temp_in_restrictors.push_back(it->id);
        }
    }
    m_object->movement().restrictions().remove_restrictions(temp_out_restrictors, temp_in_restrictors);

    // remove from storage
    m_storage.erase(std::remove_if(m_storage.begin(), m_storage.end(), remove_predicate(m_time_to_rememeber)), m_storage.end());
}

BOOL CAnomalyDetector::feel_touch_contact(CObject* obj)
{
    auto custom_zone = smart_cast<CCustomZone*>(obj);
    if (!custom_zone)
        return FALSE;

    // if its NOT A restrictor - skip
    if (custom_zone->restrictor_type() == RestrictionSpace::eRestrictorTypeNone)
        return FALSE;

    if (!custom_zone->IsEnabled())
        return FALSE;

    if (std::find(m_ignore_clsids.begin(), m_ignore_clsids.end(), obj->CLS_ID) != m_ignore_clsids.end())
        return FALSE;

    on_contact(obj);

    return FALSE;
}

void CAnomalyDetector::on_contact(CObject* obj)
{
    if (obj->getDestroy())
        return;

    auto custom_zone = smart_cast<CCustomZone*>(obj);

    ASSERT_FMT(custom_zone, "[%s]: %s not a CCUstomZone", __FUNCTION__, obj->cName().c_str());

    if (custom_zone->ZoneState() == CCustomZone::eZoneStateDisabled)
        return;

    auto it = std::find_if(m_storage.begin(), m_storage.end(), [custom_zone](const auto& it) { return it.id == custom_zone->ID(); });
    if (it != m_storage.end())
    {
        it->time_registered = time();
        return;
    }

    if (Level().space_restriction_manager().restriction_presented(m_object->movement().restrictions().in_restrictions(), custom_zone->cName()))
        return;

    SAnomalyInfo info;
    info.id = obj->ID();
    // NLC flytrap: a zone can be harder for monsters to notice (zone key monster_detect_k); stalkers always see it
    float probability = m_detect_probability;
    if (smart_cast<CBaseMonster*>(m_object))
        probability *= custom_zone->nlc_detect_k();
    if (!fsimilar(probability, 1.f) && Random.randF() >= probability)
    {
        info.ignored = true;
        info.time_registered = time();
    }
    else if (!patrol_survives(custom_zone))
    {
        // NLC: avoiding this zone would leave the active patrol path without an accessible vertex - remember it as ignored
        info.ignored = true;
        info.time_registered = time();
        if (CBaseMonster::nlc_evade_debug())
            Msg("~ [evade] [%s]: anomaly [%s] noticed, ignored (blocks every vertex of patrol path [%s])", m_object->cName().c_str(), obj->cName().c_str(),
                m_object->movement().patrol().path_name().c_str());
    }
    else
    {
        info.ignored = false;
        info.time_registered = 0;
        if (CBaseMonster::nlc_evade_debug())
            Msg("~ [evade] [%s]: anomaly [%s] noticed, avoided", m_object->cName().c_str(), obj->cName().c_str());
    }
    m_storage.emplace_back(std::move(info));
}

// NLC: true unless the object has a patrol path assigned and every vertex is already inaccessible or inside the zone.
// The path type is not checked: monsters keep the scheme's path while attacking and return to it afterwards.
// A fully blocked patrol path used to be fatal in CPatrolPathManager::select_point (X-18 snork, x18_snork_walk_1).
bool CAnomalyDetector::patrol_survives(CCustomZone* zone) const
{
    CMovementManager& movement = m_object->movement();
    const CPatrolPath* path = movement.patrol().get_path();
    if (!path || path->vertices().empty())
        return true;

    for (const auto& it : path->vertices())
    {
        const Fvector& position = it.second->data().position();
        if (!movement.restrictions().accessible(position))
            continue;

        Fsphere sphere;
        sphere.set(position, EPS_L);
        if (!zone->inside(sphere))
            return true;
    }
    return false;
}

void CAnomalyDetector::activate(bool force)
{
    if (m_forced && !force)
        return;
    m_forced = force;
    m_active = true;
}

void CAnomalyDetector::deactivate(bool force)
{
    if (m_forced && !force)
        return;
    m_forced = force;
    m_active = false;
}

void CAnomalyDetector::remove_all_restrictions()
{
    xr_vector<u16> temp_out_restrictors;
    xr_vector<u16> temp_in_restrictors;

    temp_in_restrictors.reserve(m_storage.size());
    for (ANOMALY_INFO_VEC_IT it = m_storage.begin(); it != m_storage.end(); it++)
        if (!it->held) // NLC: a held one has no restriction now
            temp_in_restrictors.push_back(it->id);
    m_object->movement().restrictions().remove_restrictions(temp_out_restrictors, temp_in_restrictors);
    m_storage.clear();
}

void CAnomalyDetector::remove_restriction(u16 id)
{
    auto it = std::find_if(m_storage.begin(), m_storage.end(), [id](const auto& it) { return it.id == id; });

    if (it != m_storage.end())
    {
        xr_vector<u16> temp_out_restrictors;
        xr_vector<u16> temp_in_restrictors;
        if (!it->held) // NLC
            temp_in_restrictors.push_back(it->id);
        m_object->movement().restrictions().remove_restrictions(temp_out_restrictors, temp_in_restrictors);
        m_storage.erase(it);
    }
}

// NLC polish pass 9: restrictions in force now (noticed, not ignored, not held)
u32 CAnomalyDetector::nlc_avoided() const
{
    u32 n = 0;
    for (const SAnomalyInfo& it : m_storage)
        if (it.time_registered != 0 && !it.ignored && !it.held)
            ++n;
    return n;
}

// NLC flytrap: a monster trapped in a zone it did not notice (and its mates around) now know it. One standing in it gets
// a held entry: the restriction comes when it is outside.
void CAnomalyDetector::nlc_learn(CCustomZone* zone)
{
    if (!zone || zone->getDestroy() || zone->restrictor_type() == RestrictionSpace::eRestrictorTypeNone || !zone->IsEnabled())
        return;
    if (!patrol_survives(zone))
        return;

    Fsphere sphere;
    sphere.set(m_object->Position(), 1.f);
    const bool in_it = !!zone->inside(sphere);

    const u16 id = zone->ID();
    auto it = std::find_if(m_storage.begin(), m_storage.end(), [id](const SAnomalyInfo& info) { return info.id == id; });
    if (it == m_storage.end())
    {
        if (Level().space_restriction_manager().restriction_presented(m_object->movement().restrictions().in_restrictions(), zone->cName()))
            return;
        SAnomalyInfo info;
        info.id = id;
        info.ignored = false;
        info.held = in_it;
        info.time_registered = in_it ? time() : 0;
        m_storage.emplace_back(std::move(info));
        return;
    }
    if (!it->ignored)
        return; // known already
    it->ignored = false;
    it->held = in_it;
    it->time_registered = in_it ? time() : 0;
}

void CAnomalyDetector::nlc_suspend()
{
    m_nlc_suspended = true;
    xr_vector<u16> temp_out_restrictors;
    xr_vector<u16> temp_in_restrictors;
    for (SAnomalyInfo& it : m_storage)
        if (it.time_registered != 0 && !it.ignored && !it.held)
        {
            it.held = true;
            temp_in_restrictors.push_back(it.id);
        }
    if (!temp_in_restrictors.empty())
        m_object->movement().restrictions().remove_restrictions(temp_out_restrictors, temp_in_restrictors);
}

// NLC polish pass 10: is the straight way from..to clear of the volumes of the anomalies avoided now? The volume itself,
// not the restriction around it: a target standing at the edge of an anomaly is inside the restriction margin, and the
// path test refused the line to it (no committed lunge, often no lunge line at all).
bool CAnomalyDetector::nlc_segment_clear(const Fvector& from, const Fvector& to) const
{
    const float len = from.distance_to(to);
    const u32 steps = u32(len / 0.5f) + 1;
    for (const SAnomalyInfo& it : m_storage)
    {
        if (it.time_registered == 0 || it.ignored || it.held)
            continue;
        CCustomZone* zone = smart_cast<CCustomZone*>(Level().Objects.net_Find(it.id));
        if (!zone || zone->getDestroy())
            continue;
        for (u32 i = 0; i <= steps; ++i)
        {
            Fsphere sphere;
            sphere.P.lerp(from, to, float(i) / float(steps));
            sphere.P.y += 0.5f;
            sphere.R = 0.4f;
            if (zone->inside(sphere))
                return false;
        }
    }
    return true;
}
