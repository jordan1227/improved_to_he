#include "stdafx.h"

#include "SoundRender_Emitter.h"
#include "SoundRender_Core.h"
#include "SoundRender_Source.h"

XRSOUND_API extern float psSoundCull;
constexpr float TIME_TO_STOP_INFINITE = static_cast<float>(0xffffffff);

inline u32 calc_cursor(const float& fTimeStarted, float& fTime, const float& fTimeTotal, const float& fFreq, const WAVEFORMATEX& wfx)
{
    if (fTime < fTimeStarted)
        fTime = fTimeStarted; // Андрюха посоветовал, ассерт что ниже вылетел из за паузы как то хитро
    R_ASSERT((fTime - fTimeStarted) >= 0.0f);
    while ((fTime - fTimeStarted) > fTimeTotal / fFreq) // looped
    {
        fTime -= fTimeTotal / fFreq;
    }
    u32 curr_sample_num = iFloor((fTime - fTimeStarted) * fFreq * wfx.nSamplesPerSec);
    return curr_sample_num * (wfx.wBitsPerSample / 8) * wfx.nChannels;
}

void CSoundRender_Emitter::update(float dt)
{
    float fTime = SoundRender->fTimer_Value;
    float fDeltaTime = SoundRender->fTimer_Delta;

    VERIFY2(!!(owner_data) || (!(owner_data) && (m_current_state == stStopped)), "owner");
    VERIFY2(owner_data ? *(int*)(&owner_data->feedback) : 1, "owner");

    if (bRewind)
    {
        if (target)
            SoundRender->i_rewind(this);
        bRewind = FALSE;
    }

    switch (m_current_state)
    {
    case stStopped: break;
    case stStartingDelayed:
        if (iPaused)
        {
            fTimeToPropagade += fDeltaTime; // NLC: no AI pulses while paused (distance-delayed sounds sit here)
            break;
        }
        starting_delay -= dt;
        if (starting_delay <= 0)
            m_current_state = stStarting;
        break;
    case stStarting:
        if (iPaused)
            break;
        // NLC: speed-of-sound delay for distant 3D sounds; AI events keep propagating on time (AUDIO_DESIGN 5.1)
        if (!nlc_delay_checked)
        {
            nlc_delay_checked = true;
            if (psSoundDelay && !b2D && nlc_pos_set && !nlc_env_ambient && psSoundSpeed > 1.f)
            {
                const float dist = nlc_listener_dist();
                if (dist > psSoundDelayMinDist)
                {
                    starting_delay = dist / psSoundSpeed;
                    if (nlc_log_match())
                        Msg("[snd_delay] %s d=%.1f delay=%.2f s", source()->fname.c_str(), dist, starting_delay);
                    m_current_state = stStartingDelayed;
                    fTimeToPropagade = fTime;
                    nlc_delayed = true;
                    break;
                }
            }
        }
        fTimeStarted = fTime;
        fTimeToStop = fTime + (get_length_sec() / p_source.freq); //--#SM+#--
        if (!nlc_delayed)
            fTimeToPropagade = fTime;
        fade_volume = 1.f;
        if (psSoundOccMode == 1)
            nlc_occ_refresh(true);
        else
            occluder_volume = SoundRender->get_occlusion(p_source.position, .2f, occluder);
        smooth_volume = p_source.base_volume * p_source.volume * (owner_data->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic) * (b2D || nlc_occ_in_filter() ? 1.f : occluder_volume);

        if (update_culling(dt))
        {
            m_current_state = stPlaying;
            set_cursor(0);
            SoundRender->i_start(this);
        }
        else
            m_current_state = stSimulating;
        break;
    case stStartingLoopedDelayed:
        if (iPaused)
            break;
        starting_delay -= dt;
        if (starting_delay <= 0)
            m_current_state = stStartingLooped;
        break;
    case stStartingLooped:
        if (iPaused)
            break;
        fTimeStarted = fTime;
        fTimeToStop = TIME_TO_STOP_INFINITE;
        fTimeToPropagade = fTime;
        fade_volume = 1.f;
        if (psSoundOccMode == 1)
            nlc_occ_refresh(true);
        else
            occluder_volume = SoundRender->get_occlusion(p_source.position, .2f, occluder);
        smooth_volume = p_source.base_volume * p_source.volume * (owner_data->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic) * (b2D || nlc_occ_in_filter() ? 1.f : occluder_volume);
        
        if (update_culling(dt))
        {
            m_current_state = stPlayingLooped;
            set_cursor(0);
            SoundRender->i_start(this);
        }
        else
            m_current_state = stSimulatingLooped;
        break;
    case stPlaying:
        if (iPaused)
        {
            if (target)
            {
                SoundRender->i_stop(this);
                m_current_state = stSimulating;
            }
            fTimeStarted += fDeltaTime;
            fTimeToStop += fDeltaTime;
            fTimeToPropagade += fDeltaTime;
            break;
        }
        if (fTime >= fTimeToStop)
        {
            // STOP
            m_current_state = stStopped;
            SoundRender->i_stop(this);
        }
        else
        {
            if (!update_culling(dt))
            {
                // switch to: SIMULATE
                m_current_state = stSimulating; // switch state
                SoundRender->i_stop(this);
            }
            else
            {
                // We are still playing
                update_environment(dt);
            }
        }
        break;
    case stSimulating:
        if (iPaused)
        {
            fTimeStarted += fDeltaTime;
            fTimeToStop += fDeltaTime;
            fTimeToPropagade += fDeltaTime;
            break;
        }
        if (fTime >= fTimeToStop)
        {
            // STOP
            m_current_state = stStopped;
        }
        else
        {
            u32 ptr = calc_cursor(fTimeStarted, fTime, get_length_sec(), p_source.freq, source()->m_wformat); //--#SM+#--
            set_cursor(ptr);

            if (update_culling(dt))
            {
                // switch to: PLAY
                m_current_state = stPlaying;
                /*
                                u32 ptr						= calc_cursor(	fTimeStarted,
                                                                            fTime,
                                                                            get_length_sec(),
                                                                            source()->m_wformat);
                                set_cursor					(ptr);
                */
                SoundRender->i_start(this);
            }
        }
        break;
    case stPlayingLooped:
        if (iPaused)
        {
            if (target)
            {
                SoundRender->i_stop(this);
                m_current_state = stSimulatingLooped;
            }
            fTimeStarted += fDeltaTime;
            fTimeToPropagade += fDeltaTime;
            break;
        }
        if (!update_culling(dt))
        {
            // switch to: SIMULATE
            m_current_state = stSimulatingLooped; // switch state
            SoundRender->i_stop(this);
        }
        else
        {
            // We are still playing
            update_environment(dt);
        }
        break;
    case stSimulatingLooped:
        if (iPaused)
        {
            fTimeStarted += fDeltaTime;
            fTimeToPropagade += fDeltaTime;
            break;
        }
        if (update_culling(dt))
        {
            // switch to: PLAY
            m_current_state = stPlayingLooped; // switch state
            u32 ptr = calc_cursor(fTimeStarted, fTime, get_length_sec(), p_source.freq, source()->m_wformat); //--#SM+#--
            set_cursor(ptr);

            SoundRender->i_start(this);
        }
        break;
    }

	//--#SM+# Begin--
    // hard rewind
    switch (m_current_state)
    {
    case stStarting:
    case stStartingLooped:
    case stPlaying:
    case stSimulating:
    case stPlayingLooped:
    case stSimulatingLooped:
        if (fTimeToRewind > 0.0f)
        {
            float fLength = get_length_sec();
            bool bLooped = (fTimeToStop == 0xffffffff);

            R_ASSERT2(fLength >= fTimeToRewind, "set_time: target time is bigger than length of sound");

            float fRemainingTime = (fLength - fTimeToRewind) / p_source.freq;
            float fPastTime = fTimeToRewind / p_source.freq;

            fTimeStarted = SoundRender->fTimer_Value - fPastTime;
            fTimeToPropagade = fTimeStarted; //--> For AI events

            if (fTimeStarted < 0.0f)
            {
                //Log("fTimer_Value = ", SoundRender->fTimer_Value);
                //Log("fTimeStarted = ", fTimeStarted);
                //Log("fRemainingTime = ", fRemainingTime);
                //Log("fPastTime = ", fPastTime);
                R_ASSERT2(fTimeStarted >= 0.0f, "Possible error in sound rewind logic! See log.");

                fTimeStarted = SoundRender->fTimer_Value;
                fTimeToPropagade = fTimeStarted;
            }

            if (!bLooped)
            {
                //--> Пересчитываем время, когда звук должен остановиться [recalculate stop time]
                fTimeToStop = SoundRender->fTimer_Value + fRemainingTime;
            }

            u32 ptr = calc_cursor(fTimeStarted, fTime, fLength, p_source.freq, source()->m_wformat);
            set_cursor(ptr);

            fTimeToRewind = 0.0f;
        }
    default: break;
    }
    //--#SM+# End--


    // if deffered stop active and volume==0 -> physically stop sound
    if (bStopping && fis_zero(fade_volume))
        i_stop();

    VERIFY2(!!(owner_data) || (!(owner_data) && (m_current_state == stStopped)), "owner");
    VERIFY2(owner_data ? *(int*)(owner_data->feedback) : 1, "owner");

    // footer
    bMoved = FALSE;
    if (m_current_state != stStopped)
    {
        if (fTime >= fTimeToPropagade)
            Event_Propagade();
    }
    else if (owner_data)
    {
        VERIFY(this == owner_data->feedback);
        owner_data->feedback = 0;
        owner_data = 0;
    }
}

