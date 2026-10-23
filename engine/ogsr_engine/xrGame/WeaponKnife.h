#pragma once

#include "WeaponCustomPistol.h"
#include "script_export_space.h"

class CWeaponKnife : public CWeapon
{
private:
    typedef CWeapon inherited;

protected:
    HUD_SOUND m_sndShot;
    HUD_SOUND sndItemOn;

    bool m_attackStart;
    bool m_attackMotionMarksAvailable;

    bool HeadLampSwitch{}, NightVisionSwitch{};

protected:
    virtual void switch2_Idle();
    virtual void switch2_Hiding();
    virtual void switch2_Hidden();
    virtual void switch2_Showing();
    void switch2_Attacking(u32 state);

    virtual void OnMotionMark(u32 state, const motion_marks& M);
    virtual void OnAnimationEnd(u32 state);
    virtual void OnStateSwitch(u32 S, u32 oldState);

    virtual void DeviceUpdate() override;
    virtual void UpdateCL() override;
    virtual void PlayAnimDeviceSwitch() override;

    void state_Attacking(float dt);

    virtual void KnifeStrike(u32 state, const Fvector& pos, const Fvector& dir);

    float fWallmarkSize;
    u16 knife_material_idx;

    // NLC: blade-line strike volume, low reach and attack buffering (actor only; all keys optional)
    struct NlcStrikeProfile
    {
        bool point{}; // stab: an ellipse around the crosshair, no direction
        float travel{}; // deg on screen the blade travels (0 right, 90 up, 180 left, 270 down)
        float along{}; // deg half-length along the blade line (point: horizontal radius)
        float across{}; // deg half-width across it (point: vertical radius)
        float offset{}; // deg shift of the line across itself
    };
    struct NlcStrikeResult
    {
        Fvector pos, dir;
        float range{};
        float k{1.f};
        bool creature{};
        const char* how{"miss"};
        u16 target{u16(-1)};
        s16 bone{-1};
        float t{}; // normalised graze offset, 0 on the line
    };
    bool m_nlc_volume{};
    float m_nlc_r0{};
    float m_nlc_graze_min{1.f};
    float m_nlc_full_band{};
    float m_nlc_low_reach_start{0.5f};
    float m_nlc_low_reach_max{};
    xr_vector<shared_str> m_nlc_exact_only_bones;
    bool m_nlc_strike_debug{};
    HUD_SOUND m_nlc_snd_hit;
    bool m_nlc_has_snd_hit{};
    u32 m_nlc_buffer_ms{};
    u32 m_nlc_buffer_ttl_ms{600};
    u32 m_nlc_buffered{}; // 0, eFire or eFire2
    u32 m_nlc_buffered_at{};
    bool m_nlc_chain_pending{}; // a buffered attack was started and its state switch is not processed yet

public:
    u32 m_nlc_last_strike_kind{}; // 1 slash, 2 stab
    float m_nlc_last_strike_k{1.f};
    u32 m_nlc_last_strike_time{};

protected:
    bool NlcStrikeProfileFor(u32 state, NlcStrikeProfile& p) const;
    void NlcResolveStrike(u32 state, const Fvector& pos, const Fvector& dir, NlcStrikeResult& res);
    bool NlcBufferPress(u32 state);
    bool NlcConsumeBuffer();

protected:
    ALife::EHitType m_eHitType_1;
    // float				fHitPower_1;
    Fvector4 fvHitPower_1;
    float fHitImpulse_1;

    ALife::EHitType m_eHitType_2;
    // float				fHitPower_2;
    Fvector4 fvHitPower_2;
    float fHitImpulse_2;
    virtual void LoadFireParams(LPCSTR section, LPCSTR prefix);

    virtual size_t GetWeaponTypeForCollision() const override { return Knife_and_other; }
    // TODO: рассчитать здесь позицию для коллизии
    virtual Fvector GetPositionForCollision() override { return Device.vCameraPosition; }
    virtual Fvector GetDirectionForCollision() override { return Device.vCameraDirection; }

public:
    CWeaponKnife();
    virtual ~CWeaponKnife();

    void Load(LPCSTR section);

    virtual void Fire2Start();
    virtual void FireStart();

    virtual bool Action(s32 cmd, u32 flags);

    virtual void GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count);

    DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CWeaponKnife)
#undef script_type_list
#define script_type_list save_type_list(CWeaponKnife)
