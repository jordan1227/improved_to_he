#include "stdafx.h"

#include "soundrender_TargetA.h"
#include "soundrender_emitter.h"
#include "soundrender_source.h"

#include <efx.h>
#include "SoundRender_EFX.h"

xr_vector<u8> g_target_temp_data;
static xr_vector<u8> g_target_temp_stereo; // NLC: mono 2D blocks duplicated to stereo for direct channels

CSoundRender_TargetA::CSoundRender_TargetA() : CSoundRender_Target()
{
    cache_gain = 0.f;
    cache_pitch = 1.f;
    pSource = 0;
}

CSoundRender_TargetA::~CSoundRender_TargetA() {}

BOOL CSoundRender_TargetA::_initialize()
{
    inherited::_initialize();
    // initialize buffer
    A_CHK(alGenBuffers(sdef_target_count, pBuffers));
    alGenSources(1, &pSource);
    ALenum error = alGetError();
    if (AL_NO_ERROR == error)
    {
        A_CHK(alSourcei(pSource, AL_LOOPING, AL_FALSE));
        A_CHK(alSourcef(pSource, AL_MIN_GAIN, 0.f));
        A_CHK(alSourcef(pSource, AL_MAX_GAIN, 1.f));
        A_CHK(alSourcef(pSource, AL_GAIN, cache_gain));
        A_CHK(alSourcef(pSource, AL_PITCH, cache_pitch));
        return TRUE;
    }
    else
    {
        Msg("! sound: OpenAL: Can't create source. Error: %s.", (LPCSTR)alGetString(error));
        return FALSE;
    }
}

void CSoundRender_TargetA::alAuxInit(ALuint slot)
{
    A_CHK(alSource3i(pSource, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL));
    if (slot == AL_EFFECTSLOT_NULL)
    {
        // EFX going away (commit error, device reopen): drop the NLC filters, recreated on demand
        alSourcei(pSource, AL_DIRECT_FILTER, AL_FILTER_NULL);
        alSource3i(pSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 1, AL_FILTER_NULL);
        alGetError();
        nlc_filters_destroy();
        nlc_filters_tried = false;
    }
    for (float& c : nlc_cache)
        c = -1.f;
}

