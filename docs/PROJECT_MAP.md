# Project map

## Priority locations

- `gamedata/scripts` — gameplay scripts and integration logic.
- `gamedata/config` — configuration files. The default-scope name supplied as `gamedata/configs` does not exist in this checkout.
- `gamedata/scripts/kotovod` — common location for the user's scripts; `sivol` identifies the user's work.
- `docs/claude-notes/claude_notes_export.txt` — high-value reference archive; search selectively for the current task only.

Inspect other `gamedata` folders only when the current task requires them.

## Major systems

- Thermal anomaly effects: `gamedata/scripts/kotovod/sivol_anom_effects.script`.
  It owns Zharka/gravity anomaly lifecycle caches, proximity effects, thermal PPE,
  scripted burn ticks, and diagnostics.
- Thirst integration: `gamedata/scripts/thirst.script` consumes the thermal thirst
  multiplier exposed by `sivol_anom_effects`.
- PDA/NVD/flashlight Electra interference: `gamedata/scripts/pda_noise.script`
  maintains the shared Electra cache; `sivol_pda.script`, `params/nvd_params.script`,
  and the flashlight path consume its intensity.
- Signal registration: `gamedata/scripts/ogse/ogse_signals_addons_list.script`.
- Configuration-to-script relationships beyond these entry points: TBD.
