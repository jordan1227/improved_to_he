#include "stdafx.h"
#include "UIBodyHealthWnd.h"
#include "UIXmlInit.h"
#include "UICursor.h"
#include "../Actor.h"
#include "../ActorCondition.h"

using namespace body_part;

// part rects inside the 300x600 skeleton cell of ui\ui_actor_body_parts (A.R.E.A. atlas)
static const float s_part_cell[count][4] = {
    {0, 0, 0, 0},
    {100, 0, 200, 100}, // head
    {98, 92, 202, 292}, // body
    {205, 100, 300, 350}, // left arm (on the right of the picture)
    {0, 100, 95, 350}, // right arm
    {150, 282, 240, 600}, // left leg
    {60, 282, 150, 600}, // right leg
};
// bleeding and fracture overlays of the same atlas are cut a little differently
static const float s_overlay_cell[count][4] = {
    {0, 0, 0, 0},
    {100, 0, 200, 100},
    {98, 100, 202, 300},
    {205, 100, 300, 350},
    {0, 100, 95, 350},
    {150, 300, 240, 600},
    {60, 300, 150, 600},
};
static LPCSTR s_part_tex[count] = {"", "head", "body", "la", "ra", "ll", "rl"};
static LPCSTR s_layer_tex[] = {"ui_inGame2_%s_inv_%d", "ui_bleeding_%s_inv_%d", "ui_fracture_%s_inv_%d"};
static const u8 s_pick_order[] = {head, left_arm, right_arm, torso, left_leg, right_leg};

static constexpr u32 COLOR_IDLE = 0xD0FFFFFF;
static constexpr u32 COLOR_HOVER = 0xFFFFFFFF;
static constexpr u32 COLOR_DIM = 0x60FFFFFF;

CUIBodyHealthWnd::CUIBodyHealthWnd() {}
CUIBodyHealthWnd::~CUIBodyHealthWnd() {}

void CUIBodyHealthWnd::InitFromXml(CUIXml& xml, LPCSTR path)
{
    CUIXmlInit::InitWindow(xml, path, 0, this);
    m_bleed_max = xml.ReadAttribFlt(path, 0, "bleed_max", 1.f);
    if (m_bleed_max <= 0.f)
        m_bleed_max = 1.f;

    const float kx = GetWidth() / 300.f;
    const float ky = GetHeight() / 600.f;
    for (u8 p = head; p < count; ++p)
    {
        SPart& P = m_parts[p];
        const float* c = s_part_cell[p];
        P.rect.set(c[0] * kx, c[1] * ky, c[2] * kx, c[3] * ky);
        const float* o = s_overlay_cell[p];
        Frect overlay;
        overlay.set(o[0] * kx, o[1] * ky, o[2] * kx, o[3] * ky);
        for (int l = 0; l < layer_count; ++l)
        {
            CUIStatic& S = P.layer[l];
            const Frect& r = l == layer_health ? P.rect : overlay;
            S.SetAutoDelete(false);
            AttachChild(&S);
            S.Init(r.x1, r.y1, r.width(), r.height());
            S.SetStretchTexture(true);
            S.Show(false);
        }
    }

}

void CUIBodyHealthWnd::SetStage(u8 part, int layer, int stage)
{
    SPart& P = m_parts[part];
    CUIStatic& S = P.layer[layer];
    if (P.stage[layer] == stage)
        return;
    P.stage[layer] = stage;
    if (stage <= 0)
    {
        S.Show(false);
        return;
    }
    string128 tex;
    xr_sprintf(tex, s_layer_tex[layer], s_part_tex[part], stage);
    S.InitTexture(tex);
    S.SetStretchTexture(true);
    S.Show(true);
}

static int stage_of(float v, float full)
{
    // 1..10, 0 = nothing to show
    if (v <= 0.f)
        return 0;
    return std::clamp(iCeil(v / full * 10.f), 1, 10);
}

u8 CUIBodyHealthWnd::PartUnderCursor()
{
    if (!IsShown() || !Actor() || !Actor()->conditions().body().Enabled())
        return none;
    Frect abs;
    GetAbsoluteRect(abs);
    const Fvector2 c = GetUICursor()->GetCursorPosition();
    for (u8 p : s_pick_order)
    {
        Frect r = m_parts[p].rect;
        r.add(abs.x1, abs.y1);
        if (r.in(c.x, c.y))
            return p;
    }
    return none;
}

void CUIBodyHealthWnd::Update()
{
    inherited::Update();
    if (!IsShown() || !Actor())
        return;

    CActorBodyHealth& B = Actor()->conditions().body();
    if (!B.Enabled())
    {
        for (u8 p = head; p < count; ++p)
            for (int l = 0; l < layer_count; ++l)
                SetStage(p, l, 0);
        return;
    }

    const u8 hover = PartUnderCursor();
    const bool dragging = m_drag_section.size() > 0;

    for (u8 p = head; p < count; ++p)
    {
        // health: stage 1 = healthy (green) .. 10 = destroyed (red)
        const float h = std::clamp(B.PartHealth(p), 0.f, 1.f);
        SetStage(p, layer_health, std::clamp(10 - iFloor(h * 10.f - 0.0001f), 1, 10));
        SetStage(p, layer_bleeding, stage_of(B.Bleeding(p), m_bleed_max));
        SetStage(p, layer_fracture, stage_of(B.Fracture(p), 1.f));

        u32 color = COLOR_IDLE;
        if (dragging)
            color = !B.ItemTargetsPart(m_drag_section.c_str(), p) ? COLOR_DIM : p == hover ? COLOR_HOVER : COLOR_IDLE;
        else if (p == hover)
            color = COLOR_HOVER;
        for (int l = 0; l < layer_count; ++l)
            m_parts[p].layer[l].SetColor(color);
    }
}
