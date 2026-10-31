#include "stdafx.h"
#include "UITradeWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"

#include "../Entity.h"
#include "../HUDManager.h"
#include "../WeaponAmmo.h"
#include "../Actor.h"
#include "../Trade.h"
#include "../UIGameSP.h"
#include "UIInventoryUtilities.h"
#include "../inventoryowner.h"
#include "../eatable_item.h"
#include "../inventory.h"
#include "../level.h"
#include "../string_table.h"
#include "../character_info.h"
#include "UIMultiTextStatic.h"
#include "UI3tButton.h"
#include "UIItemInfo.h"

#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"
#include "UIInventoryLayout.h" // NLC

#include "../game_object_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "../xr_3da/xr_input.h"
#include "../ai_space.h"
#include "../alife_simulator.h"
#include "../ai_object_location.h"
#include "../xrServer_Objects_ALife_Items.h"
#include <format>

#define TRADE_XML "trade.xml"
#define TRADE_CHARACTER_XML "trade_character.xml"
#define TRADE_ITEM_XML "trade_item.xml"

struct CUITradeInternal
{
    CUIStatic UIStaticTop;
    CUIStatic UIStaticBottom;

    CUIStatic UIOurBagWnd;
    CUIStatic UIOurMoneyStatic;
    CUIStatic UIOthersBagWnd;
    CUIStatic UIOtherMoneyStatic;
    CUIDragDropListEx UIOurBagList;
    CUIDragDropListEx UIOthersBagList;

    CUIStatic UIOurTradeWnd;
    CUIStatic UIOthersTradeWnd;
    CUIMultiTextStatic UIOurPriceCaption;
    CUIMultiTextStatic UIOthersPriceCaption;
    CUIDragDropListEx UIOurTradeList;
    CUIDragDropListEx UIOthersTradeList;

    // NLC: a rebuild keeps the deal: ids in the trade lists, also of split boxes on their way. [0] ours, [1] the other's
    xr_set<u16> keep_in_trade[2];
    xr_set<u16> leave_trade[2];

    //кнопки
    CUI3tButton UIPerformTradeButton;
    CUI3tButton UIToTalkButton;
    CUI3tButton* UISortButton{}; // NLC: optional <sort_button> node

    //информация о персонажах
    CUIStatic UIOurIcon;
    CUIStatic UIOthersIcon;
    CUICharacterInfo UICharacterInfoLeft;
    CUICharacterInfo UICharacterInfoRight;

    //информация о перетаскиваемом предмете
    CUIStatic UIDescWnd;
    CUIItemInfo UIItemInfo;

    SDrawStaticStruct* UIDealMsg;
};

bool others_zero_trade;

CUITradeWnd::CUITradeWnd() : m_bDealControlsVisible(false), m_pTrade(NULL), m_pOthersTrade(NULL), bStarted(false)
{
    m_uidata = xr_new<CUITradeInternal>();
    Init();
    Hide();
    SetCurrentItem(NULL);
    others_zero_trade = !!READ_IF_EXISTS(pSettings, r_bool, "trade", "others_zero_trade", false);
}

CUITradeWnd::~CUITradeWnd()
{
    m_uidata->UIOurBagList.ClearAll(true);
    m_uidata->UIOurTradeList.ClearAll(true);
    m_uidata->UIOthersBagList.ClearAll(true);
    m_uidata->UIOthersTradeList.ClearAll(true);
    xr_delete(m_uidata);
}

