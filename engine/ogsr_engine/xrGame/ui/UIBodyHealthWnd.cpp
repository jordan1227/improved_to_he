#include "stdafx.h"
#include "UIBodyHealthWnd.h"
#include "UIXmlInit.h"
#include "UICursor.h"
#include "map_hint.h"
#include "../Actor.h"
#include "../ActorCondition.h"
#include "../ai_space.h"
#include "../script_engine.h"
#include "../ui_base.h"
#include "UITextureMaster.h"

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
// NLC: the green layer is the healthy (stage 1) health picture drawn over the health layer, its alpha = green share
static LPCSTR s_layer_tex[] = {"ui_inGame2_%s_inv_%d", "ui_inGame2_%s_inv_%d", "ui_bleeding_%s_inv_%d", "ui_fracture_%s_inv_%d"};
static const u8 s_pick_order[] = {head, left_arm, right_arm, torso, left_leg, right_leg};

static constexpr u32 ALPHA_IDLE = 0xD0;
static constexpr u32 ALPHA_HOVER = 0xFF;
static constexpr u32 ALPHA_DIM = 0x60;
static constexpr u32 TINT_LIGHT = 0xFFC060; // light injury (sivol_actor_crit level 1), concussion
static constexpr u32 TINT_SEVERE = 0xFF3820; // severe injury (level 2)
static constexpr u32 TINT_SPLINT = 0x70B8FF; // splinted (level 3)
static constexpr int STAGE_LIGHT = 3; // fracture overlay stage used for an injury without a bigger fracture
static constexpr int STAGE_SEVERE = 6;
static constexpr u32 HINT_DELAY_MS = 300;
static constexpr u8 HINT_FIGURE = 0; // asked as part 0: total health and how injuries heal (sivol_body_text)
static constexpr u32 HINT_REFRESH_MS = 1000;

CUIBodyHealthWnd::CUIBodyHealthWnd() {}
CUIBodyHealthWnd::~CUIBodyHealthWnd() { xr_delete(m_hint); }

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
            const Frect& r = (l == layer_health || l == layer_green) ? P.rect : overlay;
            S.SetAutoDelete(false);
            AttachChild(&S);
            S.Init(r.x1, r.y1, r.width(), r.height());
            S.SetStretchTexture(true);
            S.Show(false);
        }
    }

    // hover text: the multi-line hint of the map (hint_item.xml), not attached, drawn by DrawHint
    m_hint = xr_new<CUIMapHint>();
    m_hint->Init();
    m_hint->SetHintWidth(xml.ReadAttribFlt(path, 0, "hint_width", 270.f), xml.ReadAttribFlt(path, 0, "hint_text_inset", 10.f));
    m_hint->SetBottomPadding(10.f);

    // NLC: pain bar and icon (children of this window, coordinates relative to it)
    string256 node;
    xr_sprintf(node, "%s:pain_bar", path);
    if (xml.NavigateToNode(node, 0))
    {
        m_pain_bar.SetAutoDelete(false);
        AttachChild(&m_pain_bar);
        m_pain_bar_ok = CUIXmlInit::InitProgressBar(xml, node, 0, &m_pain_bar);
        m_pain_bar.Show(false);
    }
    xr_sprintf(node, "%s:pain_icon", path);
    if (xml.NavigateToNode(node, 0))
    {
        m_pain_icon.SetAutoDelete(false);
        AttachChild(&m_pain_icon);
        m_pain_icon_ok = CUIXmlInit::InitStatic(xml, node, 0, &m_pain_icon);
        m_pain_icon.Show(false);
    }

    InitBars(xml, path);

    // NLC: the armoured figure of figure.script (Init(712, 176, 107, 320)); without its right edge, which the
    // skeleton overlaps. XML: figure_rect="x,y,w,h" in the parent (inventory) window.
    m_figure_rect.set(712.f, 176.f, 712.f + 90.f, 176.f + 320.f);
    if (LPCSTR fr = xml.ReadAttrib(path, 0, "figure_rect", nullptr))
    {
        float x, y, w, h;
        if (sscanf(fr, "%f,%f,%f,%f", &x, &y, &w, &h) == 4)
            m_figure_rect.set(x, y, x + w, y + h);
    }
}

