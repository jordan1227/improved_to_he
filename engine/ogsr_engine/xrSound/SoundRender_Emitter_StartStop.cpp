#include "stdafx.h"

#include "SoundRender_Core.h"
#include "SoundRender_Emitter.h"
#include "SoundRender_Source.h"

void CSoundRender_Emitter::start(ref_sound* _owner, BOOL _loop, float delay, u32 flags)
{
    starting_delay = delay;

    nlc_env_ambient = !!(flags & sm_EnvAmbient);
    nlc_no_occl = false;
    nlc_occ_pending = false;
    nlc_pos_set = false;
    nlc_delay_checked = false;
    nlc_delayed = false;
    nlc_occ_gain_t = nlc_occ_hf_t = nlc_occ_hf = 1.f;
    nlc_occ_db = 0.f;
    nlc_occ_next = 0.f;
    nlc_occ_path = 0;
    nlc_src_enc = 0.f;
    nlc_src_done = false;
    nlc_loud = false;

    VERIFY(_owner);
    owner_data = _owner->_p;
    VERIFY(owner_data);
    //	source					= (CSoundRender_Source*)owner_data->handle;
    p_source.position.set(0, 0, 0);
    p_source.min_distance = source()->m_fMinDist; // DS3D_DEFAULTMINDISTANCE;
    p_source.max_distance = source()->m_fMaxDist; // 300.f;
    p_source.base_volume = source()->m_fBaseVolume; // 1.f
    p_source.volume = 1.f; // 1.f
    p_source.freq = 1.f;
    p_source.max_ai_distance = source()->m_fMaxAIDist; // 300.f;

    // NLC: 2D before the emitter is visible to the sound task (the caller's switch_to_2D came after set_position, so a
    // 2D play_at_pos could be taken for a distant 3D sound and get a speed-of-sound delay)
    // NLC: stereo files listed in snd_occlusion.ltx spatial_stereo stay 3D (shell casings: all stereo, played at an NPC)
    nlc_spatial_stereo = !(flags & sm_2D) && source()->channels_num() == 2 && SoundRender->nlc_spatial_stereo(source()->fname.c_str());
    if ((flags & sm_2D) || (source()->channels_num() == 2 && !nlc_spatial_stereo))
        switch_to_2D();
    // not affected by the listener filter (gas mask): music and the script's exempt names (breathing, own body sounds, UI)
    nlc_lf_exempt = owner_data->s_type == st_Music || SoundRender->nlc_lf_exempt(source()->fname.c_str());
    nlc_psy_exempt = owner_data->s_type == st_Music || SoundRender->nlc_psy_exempt(source()->fname.c_str());

    if (fis_zero(delay, EPS_L))
    {
        m_current_state = _loop ? stStartingLooped : stStarting;
    }
    else
    {
        m_current_state = _loop ? stStartingLoopedDelayed : stStartingDelayed;
        fTimeToPropagade = SoundRender->Timer.GetElapsed_sec();
    }
    bStopping = FALSE;
    bRewind = FALSE;
}

void CSoundRender_Emitter::i_stop()
{
    bRewind = FALSE;
    if (target)
        SoundRender->i_stop(this);
    if (owner_data)
    {
        Event_ReleaseOwner();
        VERIFY(this == owner_data->feedback);
        owner_data->feedback = NULL;
        owner_data = NULL;
    }
    m_current_state = stStopped;
}

void CSoundRender_Emitter::stop(BOOL bDeffered)
{
    if (bDeffered)
        bStopping = TRUE;
    else
        i_stop();
}

void CSoundRender_Emitter::rewind()
{
    bStopping = FALSE;

    float fTime = SoundRender->Timer.GetElapsed_sec();
    float fDiff = fTime - fTimeStarted;
    fTimeStarted += fDiff;
    fTimeToStop += fDiff;
    fTimeToPropagade = fTime;

    set_cursor(0);
    bRewind = TRUE;
}

void CSoundRender_Emitter::pause(BOOL bVal, int id)
{
    if (bVal)
    {
        if (0 == iPaused)
            iPaused = id;
    }
    else
    {
        if (id == iPaused)
            iPaused = 0;
    }
}

void CSoundRender_Emitter::cancel()
{
    // Msg		("- %10s : %3d[%1.4f] : %s","cancel",dbg_ID,priority(),source->fname);
    switch (m_current_state)
    {
    case stPlaying:
        // switch to: SIMULATE
        m_current_state = stSimulating; // switch state
        SoundRender->i_stop(this);
        break;
    case stPlayingLooped:
        // switch to: SIMULATE
        m_current_state = stSimulatingLooped; // switch state
        SoundRender->i_stop(this);
        break;
    default: FATAL("Non playing ref_sound forced out of render queue"); break;
    }
}
