#include "stdafx.h"
#include "fl_hook.h"
#include "HUDManager.h"
#include "hudtarget.h"

#include "actor.h"
#include "..\xr_3da\igame_level.h"
#include "clsid_game.h"
#include "Car.h"
// #include "Flashlight.h"
#include "inventory.h"
#include "torch.h"
#include "weapon.h"
#include "ui/UIMessagesWindow.h"
#include "game_cl_single.h"
#include "../Include/xrRender/Kinematics.h" // NLC: actor leg bones follow the visible body (shadow)

CFontManager::CFontManager()
{
    Device.seqDeviceReset.Add(this, REG_PRIORITY_HIGH);

    InitializeFonts();
}

void CFontManager::InitializeFonts()
{
    //pFontMedium = nullptr;
    //pFontSmall = nullptr;

    //pFontDI = nullptr;

    //pFontArial14 = nullptr;
    //pFontArial21 = nullptr;
    //pFontGraffiti19Russian = nullptr;
    //pFontGraffiti22Russian = nullptr;
    //pFontLetterica16Russian = nullptr;
    //pFontLetterica18Russian = nullptr;
    //pFontGraffiti32Russian = nullptr;
    //pFontGraffiti40Russian = nullptr;
    //pFontGraffiti50Russian = nullptr;
    //pFontLetterica25 = nullptr;

    //delete_data(m_all_fonts);

    //m_all_fonts.clear();

    InitializeFont(pFontMedium, "hud_font_medium");
    InitializeFont(pFontSmall, "hud_font_small");

    InitializeFont(pFontDI, "stat_font", CGameFont::fsGradient | CGameFont::fsDeviceIndependent);

    InitializeFont(pFontArial14, "ui_font_arial_14");
    InitializeFont(pFontArial21, "ui_font_arial_21");

    InitializeFont(pFontGraffiti19Russian, "ui_font_graffiti19_russian");
    InitializeFont(pFontGraffiti22Russian, "ui_font_graffiti22_russian");

    InitializeFont(pFontLetterica16Russian, "ui_font_letterica16_russian");
    InitializeFont(pFontLetterica18Russian, "ui_font_letterica18_russian");

    InitializeFont(pFontGraffiti32Russian, "ui_font_graff_32");
    InitializeFont(pFontGraffiti40Russian, "ui_font_graff_40");
    InitializeFont(pFontGraffiti50Russian, "ui_font_graff_50");

    InitializeFont(pFontLetterica25, "ui_font_letter_25");

    for (auto& font : m_all_fonts)
    {
        if (font->m_bCustom)
            InitializeFont(font, font->m_font_name.c_str());
    }
}

LPCSTR CFontManager::GetFontTexName(LPCSTR section)
{
    constexpr const char* tex_names[] = {"texture800", "texture", "texture1600"};
    int def_idx = 1; // default 1024x768
    int idx = def_idx;

    u32 h = Device.dwHeight;

    if (h <= 600)
        idx = 0;
    else if (h < 1024)
        idx = 1;
    else
        idx = 2;

    while (idx >= 0)
    {
        if (pSettings->line_exist(section, tex_names[idx]))
            return pSettings->r_string(section, tex_names[idx]);
        --idx;
    }
    return pSettings->r_string(section, tex_names[def_idx]);
}

void CFontManager::InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
    LPCSTR font_tex_name = GetFontTexName(section);
    R_ASSERT(font_tex_name);

    const char* sh_name = READ_IF_EXISTS(pSettings, r_string, section, "shader", "font");
    if (!F)
    {
        F = xr_new<CGameFont>(sh_name, font_tex_name, section, flags);
        m_all_fonts.push_back(F);
    }
    else
        F->Initialize(sh_name, font_tex_name, section);

    F->m_font_name = section;

    if (!(flags & CGameFont::fsDeviceIndependent))
    {
        if (pSettings->line_exist(section, "scale_x"))
        {
            F->SetWidthScale(pSettings->r_float(section, "scale_x"));
        }
        if (pSettings->line_exist(section, "scale_y"))
        {
            F->SetHeightScale(pSettings->r_float(section, "scale_y"));
        }
    }

    if (pSettings->line_exist(section, "size"))
    {
        float sz = pSettings->r_float(section, "size");
        if (flags & CGameFont::fsDeviceIndependent)
            F->SetHeightI(sz);
        else
            F->SetHeight(sz);
    }
    if (pSettings->line_exist(section, "interval"))
        F->SetInterval(pSettings->r_fvector2(section, "interval"));
}

CGameFont* CFontManager::InitializeCustomFont(LPCSTR section, u32 flags)
{
    for (auto& font : m_all_fonts)
    {
        if (strstr(font->m_font_name.c_str(), section))
            return font;
    }

    CGameFont* pFontAdd = NULL;
    InitializeFont(pFontAdd, section, flags);
    pFontAdd->m_bCustom = true;
    return pFontAdd;
}

