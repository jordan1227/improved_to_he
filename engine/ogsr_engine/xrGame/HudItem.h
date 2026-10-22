//////////////////////////////////////////////////////////////////////
// HudItem.h: класс предок для всех предметов имеющих
//			  собственный HUD (CWeapon, CMissile etc)
//////////////////////////////////////////////////////////////////////

#pragma once

class CSE_Abstract;
class CPhysicItem;
class NET_Packet;
struct HUD_SOUND;
class CInventoryItem;
struct attachable_hud_item;
class motion_marks;
class CMotionDef;

#include "actor_defs.h"

class CHUDState
{
public:
    enum EHudStates
    {
        eIdle,
        eFire,
        eFire2,
        eReload,
        eShowing,
        eHiding,
        eHidden,
        eMisfire,
        eMagEmpty,
        eSwitch,
        eSprintStart,
        eSprintEnd,
        eBore,
        eDeviceSwitch,
        eThrowStart,
        eReady,
        eThrow,
        eThrowEnd,
        eSwitchFiremode,
    };

private:
    u32 m_hud_item_state;
    u32 m_nextState;
    u32 m_dw_curr_state_time;

protected:
    u32 m_dw_curr_substate_time;

public:
    CHUDState() { SetState(eHidden); }
    IC u32 GetNextState() const { return m_nextState; }
    IC u32 GetState() const { return m_hud_item_state; }
    IC void SetState(u32 v)
    {
        m_hud_item_state = v;
        m_dw_curr_state_time = Device.dwTimeGlobal;
        ResetSubStateTime();
    }
    IC void SetNextState(u32 v) { m_nextState = v; }
    IC u32 CurrStateTime() const { return Device.dwTimeGlobal - m_dw_curr_state_time; }
    IC void ResetSubStateTime() { m_dw_curr_substate_time = Device.dwTimeGlobal; }
    virtual void SwitchState(u32 S) = 0;
    virtual void OnStateSwitch(u32 S, u32 oldState) = 0;
};

class CHudItem : public CHUDState
{
protected: //чтоб нельзя было вызвать на прямую
    CHudItem();
    virtual ~CHudItem() = default;
    virtual DLL_Pure* _construct();

    Flags16 m_huditem_flags;
    enum
    {
        fl_pending = (1 << 0),
        // = (1 << 1),
        fl_inertion_enable = (1 << 2),
        fl_inertion_allow = (1 << 3),
        fl_bobbing_allow = (1 << 4),
    };

    // Motion data
    const CMotionDef* m_current_motion_def;
    shared_str m_current_motion;
    u32 m_dwMotionCurrTm;
    u32 m_dwMotionStartTm;
    u32 m_dwMotionEndTm;
    u32 m_startedMotionState;

    bool m_bStopAtEndAnimIsRunning{};
    bool SprintType{};
    bool BobbingEnable{};
    u32 m_dwStateTime{};

public:
    virtual void Load(LPCSTR section);
    virtual CHudItem* cast_hud_item() { return this; }

    virtual void PlaySound(HUD_SOUND& snd, const Fvector& position, bool overlap = false);

    ///////////////////////////////////////////////
    // общие функции HUD
    ///////////////////////////////////////////////
    virtual void StopHUDSounds(){};

    //для предачи команд владельцем
    virtual bool Action(s32 cmd, u32 flags);

    virtual void OnDrawUI(){};

    virtual bool IsHidden() const { return GetState() == eHidden; } // Does weapon is in hidden state
    virtual bool IsHiding() const { return GetState() == eHiding; }
    virtual bool IsShowing() const { return GetState() == eShowing; }
    //посылка сообщения на сервер о смене состояния оружия
    virtual void SwitchState(u32 S);
    //прием сообщения с сервера и его обработка
    virtual void OnStateSwitch(u32 S, u32 oldState);
    virtual void OnEvent(NET_Packet& P, u16 type);

    virtual void OnH_A_Chield();
    virtual void OnH_B_Chield();
    virtual void OnH_B_Independent(bool just_before_destroy);
    virtual void OnH_A_Independent();

    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Destroy();

    virtual bool Activate(bool = false);
    virtual void Deactivate(bool = false);

    virtual void OnActiveItem(){};
    virtual void OnHiddenItem(){};

