////////////////////////////////////////////////////////////////////////////
//	Module 		: visual_memory_manager.h
//	Created 	: 02.10.2001
//  Modified 	: 19.11.2003
//	Author		: Dmitriy Iassenev
//	Description : Visual memory manager
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "visual_memory_params.h"
#include "memory_space.h"

class CCustomMonster;
class CAI_Stalker;
class vision_client;

class CVisualMemoryManager
{
#ifdef DEBUG
    friend class CAI_Stalker;
#endif
public:
    typedef MemorySpace::CVisibleObject CVisibleObject;
    typedef MemorySpace::CNotYetVisibleObject CNotYetVisibleObject;
    typedef xr_deque<CVisibleObject> VISIBLES;
    typedef xr_vector<CObject*> RAW_VISIBLES;
    typedef xr_vector<CNotYetVisibleObject> NOT_YET_VISIBLES;

private:
    struct CDelayedVisibleObject
    {
        ALife::_OBJECT_ID m_object_id;
        CVisibleObject m_visible_object;
    };

private:
    typedef xr_vector<CDelayedVisibleObject> DELAYED_VISIBLE_OBJECTS;

private:
    CCustomMonster* m_object;
    CAI_Stalker* m_stalker;
    vision_client* m_client;

private:
    RAW_VISIBLES m_visible_objects;
    VISIBLES* m_objects;
    NOT_YET_VISIBLES m_not_yet_visible_objects;

private:
    DELAYED_VISIBLE_OBJECTS m_delayed_objects;

private:
    CVisionParameters m_free;
    CVisionParameters m_danger;

private:
    u32 m_max_object_count;
    u32 m_adaptive_max_object_count;
    bool m_enabled;
    u32 m_last_update_time;

public:
    void add_visible_object(const CObject* object, float time_delta, bool fictitious = false);

protected:
    IC void fill_object(CVisibleObject& visible_object, const CGameObject* game_object);
    void add_visible_object(CVisibleObject visible_object);
    float object_visible_distance(const CGameObject* game_object, float& object_distance) const;
    float object_luminocity(const CGameObject* game_object) const;
    float get_visible_value(float distance, float object_distance, float time_delta, float object_velocity, float luminocity, float trans) const;
    float get_object_velocity(const CGameObject* game_object, const CNotYetVisibleObject& not_yet_visible_object) const;
    u32 get_prev_time(const CGameObject* game_object) const;

public:
    u32 visible_object_time_last_seen(const CObject* object) const;

protected:
    void add_not_yet_visible_object(const CNotYetVisibleObject& not_yet_visible_object);
    CNotYetVisibleObject* not_yet_visible_object(const CGameObject* game_object);

private:
    void initialize();

public:
    CVisualMemoryManager(CCustomMonster* object);
    CVisualMemoryManager(CAI_Stalker* stalker);
    CVisualMemoryManager(vision_client* client);
    virtual ~CVisualMemoryManager();
    virtual void reinit();
    virtual void reload(LPCSTR section);
    virtual void update(float time_delta);
    virtual float feel_vision_mtl_transp(CObject* O, u32 element);
    void remove_links(CObject* object);
    // NLC: per-NPC vision profile (nlc_stealth_set_vision); "" keeps that profile
    bool nlc_set_vision_sections(LPCSTR free_section, LPCSTR danger_section);
    // NLC: per-NPC stealth state (nlc_stealth; docs/STEALTH_DESIGN.md 15). Not saved: scripts re-apply on spawn.
    Fvector m_nlc_point{}; // actor position at the last evaluation that added to the sum
    u32 m_nlc_point_time = 0;
    float m_nlc_rate_k = 1.f; // detection rate factor for the actor (suspicion "on edge")
    float m_nlc_forced = 0.f; // harness: forced suspicion value until m_nlc_forced_until
    u32 m_nlc_forced_until = 0;
    u32 m_nlc_last_shot = 0; // this NPC fired (muzzle flash for npc_light_k)
    u32 m_nlc_torch_time = 0; // torch flag cache for npc_light_k
    bool m_nlc_torch_on = false;
    bool m_nlc_nvd = false; // night-vision device (nvd_floor)
    bool m_nlc_glow = false; // eye glow particle active
    // actor sum / threshold (0..1), forced value included
    float nlc_suspicion();

