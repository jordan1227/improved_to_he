#pragma once
#include "control_combase.h"

class CControlRunAttack : public CControl_ComCustom<>
{
    float m_min_dist{};
    float m_max_dist{};

    u32 m_min_delay{};
    u32 m_max_delay{};

    u32 m_time_next_attack{};
    float m_overshoot{-1.f}; // NLC: Run_Attack_Overshoot, the lunge line ends this far past the enemy (< 0 = stock: the whole clip at run speed)
    u32 m_cooldown{2200}; // NLC: Run_Attack_Cooldown, ms after a lunge starts before the next may (stock 2200)

    // -1 = not checked yet, 0 = the visual has no run-attack cycle, 1 = usable
    s8 m_cycle_state{-1};
    bool m_nlc_anomaly_commit{}; // NLC: this lunge lifted the anomaly restrictions (restored on release)
    u32 m_nlc_force_until{};
    float m_nlc_force_min{2.f};

public:
    virtual void load(LPCSTR section);
    virtual void reinit();

    virtual void on_event(ControlCom::EEventType, ControlCom::IEventData*);
    virtual void activate();
    virtual void on_release();
    virtual bool check_start_conditions();

    // NLC: bloodsucker tactics (lunge ready) and cloak radius; the same floors as check_start_conditions
    u32 time_next_attack() const { return m_time_next_attack; }
    // NLC polish pass: until `until` the lunge ignores its cooldown and the moving test, and may start from `dmin`
    void nlc_force(u32 until, float dmin)
    {
        m_nlc_force_until = until;
        m_nlc_force_min = dmin;
    }
    float nlc_dist_min() const { return m_min_dist < 2.5f ? 2.5f : m_min_dist; }
    float nlc_dist_max() const
    {
        const float dmin = nlc_dist_min();
        float dmax = (m_max_dist < dmin + 0.5f) ? dmin + 3.f : m_max_dist;
        return dmax > 10.f ? 10.f : dmax;
    }
};
