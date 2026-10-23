#include "stdafx.h"

#include "../xr_3da/xrLevel.h"
#include "soundrender_core.h"
#include "soundrender_source.h"
#include "soundrender_emitter.h"

#pragma warning(push)
#pragma warning(disable : 4995)
#include <eax.h>
#pragma warning(pop)

BOOL bSenvironmentXrExport{};
int psSoundTargets = 256; // 512; //--#SM+#-- //32;
Flags32 psSoundFlags = {/*ss_Hardware*/};
float psSoundOcclusionScale = 0.5f;
float psSoundLinearFadeFactor = 0.4f; //--#SM+#--
float psSoundCull = 0.01f;
float psSoundRolloff = 0.75f;
u32 psSoundModel = 0;
float psSoundVEffects = 1.0f;
float psSoundVFactor = 1.0f;
float psSoundTimeFactor = 1.0f; //--#SM+#--
float psSoundVMusic = 1.f;
int psSoundCacheSizeMB = 32;

BOOL snd_enable_float_pcm{TRUE};

// NLC audio (docs/AUDIO_DESIGN.md)
BOOL psSoundOccMode = TRUE;
float psSoundOccStrength = 1.f;
int psSoundOccRays = 160;
string64 psSoundOccLog{};
int psSoundReverbMode = 1;
float psSoundReverbZoneWeight = 0.5f;
float psSoundReverbGain = 1.f;
float psSoundOutdoorEcho = 1.f;
BOOL psSoundReverbLog{};
float psSoundAirAbsorption = 3.f;
BOOL psSoundDelay{TRUE};
float psSoundDelayMinDist = 35.f;
float psSoundSpeed = 343.f;
float psSoundAmbIndoorGain = 0.3f;
u32 psSoundHrtf = 2;
int psSoundDistModel = 0;
float psSoundBulletWhineDist = 4.5f;
BOOL psSoundHrtf2D{};
int psSoundHrtfSet = 0;

CSoundRender_Core* SoundRender = 0;
CSound_manager_interface* Sound = 0;

//////////////////////////////////////////////////
#include <efx.h>
#include "SoundRender_EFX.h"
#define LOAD_PROC(x, type) ((x) = (type)alGetProcAddress(#x))
static LPALEFFECTF alEffectf;
static LPALEFFECTI alEffecti;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALISEFFECT alIsEffect;
static LPALGENEFFECTS alGenEffects;
static LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
static LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
static LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
//////////////////////////////////////////////////

CSoundRender_Core::CSoundRender_Core()
{
    bPresent = FALSE;
    bEAX = FALSE;
    bDeferredEAX = FALSE;
    geom_MODEL = NULL;
    geom_ENV = NULL;
    geom_SOM = NULL;
    s_environment = NULL;
    Handler = NULL;
    s_targets_pu = 0;
    s_emitters_u = 0;
    e_current.set_identity();
    e_target = get_environment_def();
    bListenerMoved = FALSE;
    bReady = FALSE;
    bLocked = FALSE;
    fTimer_Value = Timer.GetElapsed_sec();
    fTimer_Delta = 0.0f;
    m_iPauseCounter = 1;

    efx_reverb = EFX_REVERB_PRESET_GENERIC;
    bEFX = false;
    effect = 0;
    slot = 0;
}

void CSoundRender_Core::release_efx_objects()
{
    if (alIsEffect(effect))
        alDeleteEffects(1, &effect);

    if (alIsAuxiliaryEffectSlot(slot))
        alDeleteAuxiliaryEffectSlots(1, &slot);

    if (effect_echo && alIsEffect(effect_echo))
        alDeleteEffects(1, &effect_echo);

    if (slot_echo && alIsAuxiliaryEffectSlot(slot_echo))
        alDeleteAuxiliaryEffectSlots(1, &slot_echo);

    // NLC: no stale ids after a release (snd_audio_report, efx_attach_slots)
    effect_echo = 0;
    slot_echo = 0;
    efx_slots_attached = false;
    efx_last_valid = false;
}

CSoundRender_Core::~CSoundRender_Core()
{
    // if (bEFX)
    {
        //release_efx_objects();
    }

    xr_delete(geom_ENV);
    xr_delete(geom_SOM);
}

void CSoundRender_Core::_initialize(int stage)
{
    Timer.Start();

    // load environment
    env_load();

    // Cache
    cache_bytes_per_line = (sdef_target_block / 8) * 352800 / 1000;
    cache.initialize(psSoundCacheSizeMB * 1024, cache_bytes_per_line);
}

extern xr_vector<u8> g_target_temp_data;
void CSoundRender_Core::_clear()
{
    cache.destroy();
    env_unload();

    // remove sources
    for (u32 sit = 0; sit < s_sources.size(); sit++)
        xr_delete(s_sources[sit]);
    s_sources.clear();

    // remove emmiters
    for (u32 eit = 0; eit < s_emitters.size(); eit++)
        xr_delete(s_emitters[eit]);
    s_emitters.clear();

    g_target_temp_data.clear();
}

