#pragma once
#include "ai_monster_defs.h"

class CBaseMonster;

class CMonsterHitMemory
{
    CBaseMonster* monster;
    TTime time_memory;

    MONSTER_HIT_VECTOR m_hits;

    // NLC: health lost per attacker (fraction of max health), decays linearly to 0
    // over m_damage_memory_time; feeds damage-weighted target selection.
    // Objects are only compared by pointer, never dereferenced.
    struct SDamageInfo
    {
        const CObject* who;
        float damage;
        TTime time;
    };
    xr_vector<SDamageInfo> m_damage;
    TTime m_damage_memory_time;

    float decayed_damage(const SDamageInfo& info) const;

public:
    CMonsterHitMemory();
    ~CMonsterHitMemory();

    void init_external(CBaseMonster* M, TTime mem_time);
    void update();

    // -----------------------------------------------------
    bool is_hit() { return !m_hits.empty(); }
    bool is_hit(CObject* pO);

    // Lain: added
    int get_num_hits() { return u32(m_hits.size()); }

    void add_hit(CObject* who, EHitSide side);

    Fvector get_last_hit_dir();
    TTime get_last_hit_time();
    TTime get_last_hit_time(const CObject* who); // NLC: latest hit by `who`, 0 if none
    CObject* get_last_hit_object();
    Fvector get_last_hit_position();

    void clear()
    {
        m_hits.clear();
        m_damage.clear(); // NLC
    }

    // NLC: damage-weighted target selection
    void set_damage_memory_time(TTime t) { m_damage_memory_time = t; }
    void add_damage(const CObject* who, float health_fraction);
    float get_recent_damage(const CObject* who) const; // decayed fraction of max health, 0 if none

    void remove_hit_info(const CObject* obj);

private:
    void remove_non_actual();
};
