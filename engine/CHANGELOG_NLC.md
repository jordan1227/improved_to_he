# NLC engine changelog

One entry per shipped build (tag `nlc-<ogsr>.<n>` in `jordan1227/improved_to_he`).
Newest first. The engine source lives in this repository under `engine/`, next to the
game data of the same build. The exe and its PDB are attached to the GitHub Release
of the tag in this repository.

## nlc-3.589.46 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D17BB1BDB6E088D51144D8465CE8B2F0E6E90533A190646297A3D306850B2639` (archived with its PDB in `nlc_tools/builds/20261008_184236_d17bb1bd`).
- **Status:** runtime pending. Design after the `.45` test (`xray_hawkl_08-10-26_18-25-29.log`, open ground).

### NLC

- **Perch:** a perch counts after the enemy stood 0.4 m or more above its ground point for `pounce_perch_hold_ms`, not
  jumping, and off the ai-map as the siege sees it (weak ones "perch" pounced on flat ground at a jumping player).
- **Pair:** `tac_pair_rank` (strong 2, normal 1): the highest rank baits, so a normal + weak pair forms too. The
  flanker moves cloaked on an arc (`tac_flank_arc_dist`) until `tac_flank_arc_angle` off the player's view, then
  strikes when the player watches the bait or after `tac_flank_wait_ms` (`tac_flank_arc_max_ms` cap). The bait makes
  mock charges (`tac_bait_mock`), roars when the flanker strikes and charges for real once the flanker is within
  `tac_bait_sync_dist` (3 s cap). Both charged at once from the front before (the flanker struck 0.2 s into its stalk).
- **Open ground:** `cloak_counts_hidden`: cloaked beyond the x-ray radius and outside the view cone counts as hidden
  (siege, stalk, feint, hit and run found no cover in a field); hit and run runs zigzag legs while watched
  (`tac_recover_zz`); a feint aimed at vanishes sideways (`tac_feint_lateral`).

## nlc-3.589.45 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `879DD54560C94C78B5BFBF0E7EDC6A0A20E6B93FA9B0B1B482575BB69745EAE8` (archived with its PDB in `nlc_tools/builds/20261008_182500_879dd545`).
- **Status:** runtime pending. After the `.44` test (`xray_hawkl_08-10-26_18-12-38.log`); the relaunch fixed the
  standing pounces (Runtime accepted).

### NLC

- **Bait:** starts only beyond `tac_bait_dist` (it entered at 2-3 m in melee and stopped for 0.2 s, again and again);
  in place it walks slowly at the player (`tac_bait_advance`) instead of standing until its time ran out.

## nlc-3.589.44 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `2B56763F9A336785A4990E8696766B02C2129BEC51E0B7E026FFB2D11A29DBF8` (archived with its PDB in `nlc_tools/builds/20261008_165525_2b56763f`).
- **Status:** runtime pending. After the `.43` test (`xray_hawkl_08-10-26_16-44-30.log`).

### NLC

- **Pounce relaunch:** "on the ground" is less than half the expected flight progress, checked from 120 ms through the
  first half of the jump; the jump measures from the relaunch point (a jump started at a run kept the run's momentum,
  passed the fixed 0.5 m test and still died, moved 0.5 m).
- **Config:** `jump_max_height` 4.0 (a perch seen from a dip is 3.2-3.4 m to the target centre: "jump not possible"),
  `pounce_perch_block_ms` 8 s (a blocked perch strike left it holding a visible fallback beside the rock).

## nlc-3.589.43 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `49741AC1102173306FC4E7CA3493EA3769F2245FC212144D3DD4D3ECCF9F2679` (archived with its PDB in `nlc_tools/builds/20261008_164043_49741ac1`).
- **Status:** runtime pending. After the `.42` test (`xray_hawkl_08-10-26_16-28-12.log`).

### NLC

- **Pounce:** "baby jumps" also at the stock factor (moved 0.2-0.6 m, flat and perch, at random): the character kept its
  ground contact over the launch step, took ground control back and the speed limit ate the jump velocity.
  `CControlJump::update_frame` launches a jump that is still on the ground 120 ms after its start again, at most twice
  (species with an NLC jump only).
- **Perch strike:** the spot is chosen out of 12 directions around the enemy, preferring its back and sides (away
  from its view) over a long way around.

## nlc-3.589.42 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D7E1D3CA141E85A1D18E6404D889F41EBF2158C27B4C05E7D6205D40C4FA3531` (archived with its PDB in `nlc_tools/builds/20261008_162742_d7e1d3ca`).
- **Status:** runtime pending. After the `.41` test (`xray_hawkl_08-10-26_16-17-02.log`).

### NLC

- **Flat pounce:** still died in place at `pounce_flat_factor` 1.9 although the takeoff passed 6 m/s upward (the target
  is the enemy's centre, ~1 m up); 3 of 4 failed, every flat pounce at the stock factor flew. Config: factor 0
  (= `jump_factor`); the engine jump cannot launch that flat and fast.
- **Pounce:** the sight gate also accepts the visual memory's "visible now" (it waited 2 m from a visible player:
  "visible right now" flickers between vision updates).

