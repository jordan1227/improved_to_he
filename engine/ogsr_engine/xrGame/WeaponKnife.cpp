#include "stdafx.h"
#include "WeaponKnife.h"
#include "Entity.h"
#include "Actor.h"
#include "torch.h"
#include "inventory.h"
#include "level.h"
#include "xr_level_controller.h"
#include "../xr_3da/gamemtllib.h"
#include "level_bullet_manager.h"
#include "ai_sounds.h"
#include "game_cl_single.h"
#include "game_object_space.h"
#include "../xr_3da/x_ray.h"
#include "script_game_object.h"
#include "entity_alive.h"
#include "../Include/xrRender/Kinematics.h"

#define KNIFE_MATERIAL_NAME "objects\\knife"

CWeaponKnife::CWeaponKnife() : CWeapon("KNIFE")
{
    m_attackStart = false;
    m_attackMotionMarksAvailable = false;
    SetState(eHidden);
    SetNextState(eHidden);
    knife_material_idx = (u16)-1;
}

CWeaponKnife::~CWeaponKnife()
{
    HUD_SOUND::DestroySound(m_sndShot);
    HUD_SOUND::DestroySound(sndItemOn);
    HUD_SOUND::DestroySound(m_nlc_snd_hit);
}

void CWeaponKnife::Load(LPCSTR section)
{
    // verify class
    inherited::Load(section);

    fWallmarkSize = pSettings->r_float(section, "wm_size");

    HUD_SOUND::LoadSound(section, "snd_shoot", m_sndShot, ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING));

    if (pSettings->line_exist(section, "snd_item_on"))
        HUD_SOUND::LoadSound(section, "snd_item_on", sndItemOn);

    knife_material_idx = GMLib.GetMaterialIdx(KNIFE_MATERIAL_NAME);

    // NLC: strike volume (profiles per HUD key live in the HUD section, see NlcStrikeProfileFor)
    m_nlc_volume = READ_IF_EXISTS(pSettings, r_bool, section, "knife_volume", false);
    m_nlc_r0 = READ_IF_EXISTS(pSettings, r_float, section, "knife_volume_r0", 0.f);
    m_nlc_graze_min = READ_IF_EXISTS(pSettings, r_float, section, "knife_graze_min", 1.f);
    m_nlc_full_band = READ_IF_EXISTS(pSettings, r_float, section, "knife_graze_full_band", 0.f);
    clamp(m_nlc_full_band, 0.f, 0.95f);
    m_nlc_low_reach_start = READ_IF_EXISTS(pSettings, r_float, section, "knife_low_reach_start", 0.5f);
    m_nlc_low_reach_max = READ_IF_EXISTS(pSettings, r_float, section, "knife_low_reach_max", 0.f);
    m_nlc_strike_debug = READ_IF_EXISTS(pSettings, r_bool, section, "knife_strike_debug", false);
    m_nlc_exact_only_bones.clear();
    {
        LPCSTR list = READ_IF_EXISTS(pSettings, r_string, section, "knife_exact_only_bones", "head,jaw,eye,neck");
        string128 item;
        const int n = _GetItemCount(list);
        for (int i = 0; i < n; ++i)
            m_nlc_exact_only_bones.emplace_back(_Trim(_GetItem(list, i, item)));
    }
    m_nlc_has_snd_hit = pSettings->line_exist(section, "snd_hit");
    if (m_nlc_has_snd_hit)
        HUD_SOUND::LoadSound(section, "snd_hit", m_nlc_snd_hit, ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING));

    // NLC: attack buffering (0 = off, the press during a swing is dropped as before)
    m_nlc_buffer_ms = READ_IF_EXISTS(pSettings, r_u32, section, "knife_buffer_ms", 0);
    m_nlc_buffer_ttl_ms = READ_IF_EXISTS(pSettings, r_u32, section, "knife_buffer_ttl_ms", 600);
}

