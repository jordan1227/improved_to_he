#pragma once

#include <al.h>
#include <efx-presets.h>

#include "SoundRender.h"
#include "SoundRender_Environment.h"
#include "SoundRender_Cache.h"

class CSoundRender_Core : public CSound_manager_interface
{
    volatile BOOL bLocked{};

protected:
    virtual void _create_data(ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type);
    virtual void _destroy_data(ref_sound_data& S);

protected:
    BOOL bListenerMoved{};

    bool e_currentPaused{false};

    CSoundRender_Environment e_current;
    CSoundRender_Environment* e_target{};

public:
    typedef std::pair<ref_sound_data_ptr, float> event;
    xr_vector<event> s_events;

public:
    BOOL bPresent{};
    BOOL bEAX{}; // Boolean variable to indicate presence of EAX Extension
    BOOL bDeferredEAX{};
    bool bEFX{}; // boolean variable to indicate presence of EFX Extension
    BOOL bReady{};

    CTimer Timer{};
    float fTimer_Value{};
    float fTimer_Delta{};
    sound_event* Handler{};

protected:
    // Collider
    CDB::COLLIDER geom_DB{};
    CDB::MODEL* geom_SOM{};
    CDB::MODEL* geom_MODEL{};
    CDB::MODEL* geom_ENV{};

    // Containers
    xr_vector<CSoundRender_Source*> s_sources;
    xr_vector<CSoundRender_Emitter*> s_emitters;
    u32 s_emitters_u{}; // emitter update marker
    xr_vector<CSoundRender_Target*> s_targets;
    xr_vector<CSoundRender_Target*> s_targets_defer;
    u32 s_targets_pu{}; // parameters update
    SoundEnvironment_LIB* s_environment{};
    xr_vector<u16> s_environment_ids;

    int m_iPauseCounter{};

public:
    bool supports_float_pcm{};

    // Cache
    CSoundRender_Cache cache{};
    u32 cache_bytes_per_line{};

protected:
    virtual void i_eax_set(const GUID* guid, u32 prop, void* val, u32 sz) = 0;
    virtual void i_eax_get(const GUID* guid, u32 prop, void* val, u32 sz) = 0;

    std::mutex m_bLocked;

public:
    CSoundRender_Core();
    virtual ~CSoundRender_Core();

    // General
    virtual void _initialize(int stage) = 0;
    virtual void _clear() = 0;
    virtual void _restart();

    // Sound interface
    void verify_refsound(ref_sound& S);
    virtual void create(ref_sound& S, LPCSTR fName, esound_type sound_type, int game_type);
    virtual void attach_tail(ref_sound& S, LPCSTR fName);

    virtual void clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type);
    virtual void destroy(ref_sound& S);
    virtual void stop_emitters();
    virtual int pause_emitters(bool val);

    virtual void play(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f);
    virtual void play_at_pos(ref_sound& S, CObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f);
    virtual void play_no_feedback(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0);
    virtual void set_master_volume(float f) = 0;
    virtual void set_geometry_env(IReader* I);
    virtual void set_geometry_som(IReader* I);
    virtual void set_geometry_occ(CDB::MODEL* M);
    virtual void set_handler(sound_event* E);

    virtual void update(const Fvector& P, const Fvector& D, const Fvector& N);
    virtual void update_events();
    virtual void statistic(CSound_stats* dest, CSound_stats_ext* ext);

    // listener
    virtual void update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) = 0;

    // eax listener
    void i_eax_listener_set(CSound_environment* E);
    void i_eax_commit_setting();

    // efx listener
    void i_efx_listener_set(CSound_environment* E);
    bool i_efx_commit_setting();

    virtual CSound_environment* DbgCurrentEnv() override { return e_target; }
    virtual void DbgCurrentEnvPaused(bool v) override { e_currentPaused = v; }
    virtual void DbgCurrentEnvSave() override { env_save_all(); }

public:
    CSoundRender_Source* i_create_source(LPCSTR name);
    void i_destroy_source(CSoundRender_Source* S);
    CSoundRender_Emitter* i_play(ref_sound* S, BOOL _loop, float delay, u32 flags = 0);
    void i_start(CSoundRender_Emitter* E);
    void i_stop(CSoundRender_Emitter* E);
    void i_rewind(CSoundRender_Emitter* E);
    BOOL i_allow_play(CSoundRender_Emitter* E);

    virtual void object_relcase(CObject* obj);

    virtual float get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion = 0.2f);
    float get_occlusion(Fvector& P, float R, Fvector* occ);
    CSoundRender_Environment* get_environment_def();
    CSoundRender_Environment* get_environment(const Fvector& P);

    void env_load();
    void env_unload();
    void env_save_all() const;
    void env_apply();

