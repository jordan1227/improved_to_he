#pragma once
#include "../state.h"

// NLC burer pass 5 (docs/BURER_REDESIGN.md 5.13): a timed cast that stands and faces the enemy; the burer owns the
// logic (CBurer::nlc_cast_*): the stare window, the NPC disarm, the pull
template <typename Object>
class CStateBurerCast : public CState<Object>
{
    typedef CState<Object> inherited;
    using inherited::object;

public:
    CStateBurerCast(Object* obj, u32 kind) : inherited(obj), m_kind(kind) {}

    virtual void initialize()
    {
        inherited::initialize();
        object->set_script_capture(false);
        object->nlc_cast_begin(m_kind);
    }

    virtual void execute() { object->nlc_cast_execute(m_kind); }

    virtual void finalize()
    {
        object->nlc_cast_end(m_kind, false);
        inherited::finalize();
        object->set_script_capture(true);
    }

    virtual void critical_finalize()
    {
        object->nlc_cast_end(m_kind, true);
        inherited::critical_finalize();
        object->set_script_capture(false);
    }

    virtual void remove_links(CObject* O) { inherited::remove_links(O); }

    virtual bool check_start_conditions() { return object->nlc_cast_ready(m_kind); }
    virtual bool check_completion() { return object->nlc_cast_done(m_kind); }

private:
    u32 m_kind;
};