    virtual void OnAnimationEnd(u32 state);
    virtual void OnMotionMark(u32 state, const motion_marks&){};
    virtual void OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd);

    virtual void PlayAnimIdle();
    bool TryPlayAnimIdle();
    virtual bool IsZoomed() const { return false; }
    // virtual void	PlayAnimBore		();
    virtual void PlayAnimIdleMoving();
    virtual void PlayAnimIdleMovingSlow();
    virtual void PlayAnimIdleSprint();
    virtual void PlayAnimSprintStart();
    virtual void PlayAnimSprintEnd();
    virtual void PlayAnimIdleMovingCrouch();
    virtual void PlayAnimIdleMovingCrouchSlow();
    virtual void PlayAnimDeviceSwitch(){};

    virtual bool NeedBlendAnm();

    virtual void UpdateCL();
    virtual void renderable_Render(u32 context_id, IRenderable* root);

    virtual void Hide(bool = false) {}
    virtual void Show(bool = false) {}

    virtual void UpdateXForm() = 0;

    u32 PlayHUDMotion(std::initializer_list<const char*>, const bool bMixIn, const u32 state, const bool randomAnim = true, float speed = 1.f);
    u32 PlayHUDMotion(const char* M, const bool bMixIn, const u32 state, const bool randomAnim = true, float speed = 1.f);
    u32 PlayHUDMotion_base(const char* M, const bool bMixIn, const u32 state, const bool randomAnim, float speed);
    u32 PlayHUDMotion_noCB(const shared_str& M, const bool bMixIn, const bool randomAnim = true, float speed = 1.f);

    bool AnimationExist(const char* M) const;
    bool AddSuffixName(shared_str& anim, LPCSTR suffix, LPCSTR suffix2 = "") const;
    void StopCurrentAnimWithoutCallback();

    attachable_hud_item* HudItemData() const;
    bool used_cop_fire_point() const;
    virtual void on_a_hud_attach();
    virtual void on_b_hud_detach();
    const shared_str& HudSection() const { return hud_sect; }

    BOOL GetHUDmode();
    IC void SetPending(BOOL H) { m_huditem_flags.set(fl_pending, H); }
    IC BOOL IsPending() const { return !!m_huditem_flags.test(fl_pending); }

    void PlayBlendAnm(LPCSTR name, float speed = 1.f, float power = 1.f, bool stop_old = true);

    virtual void render_hud_mode(u32 context_id, IRenderable* root){};
    virtual bool need_renderable() { return true; };
    virtual void render_item_3d_ui() {}
    virtual bool render_item_3d_ui_query() { return false; }
    virtual bool CheckCompatibility(CHudItem*) { return true; }

protected:
    BOOL hud_mode;
    shared_str hud_sect;

    //кадры момента пересчета XFORM и FirePos
    u32 dwFP_Frame;
    u32 dwXF_Frame;

protected:
    u32 m_animation_slot;

public:
    IC u32 animation_slot() { return m_animation_slot; }

private:
    CPhysicItem* m_object;
    CInventoryItem* m_item;

public:
    IC CPhysicItem& object() const
    {
        VERIFY(m_object);
        return (*m_object);
    }
    IC CInventoryItem& item() const
    {
        VERIFY(m_item);
        return (*m_item);
    }

    virtual void on_renderable_Render(u32 context_id, IRenderable* root) = 0;

