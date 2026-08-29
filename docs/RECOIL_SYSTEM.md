# Scripted recoil system

## Ownership and files

- `gamedata/scripts/kotovod/sivol_fuzz_recoil.script` owns runtime camera and
  first-person HUD recoil, burst state, recovery, lifecycle restoration, and
  bounded diagnostics.
- `gamedata/scripts/kotovod/sivol_fuzz_recoil_profiles.script` owns global,
  class, and effective-base-section balance. Section identity follows the
  audited weapon map rather than display names.
- `gamedata/scripts/kotovod/sivol_weapon.script` supplies suppressor resolution,
  attachment mass, and the shared health/stance/outfit/artifact recoil context.
- `gamedata/scripts/ogse/ogse_signals_addons_list.script` loads the controller.
- `fl_hook/flhook.cpp` supplies the transient HUD transform API used by Lua.
- `gamedata/anims/camera_effects/sivol_onerad_006.anm` and
  `sivol_oneshove_004.anm` are optional, pre-scaled camera-effector accents.

The repository is authoritative. A matching `dinput8.dll` must be built and
deployed from the compatible `fl_hook` source for six-axis HUD recoil.

## Runtime model

Native camera recoil fields are cached, temporarily neutralized, and restored
on every controller exit path. Each accepted actor shot then contributes:

1. An immediate upward camera step.
2. A short pitch-velocity tail.
3. Restrained, correlated yaw wander.
4. Separate HUD translation, rotation, rearward shove, and critically damped
   spring motion.
5. Independent camera and HUD recovery, with faster rearward-Z recovery.

Burst continuity is short-lived and distinct from longer-lived recoil heat.
This allows isolated shots to regain first-shot behavior while long bursts gain
climb and lateral wander. The active-burst camera return scale is `0.40`.

Camera pitch uses the target build's inverted member sign: a negative
`actor_camera(0).pitch` delta produces visible upward movement. Do not change
this sign from generic X-Ray assumptions without another live direction probe.

## Global and per-weapon balance

The accepted global camera controls are:

```lua
overall = 1.00
camera_pitch = 2.90
camera_yaw = 0.72
hud_rotation = 1.12
hud_translation = 1.08
```

HUD pitch is normalized by the global camera-pitch value. Raising global
vertical camera recoil therefore does not automatically exaggerate the
viewmodel. Weapon personalities independently control pitch, yaw, first,
second, and sustained shots; recovery; first-shot floor/immediate share; heat;
yaw persistence; wander; shove; roll; and optional hip-fire response.

The P99, Glock 17, and 9x18 MP5 have explicit personalities because their live
configuration inputs are not balanced by the shared service-pistol/SMG groups.

## Composite handling context

`sivol_weapon.get_recoil_context(actor, weapon_section)` returns scalar
components without allocating a per-shot table. The recoil controller multiplies
this context with stance mode, stamina, handling, fatigue, suppressor, attachment
mass, weapon mastery, and the weapon personality.

| Component | Accepted value |
| --- | ---: |
| Health at or above 50% | `1.00` |
| Health below 50% | Quadratic increase up to `1.20` |
| Standing | `1.00` |
| Crouching | `0.92` |
| Low crouch/creep | `0.84` |
| Functional exoskeleton at full condition | `0.88` |
| Broken exoskeleton at full condition | `0.96` |
| Eye artifact | `1.15` |
| Stabilizing Gravi tier | normally `0.95` |
| Exact `af_gravi_dyn4d` override | `0.90` |

Weapon mastery reuses `sivol_weapon.get_mastery_k()` and its existing audited
class mapping. The same day curve already used by wobble now affects recoil:

| Class | Day-zero factor | Mastered factor | Plateau day |
| --- | ---: | ---: | ---: |
| Pistol | `1.15` | `0.75` | 28 |
| Shotgun | `1.25` | `0.85` | 28 |
| SMG | `1.30` | `0.80` | 35 |
| Rifle | `1.40` | `0.85` | 42 |
| Sniper | `1.40` | `0.92` | 56 |
| LMG | `1.45` | `0.94` | 56 |
| Special | `1.40` | `0.92` | 49 |

