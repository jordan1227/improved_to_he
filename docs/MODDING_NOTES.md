# Modding notes

Document class: current cross-cutting implementation patterns. These notes are
source-grounded guidance, not standalone proof of live runtime behavior; use
the labels and report format in `WORKFLOW.md`.

These are concise observations from the thermal-anomaly and device-interference work.

- Prefer existing lifecycle signals (`on_anomaly_spawn` and
  `on_anomaly_net_destroy`) for static anomaly caches. Re-enumerating global anomaly
  lists every frame is more expensive and can return server-side objects where a
  client object is required.
- Cache stable anomaly data (client position, shape radius, section/tier) and do the
  inexpensive actor-distance calculation locally. Keep per-frame evaluation only
  where smooth visual or camera response matters.
- A shared, throttled proximity result is cleaner than separate PDA, NVD, and
  flashlight scans. The current Electra refresh interval is 200 ms, with consumer
  smoothing still running every frame.
- Save callbacks should perform persistence work only on the real save signal, not
  as an `on_update` callback. Runtime capability checks are safer than caching module
  availability at load time when addon order can vary.
- `bind_stalker.script` forwards `on_item_to_slot`, `on_item_to_belt`, and
  `on_item_to_ruck` through `ogse_signals`; `on_drop_before_all` and
  `on_drop_after_all` bracket its drop bookkeeping. These are reliable cache
  invalidation hooks for outfit and belt-dependent values. Recompute lazily after
  invalidation instead of scanning inventory every frame.
- `log2` state-change and cache lifecycle messages are useful for in-game diagnosis;
  avoid per-frame or periodic damage-tick logging except when deliberately debugging.
  Synchronous log output can itself produce repeatable frame spikes.
- For ambient actor damage, `hit.burn` with zero impulse, a valid draftsman, and
  `bip_none` preserves the anomaly burn behavior while avoiding localized blood
  decals and sprint interruption.
- Never call `db.actor:get_current_outfit_protection(...)` while slot 6 is empty.
  Its native no-outfit path was measured taking roughly 230 ms and caused periodic
  stalls near thermal and chemical anomalies. Check `actor:item_in_slot(6)` first
  and return/cache zero protection when it is nil.
- `get_current_outfit_protection(hit.burn)` supplies the effective outfit layer;
  tested powered belt artifacts are not included by this getter. Their immunity
  factors stack multiplicatively (`outfit_damage_mult * art_1 * art_2`) and are
  maintained separately by the event-invalidated belt cache.
- In per-frame proximity loops, reuse persistent result tables and mutate their
  fields. Avoid table literals, `string.format`, sorting, and concatenation in the
  steady-state path. Obtain the actor position once and share it across scans.
- For weapon effects, use the zoom effector (`wpn_params.get_zie_obj()`) as the
  ADS source. It is a temporary rendered-direction overlay, not the actor camera
  base: preserving the final aim direction at ADS exit requires one controlled
  rebase of `device().cam_dir` into `actor_camera(0)` (yaw/pitch are the negative
  direction heading/pitch), plus a last-direction fallback for missed lifecycle
  signals.
- Keep hold-breath activation and every exit cause centralized in
  `sivol_weapon`; FOV/HUD-FOV cleanup, sound policy, cooldown, and mask gates
  should consume that state rather than duplicating input checks.
- Cache weapon mass by active item and attachment signature. Use the live weapon
  mass plus resolved attached addon sections; do not recompute or log it per
  frame. Diagnostic logging stays behind `sivol_weapon.weapon_breath_diagnostics`.
- Keep scripted camera recoil and HUD-model recoil as separate channels. Normalize
  HUD pitch against the global camera-pitch multiplier so broad camera balance
  changes preserve accepted viewmodel motion; use effective-section personality
  fields for intentional weapon-to-weapon HUD differences.
- Equipment recoil context belongs beside the existing `sivol_weapon` artifact
  and mass caches. Return scalar factors on a real shot; do not allocate a table
  or scan the belt from the recoil frame loop. Preserve outfit-condition scaling
  and the powered-artifact gate.
