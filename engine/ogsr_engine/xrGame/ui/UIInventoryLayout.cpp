#include "stdafx.h"
#include "UIInventoryLayout.h"
#include "UIDragDropListEx.h"
#include "UICellItem.h"
#include "UIStatic.h"
#include "UIXmlInit.h"
#include "../inventory_item.h"
#include "../inventory.h"
#include "../Actor.h"
#include "../ai_space.h"
#include "../alife_simulator.h"
#include "../alife_object_registry.h"
#include "../xrServer_Objects_ALife.h"
#include "../xrServer_Objects_ALife_Monsters.h"
#include "../script_vars_storage.h"

constexpr LPCSTR LAYOUT_VAR = "inventory_layout";
constexpr LPCSTR LAYOUT_BOX_VAR_PREFIX = "inventory_layout_";
constexpr LPCSTR LAYOUT_SECT = "inventory_layout";

Flags32 psInvLayoutFlags{ILF_MANUAL_INVENTORY | ILF_MANUAL_TRADE | ILF_MANUAL_CARBODY | ILF_ROTATE | ILF_MANUAL_BOX};

void InitSortButtonIcon(CUIXml& xml, LPCSTR button_path, CUIWindow* button)
{
    string256 path;
    strconcat(sizeof(path), path, button_path, ":icon");
    if (!xml.NavigateToNode(path, 0))
        return;

    auto icon = xr_new<CUIStatic>();
    icon->SetAutoDelete(true);
    button->AttachChild(icon);
    CUIXmlInit().InitStatic(xml, path, 0, icon);
}

CInventoryLayout& inv_layout()
{
    static CInventoryLayout layout;
    return layout;
}

// the actor's bag proper; belt and slot items shown in trade never own a cell
static bool in_bag(const CInventoryItem* item)
{
    return item->m_eItemPlace == eItemPlaceRuck || (item->m_eItemPlace == eItemPlaceSlot && item->GetSlot() == GRENADE_SLOT);
}

bool CInventoryLayout::DropPushesByDefault()
{
    static const bool push = !xr_strcmp(READ_IF_EXISTS(pSettings, r_string, LAYOUT_SECT, "drop_conflict", "reject"), "push");
    return push;
}

int CInventoryLayout::SpareRows()
{
    static const int rows = READ_IF_EXISTS(pSettings, r_s32, LAYOUT_SECT, "spare_rows", 2);
    return rows;
}

shared_str CInventoryLayout::VarName(u16 owner)
{
    if (owner == OWNER_ACTOR)
        return LAYOUT_VAR;
    string64 name;
    xr_sprintf(name, "%s%u", LAYOUT_BOX_VAR_PREFIX, u32(owner));
    return name;
}

CInventoryLayout::SStore& CInventoryLayout::Store(u16 owner)
{
    const auto it = m_stores.find(owner);
    if (it != m_stores.end())
        return it->second;

    SStore& store = m_stores[owner];
    if (LPCSTR data = g_ScriptVars.get_string(VarName(owner).c_str()))
        Parse(store, data);
    return store;
}

u32 CInventoryLayout::ModeFor(EWindow wnd, CUIDragDropListEx* list, SStore& store, u16 owner)
{
    const u32 flag = wnd == wndInventory ? ILF_MANUAL_INVENTORY : wnd == wndTrade ? ILF_MANUAL_TRADE : wnd == wndCarBody ? ILF_MANUAL_CARBODY : ILF_MANUAL_BOX;
    if (!psInvLayoutFlags.test(flag) || !Actor())
        return CUIDragDropListEx::lmAuto;

    if (wnd != wndBox)
    {
        const bool section_view = wnd == wndInventory && m_section_view;
        if (section_view || !Actor()->inventory().GetItemFromInventory("shadow_inventory"))
            return CUIDragDropListEx::lmReadOnly;
    }

    const int columns = list->CellsCapacity().x;
    if (!store.columns)
        store.columns = columns;
    else if (store.columns != columns)
    {
        // another aspect ratio: the inventory owns the actor's layout, trade and corpses fall back to the old sort
        if (wnd == wndTrade || wnd == wndCarBody)
            return CUIDragDropListEx::lmAuto;
        // a wider list keeps every stored cell valid, trade or a corpse may have seeded the narrower count
        if (columns < store.columns)
        {
            Msg("~ [inv_layout] owner %u: columns changed %d -> %d, layout reset", u32(owner), store.columns, columns);
            ResetPositions(owner);
        }
        store.columns = columns;
    }
    return CUIDragDropListEx::lmManual;
}

