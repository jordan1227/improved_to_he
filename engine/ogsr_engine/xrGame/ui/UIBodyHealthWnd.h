#pragma once

// NLC: body parts state window in the inventory (ported from A.R.E.A. ui_actor_health_wnd).
// Medical items are dropped onto a part to be used on it. Shown with body health or the
// sivol_actor_crit injury system on (docs/damage_model/DESIGN_recovery_and_body_status.md 14.2 E4).

#include "UIWindow.h"
#include "UIStatic.h"
#include "UIProgressBar.h"
#include "../nlc_body_health.h"

class CUIXml;
class CUIMapHint;

class CUIBodyHealthWnd : public CUIWindow
{
    typedef CUIWindow inherited;

public:
    CUIBodyHealthWnd();
    virtual ~CUIBodyHealthWnd();

    void InitFromXml(CUIXml& xml, LPCSTR path);
    virtual void Update();

    // part under the UI cursor, body_part::none if the cursor is outside
    u8 PartUnderCursor();
    void SetDragSection(LPCSTR section) { m_drag_section = section; }
    // NLC: predicted effect of an item on a part for the part bars (a hovered use row); nullptr = off.
    // A dragged item previews on the part under the cursor by itself.
    void SetPreview(LPCSTR section, u8 part)
    {
        m_preview_section = section;
        m_preview_part = part;
    }
    // hover text of the part under the cursor; the inventory draws it last, above everything
    void DrawHint();

private:
    enum
    {
        layer_health = 0,
        layer_green,
        layer_bleeding,
        layer_fracture,
        layer_count,
    };

    struct SPart
    {
        CUIStatic layer[layer_count];
        int stage[layer_count]{-1, -1, -1, -1};
        Frect rect{};
    };

    bool Active() const;
    void SetStage(u8 part, int layer, int stage);
    void UpdateHint(u8 hover, bool dragging); // hover: a body part, HINT_FIGURE or body_part::none
    bool FigureUnderCursor();

    SPart m_parts[body_part::count];
    float m_bleed_max{1.f};
    shared_str m_drag_section;

    CUIMapHint* m_hint{};
    bool m_hint_visible{};
    u8 m_hint_part{body_part::none};
    Frect m_figure_rect{}; // the armoured figure left of the skeleton, in the parent window (figure.script)

    // NLC: pain of the body health system (XML body_health:pain_bar / pain_icon, optional)
    CUIProgressBar m_pain_bar;
    CUIStatic m_pain_icon;
    bool m_pain_bar_ok{};
    bool m_pain_icon_ok{};

    // NLC: part health bars (XML body_health:part_bars, optional; DESIGN_recovery_and_body_status.md 32):
    // outline, background, preview gain, temporary (green), permanent, and a number
    enum
    {
        bar_outline = 0,
        bar_back,
        bar_white,
        bar_green,
        bar_gain,
        bar_blocks,
    };
    struct SBar
    {
        CUIStatic block[bar_blocks];
        CUIStatic label;
        Frect rect{};
        bool label_ok{};
    };
    void InitBars(CUIXml& xml, LPCSTR path);
    void UpdateBars(const CActorBodyHealth& B, bool show, u8 hover);
    float TotalHeal(LPCSTR section);
    SBar m_bars[body_part::count];
    bool m_bars_ok{};
    shared_str m_preview_section;
    u8 m_preview_part{body_part::none};
    shared_str m_heal_section;
    float m_heal_total{};
    u32 m_heal_next{};
    u32 m_hover_since{};
    u32 m_hint_next{};
};