## nlc-3.589.41 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `890DD254CA9B59C6E1BEB65F695AA55A28D8F5BA96F3F6915646FB0554400DC3` (archived with its PDB in `nlc_tools/builds/20261008_161632_890dd254`).
- **Status:** runtime pending. After the `.40` test.

### NLC

- **Flat pounce:** with `pounce_flat_factor` 1.9 the takeoff was too flat: the character stayed in ground contact,
  regained ground control and the pounce died in place (the running-attack clip played standing, "moved 0.3 m").
  `CControlJump::calculate_jump_time` lowers a forced factor until the takeoff is at least `nlc_jump_min_vy()` upward
  (bloodsucker `pounce_min_vy` 6 m/s; world gravity 19.62).

## nlc-3.589.40 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `093B74820D81AB2C18FF77659C72220B0143533CD6E23E43ED2E6EF166F61743` (archived with its PDB in `nlc_tools/builds/20261008_160140_093b7482`).
- **Status:** runtime pending. After the `.39` test (`xray_hawkl_08-10-26_15-46-50.log`).

### NLC

- **Flat pounce:** `pounce_flat_factor` never applied (`CControlManagerCustom::jump` resets `force_factor`); it is set
  after the start and reset when the pounce ends.
- **Perch pounce:** the facing tolerance follows `jump_max_angle` (0.6 rad; it waited 24-28 deg off, the siege turns
  only beyond 30); the perch spot tries other sides around the enemy when the straight one is off the ai-map.
- **Tactics:** a re-entry of the tactic state no longer drops the running tactic (a recover ended after 1 s, idle).
- **Logs:** every bloodsucker pounce logs how it ended, after how long and how far it moved.

## nlc-3.589.39 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CFF14B93350028F169121244F092CC8D65C2C6F55F6FC790340847C534A698EA` (archived with its PDB in `nlc_tools/builds/20261008_154404_cff14b93`).
- **Status:** runtime pending. After the `.38` test (`xray_hawkl_08-10-26_15-11-26.log`).

### NLC

- **Perch strike:** the spot is placed around the enemy itself, at the xz distance that puts the 3D jump range mid band
  (it stood 7-9 m away, out of range, when the enemy's ai vertex lay off on the rock); out of range at the spot, it
  picks a new one.
- **Pounce:** separate ranges for a perch (`pounce_perch_dist`) and flat ground (`pounce_flat_dist`); flat ground is
  lower, faster and longer (`pounce_flat_factor`); the glide clip is configurable (`pounce_anim`, now the running
  attack); an own hit test, centre to centre (`pounce_hit_dist`), as the stock one missed perched players; one hit per pounce.
- **Cloak:** never fully invisible within `cloak_xray_radius` of the enemy (weak 3, normal 2.5, strong 2 m).
- **Tactics:** hit and run for all variants (35/45/55% health, `tac_recover_cooldown_rand`); weak bloodsuckers stalk
  (closer, shorter); normal ones feint (rarer); a pair forms when one member has `tac_pair_lead` (strong), who baits.

## nlc-3.589.38 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `69190B60364EC350DA37B4FEA85A8FC8E59D8C1B74F7CC9CCAA3B24896C37F93` (archived with its PDB in `nlc_tools/builds/20261008_133415_69190b60`).
- **Status:** runtime pending. Fixes after the `.37` tests (`xray_hawkl_08-10-26_13-10-00.log`, `13-20-54`).

### NLC

- **Pounce:** 3 of 5 pounces ended one frame after takeoff (a velocity bounce at the start); `CControlJump` ignores
  bounces for `nlc_jump_bounce_grace()` ms (bloodsucker `pounce_bounce_grace_ms` 150, other species 0 = stock). The
  upward-jump log names why and after how long a jump ended; a ready perch pounce that does not start logs why.
- **Perch strike:** shot at the perch-strike spot, the bloodsucker leaves it for `pounce_perch_block_ms` (it re-picked the
  same spot after every shot when no retreat point existed).
- **Bait and flank:** a bait no longer restarts while a flanker's strike is still fresh (every later bait ended after
  0.2 s).

## nlc-3.589.37 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `34CB9B7F2F65E5C42B9D2133BA458CE8ED05AF261854BD88C917BE2F6A1B3766` (archived with its PDB in `nlc_tools/builds/20261008_130858_34cb9b7f`).
- **Status:** runtime pending. Fixes after the `.36` test (`xray_hawkl_08-10-26_12-47-40.log`) and a code review.

### NLC

- **Back-hit grab:** it broke 220 ms after its start (the stock start distance, `MinAttackDist`, was also the break
  distance); the ambush grab now breaks only beyond `vampire_break_dist` (3.5 m) and logs why and when it ends. A grab
  that breaks early lets the held-back rest of the hit land; the rest keeps its world direction. The back hit is rolled
  only when a grab can start from there, and the actor is checked again (jumping, climbing, facing it) before the grab.
