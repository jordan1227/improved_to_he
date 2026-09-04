# Weapon and weapon-mechanics balance redesign

Discussion draft for Sol 5.6

Review snapshot: repository HEAD 3140670, 2026-09-03
Document date: 2026-09-04
Status: analysis and proposed direction only; no gameplay changes are prescribed by this document and nothing has been deployed to the runnable installation.

## 1. Executive summary

The weapon system is not suffering from one universal “all guns are wrong” problem. The current direction already has several good foundations:

- recoil is resolved from effective live CWeapon fields rather than only from copied LTX values;
- weapon personalities already distinguish first shot, second shot, sustained fire, heat, lateral persistence, recovery, shove, roll, and hip response;
- hold-breath stabilization has an explicit stamina cost, release behavior, timeout wobble, and cooldown;
- real weapon mass and attachment mass feed stamina and recoil behavior;
- mastery, health, stance, exoskeleton, and artifact effects are separated into a context layer;
- suppressors provide a real composite stat change and already have an intentional durability cost;
- the inventory UI has one compact weapon-control score instead of exposing every internal recoil axis.

The largest balance risks are therefore structural and systemic:

1. Weapon identity and ammo compatibility are not always trustworthy. A weapon’s technical ID, display name, inherited section, spawner ID, reward entry, and actual ammo_class can describe different things.
2. Merc ammunition is clearly tuned as NPC scaling ammunition, but the current pickup and NPC-drop paths can make it player-accessible. Its damage should not be tuned globally until the access contract is enforced.
3. RPK-16 and USAS-12 content is still present in active references and recoil coverage even though they are now intended for removal. They should be quarantined from balancing and removed with their rewards, spawns, ammo, profiles, and attachments retargeted.
4. The new automatic first-shot factor and the pre-existing global vertical factor currently stack. The first shot of an automatic weapon can receive 1.20 x 1.20 = 1.44 before ordinary weapon, actor, and context terms. That is a deliberate-looking change, but it needs an explicit balance decision.
5. The displayed Handling score is useful but not yet a complete representation of the live shot path. It excludes some persistent factors while including others. It should remain a single minimal UI value, but its definition must be made internally consistent.
6. Mutant gunshot alerting is currently a coarse, explicit Lua radius system, while human gunshot perception is mostly native danger/sound processing plus AI memory and friend propagation. They must be measured separately. The recent mutant work does not prove that suppressed weapons are quieter to human NPCs.
7. Suppressor durability is an important NLC tradeoff and should remain. The next suppressor pass should refine the composite stat profile and stealth tiers, not remove the wear penalty or make every suppressor equivalent.

Recommended order:

1. Establish identity, period, supply, reward, and ammo invariants.
2. Enforce merc ammunition as NPC-only, then test NPC lethality and corpse/reward behavior.
3. Quarantine/remove RPK-16 and USAS-12 and replace affected rewards.
4. Converge or explicitly document the weapon taxonomies used by recoil, mastery, mass, and UI.
5. Calibrate Handling and recoil from a shared neutral simulation.
6. Tune mass, ADS timing, ready timing, suppressor composite stats, and stealth.
7. Add compact ammo-effect communication and perform controlled runtime acceptance testing.

The goal is not to make every weapon equally pleasant. The goal is to make every weapon’s strengths, costs, and acquisition role legible, consistent, and intentional.

## 2. Evidence convention and scope

This document separates four kinds of statements:

- Static: directly confirmed by the current source/configuration.
- Runtime accepted: supported by the existing testing notes or previous controlled in-game observations.
- Runtime pending: plausible from source, but dependent on engine behavior, NPC state, or a controlled A/B test.
- Proposal: a design recommendation for discussion, not an implemented change.

Canonical source:

- Editable source: C:\Games\NLC_OGSR_HE files\improved_to_he
- Runnable test installation: C:\Games\NLC_OGSR_HE
- This review only modifies the canonical source repository. Documentation work must not deploy or mirror anything into the runnable installation.

The latest commit 3140670 is primarily a large NLC 3.0 and ALT weapon-pack/content import plus unrelated UI/watch/sleeve changes. It does not constitute a root active weapon rebalance. The imported optional directories contain their own nested configs, meshes, textures, and even a hook binary; those files should not be treated as active root weapon data unless deliberately installed and tested. Commit 6d65f2f adds hook-side texture-swap tooling, not weapon balance. Commit 8db467a changes psy-phantom/danger behavior and is not evidence of weapon-specific human hearing.

The most relevant current source is:

