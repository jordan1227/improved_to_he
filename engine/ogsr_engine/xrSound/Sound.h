#pragma once

#define XRSOUND_API

#define OGG_COMMENT_VERSION 0x0003

// refs
class CObject;
class XRSOUND_API CSound_params;
class XRSOUND_API CSound_source;
class XRSOUND_API CSound_emitter;
class XRSOUND_API CSound_stream_interface;
class XRSOUND_API CSound_environment;

XRSOUND_API extern u32 psSoundModel;
XRSOUND_API extern float psSoundVEffects;
XRSOUND_API extern float psSoundVFactor;
XRSOUND_API extern float psSoundVMusic;
XRSOUND_API extern float psSoundRolloff;
XRSOUND_API extern float psSoundOcclusionScale;
XRSOUND_API extern float psSoundLinearFadeFactor; //--#SM+#--
XRSOUND_API extern Flags32 psSoundFlags;
XRSOUND_API extern int psSoundTargets;
XRSOUND_API extern int psSoundCacheSizeMB;
XRSOUND_API extern xr_token* snd_devices_token;
XRSOUND_API extern u32 snd_device_id;
XRSOUND_API extern float psSoundTimeFactor; //--#SM+#--

XRSOUND_API extern BOOL snd_enable_float_pcm;

// NLC audio (docs/AUDIO_DESIGN.md)
XRSOUND_API extern BOOL psSoundOccMode; // 0 legacy single ray, 1 thickness/material/leak model
XRSOUND_API extern float psSoundOccStrength; // dB loss multiplier
XRSOUND_API extern int psSoundOccRays; // ray budget per sound update
XRSOUND_API extern string64 psSoundOccLog; // substring of a sound name: log its occlusion paths
XRSOUND_API extern int psSoundReverbMode; // 0 snd_env zones only, 1 room probe + zones
XRSOUND_API extern float psSoundReverbZoneWeight;
XRSOUND_API extern float psSoundReverbGain;
XRSOUND_API extern float psSoundOutdoorEcho;
XRSOUND_API extern BOOL psSoundReverbLog;
XRSOUND_API extern float psSoundAirAbsorption; // dB per 100 m of high-frequency loss
XRSOUND_API extern BOOL psSoundDelay; // speed-of-sound delay for distant sounds
XRSOUND_API extern float psSoundDelayMinDist;
XRSOUND_API extern float psSoundSpeed;
XRSOUND_API extern float psSoundAmbIndoorGain; // ambient channel gain when fully enclosed
XRSOUND_API extern u32 psSoundHrtf; // 0 off, 1 on, 2 auto (applied on snd_restart)
XRSOUND_API extern float psSoundBulletWhineDist;
XRSOUND_API extern float psSoundBulletWhineVol; // level of the near-miss whine
XRSOUND_API extern BOOL psSoundHrtf2D; // 2D sounds through HRTF too (off: they bypass it as direct stereo)
XRSOUND_API extern BOOL psSoundMaskFilter; // gas-mask listener filter (off: the script target is ignored)
XRSOUND_API extern int psSoundHrtfSet; // HRTF data set index (0 = OpenAL Soft default; applied on snd_restart) // near-miss whine of non-actor bullets within this distance of the camera (0 = off)
XRSOUND_API extern int psSoundDistModel; // 0 OpenAL inverse clamped, 1 engine curve with a fade to max_dist

// NLC: sound occlusion / room absorption of one game material (misc\snd_occlusion.ltx)
struct SSndOccMtl
{
    float surface_db{6.f}; // loss per crossed surface
    float db_per_m{20.f}; // loss per metre of material
    float hf_keep{0.45f}; // high-frequency share kept per crossing
    float hf_ref_m{0.3f}; // each hf_ref_m of thickness costs one more hf_keep
    float default_thick{0.2f}; // used when the entry/exit faces do not pair up
    float absorb{0.1f}; // room probe absorption (reverb decay)
    bool skip{}; // ignored by occlusion and the room probe
    bool terrain{}; // ground: a path through terrain only is capped at terrain_cap (hills)
};

