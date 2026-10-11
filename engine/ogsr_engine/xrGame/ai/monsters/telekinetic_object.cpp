#include "stdafx.h"
#include "../../physicsshellholder.h"
#include "telekinetic_object.h"
#include "../../PhysicsShell.h"
#include "../../PHInterpolation.h"
#include "../../PHElement.h"
#include "../../level.h"
#include "../../gameobject.h"
#include "../../PhysicsCommon.h"

#define KEEP_IMPULSE_UPDATE 200
#define FIRE_TIME 3000
#define RAISE_MAX_TIME 5000

CTelekineticObject::CTelekineticObject()
{
    state = TS_None;
    object = 0;
    telekinesis = 0;
    m_rotate = false;
}

CTelekineticObject::~CTelekineticObject() {}

bool CTelekineticObject::init(CTelekinesis* tele, CPhysicsShellHolder* obj, float s, float h, u32 ttk, bool rot)
{
    if (!can_activate(obj))
        return false;

    // state				= TS_Raise;
    switch_state(TS_Raise);
    object = obj;

    target_height = obj->Position().y + h;

    time_keep_started = 0;
    time_keep_updated = 0;
    time_to_keep = ttk;

    strength = s;

    time_fire_started = 0;
    // time_raise_started	= Device.dwTimeGlobal;

    m_rotate = rot;

    if (object->m_pPhysicsShell)
        object->m_pPhysicsShell->set_ApplyByGravity(FALSE);

    return true;
}

bool CTelekineticObject::init_by_id(u16 id, float s, float h, u32 ttk, bool rot)
{
    CPhysicsShellHolder* obj = id != u16(-1) ? smart_cast<CPhysicsShellHolder*>(Level().Objects.net_Find(id)) : nullptr;
    if (!obj)
    {
        Msg("[%s] casting to CPhysicsShellHolder FAILED", __FUNCTION__);
        return false;
    }

    return init(nullptr, obj, s, h, ttk, rot);
}

void CTelekineticObject::set_sound(const ref_sound& snd_hold, const ref_sound& snd_throw)
{
    sound_hold.clone(snd_hold, st_Effect, sg_SourceType);
    sound_throw.clone(snd_throw, st_Effect, sg_SourceType);
}

void CTelekineticObject::raise_update()
{
    if (check_height() || check_raise_time_out())
        prepare_keep(); // начать удержание предмета
    // else if (check_raise_time_out()) release();
    else
    {
        if (m_rotate)
            rotate();
    }
}
void CTelekineticObject::keep_update()
{
    if (time_keep_elapsed())
        release();
}
void CTelekineticObject::fire_update()
{
    if (time_fire_elapsed())
        release();
}
void CTelekineticObject::update_state()
{
    switch (get_state())
    {
    case TS_Raise: raise_update(); break;
    case TS_Keep: keep_update(); break;
    case TS_Fire: fire_update(); break;
    case TS_None: break;
    }
}

