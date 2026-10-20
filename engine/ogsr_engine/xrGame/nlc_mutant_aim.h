#pragma once

// NLC: NPC aim and engagement against mutants ([nlc_npc_mutant_aim] in the game config, read once; script can
// override values at runtime through level.nlc_mut_aim_param). docs/damage_model/DESIGN_npc_aim_vs_mutants.md section 6.
// Everything here applies only while a stalker's sight target is a CBaseMonster; the actor and stalkers keep
// the original behaviour.
struct SNlcMutantAim
{
    bool enabled{true};         // master switch (A1-A5, B1-B2)
    bool fresh_fire_dir{true};  // A1: shoot along the current head direction, not the scheduler-stale eye_matrix
    float lag_ms[4]{150.f, 110.f, 80.f, 60.f};     // A2: tracking lag by rank point (novice .. master)
    float rank_points[4]{0.f, 300.f, 600.f, 900.f}; // rank values of the lag_ms points (linear between)
    float lag_jitter{0.3f};     // +- share, re-rolled every lag_reroll_ms (about one burst)
    float lag_reroll_ms{800.f};
    float lag_k{1.f};           // global lag multiplier (tuning)
    float lag_max_ms{700.f};
    float head_speed{270.f};    // A4: danger head speed (deg/s) against a mutant closer than snap_dist (270 = stock)
    float snap_dist{8.f};
    float cloak_cone_deg{1.5f}; // A3: extra dispersion against a cloaked bloodsucker
    float xray_cone_deg{0.75f}; // ... against one in x-ray
    float spread_deg{0.f};      // A5: extra dispersion against any mutant (0 = off)
    float psy_cone_deg{3.f};    // psy pressure: extra dispersion against mutants, full at psy health psy_cone_full
    float psy_cone_full{0.25f}; // (linear from psy 1; psy 0 counts as unaffected, see GetWeaponAccuracy)
    float flinch_cone_deg{6.f}; // melee flinch: extra dispersion for flinch_ms after a mutant hit us from within flinch_dist
    float flinch_ms{1000.f};
    float flinch_dist{3.f};
    float flinch_k_novice{1.5f}; // flinch cone x this at rank_points[0] .. flinch_k_master at rank_points[3] (linear)
    float flinch_k_master{0.6f};
    float approach_dist{30.f};  // B2: a mutant this close that attacks the stalker, a friend or the actor is not ignored
    float help_dist_friend{30.f};  // B2/B3: actor's friend helps against a mutant this close to the stalker
    float help_dist_neutral{10.f}; // ... a neutral only this close
    float threat_k{1.f};        // B1: multiplier on the monsters' nlc_npc_threat_dist
};

inline void nlc_mutant_aim_list(const SNlcMutantAim& r, string512& out)
{
    xr_sprintf(out,
               "enabled=%d fresh_fire_dir=%d lag_ms=%.0f,%.0f,%.0f,%.0f rank_points=%.0f,%.0f,%.0f,%.0f lag_jitter=%.2f lag_reroll_ms=%.0f lag_k=%.2f "
               "lag_max_ms=%.0f head_speed=%.0f snap_dist=%.1f cloak_cone_deg=%.2f xray_cone_deg=%.2f spread_deg=%.2f psy_cone_deg=%.2f "
               "psy_cone_full=%.2f flinch_cone_deg=%.1f flinch_ms=%.0f flinch_dist=%.1f flinch_k=%.2f..%.2f approach_dist=%.1f "
               "help_dist_friend=%.1f help_dist_neutral=%.1f threat_k=%.2f",
               r.enabled, r.fresh_fire_dir, r.lag_ms[0], r.lag_ms[1], r.lag_ms[2], r.lag_ms[3], r.rank_points[0], r.rank_points[1], r.rank_points[2],
               r.rank_points[3], r.lag_jitter, r.lag_reroll_ms, r.lag_k, r.lag_max_ms, r.head_speed, r.snap_dist, r.cloak_cone_deg, r.xray_cone_deg,
               r.spread_deg, r.psy_cone_deg, r.psy_cone_full, r.flinch_cone_deg, r.flinch_ms, r.flinch_dist, r.flinch_k_novice, r.flinch_k_master,
               r.approach_dist, r.help_dist_friend, r.help_dist_neutral, r.threat_k);
}