struct SSndOccConfig
{
    xr_vector<SSndOccMtl> mtl; // by game material index (CDB::TRI::material)
    SSndOccMtl def;
    // occlusion
    float strength{1.f};
    float max_db{40.f};
    float min_thick{0.05f};
    float max_thick{3.f};
    float leak_db{3.f};
    float leak_hf{0.7f};
    float leak_side{1.2f};
    float leak_up{1.5f};
    float leak_max_dist{60.f};
    float attack{0.15f}; // smoothing toward clearer, s
    float release{0.35f}; // smoothing toward muffled, s
    float loud_cap{18.f}; // shots / explosions: soft cap of the loss...
    float loud_slope{0.25f}; // ...dB above it count this much...
    float loud_max{24.f}; // ...up to this ceiling; the rest becomes extra HF loss
    float terrain_cap{22.f}; // a path through terrain only (a hill) loses at most this
    float cap_hf_div{40.f}; // capped-away dB turn into HF loss: hf x 10^(-dB / cap_hf_div)
    xr_vector<xr_string> spatial_stereo; // stereo sound name parts kept 3D (OpenAL downmixes and positions them) instead of forced 2D
    xr_vector<xr_string> no_occlude; // sound name parts never occluded (sounds typed world_ambient were exempt before)
    // room probe / reverb
    float probe_len{60.f};
    float enc_near{10.f}; // a probe hit nearer than this counts as fully enclosing
    float enc_far{40.f}; // ... and farther than this not at all
    float roof_open_k{0.4f}; // enclosure multiplier without a roof
    float decay_k{0.03f};
    float decay_min{0.3f};
    float decay_max{4.5f};
    float room_gain{0.32f};
    float outdoor_gain{0.3f}; // open-air tail level (custom near-dry preset)
    float send_mismatch{0.35f}; // room send when listener and source enclosure differ fully
    float echo_gain{0.5f};
    float rain_indoor{0.3f}; // rain volume floor when fully enclosed (before, rain went silent under any roof)
    float hrtf_2d_gain{0.71f}; // level of a mono 2D sound sent past HRTF as direct stereo (both ears get the full signal)
    // ambience
    float amb_indoor_hf{0.4f};
};

// Flags
enum
{
    //ss_Hardware = (1ul << 1ul), // Use hardware mixing only
    ss_EAX = (1ul << 2ul), // Use EAX or EFX
    ss_UseDefaultDevice = (1ul << 3ul),
    ss_forcedword = u32(-1)
};

enum
{
    sq_DEFAULT,
    sq_NOVIRT,
    sq_LIGHT,
    sq_HIGH,
    sq_forcedword = u32(-1)
};
enum
{
    sg_Undefined = 0,
    sg_SourceType = u32(-1),
    sg_forcedword = u32(-1),
};
enum
{
    sm_Looped = (1ul << 0ul), //!< Looped
    sm_2D = (1ul << 1ul), //!< 2D mode
    sm_EnvAmbient = (1ul << 2ul), //!< NLC: weather ambient channel (ducked by listener enclosure, no ray occlusion)
    sm_forcedword = u32(-1),
};
enum esound_type
{
    st_Effect = 0,
    st_Music = 1,
    st_forcedword = u32(-1),
};

class CSound_UserDataVisitor;

class CSound_UserData : public xr_resource
{
public:
    virtual ~CSound_UserData() {}
    virtual void accept(CSound_UserDataVisitor*) = 0;
    virtual void invalidate() = 0;
};
typedef resptr_core<CSound_UserData, resptr_base<CSound_UserData>> CSound_UserDataPtr;

class ref_sound_data : public xr_resource
{
public:
    //	shared_str						nm;
    CSound_source* handle; //!< Pointer to wave-source interface
    CSound_emitter* feedback; //!< Pointer to emitter, automaticaly clears on emitter-stop
    esound_type s_type;
    int g_type; //!< Sound type, usually for AI
    CObject* g_object; //!< Game object that emitts ref_sound
    CSound_UserDataPtr g_userdata;
    shared_str fn_attached[2];

    u32 dwBytesTotal;
    float fTimeTotal;

public:
    ref_sound_data();
    ref_sound_data(LPCSTR fName, esound_type sound_type, int game_type);
    virtual ~ref_sound_data();
    float get_length_sec() const { return fTimeTotal; };
};
typedef resptr_core<ref_sound_data, resptr_base<ref_sound_data>> ref_sound_data_ptr;
/*! \class ref_sound
\brief Sound source + control

The main class respresenting source/emitter interface
This class infact just hides internals and redirect calls to
specific sub-systems
*/
struct ref_sound
{
    ref_sound_data_ptr _p;

public:
    ref_sound() {}
    ~ref_sound() {}

    IC CSound_source* _handle() const { return _p ? _p->handle : NULL; }
    IC CSound_emitter* _feedback() { return _p ? _p->feedback : 0; }
    IC CObject* _g_object()
    {
        VERIFY(_p);
        return _p->g_object;
    }
    IC int _g_type()
    {
        VERIFY(_p);
        return _p->g_type;
    }
    IC esound_type _sound_type()
    {
        VERIFY(_p);
        return _p->s_type;
    }
    IC CSound_UserDataPtr _g_userdata()
    {
        VERIFY(_p);
        return _p->g_userdata;
    }

