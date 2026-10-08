// NLC: stealth/perception diagnostics and live-tuning parameters (docs/STEALTH_DESIGN.md, section 10).

#include "stdafx.h"
#include "nlc_stealth.h"

#include "CustomMonster.h"
#include "ai/stalker/ai_stalker.h"
#include "ai/monsters/basemonster/base_monster.h"
#include "Actor.h"
#include "actor_defs.h"
#include "Level.h"
#include "memory_manager.h"
#include "visual_memory_manager.h"
#include "sound_memory_manager.h"
#include "hit_memory_manager.h"
#include "visual_memory_params.h"
#include "InventoryOwner.h"
#include "torch.h"
#include "Weapon.h"
#include "attachable_item.h"
#include "attachment_owner.h"
#include "stalker_movement_manager.h"
#include "CharacterPhysicsSupport.h"
#include "PHMovementControl.h"
#include "Inventory.h"
#include "gamepersistent.h"
#include "ai_sounds.h"
#include "ai_space.h"
#include "script_engine.h"
#include "CustomOutfit.h"
#include "ActorCondition.h"
#include "HangingLamp.h"
#include "material_manager.h"
#include "../xr_3da/gamemtllib.h"
#include "ai/monsters/monster_sound_memory.h"
#include "ParticlesObject.h"
#include "../Include/xrRender/Kinematics.h"
#include "script_game_object.h"
#include "enemy_manager.h"

#include <mutex>
#include <atomic>

