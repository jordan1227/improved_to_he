#pragma once

// NLC: manual arrangement of the actor's bag and of inventory boxes (docs/INVENTORY_LAYOUT_DESIGN.md).
// Each owner has its own layout: item id -> cell, section -> home cell. Layouts are kept as strings
// in g_ScriptVars, so the save format does not change.

#include "../inventory_space.h"

class CUIDragDropListEx;
class CUICellItem;
class CInventoryItem;
class CUIXml;
class CUIWindow;

enum
{
    ILF_MANUAL_INVENTORY = (1 << 0),
    ILF_MANUAL_TRADE = (1 << 1),
    ILF_MANUAL_CARBODY = (1 << 2),
    ILF_ROTATE = (1 << 3),
    ILF_MANUAL_BOX = (1 << 4),
};
extern Flags32 psInvLayoutFlags;

class CInventoryLayout
{
public:
    enum EWindow
    {
        wndInventory,
        wndTrade,
        wndCarBody, // the actor's side of the corpse and box window
        wndBox, // the box's side of it
    };
    static constexpr u16 OWNER_ACTOR = u16(-1); // otherwise the box's object id

    // items keep the old sort on entry; manual and read-only views reorder them
    void BeginFill(CUIDragDropListEx* list, EWindow wnd, TIItemContainer& items, u16 owner = OWNER_ACTOR);
    void EndFill(CUIDragDropListEx* list);

    // the wanted cell of an item entering a manual list; false = no wish, use the first free cell
    bool Resolve(u16 owner, CInventoryItem* item, Ivector2& cell, bool& rotated);
    // an item placed by the player from another list
    void Record(u16 owner, CInventoryItem* item, const Ivector2& cell, bool rotated);
    // saves every cell the list shows; the moved item also becomes the home of its section.
    // A box nobody arranged stays out of the save.
    void Commit(CUIDragDropListEx* list, CUICellItem* moved = nullptr);
    // the Sort button: forget everything, the next fill uses the old sort
    void ResetPositions(u16 owner = OWNER_ACTOR);

    // a weapon respawning under another section keeps its cell (scope attach, detach, swap)
    void OnWeaponRespawn(u16 owner, CInventoryItem* item, LPCSTR new_section);

    void SetSectionView(bool b) { m_section_view = b; }

    bool DropPushesByDefault();
    int SpareRows();

    void Flush(); // writes g_ScriptVars, before the save
    void Reset(); // g_ScriptVars was replaced or cleared
    void Dump();

private:
    struct SCell
    {
        s16 x{}, y{};
        bool rotated{};
    };
    struct SEntry : SCell
    {
        shared_str section;
    };
    struct SStore
    {
        xr_map<u16, SEntry> ids;
        xr_map<shared_str, SCell> homes;
        xr_map<shared_str, SCell> pending;
        int columns{};
    };

    SStore& Store(u16 owner); // loads it on first use
    static shared_str VarName(u16 owner);
    u32 ModeFor(EWindow wnd, CUIDragDropListEx* list, SStore& store, u16 owner);
    void Parse(SStore& store, LPCSTR data);
    const SEntry* Lookup(SStore& store, CInventoryItem* item);
    u32 Prune(SStore& store, u16 owner);

    xr_map<u16, SStore> m_stores;
    bool m_section_view{};
};

CInventoryLayout& inv_layout();

// the optional <icon> child of a Sort button node: a child window, so it hides with the button.
// Its coordinates are relative to the button.
void InitSortButtonIcon(CUIXml& xml, LPCSTR button_path, CUIWindow* button);
