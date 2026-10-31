#pragma once
#include "UILabel.h"

class CUIListBoxItem : public CUILabel, public CUISelectable
{
public:
    using CUILabel::SetTextColor;

    CUIListBoxItem();
    virtual ~CUIListBoxItem();

    virtual void SetSelected(bool b);
    //	virtual void	Update();
    virtual void Draw();
    virtual bool OnMouseDown(int mouse_btn);
    virtual bool OnDbClick();
    virtual void OnFocusReceive();
    virtual CGameFont* GetFont();
    void SetTextColor(u32 color, u32 color_s);
    void InitDefault();
    void SetTAG(u32 value);
    u32 GetTAG();

    void SetData(void* data);
    void* GetData();

    // NLC: item menu rows (CUIPropertiesBox::ApplyMenuConfig)
    void SetMenuKey(LPCSTR key) { m_menu_key = key; }
    const shared_str& GetMenuKey() const { return m_menu_key; }
    void SetMenuRank(u32 rank) { m_menu_rank = rank; }
    u32 GetMenuRank() const { return m_menu_rank; }
    void SetSeparator();
    bool IsSeparator() const { return !!m_separator; }
    void SetIcon(LPCSTR texture, const Frect& rect); // an empty rect uses the whole texture
    void SetItemIcon(const shared_str& item_section);
    bool HasIcon() const { return !!m_icon; }

    CUIStatic* AddField(LPCSTR txt, float len, LPCSTR key = "");
    LPCSTR GetField(LPCSTR key);

protected:
    float FieldsLength();
    xr_vector<CUIStatic*> fields;
    u32 txt_color;
    u32 txt_color_s;
    u32 tag;
    void* pData{};
    shared_str m_menu_key; // NLC
    u32 m_menu_rank{}; // NLC
    CUIStatic* m_separator{}; // NLC
    CUIStatic* m_icon{}; // NLC
    void PlaceIcon(float aspect); // NLC
    //.static	u32	uid_counter;
};
