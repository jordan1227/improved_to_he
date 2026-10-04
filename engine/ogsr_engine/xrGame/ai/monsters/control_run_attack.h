#pragma once
#include "control_combase.h"

class CControlRunAttack : public CControl_ComCustom<>
{
    float m_min_dist{};
    float m_max_dist{};

    u32 m_min_delay{};
    u32 m_max_delay{};

    u32 m_time_next_attack{};

    // -1 = not checked yet, 0 = the visual has no run-attack cycle, 1 = usable
    s8 m_cycle_state{-1};

public:
    virtual void load(LPCSTR section);
    virtual void reinit();

    virtual void on_event(ControlCom::EEventType, ControlCom::IEventData*);
    virtual void activate();
    virtual void on_release();
    virtual bool check_start_conditions();
};