void CUITradeWnd::Init()
{
    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, TRADE_XML);
    R_ASSERT3(xml_result, "xml file not found", TRADE_XML);
    CUIXmlInit xml_init;

    xml_init.InitWindow(uiXml, "main", 0, this);

    //статические элементы интерфейса
    AttachChild(&m_uidata->UIStaticTop);
    xml_init.InitStatic(uiXml, "top_background", 0, &m_uidata->UIStaticTop);
    AttachChild(&m_uidata->UIStaticBottom);
    xml_init.InitStatic(uiXml, "bottom_background", 0, &m_uidata->UIStaticBottom);

    //иконки с изображение нас и партнера по торговле
    AttachChild(&m_uidata->UIOurIcon);
    xml_init.InitStatic(uiXml, "static_icon", 0, &m_uidata->UIOurIcon);
    AttachChild(&m_uidata->UIOthersIcon);
    xml_init.InitStatic(uiXml, "static_icon", 1, &m_uidata->UIOthersIcon);
    m_uidata->UIOurIcon.AttachChild(&m_uidata->UICharacterInfoLeft);
    m_uidata->UICharacterInfoLeft.Init(0, 0, m_uidata->UIOurIcon.GetWidth(), m_uidata->UIOurIcon.GetHeight(), TRADE_CHARACTER_XML);
    m_uidata->UIOthersIcon.AttachChild(&m_uidata->UICharacterInfoRight);
    m_uidata->UICharacterInfoRight.Init(0, 0, m_uidata->UIOthersIcon.GetWidth(), m_uidata->UIOthersIcon.GetHeight(), TRADE_CHARACTER_XML);

    //Списки торговли
    AttachChild(&m_uidata->UIOurBagWnd);
    xml_init.InitStatic(uiXml, "our_bag_static", 0, &m_uidata->UIOurBagWnd);
    AttachChild(&m_uidata->UIOthersBagWnd);
    xml_init.InitStatic(uiXml, "others_bag_static", 0, &m_uidata->UIOthersBagWnd);

    m_uidata->UIOurBagWnd.AttachChild(&m_uidata->UIOurMoneyStatic);
    xml_init.InitStatic(uiXml, "our_money_static", 0, &m_uidata->UIOurMoneyStatic);

    m_uidata->UIOthersBagWnd.AttachChild(&m_uidata->UIOtherMoneyStatic);
    xml_init.InitStatic(uiXml, "other_money_static", 0, &m_uidata->UIOtherMoneyStatic);

    AttachChild(&m_uidata->UIOurTradeWnd);
    xml_init.InitStatic(uiXml, "static", 0, &m_uidata->UIOurTradeWnd);
    AttachChild(&m_uidata->UIOthersTradeWnd);
    xml_init.InitStatic(uiXml, "static", 1, &m_uidata->UIOthersTradeWnd);

    m_uidata->UIOurTradeWnd.AttachChild(&m_uidata->UIOurPriceCaption);
    xml_init.InitMultiTextStatic(uiXml, "price_mt_static", 0, &m_uidata->UIOurPriceCaption);

    m_uidata->UIOthersTradeWnd.AttachChild(&m_uidata->UIOthersPriceCaption);
    xml_init.InitMultiTextStatic(uiXml, "price_mt_static", 0, &m_uidata->UIOthersPriceCaption);

    //Списки Drag&Drop
    m_uidata->UIOurBagWnd.AttachChild(&m_uidata->UIOurBagList);
    xml_init.InitDragDropListEx(uiXml, "dragdrop_list", 0, &m_uidata->UIOurBagList);

    m_uidata->UIOthersBagWnd.AttachChild(&m_uidata->UIOthersBagList);
    xml_init.InitDragDropListEx(uiXml, "dragdrop_list", 1, &m_uidata->UIOthersBagList);

    m_uidata->UIOurTradeWnd.AttachChild(&m_uidata->UIOurTradeList);
    xml_init.InitDragDropListEx(uiXml, "dragdrop_list", 2, &m_uidata->UIOurTradeList);

    m_uidata->UIOthersTradeWnd.AttachChild(&m_uidata->UIOthersTradeList);
    xml_init.InitDragDropListEx(uiXml, "dragdrop_list", 3, &m_uidata->UIOthersTradeList);

    AttachChild(&m_uidata->UIDescWnd);
    xml_init.InitStatic(uiXml, "desc_static", 0, &m_uidata->UIDescWnd);
    m_uidata->UIDescWnd.AttachChild(&m_uidata->UIItemInfo);
    m_uidata->UIItemInfo.Init(0, 0, m_uidata->UIDescWnd.GetWidth(), m_uidata->UIDescWnd.GetHeight(), TRADE_ITEM_XML);

    xml_init.InitAutoStatic(uiXml, "auto_static", this);

    AttachChild(&m_uidata->UIPerformTradeButton);
    xml_init.Init3tButton(uiXml, "button", 0, &m_uidata->UIPerformTradeButton);

    AttachChild(&m_uidata->UIToTalkButton);
    xml_init.Init3tButton(uiXml, "button", 1, &m_uidata->UIToTalkButton);

    if (uiXml.NavigateToNode("sort_button", 0)) // NLC
    {
        m_uidata->UISortButton = xr_new<CUI3tButton>();
        m_uidata->UISortButton->SetAutoDelete(true);
        AttachChild(m_uidata->UISortButton);
        xml_init.Init3tButton(uiXml, "sort_button", 0, m_uidata->UISortButton);
        InitSortButtonIcon(uiXml, "sort_button", m_uidata->UISortButton);
    }

    m_pUIPropertiesBox = xr_new<CUIPropertiesBox>();
    m_pUIPropertiesBox->SetAutoDelete(true);
    AttachChild(m_pUIPropertiesBox);
    m_pUIPropertiesBox->Init(0, 0, 300, 300);
    m_pUIPropertiesBox->Hide();

    m_uidata->UIDealMsg = NULL;

    BindDragDropListEvents(&m_uidata->UIOurBagList);
    BindDragDropListEvents(&m_uidata->UIOthersBagList);
    BindDragDropListEvents(&m_uidata->UIOurTradeList);
    BindDragDropListEvents(&m_uidata->UIOthersTradeList);

    // Load sounds
    if (uiXml.NavigateToNode("action_sounds", 0))
    {
        XML_NODE* stored_root = uiXml.GetLocalRoot();
        uiXml.SetLocalRoot(uiXml.NavigateToNode("action_sounds", 0));

        create_ui_snd(sounds[eInvProperties], uiXml.Read("snd_properties", 0, nullptr));
        create_ui_snd(sounds[eInvItemMove], uiXml.Read("snd_item_move", 0, nullptr));
        create_ui_snd(sounds[eInvSndTrade], uiXml.Read("snd_trade_completed", 0, nullptr));
        create_ui_snd(sounds[eInvSndTradeBlock], uiXml.Read("snd_trade_blocked", 0, nullptr));

        uiXml.SetLocalRoot(stored_root);
    }
}

