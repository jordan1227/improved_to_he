#include "stdafx.h"
#include "../fl_hook.h"
#include "UIInventoryWnd.h"
#include "../actor.h"
#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../Artifact.h"
#include "../eatable_item.h"
#include "../BottleItem.h"
#include "../WeaponMagazined.h"
#include "../WeaponMagazinedWGrenade.h"
#include "../inventory.h"
#include "../game_base.h"
#include "../game_cl_base.h"
#include "../xr_level_controller.h"
#include "UICellItem.h"
#include "UIListBoxItem.h"
#include "../CustomOutfit.h"
#include "../string_table.h"
#include <regex>
#include "../CustomDetector.h"
#include "../SimpleDetectorSHOC.h"
#include "../ai_space.h"
#include "../script_engine.h"
#include "../script_game_object.h"

void CUIInventoryWnd::EatItem(PIItem itm, u8 body_part)
{
    SetCurrentItem(NULL);
    if (!itm->Useful())
        return;

    SendEvent_Item_Eat(itm, body_part);

    PlaySnd(eInvItemUse);
}

#include "../Medkit.h"
#include "../Antirad.h"
#include "../ActorCondition.h"
#include "../nlc_body_health.h"
#include "UIListBoxItem.h"

// NLC: A.R.E.A. part rows: "use on head (70%) {0.12} [20%]", colored by the part state
void CUIInventoryWnd::AddBodyPartUseItems(bool& b_show)
{
    const CActorBodyHealth& B = Actor()->conditions().body();
    LPCSTR sect = CurrentIItem()->object().cNameSect().c_str();
    const u8 mask = B.ItemHealMask(sect);
    const bool only_damaged = READ_IF_EXISTS(pSettings, r_bool, sect, "show_when_damaged", false);

    for (u8 p = body_part::head; p < body_part::count; ++p)
    {
        if (!B.ItemTargetsPart(sect, p))
            continue;
        const float hp = B.PartHealth(p) * 100.f;
        const float bleed = B.Bleeding(p);
        const float frac = B.Fracture(p) * 100.f;
        if (only_damaged && fis_zero(B.PartDamage(p, mask)))
            continue;

        string32 key;
        xr_sprintf(key, "st_use_on_%s", body_part::name(p));
        string256 txt;
        xr_strcpy(txt, CStringTable().translate(key).c_str());
        string32 tmp;
        if (mask & body_part::heal_health)
        {
            xr_sprintf(tmp, hp < 99.5f ? " (%.0f%%)" : " (*)", hp);
            xr_strcat(txt, tmp);
        }
        if (mask & body_part::heal_bleeding)
        {
            xr_sprintf(tmp, bleed > 0.f ? " {%.2f}" : " {*}", bleed);
            xr_strcat(txt, tmp);
        }
        if (mask & body_part::heal_fracture)
        {
            xr_sprintf(tmp, frac > 0.f ? " [%.0f%%]" : " [*]", frac);
            xr_strcat(txt, tmp);
        }

        u32 color = 0xFF00FF00;
        if (hp < 25.f || frac > 50.f || bleed > 1.f)
            color = 0xFFFF0000;
        else if (hp < 50.f || frac > 25.f || bleed > 0.5f)
            color = 0xFFFFA500;
        else if (hp < 75.f || frac > 0.f || bleed > 0.01f)
            color = 0xFFFFFF00;

        CUIListBoxItem* row = UIPropertiesBox.AddItemEx(txt, (void*)(__int64)p, INVENTORY_EAT_BODY_PART);
        row->SetTextColor(color, color);
        b_show = true;
    }

    if (B.ItemTargetsPart(sect, body_part::all))
    {
        UIPropertiesBox.AddItemEx("st_use_on_all", (void*)(__int64)body_part::all, INVENTORY_EAT_BODY_PART);
        b_show = true;
    }
}
void CUIInventoryWnd::ActivatePropertiesBox()
{
    // Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
    bool bAlreadyDressed = false;

    UIPropertiesBox.RemoveAll();

    CMedkit* pMedkit = smart_cast<CMedkit*>(CurrentIItem());
    CAntirad* pAntirad = smart_cast<CAntirad*>(CurrentIItem());
    CEatableItem* pEatableItem = smart_cast<CEatableItem*>(CurrentIItem());
    CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(CurrentIItem());
    CWeapon* pWeapon = smart_cast<CWeapon*>(CurrentIItem());
    CBottleItem* pBottleItem = smart_cast<CBottleItem*>(CurrentIItem());

    bool b_show = false;

    if (!pOutfit && CurrentIItem()->GetSlot() != NO_ACTIVE_SLOT)
    {
        auto slots = CurrentIItem()->GetSlots();
        bool multi_slot = slots.size() > 1;
        for (u8 i = 0; i < (u8)slots.size(); ++i)
        {
            auto slot = slots[i];
            if (slot != NO_ACTIVE_SLOT && slot != GRENADE_SLOT)
            {
                if ((!m_pInv->m_slots[slot].m_pIItem || m_pInv->m_slots[slot].m_pIItem != CurrentIItem()) && AllowPutInSlot(CurrentItem(), slot) &&
                    !smart_cast<CCustomDetector*>(CurrentIItem()) && !smart_cast<CCustomDetectorSHOC*>(CurrentIItem()))
                {
                    if (multi_slot && Core.Features.test(xrCore::Feature::slots_extend_menu))
                    {
                        string128 full_action_text;
                        strconcat(sizeof(full_action_text), full_action_text, "st_move_to_slot_", std::to_string(slot).c_str());
                        UIPropertiesBox.AddItem(full_action_text, (void*)(__int64)slot, INVENTORY_TO_SLOT_ACTION);
                        b_show = true;
                    }
                    else
                    {
                        UIPropertiesBox.AddItem("st_move_to_slot", NULL, INVENTORY_TO_SLOT_ACTION);
                        b_show = true;
                        break;
                    }
                }
            }
        }
    }

    if (CurrentIItem()->Belt() && m_pInv->CanPutInBelt(CurrentIItem()))
    {
        UIPropertiesBox.AddItem("st_move_on_belt", NULL, INVENTORY_TO_BELT_ACTION);
        b_show = true;
    }

    if (CurrentIItem()->Ruck() && m_pInv->CanPutInRuck(CurrentIItem()) &&
        (CurrentIItem()->GetSlot() == NO_ACTIVE_SLOT || !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent))
    {
        if (!pOutfit)
            UIPropertiesBox.AddItem("st_move_to_bag", NULL, INVENTORY_TO_BAG_ACTION);
        else
            UIPropertiesBox.AddItem("st_undress_outfit", NULL, INVENTORY_TO_BAG_ACTION);
        if (pOutfit && fl_hook::blr_on)
        {
            UIPropertiesBox.AddItem("st_ballon_remove", NULL, INVENTORY_HANDLE_BATT_TORCH);
            ++fl_hook::blr_added;
        }
        bAlreadyDressed = true;
        b_show = true;
    }
    if (pOutfit && !bAlreadyDressed)
    {
        UIPropertiesBox.AddItem("st_dress_outfit", NULL, INVENTORY_TO_SLOT_ACTION);
        b_show = true;
    }

    //отсоединение аддонов от вещи
    if (pWeapon)
    {
        if (pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
        {
            UIPropertiesBox.AddItem("st_detach_gl", NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
            b_show = true;
        }
        if (pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
        {
            UIPropertiesBox.AddItem("st_detach_scope", NULL, INVENTORY_DETACH_SCOPE_ADDON);
            b_show = true;
        }
        if (pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
        {
            UIPropertiesBox.AddItem("st_detach_silencer", NULL, INVENTORY_DETACH_SILENCER_ADDON);
            b_show = true;
        }
        if (smart_cast<CWeaponMagazined*>(pWeapon))
        {
            auto WpnMagazWgl = smart_cast<CWeaponMagazinedWGrenade*>(pWeapon);
            bool b = pWeapon->GetAmmoElapsed() > 0 || (WpnMagazWgl && !WpnMagazWgl->m_magazine2.empty());

            if (!b)
            {
                CUICellItem* itm = CurrentItem();
                for (u32 i = 0; i < itm->ChildsCount(); ++i)
                {
                    auto pWeaponChild = static_cast<CWeaponMagazined*>(itm->Child(i)->m_pData);
                    auto WpnMagazWglChild = smart_cast<CWeaponMagazinedWGrenade*>(pWeaponChild);
                    if (pWeaponChild->GetAmmoElapsed() > 0 || (WpnMagazWglChild && !WpnMagazWglChild->m_magazine2.empty()))
                    {
                        b = true;
                        break;
                    }
                }
            }

            if (b)
            {
                UIPropertiesBox.AddItem("st_unload_magazine", NULL, INVENTORY_UNLOAD_MAGAZINE);
                b_show = true;
            }
        }
    }

    //присоединение аддонов к оружиям в слотах
    for (u32 i = 0; i < SLOTS_TOTAL; ++i)
    {
        PIItem tgt = m_pInv->m_slots[i].m_pIItem;
        if (tgt && tgt->CanAttach(CurrentIItem()))
        {
            // В локализации должно быть что-то типа 'Прикрепить %s к %s в таком-то слоте'
            const std::string trans_str = "st_attach_addon_to_wpn_in_slot_" + std::to_string(i);
            string512 str{};
            std::snprintf(str, sizeof str, CStringTable().translate(trans_str.c_str()).c_str(), CurrentIItem()->m_nameShort.c_str(), tgt->m_nameShort.c_str());
            UIPropertiesBox.AddItem(str, (void*)tgt, INVENTORY_ATTACH_ADDON);
            b_show = true;
        }
    }

    CUICellItem* cell = CurrentItem();
    u32 use_count = std::min<u32>(cell->ChildsCount() + 1, 9);
    if (pEatableItem && !pEatableItem->use_for_every_item)
        use_count = 1;

    // NLC: "use on <body part>" rows replace the plain use row
    if (pEatableItem && Actor()->conditions().body().IsBodyItem(CurrentIItem()->object().cNameSect().c_str()))
    {
        AddBodyPartUseItems(b_show);
        use_count = 0;
    }

    for (u32 i = 0; i < use_count; ++i)
    {
        PIItem cur = i == 0 ? CurrentIItem() : (PIItem)cell->Child(i - 1)->m_pData;
        LPCSTR _action = nullptr;

        if (pMedkit || pAntirad)
        {
            _action = "st_use";
            if (use_count > 1 && cur)
            {
                if (luabind::functor<LPCSTR> caption_use; ai().script_engine().functor("_G.caption_use", caption_use))
                    _action = caption_use(int(i + 1), cur->object().ID());
            }
        }
        else if (pEatableItem)
        {
            _action = pBottleItem ? "st_drink" : "st_eat";
            use_count = 0;
        }
        else
        {
            if (!cur)
                break;
            if (strstr(cur->object().cNameSect().c_str(), "batt_torch"))
            {
                UIPropertiesBox.AddItem("st_charge_gps", (void*)cur, INVENTORY_HANDLE_BATT_GPS);
                UIPropertiesBox.AddItem("st_charge_torch", (void*)cur, INVENTORY_HANDLE_BATT_TORCH);
                use_count = 0;
            }
            continue;
        }

        if (_action)
        {
            UIPropertiesBox.AddItem(_action, (void*)cur, INVENTORY_EAT_ACTION);
            b_show = true;
        }
    }

    bool disallow_drop = (pOutfit && bAlreadyDressed);
    disallow_drop |= !!CurrentIItem()->IsQuestItem();

    if (!disallow_drop)
    {
        UIPropertiesBox.AddItem("st_drop", NULL, INVENTORY_DROP_ACTION);
        b_show = true;

        if (CurrentItem()->ChildsCount())
            UIPropertiesBox.AddItem("st_drop_all", (void*)33, INVENTORY_DROP_ACTION);
    }

    if (b_show)
    {
        UIPropertiesBox.AutoUpdateSize();
        UIPropertiesBox.BringAllToTop();

        Fvector2 cursor_pos;
        Frect vis_rect;
        GetAbsoluteRect(vis_rect);
        cursor_pos = GetUICursor()->GetCursorPosition();
        cursor_pos.sub(vis_rect.lt);
        UIPropertiesBox.Show(vis_rect, cursor_pos);
        PlaySnd(eInvProperties);
    }
}

void CUIInventoryWnd::ProcessPropertiesBoxClicked()
{
    if (UIPropertiesBox.GetClickedItem())
    {
        switch (UIPropertiesBox.GetClickedItem()->GetTAG())
        {
        case INVENTORY_TO_SLOT_ACTION: {
            auto item = CurrentIItem();
            // Явно указали слот в меню
            void* d = UIPropertiesBox.GetClickedItem()->GetData();
            if (d)
            {
                auto slot = (u8)(__int64)d;
                item->SetSlot(slot);
                if (ToSlot(CurrentItem(), true))
                    return;
            }
            // Пытаемся найти свободный слот из списка разрешенных.
            // Если его нету, то принудительно займет первый слот,
            // указанный в списке.
            auto slots = item->GetSlots();
            for (u8 i = 0; i < (u8)slots.size(); ++i)
            {
                item->SetSlot(slots[i]);
                if (ToSlot(CurrentItem(), false))
                    return;
            }
            item->SetSlot(slots.size() ? slots[0] : NO_ACTIVE_SLOT);
            ToSlot(CurrentItem(), true);
            break;
        }

        case INVENTORY_TO_BELT_ACTION: ToBelt(CurrentItem(), false); break;
        case INVENTORY_TO_BAG_ACTION: ToBag(CurrentItem(), false); break;
        case INVENTORY_DROP_ACTION: {
            void* d = UIPropertiesBox.GetClickedItem()->GetData();
            bool b_all = (d == (void*)33);

            DropCurrentItem(b_all);
        }
        break;
        case INVENTORY_EAT_ACTION: {
            auto item = (PIItem)UIPropertiesBox.GetClickedItem()->GetData();
            EatItem(item ? item : CurrentIItem());
        }
        break;
        case INVENTORY_EAT_BODY_PART: EatItem(CurrentIItem(), u8((__int64)UIPropertiesBox.GetClickedItem()->GetData())); break; // NLC
        case INVENTORY_HANDLE_BATT_TORCH: {
            auto item = (PIItem)UIPropertiesBox.GetClickedItem()->GetData();
            if (!item)
                item = CurrentIItem();
            if (luabind::functor<void> func; ai().script_engine().functor("_G.batt_torch_charge", func))
                func(item->object().ID());
        }
        break;
        case INVENTORY_HANDLE_BATT_GPS: {
            auto item = (PIItem)UIPropertiesBox.GetClickedItem()->GetData();
            if (!item)
                item = CurrentIItem();
            if (luabind::functor<void> func; ai().script_engine().functor("_G.batt_gps_charge", func))
                func(item->object().ID());
        }
        break;
        case INVENTORY_ATTACH_ADDON: AttachAddon((PIItem)(UIPropertiesBox.GetClickedItem()->GetData())); break;
        case INVENTORY_DETACH_SCOPE_ADDON: DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetScopeName()); break;
        case INVENTORY_DETACH_SILENCER_ADDON: DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetSilencerName()); break;
        case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON: DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetGrenadeLauncherName()); break;
        case INVENTORY_RELOAD_MAGAZINE: (smart_cast<CWeapon*>(CurrentIItem()))->Action(kWPN_RELOAD, CMD_START); break;
        case INVENTORY_UNLOAD_MAGAZINE: {
            auto ProcessUnload = [](void* pWpn) {
                auto WpnMagaz = static_cast<CWeaponMagazined*>(pWpn);
                WpnMagaz->UnloadMagazine();
                if (auto WpnMagazWgl = smart_cast<CWeaponMagazinedWGrenade*>(WpnMagaz))
                {
                    if (WpnMagazWgl->IsGrenadeLauncherAttached())
                    {
                        WpnMagazWgl->PerformSwitchGL();
                        WpnMagazWgl->UnloadMagazine();
                        WpnMagazWgl->PerformSwitchGL();
                    }
                }
                // Сделано чтобы мгновенно переключиться в idle_empty
                if (WpnMagaz->GetState() == CHUDState::eIdle && WpnMagaz->HudItemData())
                    WpnMagaz->SwitchState(CHUDState::eIdle);
            };

            auto itm = CurrentItem();
            ProcessUnload(itm->m_pData);
            PlaySnd(eInvUnloadWpn);

            for (u32 i = 0; i < itm->ChildsCount(); ++i)
            {
                auto child_itm = itm->Child(i);
                ProcessUnload(child_itm->m_pData);
            }
        }
        break;
        }
    }
}

bool CUIInventoryWnd::TryUseItem(PIItem itm)
{
    CBottleItem* pBottleItem = smart_cast<CBottleItem*>(itm);
    CMedkit* pMedkit = smart_cast<CMedkit*>(itm);
    CAntirad* pAntirad = smart_cast<CAntirad*>(itm);
    CEatableItem* pEatableItem = smart_cast<CEatableItem*>(itm);

    if (pMedkit || pAntirad || pEatableItem || pBottleItem)
    {
        EatItem(itm);
        return true;
    }

    return false;
}

bool CUIInventoryWnd::DropItem(PIItem itm, CUIDragDropListEx* lst)
{
    if (lst == m_pUIOutfitList)
    {
        return TryUseItem(itm);
        /*
                CCustomOutfit*		pOutfit		= smart_cast<CCustomOutfit*>	(CurrentIItem());
                if(pOutfit)
                    ToSlot			(CurrentItem(), true);
                else
                    EatItem				(CurrentIItem());

                return				true;
        */
    }
    CUICellItem* _citem = lst->ItemsCount() ? lst->GetItemIdx(0) : NULL;
    PIItem _iitem = _citem ? (PIItem)_citem->m_pData : NULL;

    if (!_iitem)
        return false;
    if (!_iitem->CanAttach(itm))
        return false;
    AttachAddon(_iitem);

    return true;
}