const CInventoryLayout::SEntry* CInventoryLayout::Lookup(SStore& store, CInventoryItem* item)
{
    const u16 id = item->object().ID();
    const shared_str& section = item->object().cNameSect();

    auto it = store.ids.find(id);
    if (it != store.ids.end() && it->second.section != section)
    {
        // the id was reused by another object
        store.ids.erase(it);
        it = store.ids.end();
    }
    if (it == store.ids.end())
    {
        auto pending = store.pending.find(section);
        if (pending == store.pending.end())
            return nullptr;
        SEntry& e = store.ids[id];
        static_cast<SCell&>(e) = pending->second;
        e.section = section;
        store.pending.erase(pending);
        return &e;
    }
    return &it->second;
}

void CInventoryLayout::BeginFill(CUIDragDropListEx* list, EWindow wnd, TIItemContainer& items, u16 owner)
{
    SStore& store = Store(owner);

    const u32 mode = ModeFor(wnd, list, store, owner);
    list->SetLayoutMode(mode);
    list->SetLayoutOwner(owner);
    list->SetLayoutFilling(true);
    if (mode == CUIDragDropListEx::lmAuto)
        return;

    // stored items in reading order, the rest keeps the old sort behind them
    constexpr s64 RANK_UNKNOWN = s64(1) << 40, RANK_NOT_IN_BAG = s64(1) << 41;
    auto rank = [this, &store, owner](CInventoryItem* item) -> s64 {
        if (owner == OWNER_ACTOR && !in_bag(item))
            return RANK_NOT_IN_BAG;
        const SEntry* e = Lookup(store, item);
        return e ? (s64(e->y) << 16) + e->x : RANK_UNKNOWN;
    };
    xr_vector<std::pair<s64, PIItem>> ranked;
    ranked.reserve(items.size());
    for (PIItem item : items)
        ranked.emplace_back(rank(item), item);
    std::stable_sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (size_t i = 0; i < ranked.size(); ++i)
        items[i] = ranked[i].second;
}

void CInventoryLayout::EndFill(CUIDragDropListEx* list)
{
    list->SetLayoutFilling(false);
    if (list->LayoutMode() == CUIDragDropListEx::lmManual)
        list->EnsureSpareRows(SpareRows());
}

bool CInventoryLayout::Resolve(u16 owner, CInventoryItem* item, Ivector2& cell, bool& rotated)
{
    if (owner == OWNER_ACTOR && !in_bag(item))
        return false;

    SStore& store = Store(owner);
    if (const SEntry* e = Lookup(store, item))
    {
        cell.set(e->x, e->y);
        rotated = e->rotated;
        return true;
    }
    const auto home = store.homes.find(item->object().cNameSect());
    if (home == store.homes.end())
        return false;
    cell.set(home->second.x, home->second.y);
    rotated = home->second.rotated;
    return true;
}

void CInventoryLayout::Record(u16 owner, CInventoryItem* item, const Ivector2& cell, bool rotated)
{
    SStore& store = Store(owner);
    SEntry& e = store.ids[item->object().ID()];
    e.x = s16(cell.x);
    e.y = s16(cell.y);
    if (psInvLayoutFlags.test(ILF_ROTATE)) // switched off, items draw unturned but keep the saved bit
        e.rotated = rotated;
    e.section = item->object().cNameSect();
    store.homes[e.section] = e;
}