void CUITradeWnd::InitTrade(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{
    VERIFY(pOur);
    VERIFY(pOthers);

    m_pInvOwner = pOur;
    m_pOthersInvOwner = pOthers;
    m_uidata->UIOthersPriceCaption.GetPhraseByIndex(0)->SetText(*CStringTable().translate("ui_st_opponent_items"));

    m_uidata->UICharacterInfoLeft.InitCharacter(m_pInvOwner->object_id());
    m_uidata->UICharacterInfoRight.InitCharacter(m_pOthersInvOwner->object_id());

    m_pInv = &m_pInvOwner->inventory();
    m_pOthersInv = pOur->GetTrade()->GetPartnerInventory();

    m_pTrade = pOur->GetTrade();
    m_pOthersTrade = pOur->GetTrade()->GetPartnerTrade();

    m_pUIPropertiesBox->Hide();

    EnableAll();

    UpdateLists(eBoth);
}

void CUITradeWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
    if (pWnd == &m_uidata->UIToTalkButton && msg == BUTTON_CLICKED)
    {
        SwitchToTalk();
    }
    else if (pWnd == &m_uidata->UIPerformTradeButton && msg == BUTTON_CLICKED)
    {
        PerformTrade();
    }
    else if (m_uidata->UISortButton && pWnd == m_uidata->UISortButton && msg == BUTTON_CLICKED) // NLC
    {
        inv_layout().ResetPositions();
        UpdateLists(e1st);
        inv_layout().Commit(&m_uidata->UIOurBagList);
    }
    else if (pWnd == m_pUIPropertiesBox && msg == PROPERTY_CLICKED)
    {
        if (m_pUIPropertiesBox->GetClickedItem())
        {
            switch (m_pUIPropertiesBox->GetClickedItem()->GetTAG())
            {
            case INVENTORY_MOVE_ACTION: {
                void* d = m_pUIPropertiesBox->GetClickedItem()->GetData();
                bool b_all = (d == (void*)33);

                if (b_all)
                    MoveItems(CurrentItem());
                else
                    MoveItem(CurrentItem());
            }
            break;
            }
        }
    }

    CUIWindow::SendMessage(pWnd, msg, pData);
}

void CUITradeWnd::Draw()
{
    inherited::Draw();
    if (m_uidata->UIDealMsg)
        m_uidata->UIDealMsg->Draw();
}

extern void UpdateCameraDirection(CGameObject* pTo);

void CUITradeWnd::Update()
{
    EListType et = eNone;

    if (m_pInv->ModifyFrame() == Device.dwFrame && m_pOthersInv->ModifyFrame() == Device.dwFrame)
    {
        et = eBoth;
    }
    else if (m_pInv->ModifyFrame() == Device.dwFrame)
    {
        et = e1st;
    }
    else if (m_pOthersInv->ModifyFrame() == Device.dwFrame)
    {
        et = e2nd;
    }
    if (m_rebuild != eNone) // NLC
    {
        et = et == eNone || et == m_rebuild ? m_rebuild : eBoth;
        m_rebuild = eNone;
    }
    if (et != eNone)
    {
        // NLC: the cells these point at are about to go
        SetCurrentItem(NULL);
        if (m_pUIPropertiesBox->IsShown())
            m_pUIPropertiesBox->Hide();
        UpdateLists(et);
    }

    inherited::Update();
    UpdateCameraDirection(smart_cast<CGameObject*>(m_pOthersInvOwner));

    if (m_uidata->UIDealMsg)
    {
        m_uidata->UIDealMsg->Update();
        if (!m_uidata->UIDealMsg->IsActual())
        {
            HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
            HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
            m_uidata->UIDealMsg = NULL;
        }
    }
}

#include "UIInventoryUtilities.h"
void CUITradeWnd::Show()
{
    InventoryUtilities::SendInfoToActor("ui_trade");
    inherited::Show(true);
    inherited::Enable(true);

    SetCurrentItem(NULL);
    ResetAll();
    m_uidata->UIDealMsg = NULL;

    if (Core.Features.test(xrCore::Feature::engine_ammo_repacker) && !Core.Features.test(xrCore::Feature::hard_ammo_reload))
        if (auto pActor = Actor())
            pActor->RepackAmmo();
}

void CUITradeWnd::Hide()
{
    InventoryUtilities::SendInfoToActor("ui_trade_hide");
    inherited::Show(false);
    inherited::Enable(false);
    if (bStarted)
        StopTrade();

    m_uidata->UIDealMsg = NULL;

    if (HUD().GetUI()->UIGame())
    {
        HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
        HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
    }

    inv_layout().Commit(&m_uidata->UIOurBagList); // NLC
    m_rebuild = eNone; // NLC
    for (int side = 0; side < 2; ++side)
    {
        m_uidata->keep_in_trade[side].clear();
        m_uidata->leave_trade[side].clear();
    }
    m_uidata->UIOurBagList.ClearAll(true);
    m_uidata->UIOurTradeList.ClearAll(true);
    m_uidata->UIOthersBagList.ClearAll(true);
    m_uidata->UIOthersTradeList.ClearAll(true);
}