IC void volume_lerp(float& c, float t, float s, float dt)
{
    float diff = t - c;
    float diff_a = _abs(diff);
    if (diff_a < EPS_S)
        return;
    float mot = s * dt;
    if (mot > diff_a)
        mot = diff_a;
    c += (diff / diff_a) * mot;
}
#include "..\COMMON_AI\ai_sounds.h"
BOOL CSoundRender_Emitter::update_culling(float dt)
{
    const float type_vol = owner_data->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic;
    const bool nlc = psSoundOccMode == 1;
    if (b2D)
    {
        occluder_volume = 1.f;
        if (nlc && nlc_env_ambient)
        {
            // 2D weather loops (rain): only dulled indoors; their volume comes from the caller (Rain.cpp)
            nlc_occ_refresh(false);
            nlc_occ_follow(dt);
            occluder_volume = 1.f;
        }
        else
            nlc_occ_hf = 1.f;
        fade_volume += dt * 10.f * (bStopping ? -1.f : 1.f);
    }
    else
    {
        // Check range
        float dist = SoundRender->listener_position().distance_to(p_source.position);
        if (dist > p_source.max_distance)
        {
            smooth_volume = 0;
            return FALSE;
        }

        // Calc attenuated volume
        const float audible = att() * p_source.base_volume * p_source.volume * type_vol;
        float fade_scale = bStopping || (audible < psSoundCull) ? -1.f : 1.f;
        fade_volume += dt * 10.f * fade_scale;

        // Update occlusion
        if (nlc)
        {
            // occlusion only lowers the level: nothing to evaluate for a sound already below the cull level
            if (audible >= psSoundCull)
                nlc_occ_refresh(false, audible);
            nlc_occ_follow(dt);
        }
        else
        {
            float occ = (owner_data->g_type == SOUND_TYPE_WORLD_AMBIENT) ? 1.0f : SoundRender->get_occlusion(p_source.position, .2f, occluder);
            volume_lerp(occluder_volume, occ, 1.f, dt);
            nlc_occ_hf = 1.f;
        }
        clamp(occluder_volume, 0.f, 1.f);
    }
    clamp(fade_volume, 0.f, 1.f);
    // Update smoothing
    // NLC: with the EFX filters the occlusion goes to the direct-path filter (TargetA::nlc_fill_efx), not the voice gain:
    // culling and priority use the level before occlusion, and the reverb/echo sends keep their own share (AUDIO_DESIGN 13.1)
    const float occ_k = nlc_occ_in_filter() ? 1.f : occluder_volume;
    smooth_volume = .9f * smooth_volume + .1f * (p_source.base_volume * p_source.volume * type_vol * occ_k * fade_volume);
    if (smooth_volume < psSoundCull)
        return FALSE; // allow volume to go up
    // Here we has enought "PRIORITY" to be soundable
    // If we are playing already, return OK
    // --- else check availability of resources
    if (target)
        return TRUE;
    else
        return SoundRender->i_allow_play(this);
}