void CSoundRender_TargetA::nlc_filters_create()
{
    nlc_filters_tried = true;
    nlc_filters_ok = false;
    if (!sndefx::filters_ok())
        return;
    alGetError();
    sndefx::GenFilters(3, nlc_filter);
    for (ALuint f : nlc_filter)
        sndefx::Filteri(f, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    const ALenum err = alGetError();
    nlc_filters_ok = err == AL_NO_ERROR;
    if (!nlc_filters_ok)
    {
        Msg("! [snd] OpenAL lowpass filters unavailable: %s", alGetString(err));
        nlc_filters_destroy();
    }
    for (float& c : nlc_cache)
        c = -1.f;
}

void CSoundRender_TargetA::nlc_filters_destroy()
{
    if (sndefx::filters_ok())
        for (ALuint& f : nlc_filter)
            if (f && sndefx::IsFilter(f))
                sndefx::DeleteFilters(1, &f);
    for (ALuint& f : nlc_filter)
        f = 0;
    nlc_filters_ok = false;
}

// direct path: occlusion HF x air absorption; room send: enclosure mismatch, wet path muffled less;
// echo send: loud sounds (shots, explosions) in the open
void CSoundRender_TargetA::nlc_fill_efx(CSoundRender_Core* core)
{
    if (!nlc_filters_tried)
        nlc_filters_create();
    if (!nlc_filters_ok)
        return;

    const CSoundRender_Emitter* E = m_pEmitter;
    float direct_gain = 1.f, direct_hf = E->nlc_occ_hf, room_gain = 1.f, room_hf = 1.f, echo_gain = 0.f, occ_send = 1.f, occ_echo = 1.f;
    if (!E->b2D)
    {
        // occlusion on the direct path only (AUDIO_DESIGN 13.1); the reverberant field and the echo still reach the listener,
        // weaker: room send x occ^0.5, echo x occ^0.25
        const float dist = core->listener_position().distance_to(E->p_source.position);
        const float air = psSoundAirAbsorption > 0.f ? powf(10.f, -psSoundAirAbsorption * dist / 2000.f) : 1.f;
        const float occ = clampr(E->occluder_volume, 0.f, 1.f);
        direct_gain = occ;
        direct_hf = E->nlc_occ_hf * air;
        room_hf = sqrtf(E->nlc_occ_hf) * air;
        occ_send = sqrtf(occ);
        occ_echo = sqrtf(occ_send);
        const float mismatch = _abs(core->nlc_enc - E->nlc_src_enc);
        room_gain = (1.f + (core->nlc_cfg.send_mismatch - 1.f) * _min(mismatch, 1.f)) * occ_send;
    }
    else if (!E->owner_data || !E->owner_data->g_object || E->owner_data->s_type == st_Music)
        room_gain = 0.f; // rain, music, UI: not in the room (the actor's own 2D sounds keep the send)
    if (E->nlc_loud && core->efx_slot_echo() && core->efx_sends >= 2)
        echo_gain = (1.f - E->nlc_src_enc) * (1.f - 0.5f * core->nlc_enc) * core->nlc_cfg.echo_gain * psSoundOutdoorEcho * occ_echo;
    if (!E->nlc_lf_exempt)
    {
        // listener filter (gas mask): everything that reaches the ears from outside, dry and reverberant alike
        direct_gain *= core->nlc_lf_gain;
        direct_hf *= core->nlc_lf_hf;
        room_gain *= core->nlc_lf_gain;
        room_hf *= core->nlc_lf_hf;
        echo_gain *= core->nlc_lf_gain;
    }
    clamp(direct_gain, 0.f, 1.f);
    clamp(direct_hf, 0.01f, 1.f);
    clamp(room_hf, 0.01f, 1.f);
    clamp(room_gain, 0.f, 1.f);
    clamp(echo_gain, 0.f, 1.f);

    if (_abs(direct_hf - nlc_cache[0]) > 0.01f || _abs(direct_gain - nlc_cache[5]) > 0.002f + 0.02f * direct_gain)
    {
        nlc_cache[0] = direct_hf;
        nlc_cache[5] = direct_gain;
        sndefx::Filterf(nlc_filter[0], AL_LOWPASS_GAIN, direct_gain);
        sndefx::Filterf(nlc_filter[0], AL_LOWPASS_GAINHF, direct_hf);
        alSourcei(pSource, AL_DIRECT_FILTER, nlc_filter[0]);
    }
    const float slots = core->efx_reverb_on() ? 1.f : 0.f;
    if (_abs(room_gain - nlc_cache[1]) > 0.01f || _abs(room_hf - nlc_cache[2]) > 0.01f || slots != nlc_cache[4])
    {
        nlc_cache[1] = room_gain;
        nlc_cache[2] = room_hf;
        sndefx::Filterf(nlc_filter[1], AL_LOWPASS_GAIN, room_gain);
        sndefx::Filterf(nlc_filter[1], AL_LOWPASS_GAINHF, room_hf);
        alSource3i(pSource, AL_AUXILIARY_SEND_FILTER, core->efx_slot_room(), 0, nlc_filter[1]);
    }
    if (core->efx_sends >= 2 && core->efx_slot_echo() && (_abs(echo_gain - nlc_cache[3]) > 0.01f || slots != nlc_cache[4]))
    {
        nlc_cache[3] = echo_gain;
        sndefx::Filterf(nlc_filter[2], AL_LOWPASS_GAIN, echo_gain);
        sndefx::Filterf(nlc_filter[2], AL_LOWPASS_GAINHF, 1.f);
        alSource3i(pSource, AL_AUXILIARY_SEND_FILTER, core->efx_slot_echo(), 1, nlc_filter[2]);
    }
    nlc_cache[4] = slots;
}

void CSoundRender_TargetA::_destroy()
{
    nlc_filters_destroy();
    nlc_filters_tried = false;

    // clean up target
    if (alIsSource(pSource))
        alDeleteSources(1, &pSource);
    A_CHK(alDeleteBuffers(sdef_target_count, pBuffers));
}

void CSoundRender_TargetA::_restart()
{
    _destroy();
    _initialize();
}

void CSoundRender_TargetA::start(CSoundRender_Emitter* E)
{
    inherited::start(E);

    // Calc storage
    buf_block = sdef_target_block * E->source()->m_wformat.nAvgBytesPerSec / 1000;
    g_target_temp_data.resize(buf_block);

    // NLC (docs/AUDIO_DESIGN.md 15-16): with HRTF on, 2D sounds (the player's weapons, HUD, UI, music, rain) go straight to the
    // output channels instead of through the head model, which rendered them as a coloured "inside the head" image
    nlc_direct = E->b2D && SoundRender->nlc_hrtf_on && !psSoundHrtf2D;
    alSourcei(pSource, AL_DIRECT_CHANNELS_SOFT, nlc_direct ? AL_TRUE : AL_FALSE);
    // stereo files kept 3D: OpenAL Soft downmixes and positions them (AUTO spatializes mono only)
    alSourcei(pSource, AL_SOURCE_SPATIALIZE_SOFT, E->nlc_spatial_stereo ? AL_TRUE : AL_AUTO_SOFT);
    alGetError();
}

void CSoundRender_TargetA::render()
{
    for (u32 buf_idx = 0; buf_idx < sdef_target_count; buf_idx++)
        fill_block(pBuffers[buf_idx]);

    A_CHK(alSourceQueueBuffers(pSource, sdef_target_count, pBuffers));
    A_CHK(alSourcePlay(pSource));

    inherited::render();
}

void CSoundRender_TargetA::stop()
{
    if (rendering)
    {
        A_CHK(alSourceStop(pSource));
        A_CHK(alSourcei(pSource, AL_BUFFER, NULL));
        A_CHK(alSourcei(pSource, AL_SOURCE_RELATIVE, TRUE));
    }
    inherited::stop();
}

void CSoundRender_TargetA::rewind()
{
    inherited::rewind();

    A_CHK(alSourceStop(pSource));
    A_CHK(alSourcei(pSource, AL_BUFFER, NULL));
    for (u32 buf_idx = 0; buf_idx < sdef_target_count; buf_idx++)
        fill_block(pBuffers[buf_idx]);
    A_CHK(alSourceQueueBuffers(pSource, sdef_target_count, pBuffers));
    A_CHK(alSourcePlay(pSource));
}

void CSoundRender_TargetA::update()
{
    inherited::update();

    // Msg("--[%s] bEFX is [%d]", __FUNCTION__, this->bEFX);

    if (bAlSoft)
    {
        ALint processed, state;

        /* Get relevant source info */
        alGetSourcei(pSource, AL_SOURCE_STATE, &state);
        alGetSourcei(pSource, AL_BUFFERS_PROCESSED, &processed);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR)
        {
            Msg("!![%s]Error checking source state! OpenAL Error: [%s]", __FUNCTION__, alGetString(error));
            return;
        }

        while (processed > 0)
        {
            ALuint BufferID;
            A_CHK(alSourceUnqueueBuffers(pSource, 1, &BufferID));
            fill_block(BufferID);
            A_CHK(alSourceQueueBuffers(pSource, 1, &BufferID));
            processed--;
            ALenum error = alGetError();
            if (error != AL_NO_ERROR)
            {
                Msg("!![%s]Error buffering data! OpenAL Error: [%s]", __FUNCTION__, alGetString(error));
                return;
            }
        }

        /* Make sure the source hasn't underrun */
        if (state != AL_PLAYING && state != AL_PAUSED)
        {
            ALint queued;

            /* If no buffers are queued, playback is finished */
            alGetSourcei(pSource, AL_BUFFERS_QUEUED, &queued);
            if (queued == 0)
                return;

            alSourcePlay(pSource);
            ALenum error = alGetError();
            if (error != AL_NO_ERROR)
            {
                Msg("!![%s]Error restarting playback! OpenAL Error: [%s]", __FUNCTION__, alGetString(error));
                return;
            }
        }
    }
    else
    {
        ALint processed;
        // Get status
        A_CHK(alGetSourcei(pSource, AL_BUFFERS_PROCESSED, &processed));

        if (processed > 0)
        {
            while (processed)
            {
                ALuint BufferID;
                A_CHK(alSourceUnqueueBuffers(pSource, 1, &BufferID));
                fill_block(BufferID);
                A_CHK(alSourceQueueBuffers(pSource, 1, &BufferID));
                --processed;
            }
        }
        else
        {
            // processed == 0
            // check play status -- if stopped then queue is not being filled fast enough
            ALint state;
            A_CHK(alGetSourcei(pSource, AL_SOURCE_STATE, &state));
            if (state != AL_PLAYING)
            {
                //			Log		("Queuing underrun detected.");
                A_CHK(alSourcePlay(pSource));
            }
        }
    }
}

