# Watch EMP implementation

Status: implemented and deployed; static validation complete; in-game acceptance pending.
Date: 2026-09-14. No commit.

Canonical source: `C:\Games\NLC_OGSR_HE files\improved_to_he`.
Test installation: `C:\Games\NLC_OGSR_HE`.
Exact runtime scope: [WATCH_EMP_files.txt](WATCH_EMP_files.txt).
Focused changes against the previously accepted watch implementation:
[WATCH_EMP.diff](WATCH_EMP.diff). This diff includes previously untracked files;
it is not a diff against the older Git HEAD.

## Implemented behavior

- Shared electrical/artifact/hit interference drives horizontal tearing, flicker,
  strips and brief dropouts. Ordinary interference does not disable the alarm.
- A successful existing critical event publishes its severity to the watch after
  the PDA/NVD/torch windows have been scheduled. Severity below 0.95 cannot cause
  watch blackout. At or above 0.95, the watch has a 20% conditional failure chance
  and a separate 4–8 second outage. No watch means no watch failure roll.
- A watch that survives a critical contact still receives a 2–8 second visual
  surge. Shared exposure continues to contribute independently.
- Outage suppresses the complete electronic face, including static labels,
  additive icon/ring/clock surfaces and the backlight. Physical casing and glass
  are untouched. Zero new interference is an identity operation on the accepted
  clock, footer, thirst and radiation shaders.
- Alarm editing is rejected while unavailable. Failure during raising/editing
  uses the existing abort/session cleanup and retains Use suppression until release.
- Failure stops the current alarm sound, clears the beeper count and invalidates
  the queued 800 ms wake-up. Availability gates cover the update, direct beeper,
  delayed wake and explicit alarm-time editing entry points.
- The enabled flag and saved hour/minute survive failure. An occurrence due during
  failure advances to its next future day, including midnight and multi-day sleep.
  Recovery also checks the missed occurrence before normal updates can resume.
- If the alarm slowed sleep time before failing, it restores its saved prior
  factor only when the current factor still matches the value it applied. It
  does not overwrite a subsequent change from another sleep event.
- Load/stop/actor teardown clear transient EMP state and invalidate pending sound
  and wake output. Load/stop cleanup does not write old-save alarm configuration.
  EMP timers are intentionally transient and are not persisted across loading.
- `he_watch_status.set_power_available(boolean)` is the future power gate. It is
  currently initialized true; no battery usage, AA consumption, charging, item
  condition inference or saved charge is implemented.

## Owners and contracts

- `gamedata/scripts/he_watch_status.script`: independent outage/surge state,
  operational transitions, per-frame missed-alarm handling and lifecycle reset.
- `gamedata/scripts/sivol/sivol_emp_config.script`: `WATCH_EMP` tuning.
- `gamedata/scripts/sivol/sivol_critical_malfunction.script`: hit/proximity event
  publication only. Existing device window calculations and tables are unchanged.
- `gamedata/scripts/ogse/ogse_signals_addons_list.script`: status module registration.
- `gamedata/scripts/ogsr_shaders_control.script`: single shader writer, retaining
  the existing 200 ms cadence and all accepted watch channel conventions.
- `gamedata/config/system.ltx`: required shader export `m_he_watch_effects`.
  Its x component is interference 0–1; y is unavailable 1/0; z/w are reserved.
- `gamedata/scripts/he_watch_ui.script`: editor availability and stale-input gates.
- `gamedata/scripts/vergas/vergas_watch.script`: alarm output, chronological
  guards, wake generation and owned sleep-rate restoration.
- `gamedata/scripts/allcore_hide.script`: suppress presentation beeps when dead;
  preserve physical view/animation lifecycle.
- `gamedata/shaders/r3/he_watch_common.h`: common UV and brightness functions.
  Ten additive face shaders use it, together with the deferred and emissive
  backlight shaders. Exact shader filenames are in the runtime manifest.