void CSoundRender_Core::stop_emitters()
{
    for (u32 eit = 0; eit < s_emitters.size(); eit++)
        s_emitters[eit]->stop(FALSE);
}

int CSoundRender_Core::pause_emitters(bool val)
{
    m_iPauseCounter += val ? +1 : -1;
    VERIFY(m_iPauseCounter >= 0);

    for (u32 it = 0; it < s_emitters.size(); it++)
        ((CSoundRender_Emitter*)s_emitters[it])->pause(val, val ? m_iPauseCounter : m_iPauseCounter + 1);

    return m_iPauseCounter;
}

#define SNDENV_FILENAME_XR "senvironment.xr"
#define SNDENV_FILENAME_LTX "senvironment.ltx"

void CSoundRender_Core::env_load()
{
    // Load environment
    string_path fn;
    if (FS.exist(fn, "$game_data$", SNDENV_FILENAME_LTX))
    {
        Msg("Loading of [%s]", SNDENV_FILENAME_LTX);

        if (!s_environment)
            s_environment = xr_new<SoundEnvironment_LIB>();

        CInifile inifile{fn, 1, 1, 0};

        s_environment->LoadIni(&inifile);
    }
    else if (FS.exist(fn, "$game_data$", SNDENV_FILENAME_XR))
    {
        Msg("Loading of [%s]", SNDENV_FILENAME_XR);

        if (!s_environment)
            s_environment = xr_new<SoundEnvironment_LIB>();

        s_environment->Load(fn);
    }

    if (s_environment)
    {
        for (u32 chunk = 0; chunk < s_environment->Library().size(); chunk++)
        {
            shared_str name = s_environment->Library()[chunk]->name;

            Msg("~ env id=[%d] name=[%s]", chunk, name.c_str());
        }
    }

    // Load geometry

    // Assosiate geometry
}

void CSoundRender_Core::env_unload()
{
    // Unload
    if (s_environment)
    {
        if (bSenvironmentXrExport)
        {
            env_save_all();
        }

        s_environment->Unload();
    }

    xr_delete(s_environment);

    // Unload geometry
}

void CSoundRender_Core::env_save_all() const
{
    string_path fn;
    FS.update_path(fn, "$game_data$", SNDENV_FILENAME_LTX);

    CInifile inifile{fn, 0, 1, 1};

    s_environment->SaveIni(&inifile);
}

void CSoundRender_Core::_restart()
{
    cache.destroy();
    cache.initialize(psSoundCacheSizeMB * 1024, cache_bytes_per_line);

    env_apply();
}

void CSoundRender_Core::set_handler(sound_event* E) { Handler = E; }

void CSoundRender_Core::set_geometry_occ(CDB::MODEL* M)
{
    // NLC: under the update lock - the sound task ray-casts geom_MODEL (level unload freed it mid-query)
    std::scoped_lock m{m_bLocked};
    geom_MODEL = M;
    nlc_cache_clear();
    if (!M)
    {
        // level unload: no listener filter (gas mask) left over for the main menu
        std::scoped_lock l{nlc_lf_lock};
        nlc_lf_gain = nlc_lf_hf = nlc_lf_gain_t = nlc_lf_hf_t = 1.f;
    }
}

// NLC listener filter (docs/AUDIO_DESIGN.md 16): set from scripts (sivol_mask_audio), read by the sound task
void CSoundRender_Core::nlc_set_listener_filter(float gain, float hf, float fade_sec)
{
    std::scoped_lock l{nlc_lf_lock};
    nlc_lf_gain_t = _valid(gain) ? clampr(gain, 0.f, 1.f) : 1.f;
    nlc_lf_hf_t = _valid(hf) ? clampr(hf, 0.01f, 1.f) : 1.f;
    nlc_lf_rate = (_valid(fade_sec) && fade_sec > 0.01f) ? 1.f / fade_sec : 100.f;
}

void CSoundRender_Core::nlc_set_listener_filter_exempt(LPCSTR parts)
{
    std::scoped_lock l{nlc_lf_lock};
    nlc_lf_exempt_parts.clear();
    if (!parts || !*parts)
        return;
    string256 item;
    for (int i = 0, n = _GetItemCount(parts); i < n; ++i)
    {
        _GetItem(parts, i, item);
        _Trim(item);
        _strlwr(item);
        if (item[0])
            nlc_lf_exempt_parts.emplace_back(item);
    }
}

bool CSoundRender_Core::nlc_lf_exempt(LPCSTR name) const
{
    std::scoped_lock l{const_cast<std::mutex&>(nlc_lf_lock)};
    for (const xr_string& s : nlc_lf_exempt_parts)
        if (strstr(name, s.c_str()))
            return true;
    return false;
}

