#include "stdafx.h"
#include "UICellItem.h"
#include "../xr_level_controller.h"
#include "..\..\xr_3da\xr_input.h"
#include "../HUDManager.h"
#include "../level.h"
#include "../object_broker.h"
#include "UIDragDropListEx.h"
#include "../WeaponMagazinedWGrenade.h"
#include "UIProgressBar.h"
#include "UIXmlInit.h"
#include "UIInventoryWnd.h"
#include "../Weapon.h"
#include "../CustomOutfit.h"
#include "UICellCustomItems.h"
#include "UIInventoryLayout.h" // NLC

CUICellItem* CUICellItem::m_mouse_selected_item = nullptr;

CUICellItem::CUICellItem()
{
    m_pParentList = NULL;
    m_pData = NULL;
    m_b_already_drawn = false;
    SetAccelerator(0);
    if (Core.Features.test(xrCore::Feature::show_inv_item_condition))
    {
        m_text = NULL;
        m_pConditionState = NULL;
        m_condition_auto_width = false;
        init();
    }
    m_selected = false;
    m_select_armament = false;
    m_select_equipped = false;
    m_select_untradable = false;
}

CUICellItem::~CUICellItem()
{
    delete_data(m_childs);
}

void CUICellItem::Draw()
{
    m_b_already_drawn = true;
    inherited::Draw();
};

bool CUICellItem::OnMouse(float x, float y, EUIMessages mouse_action)
{
    if (mouse_action == WINDOW_LBUTTON_DOWN)
    {
        // GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_LBUTTON_CLICK, NULL );
        GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_SELECTED, NULL);
        m_mouse_selected_item = this;
        return false;
    }
    else if (mouse_action == WINDOW_MOUSE_MOVE)
    {
        if (pInput->iGetAsyncKeyState(MOUSE_1) && m_mouse_selected_item && m_mouse_selected_item == this)
        {
            if (pInput->iGetAsyncKeyState(MOUSE_1, true))
                GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DRAG, NULL);
            return true;
        }
    }
    else if (mouse_action == WINDOW_LBUTTON_DB_CLICK)
    {
        GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, NULL);
        return true;
    }
    else if (mouse_action == WINDOW_RBUTTON_DOWN)
    {
        GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_RBUTTON_CLICK, NULL);
        return true;
    }

    m_mouse_selected_item = NULL;
    return false;
};

bool CUICellItem::OnKeyboard(int dik, EUIMessages keyboard_action)
{
    if (WINDOW_KEY_PRESSED == keyboard_action)
    {
        if (GetAccelerator() == dik)
        {
            GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, NULL);
            return true;
        }
    }
    return inherited::OnKeyboard(dik, keyboard_action);
}

CUIDragItem* CUICellItem::CreateDragItem()
{
    CUIDragItem* tmp;
    tmp = xr_new<CUIDragItem>(this);
    Frect r;
    GetAbsoluteRect(r);
    // NLC: a rotated bag item starts from its plain size, SetRotated below turns it
    const bool layout_list = m_pParentList && !m_pParentList->GetVerticalPlacement();
    if (layout_list && m_rotated)
    {
        r.x2 = r.x1 + float(m_grid_size.x * m_pParentList->CellSize().x);
        r.y2 = r.y1 + float(m_grid_size.y * m_pParentList->CellSize().y);
    }
    else if (m_UIStaticItem.GetFixedLTWhileHeading())
    {
        float t1, t2;
        t1 = r.width();
        t2 = r.height() * UI()->get_current_kx();

        Fvector2 cp = GetUICursor()->GetCursorPosition();

        r.x1 = (cp.x - t2 / 2.0f);
        r.y1 = (cp.y - t1 / 2.0f);
        r.x2 = r.x1 + t2;
        r.y2 = r.y1 + t1;
    }
    tmp->Init(GetShader(), r, GetUIStaticItem().GetOriginalRect());
    if (layout_list)
    {
        tmp->InitLayoutGeometry(m_grid_size, m_pParentList->CellSize());
        if (m_rotated)
            tmp->SetRotated(true);
    }
    return tmp;
}

