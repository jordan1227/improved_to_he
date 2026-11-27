//////////////////////////////////////////////////////////////////////
// HudSound.h:		структура для работы со звуками применяемыми в
//					HUD-объектах (обычные звуки, но с доп. параметрами)
//////////////////////////////////////////////////////////////////////

#pragma once

struct HUD_SOUND
{
    HUD_SOUND() { m_activeSnd = NULL; }
    ~HUD_SOUND() { m_activeSnd = NULL; }

    ////////////////////////////////////
    // работа со звуками
    /////////////////////////////////////
    static void LoadSound(LPCSTR section, LPCSTR line, ref_sound& hud_snd, int type = sg_SourceType, float* volume = NULL, float* delay = NULL, float* freq = NULL);

    static void LoadSound(LPCSTR section, LPCSTR line, HUD_SOUND& hud_snd, int type = sg_SourceType);

    static void DestroySound(HUD_SOUND& hud_snd);

    static void PlaySound(HUD_SOUND& snd, const Fvector& position, const CObject* parent, bool hud_mode, bool looped = false, bool overlap = false);

    static void StopSound(HUD_SOUND& snd);

    ICF BOOL playing()
    {
        if (m_activeSnd)
            return m_activeSnd->snd._feedback() ? TRUE : FALSE;
        else
            return FALSE;
    }

    ICF void set_position(const Fvector& pos)
    {
        if (m_activeSnd)
        {
            if (m_activeSnd->snd._feedback() && !m_activeSnd->snd._feedback()->is_2D())
                m_activeSnd->snd.set_position(pos);
            else
                m_activeSnd = NULL;
        }
    }

    struct SSnd
    {
        ref_sound snd;
        float delay; //задержка перед проигрыванием
        float volume; //громкость
        float freq; //коэффициент частоты
    };
    SSnd* m_activeSnd;
    xr_vector<SSnd> sounds;
    u8 m_nlc_speed_kind{}; // NLC: NLC_SPEED_* (player_hud.h); the actor's sound follows that handling speed
    float m_nlc_item_k{1.f}; // NLC: the playing item's own draw speed ( CHudItem::nlc_draw_k ), set per play by CHudItem::PlaySound
    // NLC: the item's draw speed moves the sound timing in full but its pitch only within 0.8..1.2 ( a 1.4x pistol draw
    // would sound broken otherwise ); global factors ( injury, flytrap ) keep their full pitch
    static float nlc_item_pitch_k(float item_k) { return item_k < 0.8f ? 0.8f : (item_k > 1.2f ? 1.2f : item_k); }
};
