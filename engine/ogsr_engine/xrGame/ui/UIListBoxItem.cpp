#include "StdAfx.h"
#include "UIListBoxItem.h"
#include "UIScrollView.h"
#include "../object_broker.h"
#include "UIIconParams.h"

//. u32 CUIListBoxItem::uid_counter = 0;

CUIListBoxItem::CUIListBoxItem()
{
    txt_color = 0xffffffff;
    txt_color_s = 0xffffffff;
    //.	uid					= uid_counter++;
    tag = u32(-1);
    m_bTextureAvailable = false;
}

CUIListBoxItem::~CUIListBoxItem() { delete_data(fields); }

void CUIListBoxItem::SetTAG(u32 value) { tag = value; }

u32 CUIListBoxItem::GetTAG() { return tag; }

// NLC: a thin disabled row with the line from ui_inv_layout.dds (column 4)
void CUIListBoxItem::SetSeparator()
{
    constexpr float HEIGHT = 6.f;
    SetHeight(HEIGHT);
    Enable(false);
    m_separator = xr_new<CUIStatic>();
    m_separator->SetAutoDelete(true);
    AttachChild(m_separator);
    m_separator->Init(6.f, HEIGHT * 0.5f - 1.f, GetWidth() - 12.f, 2.f);
    m_separator->InitTexture("ui\\ui_inv_layout");
    m_separator->SetOriginalRect(96.f, 0.f, 32.f, 32.f);
    m_separator->SetStretchTexture(true);
}

void CUIListBoxItem::PlaceIcon(float aspect)
{
    static const float size = READ_IF_EXISTS(pSettings, r_float, "inventory_item_menu", "icon_size", 16.f);
    const float kx = UI()->is_widescreen() ? UI()->get_current_kx() : 1.f;
    m_icon->SetWndPos(Fvector2().set(0.f, (GetHeight() - size) * 0.5f));
    m_icon->SetWndSize(Fvector2().set(size * aspect * kx, size));
    m_icon->SetStretchTexture(true);
}

void CUIListBoxItem::SetIcon(LPCSTR texture, const Frect& rect)
{
    m_icon = xr_new<CUIStatic>();
    m_icon->SetAutoDelete(true);
    AttachChild(m_icon);
    m_icon->InitTexture(texture);
    if (rect.width() > 0.f && rect.height() > 0.f)
        m_icon->SetOriginalRect(rect.x1, rect.y1, rect.width(), rect.height());
    PlaceIcon(1.f);
}

void CUIListBoxItem::SetItemIcon(const shared_str& item_section)
{
    m_icon = xr_new<CUIStatic>();
    m_icon->SetAutoDelete(true);
    AttachChild(m_icon);
    CIconParams params(item_section);
    params.set_shader(m_icon);
    PlaceIcon(params.grid_height > 0.f ? params.grid_width / params.grid_height : 1.f);
}

void CUIListBoxItem::Draw()
{
    if (m_separator) // NLC: the row width comes from AutoUpdateSize
        m_separator->SetWidth(_max(0.f, GetWidth() - 12.f));

    m_bTextureAvailable = m_bSelected;

    u32 CurColor = GetTextColor();
    u32 ResColor = (IsEnabled() ? 0xff000000 : 0x80000000) | (CurColor & 0x00ffffff);
    SetTextColor(ResColor);

    CUILabel::Draw();
}

void CUIListBoxItem::OnFocusReceive()
{
    CUILabel::OnFocusReceive();
    if (!m_separator) // NLC: nothing to select
        GetMessageTarget()->SendMessage(this, LIST_ITEM_FOCUS_RECEIVED);
}

void CUIListBoxItem::InitDefault() { InitTexture("ui_listline"); }
bool CUIListBoxItem::OnDbClick()
{
    smart_cast<CUIScrollView*>(GetParent()->GetParent())->SetSelected(this);
    GetMessageTarget()->SendMessage(this, LIST_ITEM_DB_CLICKED, &tag);
    return false;
}

bool CUIListBoxItem::OnMouseDown(int mouse_btn)
{
    if (!IsEnabled())
        return true;

    if (mouse_btn == MOUSE_1)
    {
        smart_cast<CUIScrollView*>(GetParent()->GetParent())->SetSelected(this);
        GetMessageTarget()->SendMessage(this, LIST_ITEM_SELECT, this);
        GetMessageTarget()->SendMessage(this, LIST_ITEM_CLICKED, this);
        return true;
    }
    else
        return false;
}

void CUIListBoxItem::SetSelected(bool b)
{
    CUISelectable::SetSelected(b);
    u32 col;
    if (b)
        col = txt_color_s;
    else
        col = txt_color;

    SetTextColor(col);
    for (u32 i = 0; i < fields.size(); i++)
        fields[i]->SetTextColor(col);
}

void CUIListBoxItem::SetTextColor(u32 color, u32 color_s)
{
    txt_color = color;
    txt_color_s = color_s;
    SetTextColor(color);
}

float CUIListBoxItem::FieldsLength()
{
    float c = 0;
    for (u32 i = 0; i < fields.size(); i++)
        c += fields[i]->GetWidth();
    return c;
}

CGameFont* CUIListBoxItem::GetFont() { return CUILinesOwner::GetFont(); }

CUIStatic* CUIListBoxItem::AddField(LPCSTR txt, float len, LPCSTR key)
{
    fields.push_back(xr_new<CUIStatic>());
    CUIStatic* st = fields.back();
    AttachChild(st);
    st->Init(FieldsLength(), 0, GetWidth(), len);
    st->SetFont(GetFont());
    st->SetTextAlignment(GetTextAlignment());
    st->SetVTextAlignment(m_lines.GetVTextAlignment());
    st->SetTextColor(GetTextColor());
    st->SetText(txt);
    st->SetWindowName(key);

    return st;
}

LPCSTR CUIListBoxItem::GetField(LPCSTR key)
{
    for (u32 i = 0; i < fields.size(); i++)
    {
        if (0 == xr_strcmp(fields[i]->WindowName(), key))
            return fields[i]->GetText();
    }
    return NULL;
}

void CUIListBoxItem::SetData(void* data) { pData = data; }

void* CUIListBoxItem::GetData() { return pData; }