void CSoundRender_Core::set_geometry_som(IReader* I)
{
    std::scoped_lock m{m_bLocked}; // NLC: see set_geometry_occ
    xr_delete(geom_SOM);
    if (0 == I)
        return;

    // check version
    R_ASSERT(I->find_chunk(0));

#ifdef DEBUG
    u32 version =
#endif
        I->r_u32();
    VERIFY2(version == 0, "Invalid SOM version");
    // load geometry
    IReader* geom = I->open_chunk(1);
    VERIFY2(geom, "Corrupted SOM file");
    // Load tris and merge them
    struct SOM_poly
    {
        Fvector3 v1;
        Fvector3 v2;
        Fvector3 v3;
        u32 b2sided;
        float occ;
    };
    // Create AABB-tree
    CDB::Collector CL;
    while (!geom->eof())
    {
        SOM_poly P;
        geom->r(&P, sizeof(P));
        CL.add_face_packed_D(P.v1, P.v2, P.v3, *(size_t*)&P.occ, 0.01f);
        if (P.b2sided)
            CL.add_face_packed_D(P.v3, P.v2, P.v1, *(size_t*)&P.occ, 0.01f);
    }
    geom_SOM = xr_new<CDB::MODEL>();
    geom_SOM->build(CL.getV(), CL.getVS(), CL.getT(), CL.getTS());

    geom->close();
}

void CSoundRender_Core::set_geometry_env(IReader* I)
{
    std::scoped_lock m{m_bLocked}; // NLC: see set_geometry_occ
    xr_delete(geom_ENV);
    s_environment_ids.clear();

    if (0 == I)
        return;
    if (0 == s_environment)
        return;

    // Assosiate names
    IReader* names = I->open_chunk(0);
    while (!names->eof())
    {
        string256 n;
        names->r_stringZ(n, sizeof(n));
        int id = s_environment->GetID(n);
        R_ASSERT(id >= 0);
        s_environment_ids.push_back(u16(id));
        Msg("~ set_geometry_env id=%d name[%s]=environment id[%d]", s_environment_ids.size() - 1, n, id);
    }
    names->close();

    // Load geometry
    IReader* geom_ch = I->open_chunk(1);

    u8* _data = (u8*)xr_malloc(geom_ch->length());

    Memory.mem_copy(_data, geom_ch->pointer(), geom_ch->length());

    IReader* geom = xr_new<IReader>(_data, geom_ch->length(), 0);

    hdrCFORM H;
    geom->r(&H, sizeof(hdrCFORM));
    R_ASSERT(H.version == CFORM_CURRENT_VERSION);
    Fvector* verts = (Fvector*)geom->pointer();
    CDB::TRI* tris = (CDB::TRI*)(verts + H.vertcount);

    geom_ENV = xr_new<CDB::MODEL>();
    geom_ENV->build(verts, H.vertcount, tris, H.facecount);
    geom_ch->close();
    geom->close();
    xr_free(_data);
}

void CSoundRender_Core::create(ref_sound& S, LPCSTR fName, esound_type sound_type, int game_type)
{
    if (!bPresent)
        return;

    S._p = xr_new<ref_sound_data>(fName, sound_type, game_type);
}

void CSoundRender_Core::attach_tail(ref_sound& S, const char* fName)
{
    if (!bPresent)
        return;
    string_path fn;
    xr_strcpy(fn, fName);
    if (strext(fn))
        *strext(fn) = 0;
    if (S._p->fn_attached[0].size() && S._p->fn_attached[1].size())
    {
#ifdef DEBUG
        Msg("! 2 file already in queue [%s][%s]", S._p->fn_attached[0].c_str(), S._p->fn_attached[1].c_str());
#endif // #ifdef DEBUG
        return;
    }

    u32 idx = S._p->fn_attached[0].size() ? 1 : 0;

    S._p->fn_attached[idx] = fn;

    CSoundRender_Source* s = SoundRender->i_create_source(fn);
    S._p->dwBytesTotal += s->bytes_total();
    S._p->fTimeTotal += s->length_sec();
    if (S._feedback())
        ((CSoundRender_Emitter*)S._feedback())->fTimeToStop += s->length_sec();

    SoundRender->i_destroy_source(s);
}

void CSoundRender_Core::clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type)
{
    if (!bPresent)
        return;

    S._p = xr_new<ref_sound_data>();
    S._p->handle = from._p->handle;
    S._p->dwBytesTotal = from._p->dwBytesTotal;
    S._p->fTimeTotal = from._p->fTimeTotal;
    S._p->fn_attached[0] = from._p->fn_attached[0];
    S._p->fn_attached[1] = from._p->fn_attached[1];
    S._p->g_type = (game_type == sg_SourceType) ? S._p->handle->game_type() : game_type;
    S._p->s_type = sound_type;
}

void CSoundRender_Core::play(ref_sound& S, CObject* O, u32 flags, float delay)
{
    if (!bPresent || 0 == S._handle())
        return;
    S._p->g_object = O;
    if (S._feedback())
        ((CSoundRender_Emitter*)S._feedback())->rewind();
    else
        i_play(&S, flags & sm_Looped, delay, flags);

    if (flags & sm_2D || S._handle()->channels_num() == 2)
        S._feedback()->switch_to_2D();
}