- The FL-hook HUD transform is an absolute transient contribution. Clear it once
  at identity and stop writing while idle; retain owner checks, watchdog cleanup,
  and render-time restoration across every Lua lifecycle/error path.
- Prefer OGSR's native `condition_shot_dec_silencer` in weapon sections for
  suppressor wear. It is an absolute per-shot wear value, so derive it from the
  effective `condition_shot_dec` and the fitted suppressor profile. Keep
  `condition_shot_dec_k` on suppressor sections as balance/UI metadata; do not
  subtract condition again from an actor weapon-fire callback.
- HUD FOV is a console value. Capture the pre-hold value, smoothly approach the
  temporary target only after ADS settles, and restore that exact captured value;
  exclude optic-driven adaptive zoom and binocular cases from this visual layer.
- Electra's native zone initialization requires an `idle_light` key through its
  inheritance chain. Preserve the baseline base value (`off`) when a staged port
  is not enabling the light; weak needs an explicit `off` override when the base
  is changed to `on`.
- In this OGSR build, an `idle_light_anim` name must exist in the compiled
  `lanims.xr` table. The tested `koster_01_electra` entry was added to that table
  externally; LTX configuration alone is insufficient. Treat a missing animation
  entry as a deployment prerequisite, not as evidence that the zone feature is
  optional.
- Keep ARRIVAL's Aggressive-v2 Electra idle group under a renamed project path and
  rename its divergent spark child too. This prevents its strong-only look from
  overwriting the Core idle effect used by average/base.
- Do not infer anomaly identity from similarly named ARRIVAL gravitational assets.
  `zone_mine_gravitational_*` is the Whirligig presentation set, while
  `zone_gravi_zone` explicitly keeps Vortex particle overrides. Preserve that
  visual distinction because NLC players use anomaly recognition for cooking.

## Mutant and controller integration

- Controller decisions run every 750 ms, but a 60-140 ms reticle dwell cannot
  be measured reliably from monster-binder updates. Reticle pressure therefore
  runs from `sivol_weapon`'s existing 16 ms update and calls back into
  `bind_monster`; keep all balance, ownership, and cooldown state in the binder.
- Controller cover uses three ray samples. Sensed abilities retain a non-zero
  pressure floor behind cover; targeted abilities require LOS. An exact reticle
  hit is treated as effective 3/3 exposure for targeted mechanics, while the
  sensed pool continues to use physical cover.
- Psy defenses remain separate by outcome: outfit plus belt artifact telepathic
  protection produces `gear_mult = (1 - outfit) * belt_mult` and softened
  manipulation scaling; mask `psy_k` scales camera/damage intensity; artifact
  `dog_shield` scales weapon-control/drop outcomes. `get_mask_psy_k()` is a
  read-only cached call with no write-back into mask state.
- Controller bind scrambling needs ownership-aware snapshots and idempotent
  re-arming. Restore on timer, death, and destruction; repeated procs must extend
  one owned scramble rather than snapshot an already-scrambled mapping.
- `he_bai_compat.script` is a generic `ambush_cover()` compatibility wrapper,
  not a controller behavior manager. No controller caller was found. Blind-dog
  unreachable-target cover appears to belong to native `SM_DOG_S` AI; controllers
  use native `SM_CONTR`, whose LTX selectors are invoked by a different engine
  state machine. Do not force controller destinations from `bind_monster` without
  first instrumenting native psy-attack state and movement ownership.
- The repository and runnable installation contain no `monster_ai.script`.
  `mob/mob_combat.script` only switches scripted logic sections, and
  `dsh_battle_radius.script` returns captured monsters to a home radius; neither
  implements the observed dog cover behavior.
- `psy_dog_phantom` can be created server-side and produces sound, but was
  invisible in testing. Its rare spawn path is retained but disabled; regular
  scripted phantoms remain enabled.
