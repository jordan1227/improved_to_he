#pragma once

// NLC: gunplay feel (docs/GUNPLAY_FEEL_DESIGN.md).
// - damped springs for HUD motion (jump/fall/landing, rotational and movement inertia), used by CHudItem;
// - actor landing events (every ground contact after a fall), for the HUD and for Lua;
// - mouse-look rate, so HUD inertia follows the player's own turning and not scripted recoil;
// - an aim-neutral camera shot punch (roll + FOV spring effector) triggered from Lua.

struct lua_State;

namespace nlc_gunplay
{
// x'' = -2*zeta*omega*x' - omega^2*(x - target), sub-stepped semi-implicit Euler (stable at low FPS).
// omega <= 0 snaps to the target.
void spring_step(Fvector& x, Fvector& v, const Fvector& target, float omega, float zeta, float dt);

void on_actor_land(float contact_speed); // Actor_Movement.cpp, at the eOnActorLand callback
u32 land_count();
float land_speed();
u32 land_time();

void add_look_input(float abs_yaw, float abs_pitch); // ActorInput.cpp, mouse look after scaling (radians)
// Mouse-look angular speed (rad/s) over the current frame; evaluated once per frame.
void look_rate(float& yaw_rate, float& pitch_rate);

void shot_punch(float roll_rad, float fov_deg);

// HUD clip player: authored additive .anm clips evaluated by the engine, summed so repeats stack.
void clip_play(LPCSTR name, float power, float speed, float mirror, float aim_k);
bool clip_xform(bool aiming, float item_k, Fmatrix& out); // false = no clip active
void set_crouch_clips(LPCSTR down, LPCSTR up, float power, float speed, float aim_k);
void on_stance(u8 stance, float power_k, float speed_k); // 0 stand, 1 crouch, 2 low crouch; once per frame

// Shared HUD impulse spring (hits, explosions, stomps from Lua).
void hud_impulse(const Fvector& pos, const Fvector& rot);
void fx_sample(Fvector& pos, Fvector& rot);

// Live modifiers from Lua: HUD spring speed and inertia amplitude.
void set_motion_mod(float freq_k, float amp_k);
float motion_freq_k();
float motion_amp_k();

void clear(); // CLevel construction and destruction

// Lua-tunable engine values (nlc_gunplay.set_param); order matches s_params in nlc_gunplay.cpp.
enum EParam
{
    eAdsOmegaK,
    ePdaShare,
    ePdaTurnShare,
    ePdaUseShare,
    eMissileShare,
    eFxOmega,
    eFxZeta,
    eFxAimK,
};
float param(EParam id);
bool set_param(LPCSTR name, float value);
void script_register(lua_State* L);
} // namespace nlc_gunplay