void CWeaponKnife::OnStateSwitch(u32 S, u32 oldState)
{
    inherited::OnStateSwitch(S, oldState);
    if (S != eFire && S != eFire2) // NLC: a buffered attack never outlives the attack chain
    {
        m_nlc_buffered = 0;
        m_nlc_chain_pending = false;
    }
    switch (S)
    {
    case eIdle: switch2_Idle(); break;
    case eShowing: switch2_Showing(); break;
    case eHiding: switch2_Hiding(); break;
    case eHidden: switch2_Hidden(); break;
    case eFire:
    case eFire2: {
        switch2_Attacking(S);
    }
    break;
    case eDeviceSwitch: {
        SetPending(TRUE);
        PlayAnimDeviceSwitch();
    }
    break;
    }
}

void CWeaponKnife::PlayAnimDeviceSwitch()
{
    PlaySound(sndItemOn, Position());

    if (AnimationExist("anm_headlamp_on"))
        PlayHUDMotion("anm_headlamp_on", true, GetState());
    else
    {
        DeviceUpdate();
        SwitchState(eIdle);
    }
}

void CWeaponKnife::KnifeStrike(u32 state, const Fvector& pos, const Fvector& dir)
{
    ALife::EHitType cur_eHitType = ALife::eHitTypeBurn;
    float cur_fHitImpulse = 0;
    float cur_fHit = 0;
    bool apply = false;

    switch (state)
    {
    case eFire: {
        //-------------------------------------------
        cur_eHitType = m_eHitType_1;
        // fHitPower		= fHitPower_1;
        if (ParentIsActor())
        {
            cur_fHit = fvHitPower_1[g_SingleGameDifficulty];
        }
        else
        {
            cur_fHit = fvHitPower_1[egdMaster];
        }
        cur_fHitImpulse = fHitImpulse_1;
        apply = true;
        //-------------------------------------------
    }
    break;
    case eFire2: {
        //-------------------------------------------
        cur_eHitType = m_eHitType_2;
        // fHitPower		= fHitPower_2;
        if (ParentIsActor())
        {
            cur_fHit = fvHitPower_2[g_SingleGameDifficulty];
        }
        else
        {
            cur_fHit = fvHitPower_2[egdMaster];
        }
        cur_fHitImpulse = fHitImpulse_2;
        apply = true;
        //-------------------------------------------
    }
    break;
    }

    if (apply)
    {
        CCartridge cartridge;
        cartridge.m_buckShot = 1;
        cartridge.m_impair = 1;
        cartridge.m_kDisp = 1;
        cartridge.m_kHit = 1;
        cartridge.m_kImpulse = 1;
        cartridge.m_kPierce = 1;
        cartridge.m_flags.set(CCartridge::cfTracer, FALSE);
        cartridge.m_flags.set(CCartridge::cfRicochet, FALSE);
        cartridge.fWallmarkSize = fWallmarkSize;
        cartridge.bullet_material_idx = knife_material_idx;

        while (m_magazine.size() < 2)
            m_magazine.push_back(cartridge);

        iAmmoElapsed = m_magazine.size();

        const bool send_hit = SendHitAllowed(H_Parent());

        PlaySound(m_sndShot, pos);

        if (ParentIsActor() && !fis_zero(conditionDecreasePerShotOnHit) && GetCondition() < 0.95f)
            cur_fHit = cur_fHit * (GetCondition() / 0.95f);

        // NLC: the actor's blade reaches what its strike volume touches; the bullet is only the delivery
        NlcStrikeResult strike;
        strike.pos = pos;
        strike.dir = dir;
        strike.range = fireDistance;
        if (ParentIsActor())
        {
            NlcResolveStrike(state, pos, dir, strike);
            cur_fHit *= strike.k;
            cur_fHitImpulse *= strike.k;
            m_nlc_last_strike_kind = state == eFire2 ? 2 : 1;
            m_nlc_last_strike_k = strike.k;
            m_nlc_last_strike_time = Device.dwTimeGlobal;
            if (strike.creature && m_nlc_has_snd_hit)
                PlaySound(m_nlc_snd_hit, Fvector().mad(strike.pos, strike.dir, strike.range));
        }

        SBullet& bullet = Level().BulletManager().AddBullet(strike.pos, strike.dir, m_fStartBulletSpeed, cur_fHit, cur_fHitImpulse, H_Parent()->ID(), ID(), cur_eHitType,
                                                            strike.range, cartridge, send_hit);
        if (ParentIsActor())
            bullet.setOnBulletHit(true);
    }
}