void CInventoryLayout::Commit(CUIDragDropListEx* list, CUICellItem* moved)
{
    if (list->LayoutMode() != CUIDragDropListEx::lmManual)
        return;

    const u16 owner = list->LayoutOwner();
    SStore& store = Store(owner);
    if (owner != OWNER_ACTOR && !moved && store.ids.empty())
        return;

    auto put = [&store, owner](CInventoryItem* item, const CUICellItem* cell, bool is_moved) {
        if (!item || (owner == OWNER_ACTOR && !in_bag(item)))
            return;
        SEntry& e = store.ids[item->object().ID()];
        e.x = s16(cell->CellPos().x);
        e.y = s16(cell->CellPos().y);
        if (psInvLayoutFlags.test(ILF_ROTATE)) // switched off, items draw unturned but keep the saved bit
            e.rotated = cell->IsRotated();
        e.section = item->object().cNameSect();
        if (is_moved || store.homes.find(e.section) == store.homes.end())
            store.homes[e.section] = e;
    };

    for (u32 i = 0, n = list->ItemsCount(); i < n; ++i)
    {
        CUICellItem* cell = list->GetItemIdx(i);
        put(static_cast<CInventoryItem*>(cell->m_pData), cell, cell == moved);
        for (u32 c = 0; c < cell->ChildsCount(); ++c)
            put(static_cast<CInventoryItem*>(cell->Child(c)->m_pData), cell, cell == moved);
    }
}

void CInventoryLayout::ResetPositions(u16 owner)
{
    SStore& store = Store(owner);
    store.ids.clear();
    store.homes.clear();
    store.pending.clear();
}

void CInventoryLayout::OnWeaponRespawn(u16 owner, CInventoryItem* item, LPCSTR new_section)
{
    SStore& store = Store(owner);
    const auto it = store.ids.find(item->object().ID());
    if (it == store.ids.end())
        return;
    store.pending[new_section] = it->second;
    store.homes[new_section] = it->second; // pending is not saved
    store.ids.erase(it);
}

static bool valid_stored_cell(int x, int y)
{
    constexpr int MAX_CELL = 2000; // only a damaged string gets near it, PlaceAt would grow the list that far
    return x >= 0 && x <= MAX_CELL && y >= 0 && y <= MAX_CELL;
}

void CInventoryLayout::Parse(SStore& store, LPCSTR data)
{
    // v1|<columns>|<id>,<x>,<y>,<rot>,<section>;...|<section>,<x>,<y>,<rot>;...
    int columns = 0;
    int used = 0;
    if (sscanf_s(data, "v1|%d|%n", &columns, &used) < 1 || !used)
    {
        Msg("! [inv_layout] unknown layout string [%.32s], layout reset", data);
        return;
    }
    store.columns = columns;

    LPCSTR p = data + used;
    string256 section;
    while (*p && *p != '|')
    {
        int id = 0, x = 0, y = 0, rot = 0, n = 0;
        if (sscanf_s(p, "%d,%d,%d,%d,%255[^;];%n", &id, &x, &y, &rot, section, (unsigned)sizeof(section), &n) < 5 || !n)
        {
            Msg("! [inv_layout] broken item entry [%.32s], layout reset", p);
            store.ids.clear();
            store.homes.clear();
            return;
        }
        p += n;
        if (!valid_stored_cell(x, y))
        {
            Msg("! [inv_layout] item %d: cell %d,%d out of range, ignored", id, x, y);
            continue;
        }
        SEntry& e = store.ids[u16(id)];
        e.x = s16(x);
        e.y = s16(y);
        e.rotated = !!rot;
        e.section = section;
    }
    if (*p == '|')
        ++p;
    while (*p)
    {
        int x = 0, y = 0, rot = 0, n = 0;
        if (sscanf_s(p, "%255[^,],%d,%d,%d;%n", section, (unsigned)sizeof(section), &x, &y, &rot, &n) < 4 || !n)
        {
            Msg("! [inv_layout] broken home entry [%.32s]", p);
            return;
        }
        p += n;
        if (!valid_stored_cell(x, y))
        {
            Msg("! [inv_layout] home of %s: cell %d,%d out of range, ignored", section, x, y);
            continue;
        }
        SCell& c = store.homes[section];
        c.x = s16(x);
        c.y = s16(y);
        c.rotated = !!rot;
    }
}

