#pragma once

#include "../state.h"

// NLC: a controller thrall goes to a point (flank or guard position) and holds there,
// facing SControlledInfo::m_object when set. The controller updates the point.
template <typename _Object>
class CStateMonsterControlledMove : public CState<_Object>
{
    typedef CState<_Object> inherited;
    using inherited::object;

public:
    CStateMonsterControlledMove(_Object* obj) : inherited(obj) {}
    virtual ~CStateMonsterControlledMove() {}

    virtual void initialize()
    {
        inherited::initialize();
        object->path().prepare_builder();
    }

    virtual void execute()
    {
        CControlledEntityBase* entity = smart_cast<CControlledEntityBase*>(object);
        VERIFY(entity);
        const SControlledInfo& info = entity->get_data();

        if (object->Position().distance_to_xz(info.m_position) > info.m_radius)
        {
            object->path().set_target_point(info.m_position, info.m_node);
            object->path().set_rebuild_time(1000);
            object->path().set_distance_to_end(0.f);
            object->path().set_use_covers(false);
            object->anim().accel_activate(eAT_Aggressive);
            object->anim().accel_set_braking(false);
            object->set_action(ACT_RUN);
        }
        else
        {
            object->set_action(ACT_STAND_IDLE);
            if (info.m_object)
                object->dir().face_target(info.m_object);
        }

        object->set_state_sound(MonsterSound::eMonsterSoundAggressive);
    }

    virtual void remove_links(CObject* O) { inherited::remove_links(O); }
};