public:
    class CWeaponBobbing
    {
    public:
        CWeaponBobbing() = delete;
        CWeaponBobbing(CHudItem* parent);
        ~CWeaponBobbing() = default;

        void Update(Fmatrix& m, Fmatrix& m2);
        void CheckState();

    private:
        CHudItem* parent_hud_item;
        float fTime;
        Fvector vAngleAmplitude;
        float fYAmplitude;
        float fSpeed;

        u32 dwMState;
        float fReminderFactor;
        u32 m_dwLastUpdateFrame{}; // NLC: detects a pause in Update() (ADS-only bobbing)
        bool is_limping;
        bool m_bZoomMode;

        float m_fAmplitudeController;
        float m_fAmplitudeRun;
        float m_fAmplitudeWalk;
        float m_fAmplitudeLimp;

        float m_fSpeedRun;
        float m_fSpeedWalk;
        float m_fSpeedLimp;

        float m_fCrouchFactor;
        float m_fZoomFactor;
        float m_fScopeZoomFactor;
    };
    std::unique_ptr<CWeaponBobbing> m_bobbing;

    virtual u8 GetCurrentHudOffsetIdx() const { return 0; }
    virtual float GetHudFov(); // Получить HUD FOV от текущего оружия игрока
    void UpdateHudAdditional(Fmatrix&, const bool need_update_collision = false);
    bool HudBobbingAllowed() const { return m_huditem_flags.test(fl_bobbing_allow); }
    bool AnmIdleMovingAllowed() const;
    void AllowHudBobbing(BOOL B) { m_huditem_flags.set(fl_bobbing_allow, B); }
    void GetBoneOffsetPosDir(const shared_str& bone_name, Fvector& dest_pos, Fvector& dest_dir, const Fvector& offset);
    //Функция из ганслингера для приблизительной коррекции разности фовов худа и мира. Так себе на самом деле, но более годных способов я не нашел.
    void CorrectDirFromWorldToHud(Fvector& dir);
    float GetLastHudFov() const { return m_nearwall_last_hud_fov; }

    // NLC: gunplay feel (nlc_gunplay.h, docs/GUNPLAY_FEEL_DESIGN.md). Read from the item section on Load;
    // Lua may overwrite them per item (sivol_weapon_aim_speed derives them from weapon handling).
    float m_nlc_motion_freq{}; // hud_motion_freq: jump/fall/landing spring, rad/s; 0 = original lerps
    float m_nlc_motion_damping{}; // hud_motion_damping
    float m_nlc_landing_k{}; // hud_landing_k: landing kick scale (1 = authored peak at a 5.5 m/s landing)
    float m_nlc_rot_inertia_k{}; // hud_rot_inertia_k: seconds of rotational lag behind mouse turns; 0 = off
    float m_nlc_rot_inertia_freq{}; // hud_rot_inertia_freq
    float m_nlc_rot_inertia_aim_k{}; // hud_rot_inertia_aim_k: share kept while aiming
    float m_nlc_rot_inertia_roll{}; // hud_rot_inertia_roll: roll per unit of yaw lag (sign = direction)
    float m_nlc_move_inertia_k{}; // hud_move_inertia_k: metres of lag per m/s of velocity change; 0 = off
    float m_nlc_move_inertia_freq{}; // hud_move_inertia_freq
    float m_nlc_move_inertia_aim_k{}; // hud_move_inertia_aim_k
    float m_nlc_crouch_kick_k{}; // hud_crouch_kick_k: power of the engine-played crouch clips (nlc_gunplay.set_crouch_clips); 0 = off
    float m_nlc_clip_speed_k{}; // hud_clip_speed_k: playback speed of those clips for this item
    float m_nlc_rot_inertia_pivot_z{}; // hud_rot_inertia_pivot_z: metres ahead of the eye the lag pivots around (stock stays planted)
    float m_nlc_rot_inertia_shift{}; // hud_rot_inertia_shift: share of the muzzle swing the whole rig follows
    float m_nlc_ads_damping{}; // hud_ads_damping: settle into the sights (lower = more overshoot)
    float m_nlc_lookup_k{}; // hud_lookup_k: dip when looking up (replaces the script look-up .anm); 0 = off
    float NlcGetInertion(int i) const;
    void NlcSetInertion(int i, float v);

protected:
    enum CollisionWeaponType : size_t
    {
        CWeaponBase,
        Pistol,
        SniperRifle,
        RPG,
        RG_6,
        Groza,
        FN2000,
        BM_16,
        Bolt,
        Detector,
        Knife_and_other,
        Binocular,
        _CollisionWeaponTypesCount_
    };
    virtual size_t GetWeaponTypeForCollision() const { return Knife_and_other; }
    virtual Fvector GetPositionForCollision() { return Device.vCameraPosition; }
    virtual Fvector GetDirectionForCollision() { return Device.vCameraDirection; }
    float m_fZoomRotationFactor{}; //от 0 до 1, показывает насколько процентов мы перемещаем HUD
    float m_fZoomRotateTime{}; //время приближения
    //bool is_second_zoom_offset_enabled{};
    //bool AimAlt{};
    u32 skip_updated_frame{};
    bool HudInertionAllowed() const { return m_huditem_flags.test(fl_inertion_allow); }
    void AllowHudInertion(BOOL B) { m_huditem_flags.set(fl_inertion_allow, B); }
    void TimeLockAnimation();
    virtual void DeviceUpdate(){};