u32 CInventoryLayout::Prune(SStore& store, u16 owner)
{
    if (!ai().get_alife())
        return 0;

    const auto& objects = ai().alife().objects();
    auto keep = [&objects, owner](u16 id, const shared_str& section) {
        CSE_ALifeDynamicObject* obj = objects.object(id, true);
        if (!obj || section != obj->s_name)
            return false;
        if (owner != OWNER_ACTOR)
            return obj->ID_Parent == owner;
        CSE_ALifeDynamicObject* parent = objects.object(obj->ID_Parent, true);
        if (!parent)
            return false;
        // the actor, the backpack storage (worn or dropped) and the belt bag
        return smart_cast<CSE_ALifeCreatureActor*>(parent) || !xr_strcmp(parent->s_name, "shadow_inventory") || !xr_strcmp(parent->s_name, "shadow_inventory2") ||
            !xr_strcmp(parent->s_name, "rucksack");
    };

    u32 pruned = 0;
    for (auto it = store.ids.begin(); it != store.ids.end();)
    {
        if (keep(it->first, it->second.section))
            ++it;
        else
        {
            it = store.ids.erase(it);
            ++pruned;
        }
    }
    return pruned;
}

void CInventoryLayout::Flush()
{
    u32 items = 0, pruned = 0, bytes = 0;
    string512 tmp;
    for (auto& [owner, store] : m_stores)
    {
        pruned += Prune(store, owner);

        const shared_str var = VarName(owner);
        if (owner != OWNER_ACTOR && store.ids.empty())
        {
            g_ScriptVars.remove_var(var.c_str());
            continue;
        }

        xr_string data;
        xr_sprintf(tmp, "v1|%d|", store.columns);
        data += tmp;
        for (const auto& [id, e] : store.ids)
        {
            xr_sprintf(tmp, "%u,%d,%d,%d,%s;", u32(id), int(e.x), int(e.y), e.rotated ? 1 : 0, e.section.c_str());
            data += tmp;
        }
        data += "|";
        for (const auto& [section, c] : store.homes)
        {
            xr_sprintf(tmp, "%s,%d,%d,%d;", section.c_str(), int(c.x), int(c.y), c.rotated ? 1 : 0);
            data += tmp;
        }
        g_ScriptVars.set_string(var.c_str(), data.c_str());
        items += u32(store.ids.size());
        bytes += u32(data.size());
    }

    // layouts of boxes that no longer exist
    if (ai().get_alife())
    {
        const u32 prefix = xr_strlen(LAYOUT_BOX_VAR_PREFIX);
        xr_vector<shared_str> dead;
        for (const auto& var : g_ScriptVars.map())
            if (!strncmp(var.first.c_str(), LAYOUT_BOX_VAR_PREFIX, prefix) && !ai().alife().objects().object(u16(atoi(var.first.c_str() + prefix)), true))
                dead.push_back(var.first);
        for (const shared_str& name : dead)
            g_ScriptVars.remove_var(name.c_str());
    }

    if (!m_stores.empty())
        Msg("* [inv_layout] saved: %u owners, %u items, %u pruned, %u bytes", u32(m_stores.size()), items, pruned, bytes);
}

void CInventoryLayout::Reset()
{
    m_stores.clear();
    m_section_view = false;
}

void CInventoryLayout::Dump()
{
    Store(OWNER_ACTOR);
    Msg("[inv_layout] owners=%u section_view=%d flags=%u", u32(m_stores.size()), m_section_view ? 1 : 0, u32(psInvLayoutFlags.flags));
    for (const auto& [owner, store] : m_stores)
    {
        Msg("[inv_layout] owner %u: columns=%d items=%u homes=%u pending=%u", u32(owner), store.columns, u32(store.ids.size()), u32(store.homes.size()), u32(store.pending.size()));
        for (const auto& [id, e] : store.ids)
            Msg("[inv_layout]   #%u %s at %d,%d rot=%d", u32(id), e.section.c_str(), int(e.x), int(e.y), e.rotated ? 1 : 0);
        for (const auto& [section, c] : store.homes)
            Msg("[inv_layout]   home %s at %d,%d rot=%d", section.c_str(), int(c.x), int(c.y), c.rotated ? 1 : 0);
    }
}
