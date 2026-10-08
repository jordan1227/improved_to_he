#pragma once
#include "../BaseMonster/base_monster.h"
#include "script_export_space.h"

class CCat : public CBaseMonster
{
    typedef CBaseMonster inherited;

public:
    CCat();
    virtual ~CCat();

    virtual void Load(LPCSTR section);
    virtual void reinit();

    virtual void UpdateCL();

    virtual void CheckSpecParams(u32 spec_params);

    void try_to_jump();

    virtual void HitEntityInJump(const CEntity* pEntity);
    virtual bool check_start_conditions(ControlCom::EControlType type); // NLC

private:
    // NLC: the stock attack jump was never wired for cats (CJumpingAbility commented out); nlc_jump_attack turns it on.
    // nlc_turn180 adds the run_turn_180_r_0/1 rotation jump (a running turnaround instead of a wide loop)
    bool m_nlc_jump_attack{};
    bool m_nlc_turn180{};

public:

    DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CCat)
#undef script_type_list
#define script_type_list save_type_list(CCat)