void CUITradeWnd::StartTrade()
{
    if (m_pTrade)
        m_pTrade->TradeCB(true);
    if (m_pOthersTrade)
        m_pOthersTrade->TradeCB(true);
    bStarted = true;
}

void CUITradeWnd::StopTrade()
{
    if (m_pTrade)
        m_pTrade->TradeCB(false);
    if (m_pOthersTrade)
        m_pOthersTrade->TradeCB(false);
    bStarted = false;
}

#include "../trade_parameters.h"
bool CUITradeWnd::CanMoveToOther(PIItem pItem, bool our)
{
    if (pItem->m_flags.test(CInventoryItem::FIAlwaysUntradable))
        return false;
    if (!our)
        return true;

    float r1 = CalcItemsWeight(&m_uidata->UIOurTradeList); // our
    float r2 = CalcItemsWeight(&m_uidata->UIOthersTradeList); // other

    float itmWeight = pItem->Weight();
    float otherInvWeight = m_pOthersInv->CalcTotalWeight();
    float otherMaxWeight = m_pOthersInv->GetMaxWeight();

    if (!m_pOthersInvOwner->trade_parameters().enabled(CTradeParameters::action_buy(0), pItem->object().cNameSect()))
        return (false);

    if (pItem->GetCondition() < m_pOthersInvOwner->trade_parameters().factors(CTradeParameters::action_buy(0), pItem->object().cNameSect()).min_condition())
        return false;

    if (otherInvWeight - r2 + r1 + itmWeight > otherMaxWeight)
        return false;

    return true;
}

void move_item(CUICellItem* itm, CUIDragDropListEx* from, CUIDragDropListEx* to)
{
    CUICellItem* _itm = from->RemoveItem(itm, false);
    to->SetItem(_itm);
}

bool CUITradeWnd::ToOurTrade(CUICellItem* itm)
{
    if (!CanMoveToOther((PIItem)m_pCurrentCellItem->m_pData, true))
        return false;

    move_item(itm, &m_uidata->UIOurBagList, &m_uidata->UIOurTradeList);
    UpdatePrices();
    return true;
}

bool CUITradeWnd::ToOthersTrade(CUICellItem* itm)
{
    if (!CanMoveToOther((PIItem)m_pCurrentCellItem->m_pData, false))
        return false;

    move_item(itm, &m_uidata->UIOthersBagList, &m_uidata->UIOthersTradeList);
    UpdatePrices();

    return true;
}

bool CUITradeWnd::ToOurBag(CUICellItem* itm)
{
    move_item(itm, &m_uidata->UIOurTradeList, &m_uidata->UIOurBagList);
    UpdatePrices();

    return true;
}

bool CUITradeWnd::ToOthersBag(CUICellItem* itm)
{
    move_item(itm, &m_uidata->UIOthersTradeList, &m_uidata->UIOthersBagList);
    UpdatePrices();

    return true;
}

float CUITradeWnd::CalcItemsWeight(CUIDragDropListEx* pList)
{
    float res = 0.0f;

    for (u32 i = 0; i < pList->ItemsCount(); ++i)
    {
        CUICellItem* itm = pList->GetItemIdx(i);
        PIItem iitem = (PIItem)itm->m_pData;
        res += iitem->Weight();
        for (u32 j = 0; j < itm->ChildsCount(); ++j)
        {
            PIItem jitem = (PIItem)itm->Child(j)->m_pData;
            res += jitem->Weight();
        }
    }
    return res;
}

u32 CUITradeWnd::CalcItemsPrice(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying)
{
    u32 iPrice = 0;
    float ammo_price = 0.f; // NLC: summed before rounding, or splitting a box would change what the rounds are worth

    auto add = [&](PIItem item) {
        if (smart_cast<CWeaponAmmo*>(item))
            ammo_price += pTrade->GetItemPriceExact(item, bBuying);
        else
            iPrice += pTrade->GetItemPrice(item, bBuying);
    };

    for (u32 i = 0; i < pList->ItemsCount(); ++i)
    {
        CUICellItem* itm = pList->GetItemIdx(i);
        add((PIItem)itm->m_pData);

        for (u32 j = 0; j < itm->ChildsCount(); ++j)
            add((PIItem)itm->Child(j)->m_pData);
    }

    // the player never gains from the rounding: selling rounds down, buying up
    iPrice += u32(bBuying ? iFloor(ammo_price) : iCeil(ammo_price));
    return iPrice;
}