void CWeaponKnife::OnMotionMark(u32 state, const motion_marks& M)
{
    inherited::OnMotionMark(state, M);

    if (H_Parent())
    {
        Fvector p1, d;
        p1.set(get_LastFP());
        d.set(get_LastFD());
        smart_cast<CEntity*>(H_Parent())->g_fireParams(this, p1, d);
        KnifeStrike(state, p1, d);
    }
}

void CWeaponKnife::OnAnimationEnd(u32 state)
{
    switch (state)
    {
    case eHiding: SwitchState(eHidden); break;
    case eFire:
    case eFire2: {
        u32 time = 0;
        if (m_attackStart)
        {
            m_attackStart = false;
            if (GetState() == eFire)
                time = PlayHUDMotion({"anim_shoot1_end", "anm_attack_end"}, false, state);
            else // eFire2
                time = PlayHUDMotion({"anim_shoot2_end", "anm_attack2_end"}, false, state);

            Fvector p1, d;
            p1.set(get_LastFP());
            d.set(get_LastFD());

            if (H_Parent())
                smart_cast<CEntity*>(H_Parent())->g_fireParams(this, p1, d);
            else
                break;

            if (time != 0 && !m_attackMotionMarksAvailable)
                KnifeStrike(state, p1, d);
        }
        if (time == 0 && !NlcConsumeBuffer())
        {
            SwitchState(eIdle);
        }
    }
    break;
    case eShowing:
    case eDeviceSwitch:
    case eIdle: SwitchState(eIdle); break;
    default: inherited::OnAnimationEnd(state);
    }
}

void CWeaponKnife::state_Attacking(float) {}

void CWeaponKnife::switch2_Attacking(u32 state)
{
    m_nlc_chain_pending = false;
    if (IsPending())
        return;

    if (state == eFire)
        PlayHUDMotion({"anim_shoot1_start", "anm_attack"}, false, state);
    else // eFire2
        PlayHUDMotion({"anim_shoot2_start", "anm_attack2"}, false, state);

    m_attackMotionMarksAvailable = m_current_motion_def && !m_current_motion_def->marks.empty();
    m_attackStart = true;
    SetPending(TRUE);

    if (auto parent = smart_cast<CActor*>(H_Parent()))
    {
        parent->callback(GameObject::eOnActorWeaponFire)(lua_game_object());
    }
}

void CWeaponKnife::switch2_Idle()
{
    PlayAnimIdle();
    SetPending(FALSE);
}

void CWeaponKnife::switch2_Hiding()
{
    FireEnd();
    VERIFY(GetState() == eHiding);
    PlayHUDMotion({"anim_hide", "anm_hide"}, true, GetState());
}

void CWeaponKnife::switch2_Hidden()
{
    signal_HideComplete();
    SetPending(FALSE);
}

void CWeaponKnife::switch2_Showing()
{
    VERIFY(GetState() == eShowing);
    PlayHUDMotion({"anim_draw", "anm_show"}, false, GetState());
}

void CWeaponKnife::FireStart()
{
    inherited::FireStart();
    SwitchState(eFire);
}

void CWeaponKnife::Fire2Start()
{
    inherited::Fire2Start();
    SwitchState(eFire2);

    if (ParentIsActor())
    {
        Actor()->set_state_wishful(Actor()->get_state_wishful() & (~mcSprint));
    }
}

