#pragma once

// NLC: body parts state window in the inventory (ported from A.R.E.A. ui_actor_health_wnd).
// Medical items are dropped onto a part to be used on it.

#include "UIWindow.h"
#include "UIStatic.h"
#include "../nlc_body_health.h"

class CUIXml;

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

private:
    enum
    {
        layer_health = 0,
        layer_bleeding,
        layer_fracture,
        layer_count,
    };

    struct SPart
    {
        CUIStatic layer[layer_count];
        int stage[layer_count]{-1, -1, -1};
        Frect rect{};
    };

    void SetStage(u8 part, int layer, int stage);

    SPart m_parts[body_part::count];
    float m_bleed_max{1.f};
    shared_str m_drag_section;
};