void CUITradeWnd::PerformTrade()
{
    if (m_uidata->UIOurTradeList.ItemsCount() == 0 && m_uidata->UIOthersTradeList.ItemsCount() == 0)
        return;

    int our_money = (int)m_pInvOwner->get_money();
    int others_money = (int)m_pOthersInvOwner->get_money();

    int delta_price = int(m_iOurTradePrice - m_iOthersTradePrice);

    our_money += delta_price;
    others_money -= (m_pOthersInvOwner->InfinitiveMoney()) ? 0 : delta_price;

    if (our_money >= 0 && others_money >= 0 && (m_iOurTradePrice >= 0 || m_iOthersTradePrice > 0))
    {
        m_pOthersTrade->OnPerformTrade(m_iOthersTradePrice, m_iOurTradePrice);

        TransferItems(&m_uidata->UIOurTradeList, &m_uidata->UIOthersBagList, m_pOthersTrade, true);
        TransferItems(&m_uidata->UIOthersTradeList, &m_uidata->UIOurBagList, m_pOthersTrade, false);
        if (others_zero_trade)
        {
            m_pOthersTrade->pThis.inv_owner->set_money(others_money, true);
            m_pOthersTrade->pPartner.inv_owner->set_money(our_money, true);
        }
    }
    else
    {
        if (others_money < 0)
            m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_other", true);
        else
            m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_mine", true);

        m_uidata->UIDealMsg->m_endTime = Device.fTimeGlobal + 2.0f; // sec
    }
    SetCurrentItem(NULL);
}

#include "../xr_level_controller.h"

bool CUITradeWnd::OnKeyboard(int dik, EUIMessages keyboard_action)
{
    if (m_pUIPropertiesBox->GetVisible())
        m_pUIPropertiesBox->OnKeyboard(dik, keyboard_action);

    if (inherited::OnKeyboard(dik, keyboard_action))
        return true;

    return false;
}

bool CUITradeWnd::OnMouse(float x, float y, EUIMessages mouse_action)
{
    if (mouse_action == WINDOW_RBUTTON_DOWN)
    {
        if (m_pUIPropertiesBox->IsShown())
        {
            m_pUIPropertiesBox->Hide();
            return true;
        }
    }

    CUIWindow::OnMouse(x, y, mouse_action);

    return true; // always returns true, because ::StopAnyMove() == true;
}

void CUITradeWnd::ActivatePropertiesBox()
{
    m_pUIPropertiesBox->RemoveAll();

    bool hasMany = CurrentItem()->ChildsCount() > 0;

    m_pUIPropertiesBox->AddItem("st_move", NULL, INVENTORY_MOVE_ACTION);

    if (hasMany)
        m_pUIPropertiesBox->AddItem("st_move_all", (void*)33, INVENTORY_MOVE_ACTION);

    m_pUIPropertiesBox->ApplyMenuConfig(); // NLC
    m_pUIPropertiesBox->AutoUpdateSize();
    m_pUIPropertiesBox->BringAllToTop();

    Fvector2 cursor_pos;
    Frect vis_rect;

    GetAbsoluteRect(vis_rect);
    cursor_pos = GetUICursor()->GetCursorPosition();
    cursor_pos.sub(vis_rect.lt);
    m_pUIPropertiesBox->Show(vis_rect, cursor_pos);
    PlaySnd(eInvProperties);
}

void CUITradeWnd::DisableAll()
{
    m_uidata->UIOurBagWnd.Enable(false);
    m_uidata->UIOthersBagWnd.Enable(false);
    m_uidata->UIOurTradeWnd.Enable(false);
    m_uidata->UIOthersTradeWnd.Enable(false);
}

void CUITradeWnd::EnableAll()
{
    m_uidata->UIOurBagWnd.Enable(true);
    m_uidata->UIOthersBagWnd.Enable(true);
    m_uidata->UIOurTradeWnd.Enable(true);
    m_uidata->UIOthersTradeWnd.Enable(true);
}

void CUITradeWnd::UpdatePrices()
{
    m_iOurTradePrice = CalcItemsPrice(&m_uidata->UIOurTradeList, m_pOthersTrade, true);
    m_iOthersTradePrice = CalcItemsPrice(&m_uidata->UIOthersTradeList, m_pOthersTrade, false);

    if (!m_pOthersInvOwner->InfinitiveMoney())
    {
        u32 others_money = m_pOthersInvOwner->get_money();
        if (others_zero_trade && m_iOurTradePrice > others_money)
            m_iOurTradePrice = others_money;
    }

    static const char* StMoneyDescr = CStringTable().translate("ui_st_money_descr").c_str();
    m_uidata->UIOurPriceCaption.GetPhraseByIndex(2)->str = std::format("{} {}", m_iOurTradePrice, StMoneyDescr).c_str();
    m_uidata->UIOthersPriceCaption.GetPhraseByIndex(2)->str = std::format("{} {}", m_iOthersTradePrice, StMoneyDescr).c_str();
    m_uidata->UIOurMoneyStatic.SetText(std::format("{} {}", m_pInvOwner->get_money(), StMoneyDescr).c_str());
    m_uidata->UIOtherMoneyStatic.SetText(m_pOthersInvOwner->InfinitiveMoney() ? "---" : std::format("{} {}", m_pOthersInvOwner->get_money(), StMoneyDescr).c_str());
}

void CUITradeWnd::TransferItems(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying)
{
    while (pSellList->ItemsCount())
    {
        CUICellItem* itm = pSellList->RemoveItem(pSellList->GetItemIdx(0), false);
        auto InvItm = (PIItem)itm->m_pData;
        InvItm->m_highlight_equipped = false; //Убираем подсветку после продажи предмета
        itm->m_select_equipped = false;
        pTrade->TransferItem(InvItm, bBuying, !others_zero_trade);
        pBuyList->SetItem(itm);
    }

    if (!others_zero_trade)
    {
        pTrade->pThis.inv_owner->set_money(pTrade->pThis.inv_owner->get_money(), true);
        pTrade->pPartner.inv_owner->set_money(pTrade->pPartner.inv_owner->get_money(), true);
    }
}

