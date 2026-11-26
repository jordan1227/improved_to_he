#include "stdafx.h"
#include "actor.h"
#include "weapon.h"
#include "mercuryball.h"
#include "inventory.h"
#include "hudmanager.h"
#include "UsableScriptObject.h"
#include "customzone.h"
#include "../xr_3da/GameMtlLib.h"
#include "ui/UIMainIngameWnd.h"
#include "Grenade.h"
#include "clsid_game.h"

#include "game_cl_base.h"
#include "Level.h"
#include "UIStaticItem.h"
#include "ui_base.h"
#include "../xr_3da/xr_input.h"

#define PICKUP_INFO_COLOR 0xFFDDDDDD
// AAAAAA

void CActor::feel_touch_new(CObject* O) {}

void CActor::feel_touch_delete(CObject* O)
{
    CPhysicsShellHolder* sh = smart_cast<CPhysicsShellHolder*>(O);
    if (sh && sh->character_physics_support())
        m_feel_touch_characters--;
}

BOOL CActor::feel_touch_contact(CObject* O)
{
    CInventoryItem* item = smart_cast<CInventoryItem*>(O);
    CInventoryOwner* inventory_owner = smart_cast<CInventoryOwner*>(O);

    if (item && item->Useful() && !item->object().H_Parent())
        return TRUE;

    if (inventory_owner && inventory_owner != smart_cast<CInventoryOwner*>(this))
    {
        CPhysicsShellHolder* sh = smart_cast<CPhysicsShellHolder*>(O);
        if (sh && sh->character_physics_support())
            m_feel_touch_characters++;
        return TRUE;
    }

    return (FALSE);
}

BOOL CActor::feel_touch_on_contact(CObject* O)
{
    CCustomZone* custom_zone = smart_cast<CCustomZone*>(O);
    if (!custom_zone)
        return (TRUE);

    Fsphere sphere;
    sphere.P = Position();
    sphere.R = EPS_L;
    if (custom_zone->inside(sphere))
        return (TRUE);

    return (FALSE);
}

void CActor::PickupModeOn() { m_bPickupMode = true; }

void CActor::PickupModeOff() { m_bPickupMode = false; }

ICF static BOOL info_trace_callback(collide::rq_result& result, LPVOID params)
{
    BOOL& bOverlaped = *(BOOL*)params;
    if (result.O)
    {
        if (Level().CurrentEntity() == result.O)
        {
            // ignore self-actor
            return TRUE;
        }
        else
        {
            if (!Core.Features.test(xrCore::Feature::pickup_check_overlaped))
                return TRUE;
        }
    }
    else
    {
        //получить треугольник и узнать его материал
        CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
        const auto* mtl = GMLib.GetMaterialByIdx(T->material);
        if (mtl->Flags.is(SGameMtl::flPassable))
            return TRUE;
        // возможно это сетка-рабица и через нее можно брать предметы
        else if (fsimilar(mtl->fVisTransparencyFactor, 1.0f, EPS) && fsimilar(mtl->fShootFactor, 1.0f, EPS) && mtl->Flags.is(SGameMtl::flSuppressWallmarks))
            return TRUE;
    }
    bOverlaped = TRUE;
    return FALSE;
}

BOOL CActor::CanPickItem(const CFrustum& frustum, const Fvector& from, CObject* item)
{
    if (!item->getVisible())
        return FALSE;

    if (smart_cast<CObject*>(ObjectWeLookingAt()) == item)
        return TRUE;

    BOOL bOverlaped = FALSE;
    Fvector dir, to;
    item->Center(to);
    float range = dir.sub(to, from).magnitude();
    if (range > 0.25f)
    {
        if (frustum.testSphere_dirty(to, item->Radius()))
        {
            dir.div(range);
            collide::ray_defs RD(from, dir, range, CDB::OPT_CULL, collide::rqtBoth);
            VERIFY(!fis_zero(RD.dir.square_magnitude()));
            RQR.r_clear();
            Level().ObjectSpace.RayQuery(RQR, RD, info_trace_callback, &bOverlaped, NULL, item);
        }
    }
    return !bOverlaped;
}