inline SNlcMutantAim& nlc_mutant_aim()
{
    static SNlcMutantAim s = [] {
        SNlcMutantAim r;
        constexpr LPCSTR sect = "nlc_npc_mutant_aim";
        if (pSettings->section_exist(sect))
        {
            auto read4 = [&](LPCSTR key, float* v) {
                if (!pSettings->line_exist(sect, key))
                    return;
                LPCSTR str = pSettings->r_string(sect, key);
                string64 item;
                const int n = _GetItemCount(str);
                for (int i = 0; i < 4 && i < n; ++i)
                    v[i] = float(atof(_GetItem(str, i, item)));
            };
            r.enabled = READ_IF_EXISTS(pSettings, r_bool, sect, "enabled", r.enabled);
            r.fresh_fire_dir = READ_IF_EXISTS(pSettings, r_bool, sect, "fresh_fire_dir", r.fresh_fire_dir);
            read4("lag_ms", r.lag_ms);
            read4("rank_points", r.rank_points);
            r.lag_jitter = READ_IF_EXISTS(pSettings, r_float, sect, "lag_jitter", r.lag_jitter);
            r.lag_reroll_ms = READ_IF_EXISTS(pSettings, r_float, sect, "lag_reroll_ms", r.lag_reroll_ms);
            r.lag_k = READ_IF_EXISTS(pSettings, r_float, sect, "lag_k", r.lag_k);
            r.lag_max_ms = READ_IF_EXISTS(pSettings, r_float, sect, "lag_max_ms", r.lag_max_ms);
            r.head_speed = READ_IF_EXISTS(pSettings, r_float, sect, "head_speed", r.head_speed);
            r.snap_dist = READ_IF_EXISTS(pSettings, r_float, sect, "snap_dist", r.snap_dist);
            r.cloak_cone_deg = READ_IF_EXISTS(pSettings, r_float, sect, "cloak_cone_deg", r.cloak_cone_deg);
            r.xray_cone_deg = READ_IF_EXISTS(pSettings, r_float, sect, "xray_cone_deg", r.xray_cone_deg);
            r.spread_deg = READ_IF_EXISTS(pSettings, r_float, sect, "spread_deg", r.spread_deg);
            r.psy_cone_deg = READ_IF_EXISTS(pSettings, r_float, sect, "psy_cone_deg", r.psy_cone_deg);
            r.psy_cone_full = READ_IF_EXISTS(pSettings, r_float, sect, "psy_cone_full", r.psy_cone_full);
            r.flinch_cone_deg = READ_IF_EXISTS(pSettings, r_float, sect, "flinch_cone_deg", r.flinch_cone_deg);
            r.flinch_ms = READ_IF_EXISTS(pSettings, r_float, sect, "flinch_ms", r.flinch_ms);
            r.flinch_dist = READ_IF_EXISTS(pSettings, r_float, sect, "flinch_dist", r.flinch_dist);
            r.flinch_k_novice = READ_IF_EXISTS(pSettings, r_float, sect, "flinch_k_novice", r.flinch_k_novice);
            r.flinch_k_master = READ_IF_EXISTS(pSettings, r_float, sect, "flinch_k_master", r.flinch_k_master);
            r.approach_dist = READ_IF_EXISTS(pSettings, r_float, sect, "approach_dist", r.approach_dist);
            r.help_dist_friend = READ_IF_EXISTS(pSettings, r_float, sect, "help_dist_friend", r.help_dist_friend);
            r.help_dist_neutral = READ_IF_EXISTS(pSettings, r_float, sect, "help_dist_neutral", r.help_dist_neutral);
            r.threat_k = READ_IF_EXISTS(pSettings, r_float, sect, "threat_k", r.threat_k);
        }
        else
            Msg("! [nlc_npc_mutant_aim] section missing, engine defaults");
        string512 s;
        nlc_mutant_aim_list(r, s);
        Msg("* [nlc_npc_mutant_aim] %s", s);
        return r;
    }();
    return s;
}

