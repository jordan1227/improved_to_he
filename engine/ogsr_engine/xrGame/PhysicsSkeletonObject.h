#pragma once

#include "physicsshellholder.h"
#include "PHSkeleton.h"

class CSE_ALifePHSkeletonObject;
class CPhysicsSkeletonObject : public CPhysicsShellHolder, public CPHSkeleton

{
    typedef CPhysicsShellHolder inherited;

public:
    CPhysicsSkeletonObject(void);
    virtual ~CPhysicsSkeletonObject(void);

    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Destroy();
    virtual void Load(LPCSTR section);
    virtual void UpdateCL(); // Called each frame, so no need for dt
    virtual void shedule_Update(u32 dt); //
    virtual void net_Save(NET_Packet& P);
    virtual BOOL net_SaveRelevant();
    virtual BOOL UsedAI_Locations();

    // NLC: collision damage of a telekinetic throw (telekinesis_hit_callback.h), owned as in CPhysicObject
    virtual ICollisionHitCallback* get_collision_hit_callback() { return m_collision_hit_callback; }
    virtual void set_collision_hit_callback(ICollisionHitCallback* cc);

private:
    ICollisionHitCallback* m_collision_hit_callback{};

protected:
    virtual CPhysicsShellHolder* PPhysicsShellHolder() { return PhysicsShellHolder(); }
    virtual CPHSkeleton* PHSkeleton() { return this; }
    virtual void SpawnInitPhysics(CSE_Abstract* D);
    virtual void PHObjectPositionUpdate();
    virtual void CreatePhysicsShell(CSE_Abstract* e);
};