void CSoundRender_TargetA::fill_parameters(CSoundRender_Core* core)
{
#ifdef DEBUG
    CSoundRender_Emitter* SE = m_pEmitter;
    VERIFY(SE);
#endif

    inherited::fill_parameters(core);

    // 3D params
    VERIFY2(m_pEmitter, SE->source()->file_name());
    A_CHK(alSourcef(pSource, AL_REFERENCE_DISTANCE, m_pEmitter->p_source.min_distance));

    VERIFY2(m_pEmitter, SE->source()->file_name());
    A_CHK(alSourcef(pSource, AL_MAX_DISTANCE, m_pEmitter->p_source.max_distance));

    VERIFY2(m_pEmitter, SE->source()->file_name());
    A_CHK(alSource3f(pSource, AL_POSITION, m_pEmitter->p_source.position.x, m_pEmitter->p_source.position.y, -m_pEmitter->p_source.position.z));

    VERIFY2(m_pEmitter, SE->source()->file_name());
    A_CHK(alSourcei(pSource, AL_SOURCE_RELATIVE, m_pEmitter->b2D));

    A_CHK(alSourcef(pSource, AL_ROLLOFF_FACTOR, psSoundRolloff));

    VERIFY2(m_pEmitter, SE->source()->file_name());
    float _gain = m_pEmitter->smooth_volume;
    if (nlc_direct && m_pEmitter->source()->m_wformat.nChannels == 1)
        _gain *= core->nlc_cfg.hrtf_2d_gain; // both ears get the full mono signal (a centred pan split it)
    if (bEFX && psSoundOccMode == 1 && !nlc_filters_ok && nlc_filters_tried && !m_pEmitter->b2D)
        _gain *= m_pEmitter->occluder_volume; // NLC: this voice has no filters: occlusion back in the gain
    if (psSoundDistModel == 1 && !m_pEmitter->b2D)
    {
        // NLC: engine distance curve (OpenAL attenuation off): inverse rolloff, faded to zero over the last 20% of max_dist
        const float d = core->listener_position().distance_to(m_pEmitter->p_source.position);
        const float mn = _max(m_pEmitter->p_source.min_distance, 0.1f);
        const float mx = _max(m_pEmitter->p_source.max_distance, mn + 0.1f);
        float k = d > mn ? mn / (mn + psSoundRolloff * (d - mn)) : 1.f;
        if (d > 0.8f * mx)
            k *= clampr((mx - d) / (0.2f * mx), 0.f, 1.f);
        _gain *= k;
    }
    clamp(_gain, EPS_S, 1.f);
    if (!fsimilar(_gain, cache_gain, 0.01f))
    {
        cache_gain = _gain;
        A_CHK(alSourcef(pSource, AL_GAIN, _gain));
    }

    VERIFY2(m_pEmitter, SE->source()->file_name());
    float _pitch = m_pEmitter->p_source.freq * psSoundTimeFactor; //--#SM+#-- Correct sound "speed" by time factor
    clamp(_pitch, EPS_L, 100.f); //--#SM+#-- Increase sound frequancy (speed) limit
    if (!fsimilar(_pitch, cache_pitch))
    {
        cache_pitch = _pitch;
        A_CHK(alSourcef(pSource, AL_PITCH, _pitch));
    }
    VERIFY2(m_pEmitter, SE->source()->file_name());

    if (bEFX && psSoundOccMode == 1)
        nlc_fill_efx(core);
    else if (nlc_filters_ok && nlc_cache[0] >= 0.f)
    {
        // legacy mode switched on at runtime: unfiltered direct path, plain room send, no echo
        alSourcei(pSource, AL_DIRECT_FILTER, AL_FILTER_NULL);
        alSource3i(pSource, AL_AUXILIARY_SEND_FILTER, core->efx_slot_room(), 0, AL_FILTER_NULL);
        if (core->efx_sends >= 2)
            alSource3i(pSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 1, AL_FILTER_NULL);
        for (float& c : nlc_cache)
            c = -1.f;
    }
}