void CActor::OnUseKeyPress()
{
    if (!m_bUseKeyHeld)
    {
        m_bUseKeyHeld = true;
        m_dwUseKeyPressTime = Device.dwTimeGlobal;
    }
}

void CActor::OnUseKeyRelease() { m_bUseKeyHeld = false; }

// NLC: item labels / marks are shown while Use is physically held (docs/LOOT_INTERACTION_DESIGN.md 5.3).
// The stock game showed them only while m_bPickupMode was set, which ended on the first pickup.
bool CActor::PickupInfoActive()
{
    if (m_bUseKeyHeld)
    {
        // a release swallowed by a menu must not leave the marks on. E-2: either key bound to Use counts.
        const _binding& b = g_key_bindings.at(kUSE);
        bool any_bound = false, down = false;
        for (_keyboard* k : b.m_keyboard)
        {
            if (!k)
                continue;
            any_bound = true;
            if (pInput->iGetAsyncKeyState(k->dik))
                down = true;
        }
        if (any_bound && !down)
            m_bUseKeyHeld = false;
    }

    if (Level().CurrentViewEntity() != this || eacFirstEye != cam_active)
        return false;
    if (HUD().GetUI() && HUD().GetUI()->MainInputReceiver())
        return false;

    if (m_bUseKeyHeld)
        return Device.dwTimeGlobal - m_dwUseKeyPressTime >= m_PickupInfoHoldMs;

    return m_bPickupMode && m_PickupInfoHoldMs == 0;
}

void CActor::PickupModeUpdate()
{
    m_PickupMarks.clear();

    if (!PickupInfoActive())
        return;

    //. ????? GetNearest ?????
    // NLC: one query covers both the dot radius and the name radius (docs/LOOT_INTERACTION_DESIGN.md 10)
    feel_touch_update(Position(), /*inventory().GetTakeDist()*/ _max(m_fPickupInfoRadius, m_fPickupInfoNameRadius));

    CFrustum frustum;
    frustum.CreateFromMatrix(Device.mFullTransform, FRUSTUM_P_LRTB | FRUSTUM_P_FAR);

    //. slow (ray-query test)
    for (xr_vector<CObject*>::iterator it = feel_touch.begin(); it != feel_touch.end(); it++)
    {
        // NLC: mark only what the pickup itself would accept (same filters as PickupModeUpdate_COD)
        CInventoryItem* item = smart_cast<CInventoryItem*>(*it);
        if (!item || item->object().H_Parent() || item->object().getDestroy() || !item->CanTake())
            continue;
        if (item->object().CLS_ID == CLSID_OBJECT_G_RPG7 || item->object().CLS_ID == CLSID_OBJECT_G_FAKE)
            continue;
        if (item->cast_game_object() && Level().m_feel_deny.is_object_denied(item->cast_game_object()))
            continue;

        if (CanPickItem(frustum, Device.vCameraPosition, *it))
            PickupInfoDraw(*it);
    }
}

#include "../xr_3da/camerabase.h"
#include "script_game_object.h"

