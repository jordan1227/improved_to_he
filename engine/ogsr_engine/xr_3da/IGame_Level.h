#pragma once

#include "iinputreceiver.h"
#include "xr_object_list.h"
#include "xr_area.h"

// refs
class ENGINE_API CCameraManager;
class ENGINE_API CCursor;
class ENGINE_API CCustomHUD;
class ENGINE_API ISpatial;
namespace Feel
{
class ENGINE_API Sound;
}

//-----------------------------------------------------------------------------------------------------------
class ENGINE_API IGame_Level : public DLL_Pure, public IInputReceiver, public pureRender, public pureFrame, public IEventReceiver
{
protected:
    // Network interface
    CObject* pCurrentEntity;
    CObject* pCurrentViewEntity;

    // Static sounds
    xr_vector<ref_sound> Sounds_Random;
    u32 Sounds_Random_dwNextTime{};
    BOOL Sounds_Random_Enabled{};
    CCameraManager* m_pCameras;
    xr_string on_change_weather_callback;

    // temporary
    xr_vector<ISpatial*> snd_ER;

public:
    CObjectList Objects;
    CObjectSpace ObjectSpace;
    CCameraManager& Cameras() const { return *m_pCameras; };

    BOOL bReady;

    CInifile* pLevel;

public: // deferred sound events
    struct _esound_delegate
    {
        Feel::Sound* dest;
        ref_sound_data_ptr source;
        float power;
    };
    xr_vector<_esound_delegate> snd_Events;

public:
    // Main, global functions
    IGame_Level();
    virtual ~IGame_Level();

    virtual shared_str name() const = 0;

    virtual BOOL net_Start(LPCSTR op_server, LPCSTR op_client) = 0;
    virtual void net_Load(LPCSTR name) = 0;
    virtual void net_Save(LPCSTR name) = 0;
    virtual void net_Stop();
    virtual void net_Update() = 0;

    virtual BOOL Load(u32 dwNum);
    virtual BOOL Load_GameSpecific_Before() { return TRUE; }; // before object loading
    virtual BOOL Load_GameSpecific_After() { return TRUE; }; // after object loading
    virtual void Load_GameSpecific_CFORM(CDB::TRI* T, const size_t count) = 0;

    virtual void OnFrame(void);
    virtual void OnRender(void);

    // Main interface
    CObject* CurrentEntity(void) const { return pCurrentEntity; }
    CObject* CurrentViewEntity(void) const { return pCurrentViewEntity; }
    void SetEntity(CObject* O);
    void SetViewEntity(CObject* O);

    void SoundEvent_Register(ref_sound_data_ptr S, float range);
    void SoundEvent_Dispatch();
    void SoundEvent_OnDestDestroy(Feel::Sound*);

    void LL_CheckTextures();

    virtual void OnChangeCurrentWeather(const char* sect) {}

    virtual void OnDestroyObject(u16 id) = 0;

    virtual void GetGameTimeForShaders(u32& hours, u32& minutes, u32& seconds, u32& milliseconds) = 0;

    virtual bool is_removing_objects() const = 0;

    virtual void script_gc() const = 0;
};

//-----------------------------------------------------------------------------------------------------------
extern ENGINE_API IGame_Level* g_pGameLevel;

// NLC silence anomaly (docs/ANOMALY_DESIGN.md 16): spheres registered by the silence zones (CCustomZone, key nlc_silence).
// A sound event made inside one, or heard by a listener inside one, reaches the AI (Feel::Sound) weaker or not at all.
// rim / core: share of the sound power removed at the edge and past ramp_k of the radius.
ENGINE_API void nlc_silence_set(u16 id, const Fvector& center, float radius, float rim, float core, float ramp_k);
ENGINE_API void nlc_silence_remove(u16 id);
ENGINE_API void nlc_silence_clear();
ENGINE_API float nlc_silence_pair_k(const Fvector& source, const Fvector& listener); // 1 = no silence zone involved
ENGINE_API u32 nlc_silence_zones(); // registered now
ENGINE_API u32 nlc_silence_muted(); // AI sound deliveries weakened by a silence zone since the level started