- gamedata/config/weapons/*.ltx
- gamedata/config/weapons/weapons.ltx
- gamedata/config/weapons/add_silencers.ltx
- gamedata/scripts/kotovod/sivol_fuzz_recoil_profiles.script
- gamedata/scripts/kotovod/sivol_fuzz_recoil.script
- gamedata/scripts/kotovod/sivol_weapon.script
- gamedata/scripts/kotovod/sivol_weapon_aim_speed.script
- gamedata/scripts/ui/ui_inv_descr.script
- gamedata/scripts/death_manager.script
- gamedata/scripts/sak/sak.script and gamedata/scripts/sak/sak_dialog.script
- gamedata/scripts/dsh_monster_attack.script
- gamedata/scripts/dsh_battle_radius.script
- gamedata/scripts/xr/xr_danger.script
- gamedata/scripts/xr/xr_motivator.script
- gamedata/scripts/xrs/xrs_battle_ai.script
- gamedata/config/gameplay/character_desc_nlc30_spawn.xml
- gamedata/config/misc/treasure_manager.ltx
- gamedata/scripts/new_quests.script
- WEAPON_ID_MAP.md
- docs/RECOIL_SYSTEM.md
- docs/INVENTORY_UI.md
- docs/TESTING.md

Line references below refer to the snapshot reviewed on 2026-09-04 and are navigation aids, not a promise that future edits will preserve exact line numbers.

## 3. Current architecture and ownership

| Concern | Current owner | Balance implication |
| --- | --- | --- |
| Engine weapon properties | Effective LTX sections under gamedata/config/weapons | Source of fire interval, magazine, base dispersion, recoil fields, damage, ammo classes, weight, and native wear. Always resolve inheritance before comparing weapons. |
| Technical identity | WEAPON_ID_MAP.md, especially Step 1, Step 2, Step 3, and Step 4 | The map must be consulted before using a name or ID as a real-world assumption. |
| Recoil class and personality | sivol_fuzz_recoil_profiles.script | Owns class priors, per-weapon personality, profile coverage, effective base resolution, handling score, and attachment mass multipliers. |
| Live shot impulse | sivol_fuzz_recoil.script | Owns native recoil neutralization, camera/HUD shot impulse, burst state, heat, actor stamina, context, hold breath, suppressor recoil, and diagnostic logging. |
| Actor weapon state | sivol_weapon.script | Owns manual hold breath, ADS stamina drain, real-mass curves, mastery, health/stance/outfit/artifact context, and suppressor-wear diagnostics. |
| ADS transition speed | sivol_weapon_aim_speed.script | Owns attachment-aware zoom_rotate_time mutation and restoration. This is aim acquisition, not ballistic Handling. |
| Draw/holster readiness | sivol_weapon and the HUD timing keys anm_show_speed_k and anm_hide_speed_k | This is weapon-ready timing, not recoil recovery or ADS transition. Do not count it twice as Handling. |
| Player-facing weapon metrics | ui_inv_descr.script | Owns Accuracy, Weapon control/Handling, ready speed, scope and aim-speed summaries, and compact suppressor comparison. |
| Suppressor composite stats | add_silencers.ltx plus the native condition_shot_dec_silencer path | Owns ballistic modifiers, attachment mass, and intentional durability cost. |
| Mutant alerting | dsh_monster_attack.script and dsh_battle_radius.script | Current explicit weapon-family radius and separate pursuit radius. This is not the human NPC hearing system. |
| Human alerting | Native sound/danger system, xr_danger.script, xr_motivator.script, xrs_battle_ai.script | Current Lua source classifies heard sound but does not expose a confirmed weapon-specific sound-power control. Requires A/B testing. |
| NPC drops and rewards | death_manager.script, spawn XML, treasure_manager.ltx, new_quests.script, dialogs | Must obey resolved weapon ammo_class and the player/NPC ammunition contract. |

### Ownership invariant

Each gameplay effect should have one authoritative owner:

- LTX supplies the authored weapon and ammo inputs.
- Profile resolution converts and normalizes those inputs.
- The recoil controller applies the shot impulse once.
- The actor-state system supplies transient/context multipliers.
- The UI calls the same stateless model used for its neutral comparison.
- Native suppressor wear remains native; Lua may diagnose it but must not apply a second wear decrement.

When a new modifier is added, document whether it is:

- a permanent weapon identity term;
- an attachment term;
- a temporary player-state term;
- a progression term;
- an NPC-only term; or
- a presentation-only term.

This prevents a modifier from appearing once in the live shot path, again in the Handling score, and a third time in the UI description.

## 4. Definitions that should remain stable

### Accuracy

Accuracy means how close shots land relative to the reticle under the weapon’s dispersion/ammo model. It is primarily the LTX fire-dispersion and ammo dispersion result, modified by the relevant attachment. It is not the same thing as camera kick, recovery, sway, or ADS speed.

The player-facing definition should remain short:

> Accuracy: how tightly the weapon places shots around the aim point.

The current get_accuracy path uses fire_dispersion_base, disp_rate, and the first ammo class’s k_disp. That is a useful base approximation, but it is not always the effective loaded-ammo result when a weapon supports several ammo classes.

### Handling / Weapon control

Handling is a composite controllability value. It should summarize the weapon’s practical ability to keep the reticle usable during ordinary aimed fire:

- first-shot impulse;
- second-shot transition;
- sustained-fire climb and heat;
- lateral movement/persistence;
- return/recovery rate;
- a limited representation of practical fatigue or burst control where appropriate.

It should not become a list of technical sub-statistics in the normal UI. Detailed axes belong in diagnostics or an optional developer view.

The player-facing definition should remain short:

> Weapon control: how easily the weapon stays manageable while firing.

Handling is not Accuracy, not draw speed, and not scope magnification.

### Recoil

Recoil is the camera/HUD response to a shot. Camera recoil and HUD recoil must remain separate, as required by the current hook lifecycle. A weapon can have a controlled reticle but a strong visual shove, or vice versa, without those being accidental duplicates.

### Aim speed

Aim speed is the time required to enter the weapon’s aiming state with its current optic. It is not recovery after firing. A scope can slow aim acquisition while improving sight picture; a suppressor can improve recoil while adding mass.

### Ready speed

Ready speed is draw/holster or weapon presentation timing. The current HUD speed keys are a useful per-weapon timing mechanism. It should not be silently mixed into the combat Handling score unless that is made an explicit, separate “handling in transitions” design.

### Stealth

Stealth has at least three separate outcomes:

1. Direct mutant reaction to a gunshot.
2. Human NPC sound/danger detection.
3. Human team communication and target propagation after a shot, hit, or kill.

A suppressor can improve one of these without fully improving the others. The system should not advertise or balance them as though they were one radius.

### Durability

Durability is a resource cost, not a hidden penalty to be removed because the attachment grants other benefits. Suppressors intentionally reduce shots per condition point and should remain valuable but costly.

## 5. Review of the changes since the previous pass

### 5.1 Automatic first-shot recoil

Current source:

- sivol_fuzz_recoil.script: VERTICAL_RECOIL_MULTIPLIER = 1.20
- sivol_fuzz_recoil.script: AUTOMATIC_FIRST_SHOT_MULTIPLIER = 1.20
- profile_has_full_auto checks fire_modes for -1.
- The automatic factor is applied only on burst_shots == 1 for a full-auto-capable section.

The live path is:

1. Build base vertical impulse from live cam_dispersion and cam_dispersion_inc.
2. Apply class and personality terms.
3. Apply ADS/hip, actor stamina, handling-state, fatigue, context, and hold-breath terms.
4. Apply the global vertical factor.
5. Apply the automatic first-shot factor when applicable.
6. Apply attachment/suppressor terms and the camera/HUD output.

The current combination makes an automatic weapon’s opening shot receive 1.44 from these two global factors alone. That is not necessarily wrong. It can give full-auto weapons a meaningful first impulse and discourage careless long bursts. It is, however, a global change that affects every automatic weapon and can become too strong when combined with Day 0 mastery, low stamina, low health, standing posture, heavy weapon fatigue, and sustained heat.

Recommendation:

- Keep the factor separate during diagnostics until Sol decides whether it is a core mechanic or a temporary calibration multiplier.
- Compare three variants in controlled tests: vertical only, automatic only, and both stacked.
- If both remain, consider reducing one of the factors or replacing the pair with one clearer automatic-opening term.
- If it remains a live mechanic, include its permanent effect in the neutral Handling score. Otherwise the UI will systematically overstate the control of automatic weapons.
- Do not solve this by flattening the individual APB, SMG, rifle, and LMG personalities. The global factor and the per-weapon personality answer different questions.

### 5.2 Manual hold breath

Current source:

- sivol_weapon.script: constants around lines 38-48;
- sivol_weapon.script: state machine around lines 169-240;
- sivol_fuzz_recoil.script: get_hold_breath_recoil around lines 952-976;
- sivol_fuzz_recoil_profiles.script: hold_breath_recoil_k = 0.75.

Current behavior:

- only active while aiming with a usable weapon;
- strength ramps up and down rather than switching instantly;
- full strength reduces scripted recoil to 75% of its ordinary value;
- ordinary ADS drain remains active;
- manual holding adds normal_drain x 0.85 x strength;
- natural release after at least eight seconds can play breath-out feedback;
- maximum hold is 15 seconds;
- maximum hold produces a 2.5-second wobble at 1.35x;
- cooldown is two seconds;
- controller-related stamina wobble can still dominate at 1.75x;
- held-breath state is now included in shot diagnostics;
- the ambient breath-sound helpers in sak.script can suppress the actor’s breath sounds while holding.

Assessment:

This is a sound gameplay shape. Holding breath is a deliberate short-term stabilization tool, not a permanent passive buff. The 25% full-strength recoil reduction is meaningful without automatically making every aimed weapon a laser. The extra drain and timeout consequence provide a useful tradeoff.

Concerns:

- At full strength the extra manual drain is 85% of normal ADS drain, so the total ADS drain is approximately 1.85x normal before artifact modifiers. This may be appropriate for NLC, but it should be verified on heavy weapons and at low stamina.
- A player can combine hold breath with crouch/creep, exoskeleton, artifacts, mastery, suppressor, and a high-control weapon. The context clamp protects against extremes, but the combined result still needs an in-game ceiling test.
- The timeout wobble must be clearly distinguishable from ordinary low-stamina wobble so that it feels like the cost of overholding rather than a random weapon bug.
- It is correctly kept out of the persistent health/stance/equipment context. It should also stay out of the normal static Handling score.

Recommendation:

- Keep the current mechanic during the next gun pass.
- Calibrate its full-strength range around the current 20–25% reduction unless testing shows it is mandatory for precision play.
- Make any further change per weapon class only if heavy guns, snipers, or automatic pistols create a clear problem.
- Keep detailed hold duration/strength out of the normal UI; the player only needs the familiar stamina/feedback behavior.

### 5.3 Mastery-day progression

Current mastery table in sivol_weapon.script:

| Class | Day 0 multiplier | Long-term multiplier | Plateau |
| --- | ---: | ---: | ---: |
| Pistol | 1.15 | 0.75 | 28 |
| Shotgun | 1.25 | 0.85 | 28 |
| SMG | 1.30 | 0.80 | 35 |
| Rifle | 1.40 | 0.85 | 42 |
| Sniper | 1.40 | 0.92 | 56 |
| LMG | 1.45 | 0.94 | 56 |
| Special | 1.40 | 0.92 | 49 |

The curve is eased toward neutral during the first seven days, reaches 1.0 at Day 7, then improves toward the long-term cap. The new recoil logs record mastery days, delta percentage, and mastery multiplier.

Assessment:

- The progression gives unfamiliar, high-output classes a meaningful learning phase.
- The long-term cap is not a permanent massive penalty; it remains a moderate class identity.
- The logging is much more useful than guessing from subjective feel.

Concerns:

- This is a class progression system, not a weapon identity system. If the class taxonomy is wrong, the player can receive the wrong learning curve even when the weapon personality is correct.
- Day 0 rifle/LMG penalties are large enough to mask weapon differences in informal tests.
- By Day 7, comparisons become easier, but later mastery can make the same weapon feel substantially different from the early game. Tests must record game days.
- The cap and the exoskeleton/artifact/breath context can create very strong high-tier combinations.

Recommendation:

- Keep class mastery separate from individual weapon personality.
- Do not tune recoil profiles from a test whose game day is unknown.
- Add a runtime test at Day 0, Day 7, and a late plateau day for at least one representative of every class.
- Decide whether the intended progression is “the player learns the weapon family” or “the character learns each weapon.” The current implementation is the former.
- Do not add mastery detail to the normal UI unless the game already communicates the progression elsewhere. Debug logs are sufficient for this pass.

### 5.4 Real-mass handling and attachment mass

Current source:

- sivol_weapon.script: get_weapon_mass_stats around lines 732-768;
- sivol_fuzz_recoil_profiles.script: get_attachment_mass_effects around lines 510-521;
- live CWeapon.inv_weight is preferred, with static/inventory fallbacks;
- mass is clamped to 0.25–12 kg for the curve;
- base wobble falls as mass rises;
- stamina sensitivity rises non-linearly with mass;
- attachment stability bottoms out at 0.88;
- attachment recovery bottoms out at 0.90.

The current design intentionally makes heavy weapons steadier at full stamina but more demanding as stamina drops. This is a good foundation because it gives weight a meaningful cost and benefit rather than making it a pure inventory inconvenience.

Concerns:

- The live weight source, static section weight, inventory item weight, and fitted add-on masses can disagree. Diagnostics should always report which source was used.
- A heavy weapon with a scope, launcher, and suppressor can receive several simultaneous burdens. The total is reasonable only if the player can feel a gradual change rather than a sudden threshold.
- The weapon’s base mass curve and suppressor’s marginal mass effect must not be applied twice.
- A weapon can be low-recoil, heavy, highly mastered, exoskeleton-equipped, and suppressed. That combination should be tested before raising any individual score.

Recommendation:

- Keep the current curve for the next controlled pass.
- Use real weight as an anchor, but allow gameplay compensation for weapons whose real-world mass would otherwise dominate the whole choice.
- Retain a separate diagnostic line for total weapon mass and add-on mass.
- Compare naked, optic, suppressor, and optic-plus-suppressor states on the same weapon.
- Treat a suppressor’s mass as a marginal attachment penalty in the UI comparison, not as a second full weapon-weight penalty.

### 5.5 Attachment-aware ADS speed

Current source:

- sivol_weapon_aim_speed.script modifies only live CWeapon.zoom_rotate_time;
- authored values are cached and restored when the item/add-on/save/load/shutdown state changes or a write fails;
- sight classes use absolute target times:

| Sight class | Target time |
| --- | ---: |
| Red dot | 0.17 s |
| Low magnification | 0.34 s |
| LPVO | 0.40 s |
| Medium magnification | 0.48 s |
| High magnification | 0.58 s |
| Naked baseline | 0.25 s |

There is an additional 0.05-second launcher penalty. The module resolves integrated and attached optics, maps zoom thresholds to classes, handles Specter as LPVO in the relevant path, and protects authored outliers by not making an already-fast red dot slower or an already-slow magnified optic faster.

Assessment:

- This is correctly separated from ballistic recoil.
- The restoration behavior is important and should remain.
- The UI and runtime controller use the same scope classification, which reduces a common source of mismatch.

Pending checks:

- Many weapon configs also contain time_to_aim. The current module writes zoom_rotate_time. Runtime measurement must establish which property actually controls ADS transition in this build for every relevant weapon family.
- The integrated optic override uses technical section IDs; wpn_beretta93 resolves to an FNX-45 in the identity map. This is a reminder not to infer behavior from filenames or IDs.
- “Never make an authored outlier weaker” preserves unusual config behavior. That is safe for compatibility, but it means the absolute target classes do not fully normalize the roster. Sol should decide whether preserving authored exceptions is preferred to strict class normalization.

Recommendation:

- Keep aim speed as a separate displayed modifier only when an optic is present or the difference is meaningful.
- Do not include aim-speed time in the single ballistic Handling number.
- Runtime-test attachment changes, save/load, switching, and failed writes before further tuning.

### 5.6 Inventory UI and metric presentation

Recent UI work includes adaptive headers, native name-header handling, property spacing, weapon metric comparisons, ready speed, scope/aim-speed summaries, Accuracy, and Weapon control/Handling.

The current weapon metrics path computes:

- damage with the attached suppressor’s bullet_hit_power_k;
- bullet speed with bullet_speed_k;
- RPM;
- magazine size;
- resource/wear;
- Accuracy;
- Handling;
- weapon-ready speed;
- scope magnification and aim-speed profile;
- fire modes.

This is a substantial amount of information even when each value is compact. The user-facing UI should remain minimalistic:

Recommended normal rows:

1. Ammo.
2. Damage.
3. Bullet speed or effective range indicator, if already part of the established presentation.
4. Accuracy.
5. Weapon control.
6. Condition/resource.
7. Optional compact aim-speed or scope line when an optic is fitted.
8. Fire modes.

Suppressor detail can stay in the attachment description, with a short comparison on the weapon:

> Accuracy +X%, Weapon control +Y%, durability per shot -Z%, stealth improved.

Do not add first-shot recoil, heat coefficient, lateral persistence, recovery seconds, or mastery percentage to the normal item panel. Those are useful for diagnostics and development, not for the player’s first glance.

## 6. Overall recoil and Handling diagnosis

### 6.1 Current live formula

The live scripted shot path currently combines:

~~~text
total_multiplier =
    aim_or_hip
    x actor_stamina
    x handling_state
    x fatigue
    x health/stance/outfit/artifact/mastery_context
    x hold_breath

vertical =
    authored_live_dispersion_terms
    x personality_and_heat_terms
    x vertical_global
    x automatic_first_shot_if_applicable
    x total_multiplier
    x attachment_stability
    x suppressor_recoil

horizontal =
    authored_live_lateral_terms
    x personality_and_heat_terms
    x total_multiplier
    x attachment_stability
    x suppressor_recoil
~~~

The actual controller also applies the class caps, deterministic correlated wander, camera/HUD separation, and recovery behavior. The formula is flexible enough for a differentiated roster; the risk is that many moderate multipliers can produce an opaque final result.

### 6.2 Current profile strengths

The profile table is already meaningfully personalized:

- PM/PB/Fort use a sharp service-pistol snap with fast recovery.
- P99 has an explicit low-kick, fast-return personality because its inherited native cam_dispersion is an extreme 1.25 degrees.
- Glock 17 sits between the low-kick P99 and heavier .45/magnum families.
- APB/Glock 18C/Pernach use automatic-pistol identities with stronger sustained heat and wander.
- Standard MP5/Bizon have a different personality from the 9x18 MP5 variant.
- AK-104/AK-103, AK-74, AK-74U, Strelok variants, Abakan, M4/G36/FN2000, VAL/Vintorez, DMRs, LMGs, bolt-action rifles, and shotgun families are not simply flat copies.
- Shotguns separate manual actions from self-loaders through first shove and follow-up heat.

This is the right design direction: each weapon should be a role and feel, not only a damage number.

### 6.3 Taxonomy mismatch that should be resolved

There are currently two legitimate but different class tables:

- sivol_weapon.script treats MP5, Bizon, UMP, PP19, and similar weapons as SMG for mastery, mass handling, and progression.
- sivol_fuzz_recoil_profiles.script has five recoil class profiles: pistol, shotgun, automatic_rifle, lmg, and bolt_action. It routes MP5, Bizon, PP19, UMP, and several other SMG-like weapons through automatic_rifle while giving them distinct personalities.

Other examples:

- wpn_m110, wpn_svd, and wpn_svu are sniper-class for mastery but automatic_rifle-class in the recoil table.
- wpn_g36c is LMG-class in recoil and mastery even though its ID/name says G36C; the map resolves it as HK MG36.
- wpn_saiga12c_m2 inherits a rifle-like technical family and is explicitly put in an automatic-rifle recoil class even though its identity map resolves it as VPO-101.

This is not automatically a bug. A recoil class can mean “formula family” while mastery class means “player learning family.” However, the distinction is currently easy to miss and limits later specialization.

Recommendation:

- Either add an explicit recoil SMG class, or document that automatic_rifle is a broad formula family and add per-weapon parameters for SMGs.
- Keep a single canonical identity/role table for semantic tags such as pistol, machine-pistol, SMG, intermediate rifle, battle rifle/DMR, sniper, LMG, and shotgun.
- Derive separate mechanical class views from that table only where necessary.
- Add a validation report that shows each weapon’s semantic role, recoil class, mastery class, and profile personality.
- Do not make an automatic class change in the same patch as a large recoil retune; it would be difficult to tell which change caused a result.

### 6.4 APB versus PM versus P99

The current source explains why APB can feel as controllable as PM:

| Weapon | Current personality highlights | Interpretation |
| --- | --- | --- |
| PM/PB/Fort family | pitch 1.20, first 1.18, sustained 0.90, recovery 1.18 | Strong first snap, quick return, calmer repeated shots. |
| P99 | pitch 0.38, first 0.88, sustained 0.90, recovery 1.55 | Very soft scripted impulse and very fast return, compensating an inherited 1.25 native dispersion outlier. |
| APB family | pitch 1.08, first 0.95, sustained 1.13, recovery 1.05, wander 1.15 | Reasonable opening burst, but sustained automatic fire should become less stable. |

An APB that is controllable in a short burst is not automatically overpowered. It has a different operating envelope:

- the player can place a short burst;
- long automatic fire should expose heat, lateral wander, stamina, and magazine/ammo costs;
- the weapon’s effective range and ammunition role still matter;
- its suppressor can be strong, but the heavy APB suppressor mass and durability cost must remain part of the choice.

Suggested test:

- Compare PM single shots, APB 3-shot bursts, APB 10-shot holds, and P99 single shots at the same game day, stance, health, stamina, outfit, artifacts, optic, condition, and ammunition.
- Measure first immediate pitch, reticle displacement after 3/6/10 shots, time to return within a fixed angle, lateral drift, and stamina consumed.
- If APB only matches PM during short bursts but falls behind during sustained fire, its current identity is working.
- If it remains equal or better at every interval, adjust APB’s sustained heat/wander/recovery or its role economics rather than applying a blanket automatic-pistol nerf.

### 6.5 Recommended weapon-role anchors

These are starting design anchors, not final numeric values.

| Role | Should excel at | Should pay for it | Recoil/Handling identity |
| --- | --- | --- | --- |
| Service pistol | Fast draw, close response, quick follow-up | Lower range, smaller damage envelope, limited magazine | Noticeable first snap, quick recovery, low sustained heat. |
| Low-kick polymer pistol | Rapid reticle recovery, repeated aimed shots | Less first-shot character or lower per-shot impact | Soft impulse, fast return, should not automatically dominate Accuracy. |
| Heavy pistol/revolver | Per-shot damage and intimidation | Strong first shove, slow return, weight/ammo cost | Large first impulse and deliberate cadence. |
| Automatic pistol | Emergency close-range volume | Heat, lateral wander, stamina, ammo consumption, short useful burst | Short burst is manageable; sustained fire becomes clearly worse. |
| SMG | Close/mid-range volume and maneuverability | Cartridge range, damage, sustained heat, limited precision at distance | Low-to-moderate vertical kick, controlled burst, differentiated by rate and lateral persistence. |
| Intermediate rifle | Versatility | No single extreme advantage; ammo and weight | Reliable first shot, moderate climb, meaningful sustained behavior. |
| Compact rifle/carbine | Mobility and close handling | More muzzle movement, shorter sight/range role, lower stability | Livelier lateral response and higher sustained climb than full-size counterpart. |
| Battle rifle/DMR | Per-shot effect and reach | Stronger first impulse, stamina, slower follow-up | Deliberate first kick and recovery window; not a high-DPS automatic laser. |
| Sniper/bolt action | Precision and high-impact single shot | Slow follow-up, heavy shove, poor hip response | Strong discrete kick, rapid enough reset for deliberate shots, minimal automatic heat. |
| LMG | Sustained volume and suppression | Mass, stamina, heat, long recovery, reload burden | Firm opening, increasing wander and fatigue in long strings. |
| Manual shotgun | Close-range burst lethality | Huge discrete shove, reload/action cadence, range | Large first impulse and clear reset before the next shot. |
| Self-loading shotgun | Follow-up speed | Heat, mass, ammo use, lower per-shot character | Smaller first shove than a manual shotgun, but repeated fire destabilizes quickly. |
| Special/experimental | Unique tactical value | Scarcity, ammunition, acquisition, unfamiliar behavior | Use a deliberately authored profile; do not force it into a generic rifle curve. |

### 6.6 Real-life anchors without simulation theater

Real-world values are useful for choosing a direction, not for pretending that a game config is a full ballistic model. Relevant anchors include:

- cartridge impulse and projectile mass;
- weapon mass and balance;
- barrel length and stock/brace geometry;
- cyclic rate;
- operating system and action type;
- sight radius and optic setup;
- suppressor size and cartridge compatibility;
- expected firing cadence.

The gameplay layer should be allowed to compensate for engine limitations and NLC’s survival economy. A real-world number that makes a weapon unpleasant or redundant is not automatically the correct game value.

## 7. Handling score redesign

### 7.1 Current score

get_handling_score in sivol_fuzz_recoil_profiles.script:

- resolves effective live weapon fields;
- simulates six neutral ADS shots;
- includes class, personality, heat, magazine, fire-rate, ADS, global camera pitch/yaw, and optional suppressor recoil/recovery terms;
- uses different weights for automatic and non-automatic weapons;
- rounds to five-point steps and clamps to 10–95;
- intentionally excludes actor state, mastery, and ordinary attachments from the base score.

Current weighting:

- Automatic: first 30%, sustained 30%, horizontal 20%, recovery 20%.
- Semi/other: first 45%, sustained 15%, horizontal 15%, recovery 25%.

This is a reasonable compact UI approach. The problem is not the existence of one score. The problem is that the simulated model does not currently apply the live VERTICAL_RECOIL_MULTIPLIER, AUTOMATIC_FIRST_SHOT_MULTIPLIER, or hold breath, while some global factors and optional suppressor factors are represented.

### 7.2 Recommended definition

Keep one minimal score, but formalize two layers:

1. Base Weapon control: naked weapon, full stamina, standing, neutral mastery, no actor condition penalty, no temporary effects.
2. Effective comparison delta: attachment-only change shown as a compact arrow or comparison, while temporary player state remains dynamic and out of the inventory number.

The base model should include every permanent weapon-path factor:

- live weapon values;
- class priors;
- weapon personality;
- magazine/rate terms;
- permanent global calibration factors;
- automatic first-shot behavior if it is a normal live mechanic.

The base model should exclude:

- current health;
- current stamina;
- stance;
- exoskeleton condition;
- artifacts;
- mastery day;
- hold breath;
- temporary timeout wobble;
- current weapon condition unless the game already makes condition alter weapon control.

The suppressor comparison may remain a separate effective delta because it is a persistent attachment choice. Its mass and gas/recoil effects must be calculated together exactly once.

### 7.3 Implementation shape for a later pass

Proposal for Sol:

- extract a pure six-shot or multi-window simulation function from get_handling_score;
- let the UI call that function for base and attachment comparison;
- let runtime diagnostics call the same function with actual inputs where possible;
- return component values internally but show only the rounded total in normal UI;
- keep a debug export for first, sustained, horizontal, recovery, and the final weighted score;
- add a debug flag that reports which live factors were included/excluded.

This reduces drift between the number the player sees and the behavior the recoil controller actually produces.

### 7.4 Suggested score bands

These are relative anchors for comparison, not a mandate to force every weapon into a fixed tier:

- 80–95: unusually controllable specialist or low-kick weapon; should have a real cost in damage, range, ammo, scarcity, or role.
- 65–80: comfortable service/general-purpose weapon.
- 50–65: characterful but manageable weapon; requires cadence or stance discipline.
- 35–50: demanding weapon with a meaningful damage/reach/volume advantage.
- 10–35: deliberately difficult, experimental, heavy, or high-impact weapon; should not be common filler.

The score should compare within role first. A 70% pistol and a 70% LMG do not need to feel identical because the class weights and firing use are different.

## 8. Content integrity before numerical tuning

### 8.1 Identity source

WEAPON_ID_MAP.md already provides the needed identity work. Use its Step 1 section inventory, Step 2 name/string resolution, Step 3 merged map, and Step 4 spawner reconciliation instead of extracting a second ID list.

The following current mappings are especially important:

| Technical ID | Resolved identity | Why it matters |
| --- | --- | --- |
| wpn_aek971 | AK-104 “Tactical” | The section ID says AEK-971, but the weapon profile should use the resolved weapon and ammo. |
| wpn_ak104 | AK-103 | This is the full-size 7.62x39 AK-103 despite the technical ID. |
| wpn_ak108 | RPK-16 | This is the planned removal candidate, not an AK-101/AK-108 rifle. |
| wpn_g36c | HK MG36 | The name/ID suggests G36C, but the resolved base section is MG36 and its class is LMG-like. |
| wpn_m110 | FN SCAR-H | Do not pair it with M110-specific assumptions or 8.6x70 ammunition. |
| wpn_spas15 | Daewoo USAS-12 | Planned removal candidate. |
| wpn_spas15_m1 | Daewoo USAS-12 M1 | Planned removal candidate and modified variant. |
| wpn_aps | APB | The APB discussion must use the resolved identity, not the APS section name. |
| wpn_fort_m1 | OTs-33 Pernach | Automatic-pistol behavior is tied to a technical ID with a different display name. |
| wpn_beretta93 | FNX-45 | Also used by the integrated-optic override; do not infer it is a Beretta 93. |

The map’s proposed spawner edits are review material, not an instruction to apply them blindly. Several entries have no safe replacement because the section itself is a valid but misnamed technical ID.

### 8.2 RPK-16 and USAS-12 quarantine

This is a roster/content decision, not a recoil-tuning target.

RPK-16:

- runtime section: wpn_ak108 in w_rpk16.ltx;
- resolved by the map as RPK-16;
- currently classified as LMG in recoil and mastery;
- uses ammo_7.62x39_mg;
- appears in spawner/reward/NPC references;
- has profile personalities and scope variants.

USAS-12:

- runtime sections: wpn_spas15 and wpn_spas15_m1;
- resolved by the map as Daewoo USAS-12 and USAS-12 M1;
- currently classified as shotgun;
- appears in spawner/loot/exclusion and attachment coverage.

The user has approved removing these later with Sol because they do not fit the intended 2012 Zone roster. Until that removal:

- do not tune their recoil, Accuracy, Handling, stealth, cost, or suppressor profile;
- flag them in static validation as quarantine items;
- do not use them as baseline weapons for class calibration;
- record every reward, spawn, NPC loadout, quest, dialogue, ammo, scope, silencer, and profile reference that must be replaced;
- remove or retarget their variants as one operation so no dangling section or attachment remains.

The replacement reward should be chosen by role and acquisition context, not simply by replacing the string with the nearest technical ID. A rare USAS or RPK reward may need a different weapon, ammunition, price, and progression placement rather than a one-for-one substitute.

### 8.3 Ammo compatibility must use effective ammo_class

The authoritative pairing is the weapon’s effective ammo_class after inheritance and variant resolution. Display names, filenames, and technical IDs are not sufficient.

Confirmed or high-confidence cases:

| Weapon/reward reference | Effective weapon/ammo context | Finding | Priority |
| --- | --- | --- | --- |
| wpn_ak108 in gameplay/character_desc_nlc30_spawn.xml around lines 1170–1171 | RPK-16, ammo_7.62x39_mg | Spawn data gives 5.56 merc ammunition; wrong if the RPK is retained, and moot after quarantine/removal. | P0 with RPK removal |
| wpn_ak108 in treasure_manager.ltx line 4459 | RPK-16 | Reward gives 5.56x45 SS190; incompatible with the effective RPK ammunition. | P0 with RPK removal |
| wpn_ak108 in new_quests.script line 259 | RPK-16 | Quest gives 5.56x45 AP; incompatible with the effective RPK ammunition. | P0 with RPK removal |
| wpn_ak104 in character_desc_nlc30_spawn.xml around lines 1229–1230 | AK-103, ammo_7.62x39_fmj/ap | Spawn entry gives 5.45 merc. This may be a noncombat/special context, but it is not a literal match. | P1 investigation |
| wpn_g36c in character_desc_nlc30_spawn.xml around lines 8720–8721 | HK MG36, ammo_5.56x45_mg | NPC entry gives 5.56 merc. Same cartridge family, but a different ammo section; decide whether merc is a universal NPC scaling replacement or whether MG ammo needs its own scaling variant. | P1 investigation |
| wpn_m110 in treasure_manager.ltx line 7311 | FN SCAR-H, ammo_7.62x51_fmj/ap | Reward gives ammo_8.6x70. This is a direct compatibility mismatch. | P1 |
| wpn_rpk74 in sak_dialog.script around lines 9762–9764 | RPK-74 | Special NPC/dialogue spawn gives 5.45 merc twice. This may be intentional NPC supply, but it violates the player-access contract if it can become player loot. | P0 contract, context-dependent reward |

Not every reward line containing a weapon and ammo is wrong. Some reward systems use broad supplies, some are conditional, and some entries are not intended to be loaded into the weapon. Each pair must be classified as:

- literal compatible ammunition;
- generic supply for the same cartridge;
- deliberate NPC scaling ammunition;
- a noncombat reward;
- a stale/mistyped reward.

Recommended static checker:

1. Resolve each weapon reference through WEAPON_ID_MAP.md.
2. Resolve its effective ammo_class and all supported ammo variants.
3. Parse reward/spawn/dialogue item lists.
4. Compare weapon/ammo cartridge families and allowed special cases.
5. Report the source path, resolved weapon name, actual ammo class, supplied item, and context.

Do not use a broad replacement such as “all 5.56 rewards become 5.45” or “all merc ammo becomes AP.” That would hide the underlying identity errors.

## 9. Merc ammunition: P0 investigation and enforcement

### 9.1 Current evidence

weapons.ltx defines:

- ammo_5.45x39_merc inheriting from ammo_5.45x39_ap with k_hit = 4.6;
- ammo_5.56x45_merc inheriting from ammo_5.56x45_ap with k_hit = 4.6;
- both are visually hidden or minimized in inventory presentation;
- comments and spawn usage indicate they are intended for NPC/mercenary scaling rather than ordinary player ammunition.

For comparison, regular 5.45 and 5.56 ammunition uses much lower k_hit values. Merc is therefore not just a cosmetic alternate cartridge. It can materially change NPC lethality if a player receives it or if an NPC carries an unintended quantity.

### 9.2 Current access/leak paths

death_manager.script:

- always_offs_items_misc includes both merc sections;
- the non-actor NPC path calls he_drop_single_ammo;
- he_drop_single_ammo reads the NPC weapon’s ammo_class, releases the NPC inventory, and spawns one selected ammo section;
- therefore a weapon whose effective ammo_class is merc can generate merc ammo after the inventory cleanup logic;
- the ordinary keep_item filter is not sufficient to stop this generated object.

sak.script:

- check_used_item first returns for any section beginning with ammo_;
- the later branches intended to release ammo_5.45x39_merc and ammo_5.56x45_merc are therefore unreachable.

ui_inv_descr.script:

- the merc sections are hidden from the normal ammo property list;
- this is presentation-only and does not prevent pickup, inventory use, trade, or a custom spawn from putting the object in the player’s inventory.

The current issue is therefore enforcement, not merely visibility.

### 9.3 Recommended contract

Proposed contract for Sol to approve:

> Merc ammunition is an NPC-only scaling resource. It may be used by intended NPC loadouts, but it must not be a normal player pickup, trade item, corpse reward, quest reward, or player-usable ammunition unless a specifically approved special-case system says otherwise.

There are two viable implementations:

Option A: NPC virtual/scaling ammunition

- NPCs receive the intended special ammo behavior;
- corpse drop converts it to ordinary compatible ammunition or drops nothing;
- the player never sees the scaling cartridge.

Option B: physical but non-player-transferable ammunition

- NPCs use the physical section;
- any player pickup, trade, or reward transfer immediately removes or converts the object;
- all custom spawn paths and corpse drops are audited;
- debug tools can still expose it for testing.

Option A is safer for the reward economy. Option B may be simpler if the engine/NPC weapon path depends on the physical object. The choice must be made before changing k_hit.

### 9.4 P0 acceptance criteria

Static:

- merc sections appear only in an approved NPC-supply allowlist;
- no normal shop, player stash, quest reward, treasure reward, or actor-facing spawn creates them;
- all NPC corpse-drop paths either suppress or convert them;
- pickup code checks merc before the generic ammo early return;
- both 5.45 and 5.56 sections are covered;
- UI hiding remains a presentation layer, not the only protection.

Runtime:

- intended NPCs still use their intended weapon output;
- non-actor death does not create player-lootable merc ammo;
- actor-killed and special-NPC/dialogue paths do not leak merc ammo;
- save/load and item transfer do not bypass the rule;
- debug logging can identify the source context when a merc object is created;
- ordinary player ammunition is not silently replaced by merc in the process.

Balance:

- only after enforcement, compare NPC time-to-kill, hit lethality, and ammo consumption against rank/faction expectations;
- if NPC lethality is too high, first adjust NPC loadout distribution or weapon/ammo pairing;
- change k_hit only with a measured target and a player-access regression test.

## 10. Suppressor redesign

### 10.1 Non-negotiable design intent

Suppressors intentionally:

- improve a composite of weapon stats;
- improve stealth to some degree;
- add mass;
- reduce durability per shot;
- cost money and occupy an attachment slot;
- can change weapon handling through both gas/recoil coefficients and attachment mass.

The durability loss is a valuable NLC tradeoff and should remain. The suppressor should not be balanced as “free accuracy and damage.”

### 10.2 Current composite coefficients

Current add_silencers.ltx values:

| Family/section | Mass | Damage | Bullet speed | Accuracy effect | Recoil coefficient | Wear coefficient |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 9 mm | 0.31 kg | 1.05 | 1.00 | 0.92 | 0.87 | 1.10 |
| 5.45 rifle | 0.45 kg | 1.04 | 1.01 | 0.93 | 0.88 | 1.15 |
| 5.56 rifle | 0.425 kg | 1.05 | 1.01 | 0.93 | 0.88 | 1.13 |
| 7.62x39 rifle | 0.435 kg | 1.04 | 1.01 | 0.91 | 0.84 | 1.15 |
| 7.62x51 rifle | 0.60 kg | 1.03 | 1.02 | 0.90 | 0.82 | 1.17 |
| 7.62x54 rifle | 0.65 kg | 1.03 | 1.01 | 0.89 | 0.83 | 1.18 |
| Mosin | 0.72 kg | 1.05 | 1.00 | 0.94 | 0.82 | 1.07 |
| .45 pistol | 0.25 kg | 1.05 | 1.00 | 0.93 | 0.87 | 1.12 |
| APB | 0.70 kg | 1.05 | 1.00 | 0.92 | 0.87 | 1.10 |
| VSK | 0.50 kg | 1.03 | 1.00 | 0.92 | 0.85 | 1.12 |
| Salvo 12 | 0.97 kg | 1.00 | 1.00 | 0.95 | 0.85 | 1.20 |
| Groza | inherits VSK family | inherits | inherits | inherits | inherits | inherits |

The percentages should be read as current coefficients, not automatically as target values. For example, recoil coefficient 0.82 is an 18% gas/recoil reduction before the mass effect.

### 10.3 Current code path

The recoil controller:

- resolves the fitted suppressor section and cam_dispersion_k;
- calculates marginal suppressor mass relative to the same attachment load without the suppressor;
- applies gas/recoil and mass stability together;
- applies marginal recovery through the attachment recovery term;
- gives the UI the same combined comparison.

The native condition_shot_dec_silencer path owns durability. sivol_weapon only diagnoses configured and observed values. Keep that ownership boundary.

### 10.4 Tuning recommendations

- Keep the overall composite structure.
- Keep durability reductions as the primary resource tradeoff.
- Ensure the same suppressor is not counted in full total attachment mass, then again as a second complete weapon-mass penalty in UI or runtime.
- Tune suppressor families by cartridge and intended acquisition role, not by a universal coefficient.
- Keep the damage increase modest enough that the main reason to buy a suppressor is the combined control/stealth profile, not raw damage.
- Treat accuracy improvement as a reticle-placement benefit, separate from camera recoil.
- Verify that large suppressors such as Salvo 12 have enough wear and mass cost to justify their use.
- Verify that integrated suppressors and detachable suppressors have distinct stealth and durability roles.
- Preserve rare suppressors as direct upgrades when their cost, scarcity, or acquisition role justifies it. They do not need to be forced into sidegrades merely for numerical symmetry.

## 11. Stealth redesign and human A/B testing

### 11.1 Mutant system: what is confirmed

dsh_monster_attack.script currently uses explicit detection families:

| Family | Current alert radius |
| --- | ---: |
| Loud shot | 30 m |
| PB | 3 m |
| VAL/Vintorez family | 6 m |
| SVU family | 12 m |
| Generic detachable suppressor | 15 m |

The current flow is:

- take a short gunshot snapshot;
- eligible nearby monsters within the family radius receive a zero-damage fire-wound/panic trigger;
- dsh_battle_radius.create_radius then creates a separate fixed 10 m pursuit/battle radius.

The source explicitly says the battle/pursuit radius remains separate. Do not change that radius to tune detection.

The user’s intended ordering is important:

> SVU should not be quieter than a suppressed AK-74.

The current SVU value of 12 m is below the generic detachable suppressed value of 15 m, so it violates that intended gameplay ordering unless the AK-74 is not actually using the generic detachable rule in a given state.

Starting proposal:

- place SVU at the generic suppressed-rifle tier or slightly above it, beginning at 15 m for controlled testing;
- preserve the special 3 m PB and 6 m VAL/Vintorez roles if they remain desirable;
- decide whether SVU should be a special family at all, or simply use a cartridge/attachment rule;
- test attached/detached behavior for all family prefixes, including VAL nimble and variant sections.

Do not finalize exact radii from static source alone. The direct trigger is source-confirmed, but the perceived result depends on monster eligibility, map space, line of sight, and the one-shot snapshot window.

### 11.2 Human system: what is and is not confirmed

Human NPC perception is a different chain:

- m_stalker.ltx defines weapon sound perception and sound thresholds;
- xr_danger.script has default sound ignore distance around 40 m and sound danger persistence around 10 seconds;
- xr_motivator.script hear_callback receives sound_type, sound_position, and sound_power, classifies weapon sounds, but its final classified branch is empty;
- xrs_battle_ai.script consumes sound/visible memory and propagates target information to friends;
- wpn_is_pistol_sil and death_place suppress friend notification for a silent pistol kill, but the special case does not cover suppressed rifles;
- friend_is_near can treat same-community radio, visible relation, or distance under 25 m as sufficient for propagation.

Static evidence therefore does not prove that sound_power is controlled by the weapon’s LTX sound name or suppressor attachment. snd_silncer_shot may change what the player hears without changing the engine’s AI danger strength.

The correct next step is human A/B testing, not changing the native radius or using dsh_battle_radius as a substitute.

### 11.3 Human stealth A/B matrix

Use the same map, NPC rank/faction, weather/time, stance, actor position, weapon condition, ammo, and save state. Record:

- time from shot to first danger response;
- whether the NPC registers attack_sound, enemy_sound, bullet_ricochet, or another type;
- best enemy/target selection;
- number of direct witnesses;
- number of friends receiving a propagated target;
- time until danger expires;
- whether the result differs for a miss, body hit, and kill.

Test:

| Variable | Values |
| --- | --- |
| Weapon | unsuppressed AK-74; suppressed AK-74; suppressed SVU; suppressed pistol; integrated VAL/Vintorez where available |
| Shot outcome | miss; body hit; kill |
| Fire pattern | one shot; 3-shot burst; 10-shot burst |
| Distance | 10, 20, 30, 40, 50 m |
| Geometry | open ground; partial cover; indoor/occluded |
| NPC state | idle; already suspicious; combat with another target |
| Team context | isolated NPC; friend under 25 m; friend over 25 m; same-community radio group |

Use dsh_coordinator diagnostics where possible to log best danger type and target state. Keep mutant tests in a separate run so a monster’s direct Lua trigger cannot contaminate the human conclusion.

### 11.4 Desired future stealth model

If the engine exposes a reliable weapon-specific sound-power input:

- create a shared resolved weapon-noise profile;
- derive unsuppressed/suppressed strength from cartridge, weapon, and suppressor;
- feed the native sound system without duplicating the event;
- keep human danger persistence and friend propagation as separate downstream behavior.

If the engine does not expose that control:

- consider a native hook or a narrowly scoped synthetic danger layer;
- instrument it first;
- do not create a second parallel sound event that doubles detection;
- make the source of each human danger event visible in diagnostics.

The profile should eventually support:

- direct detection strength/radius;
- hit/kill notification modifier;
- burst/cadence accumulation;
- indoor/open-space behavior if the engine permits it;
- suppressor family and condition effects where justified.

## 12. Ammo behavior and player communication

Ammo should communicate meaningful effects without becoming another technical spreadsheet.

### 12.1 Current problems

- Merc ammo is hidden in the UI but not reliably blocked from pickup.
- Accuracy is derived from the first ammo class listed, not necessarily the currently loaded type.
- A weapon can list multiple ammo classes without a compact explanation of what changes between them.
- Damage and Accuracy rows can change under a suppressor, while the reason is only obvious in the attachment description.

### 12.2 Suggested compact presentation

Normal UI examples:

- Standard: balanced damage and control.
- AP: penetration up, possible damage/accuracy tradeoff.
- Match/precision: accuracy up, lower general-purpose effect if configured.
- Buckshot/slugs: close spread or single-projectile behavior.
- AI-only merc ammunition: never presented as normal player ammunition; only visible in debug or an explicit NPC diagnostic.

The exact labels must be derived from actual ammo fields such as k_hit, k_disp, and penetration-related values. Do not display an effect that the engine does not apply.

### 12.3 Effective loaded ammo

Later UI work should choose one clear rule:

1. If the active weapon exposes current ammo type, show the effective current type.
2. If no current type is available, show a base value plus a compact range or “varies by ammo.”
3. If a weapon has one ordinary ammo class and one hidden NPC class, ignore the hidden class in player-facing comparison.

The UI should not silently present the first listed class as though it were always loaded when that is not true.

## 13. NPC combat power and reward economy

Weapon balance cannot be evaluated only by player tooltip values. NPC combat power is a product of:

- weapon damage and fire rate;
- effective ammo type;
- NPC rank/accuracy/deviation;
- armor and hit location;
- weapon condition;
- range and AI cadence;
- number of NPCs sharing information;
- whether special ammunition is player-accessible after the kill.

Recommended budget questions for every high-tier NPC weapon:

- What can the NPC kill in a short burst?
- How much of that power is from the weapon versus merc ammunition?
- Does the player receive the same power after looting?
- Is the reward intentional for this progression point?
- Is the weapon itself a period/roster fit?
- Does the weapon have a compensating cost such as scarcity, repair expense, weight, ammo scarcity, or suppressor wear?

When a special NPC weapon is removed, replace the encounter’s role and reward budget, not only its visual model.

## 14. What should not be done in the next pass

- Do not globally reduce merc k_hit before closing pickup/drop/reward leaks.
- Do not tune RPK-16 or USAS-12 as if they will remain.
- Do not infer active root configuration from the optional NLC/ALT pack directories.
- Do not use technical IDs as real-world identities without consulting WEAPON_ID_MAP.md.
- Do not replace every wrong ammo pair with a same-caliber guess.
- Do not flatten APB, PM, P99, or SMG behavior to make one screenshot of a score look tidy.
- Do not remove suppressor durability loss.
- Do not use dsh_battle_radius to implement human stealth.
- Do not change human NPC perception without a controlled A/B baseline.
- Do not add a row for every recoil axis to the normal item UI.
- Do not change camera and HUD recoil ownership together with a balance retune unless the visual response is itself the test target.
- Do not stack a new ammo modifier on top of an existing damage/dispersion modifier without documenting the owner.

## 15. Proposed implementation phases

### Phase 0: decisions and inventory

Deliverables:

- Sol-approved weapon role/identity registry;
- explicit RPK-16 and USAS-12 removal/quarantine list;
- merc ammunition contract;
- definition of base Handling score;
- definition of human stealth test instrumentation;
- active-root versus optional-pack boundary.

Exit criteria:

- no ambiguous ID is used as a baseline without a resolved identity;
- no proposed numerical tuning targets a quarantined weapon;
- Sol agrees whether merc is virtual/converted or physical/non-transferable.

### Phase 1: supply, reward, and ammo integrity

Deliverables:

- effective ammo-class checker;
- weapon/reward/spawner mismatch report;
- corrected or replaced M110 reward;
- corrected/removed RPK references;
- reviewed AK-103 and MG36 NPC supply;
- merc pickup/drop/reward enforcement;
- diagnostics for unexpected merc creation.

Exit criteria:

- static checker has no unreviewed high-confidence mismatches;
- player cannot obtain merc ammunition through ordinary paths;
- NPC combat supply still works.

### Phase 2: canonical roles and profile coverage

Deliverables:

- one semantic identity/role registry based on WEAPON_ID_MAP.md;
- explicit mapping to recoil formula class and mastery class;
- decision on a dedicated SMG recoil class;
- profile coverage check for every active base weapon and variant;
- exclusion list for non-firearms and quarantined items.

Exit criteria:

- every active firearm is either covered or explicitly excluded;
- profile class, mastery class, and semantic role are visible in a validation report;
- new weapon configs fail validation if they have no class/profile decision.

### Phase 3: recoil and Handling calibration

Deliverables:

- decision on stacked global vertical/automatic first-shot factors;
- shared neutral sequence simulation for runtime diagnostics and UI;
- per-role anchor values;
- APB/PM/P99 and SMG comparisons;
- first/sustained/lateral/recovery target bands.

Exit criteria:

- Day 0, Day 7, and plateau measurements are recorded;
- full-stamina naked comparisons are repeatable;
- Handling score ordering matches observed role ordering;
- temporary context is not mistaken for weapon identity.

### Phase 4: mass, ADS, and readiness

Deliverables:

- live/static mass-source audit;
- naked/optic/suppressor/launcher comparisons;
- aim-speed runtime proof for zoom_rotate_time versus time_to_aim;
- draw/holster timing review;
- save/load and attachment restoration tests.

Exit criteria:

- no double-counted attachment mass;
- no failed write leaves a modified weapon;
- UI does not present ready speed as ballistic Handling.

### Phase 5: suppressor and stealth balance

Deliverables:

- current suppressor coefficients reviewed by cartridge and acquisition role;
- native wear measurements;
- mutant radius A/B, including SVU versus generic suppressed rifle;
- human sound/danger A/B;
- decision on native versus hook/synthetic human stealth control.

Exit criteria:

- suppressor wear remains measurable and intentional;
- SVU ordering matches the intended gameplay result;
- human stealth claims are based on recorded danger/propagation data;
- pursuit radius remains separate.

### Phase 6: UI communication

Deliverables:

- stable Accuracy/Weapon control definitions;
- effective-ammo or “varies by ammo” rule;
- compact ammo-effect labels;
- compact suppressor tradeoff wording;
- debug-only detailed recoil output.

Exit criteria:

- a player can distinguish Accuracy from Weapon control;
- no technical overload in the ordinary item panel;
- displayed values match the shared neutral model.

### Phase 7: regression and acceptance

Deliverables:

- representative weapon matrix;
- NPC supply/reward matrix;
- stealth matrix;
- save/load/attachment matrix;
- screenshots or short UI review notes;
- source diff and deployment list if a later implementation is approved.

Exit criteria:

- no unreviewed source errors;
- no unexpected runnable-installation changes;
- runtime evidence is separated from static assumptions;
- Sol/user can approve or reject concrete changes one workstream at a time.

## 16. Runtime test protocol

### 16.1 Recoil controls

For each test, hold constant:

- save and level;
- game day/mastery;
- actor health and stamina;
- stance;
- outfit and condition;
- artifacts and battery gate;
- ammunition;
- weapon condition;
- optic/launcher/suppressor state;
- distance and target.

Record:

- section and resolved base section;
- semantic role;
- recoil class;
- mastery class and day;
- automatic-fire decision;
- raw and degree-converted live recoil fields;
- first/second/sustained stage;
- heat;
- hold-breath strength;
- context components;
- attachment mass/stability/recovery;
- immediate pitch, tail pitch, yaw, and return time.

Use shot windows of:

- first shot;
- first two shots;
- six-shot sequence;
- 10-shot automatic string;
- 27-round burst where magazine and class permit.

### 16.2 Handling score

For each weapon:

- capture the displayed score;
- record the neutral simulated components in diagnostics;
- measure actual first-shot displacement;
- measure average sustained displacement;
- measure horizontal drift;
- measure recovery to a fixed angle;
- compare naked and suppressor states.

The score does not need to predict every player’s perception exactly. It should preserve useful ordering within role and avoid claiming precision it does not have.

### 16.3 Hold breath

Test:

- hold versus toggle input;
- strength ramp;
- natural release under eight seconds;
- natural release after eight seconds;
- forced release at 15 seconds;
- two-second cooldown;
- low stamina release;
- weapon switch/ADS exit;
- save/load/reset;
- controller artifact interaction.

### 16.4 Stealth

Use the matrix in Section 11.3. Run mutant and human tests independently. Treat “the player heard a quieter sound” as irrelevant to NPC stealth unless the danger logs confirm a changed NPC response.

### 16.5 Supply and reward

Test:

- intended merc NPC loadout;
- NPC death not caused by actor;
- NPC killed by actor;
- special dialogue spawn;
- quest reward;
- treasure reward;
- player pickup;
- trade attempt;
- save/load after pickup;
- weapon with multiple ammo classes and selected ammo type.

## 17. Open decisions for Sol

1. Is merc ammunition strictly NPC-only, including corpse drops and special NPC/dialogue rewards, or are there approved player-facing exceptions?
2. Should merc ammunition be physically present for NPCs, converted on drop, or represented through a virtual/scaling path?
3. Should the global 1.20 vertical factor and automatic first-shot 1.20 factor stack?
4. Should permanent automatic first-shot behavior be included in the displayed base Handling score?
5. Should recoil gain a dedicated SMG class, or should automatic_rifle remain a broad formula class with stronger per-weapon parameters?
6. Are mastery classes intentionally different from recoil classes for M110/SVD/SVU and SMG families?
7. Is preserving authored aim-speed outliers more important than strict sight-class normalization?
8. Can the target engine expose or modify sound_power per weapon in the human hearing path?
9. If not, should human stealth use a native hook or a synthetic danger layer?
10. Should SVU use the generic suppressed-rifle mutant tier, a slightly louder tier, or a cartridge-derived tier?
11. Which concrete rewards replace RPK-16 and USAS-12, and at what progression/acquisition role?
12. Are optional NLC/ALT weapon packs intended to be installed in this branch, or should they remain a separate source/archive?
13. Should the UI show the effective loaded ammo type, a base/range value, or only compact ammo-effect labels?
14. Should high-cost rare variants remain direct upgrades? Current recommendation: yes, when scarcity, cost, and acquisition role justify it.

## 18. Suggested handoff request to Sol

Sol’s concrete plan should return, for each proposed change:

- exact source files and sections;
- whether the change is config, Lua, native hook, reward/supply, or UI;
- the resolved weapon identity and ammo_class used;
- the mechanic owner;
- expected player/NPC effect;
- interaction with mastery, mass, suppressors, artifacts, stance, and hold breath;
- runtime test setup;
- acceptance thresholds;
- rollback/failure behavior;
- whether any optional-pack file is involved;
- explicit list of files to implement in the first patch.

The first implementation batch should be narrow enough that recoil, ammo integrity, stealth, and reward changes can be tested independently. This document is intended to give Sol the full context needed to produce that concrete plan; it is not approval to implement all proposals at once.

## 19. Source evidence index

| Topic | Navigation |
| --- | --- |
| Identity and aliases | WEAPON_ID_MAP.md Step 1–4, especially merged-map and spawner-reconciliation entries for wpn_aek971, wpn_ak104, wpn_ak108, wpn_g36c, wpn_m110, wpn_spas15, and wpn_spas15_m1 |
| Global recoil and new first-shot/hold-breath path | gamedata/scripts/kotovod/sivol_fuzz_recoil.script around lines 16–35, 952–976, 1330–1425, and 1560–1586 |
| Personalities and classes | gamedata/scripts/kotovod/sivol_fuzz_recoil_profiles.script around lines 13–255, 256–439, 510–589, and 592–710 |
| Mastery, mass, actor context, and suppressor diagnostics | gamedata/scripts/kotovod/sivol_weapon.script around lines 38–48, 169–240, 301–342, 469–539, 545–617, 732–825, and 988–1138 |
| Aim transition timing | gamedata/scripts/kotovod/sivol_weapon_aim_speed.script around lines 1–31, 170–240, and 249–315 |
| Accuracy/Handling/UI | gamedata/scripts/ui/ui_inv_descr.script around lines 267–311, 1598–1660, 1703–1727, and 1755–1812 |
| Suppressor composite coefficients | gamedata/config/weapons/add_silencers.ltx |
| Merc ammo | gamedata/config/weapons/weapons.ltx around lines 337–340 and 419–420 |
| NPC drops and merc leak | gamedata/scripts/death_manager.script around lines 24–40, 149–182, 361–389, and 459–482 |
| Pickup cleanup ordering | gamedata/scripts/sak/sak.script around lines 2055–2077 |
| Mutant gunshot alerting | gamedata/scripts/dsh_monster_attack.script around lines 1–24 and 240–290 |
| Human danger baseline | gamedata/config/creatures/m_stalker.ltx around lines 69–74 and 340–347; gamedata/scripts/xr/xr_danger.script |
| Human sound callback | gamedata/scripts/xr/xr_motivator.script around lines 511–556 |
| Human team propagation and silent pistol exception | gamedata/scripts/xrs/xrs_battle_ai.script around lines 1497–1593 and 1721–1757 |
| NPC/reward mismatches | gamedata/config/gameplay/character_desc_nlc30_spawn.xml, gamedata/config/misc/treasure_manager.ltx, gamedata/scripts/new_quests.script, and gamedata/scripts/sak/sak_dialog.script |
| Existing recoil test expectations | docs/RECOIL_SYSTEM.md and docs/TESTING.md |
| UI constraints | docs/INVENTORY_UI.md |