// NLC: the heading a rotated item draws with. CUICustomItem::Render scales the rotated width by kx
// on widescreen; weapon slots were tuned around that, a bag cell must fill its footprint.
static float rotated_width_scale() { return UI()->is_widescreen() ? 1.f / UI()->get_current_kx() : 1.f; }

void CUICellItem::SetRotated(bool b)
{
    m_rotated = b;
    if (b)
    {
        EnableHeading(true);
        SetHeading(BAG_ROTATED_HEADING);
    }
    else if (!m_pParentList || !m_pParentList->GetVerticalPlacement())
        EnableHeading(false);
    SetHeadingWidthScale(b ? rotated_width_scale() : 1.f);
}

void CUICellItem::SetOwnerList(CUIDragDropListEx* p) { m_pParentList = p; }

bool CUICellItem::EqualTo(CUICellItem* itm) { return (m_grid_size.x == itm->GetGridSize().x) && (m_grid_size.y == itm->GetGridSize().y); }

u32 CUICellItem::ChildsCount() { return m_childs.size(); }

void CUICellItem::PushChild(CUICellItem* c)
{
    R_ASSERT(c->ChildsCount() == 0);
    R_ASSERT(!c->GetParent());
    R_ASSERT(this != c);

    m_childs.push_back(c);
    UpdateItemText();
}

CUICellItem* CUICellItem::PopChild()
{
    CUICellItem* itm = m_childs.back();
    m_childs.pop_back();
    std::swap(itm->m_pData, m_pData);

    UpdateItemText();

    R_ASSERT(itm->ChildsCount() == 0);
    R_ASSERT(!itm->GetParent());
    R_ASSERT(this != itm);
 
    itm->SetOwnerList(nullptr);
    itm->SetParent(nullptr);

    return itm;
}

bool CUICellItem::HasChild(CUICellItem* item) { return (m_childs.end() != std::find(m_childs.begin(), m_childs.end(), item)); }

void CUICellItem::UpdateItemText()
{
    string32 str;
    if (Core.Features.test(xrCore::Feature::show_inv_item_condition))
    {
        if (ChildsCount())
        {
            sprintf_s(str, "x%d", ChildsCount() + 1);
            m_text->SetText(str);
            m_text->Show(true);
        }
        else
        {
            sprintf_s(str, "");
            m_text->Show(false);
        }
    }
    else
    {
        if (ChildsCount())
            sprintf_s(str, "x%d", ChildsCount() + 1);
        else
            sprintf_s(str, "");

        SetText(str);
    }
}

void CUICellItem::init()
{
    static CUIXml uiXml;
    static bool is_xml_ready = false;
    if (!is_xml_ready)
    {
        bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, "inventory_new.xml");
        R_ASSERT3(xml_result, "file parsing error ", uiXml.m_xml_file_name);
        is_xml_ready = true;
    }

    m_text = xr_new<CUIStatic>();
    m_text->SetAutoDelete(true);
    AttachChild(m_text);
    CUIXmlInit::InitStatic(uiXml, "cell_item_text", 0, m_text);
    m_text->Show(false);

    m_pConditionState = xr_new<CUIProgressBar>();
    m_pConditionState->SetAutoDelete(true);
    AttachChild(m_pConditionState);
    CUIXmlInit::InitProgressBar(uiXml, "condition_progess_bar", 0, m_pConditionState);
    m_pConditionState->Show(true);
}

void CUICellItem::UpdateConditionProgressBar()
{
    if (!m_pData)
        return;

    if (m_pParentList && m_pParentList->GetConditionProgBarVisibility())
    {
        PIItem itm = (PIItem)m_pData;
        CWeapon* pWeapon = smart_cast<CWeapon*>(itm);
        CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(itm);

        if (pWeapon || pOutfit || itm->GetInvShowCondition())
        {
            Ivector2 itm_grid_size = GetGridSize();
            if (m_pParentList->GetVerticalPlacement() || m_rotated) // NLC
                std::swap(itm_grid_size.x, itm_grid_size.y);

            const Ivector2& cell_size = m_pParentList->CellSize();
            const Ivector2& cell_space = m_pParentList->CellsSpacing();
            float x = 1.f;
            if (m_condition_auto_width || fis_zero(m_pConditionState->GetWidth()))
            {
                x = 2.f;
                m_pConditionState->SetWidth((float)cell_size.x - 4.f);
                m_condition_auto_width = true;
            }
            else
                m_condition_auto_width = false;
            float y = itm_grid_size.y * (cell_size.y + cell_space.y) - m_pConditionState->GetHeight() - 2.f;
            m_pConditionState->SetWndPos({x, y});
            m_pConditionState->SetProgressPos(itm->GetCondition() * 100.0f);
            m_pConditionState->Show(true);
            return;
        }
    }
    m_pConditionState->Show(false);
}