bool CWeaponKnife::Action(s32 cmd, u32 flags)
{
    // NLC: a press during a swing is buffered (or dropped) instead of being lost in CWeapon::Action
    if ((cmd == kWPN_FIRE || cmd == kWPN_ZOOM) && (flags & CMD_START) &&
        (m_nlc_chain_pending || (IsPending() && (GetState() == eFire || GetState() == eFire2))))
    {
        NlcBufferPress(cmd == kWPN_FIRE ? eFire : eFire2);
        return true;
    }
    if (inherited::Action(cmd, flags))
        return true;
    switch (cmd)
    {
    case kWPN_ZOOM:
        if ((flags & CMD_START) && !IsPending()) // NLC: was unguarded and switched the state mid-show/mid-swing
            Fire2Start();
        else if (!(flags & CMD_START))
            Fire2End();
        return true;
    case kTORCH:
    case kTORCH_MODE: {
        auto pActorTorch = smart_cast<CActor*>(H_Parent())->inventory().ItemFromSlot(TORCH_SLOT);
        if ((flags & CMD_START) && pActorTorch && NlcDeviceSwitchReady()) // NLC: also from the sprint transitions
        {
            if (auto torch = smart_cast<CTorch*>(pActorTorch))
                torch->switch_mode = cmd == kTORCH_MODE;
            HeadLampSwitch = true;
            SwitchState(eDeviceSwitch);
            return true;
        }
    }
    break;
    case kNIGHT_VISION: {
        auto pActorNv = nlc_night_vision_item(smart_cast<CActor*>(H_Parent())); // NLC: the NVD slot
        if ((flags & CMD_START) && pActorNv && NlcDeviceSwitchReady() && g_actor && g_actor->can_switch_nv)
        {
            NightVisionSwitch = true;
            SwitchState(eDeviceSwitch);
            return true;
        }
    }
    break;
    }
    return false;
}

void CWeaponKnife::DeviceUpdate()
{
    if (auto pA = smart_cast<CActor*>(H_Parent()))
    {
        if (HeadLampSwitch)
        {
            auto pActorTorch = smart_cast<CTorch*>(pA->inventory().ItemFromSlot(TORCH_SLOT));
            pActorTorch->Switch();
            HeadLampSwitch = false;
        }
        else if (NightVisionSwitch)
        {
            nlc_night_vision_contact(pA); // NLC: also without a torch in TORCH_SLOT
            NightVisionSwitch = false;
        }
    }
}

void CWeaponKnife::UpdateCL()
{
    if (H_Parent() && !ParentIsActor() && (strapped_mode() || IsHidden()))
        return;

    inherited::UpdateCL();
}

void CWeaponKnife::LoadFireParams(LPCSTR section, LPCSTR prefix)
{
    inherited::LoadFireParams(section, prefix);

    string256 full_name;
    string32 buffer;
    shared_str s_sHitPower_2;
    // fHitPower_1		= fHitPower;
    fvHitPower_1 = fvHitPower;
    fHitImpulse_1 = fHitImpulse;
    m_eHitType_1 = ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type"));

    // fHitPower_2			= pSettings->r_float	(section,strconcat(full_name, prefix, "hit_power_2"));
    s_sHitPower_2 = pSettings->r_string_wb(section, strconcat(sizeof(full_name), full_name, prefix, "hit_power_2"));
    fvHitPower_2[egdMaster] = (float)atof(_GetItem(*s_sHitPower_2, 0, buffer)); //первый параметр - это хит для уровня игры мастер

    fvHitPower_2[egdVeteran] = fvHitPower_2[egdMaster]; //изначально параметры для других уровней
    fvHitPower_2[egdStalker] = fvHitPower_2[egdMaster]; //сложности
    fvHitPower_2[egdNovice] = fvHitPower_2[egdMaster]; //такие же

    int num_game_diff_param = _GetItemCount(*s_sHitPower_2); //узнаём колличество параметров для хитов
    if (num_game_diff_param > 1) //если задан второй параметр хита
    {
        fvHitPower_2[egdVeteran] = (float)atof(_GetItem(*s_sHitPower_2, 1, buffer)); //то вычитываем его для уровня ветерана
    }
    if (num_game_diff_param > 2) //если задан третий параметр хита
    {
        fvHitPower_2[egdStalker] = (float)atof(_GetItem(*s_sHitPower_2, 2, buffer)); //то вычитываем его для уровня сталкера
    }
    if (num_game_diff_param > 3) //если задан четвёртый параметр хита
    {
        fvHitPower_2[egdNovice] = (float)atof(_GetItem(*s_sHitPower_2, 3, buffer)); //то вычитываем его для уровня новичка
    }

    fHitImpulse_2 = pSettings->r_float(section, strconcat(sizeof(full_name), full_name, prefix, "hit_impulse_2"));
    m_eHitType_2 = ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type_2"));
}