- **Cloak:** fully visible while its grab holds the actor (a strike's x-ray window could swap the model mid-grab).
- **Pounce:** range measured as `CControlJump::can_jump` does (3D, feet to the enemy's centre).
- **Zigzag:** the `zz_react_ms` reaction time restarts after the enemy left sight or range.
- **Stalk:** the flank preference restarts with each stalk and each new enemy; `pounce_perch_block_ms` at least 2 s.

## nlc-3.589.36 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `86154D49B50015A6CC36D172A65C60CECCDFFDE846F6D69E00933A75CF9D07CB` (archived with its PDB in `nlc_tools/builds/20261008_124644_86154d49`).
- **Status:** runtime pending. Round 6 after the `.35` test (`xray_hawkl_08-10-26_12-09-38.log`), design doc 21.

### NLC

- **Perch strike:** no lunge during a siege or at a perched enemy; the perch-strike spot stays cloaked until the pounce
  is ready; a refused perch roll or no pounce within `pounce_perch_wait_ms` gives the spot up for
  `pounce_perch_block_ms` (it hides instead of standing there).
- **Pounce:** all variants at a perched enemy (`pounce_perch_chance`, `pounce_perch_max_h`); flat ground only with
  `pounce_flat` (strong). The rolls happen per cycle before the reveal; a failed jump is logged.
- **Cloak:** a strike reveal without a strike ends after `strike_reveal_max_ms`, then no strike reveal for
  `strike_reveal_block_ms`.
- **Back-hit grab:** a hit from behind may turn into the grab (`vampire_backhit_chance`); `vampire_backhit_damage_k` of
  it lands at once, the rest only when no grab starts within `vampire_backhit_window_ms`.
- **Flanks:** stalk points on the player's flank first (`tac_stalk_flank`); an unwatched charge curves toward the flank
  or back (`charge_flank`, bloodsucker `nlc_run_target_override`).
- **Zigzag trigger:** `zz_need_gun 3` (aiming down sights at it or the crosshair on it) and `zz_react_ms`.

## nlc-3.589.35 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `56DA4D67D582E71170C8A2D7E11B1C24FB4B4D197AFCA0AA8BFF38F83F1EE649` (archived with its PDB in `nlc_tools/builds/20261008_014521_56da4d67`).
- **Status:** runtime pending. After the `.34` test (`xray_hawkl_08-10-26_01-17-40.log`).

### NLC

- **Bloodsucker cloak:** the strike reveal applies only while charging (attack run, facing within 45 degrees, enemy
  seen) or at a siege perch-strike spot (margin now 2 m); every other siege hold or move stays cloaked; a feint ends with
  a cloak window (`tac_feint_cloak_ms`).
- **Perch strike:** a strong bloodsucker besieging a player on a perch it can jump to goes to pounce distance and
  pounces (`nlc_siege_perch_strike`, `CBaseMonster` hook).
- **Feint:** shows itself while charging (no stop); with no hidden point it charges on cloaked.
- **Stalk:** points on its own side of the player (the route across ended the stalk at once).
- **Hit and run:** `tac_recover_break_health` breaks away even at point blank; with no hidden point it takes a point
  out of the enemy's view, else runs straight away.
- **Turning back after a pass:** the rotation jump also starts above the normal run speed (a cloaked bloodsucker
  never turned quickly); `Run_Attack_Overshoot` ends the lunge line shortly past the enemy. The rotation jump
  releases only the controls it owns.
- **Cats:** the stock attack jump was never wired (`nlc_jump_attack`), and the `run_turn_180_r` running turnaround
  (`nlc_turn180`); both on in `m_cat.ltx`.
- **Logs:** every lunge (`lunge at`).
- **Config:** bloodsucker `jump_factor 0.9` (strong), `tac_feint_show_ms 700`, break health 0.2 / 0.3; cats
  `zz_max_angle 14`; dogs `zz_min_leg_k 2`.

## nlc-3.589.34 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `3B287996E5A2FF6C811E2BFD6D91EC51EF270A7B9A568C5A9D530FCEBF4DBAB1` (archived with its PDB in `nlc_tools/builds/20261008_011344_3b287996`).
- **Status:** runtime pending. After the `.33` test (`xray_hawkl_08-10-26_00-49-32.log`).

### NLC

- **Bloodsucker strike tell:** `strike_reveal_margin` (3 m): fully visible within the start distance of a ready lunge or
  pounce plus the margin, else within `cloak_cooldown_radius` (5 m); a strike needs `strike_reveal_ms` (300) fully
  visible first. The pounce only opens the after-strike window (no model swap under a running jump).
  `Run_Attack_Cooldown` (stock 2200 ms) and `pounce_chance` (strong 0.5: else the lunge) are config keys.
- **Perch:** a besieging bloodsucker may pounce at a perched enemy it can reach (`jump_max_height`); bloodsuckers hold
  the low-perch bite window 0.8 s (`elev_reach_timeout`); a side-guard species tries the retreat ring before holding a
  visible spot, and holds one standing and facing the enemy. The attack-run state faces the enemy when it stands in
  place (all species; it kept its arrival heading, side-on).
- **Zigzag turning:** the heading speed follows the dodge factor (the turn radius the path was planned with holds);
  leg angle capped by the run turn rate (`zz_turn_share`), legs shorter than `zz_min_leg_k` x the turn radius run
  straight (no walk-speed curves), no new leg while heading more than `zz_heading_gate` off the enemy (cats circled).
- **Config:** cats `zz_max_angle 20`, legs 700-1000 ms, `zz_seen_grace 1500`; bloodsucker `lunge_reveal 2`,
  `vampire_struggle_need 2500`, `Melee_Rotation_Factor 3.0`.

## nlc-3.589.33 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8D411AAFC272EBF79D586492A2FFE77DA7FFAE00118CDECCF0088D21D218BB29` (archived with its PDB in `nlc_tools/builds/20261008_004826_8d411aaf`).
- **Status:** runtime pending. Fixes after the `.32` test (`xray_hawkl_08-10-26_00-39-43.log`).

### NLC

- **Crash fix (control capture steal):** a lunge started during a pounce (NLC had dropped the run attack's stock
  "nothing else captured" check for the boar ram); `capture_pure` took the jump's controls, the jump ended and
  `CControlJump::on_release` read the direction data it no longer owned. The run attack now refuses while a jump runs or
  a non-base control holds the animation (jump, vampire grab, stagger); the jump and the run attack release and unlock
  only the elements they still own (`CControl_Manager::release_pure_owned`, `unlock_owned`).
- **Grab from behind:** never starts while a lunge or jump plays or in the x-ray window after a strike (a grab started
  0.35 s into a lunge was carried past the player and ended at once: a camera twitch, no hold).

## nlc-3.589.32 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `782D95F57F65A2A7C7ECE0E21DE4A0BC598F097C38DEABEA11B28D33AB7C705B` (archived with its PDB in `nlc_tools/builds/20261008_003651_782d95f5`).
- **Status:** runtime pending. Fixes after the first `.31` test (`xray_hawkl_08-10-26_00-13-52.log`).

### NLC

- **Crash fix (bloodsucker pounce):** `CControlJump::on_event` ended in a null dereference on the first glide frame;
  the pounce decloaked to full visibility in the same tick, which swaps the model and restarts its animations. The
  jump now ends (logged) instead of crashing when its animation capture, direction capture or blend is missing; the
  pounce reveals only to x-ray (same predator visual); the bloodsucker's visibility is frozen while a jump or lunge
  plays; the lunge reveals right before the control activates, not during it.
- **Strike reveal:** `lunge_reveal` (1 x-ray, 2 full) and `lunge_reveal_ms`: after a lunge or pounce the bloodsucker
  stays in that state until the first melee swing (which decloaks at once).
- **Grab from behind reachable:** `vampire_intent_dist`: when a grab from behind is planned (cloaked, unseen, behind
  the player, chance rolled), the lunge and pounce stand down and the cloak holds until grab range. The grab start
  decloaks at once. The pounce also never fires in a siege.
- **Stalking:** the enemy is pinned in memory during stalk, hit-and-run and feint (memory 40 s vs. stalks up to 60 s);
  hiding and stalk points are picked around the last known position, not the true one.
- **Tuning keys:** `tac_recover_min_time`, `tac_feint_vis`, `tac_bait_vis` (1 x-ray, 2 full).
- **Zigzag:** `zz_seen_grace` (the monster's own sight flickers between vision updates and stopped legs), per-variant
  frequency `zz_chance`, `zz_burst`, `zz_rest`; dog, pseudodog and cat variants tuned (weak dogs least, pseudodogs
  and cats most).
- **Game data:** `game_relations.ltx` (new zigzag keys; `evade_debug_log` / `elev_debug_log` true for the test
  round), `m_bloodsucker.ltx`, `m_dog.ltx`, `m_pseudodog.ltx`, `m_cat.ltx`.

## nlc-3.589.31 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `764E22AE64F1EA523025F0D1F277D4FC1B19DEE90A0B7A557428FD55B9C56E3E` (engine/bin_x64; archived with its PDB in `nlc_tools/builds/20261007_235838_764e22ae`).
- **Status:** runtime pending. Design: `docs/DESIGN_monster_movement_under_fire.md`; handoff:
  `docs/HANDOFF_monster_movement_under_fire.md`. Rides on top of the uncommitted siege pass (`nlc-3.589.30`).

### NLC

- **Evasion pass (`base_monster_evade.cpp`, `[monster_evasion]` in `game_relations.ltx`):** every feature has its own
  key and defaults to off; `evade_enabled = false` (shared or per section) turns all of it off for a species;
  `evade_debug_log` writes `~ [evade]` lines.
  - **Threat test:** `nlc_threat()` (cached 200 ms): watched (view cone `evade_watch_cone` + one static ray), armed
    (a gun, no knife or binoculars), aimed (`evade_aim_angle`), locked (crosshair ray, actor only).
  - **Zigzag approach:** `zz_*` keys. Charging species leave the straight line in legs of `acos(1/zz_speed_k)` degrees
    (capped by `zz_max_angle`) with a dodge speed factor (`nlc_set_dodge`, separate from the giant's haste), from
    `zz_dist.y` to `zz_dist.x`, when watched (`zz_need_gun`: 0 watched, 1 armed, 2 aimed); shot sounds, near misses
    and hits switch the side. Hooks in `CStateMonsterAttackRun::execute` and the far branch of the attack-on-move state.
    On in: dog, pseudodog, cat, bloodsucker.
  - **Side guard (`side_guard`, bloodsucker):** a watched guarded monster ignores the squad encircle orientation; siege
    goto / retreat / flee picks skip points that would cross the line of sight (`side_guard_angle`,
    `side_guard_short`); with none left, a radial escape straight away from the enemy; lateral-run diagnostics.
  - **Run attack:** the 2.2 s cooldown is armed when the lunge really starts (it was armed before the species gate could
    refuse it); `run_attack_haste_k` speeds up the lunge. **Found:** `CAI_Bloodsucker` and `CAI_Boar` add the run attack
    after `control().load()`, so `Run_Attack_Dist` / `Run_Attack_Delay` were never read (code floors: 2.5-5.5 m, no
    delay). New bloodsucker key `run_attack_cfg` loads them (off by default; the boar is unchanged).
  - **Siege placement:** `elev_pick_near_anchor` ranks ambush points by their distance to the enemy's ground point.
  - **Dog sneak:** `anim_steal` (dog, `stand_steal_`); upward-jump diagnostics in `CControlJump`.
- **Bloodsucker behaviour set (`bloodsucker_tactics.cpp`, `eStateAttack_NlcTactic`):** cloak hold at the siege ambush
  (`amb_cloak_hold`), full visibility only inside `cloak_lunge_radius` while the lunge is ready, lunge from cloak with
  decloak (`run_attack_decloak`), melee reveal, faint steps while cloaked (`CStepManager::step_volume_k`), edge sneak
  (`sneak_near_dist`), hit and run (`tac_recover_*`), stalk for openings (`tac_stalk_*`, `tac_open_*`), feint
  (`tac_feint_*`), bait and flank pair (`tac_pair_enabled`), night boldness (`tac_night_*`), vampire grab as a true
  ambush from behind with a chance per variant and a mouse-shake struggle (`vampire_*`, `CActorInputHandler::on_mouse_move`),
  experimental pounce (`amb_pounce`, strong). Variants in `m_bloodsucker.ltx`; the phantom is excluded.
- **Game data:** `game_relations.ltx` (`[monster_evasion]`), `m_bloodsucker.ltx`, `m_dog.ltx`, `m_pseudodog.ltx`,
  `m_cat.ltx`. Vampire numbers and the variant table are untuned starting values.

## nlc-3.589.30 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `9FCCF18989CAC0DD9038095D4E1FE06D8A5487CA330E74FB638AD1A4EC4DCFE4`
- **Status:** runtime pending. Design: `docs/DESIGN_monster_elevation_siege.md`.

### NLC

- **Siege of an elevated enemy:** an enemy out of reach because of height (roof, ledge, vehicle) is besieged
  instead of the stock home-point shuffle (`eStateAttack_Siege`, `base_monster_siege.cpp`): monsters wait at points
  the enemy cannot see near its ground point (ambush ring, then a looser watch ring with one peeking watcher per
  squad), move when exposed or near-missed, retreat out of sight when shot, rest per species. Config
  `[monster_elevation]` in `game_relations.ltx` (`elev_enabled` master switch, `elev_vs_npc`), per-section overrides.
- **Anti-bait:** an enemy coming down within `elev_reengage_close` is attacked at once; farther away the re-engage
  delay grows with each bait (3 / 6 / 10 s), and the break-off grace drops to 1 s after the first.
- **Hostility kept (lock):** noise or sight pins the enemy in monster memory (`CMonsterEnemyMemory::pin`) for the
  species patience, then the stock 20 s memory; faint (suppressed outer-tier) shots renew up to
  `elev_faint_patience`; losing the enemy this way starts the stealth "lost the trail" alert.
- **Exits:** a squad that loses `elev_flee_losses` (default `hunt_flee_losses`) members during a siege abandons it;
  a bolt far from the enemy pulls one distractible besieger (boar, flesh) away; a low perch (up to
  `elev_reach_height`) is attacked until no bite was tried for `elev_reach_timeout`.
- **Moved helpers:** `nlc_point_on_map` / `nlc_los_to` from `CController` to `CBaseMonster` (controller unchanged).
- **After the first test rounds:** hidden = three static rays from the enemy's eyes (head, both body ends) all
  blocked; an outer ring before a visible fallback (lies down there); a still-hidden spot is kept; the sneak gait
  only from out of sight; the low-perch attack never inside a siege, dropped when shot without biting, 15 s block;
  dogs stay in the attack state during a siege (`dog_state_manager`: the 6 m / mid-home / 8 s danger gate let them
  idle); at most 4 point searches per frame over all monsters.
  The lock survives an interrupted siege (hit reaction, sound states) and a restart keeps the patience running;
  a peeking watcher gets the vision factor `elev_peek_detect_k` (dogs 2, peek 4 s).
- **Opt-outs:** controller, poltergeist, snork, burer, chimera.
- **Game data:** `game_relations.ltx` (`[monster_elevation]`), species keys in `m_dog`, `m_pseudodog`,
  `m_bloodsucker`, `m_boar`, `m_flesh`, `m_giant`, `m_zombie`; `dsh_battle_radius.script` releases the gunfire
  pursuit 25 m out when the actor is elevated.

## nlc-3.589.29 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `9D6F0A194F42DC54A3D09A19AD43360BA7616F1D82B0423BCDA8244274E6A948`
- **Status:** runtime pending. Fixes from the multi-agent review of the controller work.

### NLC

- **Crash fix:** `CController::net_Relcase` clears a destroyed enemy from its thralls' task object (thralls hold
  the controller's enemy; an offline switch or release of that enemy left a dangling pointer).
- **Controller vs non-actor enemies:** the cover / peek loop only runs against the actor (the tube targets only
  the actor); stalkers and monsters get the stock fight instead of an endless peek without attacking.
- **Thrall eyes:** the strike no longer silences the hum of a real tube that started during the wind-up; a
  wind-up dies with the controller.
- **Staggers:** a pending thrall stagger that cannot start within 1.5 s is dropped (no late, random staggers);
  thralls restored after a load do not stagger.
- **Recruit channel:** stops only the sequencer it started. **Herd call:** reports 0 when thralls do not attack.
- **Stalker aim:** psy health 0 (never regenerates for NPCs) no longer means permanent x4 spread.
- **Input scramble:** a key scrambled onto a blocked action is blocked too, and a recorded fire / zoom release
  is never blocked; a consumed fire press never swallows a click meant for an open UI window.

## nlc-3.589.28 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8D8B793A437871A0DC1ACD8CAFDDD4D94D22D6B46828DF288516DB8F0E7A6229`
- **Status:** runtime pending.

### NLC

- **Controller input scramble without binds:** `level.nlc_scramble_input(group, ms)` (0 movement, 1 fire/zoom)
  remaps the game actions in `CLevel` input dispatch for a while (random derangement); a key keeps the action it
  was pressed as until release, so fire/zoom cannot stick. The binds in `user.ltx` are never touched, so a load,
  quit or crash cannot leave them scrambled. `level.nlc_clear_input_scramble()` ends it.
- **Thrall eyes as a quick tube:** the thrall's head flares with the tube particles and the tube hums for
  `nlc_ctrl_thrall_eyes_windup` ms, then the tube hit sounds and the psy jolt land (cancelled if the thrall loses
  sight of the actor).
- **Script access:** `nlc_controller_phase()` (ranged phase name) and `nlc_controller_herd_call(ms)` (every thrall
  attacks the controller's enemy, posts and flanks dropped) on `CCustomMonster`.

## nlc-3.589.27 (2026-10-06)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `43EB91F4A9394AC1510A5F7957977A9A12865381A8F1EA8DF14BFC2201074C61`
- **Status:** runtime pending.

### NLC

- **Controller thralls attack:** monsters taken under control now hunt the controller's enemy
  (`nlc_ctrl_thrall_attack`, `nlc_ctrl_thrall_memory`) instead of only following it and sitting down
  (stock code never called `set_controlled_task`).
- **Controller reposition:** when its enemy is inaccessible (roof, crate, vehicle, outside home, restrictor)
  the controller walks to cover (tube recharging or recently hit) or to a tube-range point
  (`nlc_ctrl_reposition`, `nlc_ctrl_cover_dist`, `nlc_ctrl_range_dist`, `nlc_ctrl_hold_time`,
  `nlc_ctrl_move_timeout`) instead of standing in the open.
- **Controller ignores its thralls:** `game_relations.ltx` keeps the species hostile, so the controller
  forgets a thrall as enemy and attacker on capture and `is_relation_enemy` is false for its thralls.
- **Controller cover loop:** hides in cover while the tube recharges, peeks to a point with line of sight when
  it is ready, stock melee when close, stock hunt when the enemy is lost (`nlc_ctrl_cover_loop`, `nlc_ctrl_close_dist`,
  `nlc_ctrl_lost_time`, `nlc_ctrl_peek_radius`, `nlc_ctrl_aim_time`); real run to cover with `nlc_ctrl_run_anim`
  when the visual has the clip.
- **Controller thralls:** species roles and weights (`nlc_ctrl_thrall_table`): hunt, flank, guard (new
  `eTaskMove` / `CStateMonsterControlledMove` for every controllable species); weight budget, leash, release
  stagger and slow; active recruitment channel (`nlc_ctrl_recruit_*`) that any hit breaks.
- **Thrall release:** repeated, slowed stagger (`nlc_ctrl_release_stagger = count, speed`; new
  `CBaseMonster::nlc_stagger_repeat`, critical-wound playback speed); thralls ignore their home while controlled
  (`CMonsterHome::nlc_suspend/nlc_resume`).
- **Controller tube:** while aiming from a peek point with a clear ray to the actor seen within 3 s, the tube
  does not wait for the vision build-up (`CController::nlc_psy_sense`).
- **Controller Mirage decoy:** on reaching cover a controller may spawn a decoy controller (`nlc_ctrl_decoy_section`,
  `_chance`, `_cooldown`) and stays hidden while it lives; a decoy (`nlc_ctrl_decoy`) has no tube, thralls or
  recruits and is destroyed on any hit or after `nlc_ctrl_decoy_life` (no corpse, loot or kill).
- **Controller guards:** posts resolved without misusing `accessible_nearest` (it requires an inaccessible
  point), closer fallbacks, one guard plus flank/hunt overflow, guard promotion, 3 s thrall status log.
- **Stalker psy aim:** `nlc_psy_dispersion_k` scales weapon dispersion with lost psy health (`CAI_Stalker`).
- **Controller flushed reaction** (`nlc_ctrl_flush_*`): an enemy close to its cover gets a psy jolt, then the
  controller repositions nearby. **Thrall eyes** (`nlc_ctrl_thrall_eyes*`): a weak psy jab through a thrall that
  sees the actor while the controller does not.
- **Script hooks on `CCustomMonster`:** `nlc_set_phantom` (invisible to AI, no grenades), `nlc_add_enemy`,
  `nlc_controller_thralls`, `nlc_controller_take` (thrall persistence across save/load).
- **Stalker smart-terrain evaluator:** no crash when a script released the server object this frame.
- **Script:** `level.consume_key_press()` skips native handling of the current key press (controller fire reaction).
- **Diagnostics:** `CBaseMonster::nlc_inaccessible_reason()`; `nlc_ctrl_debug` logs `~ [controller]` lines.

### Required game data

- `m_controller.ltx` `nlc_ctrl_*` keys, `[nlc_controller_thralls]`, `[m_controller_shepherd]` (all keys optional;
  missing keys keep stock behavior); `sivol_weapon.script` (consume the fire press), `rx_gl.script` (no launcher for phantoms), `sivol_controller_psy.script` (stalker domination and guards, Mirage swap and tells, thrall persistence),
  `m_stalker.ltx` `nlc_psy_dispersion_k`.

## nlc-3.589.26 (2026-10-06)

- Co-dev build: body-part health / skeleton system from A.R.E.A (`nlc_body_health`, `UIBodyHealthWnd`),
  console command history; see commit `7090f09b` and `19db1c3c`. Changelog entry not written by the co-dev.

## nlc-3.589.25 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `6DA80D5FFF05401EC30D2A30BAF4EB01B9C4E9DD61549C148C667D004C0B9DDC`
- **Status:** runtime pending.

### NLC

- **Pseudogiant charge rework:** the rush paths to a point locked `Charge_Overshoot` m past the target's spot
  (`CBaseMonster::nlc_run_target_override`, used by the attack run substate), so sidestepping escapes; reaching
  that point without contact counts as dodged. Wind-up plays `Charge_Windup_Anim` in place at
  `Charge_Windup_Anim_Speed` through the animation sequencer; the charge requires facing the target
  (`Charge_Face_Yaw`). The stumble is a forced damaged walk (limp clip and `Velocity_WalkFwdDamaged`) for
  `Charge_Stumble_Time` instead of a speed multiplier. `Charge_Max_Turn` is no longer used.
- **Animation sequencer:** optional playback speed (`seq_run(motion, speed_k)`) and early stop (`seq_stop`).
- **Forced damaged walk:** `CBaseMonster::nlc_force_damaged` on top of the `DamagedThreshold` health rule.

### Required game data

- `m_giant.ltx` `Charge_Windup_Anim*`, `Charge_Face_Yaw`, `Charge_Overshoot`, `Charge_Stumble_Time 2500`.

## nlc-3.589.24 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `B2DD779B6A2A2B8D843993A808D3EDEFC1B3D38DAD395B4E6DB23FDEF288A7F9`
- **Status:** runtime pending.

### NLC

- **Species speed:** `move_speed_k` in a monster section scales movement and walk clip speed together
  (pseudogiant 0.83); rage and charge stack on it.
- **Pseudogiant charge:** wind-up telegraph (`Charge_Windup`: attack sound, near stop while aiming), locked rush
  line (`Charge_Max_Turn`: a target that sidesteps out of it makes the rush abort with a stumble), ram hit on touch
  (`Charge_Contact_*`) instead of waiting for an attack animation.

### Required game data

- `m_giant.ltx` `move_speed_k`, `Charge_Contact_*`, `Charge_Windup`, `Charge_Max_Turn`.

## nlc-3.589.23 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A5B2E5121060BDBEA0E7D446920B7A7CCB9E53E86A84E7D346BB29A062D45378`
- **Status:** runtime pending.

### NLC

- **Melee phase 2:** `[monster_melee] trace_ignore_objects` (the actor line-of-hit trace is blocked only by
  level geometry, not pack mates or loose objects); `close_yaw_dist` / `close_yaw_half` (wider bite yaw window
  at body contact).
- **Monster speed multipliers:** persistent bonus and temporary haste next to the stomp slow; movement clips
  without an accel chain now scale their speed with the multiplier (no foot sliding).
- **Pseudogiant moves** (each with its own switch in `m_giant.ltx`): rage below a health fraction (faster,
  wider, more frequent stomps, entry stomp), recovery stomp on burst damage, charge (rush on a clear line,
  empowered first strike, stumble on a miss).

### Required game data

- `game_relations.ltx` `[monster_melee]` keys; `m_giant.ltx` `HugeKick_Rage_*`, `HugeKick_Recover_*`,
  `Charge_*`, `stand_attack_1` impulse 400.

## nlc-3.589.22 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `813BE01F917A2F0B2F6507A8E18071D98A5FFFA43064C8E086DB8F086407E056`
- **Status:** runtime pending.

### NLC

- **Surface-aware melee distance (opt-in):** `melee_surface_distance` / `melee_surface_offset` in a monster
  section make `CMeleeChecker::distance_to_enemy` and the bite range check in `check_hit` measure to the
  target's collision surface (+ offset) instead of centre to centre (the original ray test queried the
  attacker's own collision form). Large targets such as the pseudogiant now count as close.
- **Melee diagnostics:** `[monster_melee] debug_log` logs group-attack decisions (enemy within 4 m) and every
  bite attempt (distance, yaw/pitch window, hit/miss).
- **Rotation jump guard:** animation sets whose names are missing in the model are dropped with a log line
  (pseudogiant placeholders `"1".."4"`); no rotation jump starts without a valid set.

### Required game data

- `game_relations.ltx` `[monster_melee]`; `m_dog.ltx`, `m_pseudodog.ltx` (`melee_surface_*`).

## nlc-3.589.21 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C371FCC9D14F42DC8BF5F4918AD696E368B74E749BE27450A02C05A52ECD3023`
- **Status:** runtime accepted (quick tests): target switching, damage threat, back-stab aggro, crowd stomp,
  splash damage, stagger, mutant slow, ragdoll kills, jump dodge, grenade-deflection stomp. Includes the
  unreleased `nlc-3.589.20`.

### NLC

- **Monster target selection:** `[monster_target_selection]` in `game_relations.ltx` (per-section overrides):
  player bias, current-target stickiness, recent-hit bonus, damage-weighted threat (per-attacker health lost,
  fading over `damage_memory_time`), back-stab provocation (`back_hit_threat`), `debug_log` for target switches.
  Missing keys keep the original danger formula.
- **Pseudogiant stomp:** crowd trigger, splash on enemies (optionally neutrals) with falloff, height gate,
  configurable hit type/bone per victim kind, knockback, stagger (stalker leg-hit / mutant critical-hit anims),
  mutant movement slow, actor hit inside the splash, precise jump dodge (feet clearance at impact),
  grenade-deflection stomp at a faster animation speed. All `HugeKick_*` keys optional.
- **Stomp kills** skip the CoP death animation, so the body is thrown as a ragdoll.
- **Run attack guard:** monsters whose model lacks `stand_attack_run_0` (pseudogiant) no longer start the run
  attack (null blend crash in `CControlRunAttack::on_event`).

### Required game data

- `game_relations.ltx` `[monster_target_selection]`, `m_giant.ltx` (`HugeKick_*`, damage table, `health_hit_part`),
  `m_flesh.ltx` (`target_stickiness`).

## nlc-3.589.19 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F3B6C843EA4ED42781E9614C71920A18526335D7CBFDE8992A9AB6D0837AB60D`
- **Status:** runtime pending.

### NLC

- **wpn_knife_m1 walking return:** optional `knife_return_moving_speed` in `wpn_knife_m1_hud` scales the
  `anm_hitN2idle` return clip when it replaces a moving idle (default 1.0; tuning experiment).

### Required game data

- `w_knife.ltx` key `knife_return_moving_speed` (optional).

## nlc-3.589.18 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1DB407E47AD7ED1343CE05F9DE60FE3EB5F3BAEFA363A41FDA76CF70A634D361`
- **Status:** runtime pending.

### NLC

- **wpn_knife_m1 walking return:** the swing-to-idle return clip (`anm_hitN2idle`) is now also used when the
  next idle request is a moving idle (`anm_idle_moving*`), so the return no longer cuts straight into the walk cycle.

### Required game data

- None (the torch rework and `liz_knife_flash` ship as script/config/asset changes).

## nlc-3.589.17 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8B3254F5CDC441C9E70F29A527B2D41B5008103375EB52043C0A158E5991D823`
- **Status:** runtime pending. Foliage and muzzle flash (docs STEALTH_DESIGN 20.15); first test accepted.

### NLC

- **Close-range foliage:** for stalkers looking at the actor the ray cutoff fades from the vision
  section `transparency_threshold` to `foliage_near_threshold` inside `foliage_near_range` (0 = off).
  New `Feel::Vision` virtuals `feel_vision_threshold` and `feel_vision_mtl_transp_for` (vanilla defaults);
  NPC targets and mutants unchanged.
- **Foliage table:** optional `[nlc_vis_transparency]` (material name substring = see-through, first match
  wins) and `foliage_k` (vis^k) for stalker rays towards the actor; every partly see-through material is
  logged once ("vis mtl").
- **Muzzle-flash reveal:** an unsuppressed actor shot gives stalkers in range that already have the actor
  in view a one-time sum bump of `flash_reveal` x darkness x threshold (`flash_reveal_range`,
  `flash_reveal_gap_ms`; 0 = off).
- **Lua:** `nlc_stealth_ray(id)`: see-through of the stalker's ray to the actor, cutoff, distance, last
  see-through material (debug overlay).
- **Follow-ups after the first test:** `nolos` log lines show the effective (near-faded) cutoff;
  no flash reveal for a stalker already fighting the actor. Game data: `bush_sux = 0.5` in
  `[nlc_vis_transparency]` (one thin-bush face no longer blocks sight at any range).

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