    IC void create(LPCSTR name, esound_type sound_type, int game_type);
    IC void attach_tail(LPCSTR name);

    IC void clone(const ref_sound& from, esound_type sound_type, int game_type);

    IC void destroy();

    IC void play(CObject* O, u32 flags = 0, float delay = 0.f);
    IC void play_at_pos(CObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f);
    IC void play_no_feedback(CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0);

    IC void stop();
    IC void stop_deffered();

    IC void set_position(const Fvector& pos);
    IC void set_frequency(float freq);
    IC void set_range(float min, float max);
    IC void set_volume(float vol);
    IC void set_priority(float vol);
    IC void set_time(float t); //--#SM+#--

    IC const CSound_params* get_params();
    IC void set_params(CSound_params* p);
    IC float get_length_sec() const { return _p ? _p->get_length_sec() : 0.0f; };
};

/// definition (Sound Source)
class XRSOUND_API CSound_source
{
public:
    virtual float length_sec() const = 0;
    virtual u32 game_type() const = 0;
    virtual LPCSTR file_name() const = 0;
    virtual u16 channels_num() const = 0;
    virtual u32 bytes_total() const = 0;
};

/// definition (Sound Source)
class XRSOUND_API CSound_environment
{
public:
    shared_str name;

    u32 Environment; // source environment

    float Room; // room effect level at low frequencies
    float RoomHF; // room effect high-frequency level re. low frequency level
    float RoomRolloffFactor; // like DS3D flRolloffFactor but for room effect
    float DecayTime; // reverberation decay time at low frequencies
    float DecayHFRatio; // high-frequency to low-frequency decay time ratio
    float Reflections; // early reflections level relative to room effect
    float ReflectionsDelay; // initial reflection delay time
    float Reverb; // late reverberation level relative to room effect
    float ReverbDelay; // late reverberation delay time relative to initial reflection
    float EnvironmentSize; // environment size in meters
    float EnvironmentDiffusion; // environment diffusion
    float AirAbsorptionHF; // change in level per meter at 5 kHz
};

/// definition (Sound Params)
class XRSOUND_API CSound_params
{
public:
    Fvector position;
    float base_volume;
    float volume;
    float freq;
    float min_distance;
    float max_distance;
    float max_ai_distance;
};

/// definition (Sound Interface)
class XRSOUND_API CSound_emitter
{
public:
    virtual BOOL is_2D() = 0;
    virtual void switch_to_2D() = 0;
    virtual void switch_to_3D() = 0;
    virtual void set_position(const Fvector& pos) = 0;
    virtual void set_frequency(float freq) = 0;
    virtual void set_range(float min, float max) = 0;
    virtual void set_volume(float vol) = 0;
    virtual void set_priority(float vol) = 0;
    virtual void set_time(float t) = 0; //--#SM+#--
    virtual void stop(BOOL bDeffered) = 0;
    virtual const CSound_params* get_params() = 0;
    virtual u32 play_time() = 0;
};

/// definition (Sound Stream Interface)
class XRSOUND_API CSound_stream_interface
{
public:
};

/// definition (Sound Stream Interface)
class XRSOUND_API CSound_stats
{
public:
    u32 _rendered;
    u32 _simulated;
    u32 _cache_hits;
    u32 _cache_misses;
    u32 _events;
};

class XRSOUND_API CSound_stats_ext
{
public:
    struct SItem
    {
        shared_str name;
        CSound_params params;
        float volume;
        esound_type type;
        int game_type;
        CObject* game_object;
        float occ_db; // NLC
        float occ_hf;
        struct
        {
            u32 _3D : 1;
            u32 _rendered : 1;
        };
    };
    DEFINE_VECTOR(SItem, item_vec, item_vec_it);
    item_vec items;

public:
    void clear() { items.clear(); }
    void append(const SItem& itm) { items.push_back(itm); }
};

/// definition (Sound Callback)
typedef void __stdcall sound_event(ref_sound_data_ptr S, float range);

/// definition (Sound Manager Interface)
class XRSOUND_API CSound_manager_interface
{
    virtual void _initialize(int stage) = 0;
    virtual void _clear() = 0;

protected:
    friend class ref_sound_data;
    virtual void _create_data(ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type) = 0;
    virtual void _destroy_data(ref_sound_data& S) = 0;

public:
    virtual ~CSound_manager_interface() {}

    static void _create(int stage);
    static void _destroy();

    virtual void _restart() = 0;