void CWeaponKnife::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
    str_name = NameShort();
    str_count = "";
    icon_sect_name = *cNameSect();
}

// ---------------------------------------------------------------------------
// NLC: blade-line strike volume
//
// A strike is resolved before its bullet is fired. The exact crosshair ray wins when it reaches a
// living creature. Otherwise sample rays walk the strike profile of the playing HUD key (a blade line
// from its entry end, or rings around the crosshair for a stab); the first living creature reached
// wins, at its sample closest to the line. Graze samples never count on exact-only bones (head) and
// scale the hit by k. The bullet is then fired along the chosen sample ray and cut 5 cm past the
// contact, so it delivers the hit through the normal pipeline and never pierces into a second target.

static bool nlc_knife_static_blocked(const Fvector& start, const Fvector& dir, float range)
{
    xrXRC xrc;
    xrc.ray_query(CDB::OPT_CULL, Level().ObjectSpace.GetStaticModel(), start, dir, range);
    for (CDB::RESULT* r = xrc.r_begin(); r != xrc.r_end(); ++r)
    {
        const CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + r->id;
        const SGameMtl* mtl = GMLib.GetMaterialByIdx(T->material);
        if (!mtl || !fsimilar(mtl->fShootFactor, 1.0f, EPS)) // foliage and other shoot-through faces do not stop the blade
            return true;
    }
    return false;
}

bool CWeaponKnife::NlcStrikeProfileFor(u32 state, NlcStrikeProfile& p) const
{
    const shared_str& hud = HudSection();
    if (!hud.size())
        return false;
    string256 key;
    LPCSTR line = nullptr;
    if (m_current_motion.size())
    {
        xr_strconcat(key, "strike_", m_current_motion.c_str());
        if (pSettings->line_exist(hud, key))
            line = key;
    }
    if (!line)
    {
        xr_strcpy(key, state == eFire2 ? "strike_anm_attack2" : "strike_anm_attack");
        if (!pSettings->line_exist(hud, key))
            return false;
        line = key;
    }
    LPCSTR value = pSettings->r_string(hud, line);
    string32 buf;
    const int n = _GetItemCount(value);
    if (n < 3)
    {
        Msg("! [knife_strike] [%s] %s needs travel_deg, along_deg, across_deg", hud.c_str(), line);
        return false;
    }
    p.travel = (float)atof(_GetItem(value, 0, buf));
    p.along = (float)atof(_GetItem(value, 1, buf));
    p.across = (float)atof(_GetItem(value, 2, buf));
    p.offset = n > 3 ? (float)atof(_GetItem(value, 3, buf)) : 0.f;
    p.point = p.travel < 0.f;
    return p.along > 0.f || p.across > 0.f;
}