void CActor::PickupModeUpdate_COD()
{
    if (Level().CurrentViewEntity() != this)
        return;

    if (!g_Alive() || eacFirstEye != cam_active)
    {
        m_PickupCandidateID = u16(-1);
        HUD().GetUI()->UIMainIngameWnd->SetPickUpItem(NULL);
        return;
    }

    //подбирание объекта
    if (inventory().m_pTarget 
        && inventory().m_pTarget->Useful() 
        && m_pUsableObject 
        && m_pUsableObject->nonscript_usable() &&
        !Level().m_feel_deny.is_object_denied(smart_cast<CGameObject*>(inventory().m_pTarget)))
    {
        CInventoryItem* pNearestItem = inventory().m_pTarget;
        m_PickupCandidateID = pNearestItem->object().ID();
        if (m_bPickupMode)
        {
            bool allow_pickup = true;

            if (pSettings->line_exist("engine_callbacks", "actor_on_item_before_pickup"))
            {
                std::string on_item_before_pickup = pSettings->r_string("engine_callbacks", "actor_on_item_before_pickup");

                // Tronex: ability to prevent item picking up if the export returns false
                luabind::functor<bool> func;
                if (ai().script_engine().functor(on_item_before_pickup.c_str(), func))
                {
                    allow_pickup = func(pNearestItem->cast_game_object()->lua_game_object());
                }
            }

            PickupModeOff();

            if (allow_pickup)
            {
                // подбирание объекта
                Game().SendPickUpEvent(ID(), pNearestItem->object().ID());
                pNearestItem = nullptr;
            }
        }

        HUD().GetUI()->UIMainIngameWnd->SetPickUpItem(pNearestItem);

        return;
    }

    // NLC: pickup_assist_enabled = false leaves only the crosshair path above (old NLC pixel-perfect pickup)
    if (!m_bPickupAssist)
    {
        m_PickupCandidateID = u16(-1);
        HUD().GetUI()->UIMainIngameWnd->SetPickUpItem(NULL);
        return;
    }

    CFrustum frustum;
    frustum.CreateFromMatrix(Device.mFullTransform, FRUSTUM_P_LRTB | FRUSTUM_P_FAR);

    //---------------------------------------------------------------------------
    ISpatialResult.clear();
    g_SpatialSpace->q_frustum(ISpatialResult, 0, STYPE_COLLIDEABLE, frustum);
    //---------------------------------------------------------------------------

    float maxlen = 1000.0f;

    // NLC: assist limits from actor.ltx (docs/LOOT_INTERACTION_DESIGN.md 2.2); defaults equal the stock 2 m / 1 m
    const float actor_r2 = _sqr(m_fPickupAssistActorRadius);
    const float cone_tan = m_fPickupAssistConeDeg > 0.f ? std::tan(deg2rad(_min(m_fPickupAssistConeDeg, 89.f))) : 0.f;

    CInventoryItem* pNearestItem = NULL;
    for (u32 o_it = 0; o_it < ISpatialResult.size(); o_it++)
    {
        ISpatial* spatial = ISpatialResult[o_it];
        CInventoryItem* pIItem = smart_cast<CInventoryItem*>(spatial->dcast_CObject());
        if (0 == pIItem)
            continue;

        if (pIItem->object().H_Parent() != NULL)
            continue;

        if (!pIItem->CanTake())
            continue;

        if (pIItem->object().CLS_ID == CLSID_OBJECT_G_RPG7 || pIItem->object().CLS_ID == CLSID_OBJECT_G_FAKE)
            continue;

        CGrenade* pGrenade = smart_cast<CGrenade*>(spatial->dcast_CObject());
        if (pGrenade && !pGrenade->Useful())
            continue;

        CMissile* pMissile = smart_cast<CMissile*>(spatial->dcast_CObject());
        if (pMissile && !pMissile->Useful())
            continue;

        Fvector A, B, tmp;
        pIItem->object().Center(A);
        if (A.distance_to_sqr(Position()) > actor_r2)
            continue;

        tmp.sub(A, cam_Active()->vPosition);
        const float depth = tmp.dotproduct(cam_Active()->vDirection);
        if (depth <= 0.f)
            continue;
        B.mad(cam_Active()->vPosition, cam_Active()->vDirection, depth);
        float len = B.distance_to_sqr(A);
        float ray_r = m_fPickupAssistRayRadius;
        if (cone_tan > 0.f)
            ray_r = _min(ray_r, _max(m_fPickupAssistMinRadius, depth * cone_tan));
        if (len > _sqr(ray_r))
            continue;

        if (maxlen > len && !pIItem->object().getDestroy())
        {
            maxlen = len;
            pNearestItem = pIItem;
        }
    }

    if (pNearestItem)
    {
        CFrustum frustum;
        frustum.CreateFromMatrix(Device.mFullTransform, FRUSTUM_P_LRTB | FRUSTUM_P_FAR);
        if (!CanPickItem(frustum, Device.vCameraPosition, &pNearestItem->object()))
            pNearestItem = NULL;
    }

    if (pNearestItem && pNearestItem->cast_game_object())
    {
        if (Level().m_feel_deny.is_object_denied(pNearestItem->cast_game_object()))
            pNearestItem = NULL;
    }

    if (pNearestItem)
    {
        if (m_bPickupMode)
        {
            bool allow_pickup = true;

            if (pSettings->line_exist("engine_callbacks", "actor_on_item_before_pickup"))
            {
                std::string on_item_before_pickup = pSettings->r_string("engine_callbacks", "actor_on_item_before_pickup");

                // Tronex: ability to prevent item picking up if the export returns false
                luabind::functor<bool> func;
                if (ai().script_engine().functor(on_item_before_pickup.c_str(), func))
                {
                    allow_pickup = func(pNearestItem->cast_game_object()->lua_game_object());
                }
            }

            PickupModeOff();

            if (allow_pickup)
            {
                // подбирание объекта
                Game().SendPickUpEvent(ID(), pNearestItem->object().ID());
                pNearestItem = nullptr;
            }
        }
    }

    m_PickupCandidateID = pNearestItem ? pNearestItem->object().ID() : u16(-1);
    HUD().GetUI()->UIMainIngameWnd->SetPickUpItem(pNearestItem);
};

