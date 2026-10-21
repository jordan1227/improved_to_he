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

// NLC: the actor model's shadow (g_actor_shadow) is the only one cast and the model has another animation set than the
// legs the player sees. While the visible-body puppet (sivol_visible_body; the script reports it through
// level.nlc_visible_body_active(active, id)) is shown and g_actor_shadow_legs_from_body is 1:
//   - the actor model's thighs are collapsed into the hip joints (its IK is off in first person, CIKLimbsController),
//   - the puppet is added to the shadow pass itself (the scene traversal skips it, SMAP_Skip), moved so that its pelvis
//     stands under the actor model's pelvis (x/z), and clipped above its own pelvis by the shadow pixel shader
//     (shadow_direct_model_legs.vs / shadow_legs.ps, models_selflight_det_3.s elements 2 and 3): the puppet is a full body
//     (torso, head, backpack) that only the camera hides in the main view.
// The result is the actor's torso/arms/weapon on the legs the player sees. Tuning, live:
//   g_actor_shadow_legs_cut  clip height above the puppet's pelvis joint (m)
//   g_actor_shadow_legs_dy   vertical offset of the puppet legs (m)
//   g_actor_shadow_legs_fwd  offset along the puppet's heading (m)
// g_actor_shadow_debug 1 logs the pelvis/feet heights once a second ([nlc_shadow]).
static bool g_nlc_visible_body_active = false;
static u16 g_nlc_visible_body_id = u16(-1);
static const IRenderable* g_nlc_puppet_renderable = nullptr; // only compared, never dereferenced
int g_nlc_shadow_legs_from_body = 1;
int g_nlc_shadow_debug = 0;
float g_nlc_shadow_legs_cut = 0.2f; // tuned in game 2026-10-21
float g_nlc_shadow_legs_dy = 0.1f;
float g_nlc_shadow_legs_fwd = 0.f;

namespace
{
LPCSTR const g_nlc_hip_bone_names[] = {"bip01_l_thigh", "bip01_r_thigh"};
u16 g_nlc_hip_bone_ids[2] = {BI_NONE, BI_NONE};
IKinematics* g_nlc_leg_actor_kin = nullptr; // the actor skeleton the callbacks are installed on
int g_nlc_leg_callbacks_skipped = 0; // hip bones that already had another system's callback

void nlc_hip_bone_callback(CBoneInstance* B)
{
    if (!g_nlc_visible_body_active || !g_nlc_shadow_legs_from_body)
        return;
    // keep the joint position, shrink the axes: skinning collapses the leg onto the hip
    constexpr float k = 1.e-4f;
    B->mTransform.i.mul(k);
    B->mTransform.j.mul(k);
    B->mTransform.k.mul(k);
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
        for (u16 id : g_nlc_hip_bone_ids)
        {
            if (id == BI_NONE)
                continue;
            CBoneInstance& bi = now->LL_GetBoneInstance(id);
            if (bi.callback() == nlc_hip_bone_callback)
                bi.reset_callback();
        }
        now->CalculateBones_Invalidate();
    }
    g_nlc_leg_actor_kin = nullptr;
    g_nlc_hip_bone_ids[0] = g_nlc_hip_bone_ids[1] = BI_NONE;
}

void nlc_leg_bones_install(CActor* a)
{
    IKinematics* k = PKinematics(a->Visual());
    if (!k)
        return;
    if (k == g_nlc_leg_actor_kin)
        return;
    nlc_leg_bones_uninstall(); // another skeleton (outfit visual swap)
    g_nlc_leg_callbacks_skipped = 0;
    for (int i = 0; i < 2; ++i)
    {
        g_nlc_hip_bone_ids[i] = k->LL_BoneID(g_nlc_hip_bone_names[i]);
        if (g_nlc_hip_bone_ids[i] == BI_NONE)
            continue;
        CBoneInstance& bi = k->LL_GetBoneInstance(g_nlc_hip_bone_ids[i]);
        if (bi.callback()) // never replace another system's callback
        {
            ++g_nlc_leg_callbacks_skipped;
            g_nlc_hip_bone_ids[i] = BI_NONE;
            continue;
        }
        bi.set_callback(bctCustom, nlc_hip_bone_callback, nullptr, FALSE);
    }
    g_nlc_leg_actor_kin = k;
    k->CalculateBones_Invalidate();
}

// world position of a bone origin (xform: the object's world matrix)
bool nlc_bone_world(IKinematics* k, LPCSTR name, const Fmatrix& xform, Fvector& out)
{
    const u16 id = k ? k->LL_BoneID(name) : BI_NONE;
    if (id == BI_NONE)
        return false;
    out = k->LL_GetTransform(id).c;
    xform.transform_tiny(out);
    return true;
}

