# NLC engine changelog

One entry per shipped build (tag `nlc-<ogsr>.<n>` in `jordan1227/improved_to_he`).
Newest first. The engine source lives in this repository under `engine/`, next to the
game data of the same build. The exe and its PDB are attached to the GitHub Release
of the tag in this repository.

## nlc-3.589.16 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `64321D0632C68F12C95F556FA554C7825E18F81E9698D77E45FA98016DFACB49`
- **Status:** runtime pending. Fixes from a code review of the stealth base (nlc-3.589.4 to .10).

### NLC

- **NPC-vs-NPC sight restored:** vision sections get `nlc_actor_rate_k` (actor-only rate factor);
  `stalker_vision_danger` is back to vanilla `time_quant` 0.001 with `nlc_actor_rate_k` 0.35 (same actor
  detection as tested). Before, the slower danger vision applied to every target (NPCs re-acquired
  NPC enemies about 2.9x slower).
- **Monster gates actor-only:** pack enemy sharing, the impact range gate and the near-hit gate apply
  only to the actor; NPC enemies and shooters as vanilla.
- **Near misses** use the segment the bullet really flew (clipped at its hit, capped at 60 m), not the
  whole step (NPCs behind a wall got near misses).
- **Pack far notices** are queued and handled on the main thread.
- **Squad share skip** (`squad_dying_share`) only for a dying member whose enemy is the actor.
- **Wall muffle cost:** listeners within 60 m only, 500 ms staggered cache, 3 m position tolerance.
- **Surface table** read in file order (`Ordered_Data`; first match wins as documented).
- **Psy/smell senses:** actor speed clamped (10 m/s); a jump over 5 m (teleport) is not movement.
- Outfit key caches keyed by id and section; species `pack_share_delay_min` and `nlc_rain_k` clamped.

## nlc-3.589.15 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `30B28B98AF2C159417A643BF11C34D06B6C7AA222F5EAC64D7BA52E2BCE81D98`
- **Status:** runtime pending. Fixes from a code review of the stealth pass 5 engine changes.

### NLC

- **Wall muffle:** passable hits (bushes) are stepped over instead of ending the check, and the ray runs
  both ways (the static pick culls back faces, so single-sided walls blocked in one direction only).
- **Psy auras:** `actor_psy_k` defaults to -1 = the actor's telepathic immunity (vanilla) until the
  script pushes the gear factor; before, psy gear stopped helping when the script had not run.
- **Queues:** an atomic flag replaces the unlocked `empty()` checks; queued bolt contacts and near-miss
  segments carry their level and are dropped after a level change (ids are reused).
- **Bolt contact** position from the contact geometry (the hook runs on the physics thread).
- **Monster hunts:** search and flank points are never swallowed by the "no retarget within 8 m" rule;
  hunt, alert, concern and wall-cache state reset on reinit; species hunt keys clamped.
- **Planner dump** relies on `CPlanner::update`'s own `__except` (a C++ try is not allowed there).

## nlc-3.589.14 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F6AAE6CE16302CC05DFDEA8B7A915E4F4602DB11DCB6AC2D14D0D2F1B60B2154`
- **Status:** runtime pending.

### NLC

- **Squad enemy share** (`agent_enemy_manager.cpp`, `squad_dying_share`): the squad agent shares the
  enemies of its combat members with the squad (`make_object_visible_somewhen`, memory masks). A
  member killed by a hit stays alive for a frame or two with the attacker as his enemy, so a silent
  one-shot kill told the whole squad (all Agroprom military) where the actor was. With
  `squad_dying_share` 0 a member at 0 health contributes no enemies; living members share as before.

## nlc-3.589.13 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CBBCEF9E8FB60240EB8503CF250537DB3EE4501CF2024FD265BEEC48E2AD499D`
- **Status:** runtime pending. Diagnostics only on top of `nlc-3.589.12`.

### NLC

- **Planner freeze diagnostics** (`action_planner_inline.h`): when an action planner finds no
  solution (the NPC freezes, `[CPlanner::update]: <name> has solution().empty()`), the engine also
  logs, once per freeze, every evaluator's value (`world:`), the goal (`goal:`) and each action's
  unmet preconditions (`unmet:`). Freezes of Agroprom soldiers and others predate the stealth work
  (logs of 30.09 and 02.10 before the suspicion scheme); this finds the blocking property.