float CSoundRender_Emitter::priority() { return b2D ? 1000.f : smooth_volume * att() * priority_scale; }

float CSoundRender_Emitter::att()
{
    float dist = SoundRender->listener_position().distance_to(p_source.position);
    float rolloff_dist = psSoundRolloff * dist;

    // Calc linear fade --#SM+#--
    // https://www.desmos.com/calculator/lojovfugle
    const float fMinDistDiff = rolloff_dist - p_source.min_distance;
    float att;
    if (fMinDistDiff > 0.f)
    {
        const float fMaxDistDiff = p_source.max_distance - p_source.min_distance;
        att = pow(1.f - (fMinDistDiff / fMaxDistDiff), psSoundLinearFadeFactor);
    }
    else
        att = 1.f;
    clamp(att, 0.f, 1.f);

    return att;
}

void CSoundRender_Emitter::update_environment(float dt)
{
}

// NLC occlusion (docs/AUDIO_DESIGN.md sections 2 and 13): due emitters are queued and served by the core within the ray
// budget (4-15 Hz by distance), smoothed with separate attack (clearer) and release (muffled) times
float CSoundRender_Emitter::nlc_listener_dist() const { return SoundRender->listener_position().distance_to(p_source.position); }

bool CSoundRender_Emitter::nlc_occ_in_filter() const { return psSoundOccMode == 1 && SoundRender->nlc_filters_active(); }

// snd_occ_log: "*" = every sound, otherwise a part of the sound's file name
bool CSoundRender_Emitter::nlc_log_match()
{
    if (!psSoundOccLog[0] || !owner_data)
        return false;
    return psSoundOccLog[0] == '*' || strstr(source()->fname.c_str(), psSoundOccLog);
}