void CSoundRender_Core::play_no_feedback(ref_sound& S, CObject* O, u32 flags, float delay, Fvector* pos, float* vol, float* freq, Fvector2* range)
{
    if (!bPresent || 0 == S._handle())
        return;
    ref_sound_data_ptr orig = S._p;
    S._p = xr_new<ref_sound_data>();
    S._p->handle = orig->handle;
    S._p->g_type = orig->g_type;
    S._p->g_object = O;
    S._p->dwBytesTotal = orig->dwBytesTotal;
    S._p->fTimeTotal = orig->fTimeTotal;
    S._p->fn_attached[0] = orig->fn_attached[0];
    S._p->fn_attached[1] = orig->fn_attached[1];

    i_play(&S, flags & sm_Looped, delay, flags);

    if (flags & sm_2D || S._handle()->channels_num() == 2)
        S._feedback()->switch_to_2D();

    if (pos)
        S._feedback()->set_position(*pos);
    if (freq)
        S._feedback()->set_frequency(*freq);
    if (range)
        S._feedback()->set_range((*range)[0], (*range)[1]);
    if (vol)
        S._feedback()->set_volume(*vol);
    S._p = orig;
}

void CSoundRender_Core::play_at_pos(ref_sound& S, CObject* O, const Fvector& pos, u32 flags, float delay)
{
    if (!bPresent || 0 == S._handle())
        return;
    S._p->g_object = O;
    if (S._feedback())
        ((CSoundRender_Emitter*)S._feedback())->rewind();
    else
        i_play(&S, flags & sm_Looped, delay, flags);

    S._feedback()->set_position(pos);

    if (flags & sm_2D || S._handle()->channels_num() == 2)
        S._feedback()->switch_to_2D();
}
void CSoundRender_Core::destroy(ref_sound& S)
{
    if (S._feedback())
    {
        CSoundRender_Emitter* E = (CSoundRender_Emitter*)S._feedback();
        E->stop(FALSE);
    }
    S._p = 0;
}

void CSoundRender_Core::_create_data(ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type)
{
    string_path fn;
    xr_strcpy(fn, fName);
    if (strext(fn))
        *strext(fn) = 0;
    S.handle = (CSound_source*)SoundRender->i_create_source(fn);
    S.g_type = (game_type == sg_SourceType) ? S.handle->game_type() : game_type;
    S.s_type = sound_type;
    S.feedback = 0;
    S.g_object = 0;
    S.g_userdata = 0;
    S.dwBytesTotal = S.handle->bytes_total();
    S.fTimeTotal = S.handle->length_sec();
}
void CSoundRender_Core::_destroy_data(ref_sound_data& S)
{
    if (S.feedback)
    {
        CSoundRender_Emitter* E = (CSoundRender_Emitter*)S.feedback;
        E->stop(FALSE);
    }
    R_ASSERT(0 == S.feedback);
    SoundRender->i_destroy_source((CSoundRender_Source*)S.handle);

    S.handle = NULL;
}

CSoundRender_Environment* CSoundRender_Core::get_environment_def()
{
    static CSoundRender_Environment identity;
    identity.set_identity();
    return &identity;
}

CSoundRender_Environment* CSoundRender_Core::get_environment(const Fvector& P)
{
    if (geom_ENV)
    {
        Fvector dir = {0, -1, 0};

        // хитрый способ для проверки звуковых зон в 2х направлениях от камеры. но что то он хуже работает. часто не та зона выбираеться. пока убрал

        // CDB::COLLIDER geom_DB1;
        // geom_DB1.ray_query(CDB::OPT_ONLYNEAREST, geom_ENV, P, dir, 1000.f);

        // CDB::COLLIDER geom_DB2;
        // geom_DB2.ray_query(CDB::OPT_ONLYNEAREST, geom_ENV, P, Fvector(dir).invert(), 1000.f);

        geom_DB.ray_query(CDB::OPT_ONLYNEAREST, geom_ENV, P, dir, 1000.f);

        // if (geom_DB1.r_count() && geom_DB2.r_count())
        if (geom_DB.r_count())
        {
            // CDB::RESULT* r = geom_DB1.r_begin();
            // CDB::RESULT* r2 = geom_DB2.r_begin();

            // if (r2->range < r->range)
            //     r = r2;

            CDB::RESULT* r = geom_DB.r_begin();

            CDB::TRI* T = geom_ENV->get_tris() + r->id;
            const Fvector* V = geom_ENV->get_verts();

            Fvector tri_norm;
            tri_norm.mknormal(V[T->verts[0]], V[T->verts[1]], V[T->verts[2]]);
            float dot = dir.dotproduct(tri_norm);

            if (dot <= 0)
            {
                u16 id_front = (u16)((((u32)T->dummy) & 0x0000ffff) >> 0); //	front face

                return s_environment->Get(s_environment_ids[id_front]);
            }
            else
            {
                u16 id_back = (u16)((((u32)T->dummy) & 0xffff0000) >> 16); //	back face

                return s_environment->Get(s_environment_ids[id_back]);
            }
        }
    }

    return get_environment_def();
}

