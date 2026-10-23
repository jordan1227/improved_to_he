#include "stdafx.h"
#include "nlc_gunplay.h"
#include "Actor.h"
#include "ActorEffector.h"
#include "CameraEffector.h"
#include "../xr_3da/Effector.h"
#include "../xr_3da/ObjectAnimator.h"
#include "PostprocessAnimator.h"

// Lua:
//   nlc_gunplay.shot_punch(roll_deg, fov_deg)  -- aim-neutral camera kick; roll is signed, fov < 0 narrows
//   count, speed, time_ms = nlc_gunplay.land()  -- last ground contact after a fall (count increments)
//   nlc_gunplay.hud_clip(anm, power, speed, mirror, aim_k)  -- additive HUD clip; several stack; mirror < 0 = left/right flip
//   nlc_gunplay.hud_impulse(px, py, pz, rh, rp, rb)  -- kick the shared HUD spring (metres / degrees, *_hud_offset convention)
//   nlc_gunplay.set_motion_mod(freq_k, amp_k)  -- live HUD spring speed and inertia size (stamina, exo, injury)
//   nlc_gunplay.set_crouch_clips(down_anm, up_anm, power, speed, aim_k)  -- clips the engine plays on stance changes
//   ok = nlc_gunplay.set_param(name, value)  -- engine tunables, see s_params (ads_omega_k, pda_share, ...)
// Neither changes the camera direction, so the point of aim and bullets are unaffected.

#define eCENlcShotPunch ((ECamEffectorType)(cefNext + 31))

extern float g_fov; // Actor.cpp
extern float g_3dss_lens_ppe; // Weapon.cpp
extern float g_3dss_lens_ppe_speed; // Weapon.cpp
extern float g_3dss_lens_ppe_delay; // Weapon.cpp
extern float g_3dss_lens_close_ppe; // Weapon.cpp
extern int ps_r_3dss_crop_debug; // xrRender_console.cpp

namespace nlc_gunplay
{
namespace
{
u32 s_land_count{};
float s_land_speed{};
u32 s_land_time{};

float s_look_yaw{}, s_look_pitch{};
u32 s_look_frame{u32(-1)};
float s_look_yaw_rate{}, s_look_pitch_rate{};

// Roll rings briefly (under-damped); FOV is a near-critical punch. Peak displacement of an
// impulse v0 into a critically damped spring is v0 / (omega * e), so kicks are scaled by that.
constexpr float PUNCH_ROLL_OMEGA = 30.f;
constexpr float PUNCH_ROLL_ZETA = 0.42f;
constexpr float PUNCH_FOV_OMEGA = 34.f;
constexpr float PUNCH_FOV_ZETA = 0.85f;
constexpr float PUNCH_MAX_ROLL = 4.f * (PI / 180.f);
constexpr float PUNCH_MAX_FOV = 6.f;
constexpr float E_CONST = 2.7182818f;

void scalar_step(float& x, float& v, float omega, float zeta, float dt)
{
    clamp(dt, 0.f, 0.1f);
    int n = iCeil(dt * omega / 0.25f);
    clamp(n, 1, 32);
    const float h = dt / float(n);
    for (int i = 0; i < n; ++i)
    {
        v += (-2.f * zeta * omega * v - omega * omega * x) * h;
        x += v * h;
    }
}

class CNlcShotPunch : public CEffectorCam
{
public:
    float roll{}, roll_v{}, fov{}, fov_v{};

    CNlcShotPunch() : CEffectorCam(eCENlcShotPunch, 1000.f) {}

