#pragma once
#include "../state.h"

// NLC: siege of an elevated enemy (docs/DESIGN_monster_elevation_siege.md). The behaviour lives in
// CBaseMonster (base_monster_siege.cpp); this substate only hands the attack state machine over to it.
template <typename _Object>
class CStateMonsterAttackSiege : public CState<_Object>
{
protected:
    typedef CState<_Object> inherited;
    using inherited::object;

public:
    CStateMonsterAttackSiege(_Object* obj) : inherited(obj) { obj->nlc_siege_register(); }

    virtual void initialize()
    {
        inherited::initialize();
        object->nlc_siege_begin();
    }
    virtual void execute() { object->nlc_siege_execute(); }
    virtual void finalize()
    {
        inherited::finalize();
        object->nlc_siege_end();
    }
    virtual void critical_finalize()
    {
        inherited::critical_finalize();
        object->nlc_siege_end();
    }
    virtual void remove_links(CObject* object) { inherited::remove_links(object); }

    virtual bool check_start_conditions() { return object->nlc_siege_wanted(); }
    virtual bool check_completion() { return !object->nlc_siege_wanted(); }
};