## nlc-3.589.12 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7131F07152D3DE0DB4290B47F6D6956660D0055F10ADB446D91417E055135FB9`
- **Status:** runtime pending (stealth pass 5, `docs/STEALTH_DESIGN.md` 20). Replaces the
  unshipped test build `nlc-3.589.11` (`C30A6943`), which crashed on the second bolt hit: the
  queued bolt contacts were handled in `CPHWorld::OnFrame`, which runs on the second thread
  (`seqFrameMT`), and called Lua from there. Now handled in `CLevel::OnFrame` (main thread).
  Actor bullet near misses (M3, from the bullet manager's parallel update) are queued and handled
  there too, so AI memory is no longer written from the parallel thread.

### NLC: stealth pass 5 (`xrGame/nlc_stealth.cpp/.h`)

New parameters are identity by default; `sivol_stealth_cfg.script` switches them on.

- **Monster hunt** (`hunt`; species `hunt_time`, `hunt_error_k`, `hunt_detect_k`, `hunt_alert_ms`,
  `hunt_flankers`, `hunt_flee_losses`): a bold monster whose third concern finds the actor out of
  `concern_enemy_range` commits to a hunt of the guessed shooter point (half the error), searches
  around it, sends pack flankers to points beside it, and afterwards stays alert ("lost the
  trail"); vision and aura senses run faster (`m_nlc_rate_k`, now applied to monsters too).
  A pack that lost `hunt_flee_losses` members flees instead.
- **Investigate impulses:** every repeated concern halves the point error; the gait is chosen
  once per episode and only upgrades; no retarget to a point within 8 m of the current target;
  the creature a bullet passes through gets no near miss.
- **Psy auras** use the live `actor_psy_k` (pushed by script from suit and belt) instead of the
  actor's telepathic immunity, which scales with difficulty.
- **Wall muffle** (`wall_mute`): actor sounds (not bullet impacts) heard through solid static
  geometry are scaled by `1 - wall_mute`; one static ray per listener, cached 250 ms; passable
  materials (bushes) do not count.
- **Bolts** (`bolt_range`): the first hard contact of a bolt thrown by the actor
  (`CMissile::ExitContactCallback`, physics thread, queued; handled in `CLevel::OnFrame`) is a faint noise
  (stalkers: heard kind 4; monsters: investigate) within `bolt_range` x the
  `[nlc_step_surface]` factor; a direct hit on a stalker calls `sivol_stealth_bolt.on_hit`.

## nlc-3.589.10 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `AF014A0519F11A3313951C11BE4F07F00245465ACD3271A4ACDE4DBA96809EA3`
- **Status:** runtime pending (stealth P4 build, `docs/STEALTH_DESIGN.md` 18.1 and 19).

### NLC

- **Monster investigate retarget** (`monster_state_hear_int_sound`): every accepted investigate
  impulse bumps a serial and the hear-interesting-sound state restarts toward the new point
  (`check_force_state`); a corpse check does not override a more urgent impulse (near miss,
  faint shot, sight) still in progress. Before, a monster walking to a corpse ignored later near
  misses.

## nlc-3.589.9 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `5A326042C868CFD609666D557ED97A7EE2A975511BC2C81D06E31B50F26CE19E`
- **Status:** runtime pending (stealth M4, `docs/STEALTH_DESIGN.md` section 18).

### NLC: stealth M4 (`xrGame/nlc_stealth.cpp/.h`)

New globals are identity by default; the script values switch them on.

- **Concern escalation** (`concern_escalate`, `concern_window_ms`; species `concern_bold`,
  `concern_enemy_range`): repeated near misses or faint shots make a monster hurry on the second
  event, then attack within range (bold) or flee (timid, an ownerless dangerous sound).
- **No relay chains** (`firsthand_ms`): pack mates pass the actor on only if they sensed him
  themselves recently (`CMonsterEnemyMemory::add_enemy` marks own-sense acquisitions).
- **Two-tier footsteps** (`step_alert_pow`; `sound_memory_manager.cpp`): actor footsteps weaker than
  the threshold are a heard suspicion (kind 3) for stalkers, not an enemy sound.
- **Impacts near stalkers:** an actor bullet hit sound within `near_miss_range` of a stalker counts
  like a near miss (any relation; scripts give non-hostiles a short worried reaction).
- **Footstep noise:** outfit key `stealth_noise_k` (`outfit_noise`), ground material table
  `[nlc_step_surface]` (`surface_noise`), rain masking of quiet sounds (`rain_mask`); applied to the
  actor's sounds for stalkers and monsters.
- **Dynamic lamps** (`lamp_k`, `lamp_period_ms`; `CHangingLamp::nlc_light`, lamp registry): lamps and
  spot lamps light the actor for AI (range falloff, spot cone, static-geometry ray), computed once per
  period for all observers; `vis` lines show `+lamp`.
- **Lua:** `nlc_stealth_actor_surface()` (ground material, its factor, outfit noise factor).

### Required game data

- Scripts: `sivol_stealth_suspicion.script` (worried reaction, corpse investigation, torch hook),
  `sivol_stealth_cfg.script`, `sivol_stealth_test.script`; hooks in `xr/xr_danger.script` (corpses),
  `xrs/xrs_battle_ai.script` (knife and any suppressed weapon count as a silent kill),
  `sr/sr_light.script` (torch on while investigating).
- Config: `creatures/m_stalker.ltx` (`[nlc_step_surface]`), species keys in `m_dog`, `m_pseudodog`,
  `m_boar`, `m_bloodsucker`, `m_snork`, `m_chimera`, `m_flesh`, `m_tushkano`, `m_rat`;
  `misc/all_outfits_nlc.ltx` (`stealth_noise_k`).

## nlc-3.589.8 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1668F9477BB2EFF7B5BE8F62C33252825C787D523F3F08B7969EBFB23051F69C`
- **Status:** runtime pending (stealth M3, `docs/STEALTH_DESIGN.md` section 17).

### NLC: shots, near misses, corpses (`xrGame/nlc_stealth.cpp/.h`)

New globals are identity by default; the script values switch them on.

- **Two-tier shot hearing:** a heard actor shot weaker than `shot_alert_pow` (stalkers, weighted
  power; `sound_memory_manager.cpp`) or `monster_shot_alert_pow` / species `shot_alert_pow`
  (monsters; `base_monster_feel.cpp`) is concern only: stalkers get a heard suspicion value at a
  blurred point (suspicion stages run, no `attack_sound` danger), monsters investigate a blurred
  point. Louder shots work as before.
- **Near misses** (`Level_bullet_manager.cpp` `CalcBullet`): an actor bullet segment passing within
  `near_miss_range` of a creature gives concern toward a shooter guessed back along the bullet
  (distance error): monsters investigate, stalkers get a heard suspicion (kind "near miss") that the
  script turns into ducking or taking cover by temperament. Per-creature cooldown 1.5 s.
- **Corpse checks** (`CBaseMonster::Die`, `corpse_check`): the nearest calm squad mates within the
  species `corpse_check_radius` (up to `corpse_check_count`) investigate the corpse, the rest watch it.
- **Investigate styles** (`monster_state_hear_int_sound_inline.h`): monsters sent to investigate by
  the stealth layer walk, sneak (`ACT_STEAL`), hold and watch, or run, by a stable roll with the
  species `investigate_style_weights`; `mnotice` lines show `why` and `style`.
- **Lua:** `nlc_stealth_suspicion` returns the heard kind (0 sight, 1 faint shot, 2 near miss).
- **Diagnostics:** `macq src=hit_neutral` for the neutral-attacker hit path.

### Required game data

- `config/ogg_comments_overrides.ltx`: suppressed `ai_dist` raised to the outer radii (PB 8,
  pistols 13, 9x39 13, 5.45/5.56 20, 7.62x39 25, full power 28, 12 gauge 32).
- `config/creatures/m_dog.ltx`, `m_pseudodog.ltx`, `m_boar.ltx`, `m_flesh.ltx`, `m_snork.ltx`,
  `m_bloodsucker.ltx`: corpse check and investigate style keys.
- `gamedata/scripts/sivol/sivol_stealth_cfg.script` (new values),
  `sivol_stealth_suspicion.script` (shot-at reaction by temperament and rank).

## nlc-3.589.7 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4C48BA7921655C02A6EA1760D937913BFD33CA290BD40D3FCA449E091C7D0B83`
- **Status:** runtime pending (stealth M2, mutant senses, `docs/STEALTH_DESIGN.md` section 16).

### NLC: mutant senses (`xrGame/nlc_stealth.cpp/.h`)

New globals are identity by default; species keys in `config/creatures/m_*.ltx` are absent
(vanilla) unless set.

- **Light model:** species `nlc_light_k` (share of the AI sky and near-range light terms for
  monster observers) and `nlc_dark_floor` (night vision); `visual_memory_manager.cpp`.
- **Partial cover and decay for monsters:** `monster_ray_resample`, `monster_mem_hold_ms`,
  `monster_mem_decay_s`.
- **Monster suspicion:** past `monster_notice_v` of its detection threshold a monster gets an
  "interesting" actor sound at the actor's position, so its own investigate states run
  (`monster_notice_ms` between impulses).
- **Aura sense** (generalised from `CPoltergeist::update_detection`; `CBaseMonster::shedule_Update`):
  species `nlc_sense_*` keys (range, near/far factors, actor-speed exponent and minimum, rate,
  loss, notice and success levels, psy scaling by the actor's telepathic immunity, rain, through
  walls); notice sends the monster to investigate, success makes the actor its enemy (sources
  `psy` / `smell`). Global `sense_mult`.
- **Pack sharing gate** (`CBaseMonster::feel_vision_isRelevant`): with `pack_gate`, a monster copies
  a pack mate's enemy only within the species `pack_share_range` and after a random delay
  (`pack_share_delay_min/max`); beyond the range it only investigates. Monsters already hunting
  the enemy refresh freely. The visible-body helper (`actor_legs`) never gives or receives (always).
- **Impact and near-miss gates:** species `feel_enemy_who_made_impact_max_distance`
  (`CMonsterEnemyMemory::update`, bullet impact and whine sounds) and
  `near_hit_shooter_max_distance` (`CBaseMonster::feel_sound_new`, the 2 m near-miss hit).
- **Rain:** species `nlc_rain_k` scales monster detection.
- **Lua:** `nlc_stealth_env_light()` (environment light: total, ambient, sky, sun) for the NPC
  night-vision darkness switch.
- **Diagnostics:** `mnotice` and `msense` lines; `macq` sources `psy`, `smell`.

### Required game data

- `config/creatures/m_dog.ltx`, `m_pseudodog.ltx`, `m_snork.ltx`, `m_bloodsucker.ltx`,
  `m_controller.ltx`, `m_burer.ltx`, `m_poltergeist.ltx`, `m_boar.ltx`, `m_flesh.ltx`: species
  keys and calm vision sections.
- `gamedata/scripts/sivol/sivol_stealth_cfg.script` (monster globals),
  `sivol_stealth_nvd.script` (darkness mode), `sivol_stealth_test.script` (monster `hold()`,
  environment light in `ctx()`).

## nlc-3.589.6 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `60BE35C44604D1C2A8C2EC1F95FAA224089774B28248B87FF83B219AC9A9163C`
- **Status:** runtime pending (stealth pass 3, `docs/STEALTH_DESIGN.md` section 15).

### NLC: stealth and perception (`xrGame/nlc_stealth.cpp/.h`)

New parameters are identity by default, so the exe alone behaves like nlc-3.589.5.

- **Partial cover** (`ray_resample`; `xr_3da/Feel_Vision.cpp` virtual
  `feel_vision_resample`, `CCustomMonster` override): stalkers re-trace the actor every
  update with a new sample point on the body, so detection scales with the visible part
  of the body. Before, one clear point was kept while nobody moved.
- **Suspicion exports** (`CVisualMemoryManager`): the actor's position at the last
  evaluation that added to the sum, a per-NPC rate factor; Lua `nlc_stealth_suspicion`,
  `nlc_stealth_set_rate_k`, `nlc_stealth_force_suspicion` (harness).
- **Night-vision devices:** per-NPC flag (`nlc_stealth_set_nvd`) and light floor
  `nvd_floor`; eye glow particle placed in front of the eyes every frame
  (`nlc_stealth_eye_glow`; `CAI_Stalker::UpdateCL`, stopped on death and net_Destroy).
- **Outfit and rain** (`outfit_vis`, `rain_k`): the actor outfit's hidden
  `stealth_visibility_k` and the rain density scale the detection rate for stalker
  observers.
- **NPC targets** (`npc_light_k`, `npc_sky_vis`, `npc_sun_k`): optional light model for
  stalkers seeing stalkers (environment light cached per frame, target torch flag cached
  for 0.5 s, muzzle flash after unsuppressed NPC shots, near term, night-vision floor),
  blended from the vanilla "always lit"; 0 keeps vanilla.
- **Diagnostics:** shot sounds are labelled with the weapon that fired (the actor's
  `wpn_pkp` logged `wpn=-` before); `vis` lines add `out`, `rain`, `nvd`, `ek`.

### Required game data

- `gamedata/scripts/sivol/sivol_stealth_suspicion.script` (new scheme; hooks in
  `modules.script`, `xr/xr_logic.script`, `xr/xr_motivator.script`),
  `sivol_stealth_nvd.script` (new; hook in `sr/sr_light.script`), both registered in
  `ogse/ogse_signals_addons_list.script`; `sivol_stealth_cfg.script` values.
- `config/misc/all_outfits_nlc.ltx`: `stealth_visibility_k` keys.
- `particles/stealth_nvg/nvg_dot.pe` (from the Anomaly addon "Stealth" 2.31, with
  permission).

## nlc-3.589.5 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A241D7FC2E7EEBA27A51440D1E6587953FA164A78F383FA764DF30EE84ED33A9`
- **Status:** runtime pending (stealth batch 2, `docs/STEALTH_DESIGN.md` section 14).

### NLC: stealth and perception (`xrGame/nlc_stealth.cpp/.h`)

New parameters are identity by default, so the exe alone behaves like nlc-3.589.4.

- **Suspicion decay** (`mem_hold_ms`, `mem_decay_s`; `visual_memory_manager.cpp`
  `update`): a stalker that stops evaluating the actor (out of its view cone, ray
  blocked) keeps its partial detection sum for `mem_hold_ms`, then drains it over
  `mem_decay_s` from the full threshold, instead of resetting it at once. Monsters and
  other targets keep the instant reset. `nolos` lines show `sum` and `left`.
- **Heard-shot hit record gate** (`fakehit_min_pow`; `sound_memory_manager.cpp`): a
  heard actor shot adds the actor to the listener's hit memory (an amount-0 record that
  turns into an enemy candidate once the squad is in combat) only at this weighted power
  or more. `snd` lines show `gated=1`.

### Required game data

- `config/creatures/m_stalker.ltx`: new `[stalker_vision_tower_sniper]`.
- `gamedata/scripts/sivol/sivol_stealth_cfg.script`: the new parameters and
  `vision_profiles` (applies the tower section to the Agroprom tower snipers on spawn).

## nlc-3.589.4 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7C6BA53A0470FAB27DC01BF96CE15B113010BFEC3151178EA41CC3C21017589A`
- **Status:** runtime tested in the test install (2026-10-02, stealth checks 1 to 5 of
  `docs/STEALTH_DESIGN.md` 13.1); design and measurements in `docs/STEALTH_DESIGN.md`.

### NLC: stealth and perception (`xrGame/nlc_stealth.cpp/.h`)

All terms are parameters set from Lua (`sivol_stealth_cfg.script`); every engine default
is identity, so the exe alone behaves like nlc-3.589.3. The shipped script values are
listed under game data.

- **Vision terms for the actor as target** (`visual_memory_manager.cpp`):
  - AI sky light (`sky_k`, `sky_pow`, `sky_scale`): env hemisphere brightness x sky
    visibility added to the actor's luminosity for stalker observers. The renderer scales
    its sky term by `ps_r2_dhemi_sky_scale` (0.08), so before this, shade at noon read like
    night and only direct sun or static lamps mattered.
  - Near-range contrast (`near_k`, `near_range`), observer flashlight cone (`torch_k`,
    `torch_cone`; NPC torches are found in the torch slot or among attached items),
    muzzle flash after an unsuppressed actor shot (`flash_k`, `flash_ms`; hook in
    `CWeaponMagazined::OnShot`).
  - Rate factors: observer rank (`rank_k_novice/experienced/veteran/master`, thresholds
    from `[game_relations] rating`) and actor stance (`crouch_k`, `creep_k`); never inside
    `always_visible_distance`.
  - Actor speed from physics movement (`vel_physics`) instead of the position history,
    which alternated between real and doubled values.
  - Live overrides of the existing formula: `free_rate_mult`, `danger_rate_mult`,
    `monster_rate_mult`, `lum_factor_override`, `transparency_factor_override`.
- **Per-NPC vision sections:** `nlc_stealth_set_vision(id, free, danger)` reloads an
  observer's `CVisionParameters` (`CVisualMemoryManager::nlc_set_vision_sections`); lasts
  until the NPC reloads its sections.
- **Diagnostics** (off by default): per-observer `~ [stealth]` lines for vision factors
  (`vis`, `nolos`, `seen`), actor sounds heard by stalkers (`snd`, with the actor's weapon
  and suppressor state) and monsters (`msnd`), stalker enemy selection context (`enemy`),
  monster enemy acquisition source (`macq`); once per episode for `enemy` / `macq`; the
  visible-body helper (`actor_legs`) is skipped. Hooks in `sound_memory_manager.cpp`,
  `base_monster_feel.cpp`, `enemy_manager.cpp`, `monster_enemy_memory.cpp`,
  `script_game_object2/3.cpp` (script-induced awareness timestamps).
- **Lua globals:** `nlc_stealth_set/get/dump/watch/unwatch/unwatch_all/reset/last_seen/
  set_vision` (registered in `COMMON_AI/script_engine_export.cpp`). Debug parameter
  `force_profile` forces free or danger vision on watched observers.

### NLC: sound loader (`xrSound/SoundRender_Source_loader.cpp`)

- A `[path.with.dots]` section in the system ini now overrides only the keys it sets on
  top of the `.ogg` comment. Before, any such section skipped the comment, so unset keys
  (range, volume, sound type) fell back to engine defaults. The false "Missing
  ogg-comment" warning for overridden sounds is gone.

### Required game data

- `gamedata/scripts/sivol/sivol_stealth_cfg.script` (tuning values, registered in
  `ogse_signals_addons_list.script`), `sivol_stealth_test.script` (measurement harness).
- `config/creatures/m_stalker.ltx`: stalker vision sections (`always_visible_distance`
  2 when calm, `luminocity_factor` 0.8, danger `time_quant` 0.00286, `transparency_factor`
  1) and the new `[actor_step_manager]` with landing rows; `config/creatures/actor.ltx`
  points `step_params` at it.
- `config/ogg_comments_overrides.ltx`: `ai_dist` for every suppressed shot sound.

## nlc-3.589.3 (2026-10-01)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `E145FBA5C3671C8C4E8FCE113547E63B951C43D78B288ABA4D42453DA5877F5A`

### NLC

- Monster target selection (`monster_enemy_memory.cpp`, `monster_hit_memory.cpp`):
  `[monster_target_selection]` in `game_relations.ltx` sets `actor_bias`,
  `target_stickiness`, `hit_bonus` and `hit_bonus_time`, with optional per-monster
  overrides. Missing keys keep the original danger formula. `debug_log = true`
  logs every target switch.
- Duplet (`WeaponShotgun.cpp`): `duplet_on_alt_aim = true` fires both barrels from
  the alt-aim key on weapons without alt-aim zoom. Both shots raise the actor
  weapon-fire callback, so script recoil sees two cartridges. Optional
  `anm_shots_both` / `anm_shots_both_aim` HUD motions.
- Wound model (`EntityCondition.cpp`, `Wound.cpp`): `wound_model = 2` in a
  creature's condition section sums wound sizes and heals wounds in proportion to
  their size, so total bleeding is a fixed share of the damage dealt. Default 1
  keeps the original behaviour (actor and stalkers unchanged).
- Attack-on-move monsters (`base_monster`, attack states): `aom_close_melee_dist`
  switches to the standing melee state when the enemy is that close, until it
  moves past `MaxAttackDist`. Default 0 (off).
- Version text `nlc-3.589.3, OGSR main 2021123`.

### Required game data (same repository)

- `game_relations.ltx`: `[monster_target_selection]`; actor relation `-2` -> `-1`
  for flesh, dog, cat, chimera, giant, zombie, snork, fracture and zombi.
- Weapon configs: `duplet_on_alt_aim` and duplet HUD motions on the TOZ-66, the
  sawn-off TOZ-66, the TOZ-34 and the Ruzhye Orekha.
- Creature configs: `wound_model = 2` on dogs, cats, pseudodogs, boars and flesh;
  `aom_close_melee_dist = 1.6` on dogs.

## nlc-3.589.2 (2026-09-30)

- Server object destruction (`xrServer_process_event_destroy.cpp`): destroy
  events are flushed into a new event pack before the pack would exceed
  `NET_PacketSizeLimit` (fixed lost items when many objects were destroyed at
  once, e.g. Sidorovich's stock).
- **exe SHA-256:** `01026234925690EF58BE213DE2DE29558455F24F0C182FB99A8E74C5F11C1C0F`

## nlc-3.589.1 (2026-09-30)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), newer than release
  "3.589" (tag `3.548`). Previous NLC build: "3.526" (OGSR tag `3.525` + NLC,
  codev build of 2026-09-28).
- **exe SHA-256:** `5A3F8D71BA5DA732BA597C7C6697A0607F27D9E907594AA814BFCAAAE07C0CF3`

### From upstream OGSR

- DLSS quality presets (`r_aa_dlss_quality`: DLAA, Quality, Balanced,
  Performance, Ultra Performance) and DLSS model presets (`r_aa_dlss_preset`).
- FSR3 replaces FSR2 (`r_aa_mode st_opt_fsr3`, `r_aa_fsr3_quality`); FSR3 is
  linked into the exe, the `ffx_fsr2_*.dll` files are no longer used. DLSS SDK
  v310.7.0 (`nvngx_dlss.dll`).
- `g_weapon_bobbing`, `r_lens_flare_mode`, simulated fullscreen (`rs_fullscreen`),
  `input_exclusive_mode` setting, sun-shadow and water fixes, weather fixes,
  ImGui update, and more (103 upstream commits after `3.525`).

### NLC

- All NLC engine changes of the previous build carried over: `fl_hook` features
  (console commands, Lua API, knife combo), `.sq` archive support, gameplay, UI
  and script changes, `script_random`.
- Settings file stays `appdata/user.ltx` (upstream renamed it to `user_ogsr.ltx`).
- Active task is not switched by priority when `dont_switch_active_task_by_prio`
  is set in `external.ltx` (NLC sets it to `true`).
- Scripted aim sway: `CEffectorZoomInertion` honours `switch_zoom_osc(true)`
  instead of replacing the script's target with a random point (fixed a fast
  ADS camera shake).
- Weapon bobbing: `g_weapon_bobbing_ads_only` (bob only while aiming, regular
  walk animations otherwise) and `g_weapon_bobbing_mode` (off/always/aim) for
  the menu; fade-in restarts after a pause; walk animation is re-selected when
  leaving ADS while moving. Default: while aiming.
- Fresh-start defaults (no settings file): crosshair, dynamic crosshair and NPC
  info off; hard crosshair, status icons, 3D scopes and the dynamic item
  description window on.
- Version text `nlc-3.589.1, OGSR main 2021123` in the log and main menu.

### Required game data (same repository)

- Upstream 3.589 shaders as loose overrides in `gamedata/shaders/r3` (the engine
  crashes at startup without `temporal_resolve`), merged
  `models_selflight_det_3*.s`.
- Menu rows and strings for DLSS/FSR3 quality, DLSS model, sun flare style,
  fullscreen, weapon bobbing mode; `external.ltx`
  `dont_switch_active_task_by_prio = true`.

### Build

- Dependencies re-pinned in `nlc_tools/fetch_deps.ps1` to upstream's
  2026-09-27 set (mimalloc v3.5.1, DLSS v310.7.0, FidelityFX-SDK for FSR3).
- `nlc_tools/build_release.ps1` archives every exe with its PDB in
  `nlc_tools/builds/`.
