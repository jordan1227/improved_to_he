#include "stdafx.h"
#include "UIMultiLineEdit.h"
#include "../../xr_3da/xr_input.h"
#include "../HUDManager.h"

using namespace luabind;

namespace
{
constexpr u32 HOLD_WAIT_TIME = 300;
constexpr u32 HOLD_REPEAT_TIME = 50;
constexpr float LINE_INTERVAL = 0.f;
} // namespace

u8 nlc_dik_to_cp1251(int dik)
{
    const u16 packed = pInput->DikToChar(dik, true); // UTF-8 bytes of one character
    if (!packed)
        return 0;
    char utf8[2];
    std::memcpy(utf8, &packed, 2);
    const int len = utf8[1] ? 2 : 1;
    WCHAR w{};
    if (MultiByteToWideChar(CP_UTF8, 0, utf8, len, &w, 1) != 1)
        return 0;
    char c{};
    BOOL lossy = FALSE;
    if (WideCharToMultiByte(1251, 0, &w, 1, &c, 1, nullptr, &lossy) != 1 || lossy)
        return 0;
    return u8(c);
}

CUIMultiLineEdit::CUIMultiLineEdit() { m_pFont = UI()->Font()->pFontLetterica16Russian; }

void CUIMultiLineEdit::Init(float x, float y, float width, float height)
{
    inherited::Init(x, y, width, height);
    m_bLayoutDirty = true;
}

float CUIMultiLineEdit::LineStep() const
{
    float h = m_pFont->CurrentHeight_();
    UI()->ClientToScreenScaledHeight(h);
    return h + LINE_INTERVAL;
}

int CUIMultiLineEdit::VisibleLines() const
{
    const float step = LineStep();
    return step > 0.f ? _max(1, iFloor((GetHeight() - 2.f * m_pad) / step)) : 1;
}

float CUIMultiLineEdit::TextWidth(const char* s, int len) const
{
    if (len <= 0)
        return 0.f;
    xr_string tmp{s, size_t(len)};
    float w = m_pFont->SizeOf_(tmp.c_str());
    UI()->ClientToScreenScaledWidth(w);
    return w;
}

// Splits m_text into display lines: hard breaks at '\n', soft wraps at the last space.
void CUIMultiLineEdit::Relayout()
{
    m_layout.clear();
    m_layout_width = GetWidth();
    const float max_w = GetWidth() - 2.f * m_pad - 4.f;
    const int n = (int)m_text.size();
    int start = 0;
    while (true)
    {
        int i = start, last_space = -1;
        float w = 0.f;
        bool hard = false;
        while (i < n)
        {
            const char c = m_text[i];
            if (c == '\n')
            {
                hard = true;
                break;
            }
            float cw = m_pFont->SizeOf_(c);
            UI()->ClientToScreenScaledWidth(cw);
            if (w + cw > max_w && i > start)
                break;
            if (c == ' ')
                last_space = i;
            w += cw;
            ++i;
        }
        if (i >= n || hard)
        {
            m_layout.push_back({start, i - start});
            if (i >= n)
                break;
            start = i + 1; // skip '\n'
            continue;
        }
        // soft wrap: break after the last space when there is one
        const int end = last_space >= start ? last_space + 1 : i;
        m_layout.push_back({start, end - start});
        start = end;
    }
    m_bLayoutDirty = false;
}

int CUIMultiLineEdit::LineOfCursor() const
{
    const int cnt = (int)m_layout.size();
    for (int l = 0; l < cnt; ++l)
    {
        const SLine& ln = m_layout[l];
        const bool next_starts_here = l + 1 < cnt && m_layout[l + 1].start == m_cursor;
        if (m_cursor >= ln.start && m_cursor <= ln.start + ln.len && !next_starts_here)
            return l;
    }
    return _max(0, cnt - 1);
}

void CUIMultiLineEdit::EnsureCursorVisible()
{
    if (m_bLayoutDirty || !fsimilar(m_layout_width, GetWidth()))
        Relayout();
    const int line = LineOfCursor();
    const int vis = VisibleLines();
    if (line < m_first_line)
        m_first_line = line;
    else if (line >= m_first_line + vis)
        m_first_line = line - vis + 1;
    clamp(m_first_line, 0, _max(0, (int)m_layout.size() - 1));
}

void CUIMultiLineEdit::Changed()
{
    m_bLayoutDirty = true;
    EnsureCursorVisible();
    if (GetMessageTarget())
        GetMessageTarget()->SendMessage(this, EDIT_TEXT_CHANGED, nullptr);
}

