#pragma once

// NLC: NPC bullet damage model switches ([nlc_npc_damage] in the game config, read once).
// docs/damage_model/DESIGN_npc_damage_model.md section 14. Missing keys keep the original engine behaviour.
struct SNlcNpcDamage
{
    bool apply_ap{};          // ammo k_ap reduces NPC bone armour (was APPLY_ARMOR_PIERCING_TO_NPC)
    bool use_bone_koeff{};    // bone-protection koeff column scales the post-armour bullet power
    float ap_koeff_bypass{1.f}; // share of k_ap that lifts koeff towards 1
    bool apply_ap_actor{true};  // ammo k_ap reduces the actor's outfit armour (original behaviour)
    xr_vector<shared_str> legacy_communities; // these communities keep the pre-pass NPC armour path (no k_ap, no koeff)
    bool stagger_log{}; // one log line per impact / concussion stagger attempt (section 21.1)

    bool is_legacy(const shared_str& community) const
    {
        return std::find(legacy_communities.begin(), legacy_communities.end(), community) != legacy_communities.end();
    }
};

// NLC: ammo nlc_blunt_k (head bones: nlc_blunt_head_k, default nlc_blunt_k) of the weapon that fired a hit
// (its current ammo type), 1 when unknown. docs/damage_model/DESIGN_npc_damage_model.md 21; ai_stalker_fire.cpp
float nlc_hit_blunt_k(u16 weapon_id, bool head = false);

inline const SNlcNpcDamage& nlc_npc_damage()
{
    static const SNlcNpcDamage s = [] {
        SNlcNpcDamage r;
        constexpr LPCSTR sect = "nlc_npc_damage";
        if (pSettings->section_exist(sect))
        {
            r.apply_ap = READ_IF_EXISTS(pSettings, r_bool, sect, "apply_ap", r.apply_ap);
            r.use_bone_koeff = READ_IF_EXISTS(pSettings, r_bool, sect, "use_bone_koeff", r.use_bone_koeff);
            r.ap_koeff_bypass = READ_IF_EXISTS(pSettings, r_float, sect, "ap_koeff_bypass", r.ap_koeff_bypass);
            r.apply_ap_actor = READ_IF_EXISTS(pSettings, r_bool, sect, "apply_ap_actor", r.apply_ap_actor);
            r.stagger_log = READ_IF_EXISTS(pSettings, r_bool, sect, "stagger_log", r.stagger_log);
            if (pSettings->line_exist(sect, "legacy_communities"))
            {
                LPCSTR list = pSettings->r_string(sect, "legacy_communities");
                string256 item;
                for (int i = 0, n = _GetItemCount(list); i < n; ++i)
                    r.legacy_communities.push_back(_GetItem(list, i, item));
            }
        }
        Msg("* [nlc_npc_damage] apply_ap=%d use_bone_koeff=%d ap_koeff_bypass=%.2f apply_ap_actor=%d legacy_communities=%u", r.apply_ap, r.use_bone_koeff,
            r.ap_koeff_bypass, r.apply_ap_actor, u32(r.legacy_communities.size()));
        return r;
    }();
    return s;
}

// NLC: actor bullet damage model ([nlc_actor_damage], read once). docs/damage_model/DESIGN_actor_damage_model.md
// sections 3-4 and 11-15. Off (or a suit without bullet_protection_sect) keeps the original path.
//   pen   = max(p - A*cf*(1-k_ap), p*hit_fraction)
//   K     = 1 - cov*(1 - (1 - F*cf)^fw_protection_exp), lifted by k_ap*ap_koeff_bypass
//   power = pen*K;  cf = min(1, condition/cond_full);  F = live fire_wound_protection
// Head-part bones take cov/armour from the worn mask (nlc_set_head_cover) and bullet_head_scale.
struct SNlcActorDamage
{
    bool enabled{};
    float fw_protection_exp{3.f};
    float cond_full{1.f};
    float ap_koeff_bypass{0.5f};
    float bullet_head_scale{-1.f}; // < 0: keep the damage-table hit scale
    float blunt_head{}; // blunt-impact minimum on the head, x the worn mask's coverage (section 21)
    // single health pool only (body-part health off): bullet health loss on arm / leg bones (clavicles stay torso);
    // injuries, bleeding, mutant hits and body-part health keep the unscaled value
    // (docs/damage_model/DESIGN_npc_accuracy_and_leg_hits.md 10g, design A). 1 = off
    float bullet_arm_pool_scale{1.f};
    float bullet_leg_pool_scale{1.f};
    bool log{};
};

inline const SNlcActorDamage& nlc_actor_damage()
{
    static const SNlcActorDamage s = [] {
        SNlcActorDamage r;
        constexpr LPCSTR sect = "nlc_actor_damage";
        if (pSettings->section_exist(sect))
        {
            r.enabled = READ_IF_EXISTS(pSettings, r_bool, sect, "enabled", r.enabled);
            r.fw_protection_exp = READ_IF_EXISTS(pSettings, r_float, sect, "fw_protection_exp", r.fw_protection_exp);
            r.cond_full = READ_IF_EXISTS(pSettings, r_float, sect, "cond_full", r.cond_full);
            r.ap_koeff_bypass = READ_IF_EXISTS(pSettings, r_float, sect, "ap_koeff_bypass", r.ap_koeff_bypass);
            r.bullet_head_scale = READ_IF_EXISTS(pSettings, r_float, sect, "bullet_head_scale", r.bullet_head_scale);
            r.blunt_head = READ_IF_EXISTS(pSettings, r_float, sect, "blunt_head", r.blunt_head);
            r.bullet_arm_pool_scale = READ_IF_EXISTS(pSettings, r_float, sect, "bullet_arm_pool_scale", r.bullet_arm_pool_scale);
            r.bullet_leg_pool_scale = READ_IF_EXISTS(pSettings, r_float, sect, "bullet_leg_pool_scale", r.bullet_leg_pool_scale);
            r.log = READ_IF_EXISTS(pSettings, r_bool, sect, "log", r.log);
        }
        if (r.cond_full < EPS_L)
            r.cond_full = 1.f;
        Msg("* [nlc_actor_damage] enabled=%d fw_protection_exp=%.2f cond_full=%.2f ap_koeff_bypass=%.2f bullet_head_scale=%.2f blunt_head=%.3f arm_pool=%.2f leg_pool=%.2f log=%d",
            r.enabled, r.fw_protection_exp, r.cond_full, r.ap_koeff_bypass, r.bullet_head_scale, r.blunt_head, r.bullet_arm_pool_scale, r.bullet_leg_pool_scale, r.log);
        return r;
    }();
    return s;
}