bool CUIBodyHealthWnd::Active() const
{
    const CActorBodyHealth& B = Actor()->conditions().body();
    return B.Enabled() || B.InjurySystem();
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

bool CUIBodyHealthWnd::FigureUnderCursor()
{
    if (!IsShown() || !Actor() || !Active() || !GetParent())
        return false;
    Frect pa;
    GetParent()->GetAbsoluteRect(pa);
    Frect r = m_figure_rect;
    r.add(pa.x1, pa.y1);
    const Fvector2 c = GetUICursor()->GetCursorPosition();
    return r.in(c.x, c.y);
}

u8 CUIBodyHealthWnd::PartUnderCursor()
{
    if (!IsShown() || !Actor() || !Active())
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
    m_hint_visible = false;
    if (!IsShown() || !Actor())
        return;

    CActorBodyHealth& B = Actor()->conditions().body();
    if (!Active())
    {
        for (u8 p = head; p < count; ++p)
            for (int l = 0; l < layer_count; ++l)
                SetStage(p, l, 0);
        m_hint_part = none;
        UpdateBars(B, false, none);
        if (m_pain_bar_ok)
            m_pain_bar.Show(false);
        if (m_pain_icon_ok)
            m_pain_icon.Show(false);
        return;
    }

    const bool body_on = B.Enabled();
    const bool injuries = B.InjurySystem();

    if (m_pain_bar_ok)
    {
        m_pain_bar.Show(body_on);
        if (body_on)
            m_pain_bar.SetProgressPos(100.f * std::clamp(B.Pain() / std::max(B.PainMax(), 0.001f), 0.f, 1.f));
    }
    if (m_pain_icon_ok)
        m_pain_icon.Show(body_on);
    const float total = std::clamp(Actor()->conditions().GetHealth(), 0.f, 1.f);
    const u8 hover = PartUnderCursor();
    const bool dragging = m_drag_section.size() > 0;

    for (u8 p = head; p < count; ++p)
    {
        // health: stage 1 = healthy (green) .. 10 = destroyed (red); white health, or the total with body health off
        const float h = body_on ? std::clamp(B.PartHealth(p), 0.f, 1.f) : total;
        SetStage(p, layer_health, std::clamp(10 - iFloor(h * 10.f - 0.0001f), 1, 10));

        float green = 0.f;
        if (body_on)
        {
            const float room = B.PartMaxHealth(p) - B.PartHealth(p);
            green = room > 0.005f ? std::clamp(B.PartGreen(p) / room, 0.f, 1.f) : 0.f;
        }
        SetStage(p, layer_green, green > 0.02f ? 1 : 0); // NLC: tinted bright green below, visible over a healthy limb
        SetStage(p, layer_bleeding, body_on ? stage_of(B.Bleeding(p), m_bleed_max) : 0);

        // fracture layer: real fractures, plus the injury of the part's zone (amber light, red severe)
        const float frac = body_on ? B.Fracture(p) : 0.f;
        const u8 inj = injuries ? B.Injury(p) : 0;
        int fst = stage_of(frac, 1.f);
        u32 tint = 0xFFFFFF;
        if (inj == 3)
        {
            fst = std::max(fst, STAGE_LIGHT);
            tint = TINT_SPLINT;
        }
        else if (inj >= 2 && (frac > 0.f || !body_on))
        {
            fst = std::max(fst, STAGE_SEVERE);
            tint = TINT_SEVERE;
        }
        else if (inj >= 1)
        {
            fst = std::max(fst, STAGE_LIGHT);
            tint = TINT_LIGHT;
        }
        SetStage(p, layer_fracture, fst);

        u32 a = ALPHA_IDLE;
        if (dragging)
            a = !B.ItemTargetsPart(m_drag_section.c_str(), p) ? ALPHA_DIM : p == hover ? ALPHA_HOVER : ALPHA_IDLE;
        else if (p == hover)
            a = ALPHA_HOVER;
        SPart& P = m_parts[p];
        P.layer[layer_health].SetColor((a << 24) | 0xFFFFFF);
        P.layer[layer_green].SetColor((u32(a * (0.35f + 0.65f * green)) << 24) | 0x60FF70);
        P.layer[layer_bleeding].SetColor((a << 24) | 0xFFFFFF);
        P.layer[layer_fracture].SetColor((a << 24) | tint);
    }

    UpdateBars(B, body_on, hover);
    UpdateHint(hover != none ? hover : (!dragging && FigureUnderCursor() ? HINT_FIGURE : u8(none)), dragging);
}

// ---------------------------------------------------------------------------
// NLC: part health bars. Solid blocks cut from one lit texel of ui_mg_progress_efficiency_full (as the
// artifact container's condition bar, ui_arc_container_menu.script), stretched and cut to the value.
// XML: <part_bars width height> with <head|body|left_arm|right_arm|left_leg|right_leg x y/> and an
// optional <label x y width height> template (position relative to the bar) for the number.
// ---------------------------------------------------------------------------
static LPCSTR s_bar_node[count] = {"", "head", "body", "left_arm", "right_arm", "left_leg", "right_leg"};
static constexpr LPCSTR BAR_TEX = "ui_mg_progress_efficiency_full";
static constexpr float BAR_OUTLINE = 0.7f;
static constexpr u32 BAR_GREEN = 0x60FF70;
static constexpr u32 BAR_GAIN = 0x80C8FF;

static u32 band_color(float h)
{
    if (h >= 0.6f)
        return color_argb(255, 215, 215, 215);
    if (h >= 0.3f)
        return color_argb(255, 255, 190, 80);
    return color_argb(255, 255, 85, 60);
}

void CUIBodyHealthWnd::InitBars(CUIXml& xml, LPCSTR path)
{
    string256 node;
    xr_sprintf(node, "%s:part_bars", path);
    if (!xml.NavigateToNode(node, 0))
        return;
    const float w = xml.ReadAttribFlt(node, 0, "width", 30.f);
    const float h = xml.ReadAttribFlt(node, 0, "height", 4.f);
    const Frect tr = CUITextureMaster::GetTextureRect(BAR_TEX);
    string256 lnode;
    xr_sprintf(lnode, "%s:label", node);
    const bool has_label = xml.NavigateToNode(lnode, 0);
    for (u8 p = head; p < count; ++p)
    {
        string256 pn;
        xr_sprintf(pn, "%s:%s", node, s_bar_node[p]);
        if (!xml.NavigateToNode(pn, 0))
            continue;
        SBar& B = m_bars[p];
        const float x = xml.ReadAttribFlt(pn, 0, "x", 0.f);
        const float y = xml.ReadAttribFlt(pn, 0, "y", 0.f);
        B.rect.set(x, y, x + w, y + h);
        for (int i = 0; i < bar_blocks; ++i)
        {
            CUIStatic& S = B.block[i];
            S.SetAutoDelete(false);
            AttachChild(&S);
            const float o = i == bar_outline ? BAR_OUTLINE : 0.f;
            S.Init(x - o, y - o, w + 2.f * o, h + 2.f * o);
            S.InitTexture(BAR_TEX);
            S.SetOriginalRect(tr.x1 + 13.25f, tr.y1, 0.5f, tr.height());
            S.SetStretchTexture(true);
            S.Show(false);
        }
        B.block[bar_outline].SetColor(color_argb(255, 120, 120, 120));
        B.block[bar_back].SetColor(color_argb(230, 25, 25, 25));
        if (has_label)
        {
            B.label.SetAutoDelete(false);
            AttachChild(&B.label);
            B.label_ok = CUIXmlInit::InitStatic(xml, lnode, 0, &B.label);
            const Fvector2 lp = B.label.GetWndPos();
            B.label.SetWndPos(x + lp.x, y + lp.y);
            B.label.Show(false);
        }
        m_bars_ok = true;
    }
}

// the treatment's total heal of a section (vergas_medical_elastic.get_total_heal, percent), asked once a second
float CUIBodyHealthWnd::TotalHeal(LPCSTR section)
{
    const u32 now = Device.dwTimeGlobal;
    if (m_heal_section != section || now >= m_heal_next)
    {
        m_heal_section = section;
        m_heal_next = now + 1000;
        m_heal_total = 0.f;
        luabind::functor<float> fl;
        if (ai().script_engine().functor<float>("vergas_medical_elastic.get_total_heal", fl))
            m_heal_total = fl(section);
    }
    return m_heal_total / 100.f;
}

void CUIBodyHealthWnd::UpdateBars(const CActorBodyHealth& B, bool show, u8 hover)
{
    if (!m_bars_ok)
        return;
    // preview: a dragged body item over a part it can be used on, else the hovered use row
    LPCSTR sect = nullptr;
    u8 pp = none;
    if (m_drag_section.size() && hover != none && B.ItemTargetsPart(m_drag_section.c_str(), hover))
    {
        sect = m_drag_section.c_str();
        pp = hover;
    }
    else if (m_preview_section.size() && m_preview_part != none)
    {
        sect = m_preview_section.c_str();
        pp = m_preview_part;
    }
    float pw[count]{}, pg[count]{};
    const bool preview = show && sect;
    if (preview)
        B.Predict(sect, pp, TotalHeal(sect), pw, pg);
    const u32 gain_a = u32(255.f * (0.6f + 0.4f * _abs(_sin(Device.fTimeGlobal * 3.f))));

    for (u8 p = head; p < count; ++p)
    {
        SBar& S = m_bars[p];
        if (S.rect.width() <= 0.f)
            continue;
        S.block[bar_outline].Show(show);
        S.block[bar_back].Show(show);
        if (S.label_ok)
            S.label.Show(show);
        if (!show)
        {
            S.block[bar_white].Show(false);
            S.block[bar_green].Show(false);
            S.block[bar_gain].Show(false);
            continue;
        }
        const float w = S.rect.width();
        const float white = std::clamp(B.PartHealth(p), 0.f, 1.f);
        const float total = std::clamp(B.PartTotal(p), white, 1.f);
        const float after = preview ? std::clamp(pw[p] + pg[p], total, 1.f) : total;
        auto set = [&](int i, float from, float to, u32 color) {
            CUIStatic& s = S.block[i];
            const float len = (to - from) * w;
            s.Show(len > 0.2f);
            s.SetWndPos(S.rect.x1 + from * w, S.rect.y1);
            s.SetWidth(std::max(len, 0.f));
            s.SetColor(color);
        };
        const u32 band = band_color(total);
        set(bar_white, 0.f, white, band);
        set(bar_green, white, total, (u32(255) << 24) | BAR_GREEN);
        set(bar_gain, total, after, (gain_a << 24) | BAR_GAIN);
        if (S.label_ok)
        {
            const bool gain = after - total >= 0.005f;
            string16 t;
            xr_sprintf(t, "%d", iFloor((gain ? after : total) * 100.f + 0.5f));
            S.label.SetText(t);
            S.label.SetTextColor(gain ? ((u32(255) << 24) | BAR_GAIN) : band);
        }
    }
}

// the line comes from Lua (sivol_body_text.part_text(part)), asked on a hover change and once a second
void CUIBodyHealthWnd::UpdateHint(u8 hover, bool dragging)
{
    if (!m_hint || hover == none || dragging)
    {
        m_hint_part = none;
        return;
    }
    const u32 now = Device.dwTimeGlobal;
    if (hover != m_hint_part)
    {
        m_hint_part = hover;
        m_hover_since = now;
        m_hint_next = 0;
    }
    if (now < m_hover_since + HINT_DELAY_MS)
        return;
    if (now >= m_hint_next)
    {
        m_hint_next = now + HINT_REFRESH_MS;
        LPCSTR text = nullptr;
        luabind::functor<LPCSTR> fl;
        if (ai().script_engine().functor<LPCSTR>("sivol_body_text.part_text", fl))
            text = fl(int(hover));
        if (!text || !text[0])
        {
            m_hint_part = none; // nothing to say: ask again on the next hover
            return;
        }
        m_hint->SetText(text);
    }

    // left of the cursor (the window sits at the right edge of the inventory), kept on screen
    const Fvector2 c = GetUICursor()->GetCursorPosition();
    const Fvector2 sz = m_hint->GetWndSize();
    Fvector2 pos{c.x - sz.x - 16.f, c.y - 8.f};
    clamp(pos.x, 0.f, std::max(0.f, UI_BASE_WIDTH - sz.x));
    clamp(pos.y, 0.f, std::max(0.f, UI_BASE_HEIGHT - sz.y));
    m_hint->SetWndPos(pos);
    m_hint_visible = true;
}

void CUIBodyHealthWnd::DrawHint()
{
    if (m_hint_visible && m_hint && IsShown())
        m_hint->Draw_();
}
