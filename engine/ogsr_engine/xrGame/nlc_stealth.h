#pragma once

// NLC: stealth/perception diagnostics and live-tuning parameters (docs/STEALTH_DESIGN.md, section 10).
// Everything is driven from Lua (nlc_stealth_* globals, gamedata sivol_stealth_cfg.script).
// With default parameters the hooks change nothing: logs are off and every override is identity.

class CObject;
class CGameObject;
class CEntityAlive;
class CCustomMonster;
class CBaseMonster;
class CAI_Stalker;
class CVisualMemoryManager;
class CHangingLamp;
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
extern bool g_memory_decay; // mem_hold_ms or mem_decay_s set: stalkers keep a fading actor sum instead of the instant reset
extern float g_fakehit_min_pow; // actor shots add the shooter to hit memory only at this weighted power or more (0 = always)
extern float g_ray_resample; // > 0: stalkers re-trace the actor with a new sample point every update (partial cover)
extern float g_npc_light_k; // > 0: light model for stalker targets of stalker observers (0 = vanilla: NPCs always lit)
extern float g_nvd_floor; // light floor for observers with a night-vision device (0 = off)
extern float g_monster_ray_resample; // > 0: monsters re-trace the actor every update (partial cover)
extern float g_monster_notice_v; // > 0: a monster's actor sum past this fraction sends it to investigate
extern float g_pack_gate; // > 0: monster pack sharing honours the species pack_share_range and delay
extern bool g_monster_memory_decay; // monster_mem_hold_ms or monster_mem_decay_s set
extern float g_shot_alert_pow; // stalkers: a heard actor shot below this weighted power is concern (suspicion), not danger (0 = off)
extern float g_near_miss_range; // > 0: an actor bullet passing this close to a creature causes concern (metres)
extern float g_step_alert_pow; // stalkers: a heard actor footstep below this power is concern (suspicion), not danger (0 = off)
extern bool g_actor_sound_factor; // outfit noise, surface or rain masking active

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
    float lamp = 0.f; // dynamic lamps lighting the actor
    float total() const { return sky + torch + flash + near_light + lamp; }
};
LightParts actor_light_parts(const CCustomMonster* observer, bool stalker_observer, float lum_raw);

// multiplicative rate factor for the actor as target: observer rank (stalkers) x actor stance x outfit and rain (stalkers)
float actor_rate_factor(const CCustomMonster* observer, float* rank_k = nullptr, float* stance_k = nullptr, float* outfit_k = nullptr, float* rain_k = nullptr);

// luminosity of a stalker target seen by a stalker observer (npc_light_k), before the luminocity_factor exponent
float npc_target_light(const CCustomMonster* observer, const CAI_Stalker* target, bool observer_nvd);

// eye glow particle of a stalker with a night-vision device (CAI_Stalker::UpdateCL, net_Destroy, Die)
void update_eye_glow(CAI_Stalker* stalker);
void stop_eye_glow(CAI_Stalker* stalker);

// horizontal speed of the actor from its physics movement (m/s)
float actor_speed();

// the actor fired a shot (CWeaponMagazined::OnShot); section labels the shot sound in the log
void on_actor_shot(bool silenced, LPCSTR section);
// an NPC fired an unsuppressed shot (muzzle flash for npc_light_k)
void on_npc_shot(CObject* shooter);
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

// actor sum of a stalker observer that did not evaluate the actor this update: kept for mem_hold_ms,
// then drained at threshold / mem_decay_s per second (0 = dropped at once)
float forget_actor_sum(float value, float threshold, u32 since_ms, float time_delta, bool monster = false);

// "heard shot = hit memory" rule for an actor shot of this weighted power
inline bool fake_hit_allowed(float weighted_power) { return g_fakehit_min_pow <= 0.f || weighted_power >= g_fakehit_min_pow; }

// the actor was not evaluated this update (no frustum entry, ray blocked); the engine resets or decays the sum
void on_vision_skipped(const CCustomMonster* observer, float ray, float ray_threshold, float sum_before, float sum_after);

// actor sound heard by a stalker (CSoundMemoryManager::feel_sound_new); fake_hit_gated: a heard shot below fakehit_min_pow
void on_stalker_sound(const CCustomMonster* observer, int sound_type, float distance, float raw_power, float weighted_power, float threshold, bool heard,
                      bool fake_hit, bool fake_hit_gated);
