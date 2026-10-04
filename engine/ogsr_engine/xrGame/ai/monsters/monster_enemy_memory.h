#pragma once

#include "ai_monster_defs.h"

class CBaseMonster;

class CMonsterEnemyMemory
{
    CBaseMonster* monster;
    TTime time_memory;

    ENEMIES_MAP m_objects;

    // NLC: config-driven target selection, [monster_target_selection] in
    // game_relations.ltx with optional per-monster-section overrides.
    // Identity defaults (1.0 / 0) keep the original danger formula.
    float m_actor_bias;
    float m_target_stickiness;
    float m_hit_bonus;
    TTime m_hit_bonus_time;
    float m_damage_threat_k; // multiplier = min(1 + k * recent_damage, max); 0 = off
    float m_damage_threat_max;
    float m_back_hit_threat; // recorded as damage on a back hit from within m_back_hit_max_dist; 0 = off
    float m_back_hit_max_dist;

public:
    CMonsterEnemyMemory();
    ~CMonsterEnemyMemory();

    void init_external(CBaseMonster* M, TTime mem_time);
    void load(LPCSTR section);
    void update();

    // danger score of a remembered enemy, -1 if it is not remembered
    float get_danger(const CEntityAlive* enemy) const;
    float damage_threat_multiplier(const CEntityAlive* enemy) const;
    float back_hit_threat() const { return m_back_hit_threat; }
    float back_hit_max_dist() const { return m_back_hit_max_dist; }
    static bool target_debug_log();

    // -----------------------------------------------------
    const CEntityAlive* get_enemy();
    SMonsterEnemy get_enemy_info();
    u32 get_enemies_count() { return u32(m_objects.size()); }

    const ENEMIES_MAP& get_memory() { return m_objects; }

    void clear() { m_objects.clear(); }
    void remove_links(CObject* O);

    void add_enemy(const CEntityAlive* enemy);
    void add_enemy(const CEntityAlive* enemy, const Fvector& pos, u32 vertex, u32 time);

private:
    void remove_non_actual();

    ENEMIES_MAP_IT find_best_enemy();
};