CFontManager::~CFontManager()
{
    Device.seqDeviceReset.Remove(this);

    pFontMedium = nullptr;
    pFontSmall = nullptr;

    pFontDI = nullptr;

    pFontArial14 = nullptr;
    pFontArial21 = nullptr;
    pFontGraffiti19Russian = nullptr;
    pFontGraffiti22Russian = nullptr;
    pFontLetterica16Russian = nullptr;
    pFontLetterica18Russian = nullptr;
    pFontGraffiti32Russian = nullptr;
    pFontGraffiti40Russian = nullptr;
    pFontGraffiti50Russian = nullptr;
    pFontLetterica25 = nullptr;

    delete_data(m_all_fonts);

    m_all_fonts.clear();
}

void CFontManager::Render()
{
    for (auto& font : m_all_fonts)
    {
        font->OnRender();
    }
}

void CFontManager::OnDeviceReset() { InitializeFonts(); }

//--------------------------------------------------------------------
CHUDManager::CHUDManager()
{
    pUI = 0;
    m_pHUDTarget = xr_new<CHUDTarget>();
    OnDisconnected();
}
//--------------------------------------------------------------------
CHUDManager::~CHUDManager()
{
    xr_delete(pUI);
    xr_delete(m_pHUDTarget);
    b_online = false;
}

//--------------------------------------------------------------------

void CHUDManager::Load()
{
    if (pUI)
    {
        pUI->Load(pUI->UIGame());
        return;
    }
    pUI = xr_new<CUI>(this);
    pUI->Load(NULL);
    OnDisconnected();
}
//--------------------------------------------------------------------
void CHUDManager::OnFrame()
{
    if (!b_online)
        return;
    if (pUI)
        pUI->UIOnFrame();
    m_pHUDTarget->CursorOnFrame();
}
//--------------------------------------------------------------------

bool need_render_hud()
{
    CObject* O = (g_pGameLevel) ? g_pGameLevel->CurrentViewEntity() : nullptr;
    if (!O)
        return false;

    CActor* A = smart_cast<CActor*>(O);
    if (A && (!A->HUDview() || !A->g_Alive()))
        return false;

    if (smart_cast<CCar*>(O))
        return false;

    return true;
}

void CHUDManager::Render_SMAP(u32 context_id) { Render_Actor_Shadow(context_id); }

void CHUDManager::Render_MAIN(u32 context_id)
{
    if (nullptr == pUI)
        return;

    if (!need_render_hud())
        return;

    ZoneScoped;

    std::scoped_lock slock(render_lock);

    // hud itself
    {
        CObject* O = g_pGameLevel->CurrentViewEntity();
        const auto root = O->H_Root();
        root->renderable_HUD(true);
        O->OnHUDDraw(this, context_id, root);
        root->renderable_HUD(false);
    }

    // Render_Actor_FirstPersonBody(context_id);
}

// NLC: the actor model's shadow (g_actor_shadow) is the only one cast (the visible-body puppet never reaches the
// shadow maps) and the model has another animation set than the legs the player sees, so the shadow desynced from
// them. While the puppet is shown, the actor model's leg bones take the puppet's current leg rotation (relative to
// the parent bone; the actor keeps its own bone offsets so the mesh is not stretched, the first model-space version
// stretched the boot soles), so the shadow is the actor's torso/arms/weapon on the same legs. The script reports the puppet through
// level.nlc_visible_body_active(active, id); g_actor_shadow_legs_from_body 0 switches this off.
static bool g_nlc_visible_body_active = false;
static u16 g_nlc_visible_body_id = u16(-1);
int g_nlc_shadow_legs_from_body = 1;