void CActor::PickupInfoDraw(CObject* object)
{
    LPCSTR draw_str = NULL;

    CInventoryItem* item = smart_cast<CInventoryItem*>(object);
    //.	CInventoryOwner* inventory_owner = smart_cast<CInventoryOwner*>(object);
    //.	VERIFY(item || inventory_owner);
    if (!item)
        return;

    Fvector center;
    object->Center(center);
    const float dist = center.distance_to(Device.vCameraPosition);

    if ((m_PickupInfoMode & 1) && dist <= m_fPickupInfoRadius)
    {
        SPickupMark& mark = m_PickupMarks.emplace_back();
        mark.pos = center;
        mark.dist = dist;
        mark.id = object->ID();
    }

    if (!(m_PickupInfoMode & 2) || dist > m_fPickupInfoNameRadius)
        return;
    if (m_bPickupInfoNameTargetOnly && object->ID() != m_PickupCandidateID)
        return;

    Fvector4 v_res;

    draw_str = item->Name /*Complex*/ ();

    // NLC: the name sits above the item centre (where the dot is), not at the model origin
    Device.mFullTransform.transform(v_res, center);

    if (v_res.z < 0 || v_res.w < 0)
        return;
    if (v_res.x < -1.f || v_res.x > 1.f || v_res.y < -1.f || v_res.y > 1.f)
        return;

    float x = (1.f + v_res.x) / 2.f * (Device.dwWidth);
    float y = (1.f - v_res.y) / 2.f * (Device.dwHeight);
    if (m_PickupInfoMode & 1)
        y -= (m_fPickupInfoDotSize * 0.5f + 14.f) * float(Device.dwHeight) / UI_BASE_HEIGHT;

    HUD().Font().pFontLetterica16Russian->SetAligment(CGameFont::alCenter);
    HUD().Font().pFontLetterica16Russian->SetColor(PICKUP_INFO_COLOR);
    HUD().Font().pFontLetterica16Russian->Out(x, y, draw_str);
}