void CUITradeWnd::UpdateLists(EListType mode)
{
    const int our_scroll = m_uidata->UIOurBagList.ScrollPos(); // NLC: a rebuild keeps the player's place

    auto remember_deal = [this](CUIDragDropListEx& list, int side) { // NLC
        auto& keep = m_uidata->keep_in_trade[side];
        auto add = [&keep](CUICellItem* cell) { keep.insert(static_cast<PIItem>(cell->m_pData)->object().ID()); };
        for (u32 i = 0; i < list.ItemsCount(); ++i)
        {
            CUICellItem* itm = list.GetItemIdx(i);
            add(itm);
            for (u32 j = 0; j < itm->ChildsCount(); ++j)
                add(itm->Child(j));
        }
        for (u16 id : m_uidata->leave_trade[side])
            keep.erase(id);
        m_uidata->leave_trade[side].clear();
    };

    if (mode == eBoth || mode == e1st)
    {
        remember_deal(m_uidata->UIOurTradeList, 0); // NLC
        m_uidata->UIOurBagList.ClearAll(true);
        m_uidata->UIOurTradeList.ClearAll(true);
    }

    if (mode == eBoth || mode == e2nd)
    {
        remember_deal(m_uidata->UIOthersTradeList, 1); // NLC
        m_uidata->UIOthersBagList.ClearAll(true);
        m_uidata->UIOthersTradeList.ClearAll(true);
    }

    UpdatePrices();

    if (mode == eBoth || mode == e1st)
    {
        ruck_list.clear();
        m_pInv->AddAvailableItems(ruck_list, true);
        std::sort(ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck);
        inv_layout().BeginFill(&m_uidata->UIOurBagList, CInventoryLayout::wndTrade, ruck_list); // NLC
        FillList(ruck_list, m_uidata->UIOurBagList, true);
        inv_layout().EndFill(&m_uidata->UIOurBagList); // NLC
        if (m_uidata->UISortButton) // NLC
            m_uidata->UISortButton->Show(m_uidata->UIOurBagList.LayoutMode() == CUIDragDropListEx::lmManual);
    }

    if (mode == eBoth || mode == e2nd)
    {
        ruck_list.clear();
        m_pOthersInv->AddAvailableItems(ruck_list, true);
        std::sort(ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck);
        FillList(ruck_list, m_uidata->UIOthersBagList, false);
    }

    if (mode == eBoth || mode == e1st) // NLC
        m_uidata->UIOurBagList.SetScrollPos(our_scroll);

    UpdatePrices(); // NLC: the kept deal is back in the trade lists
}

void CUITradeWnd::FillList(TIItemContainer& cont, CUIDragDropListEx& dragDropList, bool our)
{
    TIItemContainer::iterator it = cont.begin();
    TIItemContainer::iterator it_e = cont.end();

    for (; it != it_e; ++it)
    {
        CInventoryItem* item = *it;
        const bool canTrade = CanMoveToOther(item, our);

        if (pSettings->line_exist("engine_callbacks", "actor_on_item_before_show_in_trade"))
        {
            const std::string callback = pSettings->r_string("engine_callbacks", "actor_on_item_before_show_in_trade");
            if (luabind::functor<bool> func; ai().script_engine().functor(callback.c_str(), func))
                if (!func(item->object().lua_game_object(), our, canTrade))
                    continue;
        }

        if ((psActorFlags.test(AF_HIDE_UNTRADABLE_ITEMS) && !canTrade) || item->m_flags.test(CInventoryItem::FIHiddenForInventory))
            continue;

        CUICellItem* itm = create_cell_item(item);
        if (item->m_highlight_equipped)
            itm->m_select_equipped = true;
        ColorizeItem(itm, canTrade, itm->m_select_equipped);

        // NLC: back into the deal it was part of
        auto& keep = m_uidata->keep_in_trade[our ? 0 : 1];
        const auto kept = keep.find(item->object().ID());
        if (kept != keep.end())
        {
            keep.erase(kept);
            (our ? m_uidata->UIOurTradeList : m_uidata->UIOthersTradeList).SetItem(itm);
        }
        else
            dragDropList.SetItem(itm);
    }
}

bool CUITradeWnd::OnItemStartDrag(CUICellItem* itm)
{
    return false; // default behaviour
}

bool CUITradeWnd::OnItemSelected(CUICellItem* itm)
{
    SetCurrentItem(itm);
    itm->ColorizeItems({&m_uidata->UIOurTradeList, &m_uidata->UIOthersTradeList, &m_uidata->UIOurBagList, &m_uidata->UIOthersBagList});
    return false;
}

bool CUITradeWnd::OnItemRButtonClick(CUICellItem* itm)
{
    SetCurrentItem(itm);
    ActivatePropertiesBox();
    return false;
}