void CWeaponKnife::NlcResolveStrike(u32 state, const Fvector& pos, const Fvector& dir, NlcStrikeResult& res)
{
    const CObject* parent = H_Parent();

    // low reach: a strike aimed down reaches further, so creatures below eye level are in range
    auto reach = [&](const Fvector& d) {
        float extra = -d.y * fireDistance - m_nlc_low_reach_start;
        clamp(extra, 0.f, m_nlc_low_reach_max);
        return fireDistance + extra;
    };
    res.pos = pos;
    res.dir = dir;
    res.range = reach(dir);

    auto bone_name = [](CObject* O, s16 element) -> LPCSTR {
        IKinematics* K = O ? smart_cast<IKinematics*>(O->Visual()) : nullptr;
        return (K && element >= 0 && u16(element) < K->LL_BoneCount()) ? K->LL_BoneName(u16(element)) : "-";
    };
    auto take = [&](const Fvector& s, const Fvector& d, const collide::rq_result& R, float t, const char* how) {
        res.pos = s;
        res.dir = d;
        res.range = R.range + 0.05f;
        res.t = t;
        res.k = 1.f;
        if (t > m_nlc_full_band)
            res.k = 1.f + (m_nlc_graze_min - 1.f) * ((t - m_nlc_full_band) / (1.f - m_nlc_full_band));
        res.creature = smart_cast<CEntityAlive*>(R.O) != nullptr;
        res.how = how;
        res.target = R.O->ID();
        res.bone = s16(R.element);
    };

    // the exact crosshair ray
    collide::rq_result exact;
    bool exact_hit = false;
    if (Level().ObjectSpace.RayPick(pos, dir, res.range, collide::rqtObject, exact, parent) && exact.O && !nlc_knife_static_blocked(pos, dir, exact.range))
    {
        exact_hit = true;
        const CEntityAlive* E = smart_cast<CEntityAlive*>(exact.O);
        if (E && E->g_Alive())
            take(pos, dir, exact, 0.f, "exact");
    }

    NlcStrikeProfile p;
    const bool sweep = !res.creature && m_nlc_volume && NlcStrikeProfileFor(state, p);
    if (sweep)
    {
        Fvector right, top;
        right.crossproduct(Fvector().set(0.f, 1.f, 0.f), dir);
        if (right.square_magnitude() < EPS)
            right.set(Device.vCameraRight);
        right.normalize();
        top.crossproduct(dir, right);
        top.normalize();

        // screen axes of the profile: t along the blade travel, n across it
        const float th = deg2rad(p.travel);
        const float tx = _cos(th), ty = _sin(th), nx = -ty, ny = tx;

        struct Sample
        {
            float ax, ay, lx, ly, t;
        };
        xr_vector<Sample> samples;
        if (p.point)
        {
            for (const float r : {0.5f, 1.f})
                for (int i = 0; i < 8; ++i)
                {
                    const float a = PI_MUL_2 * float(i) / 8.f;
                    samples.push_back({r * p.along * _cos(a), r * p.across * _sin(a), m_nlc_r0 * r * _cos(a), m_nlc_r0 * r * _sin(a), r});
                }
        }
        else
        {
            // entry end first (u = -1 is where the blade comes from); per step the line itself, then outwards
            for (int i = 0; i <= 8; ++i)
            {
                const float u = -1.f + 0.25f * float(i);
                for (const float w : {0.f, -0.5f, 0.5f, -1.f, 1.f})
                {
                    const float c = w * p.across + p.offset;
                    samples.push_back({u * p.along * tx + c * nx, u * p.along * ty + c * ny, m_nlc_r0 * w * nx, m_nlc_r0 * w * ny, _abs(w)});
                }
            }
        }

        struct Best
        {
            bool found{};
            u16 id{u16(-1)};
            Fvector s, d;
            collide::rq_result R;
            float t{};
        } alive, dead;
        for (const Sample& smp : samples)
        {
            Fvector d, s;
            d.set(dir).mad(right, tanf(deg2rad(smp.ax))).mad(top, tanf(deg2rad(smp.ay))).normalize();
            s.set(pos).mad(right, smp.lx).mad(top, smp.ly);
            collide::rq_result R;
            if (!Level().ObjectSpace.RayPick(s, d, reach(d), collide::rqtObject, R, parent) || !R.O)
                continue;
            const CEntityAlive* E = smart_cast<CEntityAlive*>(R.O);
            if (!E) // props and other objects block the sample
                continue;
            LPCSTR bn = bone_name(R.O, s16(R.element));
            bool exact_only = false;
            for (const shared_str& token : m_nlc_exact_only_bones)
                if (token.size() && strstr(bn, token.c_str()))
                    exact_only = true;
            if (exact_only || nlc_knife_static_blocked(s, d, R.range))
                continue;
            Best& b = E->g_Alive() ? alive : dead;
            if (b.found && (b.id != R.O->ID() || smp.t >= b.t))
                continue; // the first creature reached stays the target; only a closer sample on it improves
            b.found = true;
            b.id = R.O->ID();
            b.s = s;
            b.d = d;
            b.R = R;
            b.t = smp.t;
        }
        if (alive.found)
            take(alive.s, alive.d, alive.R, alive.t, "graze");
        else if (exact_hit)
            take(pos, dir, exact, 0.f, "exact_obj");
        else if (dead.found)
            take(dead.s, dead.d, dead.R, dead.t, "graze_dead");
    }
    else if (!res.creature && exact_hit)
        take(pos, dir, exact, 0.f, "exact_obj");

    static u32 logged = 0;
    if (m_nlc_strike_debug || logged < 16)
    {
        ++logged;
        CObject* T = res.target != u16(-1) ? Level().Objects.net_Find(res.target) : nullptr;
        Msg("~ [knife_strike] %s state=%s key=%s profile=%s target=%s bone=%s t=%.2f k=%.2f range=%.2f", res.how, state == eFire2 ? "stab" : "slash",
            m_current_motion.size() ? m_current_motion.c_str() : "-", sweep ? (p.point ? "point" : "line") : "none", T ? T->cName().c_str() : "-",
            bone_name(T, res.bone), res.t, res.k, res.range);
    }
}