void CUIMultiLineEdit::SetText(LPCSTR str)
{
    m_text = str ? str : "";
    xr_string clean;
    clean.reserve(m_text.size());
    for (const char c : m_text)
        if (c != '\r')
            clean += c;
    m_text = std::move(clean);
    if (m_text.size() > m_max_chars)
        m_text.resize(m_max_chars);
    m_cursor = (int)m_text.size();
    m_first_line = 0;
    m_bLayoutDirty = true;
}

void CUIMultiLineEdit::InsertText(const char* s)
{
    for (; *s; ++s)
    {
        const char c = *s;
        if (c == '\r' || (c != '\n' && (u8)c < 32))
            continue;
        if (m_text.size() >= m_max_chars)
            break;
        m_text.insert(m_text.begin() + m_cursor, c);
        ++m_cursor;
    }
}

void CUIMultiLineEdit::SetFocus(bool focus)
{
    if (focus == m_bInputFocus)
        return;
    if (focus)
    {
        if (GetParent())
            GetParent()->SetKeyboardCapture(this, true);
        m_bInputFocus = true;
    }
    else
    {
        m_bInputFocus = false;
        m_hold_dik = 0;
        if (GetParent())
            GetParent()->SetKeyboardCapture(this, false);
        if (GetMessageTarget())
            GetMessageTarget()->SendMessage(this, EDIT_TEXT_COMMIT, nullptr);
    }
}

void CUIMultiLineEdit::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
    if (msg == WINDOW_KEYBOARD_CAPTURE_LOST && m_bInputFocus)
    {
        m_bInputFocus = false;
        m_hold_dik = 0;
        if (GetMessageTarget())
            GetMessageTarget()->SendMessage(this, EDIT_TEXT_COMMIT, nullptr);
    }
}

bool CUIMultiLineEdit::OnMouse(float x, float y, EUIMessages mouse_action)
{
    if (mouse_action == WINDOW_MOUSE_WHEEL_UP || mouse_action == WINDOW_MOUSE_WHEEL_DOWN)
    {
        if (m_bLayoutDirty)
            Relayout();
        m_first_line += mouse_action == WINDOW_MOUSE_WHEEL_UP ? -1 : 1;
        clamp(m_first_line, 0, _max(0, (int)m_layout.size() - VisibleLines()));
        return true;
    }
    if (mouse_action != WINDOW_LBUTTON_DOWN)
        return false;

    if (m_bLayoutDirty)
        Relayout();
    // place the cursor where the click landed
    const int line = _min((int)m_layout.size() - 1, m_first_line + iFloor((y - m_pad) / LineStep()));
    if (line >= 0)
    {
        const SLine& ln = m_layout[line];
        int col = 0;
        const float px = x - m_pad;
        while (col < ln.len && TextWidth(m_text.c_str() + ln.start, col + 1) <= px)
            ++col;
        m_cursor = ln.start + col;
    }
    SetFocus(true);
    return true;
}

bool CUIMultiLineEdit::OnKeyboard(int dik, EUIMessages keyboard_action)
{
    if (!m_bInputFocus)
        return false;
    if (keyboard_action == WINDOW_KEY_PRESSED)
    {
        m_hold_dik = dik;
        m_hold_wait = true;
        m_hold_time = GetTickCount();
        KeyPressed(dik);
    }
    else if (keyboard_action == WINDOW_KEY_RELEASED && m_hold_dik == dik)
        m_hold_dik = 0;
    return true;
}

