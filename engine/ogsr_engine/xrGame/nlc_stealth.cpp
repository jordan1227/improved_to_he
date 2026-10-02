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

#include <mutex>

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

void refresh()
{
    g_track = s_watch_n > 0 || p_log_events > 0.f;
    g_actor_light_bonus = g_sky_k > 0.f || p_torch_k > 0.f || p_flash_k > 0.f || p_near_k > 0.f;
    g_actor_rate_factor = p_crouch_k != 1.f || p_creep_k != 1.f;
    for (const float k : p_rank_k)
        g_actor_rate_factor = g_actor_rate_factor || k != 1.f;
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

// "-" or "<section> sil=0|1" for the actor's active weapon (weapon sounds only)
LPCSTR actor_weapon(int sound_type, string128& buf)
{
    CActor* a = Actor();
    if (!a || !(u32(sound_type) & SOUND_TYPE_WEAPON))
        return "-";
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

void on_actor_shot(bool silenced) { (silenced ? s_last_shot_silenced : s_last_shot) = Device.dwTimeGlobal; }

float actor_rate_factor(const CCustomMonster* observer, float* rank_k, float* stance_k)
{
    float rk = 1.f, sk = 1.f;
    if (const CAI_Stalker* stalker = smart_cast<const CAI_Stalker*>(observer))
        rk = p_rank_k[rank_index(int(stalker->Rank()))];
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
    return rk * sk;
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
            float rank_k = 1.f, stance_k = 1.f;
            actor_rate_factor(observer, &rank_k, &stance_k);

            string128 n;
            Msg("~ [stealth] vis t=%u obs=%s rank=%d prof=%s st=%s d=%.1f D=%.1f L=%.4f sky=%.2f hemi=%.3f +sky=%.3f +otorch=%.3f +flash=%.3f +near=%.3f Lp=%.3f torch=%d v=%.2f vp=%.2f "
                "rk=%.2f stk=%.2f ray=%.2f tr=%.2f fog=%.2f inc=%.3f sum=%.1f/%.0f%s",
                now, obj_name(observer, n), obj_rank(observer), s.danger_profile ? "danger" : "free", actor_state(), s.object_distance, s.view_distance, lum_raw,
                sky_vis, sky_bright, parts.sky, parts.torch, parts.flash, parts.near_light, s.luminocity_used, torch ? 1 : 0, s.velocity, actor_speed(), rank_k, stance_k, s.ray,
                s.trans, fog_factor, s.increment, s.sum_after, s.threshold,
                s.out_of_range ? " out_of_range=1" : (s.always_visible ? " always_visible=1" : ""));
        }
    }

    if (s.sum_after <= 0.f)
        t.start = 0;
}

void on_vision_skipped(const CCustomMonster* observer, float ray, float ray_threshold, float sum_before)
{
    if (!g_track || !observer || ignored(observer))
        return;
    const u16 id = observer->ID();
    const bool w = watched(id);
    if (!w && !on(p_log_events))
        return;

    std::scoped_lock lock(s_lock);
    Track& t = s_tracks[id];
    const bool reset = sum_before > 0.f;
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
    Msg("~ [stealth] nolos t=%u obs=%s why=%s ray=%.2f thr=%.2f sum=%.1f%s", Device.dwTimeGlobal, obj_name(observer, n), WHY[why], ray, ray_threshold, sum_before,
        reset ? " reset=1" : "");
}

void on_stalker_sound(const CCustomMonster* observer, int sound_type, float distance, float raw_power, float weighted_power, float threshold, bool heard, bool fake_hit)
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
        Msg("~ [stealth] snd t=%u obs=%s type=%s(%08x) d=%.1f pow=%.3f w=%.3f thr=%.3f heard=%d fakehit=%d st=%s wpn=%s", Device.dwTimeGlobal, obj_name(observer, n),
            sound_kind(sound_type), u32(sound_type), distance, raw_power, weighted_power, threshold, heard ? 1 : 0, fake_hit ? 1 : 0, actor_state(),
            actor_weapon(sound_type, wpn));
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
}
} // namespace nlc_stealth
