#include "stdafx.h"
#include "grenade.h"
#include "PhysicsShell.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "actor.h"
#include "inventory.h"
#include "level.h"
#include "xrmessages.h"
#include "xr_level_controller.h"
#include "game_cl_base.h"
#include "xrserver_objects_alife.h"
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"

CGrenade::CGrenade(void)
{
    m_destroy_callback.clear();
    m_eSoundCheckout = ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);
}

CGrenade::~CGrenade(void) { HUD_SOUND::DestroySound(sndCheckout); }

void CGrenade::Load(LPCSTR section)
{
    inherited::Load(section);
    CExplosive::Load(section);

    HUD_SOUND::LoadSound(section, "snd_checkout", sndCheckout, m_eSoundCheckout);

    m_grenade_detonation_threshold_hit = READ_IF_EXISTS(pSettings, r_float, section, "detonation_threshold_hit", 100);
    m_nlc_detonate_by_bullet = READ_IF_EXISTS(pSettings, r_bool, section, "nlc_detonate_by_bullet", true);
    m_nlc_detonation_explosion_hit = READ_IF_EXISTS(pSettings, r_float, section, "nlc_detonation_explosion_hit", 5.f);
}

void CGrenade::Hit(SHit* pHDS)
{
    // NLC: a grenade lying or flying in the world (no owner, not exploding yet) explodes when a bullet hits it or an
    // explosion hits it with at least nlc_detonation_explosion_hit; the shooter / blast source becomes the initiator.
    // Stock tested CExplosive::Initiator() == u16(-1), which never holds (it falls back to the grenade's own ID).
    const bool by_bullet = m_nlc_detonate_by_bullet && ALife::eHitTypeFireWound == pHDS->hit_type;
    const bool by_blast = ALife::eHitTypeExplosion == pHDS->hit_type && pHDS->damage() >= m_nlc_detonation_explosion_hit;
    if ((by_bullet || by_blast) && !H_Parent() && CExplosive::Useful() && !getDestroy())
    {
        const bool held = m_nlc_tele_held;
        m_nlc_tele_held = false;
        if (pHDS->who)
            CExplosive::SetCurrentParentID(CExplosive::nlc_blame(pHDS->who));
        Msg("~ [grenade] %s [%d]%s detonated by %s from [%s] damage=%.2f", cNameSect().c_str(), ID(), held ? " (burer-held)" : "", by_bullet ? "bullet" : "explosion",
            pHDS->who ? pHDS->who->cName().c_str() : "?", pHDS->damage());
        Destroy();
    }
    inherited::Hit(pHDS);
}

BOOL CGrenade::net_Spawn(CSE_Abstract* DC)
{
    BOOL ret = inherited::net_Spawn(DC);
    Fvector box;
    BoundingBox().getsize(box);
    float max_size = _max(_max(box.x, box.y), box.z);
    box.set(max_size, max_size, max_size);
    box.mul(3.f);
    CExplosive::SetExplosionSize(box);
    m_thrown = false;
    return ret;
}

void CGrenade::net_Destroy()
{
    if (m_destroy_callback)
    {
        m_destroy_callback(this);
        m_destroy_callback = nullptr;
    }

    inherited::net_Destroy();
    CExplosive::net_Destroy();
}

void CGrenade::OnH_B_Independent(bool just_before_destroy) { inherited::OnH_B_Independent(just_before_destroy); }

// NLC: actor grenade throw diagnostics (a throw that "never left the hand" near burers), same switch as the burer log
static bool nlc_grenade_debug()
{
    static int value = -1;
    if (value < 0)
        value = pSettings->line_exist("m_burer_e", "nlc_burer_debug") && pSettings->r_bool("m_burer_e", "nlc_burer_debug") ? 1 : 0;
    return value == 1;
}

void CGrenade::OnH_A_Independent()
{
    inherited::OnH_A_Independent();
    if (nlc_grenade_debug() && m_pOwner && smart_cast<CActor*>(m_pOwner))
    {
        Fvector vel{};
        if (m_pPhysicsShell)
            m_pPhysicsShell->get_LinearVel(vel);
        Msg("~ [grenade] t=%u %s [%d] left the hand: shell=%d active=%d fuse=%u speed=%.1f", Device.dwTimeGlobal, cNameSect().c_str(), ID(), m_pPhysicsShell ? 1 : 0,
            m_pPhysicsShell && m_pPhysicsShell->isActive() ? 1 : 0, nlc_fuse_remaining(), vel.magnitude());
    }
}

void CGrenade::OnH_A_Chield()
{
    m_dwDestroyTime = 0xffffffff;
    inherited::OnH_A_Chield();
}