void CSoundRender_Core::env_apply()
{
    /*
        // Force all sounds to change their environment
        // (set their positions to signal changes in environment)
        for (u32 it=0; it<s_emitters.size(); it++)
        {
            CSoundRender_Emitter*	pEmitter	= s_emitters[it];
            const CSound_params*	pParams		= pEmitter->get_params	();
            pEmitter->set_position	(pParams->position);
        }
    */
    bListenerMoved = TRUE;
}

void CSoundRender_Core::update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) {}

//////////////////////////////////////////////////
void CSoundRender_Core::InitAlEFXAPI()
{
    LOAD_PROC(alDeleteAuxiliaryEffectSlots, LPALDELETEAUXILIARYEFFECTSLOTS);
    LOAD_PROC(alGenEffects, LPALGENEFFECTS);
    LOAD_PROC(alDeleteEffects, LPALDELETEEFFECTS);
    LOAD_PROC(alIsEffect, LPALISEFFECT);
    LOAD_PROC(alEffecti, LPALEFFECTI);
    LOAD_PROC(alAuxiliaryEffectSloti, LPALAUXILIARYEFFECTSLOTI);
    LOAD_PROC(alGenAuxiliaryEffectSlots, LPALGENAUXILIARYEFFECTSLOTS);
    LOAD_PROC(alIsAuxiliaryEffectSlot, LPALISAUXILIARYEFFECTSLOT);
    LOAD_PROC(alEffectf, LPALEFFECTF);

    sndefx::GenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
    sndefx::DeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
    sndefx::IsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
    sndefx::Filteri = (LPALFILTERI)alGetProcAddress("alFilteri");
    sndefx::Filterf = (LPALFILTERF)alGetProcAddress("alFilterf");
    Msg("[OpenAL] EFX filters: %s", sndefx::filters_ok() ? "present" : "absent");
}

bool CSoundRender_Core::EFXTestSupport()
{
    efx_slots_attached = false;
    effect_echo = 0;
    slot_echo = 0;

    alGetError();
    alGenEffects(1, &effect);

    // NLC: EAX reverb (echo, LF, modulation) when the device has it, else standard reverb
    alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
    efx_eax = alGetError() == AL_NO_ERROR;
    if (!efx_eax)
        alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

    efx_apply(effect, efx_reverb);

    /* Check if an error occured, and clean up if so. */
    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        Msg("!![%s] OpenAL error: %s", __FUNCTION__, alGetString(err));
        if (alIsEffect(effect))
            alDeleteEffects(1, &effect);
        return false;
    }

    alGenAuxiliaryEffectSlots(1, &slot);
    err = alGetError();
    ASSERT_FMT_DBG(err == AL_NO_ERROR, "!![%s] OpenAL EFX error: [%s]", __FUNCTION__, alGetString(err));

    // NLC: outdoor echo slot on the second send (loud sounds only, AUDIO_DESIGN 3.2)
    if (efx_sends >= 2 && efx_eax)
    {
        alGenEffects(1, &effect_echo);
        alEffecti(effect_echo, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
        EFXEAXREVERBPROPERTIES echo = EFX_REVERB_PRESET_MOUNTAINS;
        echo.flEchoDepth = 0.5f; // less flutter than the preset's 1.0
        efx_apply(effect_echo, echo);
        alGenAuxiliaryEffectSlots(1, &slot_echo);
        err = alGetError();
        if (err != AL_NO_ERROR)
        {
            Msg("! [%s] OpenAL echo slot error: %s, echo disabled", __FUNCTION__, alGetString(err));
            if (effect_echo && alIsEffect(effect_echo))
                alDeleteEffects(1, &effect_echo);
            if (slot_echo && alIsAuxiliaryEffectSlot(slot_echo))
                alDeleteAuxiliaryEffectSlots(1, &slot_echo);
            effect_echo = 0;
            slot_echo = 0;
        }
    }

    Msg("[OpenAL] EFX reverb: %s, sends: %d, echo slot: %s", efx_eax ? "EAX reverb" : "standard reverb", efx_sends, slot_echo ? "yes" : "no");

    return true;
}

inline static float mB_to_gain(float mb) { return powf(10.0f, mb / 2000.0f); }

// NLC: legacy senvironment preset (EAX units) to EFX properties; same mapping as the old i_efx_listener_set
void CSoundRender_Core::env_to_efx(const CSoundRender_Environment& E, EFXEAXREVERBPROPERTIES& P) const
{
    P = EFX_REVERB_PRESET_GENERIC;

    // http://openal.org/pipermail/openal/2014-March/000083.html
    float density = powf(E.EnvironmentSize, 3.0f) / 16.0f;
    if (density > 1.0f)
        density = 1.0f;

    P.flDensity = density;
    P.flDiffusion = E.EnvironmentDiffusion;
    P.flGain = mB_to_gain(E.Room);
    P.flGainHF = mB_to_gain(E.RoomHF);
    P.flDecayTime = E.DecayTime;
    P.flDecayHFRatio = E.DecayHFRatio;
    P.flReflectionsGain = mB_to_gain(E.Reflections);
    P.flReflectionsDelay = E.ReflectionsDelay;
    P.flLateReverbDelay = E.ReverbDelay;
    P.flLateReverbGain = mB_to_gain(E.Reverb);
    P.flAirAbsorptionGainHF = mB_to_gain(E.AirAbsorptionHF);
    P.flRoomRolloffFactor = E.RoomRolloffFactor;
}

