#pragma once
#include "../state.h"
#include "../../../ai_object_location.h"

// NLC burer brain (docs/BURER_REDESIGN.md 4): a move picked by CBurer::nlc_brain_pick_move (cover, advance, peek)
template <typename Object>
class CStateBurerTactical : public CState<Object>
{
    typedef CState<Object> inherited;
    using inherited::object;

public:
    CStateBurerTactical(Object* obj) : inherited(obj) {}

    virtual void initialize()
    {
        inherited::initialize();
        object->path().prepare_builder();
    }

    virtual void execute()
    {
        object->anim().accel_activate(eAT_Aggressive);
        object->anim().accel_set_braking(false);

        object->path().set_target_point(object->m_nlc_move_target, object->m_nlc_move_node);
        object->path().set_rebuild_time(0);
        object->path().set_distance_to_end(0.f);
        object->path().set_use_covers(false);

        object->set_action(object->m_nlc_move_run ? ACT_RUN : ACT_WALK_FWD);
        object->set_state_sound(MonsterSound::eMonsterSoundAggressive);
    }

    virtual void finalize()
    {
        inherited::finalize();
        object->nlc_on_move_end();
    }

    virtual void critical_finalize()
    {
        // NLC: CState::select_state ends a pre-empted move here only; m_nlc_move_active stayed set
        inherited::critical_finalize();
        object->nlc_on_move_end();
    }
    virtual void remove_links(CObject* O) { inherited::remove_links(O); }

    virtual bool check_start_conditions() { return true; }

    virtual bool check_completion()
    {
        const bool arrived = object->ai_location().level_vertex_id() == object->m_nlc_move_node || object->Position().distance_to_xz(object->m_nlc_move_target) < 1.2f;
        return arrived || Device.dwTimeGlobal >= object->m_nlc_move_until;
    }

};