    // NLC: monster senses, read from the monster section in reload() (docs/STEALTH_DESIGN.md 16); 0 / -1 = vanilla
    float m_nlc_light_k = 0.f; // share of the AI sky and near-range light terms
    float m_nlc_dark_floor = 0.f; // night vision: light floor for the actor
    float m_nlc_rain_k = 0.f; // rate x (1 - rain_k x rain density)
    float m_nlc_pack_range = 0.f; // pack sharing gate range (0 = vanilla instant sharing)
    float m_nlc_pack_delay_min = 0.5f;
    float m_nlc_pack_delay_max = 1.5f;
    float m_nlc_impact_max = -1.f; // bullet impacts and whines make the shooter an enemy only within this range
    float m_nlc_near_hit_max = -1.f; // the 2 m near-miss counts as a hit only within this shooter range
    u32 m_nlc_pack_seen = 0;
    u32 m_nlc_pack_last = 0;
    u32 m_nlc_pack_delay = 0;
    u32 m_nlc_notice_next = 0;
    // NLC M3 (docs/STEALTH_DESIGN.md 17): heard concern (faint shot, near miss), investigate style, corpse checks
    float m_nlc_heard = 0.f; // heard suspicion value (stalkers), counts in nlc_suspicion() until m_nlc_heard_until
    u32 m_nlc_heard_until = 0;
    u32 m_nlc_heard_kind = 0; // 1 faint shot, 2 near miss
    u32 m_nlc_near_miss_next = 0;
    u32 m_nlc_inv_style = 0; // monsters: 1 walk, 2 sneak, 3 hold and watch, 4 run (0 = vanilla walk)
    u32 m_nlc_inv_until = 0;
    float m_nlc_shot_alert_pow = -1.f; // monsters: species override of monster_shot_alert_pow
    float m_nlc_corpse_count = 0.f; // monsters: pack mates that check this monster's corpse
    float m_nlc_corpse_radius = 0.f;
    float m_nlc_style_w[4] = {40.f, 35.f, 10.f, 15.f}; // walk, sneak, hold, run
    // NLC M4 (docs/STEALTH_DESIGN.md 18)
    u32 m_nlc_concern_count = 0; // monsters: concern events (near miss, faint shot) in the current window
    u32 m_nlc_concern_last = 0;
    float m_nlc_concern_bold = -1.f; // species: 1 bold (escalates to attack), 0 timid (escalates to flight), -1 no escalation
    float m_nlc_concern_range = 30.f; // species: bold escalation makes the actor an enemy within this range
    u32 m_nlc_firsthand = 0; // monsters: last time this monster acquired the actor by its own senses
    u32 m_nlc_notice_serial = 0; // monsters: bumped by every accepted investigate impulse (the investigate state retargets)
    u32 m_nlc_notice_prio = 0; // monsters: priority of the active impulse (corpse 1, others 2)
    // NLC pass 5 (docs/STEALTH_DESIGN.md 20)
    Fvector m_nlc_notice_point{}; // monsters: target of the active impulse (no retarget to points close to it)
    u32 m_nlc_concern_total = 0; // monsters: concern events in the window, not reset by escalation (shrinks the point error)
    u32 m_nlc_hunt_until = 0; // monsters: committed search around the guessed shooter point
    u32 m_nlc_hunt_next = 0;
    Fvector m_nlc_hunt_point{};
    u32 m_nlc_alert_until = 0; // monsters: "lost the trail" alert after a hunt (faster detection)
    u32 m_nlc_pack_losses = 0; // monsters: pack mates killed near this one recently
    u32 m_nlc_pack_loss_time = 0;
    float m_nlc_hunt_time = 25000.f; // species: hunt duration (ms)
    float m_nlc_hunt_error_k = 0.5f; // species: hunt point error relative to the last guess
    float m_nlc_hunt_detect_k = 1.5f; // species: vision and senses rate factor while hunting or alert
    float m_nlc_hunt_alert_ms = 30000.f; // species: alert after an empty hunt
    float m_nlc_hunt_flankers = 0.f; // species: pack mates sent to flank the hunt point
    float m_nlc_hunt_flee_losses = 0.f; // species: pack losses after which a bold monster flees instead (0 = never)
    u32 m_nlc_wall_time = 0; // listeners: wall check cache for actor sounds
    float m_nlc_wall_k = 1.f;
    Fvector m_nlc_wall_from{};
    struct NlcSense // aura sense (psy or smell), generalised from the poltergeist detection
    {
        float range = 0.f; // 0 = none
        float near_k = 1.f;
        float far_k = 1.f;
        float speed_pow = 0.f; // 0: movement-independent (smell); > 0: (1 + v)^pow - 1 (psy, CoP style)
        float speed_min = 0.f; // actor speed below this is ignored
        float rate = 1.f;
        float loose = 1.f; // level lost per second
        float notice = 0.f; // level: investigate
        float success = 0.f; // level: enemy
        float rain_k = 0.f; // range x (1 - rain_k x rain density)
        bool psy = false; // scaled by the actor's telepathic immunity (psy helmets)
        bool walls = true; // senses through walls (else needs a clear ray)
        float level = 0.f;
        u32 last = 0;
        u32 notice_next = 0;
        u32 log_next = 0;
        Fvector last_actor{};
    } m_nlc_sense;

public:
    bool visible(const CGameObject* game_object, float time_delta);
    bool visible(u32 level_vertex_id, float yaw, float eye_fov) const;

public:
    IC void set_squad_objects(xr_deque<CVisibleObject>* squad_objects);
    CVisibleObject* visible_object(const CGameObject* game_object);

public:
    // this function returns true if and only if
    // specified object is visible now
    bool visible_right_now(const CGameObject* game_object) const;
    float visible_transparency_threshold(const CGameObject*) const;
    // if current_params.m_still_visible_time == 0
    // this function returns true if and only if
    // specified object is visible now
    // if current_params.m_still_visible_time > 0
    // this function returns true if and only if
    // specified object is visible now or
    // some time ago <= current_params.m_still_visible_time
    bool visible_now(const CGameObject* game_object) const;

public:
    void enable(const CObject* object, bool enable);

public:
    IC float visibility_threshold() const;
    IC float transparency_threshold() const;

public:
    IC bool enabled() const;
    IC void enable(bool value);

public:
    IC const VISIBLES& objects() const;
    IC const RAW_VISIBLES& raw_objects() const;
    IC const NOT_YET_VISIBLES& not_yet_visible_objects() const;
    /*IC*/ const CVisionParameters& current_state() const;
    squad_mask_type mask() const;

public:
#ifdef DEBUG
    void check_visibles() const;
#endif

public:
    void save(NET_Packet& packet) const;
    void load(IReader& packet);
    void on_requested_spawn(CObject* object);

private:
    void clear_delayed_objects();
};

#include "visual_memory_manager_inline.h"
