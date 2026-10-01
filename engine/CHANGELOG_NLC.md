# NLC engine changelog

One entry per shipped build (tag `nlc-<ogsr>.<n>` in `jordan1227/improved_to_he`).
Newest first. The engine source lives in this repository under `engine/`, next to the
game data of the same build. The exe and its PDB are attached to the GitHub Release
of the tag in this repository.

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