CUIDragItem::CUIDragItem(CUICellItem* parent)
{
    m_back_list = NULL;
    m_pParent = parent;
    AttachChild(&m_static);
    Device.seqRender.Add(this, REG_PRIORITY_LOW - 5000);
    Device.seqFrame.Add(this, REG_PRIORITY_LOW - 5000);
    VERIFY(m_pParent->GetMessageTarget());
}

CUIDragItem::~CUIDragItem()
{
    Device.seqRender.Remove(this);
    Device.seqFrame.Remove(this);
}

void CUIDragItem::Init(const ui_shader& sh, const Frect& rect, const Frect& text_rect)
{
    SetWndRect(rect);
    m_static.SetShader(sh);
    m_static.SetOriginalRect(text_rect);
    m_static.SetWndPos(0.0f, 0.0f);
    m_static.SetWndSize(GetWndSize());
    m_static.TextureAvailable(true);
    m_static.TextureOn();
    m_static.SetColor(color_rgba(255, 255, 255, 170));
    m_static.SetStretchTexture(true);
    m_pos_offset.sub(rect.lt, GetUICursor()->GetCursorPosition());
}

bool CUIDragItem::OnMouse(float x, float y, EUIMessages mouse_action)
{
    if (mouse_action == WINDOW_LBUTTON_UP)
    {
        m_pParent->GetMessageTarget()->SendMessage(m_pParent, DRAG_DROP_ITEM_DROP, NULL);
        return true;
    }
    // NLC: Ctrl+wheel picks how much of the cell goes, by ten with Shift
    if ((mouse_action == WINDOW_MOUSE_WHEEL_UP || mouse_action == WINDOW_MOUSE_WHEEL_DOWN) && (pInput->iGetAsyncKeyState(DIK_LCONTROL) || pInput->iGetAsyncKeyState(DIK_RCONTROL)))
    {
        ChangeAmount(mouse_action == WINDOW_MOUSE_WHEEL_UP ? 1 : -1);
        return true;
    }
    // NLC: RMB rotates, the wheel scrolls the list under the cursor
    if (mouse_action == WINDOW_RBUTTON_DOWN)
    {
        if (CanRotate())
            SetRotated(!m_rotated);
        return true;
    }
    if ((mouse_action == WINDOW_MOUSE_WHEEL_UP || mouse_action == WINDOW_MOUSE_WHEEL_DOWN) && m_back_list)
    {
        m_back_list->ScrollByWheel(mouse_action);
        return true;
    }
    return false;
}

void CUIDragItem::InitLayoutGeometry(const Ivector2& grid, const Ivector2& cell)
{
    m_layout_geometry = true;
    m_layout_grid = grid;
    m_layout_cell = cell;
}

Ivector2 CUIDragItem::GridFootprint() const
{
    Ivector2 fp = m_layout_grid;
    if (m_rotated)
        std::swap(fp.x, fp.y);
    return fp;
}

bool CUIDragItem::CanRotate()
{
    if (!m_layout_geometry || m_layout_grid.x == m_layout_grid.y || !psInvLayoutFlags.test(ILF_ROTATE))
        return false;
    CUIDragDropListEx* owner = m_pParent->OwnerList();
    if (!owner || owner->LayoutMode() != CUIDragDropListEx::lmManual)
        return false;
    // the turned item has to fit the bag width
    const int width = m_rotated ? m_layout_grid.x : m_layout_grid.y;
    return width <= owner->CellsCapacity().x;
}