void CSoundRender_TargetA::fill_block(ALuint BufferID)
{
    R_ASSERT(m_pEmitter);

    m_pEmitter->fill_block(g_target_temp_data.data(), g_target_temp_data.size());

    const auto& wvf = m_pEmitter->source()->m_wformat;
    const bool mono = wvf.nChannels == 1;

    if (mono && nlc_direct)
    {
        // NLC: duplicate the mono block to stereo (direct channels map a stereo buffer to the front left/right outputs)
        const u32 bps = wvf.wBitsPerSample / 8;
        const size_t size = g_target_temp_data.size();
        g_target_temp_stereo.resize(size * 2);
        const u8* src = g_target_temp_data.data();
        u8* dst = g_target_temp_stereo.data();
        for (size_t i = 0; i + bps <= size; i += bps)
        {
            memcpy(dst + i * 2, src + i, bps);
            memcpy(dst + i * 2 + bps, src + i, bps);
        }
        ALuint fmt = AL_FORMAT_STEREO16;
#ifdef AL_EXT_float32
        if (wvf.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
            fmt = AL_FORMAT_STEREO_FLOAT32;
#endif
        A_CHK(alBufferData(BufferID, fmt, dst, size * 2, wvf.nSamplesPerSec));
        return;
    }

    ALuint format;
#ifdef AL_EXT_float32
    if (wvf.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        format = mono ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
    else
#endif
    {
        format = mono ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    }

    A_CHK(alBufferData(BufferID, format, g_target_temp_data.data(), g_target_temp_data.size(), m_pEmitter->source()->m_wformat.nSamplesPerSec));
}
void CSoundRender_TargetA::source_changed()
{
    dettach();
    attach();
}