// NLC: room reverb from the listener probe (enclosure, mean free distance, absorption), AUDIO_DESIGN 3.1
void CSoundRender_Core::nlc_room_props(EFXEAXREVERBPROPERTIES& P) const
{
    const SSndOccConfig& C = nlc_cfg;
    const float L = _max(nlc_mean_dist, 1.f);
    const float a = _max(nlc_absorb, 0.03f);
    const float E = nlc_enc;

    P = EFX_REVERB_PRESET_GENERIC;
    P.flDensity = clampr(L / 20.f, 0.25f, 1.f);
    P.flDiffusion = 1.f;
    P.flGain = C.room_gain * E;
    P.flGainHF = clampr(0.9f - a * 1.2f, 0.35f, 0.95f);
    P.flDecayTime = clampr(C.decay_k * _max(nlc_room_size, 1.f) / a, C.decay_min, C.decay_max);
    // air takes the highs of long tails too: big rooms decay darker
    P.flDecayHFRatio = clampr(0.95f - a * 1.5f - 0.01f * L, 0.3f, 1.f);
    P.flReflectionsGain = clampr(1.5f / L, 0.08f, 1.2f) * E;
    // first reflections come from the nearest walls, not the mean distance (a hall gave a 0.1 s slapback)
    P.flReflectionsDelay = clampr(2.f * _max(nlc_near_dist, 0.5f) / 343.f, 0.003f, 0.05f);
    P.flLateReverbGain = (0.5f + 0.75f * E) * E;
    P.flLateReverbDelay = clampr(L / 343.f, 0.01f, 0.08f);
}

// NaN-safe: a NaN parameter makes OpenAL reject the whole effect update
static float efx_clamp(float v, float lo, float hi) { return !(v >= lo) ? lo : (v > hi ? hi : v); }