void CUIDragItem::SetRotated(bool b)
{
    m_rotated = b;
    const Ivector2 fp = GridFootprint();
    const Fvector2 size{float(fp.x * m_layout_cell.x), float(fp.y * m_layout_cell.y)};
    SetWndSize(size);
    m_static.SetWndSize(size);
    m_static.EnableHeading(b);
    if (b)
    {
        m_static.SetHeading(BAG_ROTATED_HEADING);
        m_static.SetHeadingPivot(Fvector2().set(0.0f, 0.0f), Fvector2().set(size.x, 0.0f), true);
    }
    else
        m_static.ResetHeadingPivot();
    m_static.SetHeadingWidthScale(b ? rotated_width_scale() : 1.f);
    m_pos_offset.set(-size.x * 0.5f, -size.y * 0.5f); // keep it under the cursor
    m_pParent->OnDragRotated(this);
}

void CUIDragItem::ChangeAmount(int delta)
{
    const u32 total = m_pParent->DragAmountTotal();
    CUIDragDropListEx* owner = m_pParent->OwnerList();
    if (!total || !owner || !owner->m_f_item_amount || !owner->m_f_item_amount(m_pParent))
        return;

    if (pInput->iGetAsyncKeyState(DIK_LSHIFT) || pInput->iGetAsyncKeyState(DIK_RSHIFT))
        delta *= 10;
    int amount = int(m_amount ? m_amount : total) + delta;
    clamp(amount, 1, int(total));
    m_amount = u32(amount) == total ? 0 : u32(amount);

    if (!m_amount_text)
    {
        m_amount_text = xr_new<CUIStatic>();
        m_amount_text->SetAutoDelete(true);
        m_amount_text->SetWndRect(Frect().set(0.0f, 0.0f, 100.0f, 20.0f));
        m_amount_text->SetTextColor(color_rgba(255, 255, 255, 255));
        m_static.AttachChild(m_amount_text);
    }
    string32 text{};
    if (m_amount)
        sprintf_s(text, "%u/%u", m_amount, total);
    m_amount_text->SetText(text);
}

void CUIDragItem::OnRender() { Draw(); }

void CUIDragItem::OnFrame() { Update(); }

void CUIDragItem::Draw()
{
    Fvector2 tmp;
    tmp.sub(GetWndPos(), GetUICursor()->GetCursorPosition());
    tmp.sub(m_pos_offset);
    tmp.mul(-1.0f);
    MoveWndDelta(tmp);
    UI()->PushScissor(UI()->ScreenRect(), true);

    inherited::Draw();

    UI()->PopScissor();
}

void CUIDragItem::SetBackList(CUIDragDropListEx* l)
{
    if (m_back_list != l)
    {
        m_back_list = l;
    }
}

Fvector2 CUIDragItem::GetPosition() { return Fvector2().add(m_pos_offset, GetUICursor()->GetCursorPosition()); }

void CUICellItem::Update()
{
    EnableHeading(m_pParentList->GetVerticalPlacement() || m_rotated); // NLC
    if (Heading())
    {
        SetHeading(m_rotated ? BAG_ROTATED_HEADING : 90.0f * (PI / 180.0f)); // NLC
        SetHeadingPivot(Fvector2().set(0.0f, 0.0f), m_rotated ? Fvector2().set(GetWndSize().x, 0.0f) : Fvector2().set(0.0f, GetWndSize().y), true);
    }
    else
        ResetHeadingPivot();

    inherited::Update();
    m_b_already_drawn = false;
}

