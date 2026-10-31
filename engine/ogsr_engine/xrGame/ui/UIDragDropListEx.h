#pragma once

#include "UIWindow.h"
#include "UIWndCallback.h"

class CUICellContainer;
class CUIScrollBar;
class CUIStatic;
class CUICellItem;
class CUIDragItem;

enum EListType
{
    iwSlot,
    iwBag,
    iwBelt
};

struct CUICell
{
    CUICell() = default;

    CUICellItem* m_item{};
    bool m_bMainItem{};
    bool cell_disabled{};
    bool is_highlighted{};

    void SetItem(CUICellItem* itm, bool bMain)
    {
        m_item = itm;
        VERIFY(m_item);
        m_bMainItem = bMain;
    }
    bool Empty() { return m_item == NULL; }
    bool MainItem() { return m_bMainItem; }
    void Clear();
    bool operator==(const CUICell& C) const { return (m_item == C.m_item); }
};

typedef xr_vector<CUICell> UI_CELLS_VEC;
typedef UI_CELLS_VEC::iterator UI_CELLS_VEC_IT;

class CUIDragDropListEx : public CUIWindow, public CUIWndCallback
{
private:
    typedef CUIWindow inherited;

    enum
    {
        flGroupSimilar = (1 << 0),
        flAutoGrow = (1 << 1),
        flCustomPlacement = (1 << 2),
        flVerticalPlacement = (1 << 3),
        flAlwaysShowScroll = (1 << 4),
        flVirtualCells = (1 << 5),
        // !Место свободно! = (1 << 6),
        flHighlightCellSp = (1 << 7),
        flHighlightAllCells = (1 << 8),
        flShowGrid = (1 << 9),
    };
    Flags16 m_flags;
    CUICellItem* m_selected_item;
    u32 m_layout_mode{}; // NLC
    u16 m_layout_owner{u16(-1)}; // NLC: CInventoryLayout::OWNER_ACTOR or the box's id
    bool m_layout_filling{}; // NLC: a rebuild is placing items, a live drag must not attract them
    Ivector2 m_orig_cell_capacity;
    Ivector2 m_virtual_cells_alignment;
    bool m_bConditionProgBarVisible;

public:
    CUICellContainer* m_container;

protected:
    CUIScrollBar* m_vScrollBar;

    void OnScrollV(CUIWindow* w, void* pData);
    void OnItemStartDragging(CUIWindow* w, void* pData);
    void OnItemDrop(CUIWindow* w, void* pData);
    void OnItemSelected(CUIWindow* w, void* pData);
    void OnItemRButtonClick(CUIWindow* w, void* pData);
    void OnItemDBClick(CUIWindow* w, void* pData);

public:
    static CUIDragItem* m_drag_item;
    CUIDragDropListEx();
    virtual ~CUIDragDropListEx();
    void Init(float x, float y, float w, float h);

    using DRAG_DROP_EVENT = fastdelegate::FastDelegate<bool(CUICellItem*)>;

    DRAG_DROP_EVENT m_f_item_drop;
    DRAG_DROP_EVENT m_f_item_start_drag;
    DRAG_DROP_EVENT m_f_item_db_click;
    DRAG_DROP_EVENT m_f_item_selected;
    DRAG_DROP_EVENT m_f_item_rbutton_click;
    DRAG_DROP_EVENT m_f_item_amount; // NLC: true = a drag of this cell may take part of it

    u32 back_color;

    const Ivector2& CellsCapacity();
    void SetCellsCapacity(const Ivector2 c);
    void SetCellsAvailable(const u32 count);
    void SetStartCellsCapacity(const Ivector2 c)
    {
        m_orig_cell_capacity = c;
        SetCellsCapacity(c);
    };
    void ResetCellsCapacity()
    {
        VERIFY(ItemsCount() == 0);
        SetCellsCapacity(m_orig_cell_capacity);
    };
    const Ivector2& CellSize();
    void SetCellSize(const Ivector2 new_sz);
    const Ivector2& CellsSpacing();
    void SetCellsSpacing(const Ivector2& new_sz);
    void SetCellsVertAlignment(xr_string alignment);
    void SetCellsHorizAlignment(xr_string alignment);

    const Ivector2 GetVirtualCellsAlignment() { return m_virtual_cells_alignment; };

    int ScrollPos();
    void SetScrollPos(int _pos);
    void ReinitScroll();
    void GetClientArea(Frect& r);
    Fvector2 GetDragItemPosition();

    void SetAutoGrow(bool b);
    bool IsAutoGrow();
    void SetGrouping(bool b);
    bool IsGrouping();
    void SetCustomPlacement(bool b);
    bool GetCustomPlacement();
    void SetVerticalPlacement(bool b);
    bool GetVerticalPlacement();
    void SetAlwaysShowScroll(bool b);
    bool GetVirtualCells();
    void SetVirtualCells(bool b);
    bool GetHighlightCellSp();
    void SetHighlightCellSp(bool b);
    bool GetHighlightAllCells();
    void SetHighlightAllCells(bool b);
    void SetShowGrid(bool b) { m_flags.set(flShowGrid, b); }
    bool GetShowGrid() const { return !!m_flags.test(flShowGrid); }

    bool GetConditionProgBarVisibility() { return m_bConditionProgBarVisible; };
    void SetConditionProgBarVisibility(bool b) { m_bConditionProgBarVisible = b; };

public:
    // items management
    virtual void SetItem(CUICellItem* itm); // auto
    virtual void SetItem(CUICellItem* itm, Fvector2 abs_pos); // start at cursor pos
    virtual void SetItem(CUICellItem* itm, Ivector2 cell_pos); // start at cell
    bool CanSetItem(CUICellItem* itm);