void CGrenade::State(u32 state, u32 oldState)
{
    if (nlc_grenade_debug() && smart_cast<CActor*>(H_Parent()) && (state == eThrowStart || state == eReady || state == eThrow || state == eThrowEnd))
        Msg("~ [grenade] t=%u actor %s [%d] state %u -> %u fake=%d thrown=%d", Device.dwTimeGlobal, cNameSect().c_str(), ID(), oldState, state,
            m_fake_missile ? int(m_fake_missile->ID()) : -1, m_thrown ? 1 : 0);
    switch (state)
    {
    case eThrowStart: {
        if (m_thrown) // NLC: see default below
        {
            Msg("~ [grenade] %s [%d] throw interrupted (state %u -> %u), finishing it", cNameSect().c_str(), ID(), oldState, state);
            nlc_finish_thrown();
            break;
        }
        Fvector C;
        Center(C);
        PlaySound(sndCheckout, C);
    }
    break;
    case eThrowEnd: {
        if (m_thrown)
            nlc_finish_thrown();
    }
    break;
    default:
        // NLC: the throw animation was interrupted after the grenade left the hand (eThrow -> idle / hide without
        // eThrowEnd); m_thrown stayed set and CGrenade::Throw returned early on every later throw, eating grenades.
        // Finish the old throw the way eThrowEnd does.
        if (m_thrown && state != eThrow)
        {
            Msg("~ [grenade] %s [%d] throw interrupted (state %u -> %u), finishing it", cNameSect().c_str(), ID(), oldState, state);
            nlc_finish_thrown();
        }
        break;
    };
    inherited::State(state, oldState);
}

void CGrenade::nlc_finish_thrown()
{
    m_thrown = false; // finish once: the object is destroyed below, later state changes must not repeat this
    if (m_pPhysicsShell)
        m_pPhysicsShell->Deactivate();
    xr_delete(m_pPhysicsShell);
    m_dwDestroyTime = 0xffffffff;

    if (H_Parent())
        PutNextToSlot();

    if (Local())
    {
#ifdef DEBUG
        Msg("Destroying local grenade[%d][%d]", ID(), Device.dwFrame);
#endif
        DestroyObject();
    }
}

void CGrenade::nlc_snatch_arm(u16 initiator)
{
    Fvector C;
    Center(C);
    PlaySound(sndCheckout, C);
    set_destroy_time(m_dwDestroyTimeMax);
    SetInitiator(initiator);
    Msg("~ [grenade] %s [%d] snatched: pin pulled by [%u], fuse %u ms", cNameSect().c_str(), ID(), initiator, m_dwDestroyTimeMax);
}

void CGrenade::Throw()
{
    if (nlc_grenade_debug() && smart_cast<CActor*>(H_Parent()))
        Msg("~ [grenade] t=%u actor throw %s [%d] fake=%d thrown=%d point_updated=%d", Device.dwTimeGlobal, cNameSect().c_str(), ID(),
            m_fake_missile ? int(m_fake_missile->ID()) : -1, m_thrown ? 1 : 0, m_bThrowPointUpdated ? 1 : 0);
    if (!m_fake_missile || m_thrown)
        return;

    CGrenade* pGrenade = smart_cast<CGrenade*>(m_fake_missile);
    VERIFY(pGrenade);

    if (pGrenade)
    {
        pGrenade->set_destroy_time(m_dwDestroyTimeMax);
        //установить ID того кто кинул гранату
        pGrenade->SetInitiator(H_Parent()->ID());
    }
    inherited::Throw();

    if (m_pCurrentInventory->GetOwner())
    {
        CActor* pActor = smart_cast<CActor*>(m_pCurrentInventory->GetOwner());
        if (pActor)
        {
            Actor()->set_state_wishful(Actor()->get_state_wishful() & (~mcSprint));
        }
    }

    m_fake_missile->processing_activate(); //@sliph
    m_thrown = true;

    // Real Wolf.Start.18.12.14
    auto parent = smart_cast<CGameObject*>(H_Parent());
    auto obj = smart_cast<CGameObject*>(m_fake_missile);
    if (parent && obj)
    {
        parent->callback(GameObject::eOnThrowGrenade)(obj->lua_game_object());
    }
    // Real Wolf.End.18.12.14
}

void CGrenade::Destroy()
{
    // Generate Expode event
    Fvector normal;

    if (m_destroy_callback)
    {
        m_destroy_callback(this);
        m_destroy_callback = nullptr;
    }

    FindNormal(normal);
    Fvector C;
    Center(C);
    CExplosive::GenExplodeEvent(C, normal);
}

bool CGrenade::Useful() const
{
    return !m_nlc_tele_held && m_dwDestroyTime == 0xffffffff && CExplosive::Useful() && TestServerFlag(CSE_ALifeObject::flCanSave);
}

void CGrenade::OnEvent(NET_Packet& P, u16 type)
{
    inherited::OnEvent(P, type);
    CExplosive::OnEvent(P, type);
}