void CUICellItem::ColorizeItems(std::initializer_list<CUIDragDropListEx*> args)
{
    auto inventoryitem = static_cast<CInventoryItem*>(this->m_pData);
    if (!inventoryitem)
        return;

    static const bool colorize_ammo = Core.Features.test(xrCore::Feature::colorize_ammo);
    static const u32 Color = READ_IF_EXISTS(pSettings, r_color, "dragdrop", "color_ammo", color_argb(255, 0, 128, 0));

    auto ProcessColorize = [](CUICellItem* Itm, u32 Clr) {
        Itm->SetTextureColor(Clr);
        if (auto WpnCell = smart_cast<CUIWeaponCellItem*>(Itm))
            for (auto* Child : WpnCell->m_addons)
                if (Child)
                    Child->SetTextureColor(Clr);
    };

    for (auto* DdListEx : args)
    {
        if (!DdListEx)
            continue;
        DdListEx->clear_select_armament();
        if (!colorize_ammo)
            continue;
        for (u32 i = 0, item_count = DdListEx->ItemsCount(); i < item_count; ++i)
        {
            CUICellItem* CellItem = DdListEx->GetItemIdx(i);
            if (CellItem->GetTextureColor() == Color)
            {
                if (CellItem->m_select_untradable)
                    ProcessColorize(CellItem, static_cast<CInventoryItem*>(CellItem->m_pData)->ClrUntradable);
                else if (CellItem->m_select_equipped)
                    ProcessColorize(CellItem, static_cast<CInventoryItem*>(CellItem->m_pData)->ClrEquipped);
                else
                    ProcessColorize(CellItem, 0xffffffff);
            }
        }
    }

    xr_vector<shared_str> ColorizeSects;

    auto WpnScanner = [&ColorizeSects](CWeaponMagazined* Wpn) {
        ColorizeSects.clear();

        std::copy(Wpn->m_ammoTypes.begin(), Wpn->m_ammoTypes.end(), std::back_inserter(ColorizeSects));
        if (auto WpnGl = smart_cast<CWeaponMagazinedWGrenade*>(Wpn); WpnGl && WpnGl->IsGrenadeLauncherAttached())
            std::copy(WpnGl->m_ammoTypes2.begin(), WpnGl->m_ammoTypes2.end(), std::back_inserter(ColorizeSects));
        if (Wpn->SilencerAttachable())
            ColorizeSects.push_back(Wpn->GetSilencerName());
        if (Wpn->ScopeAttachable())
            ColorizeSects.push_back(Wpn->GetScopeName());
        if (Wpn->GrenadeLauncherAttachable())
            ColorizeSects.push_back(Wpn->GetGrenadeLauncherName());
        std::copy(Wpn->m_highlightAddons.begin(), Wpn->m_highlightAddons.end(), std::back_inserter(ColorizeSects));
    };

    auto ColorizeAmmoAddons = [&] {
        for (auto* DdListEx : args)
        {
            if (!DdListEx)
                continue;
            for (u32 i = 0, item_count = DdListEx->ItemsCount(); i < item_count; ++i)
            {
                CUICellItem* CellItem = DdListEx->GetItemIdx(i);
                auto invitem = static_cast<CInventoryItem*>(CellItem->m_pData);
                if (invitem && std::find(ColorizeSects.begin(), ColorizeSects.end(), invitem->object().cNameSect()) != ColorizeSects.end())
                {
                    CellItem->m_select_armament = true;
                    if (colorize_ammo)
                        ProcessColorize(CellItem, Color);
                }
            }
        }
    };

    auto ColorizeWeapons = [&](const shared_str& Sect) {
        for (auto* DdListEx : args)
        {
            if (!DdListEx)
                continue;
            for (u32 i = 0, item_count = DdListEx->ItemsCount(); i < item_count; ++i)
            {
                CUICellItem* CellItem = DdListEx->GetItemIdx(i);
                auto invitem = static_cast<CInventoryItem*>(CellItem->m_pData);
                if (invitem)
                {
                    if (auto Wpn = smart_cast<CWeaponMagazined*>(invitem))
                    {
                        WpnScanner(Wpn);
                        if (std::find(ColorizeSects.begin(), ColorizeSects.end(), Sect) != ColorizeSects.end())
                        {
                            CellItem->m_select_armament = true;
                            if (colorize_ammo)
                                ProcessColorize(CellItem, Color);
                        }
                    }
                }
            }
        }
    };

    //Подкраска выбранного предмета
    if (colorize_ammo && this->m_select_armament)
        ProcessColorize(this, Color);

    if (auto Wpn = smart_cast<CWeaponMagazined*>(inventoryitem))
    {
        static const CLASS_ID binoc_clsid = TEXT2CLSID("WP_BINOC");
        if (Wpn->CLS_ID != binoc_clsid)
        {
            WpnScanner(Wpn);
            ColorizeAmmoAddons();
        }
    }
    else
    { //Надо подумать, какое условие тут сделать. Аддоны например, могут быть не именно аддонами, а фейк-предметами, например. Лушчше наверно вообще без каких-либо условий.
        ColorizeWeapons(inventoryitem->object().cNameSect());
    }
}