    BOOL ProcessCam(SCamEffectorInfo& info) override
    {
        const float dt = Device.fTimeDelta;
        scalar_step(roll, roll_v, PUNCH_ROLL_OMEGA, PUNCH_ROLL_ZETA, dt);
        scalar_step(fov, fov_v, PUNCH_FOV_OMEGA, PUNCH_FOV_ZETA, dt);
        clamp(roll, -PUNCH_MAX_ROLL, PUNCH_MAX_ROLL);
        clamp(fov, -PUNCH_MAX_FOV, PUNCH_MAX_FOV);

        if (_abs(roll) < 0.00002f && _abs(roll_v) < 0.0005f && _abs(fov) < 0.002f && _abs(fov_v) < 0.05f)
        {
            fLifeTime = -1.f; // released by the camera manager
            return FALSE;
        }

        if (!fis_zero(roll))
        {
            Fmatrix R;
            R.rotation(info.d, roll);
            R.transform_dir(info.n);
            R.transform_dir(info.r);
        }
        info.fFov += fov;
        return TRUE;
    }
};
} // namespace

void spring_step(Fvector& x, Fvector& v, const Fvector& target, float omega, float zeta, float dt)
{
    if (omega <= EPS || dt <= 0.f)
    {
        x = target;
        v.set(0.f, 0.f, 0.f);
        return;
    }
    clamp(dt, 0.f, 0.1f);
    int n = iCeil(dt * omega / 0.25f);
    clamp(n, 1, 32);
    const float h = dt / float(n);
    const float c = 2.f * zeta * omega, k = omega * omega;
    for (int i = 0; i < n; ++i)
    {
        v.x += (-c * v.x - k * (x.x - target.x)) * h;
        v.y += (-c * v.y - k * (x.y - target.y)) * h;
        v.z += (-c * v.z - k * (x.z - target.z)) * h;
        x.mad(v, h);
    }
}

void on_actor_land(float contact_speed)
{
    ++s_land_count;
    s_land_speed = contact_speed;
    s_land_time = Device.dwTimeGlobal;
}

u32 land_count() { return s_land_count; }
float land_speed() { return s_land_speed; }
u32 land_time() { return s_land_time; }

void add_look_input(float abs_yaw, float abs_pitch)
{
    s_look_yaw += abs_yaw;
    s_look_pitch += abs_pitch;
}

void look_rate(float& yaw_rate, float& pitch_rate)
{
    if (s_look_frame != Device.dwFrame)
    {
        s_look_frame = Device.dwFrame;
        const float dt = _max(Device.fTimeDelta, 0.001f);
        s_look_yaw_rate = s_look_yaw / dt;
        s_look_pitch_rate = s_look_pitch / dt;
        s_look_yaw = 0.f;
        s_look_pitch = 0.f;
    }
    yaw_rate = s_look_yaw_rate;
    pitch_rate = s_look_pitch_rate;
}

void shot_punch(float roll_rad, float fov_deg)
{
    if (!g_actor || !_valid(roll_rad) || !_valid(fov_deg))
        return;
    auto* eff = static_cast<CNlcShotPunch*>(g_actor->Cameras().GetCamEffector(eCENlcShotPunch));
    if (!eff)
        eff = static_cast<CNlcShotPunch*>(g_actor->Cameras().AddCamEffector(xr_new<CNlcShotPunch>()));
    if (!eff)
        return;
    eff->roll_v += roll_rad * PUNCH_ROLL_OMEGA * E_CONST;
    // The FOV punch is relative to the current camera FOV: a narrow full-screen scope zoom must not punch harder.
    const float fov_k = g_fov > 1.f ? clampr(Device.fFOV / g_fov, 0.05f, 1.f) : 1.f;
    eff->fov_v += fov_deg * fov_k * PUNCH_FOV_OMEGA * E_CONST;
}

// ---------------------------------------------------------------------------
// Batch 3 (docs/GUNPLAY_FEEL_DESIGN.md 7c): HUD clip player, HUD impulse spring, live motion modifiers.
// ---------------------------------------------------------------------------
namespace
{
struct Clip
{
    CObjectAnimator* anm{};
    float power{}, aim_k{1.f}, mirror{1.f};
    bool busy{};
};
constexpr u32 MAX_CLIPS = 8;
Clip s_clips[MAX_CLIPS];
u32 s_clip_frame{u32(-1)};

shared_str s_crouch_down, s_crouch_up;
float s_crouch_power{}, s_crouch_speed{1.f}, s_crouch_aim_k{1.f};
u8 s_prev_stance{0xff};
u32 s_stance_frame{u32(-1)};

Fvector s_fx_x[2]{}, s_fx_v[2]{};
u32 s_fx_frame{u32(-1)};
bool s_pool_full_logged{};

// Tunables Lua may change without a rebuild (nlc_gunplay.set_param); defaults are the shipped values.
struct ParamDef
{
    LPCSTR name;
    float value, lo, hi;
};
ParamDef s_params[] = {
    {"ads_omega_k", 2.6f, 0.5f, 10.f}, // ADS pose spring: omega = k / ADS time
    {"pda_share", 0.6f, 0.f, 2.f}, // PDA share of the motion layers
    {"pda_turn_share", 0.35f, 0.f, 2.f}, // PDA share of the turn lag
    {"pda_use_share", 0.2f, 0.f, 1.f}, // multiplier while the 3D PDA is zoomed for the cursor
    {"missile_share", 0.7f, 0.f, 2.f}, // grenades and bolts
    {"fx_omega", 16.f, 4.f, 40.f}, // shared impulse spring
    {"fx_zeta", 0.5f, 0.1f, 1.5f},
    {"fx_aim_k", 0.5f, 0.f, 2.f}, // impulse share while aiming
};

float s_freq_k{1.f}, s_amp_k{1.f};

bool clip_finished(const Clip& c)
{
    // A one-shot CObjectAnimator keeps bPlay set and holds its last frame at the end, so test the time
    // (as player_hud does for script layers).
    const SAnimParams& m = c.anm->anim_param();
    return !c.anm->IsPlaying() || m.t_current >= m.max_t;
}

void clips_update()
{
    if (s_clip_frame == Device.dwFrame)
        return;
    s_clip_frame = Device.dwFrame;
    for (Clip& c : s_clips)
    {
        if (!c.busy)
            continue;
        c.anm->Update(Device.fTimeDelta);
        if (clip_finished(c))
        {
            c.anm->Stop();
            c.busy = false;
        }
    }
}

void fx_update()
{
    if (s_fx_frame == Device.dwFrame)
        return;
    s_fx_frame = Device.dwFrame;
    for (int i = 0; i < 2; ++i)
        spring_step(s_fx_x[i], s_fx_v[i], Fvector{}, param(eFxOmega), param(eFxZeta), Device.fTimeDelta);
}
} // namespace

void clip_play(LPCSTR name, float power, float speed, float mirror, float aim_k)
{
    if (!name || !name[0] || !_valid(power) || power <= 0.f)
        return;
    string_path full;
    if (!FS.exist(full, fsgame::level, name) && !FS.exist(full, fsgame::game_anims, name))
    {
        Msg("! [nlc_gunplay] hud clip not found: %s", name);
        return;
    }
    Clip* slot{};
    for (Clip& c : s_clips) // prefer a free slot that already holds this motion
        if (!c.busy && c.anm && !xr_strcmp(c.anm->Name(), name))
        {
            slot = &c;
            break;
        }
    if (!slot)
        for (Clip& c : s_clips)
            if (!c.busy)
            {
                slot = &c;
                break;
            }
    if (!slot) // all slots busy: reuse the clip nearest its end rather than dropping the new one
    {
        float best = -1.f;
        for (Clip& c : s_clips)
        {
            const SAnimParams& m = c.anm->anim_param();
            const float len = m.max_t - m.min_t;
            const float progress = len > EPS ? (m.t_current - m.min_t) / len : 1.f;
            if (progress > best)
            {
                best = progress;
                slot = &c;
            }
        }
        if (!s_pool_full_logged)
        {
            Msg("! [nlc_gunplay] hud clip pool full (%u); reusing the oldest clip for %s", MAX_CLIPS, name);
            s_pool_full_logged = true;
        }
        slot->anm->Stop();
        slot->busy = false;
    }
    if (!slot->anm)
        slot->anm = xr_new<CObjectAnimator>();
    if (xr_strcmp(slot->anm->Name(), name))
        slot->anm->Load(name);
    slot->anm->Speed() = _max(0.05f, speed);
    slot->anm->Play(false);
    slot->power = power;
    slot->aim_k = aim_k;
    slot->mirror = mirror < 0.f ? -1.f : 1.f;
    slot->busy = true;
}

bool clip_xform(bool aiming, float item_k, Fmatrix& out)
{
    clips_update();
    Fquaternion q;
    q.identity();
    Fvector t{};
    bool any{};
    for (Clip& c : s_clips)
    {
        if (!c.busy)
            continue;
        const float k = c.power * (aiming ? c.aim_k : 1.f) * item_k;
        if (fis_zero(k))
            continue;
        const Fmatrix& X = c.anm->XFORM();
        Fquaternion qc;
        qc.set(X);
        float angle;
        Fvector axis;
        if (qc.get_axis_angle(axis, angle))
        {
            if (c.mirror < 0.f) // reflect across the HUD's vertical plane: axial vector (x, -y, -z)
                axis.set(axis.x, -axis.y, -axis.z);
            qc.rotation(axis, angle * k);
        }
        else
            qc.identity();
        Fvector tc{X.c};
        if (c.mirror < 0.f)
            tc.x = -tc.x;
        t.mad(tc, k);
        Fquaternion r;
        r.mul(q, qc);
        q = r;
        any = true;
    }
    if (any)
        out.mk_xform(q, t);
    return any;
}

void set_crouch_clips(LPCSTR down, LPCSTR up, float power, float speed, float aim_k)
{
    s_crouch_down = down;
    s_crouch_up = up;
    s_crouch_power = power;
    s_crouch_speed = speed;
    s_crouch_aim_k = aim_k;
}

void on_stance(u8 stance, float power_k, float speed_k)
{
    if (s_stance_frame == Device.dwFrame)
        return;
    s_stance_frame = Device.dwFrame;
    if (stance == s_prev_stance)
        return;
    const bool first = s_prev_stance == 0xff;
    const bool lower = stance > s_prev_stance;
    s_prev_stance = stance;
    if (first || s_crouch_power <= 0.f)
        return;
    clip_play(lower ? s_crouch_down.c_str() : s_crouch_up.c_str(), s_crouch_power * power_k, s_crouch_speed * speed_k, 1.f, s_crouch_aim_k);
}

void hud_impulse(const Fvector& pos, const Fvector& rot)
{
    const float g = param(eFxOmega) * 2.7182818f; // ~ authored peak (a bit more: under-damped)
    s_fx_v[0].mad(pos, g);
    s_fx_v[1].mad(rot, g);
}

void fx_sample(Fvector& pos, Fvector& rot)
{
    fx_update();
    pos = s_fx_x[0];
    rot = s_fx_x[1];
}

void set_motion_mod(float freq_k, float amp_k)
{
    if (_valid(freq_k))
    {
        s_freq_k = freq_k;
        clamp(s_freq_k, 0.3f, 2.f);
    }
    if (_valid(amp_k))
    {
        s_amp_k = amp_k;
        clamp(s_amp_k, 0.f, 3.f);
    }
}
float motion_freq_k() { return s_freq_k; }

float param(EParam id) { return s_params[id].value; }

bool set_param(LPCSTR name, float value)
{
    if (!name || !_valid(value))
        return false;
    for (ParamDef& d : s_params)
        if (!xr_strcmp(d.name, name))
        {
            d.value = value;
            clamp(d.value, d.lo, d.hi);
            return true;
        }
    Msg("! [nlc_gunplay] unknown param: %s", name);
    return false;
}
float motion_amp_k() { return s_amp_k; }

// ---------------------------------------------------------------------------
// Lens-only post-process (docs/SCOPE_PIP_DESIGN.md 15): a .ppe evaluated without a camera manager. Its base/add
// colour, gray and blur drive the 3D scope lens shader (s3ds_param_8/9), so e.g. the deimos blink covers only the
// lens. Played automatically when the lens image takes over from the glass (and, faster, when it hands back), or from
// Lua for experiments. Evaluated on its own clock (pass 4): speed and delay scale the authored timeline.
// ---------------------------------------------------------------------------
namespace
{
class CLensPPE : public CPostprocessAnimator
{
public:
    CLensPPE() : CPostprocessAnimator(0, false) {}
    // t: position on the authored timeline (seconds)
    BOOL ProcessAt(SPPInfo& info, const float t)
    {
        m_start_time = Device.fTimeGlobal - t;
        return Process(info);
    }
};
CLensPPE* s_lens_ppe{};
float s_lens_ppe_start{}, s_lens_ppe_speed{1.f}, s_lens_ppe_k{1.f};
} // namespace

void lens_ppe_play(LPCSTR name, float speed, float strength)
{
    xr_delete(s_lens_ppe);
    if (!name || !name[0])
        return;
    string_path path;
    if (!FS.exist(path, "$level$", name) && !FS.exist(path, "$game_anims$", name))
    {
        Msg("! [nlc_gunplay] lens ppe [%s] not found", name);
        return;
    }
    s_lens_ppe = xr_new<CLensPPE>();
    s_lens_ppe->Load(name);
    s_lens_ppe_start = Device.fTimeGlobal + _max(g_3dss_lens_ppe_delay, 0.f);
    s_lens_ppe_speed = speed > 0.f ? speed : _max(g_3dss_lens_ppe_speed, 0.05f);
    s_lens_ppe_k = strength >= 0.f ? strength : g_3dss_lens_ppe;
    if (ps_r_3dss_crop_debug >= 1)
        Msg("--[nlc_gunplay] lens ppe [%s]: length [%.2f] s, speed [%.2f], strength [%.2f]", name, s_lens_ppe->GetLength(), s_lens_ppe_speed,
            s_lens_ppe_k);
}

void on_lens_open()
{
    if (g_3dss_lens_ppe <= 0.f)
        return;
    static const shared_str name{READ_IF_EXISTS(pSettings, r_string, "nlc_scope", "lens_open_ppe", "deimos_short.ppe")};
    static const float speed_k{READ_IF_EXISTS(pSettings, r_float, "nlc_scope", "lens_open_ppe_speed", 1.f)};
    lens_ppe_play(name.c_str(), _max(g_3dss_lens_ppe_speed, 0.05f) * speed_k);
}

void on_lens_close()
{
    if (g_3dss_lens_close_ppe <= 0.f)
        return;
    static const shared_str name{READ_IF_EXISTS(pSettings, r_string, "nlc_scope", "lens_close_ppe", "deimos_short.ppe")};
    static const float speed_k{READ_IF_EXISTS(pSettings, r_float, "nlc_scope", "lens_close_ppe_speed", 2.f)};
    lens_ppe_play(name.c_str(), _max(g_3dss_lens_ppe_speed, 0.05f) * speed_k, g_3dss_lens_close_ppe);
    s_lens_ppe_start = Device.fTimeGlobal; // the exit is short: no delay
}

void lens_ppe_update()
{
    Fvector4 base{.5f, .5f, .5f, 0.f}, add{0.f, 0.f, 0.f, 0.f};
    if (s_lens_ppe)
    {
        SPPInfo info;
        const float t = (Device.fTimeGlobal - s_lens_ppe_start) * s_lens_ppe_speed;
        const bool alive = t <= s_lens_ppe->GetLength() + 0.05f;
        if (t < 0.f)
            ; // delay: identity until the timeline starts
        else if (alive && s_lens_ppe->ProcessAt(info, t))
        {
            const float k = clampr(s_lens_ppe_k, 0.f, 2.f); // strength: 0 identity, 1 as authored, 2 exaggerated
            base.set(_max(.5f + (info.color_base.r - .5f) * k, 0.f), _max(.5f + (info.color_base.g - .5f) * k, 0.f), _max(.5f + (info.color_base.b - .5f) * k, 0.f),
                     _min(info.gray * k, 1.f));
            add.set(info.color_add.r * k, info.color_add.g * k, info.color_add.b * k, info.blur * k);
        }
        else
            xr_delete(s_lens_ppe);
    }
    shader_exports.set_custom_params("s3ds_param_8", base);
    shader_exports.set_custom_params("s3ds_param_9", add);
}

void clear()
{
    xr_delete(s_lens_ppe);
    for (Clip& c : s_clips)
    {
        xr_delete(c.anm);
        c.busy = false;
    }
    s_prev_stance = 0xff;
    s_pool_full_logged = false;
    for (int i = 0; i < 2; ++i)
    {
        s_fx_x[i].set(0.f, 0.f, 0.f);
        s_fx_v[i].set(0.f, 0.f, 0.f);
    }
    s_freq_k = 1.f;
    s_amp_k = 1.f;
}

namespace
{
int l_hud_clip(lua_State* L)
{
    clip_play(luaL_checkstring(L, 1), float(luaL_optnumber(L, 2, 1.0)), float(luaL_optnumber(L, 3, 1.0)), float(luaL_optnumber(L, 4, 1.0)),
              float(luaL_optnumber(L, 5, 1.0)));
    return 0;
}

int l_hud_impulse(lua_State* L)
{
    const Fvector pos{float(luaL_optnumber(L, 1, 0.0)), float(luaL_optnumber(L, 2, 0.0)), float(luaL_optnumber(L, 3, 0.0))};
    Fvector rot{float(luaL_optnumber(L, 4, 0.0)), float(luaL_optnumber(L, 5, 0.0)), float(luaL_optnumber(L, 6, 0.0))};
    rot.mul(-PI / 180.f); // same convention as the *_hud_offset_rot ltx keys (degrees)
    if (_valid(pos.x) && _valid(pos.y) && _valid(pos.z) && _valid(rot.x) && _valid(rot.y) && _valid(rot.z))
        hud_impulse(pos, rot);
    return 0;
}

int l_set_motion_mod(lua_State* L)
{
    set_motion_mod(float(luaL_optnumber(L, 1, 1.0)), float(luaL_optnumber(L, 2, 1.0)));
    return 0;
}

int l_set_param(lua_State* L)
{
    lua_pushboolean(L, set_param(luaL_checkstring(L, 1), float(luaL_checknumber(L, 2))));
    return 1;
}

int l_set_crouch_clips(lua_State* L)
{
    set_crouch_clips(luaL_checkstring(L, 1), luaL_checkstring(L, 2), float(luaL_optnumber(L, 3, 1.0)), float(luaL_optnumber(L, 4, 1.0)),
                     float(luaL_optnumber(L, 5, 1.0)));
    return 0;
}
} // namespace

namespace
{
int l_shot_punch(lua_State* L)
{
    shot_punch(float(luaL_checknumber(L, 1)) * (PI / 180.f), float(luaL_optnumber(L, 2, 0.0)));
    return 0;
}

int l_lens_ppe(lua_State* L)
{
    lens_ppe_play(luaL_optstring(L, 1, ""), float(luaL_optnumber(L, 2, 0.0)));
    return 0;
}

int l_land(lua_State* L)
{
    lua_pushnumber(L, lua_Number(s_land_count));
    lua_pushnumber(L, lua_Number(s_land_speed));
    lua_pushnumber(L, lua_Number(s_land_time));
    return 3;
}
} // namespace

void script_register(lua_State* L)
{
    static const luaL_Reg funcs[] = {
        {"shot_punch", l_shot_punch},
        {"land", l_land},
        {"lens_ppe", l_lens_ppe},
        {"hud_clip", l_hud_clip},
        {"hud_impulse", l_hud_impulse},
        {"set_motion_mod", l_set_motion_mod},
        {"set_crouch_clips", l_set_crouch_clips},
        {"set_param", l_set_param},
        {nullptr, nullptr},
    };
    luaL_register(L, "nlc_gunplay", funcs);
    lua_pop(L, 1);
}
} // namespace nlc_gunplay
