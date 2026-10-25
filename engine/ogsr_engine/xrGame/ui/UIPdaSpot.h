#pragma once

#include "UIDialogWnd.h"
#include "UIWndCallback.h"

class CUIXml;
class CUI3tButton;
class CUIStatic;
class CUIEditBoxEx;

class CUIPdaSpot : public CUIDialogWnd, public CUIWndCallback
{
    using base_class = CUIDialogWnd;

    CUIStatic* m_background;
    CUIEditBoxEx* m_editBox;
    CUI3tButton* m_btn_ok;
    CUI3tButton* m_btn_cancel;

    // NLC: marker icon choice, optional timestamp and a marker cap
    xr_vector<shared_str> m_types;
    xr_vector<CUIStatic*> m_icons;
    CUI3tButton* m_btn_time{};
    CUIStatic* m_counter{};
    u32 m_maxCount{};
    int m_sel{};
    static bool s_addTime;

    bool m_mainWnd;
    LPCSTR m_levelName;
    Fvector m_position;
    u16 m_spotID;
    shared_str m_spotType;
    shared_str m_curType; // NLC: type of the marker being edited

    void SelectIcon(int idx);
    void UpdateTimeButton();
    u32 UserSpotCount() const;
    xr_string ComposeHint() const;

public:
    CUIPdaSpot();
    ~CUIPdaSpot() = default;

    void Init(u16 spot_id, LPCSTR level_name, Fvector pos, bool main_wnd, LPCSTR spot_type = nullptr);
    void InitControls();

    void OnAdd(CUIWindow* w, void* d);
    void OnApply(CUIWindow* w, void* d);
    void OnExit(CUIWindow* w, void* d);
    void OnTimeToggle(CUIWindow* w, void* d);
    void Exit();
    virtual bool OnMouse(float x, float y, EUIMessages mouse_action);
    virtual bool OnKeyboard(int dik, EUIMessages keyboard_action);
    virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);
};
