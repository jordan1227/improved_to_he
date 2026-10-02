#pragma once

// NLC: stealth/perception diagnostics and live-tuning parameters (docs/STEALTH_DESIGN.md, section 10).
// Everything is driven from Lua (nlc_stealth_* globals, gamedata sivol_stealth_cfg.script).
// With default parameters the hooks change nothing: logs are off and every override is identity.

class CObject;
class CGameObject;
class CEntityAlive;
class CCustomMonster;
class CBaseMonster;
struct CVisionParameters;
struct lua_State;

namespace nlc_stealth
{
// hot-path state, kept current by the parameter setters
extern bool g_track; // a watch list entry exists or log_events is on
extern float g_free_rate_mult;
extern float g_danger_rate_mult;
extern float g_monster_rate_mult;
extern float g_lum_factor_override; // < 0: use the vision section value
extern float g_transparency_factor_override; // < 0: use the vision section value
extern float g_force_profile; // debug, watched observers only: < 0 off, 0 free, 1 danger
extern float g_sky_k; // AI sky light weight (0 = engine luminosity only)
extern float g_vel_physics; // > 0: the actor's speed comes from its physics movement, not the noisy position history
extern bool g_actor_light_bonus; // sky, observer torch or muzzle flash term active
extern bool g_actor_rate_factor; // rank or stance factor differs from 1

// vision increment multiplier for an observer (stalker free/danger profile, or monster)
inline float rate_mult(bool stalker, bool danger) { return stalker ? (danger ? g_danger_rate_mult : g_free_rate_mult) : g_monster_rate_mult; }

bool tracked(u16 observer_id); // watched, or log_events on

// AI sky light for an object: env hemisphere brightness x sky visibility (0..1 each);
// the engine's own luminosity scales sky visibility by the render constant 0.08
float sky_light(const CGameObject* object, float* visibility = nullptr, float* brightness = nullptr);

// additive light for the actor as seen by an observer: sky (stalkers), observer torch cone (stalkers),
// muzzle flash after an unsuppressed shot (all observers), near-range contrast (stalkers);
// lum_raw is the engine luminosity
struct LightParts
{
    float sky = 0.f;
    float torch = 0.f;
    float flash = 0.f;
    float near_light = 0.f; // not "near": a windef.h macro
    float total() const { return sky + torch + flash + near_light; }
};
LightParts actor_light_parts(const CCustomMonster* observer, bool stalker_observer, float lum_raw);

// multiplicative rate factor for the actor as target: observer rank (stalkers) x actor stance
float actor_rate_factor(const CCustomMonster* observer, float* rank_k = nullptr, float* stance_k = nullptr);

// horizontal speed of the actor from its physics movement (m/s)
float actor_speed();

// the actor fired a shot (CWeaponMagazined::OnShot)
void on_actor_shot(bool silenced);
bool watched(u16 observer_id);

// one evaluation of the actor by an observer (CVisualMemoryManager::visible)
struct VisionSample
{
    float object_distance; // d
    float view_distance; // D
    float luminocity_used; // after the exponent (1 under the torch rule)
    float velocity;
    float ray; // raw see-through value of the trace (-1 when unknown)
    float trans; // value used by the formula
    float increment;
    float sum_before;
    float sum_after;
    float threshold;
    float time_delta; // seconds covered by this update
    bool danger_profile;
    bool out_of_range; // object farther than the view distance (value decays)
    bool always_visible; // inside always_visible_distance
};
void on_vision(const CCustomMonster* observer, const CVisionParameters& params, const VisionSample& s);

// the actor was not evaluated this update (no frustum entry, ray blocked); the engine resets the sum
void on_vision_skipped(const CCustomMonster* observer, float ray, float ray_threshold, float sum_before);

// actor sound heard by a stalker (CSoundMemoryManager::feel_sound_new)
void on_stalker_sound(const CCustomMonster* observer, int sound_type, float distance, float raw_power, float weighted_power, float threshold, bool heard,
                      bool fake_hit);
// actor sound heard by a monster (CBaseMonster::feel_sound_new); why: nullptr, "range"
void on_monster_sound(const CBaseMonster* observer, int sound_type, float distance, float power, float threshold, bool heard, bool near_hit, LPCSTR why);

// a stalker selected the actor as its enemy (CEnemyManager::try_change_enemy)
void on_enemy_selected(const CCustomMonster* observer);

// a monster added the actor to its enemy memory for the first time (CMonsterEnemyMemory::add_enemy)
void set_monster_add_source(LPCSTR source); // tag for the next add_enemy calls; nullptr clears
void on_monster_enemy_added(const CBaseMonster* observer, const CEntityAlive* enemy, bool positional);

// script made the actor known to an observer (make_object_visible_somewhen, enable_memory_object)
void on_script_touch(const CObject* observer, const CObject* target);

void script_register(lua_State* L);
} // namespace nlc_stealth
