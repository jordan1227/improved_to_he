#pragma once

IC void CMeleeChecker::load(LPCSTR section)
{
    m_as_min_dist = pSettings->r_float(section, "as_min_dist");
    m_as_step = pSettings->r_float(section, "as_step");

    m_min_attack_distance = pSettings->r_float(section, "MinAttackDist");
    m_max_attack_distance = pSettings->r_float(section, "MaxAttackDist");

    // NLC
    m_surface_mode = !!READ_IF_EXISTS(pSettings, r_bool, section, "melee_surface_distance", false);
    m_surface_offset = READ_IF_EXISTS(pSettings, r_float, section, "melee_surface_offset", 0.3f);
    m_cache_frame = u32(-1);
    s_debug_log = !!READ_IF_EXISTS(pSettings, r_bool, "monster_melee", "debug_log", false);
    s_trace_ignore_objects = !!READ_IF_EXISTS(pSettings, r_bool, "monster_melee", "trace_ignore_objects", false);
    s_close_yaw_dist = READ_IF_EXISTS(pSettings, r_float, "monster_melee", "close_yaw_dist", 0.f);
    s_close_yaw_half = READ_IF_EXISTS(pSettings, r_float, "monster_melee", "close_yaw_half", 0.f);
}

IC void CMeleeChecker::init_attack()
{
    // инициализировать стек
    for (u32 i = 0; i < HIT_STACK_SIZE; i++)
        m_hit_stack[i] = true;

    m_current_min_distance = m_min_attack_distance;
}
IC float CMeleeChecker::get_min_distance() { return m_current_min_distance; }
IC float CMeleeChecker::get_max_distance() { return (m_max_attack_distance - (m_min_attack_distance - m_current_min_distance)); }
IC bool CMeleeChecker::can_start_melee(const CEntity* enemy) { return (distance_to_enemy(enemy) < get_min_distance()); }
IC bool CMeleeChecker::should_stop_melee(const CEntity* enemy) { return (distance_to_enemy(enemy) > get_max_distance()); }
