#pragma once

#include "../../../COMMON_AI/script_export_space.h"

enum ETelekineticState
{
    TS_None,
    TS_Raise,
    TS_Keep,
    TS_Fire,
};

class CGameObject;
class CPhysicsShellHolder;
class CTelekineticObject;
class CPHUpdateObject;
class CTelekinesis;
class CPhysicsElement;
class CTelekineticObject
{
    ETelekineticState state;

public:
    CPhysicsShellHolder* object;
    CTelekinesis* telekinesis;
    float target_height;

    u32 time_keep_started;
    u32 time_keep_updated;
    u32 time_raise_started;

    u32 time_to_keep;

    u32 time_fire_started;

    float strength;

    bool m_rotate;

    ref_sound sound_hold;
    ref_sound sound_throw;

public:
    CTelekineticObject();
    ~CTelekineticObject();

    virtual bool init(CTelekinesis* tele, CPhysicsShellHolder* obj, float s, float h, u32 ttk, bool rot = true);
    virtual bool init_by_id(u16 id, float s, float h, u32 ttk, bool rot = true);
    void set_sound(const ref_sound& snd_hold, const ref_sound& snd_throw);

    virtual void raise(float step);
    virtual void raise_update();

    void prepare_keep();
    virtual void keep();
    virtual void keep_update();
    virtual void release();
    virtual void fire(const Fvector& target, float power);
    void fire_t(const Fvector& target, float time);
    virtual void fire_update();
    virtual void update_state();
    virtual bool can_activate(CPhysicsShellHolder* obj);
    bool is_released() { return state == TS_None; }
    ETelekineticState get_state() { return state; }
    virtual void switch_state(ETelekineticState new_state);
    CPhysicsShellHolder* get_object() { return object; }

    bool check_height();
    bool check_raise_time_out();

    bool time_keep_elapsed();
    bool time_fire_elapsed();

    void enable();

    bool operator==(const CPhysicsShellHolder* obj) { return (object == obj); }

    void rotate();

    // NLC: steered hold (burer barricade / puppet gun): velocity toward m_nlc_target every physics step instead of the
    // height hover, and optionally the object's forward axis (XFORM.k) turned onto m_nlc_dir
    bool m_nlc_steer{};
    Fvector m_nlc_target{};
    float m_nlc_max_speed{6.f};
    float m_nlc_max_accel{10.f}; // m/s^2: moves ease in and out instead of snapping
    float m_nlc_gain{2.5f}; // wanted speed per metre off the target (higher: snaps into place faster)
    bool m_nlc_orient{};
    Fvector m_nlc_dir{};
    Fvector m_nlc_spin{}; // angular velocity while not oriented (a loose tumble)
    u8 m_nlc_axis{2}; // local axis turned onto m_nlc_dir: 0 = XFORM.i, 1 = XFORM.j, 2 = XFORM.k
    bool m_nlc_axis_twoway{}; // either end of the axis may face m_nlc_dir (a barricade's broad side)
    float m_nlc_turn_k{12.f}; // turn rate per radian off the goal, and its cap (rad/s)
    float m_nlc_turn_max{14.f};
    CPhysicsElement* m_nlc_grip{}; // steer only this element; the rest of the shell hangs under gravity
    void nlc_steer();

private:
    void update_hold_sound();

public:
    DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CTelekineticObject)
#undef script_type_list
#define script_type_list save_type_list(CTelekineticObject)