namespace nlc_stealth
{
bool g_track = false;
float g_free_rate_mult = 1.f;
float g_danger_rate_mult = 1.f;
float g_monster_rate_mult = 1.f;
float g_lum_factor_override = -1.f;
float g_transparency_factor_override = -1.f;
float g_force_profile = -1.f;
float g_sky_k = 0.f;
float g_vel_physics = 0.f;
bool g_actor_light_bonus = false;
bool g_actor_rate_factor = false;
bool g_memory_decay = false;
float g_fakehit_min_pow = 0.f;
float g_ray_resample = 0.f;
float g_npc_light_k = 0.f;
float g_nvd_floor = 0.f;
float g_monster_ray_resample = 0.f;
float g_monster_notice_v = 0.f;
float g_pack_gate = 0.f;
float g_shot_alert_pow = 0.f;
float g_near_miss_range = 0.f;
float g_step_alert_pow = 0.f;
bool g_actor_sound_factor = false;
bool g_squad_dying_block = false;
bool g_monster_memory_decay = false;
bool g_vis_mtl = false;

namespace
{
float p_log_vision = 0.f;
float p_log_sound = 0.f;
float p_log_events = 0.f;
float p_log_interval_ms = 0.f;
float p_log_cap_per_min = 300.f;
float p_sky_scale = 0.08f; // ps_r2_dhemi_sky_scale, the renderer's hemi scale (render module, not linked here)
float p_sky_pow = 1.f; // brightness exponent: > 1 weights bright (day) skies more than dim (night) ones
float p_crouch_k = 1.f; // actor crouched (mcCrouch without mcAccel)
float p_creep_k = 1.f; // actor in low crouch (mcCrouch with mcAccel)
float p_torch_k = 0.f; // observer flashlight: light added at the centre of its cone, at zero distance
float p_torch_cone = 40.f; // full cone angle in degrees
float p_flash_k = 0.f; // muzzle flash light right after an unsuppressed shot, scaled by darkness
float p_flash_ms = 300.f; // muzzle flash fade time
float p_rank_k[4] = {1.f, 1.f, 1.f, 1.f}; // novice, experienced, veteran, master
float p_near_k = 0.f; // near-range contrast: light added at zero distance, fading as (1 - d / near_range)^2
float p_near_range = 25.f;
float p_mem_hold_ms = 0.f; // stalker keeps the actor sum this long after losing sight of the actor
float p_mem_decay_s = 0.f; // then drains it from the threshold to zero in this many seconds (0 with no hold = instant reset)
float p_outfit_vis = 0.f; // > 0: the actor outfit's hidden stealth_visibility_k multiplies the rate (stalker observers)
float p_rain_k = 0.f; // rate x (1 - rain_k x rain density) (stalker observers)
float p_npc_sky_vis = 0.35f; // npc_light_k: assumed sky visibility of an NPC target (open ground)
float p_npc_sun_k = 0.5f; // npc_light_k: share of the sun colour an NPC target receives on average (sun and shade mixed)
float p_glow_offset = 0.04f; // eye glow: metres in front of the eye bone, along the head-to-eye direction
float p_glow_up = 0.f; // eye glow: metres up from the eye bone
float p_monster_mem_hold_ms = 0.f; // monsters: keep the actor sum this long after losing sight
float p_monster_mem_decay_s = 0.f; // monsters: then drain it in this many seconds
float p_monster_notice_ms = 3000.f; // monsters: min gap between investigate impulses
float p_sense_mult = 1.f; // monsters: aura sense rate multiplier (0 = off)
float p_monster_shot_alert_pow = 0.f; // monsters: a heard actor shot below this power is concern, not danger (0 = off)
float p_heard_ms = 1500.f; // stalkers: how long a heard concern counts as suspicion
float p_near_miss_value = 0.6f; // stalkers: suspicion value of a near miss (notice band)
float p_corpse_check = 0.f; // > 0: pack mates check a killed member's corpse (species corpse_check_count / radius)
float p_concern_escalate = 0.f; // > 0: repeated concern escalates monsters (species concern_bold / concern_enemy_range)
float p_concern_window_ms = 20000.f;
float p_firsthand_ms = 0.f; // > 0: pack mates share the actor only from monsters that sensed him themselves within this time
float p_outfit_noise = 0.f; // > 0: the actor outfit's hidden stealth_noise_k scales footstep power
float p_surface_noise = 0.f; // > 0: [nlc_step_surface] factors by ground material scale footstep power
float p_rain_mask = 0.f; // quiet actor sounds (steps, items, reloads) x (1 - rain_mask x rain density)
float p_lamp_k = 0.f; // dynamic lamps lighting the actor (0 = off)
float p_lamp_period_ms = 250.f;
// pass 5 (docs/STEALTH_DESIGN.md 20)
float p_wall_mute = 0.f; // actor sounds behind static geometry x (1 - wall_mute) (0 = off)
float p_actor_psy_k = -1.f; // psy auras: the actor's psy gear factor, pushed by script (1 = unprotected); < 0: telepathic immunity (vanilla)
float p_bolt_range = 0.f; // a landing bolt thrown by the actor is heard this far, x surface factor (0 = off)
float p_hunt = 0.f; // > 0: bold monsters commit to a hunt when escalation finds the actor out of range
float p_squad_dying_share = 1.f; // 0: a squad member killed by a hit does not pass his attacker to the squad (1 = vanilla)
// 20.15: foliage and muzzle flash (stalkers looking at the actor)
float p_foliage_near_range = 0.f; // > 0: inside this range the ray cutoff fades from the section threshold to foliage_near_threshold
float p_foliage_near_threshold = 0.15f; // ray cutoff at zero distance
float p_foliage_k = 1.f; // partly see-through materials: vis^foliage_k (> 1 denser, < 1 thinner)
float p_flash_reveal = 0.f; // unsuppressed actor shot: one-time sum bump, x visibility threshold x darkness (0 = off)
float p_flash_reveal_range = 18.f;
float p_flash_reveal_gap_ms = 2000.f; // per observer: no new reveal before this gap (full-auto)

struct ParamDef
{
    LPCSTR name;
    float* value;
    float min_value;
    float max_value;
    float default_value;
};

ParamDef s_params[] = {
    {"log_vision", &p_log_vision, 0.f, 1.f, 0.f},
    {"log_sound", &p_log_sound, 0.f, 1.f, 0.f},
    {"log_events", &p_log_events, 0.f, 1.f, 0.f},
    {"log_interval_ms", &p_log_interval_ms, 0.f, 10000.f, 0.f},
    {"log_cap_per_min", &p_log_cap_per_min, 10.f, 100000.f, 300.f},
    {"free_rate_mult", &g_free_rate_mult, 0.01f, 100.f, 1.f},
    {"danger_rate_mult", &g_danger_rate_mult, 0.01f, 100.f, 1.f},
    {"monster_rate_mult", &g_monster_rate_mult, 0.01f, 100.f, 1.f},
    {"lum_factor_override", &g_lum_factor_override, -1.f, 4.f, -1.f},
    {"transparency_factor_override", &g_transparency_factor_override, -1.f, 4.f, -1.f},
    {"force_profile", &g_force_profile, -1.f, 1.f, -1.f},
    {"sky_k", &g_sky_k, 0.f, 20.f, 0.f},
    {"sky_pow", &p_sky_pow, 0.25f, 4.f, 1.f},
    {"vel_physics", &g_vel_physics, 0.f, 1.f, 0.f},
    {"crouch_k", &p_crouch_k, 0.05f, 2.f, 1.f},
    {"creep_k", &p_creep_k, 0.05f, 2.f, 1.f},
    {"torch_k", &p_torch_k, 0.f, 4.f, 0.f},
    {"torch_cone", &p_torch_cone, 5.f, 160.f, 40.f},
    {"flash_k", &p_flash_k, 0.f, 4.f, 0.f},
    {"flash_ms", &p_flash_ms, 50.f, 3000.f, 300.f},
    {"near_k", &p_near_k, 0.f, 2.f, 0.f},
    {"near_range", &p_near_range, 1.f, 100.f, 25.f},
    {"rank_k_novice", &p_rank_k[0], 0.1f, 5.f, 1.f},
    {"rank_k_experienced", &p_rank_k[1], 0.1f, 5.f, 1.f},
    {"rank_k_veteran", &p_rank_k[2], 0.1f, 5.f, 1.f},
    {"rank_k_master", &p_rank_k[3], 0.1f, 5.f, 1.f},
    {"sky_scale", &p_sky_scale, 0.001f, 10.f, 0.08f},
    {"mem_hold_ms", &p_mem_hold_ms, 0.f, 60000.f, 0.f},
    {"mem_decay_s", &p_mem_decay_s, 0.f, 300.f, 0.f},
    {"fakehit_min_pow", &g_fakehit_min_pow, 0.f, 10.f, 0.f},
    {"ray_resample", &g_ray_resample, 0.f, 1.f, 0.f},
    {"outfit_vis", &p_outfit_vis, 0.f, 1.f, 0.f},
    {"rain_k", &p_rain_k, 0.f, 1.f, 0.f},
    {"nvd_floor", &g_nvd_floor, 0.f, 1.f, 0.f},
    {"npc_light_k", &g_npc_light_k, 0.f, 1.f, 0.f},
    {"npc_sky_vis", &p_npc_sky_vis, 0.f, 1.f, 0.35f},
    {"npc_sun_k", &p_npc_sun_k, 0.f, 1.f, 0.5f},
    {"glow_offset", &p_glow_offset, -0.5f, 0.5f, 0.04f},
    {"glow_up", &p_glow_up, -0.5f, 0.5f, 0.f},
    {"monster_ray_resample", &g_monster_ray_resample, 0.f, 1.f, 0.f},
    {"monster_mem_hold_ms", &p_monster_mem_hold_ms, 0.f, 60000.f, 0.f},
    {"monster_mem_decay_s", &p_monster_mem_decay_s, 0.f, 300.f, 0.f},
    {"monster_notice_v", &g_monster_notice_v, 0.f, 1.f, 0.f},
    {"monster_notice_ms", &p_monster_notice_ms, 250.f, 60000.f, 3000.f},
    {"sense_mult", &p_sense_mult, 0.f, 10.f, 1.f},
    {"pack_gate", &g_pack_gate, 0.f, 1.f, 0.f},
    {"shot_alert_pow", &g_shot_alert_pow, 0.f, 5.f, 0.f},
    {"monster_shot_alert_pow", &p_monster_shot_alert_pow, 0.f, 5.f, 0.f},
    {"near_miss_range", &g_near_miss_range, 0.f, 20.f, 0.f},
    {"near_miss_value", &p_near_miss_value, 0.f, 1.f, 0.6f},
    {"heard_ms", &p_heard_ms, 100.f, 30000.f, 1500.f},
    {"corpse_check", &p_corpse_check, 0.f, 1.f, 0.f},
    {"concern_escalate", &p_concern_escalate, 0.f, 1.f, 0.f},
    {"concern_window_ms", &p_concern_window_ms, 1000.f, 120000.f, 20000.f},
    {"firsthand_ms", &p_firsthand_ms, 0.f, 60000.f, 0.f},
    {"step_alert_pow", &g_step_alert_pow, 0.f, 5.f, 0.f},
    {"outfit_noise", &p_outfit_noise, 0.f, 1.f, 0.f},
    {"surface_noise", &p_surface_noise, 0.f, 1.f, 0.f},
    {"rain_mask", &p_rain_mask, 0.f, 1.f, 0.f},
    {"lamp_k", &p_lamp_k, 0.f, 4.f, 0.f},
    {"lamp_period_ms", &p_lamp_period_ms, 0.f, 5000.f, 250.f},
    {"wall_mute", &p_wall_mute, 0.f, 1.f, 0.f},
    {"actor_psy_k", &p_actor_psy_k, -1.f, 3.f, -1.f},
    {"bolt_range", &p_bolt_range, 0.f, 60.f, 0.f},
    {"hunt", &p_hunt, 0.f, 1.f, 0.f},
    {"squad_dying_share", &p_squad_dying_share, 0.f, 1.f, 1.f},
    {"foliage_near_range", &p_foliage_near_range, 0.f, 50.f, 0.f},
    {"foliage_near_threshold", &p_foliage_near_threshold, 0.f, 1.f, 0.15f},
    {"foliage_k", &p_foliage_k, 0.2f, 5.f, 1.f},
    {"flash_reveal", &p_flash_reveal, 0.f, 1.f, 0.f},
    {"flash_reveal_range", &p_flash_reveal_range, 1.f, 60.f, 18.f},
    {"flash_reveal_gap_ms", &p_flash_reveal_gap_ms, 0.f, 10000.f, 2000.f},
};

constexpr u32 MAX_WATCH = 4;
u16 s_watch[MAX_WATCH]{};
u32 s_watch_n = 0;

struct Track
{
    u32 start = 0; // accumulation start of the actor (0 = none)
    u32 last_line = 0; // last per-update vision line
    int last_why = -1; // last skip reason logged (-1 = evaluated)
    float last_ray = -2.f;
    float last_threshold = 0.f; // a profile switch changes the threshold and restarts the measurement
    // last sight acquisition
    u32 seen_time = 0;
    u32 seen_after = 0;
    float seen_dist = 0.f;
    bool seen_danger = false;
    // last actor sound accepted by the observer
    u32 snd_time = 0;
    int snd_type = 0;
    float snd_power = 0.f;
    bool snd_fake_hit = false;
    // last script touch (make_object_visible_somewhen, enable_memory_object)
    u32 script_time = 0;
    // a monster's enemy selection is cleared every update: log one line per episode
    u32 enemy_last = 0;
    u32 macq_last = 0;
};

constexpr u32 EPISODE_GAP_MS = 3000;

// true for a repeat inside the same episode; updates the episode clock
bool repeat_in_episode(u32& last)
{
    const u32 now = Device.dwTimeGlobal;
    const bool repeat = last && now >= last && now - last < EPISODE_GAP_MS;
    last = now;
    return repeat;
}

xr_map<u16, Track> s_tracks;
std::recursive_mutex s_lock;

u32 s_cap_window = 0;
u32 s_cap_lines = 0;
u32 s_cap_dropped = 0;

LPCSTR s_monster_source = nullptr;

u32 s_last_shot = 0; // last unsuppressed actor shot
u32 s_last_shot_silenced = 0;
u32 s_last_shot_any = 0; // last actor shot of any kind, with its weapon (log label)
shared_str s_last_shot_section;
bool s_last_shot_sil = false;

// actor outfit visibility key, cached by outfit object id
u16 s_outfit_id = u16(-2);
float s_outfit_k = 1.f;

// eye glow particles by NPC id
xr_map<u16, CParticlesObject*> s_glows;

// lamps with dynamic lights
xr_vector<CHangingLamp*> s_lamps;

// actor outfit noise key, cached like the visibility key
u16 s_noise_outfit_id = u16(-2);
float s_noise_k = 1.f;

// [nlc_vis_transparency] by material index (-1 = the material's own value); built once on the main thread
xr_vector<float> s_mtl_vis;
bool s_mtl_built = false;
bool s_mtl_any = false;

// material name substring = see-through for stalker rays towards the actor; file order, the first matching key wins.
// Logs every partly see-through material once (tuning reference).
void build_mtl_table()
{
    s_mtl_built = true;
    s_mtl_vis.assign(GMLib.CountMaterial(), -1.f);
    CInifile::Sect* sect = pSettings->section_exist("nlc_vis_transparency") ? &pSettings->r_section("nlc_vis_transparency") : nullptr;
    u16 idx = 0;
    for (auto it = GMLib.FirstMaterial(); it != GMLib.LastMaterial(); ++it, ++idx)
    {
        LPCSTR name = (*it)->m_Name.c_str();
        if (sect && name)
            for (const auto& [key, value] : sect->Ordered_Data)
                if (strstr(name, key.c_str()))
                {
                    s_mtl_vis[idx] = std::clamp(float(atof(value.c_str())), 0.f, 1.f);
                    s_mtl_any = true;
                    break;
                }
        const float vis = (*it)->fVisTransparencyFactor;
        if ((vis > 0.f && vis < 1.f) || s_mtl_vis[idx] >= 0.f)
            Msg("~ [stealth] vis mtl [%s] %.2f -> %.2f", name ? name : "?", vis, s_mtl_vis[idx] >= 0.f ? s_mtl_vis[idx] : vis);
    }
}

void refresh()
{
    if (!s_mtl_built)
        build_mtl_table();
    g_track = s_watch_n > 0 || p_log_events > 0.f;
    g_actor_light_bonus = g_sky_k > 0.f || p_torch_k > 0.f || p_flash_k > 0.f || p_near_k > 0.f;
    g_actor_rate_factor = p_crouch_k != 1.f || p_creep_k != 1.f;
    for (const float k : p_rank_k)
        g_actor_rate_factor = g_actor_rate_factor || k != 1.f;
    g_actor_rate_factor = g_actor_rate_factor || p_outfit_vis > 0.f || p_rain_k > 0.f;
    g_memory_decay = p_mem_hold_ms > 0.f || p_mem_decay_s > 0.f;
    g_monster_memory_decay = p_monster_mem_hold_ms > 0.f || p_monster_mem_decay_s > 0.f;
    g_actor_sound_factor = p_outfit_noise > 0.f || p_surface_noise > 0.f || p_rain_mask > 0.f || p_wall_mute > 0.f;
    g_actor_light_bonus = g_actor_light_bonus || p_lamp_k > 0.f;
    g_squad_dying_block = p_squad_dying_share <= 0.f;
    g_vis_mtl = s_mtl_any || p_foliage_k != 1.f;
}

float outfit_k()
{
    CActor* a = Actor();
    const CCustomOutfit* outfit = a ? a->GetOutfit() : nullptr;
    const u16 id = outfit ? outfit->ID() : u16(-1);
    static shared_str sect;
    if (id != s_outfit_id || (outfit && sect != outfit->cNameSect()))
    {
        s_outfit_id = id;
        sect = outfit ? outfit->cNameSect() : shared_str();
        s_outfit_k = outfit ? READ_IF_EXISTS(pSettings, r_float, outfit->cNameSect(), "stealth_visibility_k", 1.f) : 1.f;
    }
    return s_outfit_k;
}

float outfit_noise_k()
{
    CActor* a = Actor();
    const CCustomOutfit* outfit = a ? a->GetOutfit() : nullptr;
    const u16 id = outfit ? outfit->ID() : u16(-1);
    static shared_str sect;
    if (id != s_noise_outfit_id || (outfit && sect != outfit->cNameSect()))
    {
        s_noise_outfit_id = id;
        sect = outfit ? outfit->cNameSect() : shared_str();
        s_noise_k = outfit ? READ_IF_EXISTS(pSettings, r_float, outfit->cNameSect(), "stealth_noise_k", 1.f) : 1.f;
    }
    return s_noise_k;
}

// [nlc_step_surface] factor of a material name (substring of the material name = factor)
float surface_k_of(LPCSTR name)
{
    if (!name || !pSettings->section_exist("nlc_step_surface"))
        return 1.f;
    CInifile::Sect& sect = pSettings->r_section("nlc_step_surface");
    for (const auto& [key, value] : sect.Ordered_Data) // file order: the first matching key wins
        if (strstr(name, key.c_str()))
            return float(atof(value.c_str()));
    return 1.f;
}

// ground material under the actor and its footstep factor
float surface_k(LPCSTR* name_out = nullptr)
{
    static u16 last_idx = u16(-1);
    static float last_k = 1.f;
    static shared_str last_name;
    CActor* a = Actor();
    if (!a)
        return 1.f;
    const u16 idx = a->material().last_material_idx();
    if (idx != last_idx)
    {
        last_idx = idx;
        const SGameMtl* mtl = GMLib.GetMaterialByIdx(idx);
        last_name = mtl ? mtl->m_Name : shared_str("?");
        last_k = mtl ? surface_k_of(last_name.c_str()) : 1.f;
    }
    if (name_out)
        *name_out = last_name.c_str();
    return last_k;
}

float rain_density()
{
    const CEnvDescriptorMixer* env = GamePersistent().Environment().CurrentEnv;
    return env ? std::clamp(env->rain_density, 0.f, 1.f) : 0.f;
}

// environment light for NPC targets, once per frame: ambient + AI sky term at an assumed sky visibility + part of the sun
float npc_env_light()
{
    static u32 frame = u32(-1);
    static float light = 1.f;
    if (frame == Device.dwFrame)
        return light;
    frame = Device.dwFrame;
    const CEnvDescriptorMixer* env = GamePersistent().Environment().CurrentEnv;
    if (!env)
        return light = 1.f;
    const float ambient = _max(env->ambient.x, _max(env->ambient.y, env->ambient.z));
    const float hemi = _max(env->hemi_color.x, _max(env->hemi_color.y, env->hemi_color.z));
    const float sun = _max(env->sun_color.x, _max(env->sun_color.y, env->sun_color.z));
    light = ambient + g_sky_k * p_npc_sky_vis * (p_sky_pow == 1.f ? hemi : pow(hemi, p_sky_pow)) + p_npc_sun_k * sun;
    clamp(light, 0.f, 1.f);
    return light;
}

// rank thresholds from [game_relations] rating (novice, 300, experienced, 600, veteran, 900, master)
int rank_index(int rank)
{
    static int limits[3] = {300, 600, 900};
    static bool loaded = false;
    if (!loaded)
    {
        loaded = true;
        if (pSettings->line_exist("game_relations", "rating"))
        {
            LPCSTR rating = pSettings->r_string("game_relations", "rating");
            string64 item;
            for (int i = 0; i < 3 && int(_GetItemCount(rating)) >= 2 * i + 2; ++i)
                limits[i] = atoi(_GetItem(rating, 2 * i + 1, item));
        }
    }
    int index = 0;
    while (index < 3 && rank >= limits[index])
        ++index;
    return index;
}

const CTorch* active_torch(const CAI_Stalker* stalker)
{
    if (const CTorch* torch = smart_cast<const CTorch*>(stalker->GetCurrentTorch()))
        if (torch->torch_active())
            return torch;
    if (const CAttachmentOwner* owner = smart_cast<const CAttachmentOwner*>(stalker))
        for (const CAttachableItem* item : owner->attached_objects())
            if (const CTorch* torch = smart_cast<const CTorch*>(&item->object()))
                if (torch->torch_active())
                    return torch;
    return nullptr;
}

bool on(float v) { return v > 0.f; }

// global line cap per minute; true when the line may be written
bool can_log()
{
    const u32 now = Device.dwTimeGlobal;
    if (now < s_cap_window || now >= s_cap_window + 60000)
    {
        if (s_cap_dropped)
            Msg("~ [stealth] %u line(s) dropped by log_cap_per_min=%.0f", s_cap_dropped, p_log_cap_per_min);
        s_cap_window = now;
        s_cap_lines = 0;
        s_cap_dropped = 0;
    }
    if (s_cap_lines >= u32(p_log_cap_per_min))
    {
        ++s_cap_dropped;
        return false;
    }
    ++s_cap_lines;
    return true;
}

LPCSTR obj_name(const CObject* o, string128& buf)
{
    if (!o)
        return "none";
    xr_sprintf(buf, "%s#%u", o->cName().c_str(), u32(o->ID()));
    return buf;
}

// stalker rank, -1 for anything else
int obj_rank(const CObject* o)
{
    const CAI_Stalker* stalker = smart_cast<const CAI_Stalker*>(o);
    return stalker ? int(stalker->Rank()) : -1;
}

// the NLC visible-body helper (sivol_visible_body, section actor_legs) is a monster object
// that follows the actor; it is not an observer worth logging
bool ignored(const CObject* o) { return o && !xr_strcmp(o->cNameSect().c_str(), "actor_legs"); }

LPCSTR actor_state()
{
    CActor* a = Actor();
    if (!a)
        return "-";
    const u32 s = a->MovingState();
    if (s & (mcJump | mcFall | mcLanding | mcLanding2))
        return "air";
    if (a->is_actor_climbing())
        return "climb";
    if (a->is_actor_sprinting())
        return "sprint";
    if (a->is_actor_running())
        return "run";
    if (a->is_actor_walking())
        return "walk";
    if (a->is_actor_creeping())
        return "creep";
    if (a->is_actor_crouching())
        return "crouch";
    if (s & mcCrouch)
        return "crouch_still";
    return (s & mcAnyMove) ? "move" : "still";
}

LPCSTR sound_kind(int type)
{
    const u32 t = u32(type);
    if ((t & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING)
        return "weapon_shooting";
    if ((t & SOUND_TYPE_WEAPON_BULLET_HIT) == SOUND_TYPE_WEAPON_BULLET_HIT)
        return "weapon_bullet_hit";
    if ((t & SOUND_TYPE_WEAPON_RECHARGING) == SOUND_TYPE_WEAPON_RECHARGING)
        return "weapon_reload";
    if ((t & SOUND_TYPE_WEAPON_EMPTY_CLICKING) == SOUND_TYPE_WEAPON_EMPTY_CLICKING)
        return "weapon_empty";
    if (t & SOUND_TYPE_WEAPON)
        return "weapon";
    if (t & SOUND_TYPE_ITEM)
        return "item";
    if ((t & SOUND_TYPE_MONSTER_STEP) == SOUND_TYPE_MONSTER_STEP)
        return "step";
    if (t & SOUND_TYPE_MONSTER)
        return "creature";
    if (t & SOUND_TYPE_ANOMALY)
        return "anomaly";
    if ((t & SOUND_TYPE_WORLD_OBJECT_COLLIDING) == SOUND_TYPE_WORLD_OBJECT_COLLIDING)
        return "world_colliding";
    if (t & SOUND_TYPE_WORLD)
        return "world";
    return "other";
}

// "-" or the age in seconds of a timestamp
LPCSTR age(u32 time, string32& buf)
{
    if (!time)
        return "-";
    xr_sprintf(buf, "%.1fs", float(Device.dwTimeGlobal - time) / 1000.f);
    return buf;
}

// "-" or "<section> sil=0|1" for the actor's active weapon (weapon sounds only); shot sounds use the weapon that fired
LPCSTR actor_weapon(int sound_type, string128& buf)
{
    CActor* a = Actor();
    if (!a || !(u32(sound_type) & SOUND_TYPE_WEAPON))
        return "-";
    if ((u32(sound_type) & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING && s_last_shot_any && Device.dwTimeGlobal >= s_last_shot_any &&
        Device.dwTimeGlobal - s_last_shot_any < 1000 && s_last_shot_section.size())
    {
        xr_sprintf(buf, "%s sil=%d", s_last_shot_section.c_str(), s_last_shot_sil ? 1 : 0);
        return buf;
    }
    const CWeapon* w = smart_cast<const CWeapon*>(a->inventory().ActiveItem());
    if (!w)
        return "-";
    xr_sprintf(buf, "%s sil=%d", w->cNameSect().c_str(), w->IsSilencerAttached() ? 1 : 0);
    return buf;
}

ParamDef* find_param(LPCSTR name)
{
    if (!name)
        return nullptr;
    for (auto& p : s_params)
        if (!xr_strcmp(p.name, name))
            return &p;
    return nullptr;
}
} // namespace

bool watched(u16 observer_id)
{
    for (u32 i = 0; i < s_watch_n; ++i)
        if (s_watch[i] == observer_id)
            return true;
    return false;
}

bool tracked(u16 observer_id) { return on(p_log_events) || watched(observer_id); }

float actor_speed()
{
    CActor* a = Actor();
    if (!a || !a->character_physics_support() || !a->character_physics_support()->movement())
        return 0.f;
    const Fvector& v = a->character_physics_support()->movement()->GetVelocity();
    return _sqrt(v.x * v.x + v.z * v.z);
}

void on_npc_shot(CObject* shooter)
{
    if (CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(shooter))
        stalker->memory().visual().m_nlc_last_shot = Device.dwTimeGlobal;
}

void on_actor_shot(bool silenced, LPCSTR section)
{
    (silenced ? s_last_shot_silenced : s_last_shot) = Device.dwTimeGlobal;
    s_last_shot_any = Device.dwTimeGlobal;
    s_last_shot_section = section;
    s_last_shot_sil = silenced;
}

float actor_ray_threshold(float distance, float base)
{
    if (p_foliage_near_range <= 0.f || distance >= p_foliage_near_range || p_foliage_near_threshold >= base)
        return base;
    const float t = _max(distance, 0.f) / p_foliage_near_range;
    return p_foliage_near_threshold + (base - p_foliage_near_threshold) * t;
}

float actor_mtl_transp(u16 mtl, float vis)
{
    if (mtl < s_mtl_vis.size() && s_mtl_vis[mtl] >= 0.f)
        vis = s_mtl_vis[mtl];
    if (p_foliage_k != 1.f && vis > 0.f && vis < 1.f)
        vis = std::pow(vis, p_foliage_k);
    return vis;
}

float flash_reveal(CCustomMonster* observer, CVisualMemoryManager& v, float distance)
{
    constexpr u32 WINDOW_MS = 1000; // the observer's next vision update after the shot
    const u32 now = Device.dwTimeGlobal;
    if (p_flash_reveal <= 0.f || !s_last_shot || now < s_last_shot || now - s_last_shot > WINDOW_MS || v.m_nlc_flash_shot == s_last_shot)
        return 0.f;
    if (distance > p_flash_reveal_range)
        return 0.f;
    if (observer && Actor() && observer->memory().enemy().selected() == Actor())
        return 0.f; // already fighting the actor: nothing to reveal
    if (now < v.m_nlc_flash_next && v.m_nlc_flash_next - now <= u32(p_flash_reveal_gap_ms))
        return 0.f;
    v.m_nlc_flash_shot = s_last_shot;
    v.m_nlc_flash_next = now + u32(p_flash_reveal_gap_ms);
    CActor* a = Actor();
    if (!a)
        return 0.f;
    float light = a->ROS()->get_luminocity();
    if (g_sky_k > 0.f)
        light += g_sky_k * sky_light(a);
    const float dark = 1.f - std::clamp(light, 0.f, 1.f);
    const float bump = p_flash_reveal * dark;
    if (observer && tracked(observer->ID()) && can_log())
    {
        string128 nb;
        Msg("~ [stealth] flash_reveal t=%u obs=%s d=%.1f light=%.2f bump=%.2f", now, obj_name(observer, nb), distance, light, bump);
    }
    return bump;
}

float forget_actor_sum(float value, float threshold, u32 since_ms, float time_delta, bool monster)
{
    const float hold_ms = monster ? p_monster_mem_hold_ms : p_mem_hold_ms;
    const float decay_s = monster ? p_monster_mem_decay_s : p_mem_decay_s;
    if (value <= 0.f)
        return 0.f;
    if (float(since_ms) <= hold_ms)
        return value;
    if (decay_s <= 0.f)
        return 0.f;
    value -= threshold * _max(time_delta, 0.f) / decay_s;
    return value > 0.f ? value : 0.f;
}

bool is_visible_body(const CObject* o) { return ignored(o); }

void load_monster_keys(CVisualMemoryManager& v, LPCSTR section)
{
    v.m_nlc_light_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_light_k", 0.f);
    v.m_nlc_dark_floor = READ_IF_EXISTS(pSettings, r_float, section, "nlc_dark_floor", 0.f);
    v.m_nlc_rain_k = std::clamp(READ_IF_EXISTS(pSettings, r_float, section, "nlc_rain_k", 0.f), 0.f, 1.f);
    v.m_nlc_pack_range = READ_IF_EXISTS(pSettings, r_float, section, "pack_share_range", 0.f);
    v.m_nlc_pack_delay_min = READ_IF_EXISTS(pSettings, r_float, section, "pack_share_delay_min", 0.5f);
    v.m_nlc_pack_delay_min = _max(0.f, v.m_nlc_pack_delay_min);
    v.m_nlc_pack_delay_max = _max(v.m_nlc_pack_delay_min, READ_IF_EXISTS(pSettings, r_float, section, "pack_share_delay_max", 1.5f));
    v.m_nlc_impact_max = READ_IF_EXISTS(pSettings, r_float, section, "feel_enemy_who_made_impact_max_distance", -1.f);
    v.m_nlc_near_hit_max = READ_IF_EXISTS(pSettings, r_float, section, "near_hit_shooter_max_distance", -1.f);
    v.m_nlc_shot_alert_pow = READ_IF_EXISTS(pSettings, r_float, section, "shot_alert_pow", -1.f);
    v.m_nlc_corpse_count = READ_IF_EXISTS(pSettings, r_float, section, "corpse_check_count", 0.f);
    v.m_nlc_corpse_radius = READ_IF_EXISTS(pSettings, r_float, section, "corpse_check_radius", 0.f);
    v.m_nlc_concern_bold = READ_IF_EXISTS(pSettings, r_float, section, "concern_bold", -1.f);
    v.m_nlc_concern_range = READ_IF_EXISTS(pSettings, r_float, section, "concern_enemy_range", 30.f);
    v.m_nlc_hunt_time = READ_IF_EXISTS(pSettings, r_float, section, "hunt_time", 25000.f);
    v.m_nlc_hunt_error_k = READ_IF_EXISTS(pSettings, r_float, section, "hunt_error_k", 0.5f);
    v.m_nlc_hunt_detect_k = READ_IF_EXISTS(pSettings, r_float, section, "hunt_detect_k", 1.5f);
    v.m_nlc_hunt_alert_ms = READ_IF_EXISTS(pSettings, r_float, section, "hunt_alert_ms", 30000.f);
    v.m_nlc_hunt_flankers = READ_IF_EXISTS(pSettings, r_float, section, "hunt_flankers", 0.f);
    v.m_nlc_hunt_flee_losses = READ_IF_EXISTS(pSettings, r_float, section, "hunt_flee_losses", 0.f);
    v.m_nlc_hunt_time = _max(0.f, v.m_nlc_hunt_time);
    v.m_nlc_hunt_alert_ms = _max(0.f, v.m_nlc_hunt_alert_ms);
    v.m_nlc_hunt_flankers = std::clamp(v.m_nlc_hunt_flankers, 0.f, 8.f);
    v.m_nlc_hunt_flee_losses = _max(0.f, v.m_nlc_hunt_flee_losses);
    if (pSettings->line_exist(section, "investigate_style_weights"))
    {
        LPCSTR w = pSettings->r_string(section, "investigate_style_weights");
        string32 item;
        for (int i = 0; i < 4 && i < int(_GetItemCount(w)); ++i)
            v.m_nlc_style_w[i] = _max(0.f, float(atof(_GetItem(w, i, item))));
    }
    CVisualMemoryManager::NlcSense& s = v.m_nlc_sense;
    s = CVisualMemoryManager::NlcSense{};
    s.range = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_range", 0.f);
    if (s.range <= 0.f)
        return;
    s.near_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_near_k", 1.f);
    s.far_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_far_k", 1.f);
    s.speed_pow = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_speed_pow", 0.f);
    s.speed_min = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_speed_min", 0.f);
    s.rate = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_rate", 1.f);
    s.loose = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_loose", 1.f);
    s.notice = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_notice", 0.f);
    s.success = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_success", 0.f);
    s.rain_k = READ_IF_EXISTS(pSettings, r_float, section, "nlc_sense_rain_k", 0.f);
    s.psy = !!READ_IF_EXISTS(pSettings, r_bool, section, "nlc_sense_psy", false);
    s.walls = !!READ_IF_EXISTS(pSettings, r_bool, section, "nlc_sense_walls", true);
}

float actor_rate_factor(const CCustomMonster* observer, float* rank_k, float* stance_k, float* outfit_k_out, float* rain_k_out)
{
    float rk = 1.f, sk = 1.f, ok = 1.f, wk = 1.f;
    if (const CAI_Stalker* stalker = smart_cast<const CAI_Stalker*>(observer))
    {
        rk = p_rank_k[rank_index(int(stalker->Rank()))];
        if (p_outfit_vis > 0.f)
            ok = outfit_k();
        if (p_rain_k > 0.f)
            wk = 1.f - p_rain_k * rain_density();
    }
    else if (observer && observer->memory().visual().m_nlc_rain_k > 0.f)
        wk = 1.f - observer->memory().visual().m_nlc_rain_k * rain_density();
    if (CActor* a = Actor())
    {
        const u32 s = a->MovingState();
        if (s & mcCrouch)
            sk = (s & mcAccel) ? p_creep_k : p_crouch_k;
    }
    if (rank_k)
        *rank_k = rk;
    if (stance_k)
        *stance_k = sk;
    if (outfit_k_out)
        *outfit_k_out = ok;
    if (rain_k_out)
        *rain_k_out = wk;
    return rk * sk * ok * wk;
}

float npc_target_light(const CCustomMonster* observer, const CAI_Stalker* target, bool observer_nvd)
{
    CVisualMemoryManager& tv = const_cast<CAI_Stalker*>(target)->memory().visual();
    const u32 now = Device.dwTimeGlobal;
    // torch flag of the target, refreshed every 0.5 s
    if (now >= tv.m_nlc_torch_time + 500 || now < tv.m_nlc_torch_time)
    {
        tv.m_nlc_torch_time = now;
        tv.m_nlc_torch_on = active_torch(target) != nullptr;
    }
    if (tv.m_nlc_torch_on)
        return 1.f;

    float light = npc_env_light();
    if (p_near_k > 0.f)
    {
        const float d = observer->Position().distance_to(target->Position());
        if (d < p_near_range)
            light += p_near_k * _sqr(1.f - d / p_near_range);
    }
    if (p_flash_k > 0.f && tv.m_nlc_last_shot && now >= tv.m_nlc_last_shot)
    {
        const float age = float(now - tv.m_nlc_last_shot);
        if (age < p_flash_ms)
            light += p_flash_k * (1.f - age / p_flash_ms);
    }
    if (observer_nvd && g_nvd_floor > 0.f)
        light = _max(light, g_nvd_floor);
    clamp(light, 0.f, 1.f);
    // blend from the vanilla value (always lit) towards the model
    return 1.f + (light - 1.f) * g_npc_light_k;
}

void update_eye_glow(CAI_Stalker* stalker)
{
    const auto it = s_glows.find(stalker->ID());
    if (it == s_glows.end() || !it->second)
        return;
    IKinematics* K = smart_cast<IKinematics*>(stalker->Visual());
    if (!K)
        return;
    // the eye bone follows the animation; the small offset goes forward horizontally (head bone -> eye bone, flattened)
    const u16 head = u16(stalker->eye_bone);
    u16 eye = K->LL_BoneID("eyelid_1");
    if (eye == BI_NONE)
        eye = head;
    Fvector eye_pos, head_pos;
    stalker->XFORM().transform_tiny(eye_pos, K->LL_GetTransform(eye).c);
    stalker->XFORM().transform_tiny(head_pos, K->LL_GetTransform(head).c);
    Fvector dir;
    dir.sub(eye_pos, head_pos);
    dir.y = 0.f;
    if (dir.square_magnitude() < EPS_S)
        dir.setHP(-stalker->movement().m_head.current.yaw, -stalker->movement().m_head.current.pitch);
    else
        dir.normalize();
    Fmatrix xf;
    xf.identity();
    xf.c.mad(eye_pos, dir, p_glow_offset);
    xf.c.y += p_glow_up;
    it->second->UpdateParent(xf, Fvector().set(0.f, 0.f, 0.f));
    if (!it->second->IsPlaying())
        it->second->Play(false);
}

// stable roll in [0, 1) from an object id and a salt
float id_roll(u16 id, u32 salt)
{
    const u64 h = (u64(id) * 2654435761ull + u64(salt) * 40503ull + 7919ull) % 4294967296ull;
    return float(double(h) / 4294967296.0);
}

// 1 walk, 2 sneak, 3 hold, 4 run by the species weights
u32 roll_style(CBaseMonster* monster)
{
    const float* w = monster->memory().visual().m_nlc_style_w;
    const float total = w[0] + w[1] + w[2] + w[3];
    if (total <= 0.f)
        return 1;
    float x = id_roll(monster->ID(), 11) * total;
    for (u32 i = 0; i < 4; ++i)
    {
        x -= w[i];
        if (x < 0.f)
            return i + 1;
    }
    return 1;
}

// a point near the target with an error growing with distance
Fvector blurred(const Fvector& from, const Fvector& target, float k)
{
    const float radius = std::clamp(from.distance_to(target) * k, 2.f, 12.f);
    const float a = ::Random.randF(0.f, PI_MUL_2);
    const float r = radius * _sqrt(::Random.randF(0.f, 1.f));
    return Fvector().set(target.x + r * _cos(a), target.y, target.z + r * _sin(a));
}

static constexpr LPCSTR STYLE_NAMES[] = {"vanilla", "walk", "sneak", "hold", "run"};

// gait order for upgrades inside one episode: hold < walk < sneak < run
u32 style_rank(u32 style)
{
    switch (style)
    {
    case 3: return 0;
    case 2: return 2;
    case 4: return 3;
    default: return 1;
    }
}

void notice_log(CBaseMonster* monster, LPCSTR why, const Fvector& point, LPCSTR extra)
{
    if (!(g_track && (watched(monster->ID()) || on(p_log_events)) && on(p_log_events + p_log_vision)))
        return;
    std::scoped_lock lock(s_lock);
    if (can_log())
    {
        const CVisualMemoryManager& v = monster->memory().visual();
        string128 n;
        Msg("~ [stealth] mnotice t=%u obs=%s why=%s style=%s d=%.1f point=%.1f,%.1f,%.1f%s", Device.dwTimeGlobal, obj_name(monster, n), why,
            STYLE_NAMES[std::min(v.m_nlc_inv_style, 4u)], monster->Position().distance_to(point), point.x, point.y, point.z, extra ? extra : "");
    }
}

void send_flankers(CBaseMonster* leader, const Fvector& point);

void monster_notice(CBaseMonster* monster, const Fvector& point_in, LPCSTR why, u32 style)
{
    CActor* a = Actor();
    if (!monster || !a || !monster->g_Alive())
        return;
    CVisualMemoryManager& v = monster->memory().visual();
    const u32 now = Device.dwTimeGlobal;
    Fvector point = point_in;

    // repeated concern (near misses, faint shots): hurry on the second, then attack (bold) or flee (timid);
    // a bold monster with the actor out of reach commits to a hunt of the guessed shooter point (hunt)
    const bool concern = !xr_strcmp(why, "near_miss") || !xr_strcmp(why, "shot_far");
    if (concern && p_concern_escalate > 0.f && v.m_nlc_concern_bold >= 0.f)
    {
        if (now > v.m_nlc_concern_last + u32(p_concern_window_ms))
        {
            v.m_nlc_concern_count = 0;
            v.m_nlc_concern_total = 0;
        }
        v.m_nlc_concern_last = now;
        ++v.m_nlc_concern_count;
        ++v.m_nlc_concern_total;
        // every repeat halves the guess error: the shooter is found sooner (12, 6, 3 m)
        if (v.m_nlc_concern_total > 1)
        {
            const float k = powf(0.5f, float(_min(v.m_nlc_concern_total, 4u) - 1));
            Fvector err;
            err.sub(point, a->Position());
            point.mad(a->Position(), err, k);
        }
        if (v.m_nlc_concern_count == 2)
            style = 4; // run there
        else if (v.m_nlc_concern_count >= 3)
        {
            v.m_nlc_concern_count = 0;
            const bool losses_flee = v.m_nlc_hunt_flee_losses > 0.f && float(v.m_nlc_pack_losses) >= v.m_nlc_hunt_flee_losses;
            if (v.m_nlc_concern_bold > 0.5f && !losses_flee)
            {
                if (monster->Position().distance_to(a->Position()) < v.m_nlc_concern_range && monster->EnemyMan.is_enemy(a))
                {
                    set_monster_add_source("escalate");
                    monster->EnemyMemory.add_enemy(a);
                    set_monster_add_source(nullptr);
                    return;
                }
                style = 4;
                if (p_hunt > 0.f)
                {
                    Fvector err;
                    err.sub(point, a->Position());
                    point.mad(a->Position(), err, std::clamp(v.m_nlc_hunt_error_k, 0.f, 1.f));
                    v.m_nlc_hunt_point = point;
                    v.m_nlc_hunt_until = now + u32(v.m_nlc_hunt_time);
                    v.m_nlc_hunt_next = now + 6000;
                    v.m_nlc_alert_until = 0;
                    v.m_nlc_rate_k = _max(1.f, v.m_nlc_hunt_detect_k);
                    why = "hunt";
                }
            }
            else
            {
                // timid (or a pack that lost too many): a dangerous sound with no owner makes it run away from that point
                monster->SoundMemory.HearSound(nullptr, SOUND_TYPE_WEAPON_SHOOTING, point, 1.f, now);
                why = losses_flee ? "flee_losses" : "flee";
            }
        }
    }

    // a corpse check does not override a more urgent impulse (near miss, shot, sight) still in progress
    const u32 prio = (!xr_strcmp(why, "corpse") || !xr_strcmp(why, "corpse_watch")) ? 1u : 2u;
    const bool active = now < v.m_nlc_inv_until;
    if (active && prio < v.m_nlc_notice_prio)
        return;

    // the gait is chosen once per episode; later impulses can only upgrade it (the hunt search sneaks)
    u32 new_style = style ? style : ((active && v.m_nlc_inv_style) ? v.m_nlc_inv_style : roll_style(monster));
    if (active && v.m_nlc_inv_style && style_rank(v.m_nlc_inv_style) > style_rank(new_style) && xr_strcmp(why, "hunt_search"))
        new_style = v.m_nlc_inv_style;
    const bool upgraded = active && new_style != v.m_nlc_inv_style;
    v.m_nlc_inv_style = new_style;
    v.m_nlc_inv_until = _max(now + 20000, v.m_nlc_hunt_until);

    // no retarget to a point close to the current target (unless the gait changed or a hunt starts)
    if (active && !upgraded && prio == v.m_nlc_notice_prio && xr_strcmp(why, "hunt") && xr_strcmp(why, "hunt_search") && xr_strcmp(why, "flank") &&
        v.m_nlc_notice_point.distance_to(point) < 8.f)
    {
        notice_log(monster, why, point, " keep");
        return;
    }
    v.m_nlc_notice_prio = prio;
    v.m_nlc_notice_point = point;
    ++v.m_nlc_notice_serial;
    monster->SoundMemory.HearSound(a, SOUND_TYPE_MONSTER_STEP, point, 1.f, now);
    notice_log(monster, why, point, nullptr);
    if (!xr_strcmp(why, "hunt"))
        send_flankers(monster, point);
}

// pack flanking: the nearest pack mates go to points beside the hunt point (+-50 degrees around it, half way)
void send_flankers(CBaseMonster* leader, const Fvector& point)
{
    const CVisualMemoryManager& lv = leader->memory().visual();
    const u32 want = u32(lv.m_nlc_hunt_flankers);
    if (!want)
        return;
    const float radius = lv.m_nlc_pack_range > 0.f ? lv.m_nlc_pack_range : 30.f;
    xr_vector<CObject*> nearest;
    Level().ObjectSpace.GetNearest(nearest, leader->Position(), radius, leader);
    xr_vector<std::pair<float, CBaseMonster*>> mates;
    for (CObject* o : nearest)
    {
        CBaseMonster* m = smart_cast<CBaseMonster*>(o);
        if (!m || m == leader || !m->g_Alive() || ignored(m) || m->EnemyMan.get_enemy())
            continue;
        if (m->g_Team() != leader->g_Team() || m->g_Squad() != leader->g_Squad() || m->g_Group() != leader->g_Group())
            continue;
        mates.emplace_back(m->Position().distance_to(leader->Position()), m);
    }
    std::sort(mates.begin(), mates.end(), [](const auto& l, const auto& r) { return l.first < r.first; });
    Fvector arm;
    arm.sub(leader->Position(), point);
    arm.y = 0.f;
    arm.mul(0.5f);
    const u32 now = Device.dwTimeGlobal;
    for (u32 i = 0; i < mates.size() && i < want; ++i)
    {
        const float ang = deg2rad((i % 2) ? -50.f : 50.f);
        Fvector p;
        p.set(point.x + arm.x * _cos(ang) - arm.z * _sin(ang), point.y, point.z + arm.x * _sin(ang) + arm.z * _cos(ang));
        CVisualMemoryManager& mv = mates[i].second->memory().visual();
        mv.m_nlc_hunt_point = point;
        mv.m_nlc_hunt_until = lv.m_nlc_hunt_until;
        mv.m_nlc_hunt_next = now + 8000;
        mv.m_nlc_alert_until = 0;
        mv.m_nlc_rate_k = _max(1.f, mv.m_nlc_hunt_detect_k);
        monster_notice(mates[i].second, p, "flank", 4);
    }
}

// hunt: search around the hunt point until it times out, then stay alert for a while ("lost the trail")
void monster_hunt_update(CBaseMonster* monster)
{
    CVisualMemoryManager& v = monster->memory().visual();
    if (!v.m_nlc_hunt_until && !v.m_nlc_alert_until)
        return;
    const u32 now = Device.dwTimeGlobal;
    if (monster->EnemyMan.get_enemy())
    {
        v.m_nlc_hunt_until = v.m_nlc_alert_until = 0;
        v.m_nlc_rate_k = 1.f;
        return;
    }
    if (v.m_nlc_hunt_until)
    {
        if (now >= v.m_nlc_hunt_until)
        {
            v.m_nlc_hunt_until = 0;
            v.m_nlc_alert_until = now + u32(v.m_nlc_hunt_alert_ms);
            notice_log(monster, "lost_trail", v.m_nlc_hunt_point, nullptr);
            return;
        }
        if (now >= v.m_nlc_hunt_next)
        {
            v.m_nlc_hunt_next = now + u32(::Random.randI(4000, 7000));
            const float ang = ::Random.randF(0.f, PI_MUL_2);
            const float r = ::Random.randF(8.f, 12.f);
            Fvector p;
            p.set(v.m_nlc_hunt_point.x + r * _cos(ang), v.m_nlc_hunt_point.y, v.m_nlc_hunt_point.z + r * _sin(ang));
            monster_notice(monster, p, "hunt_search", 2);
        }
        return;
    }
    if (now >= v.m_nlc_alert_until)
    {
        v.m_nlc_alert_until = 0;
        v.m_nlc_rate_k = 1.f;
    }
}

u32 monster_notice_serial(CBaseMonster* monster) { return monster->memory().visual().m_nlc_notice_serial; }

u32 monster_inv_style(CBaseMonster* monster)
{
    const CVisualMemoryManager& v = monster->memory().visual();
    return Device.dwTimeGlobal < v.m_nlc_inv_until ? v.m_nlc_inv_style : 0;
}

bool monster_shot_is_faint(const CBaseMonster* monster, int sound_type, float power)
{
    if ((u32(sound_type) & SOUND_TYPE_WEAPON_SHOOTING) != SOUND_TYPE_WEAPON_SHOOTING)
        return false;
    const CVisualMemoryManager& v = monster->memory().visual();
    const float alert = v.m_nlc_shot_alert_pow >= 0.f ? v.m_nlc_shot_alert_pow : p_monster_shot_alert_pow;
    return alert > 0.f && power < alert;
}

bool monster_faint_shot(CBaseMonster* monster, int sound_type, const Fvector& position, float power)
{
    if (!monster_shot_is_faint(monster, sound_type, power))
        return false;
    const CVisualMemoryManager& v = monster->memory().visual();
    if (!monster->EnemyMan.get_enemy() && Device.dwTimeGlobal >= v.m_nlc_notice_next)
    {
        monster->memory().visual().m_nlc_notice_next = Device.dwTimeGlobal + u32(p_monster_notice_ms);
        monster_notice(monster, blurred(monster->Position(), position, 0.25f), "shot_far");
    }
    return true;
}

void set_heard(CVisualMemoryManager& v, const Fvector& point, float value, u32 kind)
{
    const u32 now = Device.dwTimeGlobal;
    if (now >= v.m_nlc_heard_until || value >= v.m_nlc_heard)
    {
        v.m_nlc_heard = value;
        v.m_nlc_heard_kind = kind;
        v.m_nlc_point = point;
        v.m_nlc_point_time = now;
    }
    v.m_nlc_heard_until = now + u32(p_heard_ms);
}

u32 heard_kind(const CVisualMemoryManager& v) { return Device.dwTimeGlobal < v.m_nlc_heard_until ? v.m_nlc_heard_kind : 0; }

void stalker_faint_shot(CAI_Stalker* stalker, const Fvector& position, float weighted_power)
{
    const float share = g_shot_alert_pow > 0.f ? weighted_power / g_shot_alert_pow : 0.f;
    // notice band rising toward investigate with loudness
    const float value = std::clamp(0.4f + 0.4f * share, 0.4f, 0.8f);
    set_heard(stalker->memory().visual(), blurred(stalker->Position(), position, 0.2f), value, 1);
}

void stalker_faint_step(CAI_Stalker* stalker, const Fvector& position, float power)
{
    const float share = g_step_alert_pow > 0.f ? power / g_step_alert_pow : 0.f;
    // notice band; close and louder steps reach the investigate edge
    const float value = std::clamp(0.35f + 0.35f * share, 0.35f, 0.7f);
    set_heard(stalker->memory().visual(), blurred(stalker->Position(), position, 0.15f), value, 3);
}

void stalker_near_impact(CAI_Stalker* stalker, const Fvector& position)
{
    CActor* a = Actor();
    if (!a || g_near_miss_range <= 0.f)
        return;
    CVisualMemoryManager& v = stalker->memory().visual();
    const u32 now = Device.dwTimeGlobal;
    if (now < v.m_nlc_near_miss_next)
        return;
    v.m_nlc_near_miss_next = now + 1500;
    // the shooter is guessed from the impact toward the actor, with an error
    Fvector dir;
    dir.sub(a->Position(), position);
    const float real = dir.magnitude();
    if (real > EPS_L)
        dir.div(real);
    Fvector origin;
    origin.mad(position, dir, real * ::Random.randF(0.6f, 1.3f));
    set_heard(v, blurred(stalker->Position(), origin, 0.15f), p_near_miss_value, 2);
}

// true when solid static geometry (not passable: bushes, grass) lies between two points
// one direction: RayPick returns the nearest front face only, so passable hits (bushes) are stepped over
bool solid_hit(Fvector from, const Fvector& dir, float len)
{
    for (int i = 0; i < 3 && len > 0.5f; ++i)
    {
        collide::rq_result rq;
        if (!Level().ObjectSpace.RayPick(from, dir, len, collide::rqtStatic, rq, nullptr) || rq.range >= len - 0.5f)
            return false;
        bool passable = false;
        if (rq.element >= 0)
        {
            const CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + rq.element;
            const SGameMtl* mtl = GMLib.GetMaterialByIdx(T->material);
            passable = mtl && mtl->Flags.is(SGameMtl::flPassable);
        }
        if (!passable)
            return true;
        const float step = rq.range + 0.05f;
        from.mad(dir, step);
        len -= step;
    }
    return false;
}

// true when solid static geometry (not passable: bushes, grass) lies between two points; both directions,
// because the pick culls back faces (single-sided walls and fences block in one direction only)
bool wall_between(const Fvector& from, const Fvector& to)
{
    Fvector dir;
    dir.sub(to, from);
    const float len = dir.magnitude();
    if (len <= 1.f)
        return false;
    dir.div(len);
    if (solid_hit(from, dir, len))
        return true;
    Fvector back;
    back.invert(dir);
    return solid_hit(to, back, len);
}

float actor_sound_factor(int sound_type, const CCustomMonster* listener, const Fvector& position)
{
    const u32 t = u32(sound_type);
    const bool impact = (t & SOUND_TYPE_WEAPON_BULLET_HIT) == SOUND_TYPE_WEAPON_BULLET_HIT;
    const bool shot = (t & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING;
    float k = 1.f;
    if (!shot && !impact)
    {
        const bool step = (t & SOUND_TYPE_MONSTER_STEP) == SOUND_TYPE_MONSTER_STEP;
        if (step)
        {
            if (p_outfit_noise > 0.f)
                k *= outfit_noise_k();
            if (p_surface_noise > 0.f)
                k *= surface_k();
        }
        const bool quiet = step || (t & SOUND_TYPE_ITEM) || (t & SOUND_TYPE_WEAPON_RECHARGING) == SOUND_TYPE_WEAPON_RECHARGING;
        if (quiet && p_rain_mask > 0.f)
            k *= 1.f - p_rain_mask * rain_density();
    }
    // walls: one static ray from the sound to the listener, cached per listener for 250 ms (bullet impacts excluded)
    // cost: only listeners within 60 m (farther sounds are faint anyway), cached 500 ms per listener
    // (staggered by id) and kept while the sound moves less than 3 m
    if (p_wall_mute > 0.f && listener && !impact && listener->Position().distance_to_sqr(position) < 3600.f)
    {
        CVisualMemoryManager& v = const_cast<CCustomMonster*>(listener)->memory().visual();
        const u32 now = Device.dwTimeGlobal;
        if (now >= v.m_nlc_wall_time + 500 + (listener->ID() % 8) * 25 || now < v.m_nlc_wall_time || v.m_nlc_wall_from.distance_to(position) > 3.f)
        {
            Fvector from = position;
            from.y += 0.6f;
            Fvector to;
            listener->Center(to);
            v.m_nlc_wall_k = wall_between(from, to) ? 1.f - p_wall_mute : 1.f;
            v.m_nlc_wall_time = now;
            v.m_nlc_wall_from = position;
        }
        k *= v.m_nlc_wall_k;
    }
    return k;
}

void on_monster_firsthand(CBaseMonster* monster, const CEntityAlive* enemy)
{
    if (monster && enemy && enemy == Actor())
        monster->memory().visual().m_nlc_firsthand = Device.dwTimeGlobal;
}

void register_lamp(CHangingLamp* lamp, bool add)
{
    const auto it = std::find(s_lamps.begin(), s_lamps.end(), lamp);
    if (add && it == s_lamps.end())
        s_lamps.push_back(lamp);
    else if (!add && it != s_lamps.end())
        s_lamps.erase(it);
}

// bullet segments come from the bullet manager's parallel update: queued, handled on the main thread (on_frame)
struct NearMissSeg
{
    Fvector start, dir;
    float length;
    const void* level; // the level that queued it: entries left over from a previous level are dropped
};
xr_vector<NearMissSeg> s_near_miss;
std::atomic<bool> s_queued{false}; // set by the producers under s_lock, read unlocked by on_frame

void bullet_near_miss(const Fvector& start, const Fvector& dir, float length)
{
    if (g_near_miss_range <= 0.f || length <= EPS_L)
        return;
    std::scoped_lock lock(s_lock);
    if (s_near_miss.size() < 256)
    {
        s_near_miss.push_back({start, dir, length, g_pGameLevel});
        s_queued = true;
    }
}

void near_miss_process(const Fvector& start, const Fvector& dir, float length)
{
    CActor* a = Actor();
    if (!a || g_near_miss_range <= 0.f || length <= EPS_L)
        return;
    const u32 now = Device.dwTimeGlobal;
    Fvector mid;
    mid.mad(start, dir, length * 0.5f);
    xr_vector<CObject*> nearest;
    Level().ObjectSpace.GetNearest(nearest, mid, length * 0.5f + g_near_miss_range, nullptr);
    for (CObject* o : nearest)
    {
        CCustomMonster* creature = smart_cast<CCustomMonster*>(o);
        if (!creature || !creature->g_Alive() || ignored(creature))
            continue;
        Fvector center;
        creature->Center(center);
        Fvector to;
        to.sub(center, start);
        const float along = std::clamp(to.dotproduct(dir), 0.f, length);
        Fvector closest;
        closest.mad(start, dir, along);
        const float miss = closest.distance_to(center);
        if (miss > g_near_miss_range)
            continue;
        // the bullet goes through this creature: a hit, not a near miss
        if (miss < creature->Radius() * 0.75f)
            continue;
        CVisualMemoryManager& v = creature->memory().visual();
        if (now < v.m_nlc_near_miss_next)
            continue;
        v.m_nlc_near_miss_next = now + 1500;
        // the shooter is guessed back along the bullet, with a distance error
        const float real = creature->Position().distance_to(a->Position());
        const float guess = real * ::Random.randF(0.6f, 1.3f);
        Fvector origin;
        origin.mad(closest, dir, -guess);
        origin = blurred(creature->Position(), origin, 0.15f);
        if (CBaseMonster* monster = smart_cast<CBaseMonster*>(creature))
        {
            if (!monster->EnemyMan.get_enemy())
                monster_notice(monster, origin, "near_miss");
            else
            {
                monster->nlc_siege_on_near_miss(origin); // NLC: a besieger's spot is exposed (no-op outside a siege)
                if (monster->EnemyMan.get_enemy()->ID() == a->ID())
                    monster->nlc_zz_on_fire(); // NLC: a zigzagging charge switches side (no-op without a running leg)
            }
        }
        else if (smart_cast<CAI_Stalker*>(creature))
            set_heard(v, origin, p_near_miss_value, 2);
    }
}

void on_monster_death(CBaseMonster* dead)
{
    if (p_corpse_check <= 0.f || !dead || ignored(dead))
        return;
    const CVisualMemoryManager& dv = dead->memory().visual();
    const u32 count = u32(dv.m_nlc_corpse_count);
    const Fvector corpse = dead->Position();
    const u32 now = Device.dwTimeGlobal;
    xr_vector<CObject*> nearest;
    Level().ObjectSpace.GetNearest(nearest, corpse, dv.m_nlc_corpse_radius > 0.f ? dv.m_nlc_corpse_radius : 30.f, dead);
    xr_vector<std::pair<float, CBaseMonster*>> mates;
    for (CObject* o : nearest)
    {
        CBaseMonster* m = smart_cast<CBaseMonster*>(o);
        if (!m || m == dead || !m->g_Alive() || ignored(m))
            continue;
        if (m->g_Team() != dead->g_Team() || m->g_Squad() != dead->g_Squad() || m->g_Group() != dead->g_Group())
            continue;
        // pack losses (hunt_flee_losses), forgotten after ten minutes without a new loss
        CVisualMemoryManager& mv = m->memory().visual();
        if (now > mv.m_nlc_pack_loss_time + 600000)
            mv.m_nlc_pack_losses = 0;
        ++mv.m_nlc_pack_losses;
        mv.m_nlc_pack_loss_time = now;
        if (m->EnemyMan.get_enemy())
            continue;
        mates.emplace_back(m->Position().distance_to(corpse), m);
    }
    if (!count || dv.m_nlc_corpse_radius <= 0.f)
        return;
    std::sort(mates.begin(), mates.end(), [](const auto& l, const auto& r) { return l.first < r.first; });
    for (u32 i = 0; i < mates.size(); ++i)
    {
        // the nearest few check the corpse by their own style; the rest turn and watch it
        if (i < count)
            monster_notice(mates[i].second, corpse, "corpse");
        else
            monster_notice(mates[i].second, corpse, "corpse_watch", 3);
    }
}

// monster notices requested off the main thread (pack gate on the vision thread)
struct QueuedNotice
{
    u16 id;
    Fvector point;
    LPCSTR why; // string literal
    const void* level;
};
xr_vector<QueuedNotice> s_notices;

void queue_notice(CBaseMonster* monster, const Fvector& point, LPCSTR why)
{
    std::scoped_lock lock(s_lock);
    if (s_notices.size() < 64)
    {
        s_notices.push_back({monster->ID(), point, why, g_pGameLevel});
        s_queued = true;
    }
}

// bolts thrown by the actor: contacts are queued in the physics step and handled once per frame
struct BoltContact
{
    Fvector pos;
    u16 contact;
    shared_str material;
    const void* level;
};
xr_vector<BoltContact> s_bolts;

void bolt_contact(CObject* bolt, CObject* contact, const Fvector& position, LPCSTR material)
{
    if (p_bolt_range <= 0.f || !bolt)
        return;
    std::scoped_lock lock(s_lock);
    if (s_bolts.size() < 16)
    {
        s_bolts.push_back({position, contact ? contact->ID() : u16(-1), shared_str(material ? material : ""), g_pGameLevel});
        s_queued = true;
    }
}

void bolt_noise(const BoltContact& b)
{
    CActor* a = Actor();
    if (!a)
        return;
    CObject* hit_obj = b.contact != u16(-1) ? Level().Objects.net_Find(b.contact) : nullptr;
    CAI_Stalker* hit = smart_cast<CAI_Stalker*>(hit_obj);
    if (hit && hit->g_Alive())
    {
        // a direct hit on a stalker: the script decides the reaction (sivol_stealth_bolt)
        luabind::functor<void> f;
        if (ai().script_engine().functor("sivol_stealth_bolt.on_hit", f))
            f(hit->lua_game_object());
    }
    const float range = p_bolt_range * surface_k_of(b.material.c_str());
    if (g_track && on(p_log_events))
    {
        std::scoped_lock lock(s_lock);
        if (can_log())
            Msg("~ [stealth] bolt t=%u at=%.1f,%.1f,%.1f mtl=%s range=%.1f hit=%s", Device.dwTimeGlobal, b.pos.x, b.pos.y, b.pos.z, b.material.c_str(), range,
                hit ? hit->cName().c_str() : "-");
    }
    if (range <= 0.f)
        return;
    xr_vector<CObject*> nearest;
    Level().ObjectSpace.GetNearest(nearest, b.pos, range, nullptr);
    Fvector from = b.pos;
    from.y += 0.3f;
    bool siege_distracted = false; // NLC: one besieger per bolt
    for (CObject* o : nearest)
    {
        CCustomMonster* c = smart_cast<CCustomMonster*>(o);
        if (!c || c == hit || !c->g_Alive() || ignored(c))
            continue;
        Fvector center;
        c->Center(center);
        const float d = center.distance_to(b.pos);
        float r = range;
        if (p_wall_mute > 0.f && wall_between(from, center))
            r *= 1.f - p_wall_mute;
        if (d > r)
            continue;
        if (CAI_Stalker* s = smart_cast<CAI_Stalker*>(c))
        {
            if (s->memory().enemy().selected())
                continue;
            // close: go and check; farther: look
            set_heard(s->memory().visual(), blurred(s->Position(), b.pos, 0.1f), d < r * 0.5f ? 0.75f : 0.5f, 4);
        }
        else if (CBaseMonster* m = smart_cast<CBaseMonster*>(c))
        {
            // NLC: a distractible besieger goes to look (docs/DESIGN_monster_elevation_siege.md 4.9)
            if (m->EnemyMan.get_enemy() && !siege_distracted && m->nlc_siege_on_bolt(b.pos))
                siege_distracted = true;
            if (m->EnemyMan.get_enemy() || !m->EnemyMan.is_enemy(a))
                continue;
            CVisualMemoryManager& v = m->memory().visual();
            if (Device.dwTimeGlobal < v.m_nlc_notice_next)
                continue;
            v.m_nlc_notice_next = Device.dwTimeGlobal + u32(p_monster_notice_ms);
            monster_notice(m, b.pos, "noise");
        }
    }
}

void on_frame()
{
    if (!s_queued)
        return;
    xr_vector<BoltContact> list;
    xr_vector<NearMissSeg> segs;
    xr_vector<QueuedNotice> notices;
    {
        std::scoped_lock lock(s_lock);
        list.swap(s_bolts);
        segs.swap(s_near_miss);
        notices.swap(s_notices);
        s_queued = false;
    }
    for (const QueuedNotice& n : notices)
    {
        if (n.level != g_pGameLevel)
            continue;
        CBaseMonster* m = smart_cast<CBaseMonster*>(Level().Objects.net_Find(n.id));
        if (m && m->g_Alive() && !m->getDestroy())
            monster_notice(m, n.point, n.why);
    }
    for (const NearMissSeg& s : segs)
        if (s.level == g_pGameLevel)
            near_miss_process(s.start, s.dir, s.length);
    for (const BoltContact& b : list)
        if (b.level == g_pGameLevel)
            bolt_noise(b);
}

void monster_visual_notice(CCustomMonster* observer, float ratio)
{
    CBaseMonster* monster = smart_cast<CBaseMonster*>(observer);
    CActor* a = Actor();
    if (!monster || !a || ratio < g_monster_notice_v || ratio >= 1.f || monster->EnemyMan.get_enemy() || ignored(monster))
        return;
    CVisualMemoryManager& v = monster->memory().visual();
    const u32 now = Device.dwTimeGlobal;
    if (now < v.m_nlc_notice_next)
        return;
    v.m_nlc_notice_next = now + u32(p_monster_notice_ms);
    monster_notice(monster, a->Position(), "sight");
}

void monster_sense_update(CBaseMonster* monster)
{
    if (!ignored(monster) && monster->g_Alive())
        monster_hunt_update(monster);
    CVisualMemoryManager& v = monster->memory().visual();
    CVisualMemoryManager::NlcSense& s = v.m_nlc_sense;
    if (s.range <= 0.f || p_sense_mult <= 0.f || ignored(monster))
        return;
    CActor* a = Actor();
    const u32 now = Device.dwTimeGlobal;
    if (!a || !a->g_Alive() || !monster->g_Alive())
    {
        s.level = 0.f;
        s.last = now;
        return;
    }
    const float dt = s.last ? float(now - s.last) / 1000.f : 0.f;
    s.last = now;
    const Fvector actor_pos = a->Position();
    if (dt <= 0.f || dt > 2.f)
    {
        s.last_actor = actor_pos;
        return;
    }

    float range = s.range;
    if (s.rain_k > 0.f)
        range *= 1.f - s.rain_k * rain_density();
    const float dist = monster->Position().distance_to(actor_pos);
    // a teleport (scripts, anomalies) is not movement: speed clamped, a jump over 5 m only resets the reference
    const float moved = actor_pos.distance_to(s.last_actor);
    const float speed = moved > 5.f ? 0.f : _min(moved / dt, 10.f);
    s.last_actor = actor_pos;

    float gain = 0.f;
    bool blocked = false;
    if (dist < range && monster->EnemyMan.is_enemy(a))
    {
        if (!s.walls)
        {
            Fvector from, to, dir;
            monster->Center(from);
            a->Center(to);
            dir.sub(to, from);
            const float len = dir.magnitude();
            if (len > EPS_L)
            {
                dir.div(len);
                collide::rq_result rq;
                blocked = Level().ObjectSpace.RayPick(from, dir, len, collide::rqtStatic, rq, nullptr) && rq.range < len - 0.3f;
            }
        }
        if (!blocked)
        {
            const float rel = dist / range;
            const float range_k = rel * s.far_k + (1.f - rel) * s.near_k;
            float speed_k = 1.f;
            if (speed < s.speed_min)
                speed_k = 0.f;
            else if (s.speed_pow > 0.f)
                speed_k = pow(1.f + speed, s.speed_pow) - 1.f;
            // psy gear factor pushed by script (actor_psy_k: 1 = unprotected, any difficulty); hunt / alert rate factor
            const float psy_k = !s.psy ? 1.f : (p_actor_psy_k >= 0.f ? p_actor_psy_k : a->conditions().GetHitImmunity(ALife::eHitTypeTelepatic));
            gain = dt * s.rate * range_k * speed_k * psy_k * p_sense_mult * v.m_nlc_rate_k;
        }
    }
    const float cap = 1.5f * _max(s.success, s.notice);
    s.level += gain - dt * s.loose;
    clamp(s.level, 0.f, cap > 0.f ? cap : 10.f);

    LPCSTR kind = s.psy ? "psy" : "smell";
    if (g_track && watched(monster->ID()) && on(p_log_vision) && now >= s.log_next && (s.level > 0.f || gain > 0.f))
    {
        s.log_next = now + 500;
        std::scoped_lock lock(s_lock);
        if (can_log())
        {
            string128 n;
            Msg("~ [stealth] msense t=%u obs=%s kind=%s d=%.1f range=%.1f v=%.2f blocked=%d gain=%.3f level=%.2f notice=%.2f success=%.2f", now, obj_name(monster, n),
                kind, dist, range, speed, blocked ? 1 : 0, gain, s.level, s.notice, s.success);
        }
    }

    if (s.success > 0.f && s.level >= s.success)
    {
        set_monster_add_source(kind);
        monster->EnemyMemory.add_enemy(a);
        set_monster_add_source(nullptr);
    }
    else if (s.notice > 0.f && s.level >= s.notice && now >= s.notice_next && !monster->EnemyMan.get_enemy())
    {
        s.notice_next = now + u32(p_monster_notice_ms);
        monster_notice(monster, actor_pos, kind);
    }
}

bool monster_pack_share(CBaseMonster* self, CBaseMonster* mate)
{
    // the visible-body helper never gives or receives pack enemies
    if (ignored(self) || ignored(mate))
        return true;
    if (g_pack_gate <= 0.f)
        return false;
    const CEntityAlive* enemy = mate->EnemyMan.get_enemy();
    if (!enemy)
        return true;
    if (enemy != Actor())
        return false; // NPC and monster enemies are shared as vanilla
    // already hunting it: refresh freely, so an alerted pack stays hard to lose
    if (self->EnemyMemory.get_danger(enemy) >= 0.f)
        return false;
    // second-hand knowledge is not passed on: only mates that sensed the actor themselves share him
    if (p_firsthand_ms > 0.f && enemy == Actor() && !mate->memory().visual().visible_now(enemy) &&
        Device.dwTimeGlobal > mate->memory().visual().m_nlc_firsthand + u32(p_firsthand_ms))
        return true;
    CVisualMemoryManager& v = self->memory().visual();
    if (v.m_nlc_pack_range <= 0.f)
        return false;
    const u32 now = Device.dwTimeGlobal;
    if (now > v.m_nlc_pack_last + 3000 || !v.m_nlc_pack_seen)
    {
        v.m_nlc_pack_seen = now;
        v.m_nlc_pack_delay = u32(1000.f * ::Random.randF(v.m_nlc_pack_delay_min, v.m_nlc_pack_delay_max));
    }
    v.m_nlc_pack_last = now;
    if (self->Position().distance_to(mate->Position()) <= v.m_nlc_pack_range)
        return now < v.m_nlc_pack_seen + v.m_nlc_pack_delay; // false = share now
    // beyond the pack range: only alert and investigate where the mate's enemy was
    if (now >= v.m_nlc_notice_next)
    {
        v.m_nlc_notice_next = now + u32(p_monster_notice_ms);
        queue_notice(self, mate->EnemyMan.get_enemy_position(), "pack_far"); // vision thread: handled in on_frame
    }
    return true;
}

bool monster_impact_allowed(CBaseMonster* monster, bool impact, float distance)
{
    if (!impact)
        return true;
    const float limit = monster->memory().visual().m_nlc_impact_max;
    return limit < 0.f || distance < limit;
}

bool monster_near_hit_allowed(CBaseMonster* monster, const CObject* shooter)
{
    if (!shooter || shooter != Actor())
        return true; // the stealth gates are for the actor; NPC shooters as vanilla
    const float limit = monster->memory().visual().m_nlc_near_hit_max;
    return limit < 0.f || !shooter || monster->Position().distance_to(shooter->Position()) < limit;
}

void stop_eye_glow(CAI_Stalker* stalker)
{
    stalker->memory().visual().m_nlc_glow = false;
    const auto it = s_glows.find(stalker->ID());
    if (it == s_glows.end())
        return;
    if (it->second)
    {
        it->second->Stop(FALSE);
        CParticlesObject::Destroy(it->second);
    }
    s_glows.erase(it);
}

LightParts actor_light_parts(const CCustomMonster* observer, bool stalker_observer, float lum_raw)
{
    LightParts parts;
    CActor* a = Actor();
    if (!a || !observer)
        return parts;

    if (stalker_observer && g_sky_k > 0.f)
        parts.sky = g_sky_k * sky_light(a);

    if (stalker_observer && p_near_k > 0.f)
    {
        const float d = observer->Position().distance_to(a->Position());
        if (d < p_near_range)
            parts.near_light = p_near_k * _sqr(1.f - d / p_near_range);
    }

    if (stalker_observer && p_torch_k > 0.f)
    {
        const CAI_Stalker* stalker = smart_cast<const CAI_Stalker*>(observer);
        const CTorch* torch = stalker ? active_torch(stalker) : nullptr;
        const float range = torch ? torch->get_range() : 0.f;
        if (range > 0.f)
        {
            Fvector eye = observer->Position();
            eye.y += 1.6f;
            Fvector to;
            a->Center(to);
            to.sub(eye);
            const float d = to.magnitude();
            if (d > EPS_L && d < range)
            {
                to.div(d);
                Fvector dir;
                dir.setHP(-stalker->movement().m_head.current.yaw, -stalker->movement().m_head.current.pitch);
                const float cos_half = _cos(deg2rad(p_torch_cone * 0.5f));
                const float cos_a = dir.dotproduct(to);
                if (cos_a > cos_half)
                    parts.torch = p_torch_k * (cos_a - cos_half) / (1.f - cos_half) * (1.f - d / range);
            }
        }
    }

    if (p_flash_k > 0.f && s_last_shot && Device.dwTimeGlobal >= s_last_shot)
    {
        const float age = float(Device.dwTimeGlobal - s_last_shot);
        if (age < p_flash_ms)
            parts.flash = p_flash_k * (1.f - age / p_flash_ms) * (1.f - std::clamp(lum_raw, 0.f, 1.f));
    }

    // dynamic lamps: computed for the actor at most every lamp_period_ms, shared by all observers
    if (p_lamp_k > 0.f)
    {
        static u32 next = 0;
        static float lamp = 0.f;
        const u32 now = Device.dwTimeGlobal;
        if (now >= next || now + 60000 < next)
        {
            next = now + u32(p_lamp_period_ms);
            lamp = 0.f;
            Fvector center;
            a->Center(center);
            for (CHangingLamp* l : s_lamps)
            {
                Fvector pos, dir;
                float range, bright, cos_half;
                if (!l || !l->nlc_light(pos, dir, range, bright, cos_half))
                    continue;
                Fvector to;
                to.sub(center, pos);
                const float d = to.magnitude();
                if (d >= range || d < EPS_L)
                    continue;
                to.div(d);
                float k = bright * _sqr(1.f - d / range);
                if (cos_half > -1.f)
                {
                    const float c = dir.dotproduct(to);
                    if (c <= cos_half)
                        continue;
                    k *= std::clamp((c - cos_half) / (1.f - cos_half) * 3.f, 0.f, 1.f); // soft cone edge
                }
                if (k <= lamp)
                    continue;
                collide::rq_result rq;
                if (Level().ObjectSpace.RayPick(pos, to, d, collide::rqtStatic, rq, nullptr) && rq.range < d - 0.3f)
                    continue; // blocked
                lamp = k;
            }
        }
        parts.lamp = p_lamp_k * lamp;
    }
    return parts;
}

float sky_light(const CGameObject* object, float* visibility, float* brightness)
{
    float vis = 0.f, bright = 0.f;
    if (object)
    {
        vis = const_cast<CGameObject*>(object)->ROS()->get_luminocity_hemi() / p_sky_scale;
        clamp(vis, 0.f, 1.f);
    }
    if (const CEnvDescriptorMixer* env = GamePersistent().Environment().CurrentEnv)
        bright = _max(env->hemi_color.x, _max(env->hemi_color.y, env->hemi_color.z));
    if (visibility)
        *visibility = vis;
    if (brightness)
        *brightness = bright;
    return vis * (p_sky_pow == 1.f ? bright : pow(bright, p_sky_pow));
}

void on_vision(const CCustomMonster* observer, const CVisionParameters& params, const VisionSample& s)
{
    if (!g_track || !observer || ignored(observer))
        return;
    const u16 id = observer->ID();
    const bool w = watched(id);
    if (!w && !on(p_log_events))
        return;

    std::scoped_lock lock(s_lock);
    Track& t = s_tracks[id];
    const u32 now = Device.dwTimeGlobal;
    t.last_why = -1;

    // the first update that adds to the sum already covers its time step;
    // a threshold change (free <-> danger profile) restarts the measurement
    const bool threshold_changed = t.last_threshold > 0.f && t.last_threshold != s.threshold;
    t.last_threshold = s.threshold;
    if (s.sum_after > 0.f && (t.start == 0 || s.sum_before <= 0.f || threshold_changed))
    {
        const u32 step = u32(_max(s.time_delta, 0.f) * 1000.f);
        t.start = (now > step) ? now - step : now;
    }

    const bool crossed = s.sum_before < s.threshold && s.sum_after >= s.threshold;
    if (crossed)
    {
        t.seen_time = now;
        t.seen_after = t.start ? now - t.start : 0;
        t.seen_dist = s.object_distance;
        t.seen_danger = s.danger_profile;
        if ((w && on(p_log_vision)) || on(p_log_events))
        {
            if (can_log())
            {
                string128 n;
                Msg("~ [stealth] seen t=%u obs=%s rank=%d after=%.2fs d=%.1f prof=%s Lp=%.3f v=%.2f tr=%.2f st=%s%s", now, obj_name(observer, n), obj_rank(observer),
                    float(t.seen_after) / 1000.f, s.object_distance, s.danger_profile ? "danger" : "free", s.luminocity_used, s.velocity, s.trans, actor_state(),
                    s.always_visible ? " always_visible=1" : "");
            }
        }
    }

    // per-update lines stop while the actor stays detected (the sum is pinned at the threshold)
    const bool still_seen = s.sum_before >= s.threshold && s.sum_after >= s.threshold;
    if (w && on(p_log_vision) && !still_seen && (p_log_interval_ms <= 0.f || now >= t.last_line + u32(p_log_interval_ms)))
    {
        t.last_line = now;
        if (can_log())
        {
            CActor* a = Actor();
            float lum_raw = -1.f;
            bool torch = false;
            if (a)
            {
                lum_raw = a->ROS()->get_luminocity();
                const CTorch* tr = smart_cast<const CTorch*>(a->GetCurrentTorch());
                torch = tr && tr->torch_active() && observer->Position().distance_to(a->Position()) < tr->get_range();
            }

            float fog_factor = 1.f;
            const CEnvDescriptorMixer* env = GamePersistent().Environment().CurrentEnv;
            if (env && env->fog_far > env->fog_near)
            {
                const float fog_w = 1.f / (env->fog_far - env->fog_near);
                float fog = (s.object_distance * fog_w - env->fog_near * fog_w) * params.m_fog_factor;
                clamp(fog, 0.f, 1.f);
                fog_factor = 1.f - pow(fog, params.m_fog_pow);
            }

            float sky_vis = 0.f, sky_bright = 0.f;
            sky_light(a, &sky_vis, &sky_bright);
            const LightParts parts = actor_light_parts(observer, !!smart_cast<const CAI_Stalker*>(observer), lum_raw);
            float rank_k = 1.f, stance_k = 1.f, out_k = 1.f, rain_k = 1.f;
            actor_rate_factor(observer, &rank_k, &stance_k, &out_k, &rain_k);
            const CVisualMemoryManager& ov = observer->memory().visual();

            string128 n;
            Msg("~ [stealth] vis t=%u obs=%s rank=%d prof=%s st=%s d=%.1f D=%.1f L=%.4f sky=%.2f hemi=%.3f +sky=%.3f +otorch=%.3f +flash=%.3f +near=%.3f +lamp=%.3f Lp=%.3f torch=%d v=%.2f vp=%.2f "
                "rk=%.2f stk=%.2f out=%.2f rain=%.2f nvd=%d ek=%.2f ray=%.2f tr=%.2f fog=%.2f inc=%.3f sum=%.1f/%.0f%s",
                now, obj_name(observer, n), obj_rank(observer), s.danger_profile ? "danger" : "free", actor_state(), s.object_distance, s.view_distance, lum_raw,
                sky_vis, sky_bright, parts.sky, parts.torch, parts.flash, parts.near_light, parts.lamp, s.luminocity_used, torch ? 1 : 0, s.velocity, actor_speed(), rank_k, stance_k, out_k, rain_k,
                ov.m_nlc_nvd ? 1 : 0, ov.m_nlc_rate_k, s.ray,
                s.trans, fog_factor, s.increment, s.sum_after, s.threshold,
                s.out_of_range ? " out_of_range=1" : (s.always_visible ? " always_visible=1" : ""));
        }
    }

    if (s.sum_after <= 0.f)
        t.start = 0;
}

void on_vision_skipped(const CCustomMonster* observer, float ray, float ray_threshold, float sum_before, float sum_after)
{
    if (!g_track || !observer || ignored(observer))
        return;
    const u16 id = observer->ID();
    const bool w = watched(id);
    if (!w && !on(p_log_events))
        return;

    std::scoped_lock lock(s_lock);
    Track& t = s_tracks[id];
    // with mem_hold_ms / mem_decay_s the sum fades instead; "reset" is the update where it reaches zero
    const bool reset = sum_before > 0.f && sum_after <= 0.f;
    if (reset)
        t.start = 0;

    if (!w || !on(p_log_vision))
        return;

    // 0: not in the vision frustum, 1: ray blocked, 2: ray clear again but not yet confirmed
    const int why = (ray < 0.f) ? 0 : (ray < ray_threshold ? 1 : 2);
    if (!reset && why == t.last_why && _abs(ray - t.last_ray) <= 0.02f)
        return;
    t.last_why = why;
    t.last_ray = ray;
    if (!can_log())
        return;

    static constexpr LPCSTR WHY[] = {"frustum", "ray", "recover"};
    string128 n;
    Msg("~ [stealth] nolos t=%u obs=%s why=%s ray=%.2f thr=%.2f sum=%.1f left=%.1f%s", Device.dwTimeGlobal, obj_name(observer, n), WHY[why], ray, ray_threshold,
        sum_before, sum_after, reset ? " reset=1" : "");
}

void on_stalker_sound(const CCustomMonster* observer, int sound_type, float distance, float raw_power, float weighted_power, float threshold, bool heard, bool fake_hit,
                      bool fake_hit_gated)
{
    if (!g_track || !observer || ignored(observer))
        return;
    const u16 id = observer->ID();
    const bool w = watched(id);
    if (!w && !on(p_log_events))
        return;

    std::scoped_lock lock(s_lock);
    if (heard)
    {
        Track& t = s_tracks[id];
        t.snd_time = Device.dwTimeGlobal;
        t.snd_type = sound_type;
        t.snd_power = weighted_power;
        t.snd_fake_hit = fake_hit;
    }

    if (w && on(p_log_sound) && can_log())
    {
        string128 n, wpn;
        Msg("~ [stealth] snd t=%u obs=%s type=%s(%08x) d=%.1f pow=%.3f w=%.3f thr=%.3f heard=%d fakehit=%d%s st=%s wpn=%s", Device.dwTimeGlobal, obj_name(observer, n),
            sound_kind(sound_type), u32(sound_type), distance, raw_power, weighted_power, threshold, heard ? 1 : 0, fake_hit ? 1 : 0, fake_hit_gated ? " gated=1" : "",
            actor_state(), actor_weapon(sound_type, wpn));
    }
}

void on_monster_sound(const CBaseMonster* observer, int sound_type, float distance, float power, float threshold, bool heard, bool near_hit, LPCSTR why)
{
    if (!g_track || !observer || ignored(observer) || !watched(observer->ID()) || !on(p_log_sound))
        return;

    std::scoped_lock lock(s_lock);
    if (!can_log())
        return;
    string128 n, wpn;
    Msg("~ [stealth] msnd t=%u obs=%s type=%s(%08x) d=%.1f pow=%.3f thr=%.3f heard=%d nearhit=%d%s%s st=%s wpn=%s", Device.dwTimeGlobal, obj_name(observer, n),
        sound_kind(sound_type), u32(sound_type), distance, power, threshold, heard ? 1 : 0, near_hit ? 1 : 0, why ? " why=" : "", why ? why : "", actor_state(),
        actor_weapon(sound_type, wpn));
}

void on_enemy_selected(const CCustomMonster* observer)
{
    if (!g_track || !observer || ignored(observer))
        return;
    const u16 id = observer->ID();
    if (!tracked(id))
        return;
    CActor* a = Actor();
    if (!a)
        return;

    std::scoped_lock lock(s_lock);
    Track& t = s_tracks[id];
    if (repeat_in_episode(t.enemy_last) || !can_log())
        return;

    const CMemoryManager& mm = observer->memory();
    const bool vis_now = mm.visual().visible_now(a);

    u32 vis_time = 0;
    for (const auto& v : mm.visual().objects())
        if (v.m_object == a)
        {
            vis_time = v.m_level_time;
            break;
        }

    u32 snd_time = 0;
    int snd_type = 0;
    float snd_power = 0.f;
    for (const auto& snd : mm.sound().objects())
        if (snd.m_object == a && snd.m_level_time > snd_time)
        {
            snd_time = snd.m_level_time;
            snd_type = snd.sound_type();
            snd_power = snd.m_power;
        }

    u32 hit_time = 0;
    for (const auto& h : mm.hit().objects())
        if (h.m_object == a && h.m_level_time > hit_time)
            hit_time = h.m_level_time;

    string128 n;
    string32 a_vis, a_seen, a_snd, a_hit, a_scr, a_heard;
    Msg("~ [stealth] enemy t=%u obs=%s rank=%d d=%.1f ctx: vis_now=%d vis_mem=%s seen=%s(after %.2fs) snd_mem=%s %s pow=%.3f heard=%s%s hit_mem=%s script=%s st=%s",
        Device.dwTimeGlobal, obj_name(observer, n), obj_rank(observer), observer->Position().distance_to(a->Position()), vis_now ? 1 : 0, age(vis_time, a_vis),
        age(t.seen_time, a_seen), float(t.seen_after) / 1000.f, age(snd_time, a_snd), snd_time ? sound_kind(snd_type) : "-", snd_power, age(t.snd_time, a_heard),
        t.snd_fake_hit ? "(fakehit)" : "", age(hit_time, a_hit), age(t.script_time, a_scr), actor_state());
}

void set_monster_add_source(LPCSTR source) { s_monster_source = source; }

void on_monster_enemy_added(const CBaseMonster* observer, const CEntityAlive* enemy, bool positional)
{
    if (!g_track || !observer || !enemy || enemy != Actor() || ignored(observer) || !tracked(observer->ID()))
        return;

    std::scoped_lock lock(s_lock);
    if (repeat_in_episode(s_tracks[observer->ID()].macq_last) || !can_log())
        return;
    string128 n;
    Msg("~ [stealth] macq t=%u obs=%s src=%s d=%.1f st=%s", Device.dwTimeGlobal, obj_name(observer, n), s_monster_source ? s_monster_source : (positional ? "shared" : "other"),
        observer->Position().distance_to(enemy->Position()), actor_state());
}

void on_script_touch(const CObject* observer, const CObject* target)
{
    if (!g_track || !observer || !target || target != Actor() || !tracked(observer->ID()))
        return;
    std::scoped_lock lock(s_lock);
    s_tracks[observer->ID()].script_time = Device.dwTimeGlobal;
}

// ---------------------------------------------------------------------------
// Lua
// ---------------------------------------------------------------------------

namespace
{
int lua_set(lua_State* L)
{
    LPCSTR name = lua_isstring(L, 1) ? lua_tostring(L, 1) : nullptr;
    ParamDef* p = find_param(name);
    if (!p || !lua_isnumber(L, 2))
    {
        Msg("! [stealth] set: unknown parameter or non-numeric value [%s]", name ? name : "nil");
        lua_pushboolean(L, 0);
        return 1;
    }
    const float v = float(lua_tonumber(L, 2));
    if (v < p->min_value || v > p->max_value)
    {
        Msg("! [stealth] set: %s=%.4f outside [%.4f, %.4f]", p->name, v, p->min_value, p->max_value);
        lua_pushboolean(L, 0);
        return 1;
    }
    std::scoped_lock lock(s_lock);
    *p->value = v;
    refresh();
    lua_pushboolean(L, 1);
    return 1;
}

int lua_get(lua_State* L)
{
    ParamDef* p = find_param(lua_isstring(L, 1) ? lua_tostring(L, 1) : nullptr);
    if (!p)
        return 0;
    lua_pushnumber(L, *p->value);
    return 1;
}

int lua_dump(lua_State*)
{
    std::scoped_lock lock(s_lock);
    for (const auto& p : s_params)
        Msg("~ [stealth] param %s=%.4f (default %.4f)", p.name, *p.value, p.default_value);
    for (u32 i = 0; i < s_watch_n; ++i)
    {
        string128 n;
        Msg("~ [stealth] watch %s", obj_name(Level().Objects.net_Find(s_watch[i]), n));
    }
    Msg("~ [stealth] tracking=%d watched=%u", g_track ? 1 : 0, s_watch_n);
    return 0;
}

int lua_watch(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 0;
    const u16 id = u16(lua_tointeger(L, 1));
    std::scoped_lock lock(s_lock);
    bool ok = watched(id);
    if (!ok && s_watch_n < MAX_WATCH)
    {
        s_watch[s_watch_n++] = id;
        s_tracks.erase(id);
        ok = true;
    }
    if (!ok)
        Msg("! [stealth] watch: list full (%u)", MAX_WATCH);
    refresh();
    lua_pushboolean(L, ok ? 1 : 0);
    return 1;
}

int lua_unwatch(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 0;
    const u16 id = u16(lua_tointeger(L, 1));
    std::scoped_lock lock(s_lock);
    for (u32 i = 0; i < s_watch_n; ++i)
        if (s_watch[i] == id)
        {
            s_watch[i] = s_watch[--s_watch_n];
            break;
        }
    refresh();
    return 0;
}

int lua_unwatch_all(lua_State*)
{
    std::scoped_lock lock(s_lock);
    s_watch_n = 0;
    s_tracks.clear();
    refresh();
    return 0;
}

// debug: forget the actor in this observer's visual memory so a measurement restarts from zero
int lua_reset(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 0;
    const u16 id = u16(lua_tointeger(L, 1));
    CCustomMonster* m = smart_cast<CCustomMonster*>(Level().Objects.net_Find(id));
    CActor* a = Actor();
    if (!m || !a || !m->g_Alive())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    m->memory().visual().remove_links(a);
    std::scoped_lock lock(s_lock);
    Track& t = s_tracks[id];
    t.start = 0;
    t.last_why = -1;
    lua_pushboolean(L, 1);
    return 1;
}

// per-NPC vision profile: nlc_stealth_set_vision(id, free_section, danger_section); "" keeps that profile.
// Lasts until the NPC reloads its sections (respawn, save load); scripts re-apply on spawn.
int lua_set_vision(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 0;
    CCustomMonster* m = smart_cast<CCustomMonster*>(Level().Objects.net_Find(u16(lua_tointeger(L, 1))));
    LPCSTR free_section = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";
    LPCSTR danger_section = lua_isstring(L, 3) ? lua_tostring(L, 3) : "";
    const bool ok = m && m->g_Alive() && m->memory().visual().nlc_set_vision_sections(free_section, danger_section);
    string128 n;
    Msg("%s [stealth] set_vision %s free=[%s] danger=[%s] %s", ok ? "~" : "!", obj_name(m, n), free_section, danger_section, ok ? "ok" : "failed");
    lua_pushboolean(L, ok ? 1 : 0);
    return 1;
}

CAI_Stalker* lua_stalker(lua_State* L, int index)
{
    if (!lua_isnumber(L, index))
        return nullptr;
    CAI_Stalker* s = smart_cast<CAI_Stalker*>(Level().Objects.net_Find(u16(lua_tointeger(L, index))));
    return (s && s->g_Alive()) ? s : nullptr;
}

// suspicion of a stalker toward the actor: value (0..1), age of the last positive evaluation in ms (-1 = none), x, y, z of that point
int lua_suspicion(lua_State* L)
{
    CAI_Stalker* s = lua_stalker(L, 1);
    if (!s)
    {
        lua_pushnumber(L, 0);
        return 1;
    }
    CVisualMemoryManager& v = s->memory().visual();
    lua_pushnumber(L, v.nlc_suspicion());
    lua_pushnumber(L, v.m_nlc_point_time ? double(Device.dwTimeGlobal - v.m_nlc_point_time) : -1.0);
    lua_pushnumber(L, v.m_nlc_point.x);
    lua_pushnumber(L, v.m_nlc_point.y);
    lua_pushnumber(L, v.m_nlc_point.z);
    lua_pushnumber(L, heard_kind(v));
    return 6;
}

// per-NPC detection rate factor for the actor (suspicion "on edge"); 1 = normal
int lua_set_rate_k(lua_State* L)
{
    CAI_Stalker* s = lua_stalker(L, 1);
    if (s && lua_isnumber(L, 2))
        s->memory().visual().m_nlc_rate_k = std::clamp(float(lua_tonumber(L, 2)), 0.1f, 5.f);
    return 0;
}

// harness: report suspicion value v (0..1) for ms milliseconds (default 1000), point = actor position now
int lua_force_suspicion(lua_State* L)
{
    CAI_Stalker* s = lua_stalker(L, 1);
    CActor* a = Actor();
    if (!s || !a || !lua_isnumber(L, 2))
        return 0;
    CVisualMemoryManager& v = s->memory().visual();
    v.m_nlc_forced = std::clamp(float(lua_tonumber(L, 2)), 0.f, 1.f);
    v.m_nlc_forced_until = Device.dwTimeGlobal + (lua_isnumber(L, 3) ? u32(lua_tointeger(L, 3)) : 1000u);
    v.m_nlc_point = a->Position();
    v.m_nlc_point_time = Device.dwTimeGlobal;
    return 0;
}

// night-vision device flag (nvd_floor)
int lua_set_nvd(lua_State* L)
{
    CAI_Stalker* s = lua_stalker(L, 1);
    if (s)
        s->memory().visual().m_nlc_nvd = lua_toboolean(L, 2) != 0;
    return 0;
}

// eye glow particle: nlc_stealth_eye_glow(id, on[, particle]); follows the eye bone every frame
int lua_eye_glow(lua_State* L)
{
    CAI_Stalker* s = lua_stalker(L, 1);
    if (!s)
        return 0;
    const bool want = lua_toboolean(L, 2) != 0;
    if (!want)
    {
        stop_eye_glow(s);
        return 0;
    }
    if (s_glows.find(s->ID()) != s_glows.end())
        return 0;
    LPCSTR name = lua_isstring(L, 3) ? lua_tostring(L, 3) : "stealth_nvg\\nvg_dot";
    CParticlesObject* ps = CParticlesObject::Create(name, FALSE, false);
    if (!ps)
        return 0;
    s_glows[s->ID()] = ps;
    s->memory().visual().m_nlc_glow = true;
    update_eye_glow(s);
    return 0;
}

// ground material under the actor and its footstep factor, outfit noise factor
int lua_actor_surface(lua_State* L)
{
    LPCSTR name = "?";
    const float k = surface_k(&name);
    lua_pushstring(L, name);
    lua_pushnumber(L, k);
    lua_pushnumber(L, outfit_noise_k());
    return 3;
}

// environment light: total (ambient + sky + sun, max components), ambient, sky (hemi), sun
int lua_env_light(lua_State* L)
{
    const CEnvDescriptorMixer* env = GamePersistent().Environment().CurrentEnv;
    if (!env)
    {
        lua_pushnumber(L, 1);
        return 1;
    }
    const float ambient = _max(env->ambient.x, _max(env->ambient.y, env->ambient.z));
    const float hemi = _max(env->hemi_color.x, _max(env->hemi_color.y, env->hemi_color.z));
    const float sun = _max(env->sun_color.x, _max(env->sun_color.y, env->sun_color.z));
    lua_pushnumber(L, ambient + hemi + sun);
    lua_pushnumber(L, ambient);
    lua_pushnumber(L, hemi);
    lua_pushnumber(L, sun);
    return 4;
}

// returns after_ms (-1 = never), distance, danger profile, level time of the acquisition
int lua_last_seen(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 0;
    const u16 id = u16(lua_tointeger(L, 1));
    std::scoped_lock lock(s_lock);
    const auto it = s_tracks.find(id);
    if (it == s_tracks.end() || !it->second.seen_time)
    {
        lua_pushnumber(L, -1);
        return 1;
    }
    lua_pushnumber(L, it->second.seen_after);
    lua_pushnumber(L, it->second.seen_dist);
    lua_pushboolean(L, it->second.seen_danger ? 1 : 0);
    lua_pushnumber(L, it->second.seen_time);
    return 4;
}

// debug overlay (20.15): see-through of this stalker's last ray to the actor (-1 = actor not in its view cone),
// the ray cutoff at the current distance, eye-to-actor distance, last partly see-through material on such a ray
int lua_ray(lua_State* L)
{
    CAI_Stalker* s = lua_stalker(L, 1);
    CActor* a = Actor();
    if (!s || !a)
        return 0;
    CVisualMemoryManager& v = s->memory().visual();
    Fvector eye, c;
    eye.set(s->eye_matrix.c);
    a->Center(c);
    const float d = eye.distance_to(c);
    lua_pushnumber(L, v.visible_transparency_threshold(a));
    lua_pushnumber(L, actor_ray_threshold(d, v.transparency_threshold()));
    lua_pushnumber(L, d);
    const u16 mtl = v.m_nlc_ray_mtl.load(std::memory_order_relaxed);
    lua_pushstring(L, mtl < GMLib.CountMaterial() ? GMLib.GetMaterialByIdx(mtl)->m_Name.c_str() : "-");
    return 4;
}
} // namespace

void script_register(lua_State* L)
{
    lua_register(L, "nlc_stealth_set", lua_set);
    lua_register(L, "nlc_stealth_get", lua_get);
    lua_register(L, "nlc_stealth_dump", lua_dump);
    lua_register(L, "nlc_stealth_watch", lua_watch);
    lua_register(L, "nlc_stealth_unwatch", lua_unwatch);
    lua_register(L, "nlc_stealth_unwatch_all", lua_unwatch_all);
    lua_register(L, "nlc_stealth_reset", lua_reset);
    lua_register(L, "nlc_stealth_last_seen", lua_last_seen);
    lua_register(L, "nlc_stealth_set_vision", lua_set_vision);
    lua_register(L, "nlc_stealth_suspicion", lua_suspicion);
    lua_register(L, "nlc_stealth_set_rate_k", lua_set_rate_k);
    lua_register(L, "nlc_stealth_force_suspicion", lua_force_suspicion);
    lua_register(L, "nlc_stealth_set_nvd", lua_set_nvd);
    lua_register(L, "nlc_stealth_eye_glow", lua_eye_glow);
    lua_register(L, "nlc_stealth_env_light", lua_env_light);
    lua_register(L, "nlc_stealth_actor_surface", lua_actor_surface);
    lua_register(L, "nlc_stealth_ray", lua_ray);
}
} // namespace nlc_stealth
