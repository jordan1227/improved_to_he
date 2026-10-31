#include "stdafx.h"
#include "UIPropertiesBox.h"
#include "../hudmanager.h"
#include "../level.h"
#include "UIListBoxItem.h"
#include "UIXmlInit.h"
#include "UICursor.h" // NLC: GetHoveredItem

#define OFFSET_X (5)
#define OFFSET_Y (5)
#define FRAME_BORDER_WIDTH 20
#define FRAME_BORDER_HEIGHT 22

#define ITEM_HEIGHT (GetFont()->CurrentHeight() + 2.0f)

CUIPropertiesBox::CUIPropertiesBox()
{
    SetFont(HUD().Font().pFontArial14);
    m_UIListWnd.SetImmediateSelection(true);
}

CUIPropertiesBox::~CUIPropertiesBox() {}

void CUIPropertiesBox::Init(float x, float y, float width, float height)
{
    inherited::Init(x, y, width, height);

    AttachChild(&m_UIListWnd);

    CUIXml xml_doc;
    xml_doc.Init(CONFIG_PATH, UI_PATH, "inventory_new.xml");

    LPCSTR t = xml_doc.Read("properties_box:texture", 0, "");
    R_ASSERT(t);
    InitTexture(t);

    CUIXmlInit::InitListBox(xml_doc, "properties_box:list", 0, &m_UIListWnd);

    m_UIListWnd.Init(OFFSET_X, OFFSET_Y, width - OFFSET_X * 2, height - OFFSET_Y * 2);
}

void CUIPropertiesBox::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
    if (pWnd == &m_UIListWnd)
    {
        if (msg == LIST_ITEM_CLICKED)
        {
            GetMessageTarget()->SendMessage(this, PROPERTY_CLICKED, pData);
            Hide();
        }
    }
    inherited::SendMessage(pWnd, msg, pData);
}

bool CUIPropertiesBox::AddItem(const char* str, void* pData, u32 tag_value, const char* key)
{
    AddItemEx(str, pData, tag_value, key);
    return true;
}
CUIListBoxItem* CUIPropertiesBox::AddItemEx(const char* str, void* pData, u32 tag_value, const char* key)
{
    CUIListBoxItem* itm = m_UIListWnd.AddItem(str);
    itm->SetTAG(tag_value);
    itm->SetData(pData);
    itm->SetMenuKey(key ? key : str);
    return itm;
}

// NLC: the order line lists row keys, "-" starts a new group. Groups that end up with rows get a separator between them.
void CUIPropertiesBox::ApplyMenuConfig()
{
    constexpr LPCSTR MENU_SECT = "inventory_item_menu";
    constexpr u32 GROUP_SIZE = 1000;
    constexpr u32 SEPARATOR_RANK = GROUP_SIZE - 1;

    static xr_map<shared_str, u32> ranks;
    static u32 unlisted_group{};
    static bool loaded{};
    static xr_set<shared_str> reported;
    if (!loaded)
    {
        loaded = true;
        if (pSettings->section_exist(MENU_SECT) && pSettings->line_exist(MENU_SECT, "order"))
        {
            LPCSTR order = pSettings->r_string(MENU_SECT, "order");
            string128 token;
            u32 group = 0, index = 0;
            for (int i = 0, n = _GetItemCount(order); i < n; ++i)
            {
                _GetItem(order, i, token);
                if (!xr_strcmp(token, "-"))
                {
                    ++group;
                    index = 0;
                }
                else
                    ranks[token] = group * GROUP_SIZE + index++;
            }
            unlisted_group = group + 1;
        }
        else
            Msg("! [%s] no [%s] order, the item menu keeps its code order", __FUNCTION__, MENU_SECT);
    }
    if (ranks.empty())
        return;

    const u32 count = m_UIListWnd.GetSize();
    u32 next_unlisted = unlisted_group * GROUP_SIZE;
    xr_set<u32> groups;
    bool any_icon = false;
    for (u32 i = 0; i < count; ++i)
    {
        CUIListBoxItem* row = m_UIListWnd.GetItemByIDX(i);
        const shared_str& key = row->GetMenuKey();
        const auto it = ranks.find(key);
        u32 rank;
        if (it != ranks.end())
            rank = it->second;
        else
        {
            rank = next_unlisted++;
            if (reported.insert(key).second)
                Msg("~ [%s] row [%s] is not in [%s] order, it goes last", __FUNCTION__, key.c_str(), MENU_SECT);
        }
        row->SetMenuRank(rank);
        groups.insert(rank / GROUP_SIZE);

        // optional art: icon_<key> = texture[, x, y, width, height]
        string128 icon_key;
        xr_sprintf(icon_key, "icon_%s", key.c_str());
        if (!row->HasIcon() && pSettings->line_exist(MENU_SECT, icon_key))
        {
            LPCSTR value = pSettings->r_string(MENU_SECT, icon_key);
            string128 texture;
            _GetItem(value, 0, texture);
            Frect rect;
            rect.set(0.f, 0.f, 0.f, 0.f);
            if (_GetItemCount(value) >= 5)
            {
                string32 x, y, w, h;
                _GetItem(value, 1, x);
                _GetItem(value, 2, y);
                _GetItem(value, 3, w);
                _GetItem(value, 4, h);
                rect.set(float(atof(x)), float(atof(y)), float(atof(x) + atof(w)), float(atof(y) + atof(h)));
            }
            row->SetIcon(texture, rect);
        }
        any_icon = any_icon || row->HasIcon();
    }

    // a separator opens every group but the first shown
    bool first = true;
    for (const u32 group : groups)
    {
        if (!first)
        {
            CUIListBoxItem* separator = m_UIListWnd.AddItem("");
            separator->SetMenuRank(group * GROUP_SIZE - GROUP_SIZE + SEPARATOR_RANK);
            separator->SetSeparator();
        }
        first = false;
    }
    m_UIListWnd.SortByMenuRank();

    // rows with icons need room: every row moves over so the text stays aligned
    if (any_icon)
    {
        static const int spaces = READ_IF_EXISTS(pSettings, r_s32, MENU_SECT, "icon_text_spaces", 5);
        const xr_string indent(size_t(_max(0, spaces)), ' ');
        for (u32 i = 0, n = m_UIListWnd.GetSize(); i < n; ++i)
        {
            CUIListBoxItem* row = m_UIListWnd.GetItemByIDX(i);
            if (!row->IsSeparator())
                row->SetText((indent + row->GetText()).c_str());
        }
    }
}