    virtual void create(ref_sound& S, LPCSTR fName, esound_type sound_type, int game_type) = 0;
    virtual void attach_tail(ref_sound& S, LPCSTR fName) = 0;
    virtual void clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type) = 0;
    virtual void destroy(ref_sound& S) = 0;
    virtual void stop_emitters() = 0;
    virtual int pause_emitters(bool val) = 0;

    virtual void play(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f) = 0;
    virtual void play_at_pos(ref_sound& S, CObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f) = 0;
    virtual void play_no_feedback(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0) = 0;

    virtual void set_master_volume(float f = 1.f) = 0;
    virtual void set_geometry_env(IReader* I) = 0;
    virtual void set_geometry_som(IReader* I) = 0;
    virtual void set_geometry_occ(CDB::MODEL* M) = 0;
    virtual void set_occlusion_config(const SSndOccConfig& cfg) = 0; // NLC
    virtual void nlc_report() = 0; // NLC: snd_audio_report
    virtual float nlc_listener_enclosure() const = 0; // NLC: 0 open air .. 1 enclosed (room probe)
    virtual float nlc_rain_indoor() const = 0; // NLC: rain volume floor when fully enclosed (snd_occlusion.ltx)
    // NLC listener filter (gas mask etc., script-driven): gain and kept high frequencies for every sound whose name matches
    // none of the exempt parts (comma list), faded over fade_sec; 1, 1 = off. Reset on level unload.
    virtual void nlc_set_listener_filter(float gain, float hf, float fade_sec) = 0;
    virtual void nlc_set_listener_filter_exempt(LPCSTR parts) = 0;
    virtual void set_handler(sound_event* E) = 0;

    virtual void update(const Fvector& P, const Fvector& D, const Fvector& N) = 0;
    virtual void statistic(CSound_stats* s0, CSound_stats_ext* s1) = 0;

    virtual float get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion = 0.2f) = 0;

    virtual void object_relcase(CObject* obj) = 0;
    virtual const Fvector& listener_position() = 0;

    virtual CSound_environment* DbgCurrentEnv() = 0;
    virtual void DbgCurrentEnvPaused(bool v) = 0;
    virtual void DbgCurrentEnvSave() = 0;
};

class CSound_manager_interface;
extern XRSOUND_API CSound_manager_interface* Sound;

/// ********* Sound ********* (utils, accessors, helpers)
IC ref_sound_data::ref_sound_data()
{
    handle = 0;
    feedback = 0;
    g_type = 0;
    g_object = 0;
    s_type = st_Effect;
    dwBytesTotal = 0;
    fTimeTotal = 0.0f;
}
IC ref_sound_data::ref_sound_data(LPCSTR fName, esound_type sound_type, int game_type) { ::Sound->_create_data(*this, fName, sound_type, game_type); }
IC ref_sound_data::~ref_sound_data() { ::Sound->_destroy_data(*this); }

IC void ref_sound::create(LPCSTR name, esound_type sound_type, int game_type)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->create(*this, name, sound_type, game_type);
}
IC void ref_sound::attach_tail(LPCSTR name)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->attach_tail(*this, name);
}

IC void ref_sound::clone(const ref_sound& from, esound_type sound_type, int game_type)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->clone(*this, from, sound_type, game_type);
}
IC void ref_sound::destroy()
{
    VERIFY(!::Sound->i_locked());
    ::Sound->destroy(*this);
}
IC void ref_sound::play(CObject* O, u32 flags, float d)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->play(*this, O, flags, d);
}
IC void ref_sound::play_at_pos(CObject* O, const Fvector& pos, u32 flags, float d)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->play_at_pos(*this, O, pos, flags, d);
}
IC void ref_sound::play_no_feedback(CObject* O, u32 flags, float d, Fvector* pos, float* vol, float* freq, Fvector2* range)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->play_no_feedback(*this, O, flags, d, pos, vol, freq, range);
}
IC void ref_sound::set_position(const Fvector& pos)
{
    VERIFY(!::Sound->i_locked());
    VERIFY(_feedback());
    _feedback()->set_position(pos);
}
IC void ref_sound::set_frequency(float freq)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->set_frequency(freq);
}
IC void ref_sound::set_range(float min, float max)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->set_range(min, max);
}
IC void ref_sound::set_volume(float vol)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->set_volume(vol);
}
IC void ref_sound::set_priority(float p)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->set_priority(p);
}
IC void ref_sound::set_time(float t)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->set_time(t);
}
IC void ref_sound::stop()
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->stop(FALSE);
}
IC void ref_sound::stop_deffered()
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->stop(TRUE);
}
IC const CSound_params* ref_sound::get_params()
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        return _feedback()->get_params();
    else
        return NULL;
}
IC void ref_sound::set_params(CSound_params* p)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
    {
        _feedback()->set_position(p->position);
        _feedback()->set_frequency(p->freq);
        _feedback()->set_range(p->min_distance, p->max_distance);
        _feedback()->set_volume(p->volume);
    }
}