// ---------------------------------------------------------------------------
// NLC: attack buffering

bool CWeaponKnife::NlcBufferPress(u32 state)
{
    const u32 now = Device.dwTimeGlobal;
    const u32 left = m_dwMotionEndTm > now ? m_dwMotionEndTm - now : 0;
    static u32 logged = 0;
    const bool log = m_nlc_strike_debug || logged < 24;
    if (!m_nlc_buffer_ms || (!m_nlc_chain_pending && left > m_nlc_buffer_ms))
    {
        if (log && m_nlc_buffer_ms)
        {
            ++logged;
            Msg("~ [knife_buffer] ignored_early %s ms_to_end=%u", state == eFire2 ? "stab" : "slash", left);
        }
        return false;
    }
    m_nlc_buffered = state;
    m_nlc_buffered_at = now;
    if (log)
    {
        ++logged;
        Msg("~ [knife_buffer] stored %s ms_to_end=%u", state == eFire2 ? "stab" : "slash", left);
    }
    return true;
}

bool CWeaponKnife::NlcConsumeBuffer()
{
    const u32 s = m_nlc_buffered;
    m_nlc_buffered = 0;
    if (!s)
        return false;
    const u32 age = Device.dwTimeGlobal - m_nlc_buffered_at;
    if (!ParentIsActor() || age > m_nlc_buffer_ttl_ms)
    {
        Msg("~ [knife_buffer] expired age=%u", age);
        return false;
    }
    SetPending(FALSE);
    m_nlc_chain_pending = true;
    if (s == eFire2)
        Fire2Start();
    else
        FireStart();
    if (m_nlc_strike_debug)
        Msg("~ [knife_buffer] consumed %s age=%u", s == eFire2 ? "stab" : "slash", age);
    return true;
}

// Lua (level.nlc_knife_strike_kind / _k): the last strike of a knife within 1.5 s, for hit callbacks
static CWeaponKnife* nlc_recent_knife(u16 weapon_id)
{
    CWeaponKnife* K = smart_cast<CWeaponKnife*>(Level().Objects.net_Find(weapon_id));
    return (K && K->m_nlc_last_strike_kind && Device.dwTimeGlobal - K->m_nlc_last_strike_time <= 1500) ? K : nullptr;
}

int nlc_knife_strike_kind(u16 weapon_id)
{
    const CWeaponKnife* K = nlc_recent_knife(weapon_id);
    return K ? int(K->m_nlc_last_strike_kind) : 0;
}

float nlc_knife_strike_k(u16 weapon_id)
{
    const CWeaponKnife* K = nlc_recent_knife(weapon_id);
    return K ? K->m_nlc_last_strike_k : 0.f;
}