// actor sound heard by a monster (CBaseMonster::feel_sound_new); why: nullptr, "range"
void on_monster_sound(const CBaseMonster* observer, int sound_type, float distance, float power, float threshold, bool heard, bool near_hit, LPCSTR why);

// a stalker selected the actor as its enemy (CEnemyManager::try_change_enemy)
void on_enemy_selected(const CCustomMonster* observer);

// a monster added the actor to its enemy memory for the first time (CMonsterEnemyMemory::add_enemy)
void set_monster_add_source(LPCSTR source); // tag for the next add_enemy calls; nullptr clears
void on_monster_enemy_added(const CBaseMonster* observer, const CEntityAlive* enemy, bool positional);

// script made the actor known to an observer (make_object_visible_somewhen, enable_memory_object)
void on_script_touch(const CObject* observer, const CObject* target);

// ---- monsters (docs/STEALTH_DESIGN.md 16) ----
// species keys from the monster section into its visual memory (CVisualMemoryManager::reload)
void load_monster_keys(CVisualMemoryManager& v, LPCSTR section);
// a monster turns and goes to a point: an "interesting" actor sound in its sound memory;
// style: 0 = by the monster's stable roll (species weights), 1 walk, 2 sneak, 3 hold and watch, 4 run
void monster_notice(CBaseMonster* monster, const Fvector& point, LPCSTR why, u32 style = 0);
// investigate style for the hear-interesting-sound state (0 = vanilla walk)
u32 monster_inv_style(CBaseMonster* monster);
// serial of the last accepted investigate impulse (the hear-interesting-sound state retargets when it changes)
u32 monster_notice_serial(CBaseMonster* monster);
// a heard actor shot below the alert power: concern only (CBaseMonster::feel_sound_new); true = handled
bool monster_faint_shot(CBaseMonster* monster, int sound_type, const Fvector& position, float power);
// a heard actor shot below g_shot_alert_pow (CSoundMemoryManager::feel_sound_new)
void stalker_faint_shot(CAI_Stalker* stalker, const Fvector& position, float weighted_power);
// an actor bullet segment (CBulletManager::CalcBullet): creatures it passes within g_near_miss_range get concern
void bullet_near_miss(const Fvector& start, const Fvector& dir, float length);
// a monster died (CBaseMonster::Die): pack mates check the corpse
void on_monster_death(CBaseMonster* monster);
// heard concern kind for scripts (0 = none)
u32 heard_kind(const CVisualMemoryManager& v);

// ---- M4 (docs/STEALTH_DESIGN.md 18) ----
// a heard actor footstep below g_step_alert_pow (CSoundMemoryManager::feel_sound_new)
void stalker_faint_step(CAI_Stalker* stalker, const Fvector& position, float power);
// an actor bullet hit sound close to a stalker (impact near him): concern like a near miss
void stalker_near_impact(CAI_Stalker* stalker, const Fvector& position);
// multiplier for an actor-owned sound's AI power: outfit noise and surface (steps), rain masking (quiet sounds)
float actor_sound_factor(int sound_type);
// a monster acquired an enemy by its own senses (CMonsterEnemyMemory::add_enemy, non-positional)
void on_monster_firsthand(CBaseMonster* monster, const CEntityAlive* enemy);
// lamps with a dynamic light (CHangingLamp net_Spawn / net_Destroy)
void register_lamp(CHangingLamp* lamp, bool add);
// partial visual detection by a monster (CVisualMemoryManager::visible); ratio = sum / threshold
void monster_visual_notice(CCustomMonster* observer, float ratio);
// aura sense (psy or smell) per schedule update (CBaseMonster::shedule_Update)
void monster_sense_update(CBaseMonster* monster);
// pack sharing gate (CBaseMonster::feel_vision_isRelevant); true = handled, skip the vanilla transfer
bool monster_pack_share(CBaseMonster* self, CBaseMonster* mate);
// impact / whine sounds make the shooter an enemy (CMonsterEnemyMemory::update)
bool monster_impact_allowed(CBaseMonster* monster, bool impact, float distance);
// the 2 m near-miss counts as a hit (CBaseMonster::feel_sound_new)
bool monster_near_hit_allowed(CBaseMonster* monster, const CObject* shooter);
// the visible-body helper (section actor_legs) is not a real monster
bool is_visible_body(const CObject* o);

void script_register(lua_State* L);
} // namespace nlc_stealth
