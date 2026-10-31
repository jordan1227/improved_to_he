#pragma once

#include "UIStatic.h"
#include "UIDialogWnd.h"

class CUIDragItem;

// NLC: the heading of a turned bag item: clockwise, its top-left corner at the top right of the footprint
constexpr float BAG_ROTATED_HEADING = -90.0f * (PI / 180.0f);
class CUIDragDropListEx;
class CUICellItem;
class CUIProgressBar;

class CUICellItem : public CUIStatic
{
private:
    typedef CUIStatic inherited;

protected:
    xr_vector<CUICellItem*> m_childs;

    CUIDragDropListEx* m_pParentList;
    Ivector2 m_grid_size;
    Fvector2 m_cell_size;
    int m_accelerator;

    CUIProgressBar* m_pConditionState;
    bool m_condition_auto_width;
    CUIStatic* m_text;
    void init();
    bool m_rotated{}; // NLC
    Ivector2 m_cell_pos{}; // NLC

public:
    CUICellItem();
    virtual ~CUICellItem();

    virtual bool OnKeyboard(int dik, EUIMessages keyboard_action);
    virtual bool OnMouse(float x, float y, EUIMessages mouse_action);
    virtual void Draw();
    virtual void Update();
    virtual void UpdateItemText();

    virtual void OnAfterChild(CUIDragDropListEx* parent_list){};

    // NLC: per-item rotation and cell in a manual-layout bag
    bool IsRotated() const { return m_rotated; }
    void SetRotated(bool b);
    Ivector2 GridFootprint() const
    {
        Ivector2 fp = m_grid_size;
        if (m_rotated)
            std::swap(fp.x, fp.y);
        return fp;
    }
    const Ivector2& CellPos() const { return m_cell_pos; }
    void SetCellPos(const Ivector2& pos) { m_cell_pos = pos; }
    virtual void OnDragRotated(CUIDragItem* drag_item) {}
    virtual u32 DragAmountTotal() { return 0; } // NLC: above 0, a drag can take part of this cell (ammo rounds)

    u32 ChildsCount();
    void PushChild(CUICellItem*);
    CUICellItem* PopChild();
    CUICellItem* Child(u32 idx) { return m_childs.at(idx); };
    bool HasChild(CUICellItem* item);
    virtual bool EqualTo(CUICellItem* itm);
    IC const Ivector2& GetGridSize() { return m_grid_size; }; // size in grid
    IC void SetAccelerator(int dik) { m_accelerator = dik; };
    IC int GetAccelerator() const { return m_accelerator; };

    virtual CUIDragItem* CreateDragItem();

    CUIDragDropListEx* OwnerList() { return m_pParentList; }
    static CUICellItem* m_mouse_selected_item;
    void SetOwnerList(CUIDragDropListEx* p);
    void* m_pData;
    int m_index;
    bool m_b_already_drawn;
    void ColorizeItems(std::initializer_list<CUIDragDropListEx*>);
    void UpdateConditionProgressBar();
    bool m_selected;
    bool m_select_armament;
    bool m_select_equipped;
    bool m_select_untradable;
};

class CUIDragItem : public CUIWindow, public pureRender, public pureFrame
{
private:
    typedef CUIWindow inherited;
    CUIStatic m_static;
    CUICellItem* m_pParent;
    Fvector2 m_pos_offset;
    CUIDragDropListEx* m_back_list;

public:
    CUIDragItem(CUICellItem* parent);
    virtual void Init(const ui_shader& sh, const Frect& rect, const Frect& text_rect);
    virtual ~CUIDragItem();
    CUIStatic* wnd() { return &m_static; }
    virtual bool OnMouse(float x, float y, EUIMessages mouse_action);
    virtual void Draw();
    virtual void OnRender();
    virtual void OnFrame();
    CUICellItem* ParentItem() { return m_pParent; }
    void SetBackList(CUIDragDropListEx* l);
    CUIDragDropListEx* BackList() { return m_back_list; }
    Fvector2 GetPosition();
    virtual bool CapturesFocusToo() { return false; };

    // NLC: rotation while dragging out of a manual-layout bag
    void InitLayoutGeometry(const Ivector2& grid, const Ivector2& cell);
    bool HasLayoutGeometry() const { return m_layout_geometry; }
    const Ivector2& LayoutGrid() const { return m_layout_grid; }
    const Ivector2& LayoutCell() const { return m_layout_cell; }
    bool IsRotated() const { return m_rotated; }
    void SetRotated(bool b);
    Ivector2 GridFootprint() const;
    u32 ChosenAmount() const { return m_amount; } // NLC: 0 = the whole cell

private:
    bool CanRotate();
    void ChangeAmount(int delta);
    u32 m_amount{};
    CUIStatic* m_amount_text{};
    bool m_layout_geometry{};
    bool m_rotated{};
    Ivector2 m_layout_grid{};
    Ivector2 m_layout_cell{};
};
