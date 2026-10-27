#pragma once

#include "soundrender_Target.h"
#include "soundrender_CoreA.h"

class CSoundRender_TargetA : public CSoundRender_Target
{
    typedef CSoundRender_Target inherited;

    // OpenAL
    ALuint pSource;
    ALuint pBuffers[sdef_target_count]{};

    float cache_gain{};
    float cache_pitch{};

    ALuint buf_block{};

    // NLC: low-pass filters for the direct path, the room send and the echo send (docs/AUDIO_DESIGN.md 2.4, 3.2)
    ALuint nlc_filter[4]{}; // direct, room send, echo send, psy send
    bool nlc_filters_ok{};
    bool nlc_filters_tried{};
    float nlc_cache[7]{-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f}; // direct hf, room gain, room hf, echo gain, slot state, direct gain, psy send
    void nlc_filters_create();
    void nlc_filters_destroy();
    void nlc_fill_efx(CSoundRender_Core* core);
    bool nlc_direct{}; // 2D voice sent past HRTF as direct stereo (mono duplicated to both channels)

private:
    void fill_block(ALuint BufferID);

public:
    CSoundRender_TargetA();
    virtual ~CSoundRender_TargetA();

    virtual BOOL _initialize();
    virtual void _destroy();
    virtual void _restart();

    virtual void start(CSoundRender_Emitter* E);
    virtual void render();
    virtual void rewind();
    virtual void stop();
    virtual void update();

    void source_changed();

    virtual void fill_parameters(CSoundRender_Core* core);

    virtual void alAuxInit(ALuint slot) override;
};