bool CUIMultiLineEdit::KeyPressed(int dik)
{
    const bool ctrl = pInput->iGetAsyncKeyState(DIK_LCONTROL) || pInput->iGetAsyncKeyState(DIK_RCONTROL);
    if (m_bLayoutDirty)
        Relayout();

    auto move_vertical = [&](int delta) {
        const int line = LineOfCursor();
        const int target = line + delta;
        if (target < 0 || target >= (int)m_layout.size())
            return;
        const float px = TextWidth(m_text.c_str() + m_layout[line].start, m_cursor - m_layout[line].start);
        const SLine& ln = m_layout[target];
        int col = 0;
        while (col < ln.len && TextWidth(m_text.c_str() + ln.start, col + 1) <= px + 0.5f)
            ++col;
        m_cursor = ln.start + col;
    };

    switch (dik)
    {
    case DIK_TAB: return true;
    case DIK_ESCAPE: SetFocus(false); return true;
    case DIK_RETURN:
    case DIK_NUMPADENTER:
        if (ctrl)
        {
            SetFocus(false);
            return true;
        }
        if (m_text.size() < m_max_chars)
        {
            m_text.insert(m_text.begin() + m_cursor, '\n');
            ++m_cursor;
            Changed();
        }
        return true;
    case DIK_BACK:
        if (m_cursor > 0)
        {
            m_text.erase(m_text.begin() + m_cursor - 1);
            --m_cursor;
            Changed();
        }
        return true;
    case DIK_DELETE:
        if (m_cursor < (int)m_text.size())
        {
            m_text.erase(m_text.begin() + m_cursor);
            Changed();
        }
        return true;
    case DIK_LEFT:
        if (m_cursor > 0)
            --m_cursor;
        break;
    case DIK_RIGHT:
        if (m_cursor < (int)m_text.size())
            ++m_cursor;
        break;
    case DIK_UP: move_vertical(-1); break;
    case DIK_DOWN: move_vertical(1); break;
    case DIK_HOME:
        if (ctrl)
            m_cursor = 0;
        else if (!m_layout.empty())
            m_cursor = m_layout[LineOfCursor()].start;
        break;
    case DIK_END:
        if (ctrl)
            m_cursor = (int)m_text.size();
        else if (!m_layout.empty())
        {
            const SLine& ln = m_layout[LineOfCursor()];
            m_cursor = ln.start + ln.len;
        }
        break;
    default:
        if (ctrl && dik == DIK_V)
        {
            if (OpenClipboard(nullptr))
            {
                if (HANDLE h = GetClipboardData(CF_TEXT))
                {
                    if (const char* data = static_cast<const char*>(GlobalLock(h)))
                    {
                        InsertText(data);
                        GlobalUnlock(h);
                    }
                }
                CloseClipboard();
                Changed();
            }
            return true;
        }
        if (ctrl)
            return true;
        const u8 c = nlc_dik_to_cp1251(dik);
        if (c >= 32 && m_text.size() < m_max_chars)
        {
            m_text.insert(m_text.begin() + m_cursor, static_cast<char>(c));
            ++m_cursor;
            Changed();
        }
        return true;
    }
    EnsureCursorVisible();
    return true;
}

void CUIMultiLineEdit::Update()
{
    if (m_bInputFocus && m_hold_dik)
    {
        const u32 now = GetTickCount();
        if (m_hold_wait)
        {
            if (now - m_hold_time > HOLD_WAIT_TIME)
            {
                m_hold_wait = false;
                m_hold_time = now;
            }
        }
        else if (now - m_hold_time > HOLD_REPEAT_TIME)
        {
            m_hold_time = now;
            KeyPressed(m_hold_dik);
        }
    }
    inherited::Update();
}

void CUIMultiLineEdit::Draw()
{
    inherited::Draw();
    if (m_bLayoutDirty || !fsimilar(m_layout_width, GetWidth()))
        Relayout();

    Fvector2 abs;
    GetAbsolutePos(abs);
    const float step = LineStep();
    const int vis = VisibleLines();
    const int cursor_line = m_bInputFocus ? LineOfCursor() : -1;

    m_pFont->SetColor(m_color);
    m_pFont->SetAligment(CGameFont::alLeft);
    const int last = _min((int)m_layout.size(), m_first_line + vis);
    for (int l = m_first_line; l < last; ++l)
    {
        const SLine& ln = m_layout[l];
        Fvector2 p{abs.x + m_pad, abs.y + m_pad + step * (l - m_first_line)};
        UI()->ClientToScreenScaled(p);
        if (ln.len > 0)
        {
            xr_string s{m_text.c_str() + ln.start, size_t(ln.len)};
            m_pFont->Out(p.x, p.y, "%s", s.c_str());
        }
        if (l == cursor_line && (Device.dwTimeGlobal / 500) % 2 == 0)
        {
            Fvector2 c{abs.x + m_pad + TextWidth(m_text.c_str() + ln.start, m_cursor - ln.start),
                       abs.y + m_pad + step * (l - m_first_line)};
            UI()->ClientToScreenScaled(c);
            m_pFont->Out(c.x, c.y, "_");
        }
    }
    m_pFont->OnRender();
}

void CUIMultiLineEdit::script_register(lua_State* L)
{
    module(L)[class_<CUIMultiLineEdit, CUIWindow>("CUIMultiLineEdit")
                  .def(constructor<>())
                  .def("SetText", &CUIMultiLineEdit::SetText)
                  .def("GetText", &CUIMultiLineEdit::GetText)
                  .def("SetMaxCharCount", &CUIMultiLineEdit::SetMaxCharCount)
                  .def("SetFont", &CUIMultiLineEdit::SetFont)
                  .def("SetTextColor", &CUIMultiLineEdit::SetTextColor)
                  .def("CaptureFocus", &CUIMultiLineEdit::CaptureFocus)
                  .def("HasFocus", &CUIMultiLineEdit::HasFocus)];
}
