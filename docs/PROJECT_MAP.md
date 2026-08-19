# Project map

## Priority locations

- `gamedata/scripts` — gameplay scripts and integration logic.
- `gamedata/config` — configuration files. The default-scope name supplied as `gamedata/configs` does not exist in this checkout.
- `gamedata/scripts/kotovod` — common location for the user's scripts; `sivol` identifies the user's work.
- `docs/claude-notes/claude_notes_export.txt` — high-value reference archive; search selectively for the current task only.

Inspect other `gamedata` folders only when the current task requires them.

## Major systems

- Thermal anomaly effects: `gamedata/scripts/kotovod/sivol_anom_effects.script`.
  It owns Zharka, Buzz, and gravity anomaly lifecycle caches; proximity/LOS effects;
  thermal, acidic, and intoxication PPE; scripted burn/acid ticks; protection
  caching; Buzz weapon degradation; and diagnostics.
- Thirst integration: `gamedata/scripts/thirst.script` consumes the thermal thirst
  multiplier exposed by `sivol_anom_effects`.
- PDA/NVD/flashlight Electra interference: `gamedata/scripts/pda_noise.script`
  maintains the shared Electra cache; `sivol_pda.script`, `params/nvd_params.script`,
  and the flashlight path consume its intensity.
- Signal registration: `gamedata/scripts/ogse/ogse_signals_addons_list.script`.
- Actor inventory signal forwarding: `gamedata/scripts/binders/bind_stalker.script`.
- Manual weapon breath and mass handling:
  - `gamedata/scripts/kotovod/sivol_weapon.script` owns hold/toggle state,
    stamina costs, cooldown/timeout, sounds, artifact-effect hooks, and cached
    live weapon/add-on mass statistics.
  - `gamedata/scripts/params/wpn_params.script` applies mass-based ADS wobble
    and retains the final visible aim direction when the zoom effector exits.
  - `gamedata/scripts/kotovod/sivol_weapon_zoom.script` owns the temporary
    hold-breath FOV and HUD-FOV transitions.
  - `gamedata/scripts/kotovod/game_options.script` and the gameplay-options UI
    expose only the hold-versus-toggle input-mode setting.
  - `gamedata/scripts/sak/sak.script` and
    `gamedata/scripts/vergas/vergas_masks.script` suspend mask breath sounds
    and condensation respectively while breath is held.
- Mutant abilities and controller overhaul:
  - `gamedata/scripts/binders/bind_monster.script` owns the ported mutant
    abilities, controller sensed/targeted pools, cover sampling, cooldowns,
    reticle/fire reactions, gear integration, phantoms, and diagnostics.
  - `gamedata/scripts/kotovod/sivol_masks.script` exposes the read-only cached
    `psy_k` multiplier consumed by mutant psy effects.
  - `gamedata/scripts/kotovod/sivol_weapon.script` supplies the 16 ms reticle
    pressure clock, camera diversion, hold-breath interruption, stamina wobble,
    and controller trigger sound integration.
  - `gamedata/scripts/kotovod/sivol_weapon_zoom.script` resolves authored
    scope magnification for controller threat scaling.
  - `gamedata/scripts/sak/sak.script` owns reversible controller bind
    scrambling/restoration; `gamedata/scripts/arts/arc_arts.script` maintains
    condition-aware `dog_shield`; `gamedata/scripts/phantom_manager.script`
    owns scripted phantom creation and the disabled experimental psy variant.
  - Native locomotion remains owned by the engine class (`SM_CONTR`) and
    `gamedata/config/creatures/m_controller.ltx`; the scripted overhaul does
    not assign controller destinations or disable movement.
- Configuration-to-script relationships beyond these entry points: TBD.