bool CUITradeWnd::OnItemDrop(CUICellItem* itm)
{
    CUIDragDropListEx* old_owner = itm->OwnerList();
    CUIDragDropListEx* new_owner = CUIDragDropListEx::m_drag_item->BackList();
    if (old_owner == new_owner || !old_owner || !new_owner)
        return false;

    if (const u32 amount = CUIDragDropListEx::m_drag_item->ChosenAmount()) // NLC
    {
        MoveAmount(itm, amount);
    }
    else if (pInput->iGetAsyncKeyState(DIK_LSHIFT) || pInput->iGetAsyncKeyState(DIK_RSHIFT))
    {
        MoveItems(itm);
    }
    else
    {
        MoveItem(itm);
    }

    return true;
}

// NLC: a script can refuse a box whose count it manages itself
static bool can_split_ammo(PIItem item)
{
    if (!smart_cast<CWeaponAmmo*>(item))
        return false;
    if (pSettings->line_exist("engine_callbacks", "trade_ammo_can_split"))
    {
        const std::string callback = pSettings->r_string("engine_callbacks", "trade_ammo_can_split");
        if (luabind::functor<bool> func; ai().script_engine().functor(callback.c_str(), func))
            return func(item->object().lua_game_object());
    }
    return true;
}

bool CUITradeWnd::OnItemAmount(CUICellItem* itm)
{
    if (!can_split_ammo((PIItem)itm->m_pData))
        return false;
    for (u32 j = 0; j < itm->ChildsCount(); ++j)
        if (!can_split_ammo((PIItem)itm->Child(j)->m_pData))
            return false;
    return true;
}

CSE_Abstract* CALifeSimulator__spawn_ammo(CALifeSimulator* self, LPCSTR section, const Fvector& position, u32 level_vertex_id, GameGraph::_GRAPH_ID game_vertex_id,
                                          ALife::_OBJECT_ID id_parent, int ammo_to_spawn);

// Part of a box stays where it is and the rest becomes a new box of the same owner. The new box arrives
// a few frames later, so moves go by id through the next rebuild.
void CUITradeWnd::MoveAmount(CUICellItem* itm, u32 amount)
{
    CUIDragDropListEx* owner = itm->OwnerList();
    const bool our = owner == &m_uidata->UIOurBagList || owner == &m_uidata->UIOurTradeList;
    const bool to_trade = owner == &m_uidata->UIOurBagList || owner == &m_uidata->UIOthersBagList;

    if (amount >= itm->DragAmountTotal())
    {
        MoveItems(itm);
        return;
    }
    if (to_trade && !CanMoveToOther((PIItem)itm->m_pData, our))
        return;

    xr_vector<CWeaponAmmo*> boxes;
    boxes.push_back(smart_cast<CWeaponAmmo*>((PIItem)itm->m_pData));
    for (u32 j = 0; j < itm->ChildsCount(); ++j)
        boxes.push_back(smart_cast<CWeaponAmmo*>((PIItem)itm->Child(j)->m_pData));
    std::sort(boxes.begin(), boxes.end(), [](const CWeaponAmmo* a, const CWeaponAmmo* b) { return a->m_boxCurr > b->m_boxCurr; });

    const int side = our ? 0 : 1;
    auto& keep = m_uidata->keep_in_trade[side];
    auto& leave = m_uidata->leave_trade[side];

    // whole boxes first, one box splits for the rest
    CWeaponAmmo* to_split = nullptr;
    u32 rest = amount;
    for (CWeaponAmmo* box : boxes)
    {
        if (box->m_boxCurr <= rest)
        {
            rest -= box->m_boxCurr;
            if (to_trade)
                keep.insert(box->ID());
            else
                leave.insert(box->ID());
        }
        else
            to_split = box; // boxes run from big to small: the smallest that is too big
    }

    if (rest && to_split)
    {
        CGameObject* parent = smart_cast<CGameObject*>(to_split->H_Parent());
        CSE_Abstract* spawned = parent ? CALifeSimulator__spawn_ammo(const_cast<CALifeSimulator*>(ai().get_alife()), to_split->cNameSect().c_str(), parent->Position(),
                                                                     parent->ai_location().level_vertex_id(), parent->ai_location().game_vertex_id(), parent->ID(), int(rest)) :
                                         nullptr;
        if (spawned)
        {
            to_split->m_boxCurr = u16(to_split->m_boxCurr - rest);
            if (to_split->m_pCurrentInventory)
                to_split->m_pCurrentInventory->InvalidateState();
            if (to_trade)
                keep.insert(spawned->ID);
        }
        else
            Msg("! [CUITradeWnd::MoveAmount] could not split [%s]", to_split->cNameSect().c_str());
    }

    m_rebuild = our ? e1st : e2nd;
    SetCurrentItem(NULL);
    PlaySnd(eInvItemMove);
}

bool CUITradeWnd::OnItemDbClick(CUICellItem* itm)
{
    SetCurrentItem(itm);

    if (pInput->iGetAsyncKeyState(DIK_LSHIFT) || pInput->iGetAsyncKeyState(DIK_RSHIFT))
    {
        MoveItems(itm);
    }
    else
    {
        MoveItem(itm);
    }

    return true;
}