void CUIPropertiesBox::RemoveItemByTAG(u32 tag) { m_UIListWnd.RemoveWindow(m_UIListWnd.GetItemByTAG(tag)); }

void CUIPropertiesBox::RemoveAll() { m_UIListWnd.Clear(); }

void CUIPropertiesBox::Show(const Frect& parent_rect, const Fvector2& point)
{
    Fvector2 prop_pos;
    Fvector2 prop_size = GetWndSize();

    if (point.x - prop_size.x > parent_rect.x1 && point.y + prop_size.y < parent_rect.y2)
    {
        prop_pos.set(point.x - prop_size.x, point.y);
    }
    else if (point.x - prop_size.x > parent_rect.x1 && point.y - prop_size.y > parent_rect.y1)
    {
        prop_pos.set(point.x - prop_size.x, point.y - prop_size.y);
    }
    else if (point.x + prop_size.x < parent_rect.x2 && point.y - prop_size.y > parent_rect.y1)
    {
        prop_pos.set(point.x, point.y - prop_size.y);
    }
    else
        prop_pos.set(point.x, point.y);

    SetWndPos(prop_pos);

    inherited::Show(true);
    inherited::Enable(true);

    ResetAll();

    GetParent()->SetMouseCapture(this, true);
    m_pOrignMouseCapturer = this;
    m_UIListWnd.Reset();
}

void CUIPropertiesBox::Hide()
{
    CUIWindow::Show(false);
    CUIWindow::Enable(false);
    CUIWindow::Reset();

    if (GetParent()->GetMouseCapturer() == this)
        GetParent()->SetMouseCapture(this, false);
}

bool CUIPropertiesBox::OnMouse(float x, float y, EUIMessages mouse_action)
{
    bool cursor_on_box;

    if (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight())
        cursor_on_box = true;
    else
        cursor_on_box = false;

    if (mouse_action == WINDOW_LBUTTON_DOWN && !cursor_on_box)
    {
        Hide();
        return true;
    }

    return inherited::OnMouse(x, y, mouse_action);
}

void CUIPropertiesBox::AutoUpdateSize()
{
    // NLC: separator rows are thinner than the rest
    float rows_height = 0.f;
    for (u32 i = 0, n = m_UIListWnd.GetSize(); i < n; ++i)
        rows_height += m_UIListWnd.GetItemByIDX(i)->GetHeight();
    SetHeight(rows_height + m_UIListWnd.GetVertIndent());
    m_UIListWnd.SetHeight(GetHeight());
    float f = float(m_UIListWnd.GetLongestLength() + m_UIListWnd.GetHorizIndent()) + 2;
    SetWidth(_max(20.0f, f));
    m_UIListWnd.SetWidth(_max(20.0f, f));
    m_UIListWnd.UpdateChildrenLenght();
}

CUIListBoxItem* CUIPropertiesBox::GetClickedItem() { return m_UIListWnd.GetSelectedItem(); }

CUIListBoxItem* CUIPropertiesBox::GetHoveredItem()
{
    if (!IsShown())
        return nullptr;
    const Fvector2 c = GetUICursor()->GetCursorPosition();
    for (u32 i = 0, n = m_UIListWnd.GetSize(); i < n; ++i)
    {
        CUIListBoxItem* it = m_UIListWnd.GetItemByIDX(i);
        if (!it || !it->IsShown())
            continue;
        Frect r;
        it->GetAbsoluteRect(r);
        if (r.in(c.x, c.y))
            return it;
    }
    return nullptr;
}
void CUIPropertiesBox::Update() { inherited::Update(); }
void CUIPropertiesBox::Draw() { inherited::Draw(); }

bool CUIPropertiesBox::OnKeyboard(int dik, EUIMessages keyboard_action)
{
    Hide();
    return true;
}