private:
    shared_str world_sect;
    float hud_recalc_koef{};
    void UpdateCollision(Fmatrix& trans);
    bool CollisionAllowed() const;
    bool m_nearwall_on{};
    float m_nearwall_target_hud_fov{}, m_nearwall_target_aim_hud_fov{};
    float m_nearwall_dist_max{}, m_nearwall_dist_min{};
    float m_nearwall_last_hud_fov{};
    float m_nearwall_speed_mod{}, m_nearwall_hud_offset_speed{};
    Fvector m_nearwall_target_hud_offset{}, m_nearwall_target_hud_rotate{};
    float saved_rq_range{};
    Fvector m_nearwall_last_pos{}, m_nearwall_last_rot{};

    Fvector m_strafe_offset[3][2]{}, m_lookout_offset[3][2]{}, m_jump_offset[3][2]{}, m_fall_offset[2][2]{}, m_landing_offset[2][2]{}, m_move_offset[3]{}, m_walk_offset[3]{};
    Fvector current_difference[2]{}, current_strafe[2]{}, current_lookout[2]{}, current_jump[2]{}, current_move[2]{}, current_walk[2]{};

    float m_base_fov{};

    bool allow_bobbing{true};

protected:
    bool allow_drum_anims{};
    bool m_bUseAltAimZoom{};

private:

    struct inertion_params
    {
        float m_pitch_offset_r;
        float m_pitch_offset_n;
        float m_pitch_offset_d;
        float m_pitch_low_limit;
        // отклонение модели от "курса" из за инерции во время движения
        float m_origin_offset;
        // отклонение модели от "курса" из за инерции во время движения с прицеливанием
        float m_origin_offset_aim;
        // скорость возврата худ модели в нужное положение
        float m_tendto_speed;
        // скорость возврата худ модели в нужное положение во время прицеливания
        float m_tendto_speed_aim;
    } inertion_data{}; //--#SM+#--
    Fvector inert_st_last_dir{};
    void UpdateInertion(Fmatrix& trans);

    // NLC: gunplay feel state
    Fvector m_nlc_landing_shape[2][2]{}; // [pos/rot][hip/aim]: peak displacement of a reference landing
    Fvector m_nlc_spring_state[8]{}; // jump pos x/v, jump rot x/v, rot inertia x/v, move inertia x/v
    Fvector m_nlc_vel_filtered{};
    u32 m_nlc_land_seen{};
    u32 m_nlc_last_update{};
    u32 m_nlc_last_frame{u32(-1)};
    Fvector m_nlc_jump_out[2]{}, m_nlc_inertia_out[2]{};
    Fvector m_nlc_prev_hp{};
    Fvector m_nlc_blk_v[4][2]{}; // spring velocities of the strafe, lookout, stance and walk offsets
    Fvector m_nlc_ads_v[2]{}, m_nlc_wall_v[2]{}; // spring velocities of the ADS pose and the near-wall pose
    float m_nlc_pda_fade{1.f};
    bool m_nlc_step_now{};
    float NlcItemK(bool rotation) const; // PDA / grenade share of the motion layers
    Fvector m_nlc_lookup_offset[2]{}; // lookup_hud_offset_pos/rot: pose when looking straight up
    float NlcBlockOmega(float transition_time) const;
    void NlcBlend(int block, Fvector* cur, const Fvector& tp, const Fvector& tr, float step_lerp, float transition_time, bool step);
    void NlcUpdateJumpSpring(u32 moving_state, u8 idx, bool step, Fvector& summary_offset, Fvector& summary_rotate);
    void NlcUpdateInertia(const class CActor* actor, bool aiming, bool step, Fvector& summary_offset, Fvector& summary_rotate);
    float GetInertionFactor() const { return 1.f; } //--#SM+#--
    float GetInertionPowerFactor() const { return 0.5f; } //--#SM+#--
    bool HudInertionEnabled() const { return m_huditem_flags.test(fl_inertion_enable); }
    void EnableHudInertion(BOOL B) { m_huditem_flags.set(fl_inertion_enable, B); }
};