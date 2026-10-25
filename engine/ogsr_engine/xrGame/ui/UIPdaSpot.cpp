#include "stdafx.h"
#include "UIPdaSpot.h"
#include <dinput.h>
#include "Level.h"
#include "map_manager.h"
#include "map_location.h"
#include "UIEditBoxEx.h"
#include "UIStatic.h"
#include "UIXmlInit.h"
#include "UI3tButton.h"
#include "UIInventoryUtilities.h"
#include "string_table.h"

bool CUIPdaSpot::s_addTime = true;

CUIPdaSpot::CUIPdaSpot()
{
    m_mainWnd = false;
    m_levelName = nullptr;
    m_position = Fvector();

    m_spotID = u16(-1);
    m_spotType = READ_IF_EXISTS(pSettings, r_string, "user_spots", "spot_type", "red_location");
    m_maxCount = READ_IF_EXISTS(pSettings, r_u32, "user_spots", "max_count", 0);

    InitControls();
}

void CUIPdaSpot::Init(u16 spot_id, LPCSTR level_name, Fvector pos, bool main_wnd, LPCSTR spot_type)
{
    m_mainWnd = main_wnd;
    m_levelName = level_name;
    m_position = pos;
    m_spotID = m_mainWnd ? u16(-1) : spot_id;
    m_curType = (!m_mainWnd && spot_type) ? shared_str(spot_type) : m_spotType;

    // NLC: preselect the icon of the edited marker (or the default type)
    int sel = 0;
    for (u32 i = 0; i < m_types.size(); ++i)
        if (m_types[i] == m_curType)
            sel = int(i);
    SelectIcon(sel);
    UpdateTimeButton();

    if (m_counter)
    {
        string128 buf;
        if (m_maxCount)
            xr_sprintf(buf, "%s: %u / %u", CStringTable().translate("st_pda_spot_count").c_str(), UserSpotCount(), m_maxCount);
        else
            xr_sprintf(buf, "%s: %u", CStringTable().translate("st_pda_spot_count").c_str(), UserSpotCount());
        m_counter->SetText(buf);
    }

    if (!m_mainWnd)
    {
        CMapLocation* ml = Level().MapManager().GetMapLocation(m_curType, m_spotID);
        if (!ml)
            return;
        m_editBox->SetText(ml->GetHint());
        ml->HighlightSpot(true, Fcolor().set(255.f, 36.f, 0.f, 255.f));
    }

    m_editBox->CaptureFocus(true);
}

void CUIPdaSpot::InitControls()
{
    this->SetWndRect(Frect().set(0.0f, 0.0f, 1024.f, 768.f));

    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, "pda_spot.xml");
    R_ASSERT3(xml_result, "xml file not found", "pda_spot.xml");

    CUIXmlInit xml_init;

    m_background = xr_new<CUIStatic>();
    AttachChild(m_background);
    m_background->SetAutoDelete(true);
    xml_init.InitStatic(uiXml, "background", 0, m_background);

    m_editBox = xr_new<CUIEditBoxEx>();
    AttachChild(m_editBox);
    m_editBox->SetAutoDelete(true);
    xml_init.InitEditBoxEx(uiXml, "spot_name_edit", 0, m_editBox);
    m_editBox->CaptureFocus(true);

    m_btn_ok = xr_new<CUI3tButton>();
    AttachChild(m_btn_ok);
    m_btn_ok->SetAutoDelete(true);
    xml_init.Init3tButton(uiXml, "btn_apply", 0, m_btn_ok);
    m_btn_ok->SetWindowName("btn_apply");
    Register(m_btn_ok);
    AddCallback("btn_apply", BUTTON_CLICKED, fastdelegate::MakeDelegate(this, &CUIPdaSpot::OnApply));

    m_btn_cancel = xr_new<CUI3tButton>();
    AttachChild(m_btn_cancel);
    m_btn_cancel->SetAutoDelete(true);
    xml_init.Init3tButton(uiXml, "btn_cancel", 0, m_btn_cancel);
    m_btn_cancel->SetWindowName("btn_cancel");
    Register(m_btn_cancel);
    AddCallback("btn_cancel", BUTTON_CLICKED, fastdelegate::MakeDelegate(this, &CUIPdaSpot::OnExit));

    // NLC: icon row <icons x y size step><icon type="...">texture</icon>...</icons>
    if (uiXml.NavigateToNode("icons", 0))
    {
        const float x0 = uiXml.ReadAttribFlt("icons", 0, "x", 20.f);
        const float y0 = uiXml.ReadAttribFlt("icons", 0, "y", 15.f);
        const float sz = uiXml.ReadAttribFlt("icons", 0, "size", 30.f);
        const float step = uiXml.ReadAttribFlt("icons", 0, "step", 38.f);
        XML_NODE* icons = uiXml.NavigateToNode("icons", 0);
        const int n = uiXml.GetNodesNum(icons, "icon");
        for (int i = 0; i < n; ++i)
        {
            LPCSTR type = uiXml.ReadAttrib(icons, "icon", i, "type", "");
            LPCSTR tex = uiXml.Read(icons, "icon", i, "");
            if (!xr_strlen(type) || !xr_strlen(tex))
                continue;
            CUIStatic* st = xr_new<CUIStatic>();
            st->SetAutoDelete(true);
            st->Init(x0 + step * m_icons.size(), y0, sz, sz);
            st->InitTexture(tex);
            st->SetStretchTexture(true);
            AttachChild(st);
            m_icons.push_back(st);
            m_types.push_back(type);
        }
    }

    if (uiXml.NavigateToNode("btn_time", 0))
    {
        m_btn_time = xr_new<CUI3tButton>();
        AttachChild(m_btn_time);
        m_btn_time->SetAutoDelete(true);
        xml_init.Init3tButton(uiXml, "btn_time", 0, m_btn_time);
        m_btn_time->SetWindowName("btn_time");
        Register(m_btn_time);
        AddCallback("btn_time", BUTTON_CLICKED, fastdelegate::MakeDelegate(this, &CUIPdaSpot::OnTimeToggle));
    }

    if (uiXml.NavigateToNode("counter", 0))
    {
        m_counter = xr_new<CUIStatic>();
        AttachChild(m_counter);
        m_counter->SetAutoDelete(true);
        xml_init.InitStatic(uiXml, "counter", 0, m_counter);
    }
}

