#pragma once

// NLC: multi-line text editor for script UIs (PDA notes).
// Text keeps real '\n' characters; lines wrap at spaces. Single-byte (CP1251) fonts.
// Enter inserts a line break; Ctrl+Enter sends EDIT_TEXT_COMMIT; every change sends
// EDIT_TEXT_CHANGED; losing focus also sends EDIT_TEXT_COMMIT so scripts can save drafts.

#include "UIWindow.h"
#include "../script_export_space.h"

class CUIMultiLineEdit : public CUIWindow
{
    using inherited = CUIWindow;

    struct SLine
    {
        int start; // index in m_text
        int len; // characters shown (without the '\n')
    };

    xr_string m_text;
    xr_vector<SLine> m_layout;
    int m_cursor{};
    int m_first_line{};
    u32 m_max_chars{4000};
    bool m_bInputFocus{};
    bool m_bLayoutDirty{true};
    float m_layout_width{};
    CGameFont* m_pFont{};
    u32 m_color{color_argb(255, 235, 219, 185)};
    float m_pad{3.f};

    int m_hold_dik{};
    bool m_hold_wait{};
    u32 m_hold_time{};

    float LineStep() const;
    int VisibleLines() const;
    float TextWidth(const char* s, int len) const;
    void Relayout();
    int LineOfCursor() const;
    void EnsureCursorVisible();
    void Changed();
    void InsertText(const char* s);
    bool KeyPressed(int dik);
    void SetFocus(bool focus);

public:
    CUIMultiLineEdit();
    virtual ~CUIMultiLineEdit() = default;

    virtual void Init(float x, float y, float width, float height);
    virtual void Draw();
    virtual void Update();
    virtual bool OnMouse(float x, float y, EUIMessages mouse_action);
    virtual bool OnKeyboard(int dik, EUIMessages keyboard_action);
    virtual bool OnKeyboardHold(int dik) { return m_bInputFocus; }
    virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = nullptr);

    void SetText(LPCSTR str);
    LPCSTR GetText() const { return m_text.c_str(); }
    void SetMaxCharCount(u32 cnt) { m_max_chars = cnt; }
    void SetFont(CGameFont* f)
    {
        m_pFont = f;
        m_bLayoutDirty = true;
    }
    void SetTextColor(u32 c) { m_color = c; }
    void CaptureFocus(bool focus) { SetFocus(focus); }
    bool HasFocus() const { return m_bInputFocus; }

    DECLARE_SCRIPT_REGISTER_FUNCTION
};

// NLC: key -> CP1251 character through the active keyboard layout (0 if none or not representable)
u8 nlc_dik_to_cp1251(int dik);

add_to_type_list(CUIMultiLineEdit)
#undef script_type_list
#define script_type_list save_type_list(CUIMultiLineEdit)