void CUITradeWnd::MoveItems(CUICellItem* itm)
{
    if (!itm)
    {
        return;
    }

    // Msg("Move all items %d", cnt);

    CUIDragDropListEx* old_owner = itm->OwnerList();
    CUIDragDropListEx* to = nullptr;

    if (old_owner == &m_uidata->UIOurBagList)
    {
        if (!CanMoveToOther((PIItem)itm->m_pData, true))
            return;

        to = &m_uidata->UIOurTradeList;
    }
    else if (old_owner == &m_uidata->UIOurTradeList)
        to = &m_uidata->UIOurBagList;
    else if (old_owner == &m_uidata->UIOthersBagList)
    {
        if (!CanMoveToOther((PIItem)itm->m_pData, false))
            return;

        to = &m_uidata->UIOthersTradeList;
    }
    else if (old_owner == &m_uidata->UIOthersTradeList)
        to = &m_uidata->UIOthersBagList;

    R_ASSERT(to != nullptr);

    u32 cnt = itm->ChildsCount();
    for (u32 i = 0; i < cnt; ++i)
    {
        CUICellItem* child_itm = itm->PopChild();

        // Msg("MoveAllItems child ... %d", i);

        to->SetItem(child_itm);
    }

    CUICellItem* _itm = old_owner->RemoveItem(itm, true);
    to->SetItem(_itm);

    UpdatePrices();
    PlaySnd(eInvItemMove);

    SetCurrentItem(NULL);
}

bool CUITradeWnd::MoveItem(CUICellItem* itm)
{
    CUIDragDropListEx* old_owner = itm->OwnerList();

    if (old_owner == &m_uidata->UIOurBagList)
        ToOurTrade(itm);
    else if (old_owner == &m_uidata->UIOurTradeList)
        ToOurBag(itm);
    else if (old_owner == &m_uidata->UIOthersBagList)
        ToOthersTrade(itm);
    else if (old_owner == &m_uidata->UIOthersTradeList)
        ToOthersBag(itm);

    PlaySnd(eInvItemMove);
    return true;
}

CUICellItem* CUITradeWnd::CurrentItem() { return m_pCurrentCellItem; }

PIItem CUITradeWnd::CurrentIItem() { return (m_pCurrentCellItem) ? (PIItem)m_pCurrentCellItem->m_pData : NULL; }

void CUITradeWnd::SetCurrentItem(CUICellItem* itm)
{
    if (m_pCurrentCellItem == itm)
        return;
    m_pCurrentCellItem = itm;
    m_uidata->UIItemInfo.InitItem(CurrentIItem());

    if (!m_pCurrentCellItem)
        return;

    m_pCurrentCellItem->m_select_armament = true;

    CUIDragDropListEx* owner = itm->OwnerList();
    bool bBuying = (owner == &m_uidata->UIOurBagList) || (owner == &m_uidata->UIOurTradeList);

    if (m_uidata->UIItemInfo.UICost)
    {
        static const char* StMoneyDescr = CStringTable().translate("ui_st_money_descr").c_str();
        m_uidata->UIItemInfo.UICost->SetText(std::format("{} {}", m_pOthersTrade->GetItemPrice(CurrentIItem(), bBuying), StMoneyDescr).c_str());
    }

    auto script_obj = CurrentIItem()->object().lua_game_object();
    g_actor->callback(GameObject::eCellItemSelect)(script_obj);
}

void CUITradeWnd::SwitchToTalk() { GetMessageTarget()->SendMessage(this, TRADE_WND_CLOSED); }

void CUITradeWnd::BindDragDropListEvents(CUIDragDropListEx* lst)
{
    lst->m_f_item_drop = fastdelegate::MakeDelegate(this, &CUITradeWnd::OnItemDrop);
    lst->m_f_item_start_drag = fastdelegate::MakeDelegate(this, &CUITradeWnd::OnItemStartDrag);
    lst->m_f_item_db_click = fastdelegate::MakeDelegate(this, &CUITradeWnd::OnItemDbClick);
    lst->m_f_item_selected = fastdelegate::MakeDelegate(this, &CUITradeWnd::OnItemSelected);
    lst->m_f_item_rbutton_click = fastdelegate::MakeDelegate(this, &CUITradeWnd::OnItemRButtonClick);
    lst->m_f_item_amount = fastdelegate::MakeDelegate(this, &CUITradeWnd::OnItemAmount); // NLC
}

void CUITradeWnd::PlaySnd(eInventorySndAction a)
{
    if (sounds[a]._handle() && !sounds[a]._feedback())
        sounds[a].play(nullptr, sm_2D);
}

void CUITradeWnd::ColorizeItem(CUICellItem* itm, bool canTrade, bool highlighted)
{
    static const bool highlight_cop_enabled = !Core.Features.test(xrCore::Feature::colorize_untradable); //Это опция для Dsh, не убирать!

    if (!canTrade)
    {
        if (highlight_cop_enabled)
            itm->m_select_untradable = true;
        itm->SetColor(static_cast<CInventoryItem*>(itm->m_pData)->ClrUntradable);
    }
    else
    {
        if (highlight_cop_enabled)
            itm->m_select_untradable = false;
        if (highlighted)
            itm->SetColor(static_cast<CInventoryItem*>(itm->m_pData)->ClrEquipped);
    }
}