void CSoundRender_Emitter::nlc_occ_refresh(bool immediate, float audible)
{
    if (immediate)
    {
        nlc_loud = owner_data && ((owner_data->g_type & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING ||
                                  (owner_data->g_type & SOUND_TYPE_WORLD_OBJECT_EXPLODING) == SOUND_TYPE_WORLD_OBJECT_EXPLODING);
        nlc_no_occl = owner_data && SoundRender->nlc_no_occlude(source()->fname.c_str());
    }

    if (b2D)
    {
        nlc_occ_gain_t = 1.f;
        nlc_occ_hf_t = nlc_env_ambient ? 1.f + (SoundRender->nlc_cfg.amb_indoor_hf - 1.f) * SoundRender->nlc_enc : 1.f;
        nlc_src_enc = SoundRender->nlc_enc;
        if (immediate)
        {
            occluder_volume = 1.f;
            nlc_occ_hf = nlc_occ_hf_t;
        }
        return;
    }

    const float now = SoundRender->fTimer_Value;
    if (nlc_occ_next > now + 0.5f) // the timer jumped back (load, pause): do not wait for it to catch up
        nlc_occ_next = 0.f;

    if (nlc_env_ambient)
    {
        // weather ambient channels: random positions around the camera, ducked by the listener's enclosure instead of rays
        const float E = SoundRender->nlc_enc;
        nlc_occ_gain_t = 1.f + (psSoundAmbIndoorGain - 1.f) * E;
        nlc_occ_hf_t = 1.f + (SoundRender->nlc_cfg.amb_indoor_hf - 1.f) * E;
        nlc_occ_db = -20.f * log10f(_max(nlc_occ_gain_t, 0.001f));
    }
    else if (nlc_no_occl)
    {
        nlc_occ_gain_t = nlc_occ_hf_t = 1.f;
        nlc_occ_db = 0.f;
    }
    else if (immediate)
    {
        if (!nlc_pos_set)
        {
            // position not set yet (the caller sets it right after play): start clear, evaluate next update
            occluder_volume = 1.f;
            nlc_occ_hf = 1.f;
            nlc_occ_next = 0.f;
            nlc_occ_pending = true;
            return;
        }
        nlc_occ_compute(true);
    }
    else
    {
        if (now < nlc_occ_next)
            return;
        // priority: loudness x proximity (att), loud sounds and first full evaluations x4, older requests climb
        const float stale = _min(now - nlc_occ_next, 2.f);
        const float key = audible * (nlc_loud ? 4.f : 1.f) * (nlc_occ_pending ? 4.f : 1.f) * (1.f + 4.f * stale);
        SoundRender->nlc_queue(this, key);
        return;
    }

    if (immediate)
    {
        occluder_volume = nlc_occ_gain_t;
        nlc_occ_hf = nlc_occ_hf_t;
    }
}

// one evaluation; at a sound's start loud sounds always get the leak paths, others only while the budget lasts
// (then the direct ray now and the full evaluation queued for the next update)
void CSoundRender_Emitter::nlc_occ_compute(bool immediate)
{
    const float now = SoundRender->fTimer_Value;
    const float dist = nlc_listener_dist();
    const bool leaks = !immediate || nlc_loud || SoundRender->nlc_rays_left > 0;
    const bool log = nlc_log_match();
    float gain, hf, db;
    u8 path;
    SoundRender->nlc_occlusion(p_source.position, dist, nlc_loud, leaks, gain, hf, db, path, log ? source()->fname.c_str() : nullptr);
    nlc_occ_gain_t = gain;
    nlc_occ_hf_t = hf;
    nlc_occ_db = db;
    nlc_occ_path = path;
    nlc_occ_pending = !leaks;
    nlc_occ_next = leaks ? now + (dist < 15.f ? 1.f / 15.f : (dist < 50.f ? 1.f / 8.f : 1.f / 4.f)) : 0.f;

    if (!nlc_src_done)
    {
        nlc_src_done = true;
        nlc_src_enc = dist < 200.f ? SoundRender->nlc_source_enclosure(p_source.position) : 0.f;
    }
}

void CSoundRender_Emitter::nlc_occ_follow(float dt)
{
    const SSndOccConfig& C = SoundRender->nlc_cfg;
    dt = clampr(dt, 0.f, 0.5f); // see nlc_probe_update: a bad first delta must not poison the smoothing
    auto follow = [&](float& c, float t) {
        const float tau = _max(t > c ? C.attack : C.release, 0.01f);
        c += (t - c) * (1.f - expf(-dt / tau));
        if (!_valid(c))
            c = t;
    };
    follow(occluder_volume, nlc_occ_gain_t);
    follow(nlc_occ_hf, nlc_occ_hf_t);
}