namespace
{
struct SNlcLegBone
{
    LPCSTR name;
    u16 actor_id;
    u16 puppet_id;
    IKinematics* puppet_kin;
    u16 actor_parent;
    u16 puppet_parent;
};

SNlcLegBone g_nlc_leg_bones[] = {
    {"bip01_l_thigh", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE}, {"bip01_l_calf", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE}, {"bip01_l_foot", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE},
    {"bip01_l_toe0", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE},  {"bip01_r_thigh", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE}, {"bip01_r_calf", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE},
    {"bip01_r_foot", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE},  {"bip01_r_toe0", BI_NONE, BI_NONE, nullptr, BI_NONE, BI_NONE},
};
IKinematics* g_nlc_leg_actor_kin = nullptr; // the actor skeleton the callbacks are installed on

void nlc_leg_bone_callback(CBoneInstance* B)
{
    if (!g_nlc_visible_body_active)
        return;
    SNlcLegBone* leg = static_cast<SNlcLegBone*>(B->callback_param());
    CObject* puppet = g_pGameLevel ? g_pGameLevel->Objects.net_Find(g_nlc_visible_body_id) : nullptr;
    IKinematics* pk = puppet ? PKinematics(puppet->Visual()) : nullptr;
    if (!leg || !pk)
        return;
    if (leg->puppet_kin != pk)
    {
        leg->puppet_kin = pk;
        leg->puppet_id = pk->LL_BoneID(leg->name);
        leg->puppet_parent = leg->puppet_id != BI_NONE ? pk->GetBoneData(leg->puppet_id).GetParentID() : BI_NONE;
    }
    IKinematics* ak = g_nlc_leg_actor_kin;
    if (!ak || leg->puppet_id == BI_NONE || leg->puppet_parent == BI_NONE || leg->actor_parent == BI_NONE)
        return;
    pk->CalculateBones(TRUE); // same-ms calls return at once

    // puppet bone rotation relative to its parent, with the actor bone's own local offset
    Fmatrix puppet_parent_inv, actor_parent_inv, puppet_local, actor_local;
    if (!puppet_parent_inv.invert_b(pk->LL_GetBoneInstance(leg->puppet_parent).mTransform))
        return;
    const Fmatrix& actor_parent = ak->LL_GetBoneInstance(leg->actor_parent).mTransform; // final: parents are calculated first
    if (!actor_parent_inv.invert_b(actor_parent))
        return;
    puppet_local.mul_43(puppet_parent_inv, pk->LL_GetBoneInstance(leg->puppet_id).mTransform);
    actor_local.mul_43(actor_parent_inv, B->mTransform);
    puppet_local.c.set(actor_local.c);
    B->mTransform.mul_43(actor_parent, puppet_local);
}

void nlc_leg_bones_uninstall()
{
    if (!g_nlc_leg_actor_kin)
        return;
    // the kinematics may be gone (visual swap): only touch it while the actor still has it
    CActor* a = Actor();
    IKinematics* now = a ? PKinematics(a->Visual()) : nullptr;
    if (now == g_nlc_leg_actor_kin)
    {
        for (SNlcLegBone& leg : g_nlc_leg_bones)
        {
            if (leg.actor_id == BI_NONE)
                continue;
            CBoneInstance& bi = now->LL_GetBoneInstance(leg.actor_id);
            if (bi.callback() == nlc_leg_bone_callback)
                bi.reset_callback();
        }
        now->CalculateBones_Invalidate();
    }
    g_nlc_leg_actor_kin = nullptr;
}

void nlc_leg_bones_install(CActor* a)
{
    IKinematics* k = PKinematics(a->Visual());
    if (!k)
        return;
    if (k == g_nlc_leg_actor_kin)
        return;
    g_nlc_leg_actor_kin = nullptr; // a different skeleton (outfit visual swap): the old one is not ours to touch
    for (SNlcLegBone& leg : g_nlc_leg_bones)
    {
        leg.actor_id = k->LL_BoneID(leg.name);
        leg.puppet_kin = nullptr;
        leg.puppet_id = BI_NONE;
        leg.puppet_parent = BI_NONE;
        leg.actor_parent = leg.actor_id != BI_NONE ? k->GetBoneData(leg.actor_id).GetParentID() : BI_NONE;
        if (leg.actor_id == BI_NONE)
            continue;
        CBoneInstance& bi = k->LL_GetBoneInstance(leg.actor_id);
        if (bi.callback()) // never replace another system's callback
        {
            leg.actor_id = BI_NONE;
            leg.actor_parent = BI_NONE;
            continue;
        }
        bi.set_callback(bctCustom, nlc_leg_bone_callback, &leg, FALSE);
    }
    g_nlc_leg_actor_kin = k;
    k->CalculateBones_Invalidate();
}
} // namespace

void nlc_visible_body_set(bool active, u16 id)
{
    g_nlc_visible_body_active = active;
    g_nlc_visible_body_id = id;
    if (!active || !g_nlc_shadow_legs_from_body)
        nlc_leg_bones_uninstall();
}

