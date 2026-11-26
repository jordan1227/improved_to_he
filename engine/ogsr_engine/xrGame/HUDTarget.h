#pragma once

#include "HUDCrosshair.h"
#include "..\xrcdb\xr_collide_defs.h"

constexpr u32 C_ON_ENEMY = D3DCOLOR_XRGB(0xff, 0, 0), C_ON_NEUTRAL = D3DCOLOR_XRGB(0xff, 0xff, 0x80), C_ON_FRIEND = D3DCOLOR_XRGB(0, 0xff, 0),
              C_DEFAULT = D3DCOLOR_XRGB(0xff, 0xff, 0xff);

class CHUDManager;

class CHUDTarget
{
private:
    friend class CHUDManager;

private:
    typedef collide::rq_result rq_result;
    typedef collide::rq_results rq_results;

private:
    ui_shader hShader;
    float fuzzyShowInfo;
    rq_result RQ;
    rq_results RQR;
    // NLC: use/pickup ray from the camera centre, short (take distance). Equals RQ when the
    // crosshair ray already starts at the camera (docs/LOOT_INTERACTION_DESIGN.md 10)
    rq_result RQ_use;
    float m_real_dist;

private:
    bool m_bShowCrosshair;
    CHUDCrosshair HUDCrosshair;

private:
    void net_Relcase(CObject* O);

public:
    CHUDTarget();
    void CursorOnFrame();
    void Render();
    void Load();
    float GetDist();
    float GetRealDist();
    CObject* GetObj();
    rq_result& GetUseRayQuery() { return RQ_use; }
};