void CUIPdaSpot::SelectIcon(int idx)
{
    m_sel = idx;
    for (u32 i = 0; i < m_icons.size(); ++i)
        m_icons[i]->SetColor(int(i) == idx ? color_argb(255, 255, 255, 255) : color_argb(110, 255, 255, 255));
}

void CUIPdaSpot::UpdateTimeButton()
{
    if (m_btn_time)
        m_btn_time->SetText(CStringTable().translate(s_addTime ? "st_pda_spot_time_on" : "st_pda_spot_time_off").c_str());
}

u32 CUIPdaSpot::UserSpotCount() const
{
    u32 cnt = 0;
    for (const auto& it : Level().MapManager().Locations())
        if (it.location && it.location->IsUserDefined())
            ++cnt;
    return cnt;
}

xr_string CUIPdaSpot::ComposeHint() const
{
    xr_string hint = m_editBox->GetText();
    if (s_addTime && m_mainWnd)
    {
        hint += " [";
        hint += InventoryUtilities::GetGameDateAsString(InventoryUtilities::edpDateToDay, '.').c_str();
        hint += " ";
        hint += InventoryUtilities::GetGameTimeAsString(InventoryUtilities::etpTimeToMinutes, ':').c_str();
        hint += "]";
    }
    return hint;
}

void CUIPdaSpot::OnTimeToggle(CUIWindow*, void*)
{
    s_addTime = !s_addTime;
    UpdateTimeButton();
}

void CUIPdaSpot::OnAdd(CUIWindow* ui, void* d)
{
    if (m_maxCount && UserSpotCount() >= m_maxCount)
    {
        if (m_counter)
            m_counter->SetTextColor(color_argb(255, 255, 80, 80));
        return;
    }

    const shared_str type = m_types.empty() ? m_spotType : m_types[m_sel];
    CMapLocation* ml = Level().MapManager().AddUserLocation(type, m_levelName, m_position);
    ml->SetHint(ComposeHint().c_str());
    ml->SetSerializable(true);

    OnExit(ui, d);
}

void CUIPdaSpot::OnApply(CUIWindow* ui, void* d)
{
    if (m_mainWnd)
    {
        OnAdd(ui, d);
        return;
    }

    CMapLocation* ml = Level().MapManager().GetMapLocation(m_curType, m_spotID);
    if (!ml)
        return;

    const shared_str type = m_types.empty() ? m_curType : m_types[m_sel];
    if (type != m_curType)
    {
        // NLC: another icon means another spot type: re-create the marker in place
        ml->HighlightSpot(false, Fcolor().set(0.f, 0.f, 0.f, 0.f));
        const shared_str level = ml->LevelName();
        const Fvector pos = ml->GetLastPosition();
        Level().MapManager().RemoveMapLocation(m_curType, m_spotID);
        CMapLocation* nl = Level().MapManager().AddUserLocation(type, level, pos);
        nl->SetHint(m_editBox->GetText());
        nl->SetSerializable(true);
        m_mainWnd = true; // nothing to un-highlight in Exit
        Exit();
        return;
    }

    if (m_editBox->GetText() != ml->GetHint())
        ml->SetHint(m_editBox->GetText());

    OnExit(ui, d);
}

void CUIPdaSpot::OnExit(CUIWindow* w, void* d) { Exit(); }

void CUIPdaSpot::Exit()
{
    if (!m_mainWnd)
    {
        CMapLocation* ml = Level().MapManager().GetMapLocation(m_curType, m_spotID);
        if (ml)
            ml->HighlightSpot(false, Fcolor().set(0.f, 0.f, 0.f, 0.f));
    }

    m_mainWnd = false;
    m_levelName = nullptr;
    m_position = Fvector();
    m_spotID = u16(-1);
    m_editBox->SetText("");
    if (m_counter)
        m_counter->SetTextColor(color_argb(255, 220, 220, 220));

    this->Hide();
}

bool CUIPdaSpot::OnMouse(float x, float y, EUIMessages mouse_action)
{
    if (mouse_action == WINDOW_LBUTTON_DOWN)
    {
        for (u32 i = 0; i < m_icons.size(); ++i)
        {
            Frect r = m_icons[i]->GetWndRect();
            if (r.in(x, y))
            {
                SelectIcon(int(i));
                return true;
            }
        }
    }
    return base_class::OnMouse(x, y, mouse_action);
}

bool CUIPdaSpot::OnKeyboard(int dik, EUIMessages keyboard_action)
{
    if (base_class::OnKeyboard(dik, keyboard_action))
        return true;

    switch (dik)
    {
    case DIK_RETURN:
    case DIK_NUMPADENTER: {
        if (IsShown())
        {
            OnApply(nullptr, nullptr);
            return true;
        }
    }
    break;
    case DIK_ESCAPE: {
        if (IsShown())
        {
            Exit();
            return true;
        }
    }
    break;
    }

    return false;
}

void CUIPdaSpot::SendMessage(CUIWindow* pWnd, s16 msg, void* pData) { CUIWndCallback::OnEvent(pWnd, msg, pData); }