void CTelekineticObject::switch_state(ETelekineticState new_state)
{
    u32 time = Device.dwTimeGlobal;

    switch (new_state)
    {
    case TS_Raise: time_raise_started = time; break;
    case TS_Keep: time_keep_started = time; break;
    case TS_Fire: time_fire_started = time; break;
    case TS_None: break;
    }
    state = new_state;
}
void CTelekineticObject::nlc_steer()
{
    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    CPhysicsShell* shell = object->m_pPhysicsShell;
    // gripped by one element (a corpse held by the head or the torso): that element is steered, the rest hangs
    CPhysicsElement* grip = m_nlc_grip;
    Fvector pos = object->Position();
    Fvector velocity;
    if (grip)
    {
        Fmatrix m;
        grip->GetGlobalTransformDynamic(&m);
        pos = m.c;
        grip->get_LinearVel(velocity);
    }
    else
        shell->get_LinearVel(velocity);

    // spring toward the target: a wanted velocity (capped), reached with limited acceleration
    Fvector wanted;
    wanted.sub(m_nlc_target, pos);
    wanted.mul(m_nlc_gain);
    const float speed = wanted.magnitude();
    if (speed > m_nlc_max_speed)
        wanted.mul(m_nlc_max_speed / speed);
    Fvector change;
    change.sub(wanted, velocity);
    const float max_change = m_nlc_max_accel * fixed_step;
    const float change_len = change.magnitude();
    if (change_len > max_change)
        change.mul(max_change / change_len);
    velocity.add(change);
    if (grip)
    {
        grip->set_LinearVel(velocity);
        update_hold_sound();
        return;
    }
    shell->set_LinearVel(velocity);

    Fvector spin = m_nlc_spin;
    if (m_nlc_orient && m_nlc_dir.square_magnitude() > EPS)
    {
        const Fmatrix& xf = object->XFORM();
        Fvector forward = m_nlc_axis == 0 ? xf.i : (m_nlc_axis == 1 ? xf.j : xf.k);
        if (m_nlc_axis_twoway && forward.dotproduct(m_nlc_dir) < 0.f)
            forward.invert();
        Fvector axis;
        axis.crossproduct(forward, m_nlc_dir);
        const float s = axis.magnitude();
        const float angle = atan2f(s, forward.dotproduct(m_nlc_dir));
        if (s > EPS_L)
        {
            axis.div(s);
            spin.mul(axis, _min(angle * m_nlc_turn_k, m_nlc_turn_max));
        }
    }
    // the turn eases too
    Fvector turning;
    shell->get_AngularVel(turning);
    turning.lerp(turning, spin, _min(fixed_step * 12.f, 1.f));
    shell->set_AngularVel(turning);
    update_hold_sound();
}

void CTelekineticObject::raise(float step)
{
    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    if (m_nlc_steer)
    {
        nlc_steer();
        return;
    }

    step *= strength;

    Fvector dir;
    dir.set(0.f, 1.0f, 0.f);

    float elem_size = float(object->m_pPhysicsShell->Elements().size());
    dir.mul(elem_size * elem_size * strength);

    (object->m_pPhysicsShell->get_ElementByStoreOrder(0))->applyGravityAccel(dir);

    update_hold_sound();
}

void CTelekineticObject::prepare_keep()
{
    // time_keep_started	= Device.dwTimeGlobal;
    // state				= TS_Keep;
    switch_state(TS_Keep);
    time_keep_updated = 0;
}

bool CTelekineticObject::time_keep_elapsed()
{
    if (time_keep_started + time_to_keep < Device.dwTimeGlobal)
        return true;
    return false;
}

bool CTelekineticObject::time_fire_elapsed()
{
    if (time_fire_started + FIRE_TIME < Device.dwTimeGlobal)
        return true;
    return false;
}

void CTelekineticObject::keep()
{
    // проверить время последнего обновления
    // if (time_keep_updated + KEEP_IMPULSE_UPDATE > Device.dwTimeGlobal) return;

    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    if (m_nlc_steer)
    {
        nlc_steer();
        time_keep_updated = Device.dwTimeGlobal;
        return;
    }

    // проверить высоту
    float cur_h = object->Position().y;

    // установить dir в соответствие с текущей высотой
    Fvector dir;
    if (cur_h > target_height + 0.6f)
        dir.set(0.f, -1.0f, 0.f);
    else if (cur_h < target_height + 0.6f)
        dir.set(0.f, 1.0f, 0.f);
    else
    {
        dir.set(Random.randF(-1.0f, 1.0f), Random.randF(-1.0f, 1.0f), Random.randF(-1.0f, 1.0f));
        dir.normalize_safe();
    }

    // float elem_size = float(object->m_pPhysicsShell->Elements().size());
    dir.mul(5.0f);
    (object->m_pPhysicsShell->get_ElementByStoreOrder(0))->applyGravityAccel(dir);

    // установить время последнего обновления
    time_keep_updated = Device.dwTimeGlobal;

    update_hold_sound();
}

