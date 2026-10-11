#pragma once
#include "../state.h"
#include "../../../grenade.h"

template <typename Object>
class CStateBurerAttackTele : public CState<Object>
{
    typedef CState<Object> inherited;
    using inherited::object;

    xr_vector<CPhysicsShellHolder*> tele_objects;
    CPhysicsShellHolder* selected_object;
    xr_vector<CObject*> m_nearest;

    u32 time_started{};

    enum
    {
        ACTION_TELE_STARTED,
        ACTION_TELE_CONTINUE,
        ACTION_TELE_FIRE,
        ACTION_WAIT_FIRE_END,
        ACTION_COMPLETED,
    } m_action;

public:
    CStateBurerAttackTele(Object* obj);

    virtual void initialize();
    virtual void execute();
    virtual void finalize();
    virtual void critical_finalize();
    virtual void remove_links(CObject* O)
    {
        inherited::remove_links(O);
    }

    virtual bool check_start_conditions();
    virtual bool check_completion();

private:
    void FindObjects();

    void ExecuteTeleContinue();
    void ExecuteTeleFire();

    bool IsActiveObjects();

    bool CheckTeleStart();
    void SelectObjects();

    // internal for FindObjects
    void FindFreeObjects(xr_vector<CObject*>& tpObjects, const Fvector& pos);

    // NLC: led throw with the mass-scaled impact damage callback; clear path to the enemy's head
    void FireObject(CPhysicsShellHolder* obj);
    bool IsPathClear(CPhysicsShellHolder* obj);

    void FireAllToEnemy();
    void deactivate();

private:
    TTime m_anim_end_tick;
    bool m_nlc_end_logged{};
    u32 m_nlc_find_next{}; // nothing found: no new search before then (the search is three spatial queries)
    TTime m_end_tick{};
    float m_initial_health{};
};

#include "burer_state_attack_tele_inline.h"