void CSoundRender_Core::efx_apply(ALuint fx, const EFXEAXREVERBPROPERTIES& P) const
{
    if (efx_eax)
    {
        alEffectf(fx, AL_EAXREVERB_DENSITY, efx_clamp(P.flDensity, AL_EAXREVERB_MIN_DENSITY, AL_EAXREVERB_MAX_DENSITY));
        alEffectf(fx, AL_EAXREVERB_DIFFUSION, efx_clamp(P.flDiffusion, AL_EAXREVERB_MIN_DIFFUSION, AL_EAXREVERB_MAX_DIFFUSION));
        alEffectf(fx, AL_EAXREVERB_GAIN, efx_clamp(P.flGain, AL_EAXREVERB_MIN_GAIN, AL_EAXREVERB_MAX_GAIN));
        alEffectf(fx, AL_EAXREVERB_GAINHF, efx_clamp(P.flGainHF, AL_EAXREVERB_MIN_GAINHF, AL_EAXREVERB_MAX_GAINHF));
        alEffectf(fx, AL_EAXREVERB_GAINLF, efx_clamp(P.flGainLF, AL_EAXREVERB_MIN_GAINLF, AL_EAXREVERB_MAX_GAINLF));
        alEffectf(fx, AL_EAXREVERB_DECAY_TIME, efx_clamp(P.flDecayTime, AL_EAXREVERB_MIN_DECAY_TIME, AL_EAXREVERB_MAX_DECAY_TIME));
        alEffectf(fx, AL_EAXREVERB_DECAY_HFRATIO, efx_clamp(P.flDecayHFRatio, AL_EAXREVERB_MIN_DECAY_HFRATIO, AL_EAXREVERB_MAX_DECAY_HFRATIO));
        alEffectf(fx, AL_EAXREVERB_DECAY_LFRATIO, efx_clamp(P.flDecayLFRatio, AL_EAXREVERB_MIN_DECAY_LFRATIO, AL_EAXREVERB_MAX_DECAY_LFRATIO));
        alEffectf(fx, AL_EAXREVERB_REFLECTIONS_GAIN, efx_clamp(P.flReflectionsGain, AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN));
        alEffectf(fx, AL_EAXREVERB_REFLECTIONS_DELAY, efx_clamp(P.flReflectionsDelay, AL_EAXREVERB_MIN_REFLECTIONS_DELAY, AL_EAXREVERB_MAX_REFLECTIONS_DELAY));
        alEffectf(fx, AL_EAXREVERB_LATE_REVERB_GAIN, efx_clamp(P.flLateReverbGain, AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN));
        alEffectf(fx, AL_EAXREVERB_LATE_REVERB_DELAY, efx_clamp(P.flLateReverbDelay, AL_EAXREVERB_MIN_LATE_REVERB_DELAY, AL_EAXREVERB_MAX_LATE_REVERB_DELAY));
        alEffectf(fx, AL_EAXREVERB_ECHO_TIME, efx_clamp(P.flEchoTime, AL_EAXREVERB_MIN_ECHO_TIME, AL_EAXREVERB_MAX_ECHO_TIME));
        alEffectf(fx, AL_EAXREVERB_ECHO_DEPTH, efx_clamp(P.flEchoDepth, AL_EAXREVERB_MIN_ECHO_DEPTH, AL_EAXREVERB_MAX_ECHO_DEPTH));
        alEffectf(fx, AL_EAXREVERB_MODULATION_TIME, efx_clamp(P.flModulationTime, AL_EAXREVERB_MIN_MODULATION_TIME, AL_EAXREVERB_MAX_MODULATION_TIME));
        alEffectf(fx, AL_EAXREVERB_MODULATION_DEPTH, efx_clamp(P.flModulationDepth, AL_EAXREVERB_MIN_MODULATION_DEPTH, AL_EAXREVERB_MAX_MODULATION_DEPTH));
        alEffectf(fx, AL_EAXREVERB_AIR_ABSORPTION_GAINHF,
                  efx_clamp(P.flAirAbsorptionGainHF, AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF));
        alEffectf(fx, AL_EAXREVERB_HFREFERENCE, efx_clamp(P.flHFReference, AL_EAXREVERB_MIN_HFREFERENCE, AL_EAXREVERB_MAX_HFREFERENCE));
        alEffectf(fx, AL_EAXREVERB_LFREFERENCE, efx_clamp(P.flLFReference, AL_EAXREVERB_MIN_LFREFERENCE, AL_EAXREVERB_MAX_LFREFERENCE));
        alEffectf(fx, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, efx_clamp(P.flRoomRolloffFactor, AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR, AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR));
        alEffecti(fx, AL_EAXREVERB_DECAY_HFLIMIT, P.iDecayHFLimit ? AL_TRUE : AL_FALSE);
    }
    else
    {
        alEffectf(fx, AL_REVERB_DENSITY, efx_clamp(P.flDensity, AL_REVERB_MIN_DENSITY, AL_REVERB_MAX_DENSITY));
        alEffectf(fx, AL_REVERB_DIFFUSION, efx_clamp(P.flDiffusion, AL_REVERB_MIN_DIFFUSION, AL_REVERB_MAX_DIFFUSION));
        alEffectf(fx, AL_REVERB_GAIN, efx_clamp(P.flGain, AL_REVERB_MIN_GAIN, AL_REVERB_MAX_GAIN));
        alEffectf(fx, AL_REVERB_GAINHF, efx_clamp(P.flGainHF, AL_REVERB_MIN_GAINHF, AL_REVERB_MAX_GAINHF));
        alEffectf(fx, AL_REVERB_DECAY_TIME, efx_clamp(P.flDecayTime, AL_REVERB_MIN_DECAY_TIME, AL_REVERB_MAX_DECAY_TIME));
        alEffectf(fx, AL_REVERB_DECAY_HFRATIO, efx_clamp(P.flDecayHFRatio, AL_REVERB_MIN_DECAY_HFRATIO, AL_REVERB_MAX_DECAY_HFRATIO));
        alEffectf(fx, AL_REVERB_REFLECTIONS_GAIN, efx_clamp(P.flReflectionsGain, AL_REVERB_MIN_REFLECTIONS_GAIN, AL_REVERB_MAX_REFLECTIONS_GAIN));
        alEffectf(fx, AL_REVERB_REFLECTIONS_DELAY, efx_clamp(P.flReflectionsDelay, AL_REVERB_MIN_REFLECTIONS_DELAY, AL_REVERB_MAX_REFLECTIONS_DELAY));
        alEffectf(fx, AL_REVERB_LATE_REVERB_GAIN, efx_clamp(P.flLateReverbGain, AL_REVERB_MIN_LATE_REVERB_GAIN, AL_REVERB_MAX_LATE_REVERB_GAIN));
        alEffectf(fx, AL_REVERB_LATE_REVERB_DELAY, efx_clamp(P.flLateReverbDelay, AL_REVERB_MIN_LATE_REVERB_DELAY, AL_REVERB_MAX_LATE_REVERB_DELAY));
        alEffectf(fx, AL_REVERB_AIR_ABSORPTION_GAINHF, efx_clamp(P.flAirAbsorptionGainHF, AL_REVERB_MIN_AIR_ABSORPTION_GAINHF, AL_REVERB_MAX_AIR_ABSORPTION_GAINHF));
        alEffectf(fx, AL_REVERB_ROOM_ROLLOFF_FACTOR, efx_clamp(P.flRoomRolloffFactor, AL_REVERB_MIN_ROOM_ROLLOFF_FACTOR, AL_REVERB_MAX_ROOM_ROLLOFF_FACTOR));
        alEffecti(fx, AL_REVERB_DECAY_HFLIMIT, P.iDecayHFLimit ? AL_TRUE : AL_FALSE);
    }
}

