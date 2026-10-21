#pragma once
#include "UIWindow.h"

class CUIStatic;
class CUIFrameWindow;

class CUIMapHint : public CUIWindow
{
    typedef CUIWindow inherited;
    CUIStatic* m_text;
    CUIFrameWindow* m_border;
    CUIWindow* m_owner;
    float m_pad_bottom{}; // NLC: extra room under the text (coloured multi-line hints clipped their last line)

public:
    CUIMapHint() : m_owner(NULL){};
    virtual ~CUIMapHint();
    void Init();
    void SetText(LPCSTR text);
    void SetHintWidth(float w, float text_inset = 0.f); // NLC: narrower hint; text_inset narrows only the text
    void SetBottomPadding(float p) { m_pad_bottom = p; }
    virtual void Draw() { return; };
    void Draw_();
    void SetOwner(CUIWindow* w) { m_owner = w; }
    CUIWindow* GetOwner() { return m_owner; }
};