## Validation evidence

- Real Lua 5.1 syntax parsing passed for all eight changed/new scripts.
- Integrated real Lua 5.1 harness with engine calls mocked: 56 checks passed,
  covering threshold/chance boundaries, source publication, recovery before PDA,
  absent watch, missed occurrences, midnight/multi-day rollover, active sound,
  stale wake-up, ordinary alarm, future power gate, owned sleep-rate restoration,
  load/death cleanup and editor/input behavior.
- The existing editor regression harness also passed: normal alarm-key polling,
  minute carry, Use tap/hold, release suppression, cancel, menus and timeout.
- D3DCompiler_47 compiled all 12 pixel shaders against available reference headers
  in the non-MSAA SM5 configuration, plus two backlight lighting/detail variants:
  14 successful compilations. This is not compilation by the target engine.
- An exploratory MSAA variant fails in the unmodified reference
  `common_functions.h` (`Texture2DMS` has no `Sample`). The unchanged, previously
  accepted clock shader reproduces exactly the same error with those headers.
  No shared header was changed to hide that reference limitation. Target shader
  compilation/packed-header compatibility remains runtime-pending.
- Existing non-ASCII bytes and original newline conventions were preserved.
  Legacy files retain CP1251/no BOM/CRLF; the two existing UTF-8/LF EMP scripts
  retain their own format. New runtime files are ASCII/no BOM/CRLF.
- `git diff --check` passed. New-file whitespace and manifest scope checked.
- Before deployment, all 18 existing target files matched their pre-edit canonical
  versions exactly; the four new target paths were absent.

## Deployment

Deployed all 22 paths in `WATCH_EMP_files.txt` to the test installation; every
source/target SHA-256 matched (zero mismatches). A full game restart is required
for the new shader export. Deployment hashes are recorded separately in
`WATCH_EMP_deployment.csv`; hashes establish file identity, not runtime behavior.
No textures, models, battery files, shared EMP shader headers or unrelated configs
belong in this deployment. Review/test artifacts stay in the repository.

## In-game acceptance matrix

1. Fully restart. In a quiet location, compare the accepted clock spacing, alarm
   footer, thirst direction and ring against the previous build. No new flicker,
   dark patches or layout change should occur without interference.
2. Approach an electrical source gradually. Expect increasing distortion and
   dropouts across the electronic face. Leave: the effect should disappear after
   exposure and any contact surge expire. The alarm must still work during ordinary
   glitches, even during a momentary visual dropout.
3. Under extreme exposure, wait for a qualifying critical event. Most events
   should leave the watch operational; failure is not guaranteed. On watch failure,
   expect a fully dark electronic face for 4–8 seconds, then automatic recovery.
   Existing PDA/NVD/torch outages may continue after watch recovery.
4. During blackout, the alarm key must not open its editor. Ordinary watch viewing
   remains possible. Failure while editing/raising must cleanly lower/abort the
   watch, remove the hint, restore the weapon/input and avoid a leaked Use action.
5. Set an alarm across a failure interval, including sleeping if practical.
   Expect no beep or alarm-induced wake during outage and no catch-up on recovery.
   The saved time/indicator should return; a later eligible occurrence must work.
6. If failure begins while an alarm rings, its sound must stop. A delayed wake
   already queued by that occurrence must not execute after recovery. Sleep
   should retain its natural progress rather than stay at alarm-slowed speed.
7. Save/load, death and re-entry must leave no stuck editor/input, inherited
   transient blackout, stale ringing or queued wake from the previous session.

Diagnostic log prefix: `[he_watch_emp]`. A critical watch failure logs severity
and duration; operational transitions log false/true. Regular visual interference
does not produce per-frame logs.

Deferred: battery drain/recharging, empty-battery persistence, and regular-watch
scrollwheel bleeding/psy swapping. No commits or runtime acceptance claimed.