Each curve reaches `1.00` on day 7, then approaches its mastered factor. Recoil
uses the resolved base weapon section so addon-generated sections retain the
correct class. `mastery_k` is included in bounded shot diagnostics.

Outfit stabilization approaches `1.00` as outfit condition falls. Artifact
recoil and the pre-existing artifact wobble value come from one condition-aware,
powered-belt scan every two seconds. Their combined recoil context is clamped to
`0.70..1.50`; the controller must not rescan the belt every shot or frame.

## HUD hook contract

Lua prefers:

```lua
fl_hud_recoil_set6(pos_x, pos_y, pos_z, pitch, yaw, roll)
fl_hud_recoil_clear()
```

and falls back to the legacy three-axis setter when necessary. Values are
absolute hook-owned contributions, not accumulating deltas. The hook validates
and clamps inputs, associates the contribution with the active HUD item, applies
it only around HUD rendering, restores the original transform afterward, and
has a watchdog. Lua sends one neutral clear when recoil settles and then stops
calling the hook while idle.

`fl_hook` also contains the optional native
`fl_on_actor_weapon_before_fire(is_gl)` callback. The current scripted recoil
uses the post-discharge `on_actor_weapon_fire` signal; no Lua definition of the
pre-fire bridge is present in the repository as of 2026-08-30.

## Diagnostics and performance

`RECOIL_DIAGNOSTICS` logs capability/preparation, tuning, the first three shots
per equipped weapon, burst summaries, recovery, camera direction guards, and
restoration failures. Shot records identify stance as `standing`, `crouch`,
`creep`, `unavailable`, or `api_error` as well as logging its multiplier. Keep
diagnostics enabled only for focused acceptance or balancing runs; synchronous
log formatting is not ordinary gameplay work.

Script size is not a useful frame-cost proxy. The important rules are:

- keep belt scans on the existing two-second cache;
- calculate composite context only on a real shot;
- avoid table allocation in the per-frame recovery path;
- clear the HUD hook once at identity rather than writing zero every frame;
- disable the optional ANM experiment first when isolating sustained-fire
  performance.

## Accepted in-game evidence

The 2026-08-30 acceptance session confirmed correct positive vertical movement,
distinct first/sustained behavior, HUD motion, P99/Glock/MP5 personalities, and
the tested equipment/artifact context. Logged examples include:

- baseline AK-103 first shot: `1.5260` degrees, context `1.000`;
- exoskeleton plus Eye: `0.880 * 1.150 = 1.012`;
- exoskeleton plus stabilizing Gravi: `0.880 * 0.950 = 0.836`;
- 27-round AK-103 burst: `9.5434` degrees peak pitch, `1.4344` degrees
  peak yaw, zero camera clamps, and zero HUD clamps;
- isolated AK-103 recovery: approximately `1.25` seconds;
- separate low-health samples reported `health_k=1.029` and `1.047`.

The new weapon personalities were reported operational and adequately
calibrated. A later explicit low-crouch test exposed that the original recoil
context called movement predicates on `db.actor`; all three shots remained at
`stance_k=1.000`. After switching to the proven `get_actor_obj()` path, the
focused retest logged low crouch at `0.840` and ordinary crouch at `0.920`.
Health, stance, outfit, artifact, camera, and HUD behavior are accepted.

## Regression checklist

After changing controller math, profiles, or the hook, test:

1. ADS and hip single shots, short bursts, and sustained automatic fire.
2. Weapon switch during recovery, holster/non-weapon item, GL mode, empty click,
   reload, misfire, save/load, level transition, death, and menu exit.
3. Standing, crouch, low crouch, low health, normal outfit, functional/broken
   exoskeleton, Eye, and representative Gravi tiers.
4. Suppressor attach/detach and attachment-mass changes without reopening the
   inventory.
5. P99, Glock 17, standard MP5, 9x18 MP5, AK-74, AK-103, one shotgun, and one
   bolt-action rifle.
6. Burst summaries for direction guards, camera/HUD clamps, and restoration
   errors. Static validation and deployment hashes do not replace this test.