// NLC: draws the marks collected by PickupModeUpdate. Runs inside the HUD UI pass (CUIMainIngameWnd::Draw),
// so Device.mFullTransform is the current world view, as in CHUDTarget::Render.
void CActor::PickupInfoRender()
{
    if (!g_Alive()) // E-5: no marks left over from the moment of death
    {
        m_PickupMarks.clear();
        return;
    }
    if (m_PickupMarks.empty() || !(m_PickupInfoMode & 1))
        return;

    if (!m_pPickupDot)
    {
        m_pPickupDot = xr_new<CUIStaticItem>();
        m_pPickupDot->CreateShader(m_PickupInfoDotTexture.c_str(), "hud\\default");
    }

    const float kx = UI()->get_current_kx();
    const u16 target_id = m_bPickupInfoHighlightTarget ? m_PickupCandidateID : u16(-1);

    for (const SPickupMark& mark : m_PickupMarks)
    {
        Fvector4 v;
        Device.mFullTransform.transform(v, mark.pos);
        if (v.z < 0 || v.w < 0)
            continue;
        if (v.x < -1.f || v.x > 1.f || v.y < -1.f || v.y > 1.f)
            continue;

        const float f = m_fPickupInfoRadius > EPS ? std::clamp(mark.dist / m_fPickupInfoRadius, 0.f, 1.f) : 0.f;
        const float h = m_fPickupInfoDotSize + (m_fPickupInfoDotSizeFar - m_fPickupInfoDotSize) * f;
        const float w = h * kx; // keep the dot round on wide screens

        const float x = (1.f + v.x) * 0.5f * UI_BASE_WIDTH - w * 0.5f;
        const float y = (1.f - v.y) * 0.5f * UI_BASE_HEIGHT - h * 0.5f;

        const u32 base = (mark.id == target_id) ? m_PickupInfoTargetColor : m_PickupInfoColor;
        const float a = float(color_get_A(base)) * (1.f + (m_fPickupInfoFadeMin - 1.f) * f);
        const u32 color = subst_alpha(base, u32(std::clamp(iFloor(a + 0.5f), 0, 255)));

        m_pPickupDot->SetRect(0, 0, w, h);
        m_pPickupDot->SetPos(x, y);
        m_pPickupDot->SetColor(color);
        m_pPickupDot->Render();
    }
}

// NLC: script pickup that goes through the engine's own take path (docs/LOOT_INTERACTION_DESIGN.md 3.2, E2).
// Skips actor_on_item_before_pickup, so the pickup animation can take the item at its contact frame.
bool CActor::ScriptPickupItem(CScriptGameObject* obj)
{
    if (!obj)
    {
        Msg("! [pickup_item] refused: nil object");
        return false;
    }

    CInventoryItem* item = smart_cast<CInventoryItem*>(&obj->object());
    if (!item)
    {
        Msg("! [pickup_item] refused: [%s] is not an inventory item", obj->object().cName().c_str());
        return false;
    }
    if (!g_Alive())
    {
        Msg("! [pickup_item] refused: actor is dead");
        return false;
    }
    if (item->object().H_Parent() || item->object().getDestroy() || !item->CanTake())
    {
        Msg("! [pickup_item] refused: [%s] has a parent, is being destroyed or cannot be taken", obj->object().cName().c_str());
        return false;
    }
    // E-6: the same filters as PickupModeUpdate_COD (live grenades, rockets, fake grenades)
    const CLASS_ID cls = item->object().CLS_ID;
    CMissile* missile = smart_cast<CMissile*>(&item->object());
    if (cls == CLSID_OBJECT_G_RPG7 || cls == CLSID_OBJECT_G_FAKE || (missile && !missile->Useful()))
    {
        Msg("! [pickup_item] refused: [%s] is a live or fake projectile", obj->object().cName().c_str());
        return false;
    }
    if (item->cast_game_object() && Level().m_feel_deny.is_object_denied(item->cast_game_object()))
    {
        Msg("! [pickup_item] refused: [%s] is pickup-denied (just dropped)", obj->object().cName().c_str());
        return false;
    }

    Fvector c;
    item->object().Center(c);
    const float range = c.distance_to(Device.vCameraPosition);
    const float max_range = inventory().GetTakeDist() + 1.f;
    if (range > max_range)
    {
        Msg("! [pickup_item] refused: [%s] is %.2f m away (max %.2f)", obj->object().cName().c_str(), range, max_range);
        return false;
    }

    const u16 id = item->object().ID();
    if (id == m_PickupPendingID && Device.dwTimeGlobal - m_dwPickupPendingTime < 500)
    {
        Msg("! [pickup_item] refused: [%s] already sent", obj->object().cName().c_str());
        return false;
    }
    m_PickupPendingID = id;
    m_dwPickupPendingTime = Device.dwTimeGlobal;

    Game().SendPickUpEvent(ID(), id);
    return true;
}

void CActor::feel_sound_new(CObject* who, int type, CSound_UserDataPtr user_data, const Fvector& Position, float power)
{
    if (who == this)
        m_snd_noise = _max(m_snd_noise, power);
}
