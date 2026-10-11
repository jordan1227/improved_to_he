#pragma once

#include "../../PhysicsShellHolder.h"
#include "../../PhysicObject.h"
#include "../../IColisiondamageInfo.h"

// Collision damage of a telekinetically thrown object (moved from poltergeist_telekinesis.cpp for the burer,
// docs/BURER_REDESIGN.md 5.2). The callback reaches the hit character through its collision-damage info and replaces
// the stock speed-based health loss by a fixed hit once the contact speed exceeds half the character's min crash speed.
struct SCollisionHitCallback : public ICollisionHitCallback
{
    CPhysicsShellHolder* m_object;
    float m_pmt_object_collision_damage;
    u32 m_expire_time; // NLC: 0 = never; otherwise no hit after this Device.dwTimeGlobal (a missed throw stays harmless)

    bool done = false;

    SCollisionHitCallback(CPhysicsShellHolder* object, float pmt_object_collision_damage, u32 expire_time = 0)
        : m_object(object), m_pmt_object_collision_damage(pmt_object_collision_damage), m_expire_time(expire_time)
    {
        VERIFY(object);
    }
    void call(CPhysicsShellHolder* obj, float min_cs, float max_cs, float& cs, float& hl, ICollisionDamageInfo* di) override
    {
        if (done)
            return;

        if (m_expire_time && Device.dwTimeGlobal > m_expire_time)
        {
            done = true;
            return;
        }

        if (cs > min_cs * 0.5f)
            hl = m_pmt_object_collision_damage;

        di->SetInitiated();

        done = true;
    }
};

// NLC: attaches the callback (CPhysicsShellHolder stores it for every object, inventory items too); the allocation is
// read back and freed if the object did not keep it.
inline bool nlc_tele_set_hit_callback(CPhysicsShellHolder* obj, float damage, u32 expire_time)
{
    if (!obj || damage <= 0.f)
        return false;
    SCollisionHitCallback* callback = xr_new<SCollisionHitCallback>(obj, damage, expire_time);
    obj->set_collision_hit_callback(callback);
    if (obj->get_collision_hit_callback() != callback)
    {
        xr_delete(callback);
        return false;
    }
    return true;
}