void CHUDManager::Render_Actor_Shadow(u32 context_id) // added by KD
{
    if (!psActorFlags.test(AF_ACTOR_SHADOW))
        return;

    if (nullptr == pUI)
        return;

    CObject* O = g_pGameLevel->CurrentViewEntity();
    if (nullptr == O)
        return;

    CActor* A = smart_cast<CActor*>(O);
    if (!A)
        return;

    // KD: we need to render actor shadow only in first eye cam mode because
    // in other modes actor model already in scene graph and renders well
    if (A->active_cam() != eacFirstEye)
        return;
/*
    const auto flashlight = smart_cast<CFlashlight*>(A->inventory().ItemFromSlot(DETECTOR_SLOT));
    if (flashlight && flashlight->torch_active())
        return;
*/
    const auto torch = smart_cast<CTorch*>(A->inventory().ItemFromSlot(TORCH_SLOT));
    if (torch && torch->torch_active())
        return;

    const auto wpn = smart_cast<CWeapon*>(A->inventory().ActiveItem());
    if (wpn && (wpn->IsFlashlightOn()))
        return;

    std::scoped_lock slock(render_lock);
    if (g_nlc_visible_body_active && g_nlc_shadow_legs_from_body)
        nlc_leg_bones_install(A); // NLC: cheap when already installed on this skeleton
    O->renderable_Render(context_id, O->H_Root());
}

/*
void CHUDManager::Render_Actor_FirstPersonBody(u32 context_id)
{
    CObject* O = g_pGameLevel->CurrentEntity();
    if (nullptr == O)
        return;

    CActor* A = smart_cast<CActor*>(O);
    if (!A)
        return;

    A->RenderFirstPersonBody(context_id, O->H_Root());
}
*/

extern void draw_wnds_rects();

extern ENGINE_API BOOL bShowPauseString;

//отрисовка элементов интерфейса
#include "string_table.h"

void CHUDManager::RenderUI()
{
    fl_hook::CHudRenderUIGuard pda3d_guard;

    if (!b_online)
        return;

    if (HUD().GetUI()->GameIndicatorsShown() /*&& psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT)*/)
    {
        HitMarker.Render();

        if (psHUD_Flags.is(HUD_CROSSHAIR | HUD_CROSSHAIR_RT | HUD_CROSSHAIR_RT2) && g_SingleGameDifficulty < egdMaster)
            m_pHUDTarget->Render();
    }

    if (pUI)
        pUI->Render();

    Font().Render();


    if (Device.Paused() && bShowPauseString)
    {
        CGameFont* pFont = Font().pFontGraffiti50Russian;
        pFont->SetColor(0x80FF0000);
        LPCSTR _str = CStringTable().translate("st_game_paused").c_str();

        Fvector2 _pos;
        _pos.set(UI_BASE_WIDTH / 2.0f, UI_BASE_HEIGHT / 2.0f);
        UI()->ClientToScreenScaled(_pos);
        pFont->SetAligment(CGameFont::alCenter);
        pFont->Out(_pos.x, _pos.y, _str);
        pFont->OnRender();
    }
}

void CHUDManager::OnEvent(EVENT E, u64 P1, u64 P2) {}

collide::rq_result& CHUDManager::GetCurrentRayQuery() { return m_pHUDTarget->RQ; }

void CHUDManager::SetCrosshairDisp(float dispf, float disps) { m_pHUDTarget->HUDCrosshair.SetDispersion(psHUD_Flags.test(HUD_CROSSHAIR_DYNAMIC) ? dispf : disps); }

void CHUDManager::ShowCrosshair(bool show) { m_pHUDTarget->m_bShowCrosshair = show; }

void CHUDManager::Hit(int idx, float power, const Fvector& dir) { HitMarker.Hit(idx, dir); }

void CHUDManager::SetHitmarkType(LPCSTR tex_name) { HitMarker.InitShader(tex_name); }
#include "ui\UIMainInGameWnd.h"
void CHUDManager::OnScreenRatioChanged()
{
    xr_delete(pUI->UIMainIngameWnd);

    pUI->UIMainIngameWnd = xr_new<CUIMainIngameWnd>();
    pUI->UIMainIngameWnd->Init();
    pUI->UIMainIngameWnd->OnConnected(); //Перезагружаем здесь миникарту, чтобы она не пропадала при изменении разрешения экрана.
    pUI->UnLoad();
    pUI->Load(pUI->UIGame());
    if (auto act = Actor())
        act->UpdateArtefactPanel(); //Обновляем панель с артами на худе
}

void CHUDManager::OnDisconnected()
{
    //.	if(!b_online)			return;
    b_online = false;
    if (pUI)
        Device.seqFrame.Remove(pUI);
}

void CHUDManager::OnConnected()
{
    if (b_online)
        return;
    b_online = true;
    if (pUI)
    {
        Device.seqFrame.Add(pUI, REG_PRIORITY_LOW - 1000);
    }
}

void CHUDManager::net_Relcase(CObject* object)
{
    VERIFY(m_pHUDTarget);
    m_pHUDTarget->net_Relcase(object);
}

#include "player_hud.h"
bool CHUDManager::RenderActiveItemUIQuery()
{
    if (!need_render_hud())
        return false;

    return (g_player_hud && g_player_hud->render_item_ui_query());
}

void CHUDManager::RenderActiveItemUI() { g_player_hud->render_item_ui(); }