void CGrenade::PutNextToSlot()
{
    VERIFY(!getDestroy());

    //выкинуть гранату из инвентаря
    if (m_pCurrentInventory)
    {
        NET_Packet P;
        m_pCurrentInventory->Ruck(this);

        this->u_EventGen(P, GEG_PLAYER_ITEM2RUCK, this->H_Parent()->ID());
        P.w_u16(this->ID());
        this->u_EventSend(P);

        CGrenade* pNext = smart_cast<CGrenade*>(m_pCurrentInventory->Same(this, true));
        if (!pNext)
            pNext = smart_cast<CGrenade*>(m_pCurrentInventory->SameSlot(GRENADE_SLOT, this, true));

        VERIFY(pNext != this);

        if (pNext && m_pCurrentInventory->Slot(pNext))
        {
            pNext->u_EventGen(P, GEG_PLAYER_ITEM2SLOT, pNext->H_Parent()->ID());
            P.w_u16(pNext->ID());
            pNext->u_EventSend(P);
            m_pCurrentInventory->SetActiveSlot(pNext->GetSlot());
        }
        else
        {
            CActor* pActor = smart_cast<CActor*>(m_pCurrentInventory->GetOwner());

            if (pActor)
                pActor->OnPrevWeaponSlot();
        }
    }
}

void CGrenade::TrySwitchGrenade(CGrenade* grenade)
{  
    static CGrenade* target{}; 
    static bool need_next{};

    if (!need_next && grenade)
    {
        target = grenade;
        need_next = true;
        SwitchState(eHiding);
    }
    else if (need_next)
    {
        need_next = false;
        if (m_pCurrentInventory && target)
        {
            m_pCurrentInventory->Ruck(this);
            m_pCurrentInventory->SetActiveSlot(NO_ACTIVE_SLOT);
            m_pCurrentInventory->Slot(target);
        }
        target = nullptr;
    }
}

void CGrenade::OnAnimationEnd(u32 state)
{
    switch (state)
    {
    case eThrowEnd: SwitchState(eHidden); break;
    case eHiding: {
        TrySwitchGrenade();
        setVisible(FALSE);
        SwitchState(eHidden);
        break;
    }
    default: inherited::OnAnimationEnd(state);
    }
}

void CGrenade::UpdateCL()
{
    inherited::UpdateCL();
    CExplosive::UpdateCL();
}

bool CGrenade::Action(s32 cmd, u32 flags)
{
    if (inherited::Action(cmd, flags))
        return true;

    switch (cmd)
    {
    //переключение типа гранаты
    case kWPN_NEXT: {
        if (flags & CMD_START)
        {
            const u32 state = GetState();
            if (m_pCurrentInventory && (state == eHidden || state == eIdle || state == eBore))
            {
                xr_map<shared_str, CGrenade*> tmp;
                tmp.insert(mk_pair(cNameSect(), this));
                for (auto it : m_pCurrentInventory->m_ruck)
                {
                    CGrenade* pGrenade = smart_cast<CGrenade*>(it);
                    if (pGrenade && (tmp.find(pGrenade->cNameSect()) == tmp.end()))
                        tmp.insert(mk_pair(pGrenade->cNameSect(), pGrenade));
                }
                xr_map<shared_str, CGrenade*>::iterator curr_it = tmp.find(cNameSect());
                curr_it++;
                CGrenade* tgt = curr_it != tmp.end() ? curr_it->second : tmp.begin()->second;
                if (tgt->cNameSect() != this->cNameSect()) // prxphet: Если тот же тип, ничего не делаем
                    TrySwitchGrenade(tgt);
            }
        }
        return true;
    };
    }
    return false;
}

BOOL CGrenade::UsedAI_Locations()
{
#pragma todo("Dima to Yura : It crashes, because on net_Spawn object doesn't use AI locations, but on net_Destroy it does use them")
    return TRUE; // m_dwDestroyTime == 0xffffffff;
}

void CGrenade::net_Relcase(CObject* O)
{
    CExplosive::net_Relcase(O);
    inherited::net_Relcase(O);
}

void CGrenade::Deactivate(bool now)
{
    // Drop grenade if primed
    StopCurrentAnimWithoutCallback();
    CEntityAlive* entity = smart_cast<CEntityAlive*>(m_pCurrentInventory->GetOwner());
    if (!entity->g_Alive() && !GetTmpPreDestroy() && Local() && (GetState() == eThrowStart || GetState() == eReady || GetState() == eThrow))
    {
        if (m_fake_missile)
        {
            CGrenade* pGrenade = smart_cast<CGrenade*>(m_fake_missile);
            if (pGrenade)
            {
                if (m_pCurrentInventory->GetOwner())
                {
                    CActor* pActor = smart_cast<CActor*>(m_pCurrentInventory->GetOwner());
                    if (pActor)
                    {
                        if (!pActor->g_Alive())
                        {
                            m_constpower = false;
                            m_fThrowForce = 0;
                        }
                    }
                }
                Throw();
            };
        };
    };

    inherited::Deactivate(now || (GetState() == eThrowStart || GetState() == eReady || GetState() == eThrow));
}

void CGrenade::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
    str_name = NameShort();
    u32 ThisGrenadeCount = m_pCurrentInventory->dwfGetSameItemCount(*cNameSect(), true);
    string16 stmp;
    sprintf_s(stmp, "%d", ThisGrenadeCount);
    str_count = stmp;
    icon_sect_name = *cNameSect();
}

bool CGrenade::CanTake() const
{
    if (m_nlc_tele_held)
        return false;
    if (!inherited::CanTake())
        return false;

    return !m_explosion_flags.test(flExploding) && !m_explosion_flags.test(flExploded);
}
