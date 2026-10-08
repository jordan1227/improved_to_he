#pragma once
#include "../state.h"

// NLC: species tactic substate (bloodsucker hit-and-run, stalk, feint, bait; docs/DESIGN_monster_movement_under_fire.md 20).
// The behaviour lives in the species class (bloodsucker_tactics.cpp); this substate only hands the attack state machine over
// to its nlc_tactic_* hooks. Unlike the siege substate it needs no registration: only species that override
// nlc_tactic_wanted() ever select it.
template <typename _Object>
class CStateMonsterAttackTactic : public CState<_Object>
{
protected:
    typedef CState<_Object> inherited;
    using inherited::object;

public:
    CStateMonsterAttackTactic(_Object* obj) : inherited(obj) {}

    virtual void initialize()
    {
        inherited::initialize();
        object->nlc_tactic_begin();
    }
    virtual void execute() { object->nlc_tactic_execute(); }
    virtual void finalize()
    {
        inherited::finalize();
        object->nlc_tactic_end();
    }
    virtual void critical_finalize()
    {
        inherited::critical_finalize();
        object->nlc_tactic_end();
    }
    virtual void remove_links(CObject* object) { inherited::remove_links(object); }

    virtual bool check_start_conditions() { return object->nlc_tactic_wanted(); }
    virtual bool check_completion() { return !object->nlc_tactic_wanted(); }
};
