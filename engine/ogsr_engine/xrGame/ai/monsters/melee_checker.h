#pragma once
class CBaseMonster;
class CEntity;

#define HIT_STACK_SIZE 2

class CMeleeChecker
{
private:
    collide::rq_results r_res;

private:
    CBaseMonster* m_object{};

    // ltx parameters
    float m_min_attack_distance{};
    float m_max_attack_distance{};
    float m_as_min_dist{};
    float m_as_step{};

    bool m_hit_stack[HIT_STACK_SIZE]{};

    float m_current_min_distance{};

    // NLC: surface-aware distance (melee_surface_distance / melee_surface_offset), off by default
    bool m_surface_mode{};
    float m_surface_offset{0.3f};
    u32 m_cache_frame{u32(-1)};
    u16 m_cache_enemy{u16(-1)};
    float m_cache_value{};
    u32 m_last_attempt_time{}; // NLC
    static bool s_debug_log;
    static bool s_trace_ignore_objects;
    static float s_close_yaw_dist;
    static float s_close_yaw_half;

    float surface_distance(const CEntity* enemy, float centre);

public:
    void init_external(CBaseMonster* obj) { m_object = obj; }
    IC void load(LPCSTR section);

    // инициализировано состояние атаки
    IC void init_attack();
    void on_hit_attempt(bool hit_success);

    // Получить расстояние от fire_bone до врага
    // Выполнить RayQuery от fire_bone в enemy.center
    float distance_to_enemy(const CEntity* enemy);

    bool surface_mode() const { return m_surface_mode; }
    // NLC: time of the last bite/strike hit check (siege low-crate fallback)
    u32 last_attempt_time() const { return m_last_attempt_time; }
    // [monster_melee] debug_log in game_relations.ltx
    static bool debug_log() { return s_debug_log; }
    // [monster_melee] trace_ignore_objects: the actor line-of-hit trace is blocked only by level geometry
    static bool trace_ignore_objects() { return s_trace_ignore_objects; }
    // [monster_melee] close_yaw_dist / close_yaw_half: below this bite distance the yaw window is at least +-half
    static float close_yaw_dist() { return s_close_yaw_dist; }
    static float close_yaw_half() { return s_close_yaw_half; }

    IC float get_min_distance();
    IC float get_max_distance();

    IC bool can_start_melee(const CEntity* enemy);
    IC bool should_stop_melee(const CEntity* enemy);

#ifdef DEBUG
    IC float dbg_as_min_dist() { return m_as_min_dist; }
    IC float dbg_as_step() { return m_as_step; }
#endif
};

#include "melee_checker_inline.h"