// g_actor_shadow_debug: pelvis and feet heights, once a second
void nlc_shadow_debug(CActor* a, CObject* puppet, const Fmatrix& xform)
{
    static u32 next = 0;
    if (Device.dwTimeGlobal < next)
        return;
    next = Device.dwTimeGlobal + 1000;
    IKinematics* pk = PKinematics(puppet->Visual());
    IKinematics* ak = PKinematics(a->Visual());
    Fvector pp{-999.f, -999.f, -999.f}, ap{-999.f, -999.f, -999.f}, fl{-999.f, -999.f, -999.f}, fr{-999.f, -999.f, -999.f};
    nlc_bone_world(pk, "bip01_pelvis", xform, pp);
    nlc_bone_world(ak, "bip01_pelvis", a->XFORM(), ap);
    nlc_bone_world(pk, "bip01_l_foot", xform, fl);
    nlc_bone_world(pk, "bip01_r_foot", xform, fr);
    const float feet = a->XFORM().c.y;
    Msg("[nlc_shadow] callbacks_skipped=%d pelvis_y puppet=%.3f actor=%.3f | pelvis_xz_gap=%.3f | foot_l=%.3f foot_r=%.3f (heights above the actor's feet) cut=%.3f dy=%.3f fwd=%.3f",
        g_nlc_leg_callbacks_skipped, pp.y - feet, ap.y - feet, _sqrt(_sqr(pp.x - ap.x) + _sqr(pp.z - ap.z)), fl.y - feet, fr.y - feet, g_nlc_shadow_legs_cut, g_nlc_shadow_legs_dy,
        g_nlc_shadow_legs_fwd);
}

// The puppet goes into the shadow pass with its own pose, hips under the actor model's hips; the shadow pixel shader clips it
// above its own pelvis (m_nlc_vb_shadow.x = clip height in the puppet's model space).
void nlc_render_puppet_shadow(u32 context_id, CActor* a)
{
    CObject* puppet = g_pGameLevel ? g_pGameLevel->Objects.net_Find(g_nlc_visible_body_id) : nullptr;
    if (!puppet || !puppet->Visual())
        return;
    IKinematics* pk = PKinematics(puppet->Visual());
    if (pk)
        pk->CalculateBones(TRUE); // the shadow phase does not calculate bones itself; same-ms calls return at once

    Fmatrix xform = puppet->XFORM();
    float cut = 1.f;
    Fvector puppet_pelvis;
    if (nlc_bone_world(pk, "bip01_pelvis", xform, puppet_pelvis))
    {
        cut = pk->LL_GetTransform(pk->LL_BoneID("bip01_pelvis")).c.y + g_nlc_shadow_legs_cut; // model space of the puppet
        Fvector actor_pelvis;
        if (nlc_bone_world(PKinematics(a->Visual()), "bip01_pelvis", a->XFORM(), actor_pelvis))
        {
            xform.c.x += actor_pelvis.x - puppet_pelvis.x;
            xform.c.z += actor_pelvis.z - puppet_pelvis.z;        }
        else
        {
            xform.c.x = a->XFORM().c.x;
            xform.c.z = a->XFORM().c.z;
        }
    }
    else
    {
        xform.c.x = a->XFORM().c.x;
        xform.c.z = a->XFORM().c.z;
    }
    xform.c.x += xform.k.x * g_nlc_shadow_legs_fwd;
    xform.c.z += xform.k.z * g_nlc_shadow_legs_fwd;
    xform.c.y += g_nlc_shadow_legs_dy;

    shader_exports.set_custom_params("m_nlc_vb_shadow", Fvector4{cut, 0.f, 0.f, 0.f});
    if (g_nlc_shadow_debug)
        nlc_shadow_debug(a, puppet, xform);
    ::Render->add_Visual(context_id, puppet->H_Root(), puppet->Visual(), xform);
}
} // namespace

void nlc_visible_body_set(bool active, u16 id)
{
    g_nlc_visible_body_active = active;
    g_nlc_visible_body_id = id;
    CObject* puppet = (active && g_pGameLevel) ? g_pGameLevel->Objects.net_Find(id) : nullptr;
    g_nlc_puppet_renderable = puppet ? static_cast<IRenderable*>(puppet) : nullptr;
    if (active)
        (void)shader_exports.get_custom_params("m_nlc_vb_shadow"); // creates the key before any render thread reads it (never overwrites)
    if (!active || !g_nlc_shadow_legs_from_body)
        nlc_leg_bones_uninstall();
}

// NLC: while the puppet is shown it is not part of the shadow-pass scene traversal: its shader now has shadow elements, and the
// traversal would add it at its true place (a second pair of legs). Render_Actor_Shadow adds it itself, per g_actor_shadow_legs_from_body.
bool CHUDManager::SMAP_Skip(const IRenderable* renderable) { return renderable && renderable == g_nlc_puppet_renderable; }

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
    if (g_nlc_visible_body_active && g_nlc_shadow_legs_from_body)
        nlc_render_puppet_shadow(context_id, A); // NLC: the puppet's legs replace the collapsed actor legs
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