// script access by key name (level.nlc_mut_aim_param / level.nlc_mut_aim_get); lag_ms_N / rank_points_N address
// the four points (N = 0..3). Returns false for an unknown name.
inline bool nlc_mutant_aim_field(LPCSTR name, float** f, bool** b)
{
    SNlcMutantAim& r = nlc_mutant_aim();
    *f = nullptr;
    *b = nullptr;
    if (!xr_strcmp(name, "enabled"))
        *b = &r.enabled;
    else if (!xr_strcmp(name, "fresh_fire_dir"))
        *b = &r.fresh_fire_dir;
    else if (!strncmp(name, "lag_ms_", 7) && name[7] >= '0' && name[7] <= '3' && !name[8])
        *f = &r.lag_ms[name[7] - '0'];
    else if (!strncmp(name, "rank_points_", 12) && name[12] >= '0' && name[12] <= '3' && !name[13])
        *f = &r.rank_points[name[12] - '0'];
    else if (!xr_strcmp(name, "lag_jitter"))
        *f = &r.lag_jitter;
    else if (!xr_strcmp(name, "lag_reroll_ms"))
        *f = &r.lag_reroll_ms;
    else if (!xr_strcmp(name, "lag_k"))
        *f = &r.lag_k;
    else if (!xr_strcmp(name, "lag_max_ms"))
        *f = &r.lag_max_ms;
    else if (!xr_strcmp(name, "head_speed"))
        *f = &r.head_speed;
    else if (!xr_strcmp(name, "snap_dist"))
        *f = &r.snap_dist;
    else if (!xr_strcmp(name, "cloak_cone_deg"))
        *f = &r.cloak_cone_deg;
    else if (!xr_strcmp(name, "xray_cone_deg"))
        *f = &r.xray_cone_deg;
    else if (!xr_strcmp(name, "spread_deg"))
        *f = &r.spread_deg;
    else if (!xr_strcmp(name, "psy_cone_deg"))
        *f = &r.psy_cone_deg;
    else if (!xr_strcmp(name, "psy_cone_full"))
        *f = &r.psy_cone_full;
    else if (!xr_strcmp(name, "flinch_cone_deg"))
        *f = &r.flinch_cone_deg;
    else if (!xr_strcmp(name, "flinch_ms"))
        *f = &r.flinch_ms;
    else if (!xr_strcmp(name, "flinch_dist"))
        *f = &r.flinch_dist;
    else if (!xr_strcmp(name, "flinch_k_novice"))
        *f = &r.flinch_k_novice;
    else if (!xr_strcmp(name, "flinch_k_master"))
        *f = &r.flinch_k_master;
    else if (!xr_strcmp(name, "approach_dist"))
        *f = &r.approach_dist;
    else if (!xr_strcmp(name, "help_dist_friend"))
        *f = &r.help_dist_friend;
    else if (!xr_strcmp(name, "help_dist_neutral"))
        *f = &r.help_dist_neutral;
    else if (!xr_strcmp(name, "threat_k"))
        *f = &r.threat_k;
    return *f || *b;
}

inline bool nlc_mutant_aim_param(LPCSTR name, float value)
{
    float* f;
    bool* b;
    if (!name || !nlc_mutant_aim_field(name, &f, &b))
    {
        Msg("! [nlc_npc_mutant_aim] unknown parameter [%s]", name ? name : "<null>");
        return false;
    }
    if (f)
        *f = value;
    else
        *b = !fis_zero(value);
    Msg("* [nlc_npc_mutant_aim] script set %s = %.3f", name, value);
    return true;
}

inline float nlc_mutant_aim_get(LPCSTR name)
{
    float* f;
    bool* b;
    if (!name || !nlc_mutant_aim_field(name, &f, &b))
        return -1.f;
    return f ? *f : (*b ? 1.f : 0.f);
}