void CSoundRender_Core::i_efx_listener_set(CSound_environment* _E)
{
    EFXEAXREVERBPROPERTIES P;
    env_to_efx(*static_cast<CSoundRender_Environment*>(_E), P);
    efx_apply(effect, P);
}

bool CSoundRender_Core::i_efx_commit_setting()
{
    alGetError();
    /* Tell the effect slot to use the loaded effect object. Note that the this
     * effectively copies the effect properties. You can modify or delete the
     * effect object afterward without affecting the effect slot.
     */
    alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        Msg("!![%s] OpenAL EFX commit error: [%s]. EFX will be disabled.", __FUNCTION__, alGetString(err));
        return false;
    }

    return true;
}

// NLC: snd_efx on/off attaches/detaches both effects (before, turning it off kept the last reverb in the slot)
void CSoundRender_Core::efx_attach_slots(bool attach)
{
    if (attach == efx_slots_attached)
        return;
    efx_slots_attached = attach;
    alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, attach ? effect : AL_EFFECT_NULL);
    if (slot_echo)
        alAuxiliaryEffectSloti(slot_echo, AL_EFFECTSLOT_EFFECT, attach ? effect_echo : AL_EFFECT_NULL);
    efx_next_commit = 0.f;
}

bool CSoundRender_Core::nlc_filters_active() const { return bEFX && sndefx::filters_ok(); }

void CSoundRender_Core::set_occlusion_config(const SSndOccConfig& cfg)
{
    std::scoped_lock m{m_bLocked};
    nlc_cfg = cfg;
    nlc_cache_clear();
    Msg("* [snd_occ] config: %u materials, strength %.2f, leak %.1f dB", cfg.mtl.size(), cfg.strength, cfg.leak_db);
}
//////////////////////////////////////////////////

void CSoundRender_Core::i_eax_listener_set(CSound_environment* _E)
{
    VERIFY(bEAX);
    CSoundRender_Environment* E = static_cast<CSoundRender_Environment*>(_E);
    EAXLISTENERPROPERTIES ep;
    ep.lRoom = iFloor(E->Room); // room effect level at low frequencies
    ep.lRoomHF = iFloor(E->RoomHF); // room effect high-frequency level re. low frequency level
    ep.flRoomRolloffFactor = E->RoomRolloffFactor; // like DS3D flRolloffFactor but for room effect
    ep.flDecayTime = E->DecayTime; // reverberation decay time at low frequencies
    ep.flDecayHFRatio = E->DecayHFRatio; // high-frequency to low-frequency decay time ratio
    ep.lReflections = iFloor(E->Reflections); // early reflections level relative to room effect
    ep.flReflectionsDelay = E->ReflectionsDelay; // initial reflection delay time
    ep.lReverb = iFloor(E->Reverb); // late reverberation level relative to room effect
    ep.flReverbDelay = E->ReverbDelay; // late reverberation delay time relative to initial reflection
    //ep.dwEnvironment = EAXLISTENER_DEFAULTENVIRONMENT; // sets all listener properties
    //ep.flEnvironmentSize = E->EnvironmentSize; // environment size in meters
    ep.flEnvironmentDiffusion = E->EnvironmentDiffusion; // environment diffusion
    ep.flAirAbsorptionHF = E->AirAbsorptionHF; // change in level per meter at 5 kHz
    ep.dwFlags = EAXLISTENER_DEFAULTFLAGS; // modifies the behavior of properties

    u32 deferred = bDeferredEAX ? DSPROPERTY_EAXLISTENER_DEFERRED : 0;

    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ROOM, &ep.lRoom, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ROOMHF, &ep.lRoomHF, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR, &ep.flRoomRolloffFactor, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_DECAYTIME, &ep.flDecayTime, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_DECAYHFRATIO, &ep.flDecayHFRatio, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REFLECTIONS, &ep.lReflections, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY, &ep.flReflectionsDelay, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REVERB, &ep.lReverb, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REVERBDELAY, &ep.flReverbDelay, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION, &ep.flEnvironmentDiffusion, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF, &ep.flAirAbsorptionHF, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_FLAGS, &ep.dwFlags, sizeof(DWORD));
}

void CSoundRender_Core::i_eax_commit_setting()
{
    // commit eax
    if (bDeferredEAX)
        i_eax_set(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_COMMITDEFERREDSETTINGS, NULL, 0);
}

void CSoundRender_Core::object_relcase(CObject* obj)
{
    if (obj)
    {
        for (u32 eit = 0; eit < s_emitters.size(); eit++)
        {
            if (s_emitters[eit])
                if (s_emitters[eit]->owner_data)
                    if (obj == s_emitters[eit]->owner_data->g_object)
                        s_emitters[eit]->owner_data->g_object = 0;
        }
    }
}

XRSOUND_API float SoundRenderGetOcculution(Fvector& P, float R, Fvector* occ) { return SoundRender->get_occlusion(P, R, occ); }