    u32 ItemsCount();
    CUICellItem* GetItemIdx(u32 idx);
    virtual CUICellItem* RemoveItem(CUICellItem* itm, bool force_root);
    void CreateDragItem(CUICellItem* itm);

    void DestroyDragItem();
    void ClearAll(bool bDestroy);
    void Compact();
    bool IsOwner(CUICellItem* itm);
    void clear_select_armament();
    Ivector2 PickCell(const Fvector2& abs_pos);
    CUICell& GetCellAt(const Ivector2& pos);

    // NLC: manual bag arrangement (UIInventoryLayout)
    enum ELayoutMode
    {
        lmAuto, // old behaviour
        lmManual, // cells come from the layout, drops are saved
        lmReadOnly, // compact list in backpack order, dragging inside snaps back
    };
    u32 LayoutMode() const { return m_layout_mode; }
    void SetLayoutMode(u32 mode) { m_layout_mode = mode; }
    void SetLayoutFilling(bool b) { m_layout_filling = b; }
    u16 LayoutOwner() const { return m_layout_owner; }
    void SetLayoutOwner(u16 owner) { m_layout_owner = owner; }
    void EnsureSpareRows(int rows);
    void ScrollByWheel(EUIMessages mouse_action);

private:
    void PlaceByLayout(CUICellItem* itm);
    void PlaceAt(CUICellItem* itm, const Ivector2& cell);
    void LayoutDrop(CUICellItem* itm);

public:
public:
    // UIWindow overriding
    virtual void Draw();
    virtual void Update();
    virtual bool OnMouse(float x, float y, EUIMessages mouse_action);
    virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);

    void enable_highlight(const bool);
};

class CUICellContainer : public CUIWindow
{
    friend class CUIDragDropListEx;

private:
    typedef CUIWindow inherited;
    ui_shader hShader; // ownerDraw
    ui_shader hShader0;
    ui_shader hShaderPreview; // NLC: footprint of the dragged item
    UI_CELLS_VEC m_cells_to_draw;

protected:
    CUIDragDropListEx* m_pParentDragDropList;

    Ivector2 m_cellsCapacity; // count		(col,	row)
    Ivector2 m_cellSize; // pixels	(width, height)
    Ivector2 m_cellSpacing; // pixels	(width, height)

    UI_CELLS_VEC m_cells;

    void ReinitSize();
    u32 GetCellsInRange(const Irect& rect, UI_CELLS_VEC& res);

public:
    CUICellContainer(CUIDragDropListEx* parent);
    virtual ~CUICellContainer();

protected:
    virtual void Draw();

    IC const Ivector2& CellsCapacity() { return m_cellsCapacity; };
    void SetCellsCapacity(const Ivector2& c);
    void SetCellsAvailable(const u32 count);

    void enable_highlight(const bool);

    IC const Ivector2& CellSize() { return m_cellSize; };
    void SetCellSize(const Ivector2& new_sz);
    IC const Ivector2& CellsSpacing() { return m_cellSpacing; };
    void SetCellsSpacing(const Ivector2& new_sz);
    Ivector2 TopVisibleCell();

public:
    CUICell& GetCellAt(const Ivector2& pos);
    Ivector2 PickCell(const Fvector2& abs_pos);

protected:
    Ivector2 GetItemPos(CUICellItem* itm);
    Ivector2 FindFreeCell(const Ivector2& size);
    bool HasFreeSpace(const Ivector2& size);
    bool IsRoomFree(const Ivector2& pos, const Ivector2& size);

    bool AddSimilar(CUICellItem* itm);
    CUICellItem* FindSimilar(CUICellItem* itm);

    void PlaceItemAtPos(CUICellItem* itm, Ivector2& cell_pos);
    CUICellItem* RemoveItem(CUICellItem* itm, bool force_root);

    // NLC: manual layout
    enum EDropState
    {
        dsPlace, // free, or only the item's own old cells
        dsSwap, // exactly one other item, it takes the old cell
        dsPush, // several items, they move to the nearest free cells
        dsReject, // several items, the drop is refused
        dsRelocate, // from another list onto occupied cells: lands on the nearest free cell
    };
    struct SDropInfo
    {
        EDropState state{dsPlace};
        Ivector2 cell{};
        Ivector2 footprint{};
        xr_vector<CUICellItem*> hits;
    };
    void ClassifyDrop(SDropInfo& info);
    Ivector2 FindNearestFree(const Ivector2& wanted, const Ivector2& footprint);
    void DrawDropPreview(const Irect& visible, const Fvector2& drawLT, const Fvector2& cell_len, const Fvector2& spacing_len);

public:
    bool ValidCell(const Ivector2& pos) const;

protected:
    void Grow();
    void Shrink();
    void ClearAll(bool bDestroy);
    void clear_select_armament();

private:
    enum CellTextureType : u32
    {
        CellTextureTypeNormal, //Обычная клетка инвентаря
        CellTextureTypeCursorHover, //При наведении курсора на предмет
        CellTextureTypeEquipped, //Предмет в слотах при торговле
        CellTextureTypeArmanent, //Патроны для стволов, аддоны
        CellTextureTypeUntradable, //Непродаваемый предмет при торговле
        CellTextureTypeAvailableSlots, //Доступные слоты для предмета
        CellTextureTypeBlockedSlots, //Заблокированные клетки для артов
    };
    CellTextureType get_select_mode(const int x, const int y);
};
