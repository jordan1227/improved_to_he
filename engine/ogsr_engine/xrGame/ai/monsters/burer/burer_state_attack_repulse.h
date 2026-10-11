#pragma once
#include "../state.h"

// NLC burer pass 1 (docs/BURER_REDESIGN.md 5.2): close-range telekinetic blast instead of only running away
template <typename Object>
class CStateBurerRepulse : public CState<Object>
{
    typedef CState<Object> inherited;
    using inherited::object;

public:
    CStateBurerRepulse(Object* obj);

    virtual void initialize();
    virtual void execute();
    virtual void finalize();
    virtual void critical_finalize();
    virtual void remove_links(CObject* object) { inherited::remove_links(object); }

    virtual bool check_start_conditions();
    virtual bool check_completion();

private:
    TTime m_blast_tick{};
    TTime m_end_tick{};
    bool m_blast_done{};
};

#include "burer_state_attack_repulse_inline.h"
