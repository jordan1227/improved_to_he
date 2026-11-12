#pragma once
#include "..\xr_3da\feel_touch.h"
#include "../../customzone.h"

class CCustomMonster;

class CAnomalyDetector : public Feel::Touch
{
    CCustomMonster* m_object;

public:
    float m_radius;
    u32 m_time_to_rememeber;
    float m_detect_probability;

    bool m_active;
    bool m_forced;

private:
    xr_vector<CLASS_ID> m_ignore_clsids;
    struct SAnomalyInfo
    {
        u16 id;
        bool ignored;
        u32 time_registered;
        bool held{}; // NLC: noticed, but its restriction is lifted (a committed lunge) until the monster is outside it
        bool follow{}; // NLC flytrap: held because the enemy is inside it (zone key monster_follow_enemy)
    };

    struct remove_predicate
    {
        u32 time_remember;
        remove_predicate(u32 time) : time_remember(time) {}

        IC bool operator()(const SAnomalyInfo& info) { return (!info.held && info.time_registered + time_remember < Device.dwTimeGlobal); }
    };

    using ANOMALY_INFO_VEC = xr_vector<SAnomalyInfo>;
    using ANOMALY_INFO_VEC_IT = ANOMALY_INFO_VEC::iterator;
    ANOMALY_INFO_VEC m_storage;

    bool patrol_survives(CCustomZone* zone) const;

public:
    CAnomalyDetector(CCustomMonster* monster);
    virtual ~CAnomalyDetector();

    void load(LPCSTR section);
    void reinit();

    void update_schedule();
    void on_contact(CObject*);
    virtual BOOL feel_touch_contact(CObject*);

    void activate(bool = false);
    void deactivate(bool = false);
    void remove_all_restrictions();
    void remove_restriction(u16);

    // NLC polish pass 9: a committed lunge / pounce ignores the noticed anomalies; each restriction comes back once
    // the monster is outside that anomaly (it could not path out of a restriction it stands in)
    void nlc_suspend();
    void nlc_resume() { m_nlc_suspended = false; }
    bool nlc_suspended() const { return m_nlc_suspended; }
    u32 nlc_avoided() const;
    bool nlc_segment_clear(const Fvector& from, const Fvector& to) const; // no avoided anomaly volume on the segment
    void nlc_learn(CCustomZone* zone); // NLC flytrap: the zone becomes known and avoided, whatever the detection roll said

private:
    bool m_nlc_suspended{};
};