protected: // EFX
    EFXEAXREVERBPROPERTIES efx_reverb{};
    ALuint effect{};
    ALuint slot{};

    bool EFXTestSupport();
    void InitAlEFXAPI();

    void release_efx_objects();

    // NLC: EAX reverb effect type, second (outdoor echo) slot, commit throttle
    bool efx_eax{};
    ALuint effect_echo{};
    ALuint slot_echo{};
    bool efx_slots_attached{};
    float efx_next_commit{};
    EFXEAXREVERBPROPERTIES efx_last{}; // last committed room reverb (snd_audio_report)
    bool efx_last_valid{};
    void efx_apply(ALuint fx, const EFXEAXREVERBPROPERTIES& P) const;
    void efx_attach_slots(bool attach);
    void env_to_efx(const CSoundRender_Environment& E, EFXEAXREVERBPROPERTIES& P) const;
    void nlc_room_props(EFXEAXREVERBPROPERTIES& P) const;

public:
    int efx_sends{1}; // auxiliary sends per source granted by the device
    ALuint efx_slot_room() const { return slot; }
    ALuint efx_slot_echo() const { return slot_echo; }
    bool efx_reverb_on() const { return efx_slots_attached; }

    // NLC occlusion (docs/AUDIO_DESIGN.md section 2)
    SSndOccConfig nlc_cfg{};
    int nlc_rays_left{};
    u32 nlc_rays_used{};
    void nlc_occlusion(const Fvector& P, float dist, bool loud, bool leaks, float& gain, float& hf, float& db, u8& path, LPCSTR log_name);
    float nlc_source_enclosure(const Fvector& P);
    virtual void set_occlusion_config(const SSndOccConfig& cfg);
    virtual void nlc_report();
    virtual float nlc_listener_enclosure() const { return nlc_enc; }
    virtual float nlc_rain_indoor() const { return nlc_cfg.rain_indoor; }
    virtual void nlc_set_listener_filter(float gain, float hf, float fade_sec);
    virtual void nlc_set_listener_filter_exempt(LPCSTR parts);
    bool nlc_lf_exempt(LPCSTR name) const;
    float nlc_lf_gain{1.f}, nlc_lf_hf{1.f}; // current listener filter (smoothed)
    bool nlc_hrtf_on{}; // the device renders through HRTF (2D sounds bypass it, see TargetA)
    bool nlc_filters_active() const;

    // NLC occlusion queue (AUDIO_DESIGN 13.2 item 3): due emitters are collected during the update and served by priority
    void nlc_queue(CSoundRender_Emitter* E, float key) { nlc_due.push_back({E, key}); }
    void nlc_unqueue(CSoundRender_Emitter* E);
    bool nlc_no_occlude(LPCSTR name) const;
    bool nlc_spatial_stereo(LPCSTR name) const;

    // NLC room probe (section 3.1): smoothed listener enclosure 0..1, mean free distance, absorption
    float nlc_enc{};
    float nlc_mean_dist{60.f};
    float nlc_near_dist{60.f}; // 25th percentile probe distance (first reflections)
    float nlc_room_size{60.f}; // decay size: sqrt(trimmed mean x cube mean) of the probe distances
    float nlc_absorb{0.1f};

protected:
    CDB::COLLIDER nlc_DB{};
    struct nlc_hit
    {
        float range;
        u16 mtl;
        bool enter;
    };
    xr_vector<nlc_hit> nlc_hits;
    float nlc_path(const Fvector& A, const Fvector& B, float& hf, u32& occluders, bool& terrain_only);

    struct nlc_due_t
    {
        CSoundRender_Emitter* E;
        float key;
    };
    xr_vector<nlc_due_t> nlc_due;
    void nlc_process_due();

    // raw occlusion (best path before caps/strength) by listener and source cell, 0.5 m, kept 0.3 s
    struct nlc_occ_cache_t
    {
        s32 l[3], s[3];
        float time;
        float db, hf;
        u8 path;
        bool terrain;
        bool leaks;
        bool valid;
    };
    static constexpr u32 nlc_occ_cache_size = 128;
    nlc_occ_cache_t nlc_occ_cache[nlc_occ_cache_size]{};
    xr_unordered_map<u64, float> nlc_enc_cache; // source enclosure by 3 m cell (static geometry)
    u32 nlc_cache_hits{};
    const CDB::MODEL* nlc_cache_model{};
    void nlc_cache_clear();
    float nlc_probe_accum{};
    float nlc_lf_gain_t{1.f}, nlc_lf_hf_t{1.f}, nlc_lf_rate{4.f};
    xr_vector<xr_string> nlc_lf_exempt_parts;
    std::mutex nlc_lf_lock;
    bool nlc_probe_ray(const Fvector& P, const Fvector& dir, float range, float& dist, float& absorb);
    void nlc_probe_update(float dt);
    static constexpr u32 nlc_probe_dirs = 25; // 24 around + sky
    float nlc_probe_dist[nlc_probe_dirs]{};
    float nlc_probe_abs[nlc_probe_dirs]{};
    u32 nlc_probe_idx{};
    bool nlc_probe_valid{};
    float nlc_rv_log_time{};
    int nlc_dist_model_applied{-1};

    const SSndOccMtl& nlc_mtl(u32 idx) const { return idx < nlc_cfg.mtl.size() ? nlc_cfg.mtl[idx] : nlc_cfg.def; }
};

extern XRSOUND_API CSoundRender_Core* SoundRender;