void CTelekineticObject::release()
{
    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    Fvector dir_inv;
    dir_inv.set(0.f, -1.0f, 0.f);

    // включить гравитацию
    object->m_pPhysicsShell->set_ApplyByGravity(TRUE);
    // приложить небольшую силу для того, чтобы объект начал падать
    object->m_pPhysicsShell->applyImpulse(dir_inv, 0.5f * object->m_pPhysicsShell->getMass());

    // state = TS_None;
    switch_state(TS_None);
}

void CTelekineticObject::fire_t(const Fvector& target, float time)
{
    switch_state(TS_Fire);
    // time_fire_started	= Device.dwTimeGlobal;

    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    // включить гравитацию
    object->m_pPhysicsShell->set_ApplyByGravity(TRUE);
    // NLC: the ballistic solution assumes no drag; with the default air resistance throws fell short
    object->m_pPhysicsShell->SetAirResistance(0.f, 0.f);

    Fvector transference;
    transference.sub(target, object->Position());
    TransferenceToThrowVel(transference, time, object->EffectiveGravity());
    object->m_pPhysicsShell->set_LinearVel(transference);

    if (sound_throw._handle())
        sound_throw.play_at_pos(object, object->Position());

    if (sound_hold._handle() && sound_hold._feedback())
        sound_hold.stop();
}
void CTelekineticObject::fire(const Fvector& target, float power)
{
    // state				= TS_Fire;
    switch_state(TS_Fire);
    // time_fire_started	= Device.dwTimeGlobal;

    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    // вычислить направление
    Fvector dir;
    dir.sub(target, object->Position());
    dir.normalize();

    // включить гравитацию
    object->m_pPhysicsShell->set_ApplyByGravity(TRUE);

    // выполнить бросок
    for (u32 i = 0; i < object->m_pPhysicsShell->get_ElementsNumber(); i++)
        object->m_pPhysicsShell->get_ElementByStoreOrder(u16(i))->applyImpulse(dir, power * 20.f * object->m_pPhysicsShell->getMass() / object->m_pPhysicsShell->Elements().size());
};

bool CTelekineticObject::check_height()
{
    if (!object)
        return true;

    return (object->Position().y > target_height);
}
bool CTelekineticObject::check_raise_time_out()
{
    if (time_raise_started + RAISE_MAX_TIME < Device.dwTimeGlobal)
        return true;

    return false;
}

void CTelekineticObject::enable()
{
    if (object->m_pPhysicsShell)
        object->m_pPhysicsShell->Enable();
}

void CTelekineticObject::rotate()
{
    if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->isActive())
        return;

    // вычислить направление
    Fvector dir;
    dir.random_dir();
    dir.normalize();

    object->m_pPhysicsShell->applyImpulse(dir, 2.5f * object->m_pPhysicsShell->getMass());
}

bool CTelekineticObject::can_activate(CPhysicsShellHolder* obj) { return (obj && obj->m_pPhysicsShell); }

void CTelekineticObject::update_hold_sound()
{
    if (!sound_hold._handle())
        return;

    if (sound_hold._feedback())
        sound_hold.set_position(object->Position());
    else
        sound_hold.play_at_pos(object, object->Position());
}

using namespace luabind;

void CTelekineticObject::script_register(lua_State* L)
{
    module(L)[class_<CTelekineticObject>("tele_object")
                  .def("init_by_id", &CTelekineticObject::init_by_id)
                  .def("raise", &CTelekineticObject::raise)
                  .def("keep", &CTelekineticObject::keep)
                  .def("rotate", &CTelekineticObject::rotate)
                  .def("release", &CTelekineticObject::release)
                  .def_readwrite("target_height", &CTelekineticObject::target_height)];
}
