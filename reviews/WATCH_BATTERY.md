# Watch battery implementation and test handoff

Status: implemented in canonical source and deployed for user testing. No commit.
The optional inventory right-click switch is deferred. No native engine change.

## Behavior

- `device_watch` condition is charge. One actor-owned `batt_torch` restores it to 1.
- Capacity is 518,400 game seconds (six days), in `he_watch_battery.script:CAPACITY_SECONDS`.
- Only the watch selected by `db.actor:object("device_watch")` drains. Spares do not.
- Drain continues while lowered and during EMP, and includes sleep/accelerated game time.
- Charge updates at accumulated intervals of at least 60 game seconds. Saving,
  dropping the active watch, and recharging flush the pending interval.
- Load/absence/death/active ID changes reset the transient time baseline; the saved
  item condition remains authoritative. Closed-game wall time is never charged.
  Backward game time establishes a fresh baseline, without adding charge.
- Drag the AA directly onto the watch, with BOTH in actor inventory. Missing IDs,
  foreign ownership, wrong sections and a full watch reject without consumption.
  This exact pair is handled before the old generic/deferred drag combinations;
  existing torch/detector/resonator behavior is unchanged.
- Zero charge enters the existing operational gate: black face, no alarm output or
  editing. Alarm settings remain stored, missed occurrences are skipped. A recharge
  does not bypass an active EMP outage.
- Existing `bar_battery.dds` is bound on the clock surface at raw UV (0.20,0.82),
  size (0.1067,0.16), preserving its physical aspect. RGB contrast removes its opaque
  background. Segments recede right-to-left, while the case remains. Below 15% the
  indicator is amber; below 5% it slowly pulses red (three-second cycle). No sound.
- Inventory description shows percentage and AA replacement instructions in EN/RU.
- Existing condition is not reset/migrated: an already damaged watch begins with
  that proportion of charge. Price/other item-condition consequences were accepted.

## Optional right-click switch: not implemented

The searched canonical scripts/config provide no established custom action hook.
The nearby **reference**, not proven target-build source,
`C:/Games/NLC_OGSR_HE files/OGSR-Engine-main/ogsr_engine/xrGame/ui/UIInventoryWnd3.cpp`
constructs the hardcoded native menu in `ActivatePropertiesBox()` and dispatches
fixed tags in `ProcessPropertiesBoxClicked()`. Localization strings only label
these existing actions. A reliable new switch requires further target-build hook
investigation or an explicitly designed replacement UI. No speculative callback,
replacement menu or native binary patch was introduced for this optional request.

## Tool-generated verification

- 32 battery checks using the real new Lua module and real watch status module,
  with mocked engine objects/time: capacity, sleep-sized jumps, load, backwards time,
  active selection/spares, ownership, replay/missing IDs, full rejection, consumption,
  empty-state shutdown, save/drop flushing and recharge during EMP.
- 56 existing EMP/alarm/editor regression checks with a charged-battery hardware stub.
- 23 existing screen persistence/hint/lifecycle regression checks.
- All 7 affected Lua/script shader-binding files pass the Lua 5.1 parser.
- Both localization XML files parse. CP1251/no-BOM retained; pre-existing mixed
  newline regions in legacy files retained, inserted text uses CRLF.
- `git diff --check` passes. Git emitted pre-existing safe.directory and newline
  normalization warnings; no whitespace errors.
- `model_clock.ps` compiles with D3DCompile, SM5 non-MSAA and available reference
  includes. Packed target parity and runtime compilation remain unverified.
  Existing reference-header MSAA incompatibility was not changed.

These checks are not in-game proof. `WATCH_BATTERY.diff` is the focused diff against
this pass's pre-edit snapshot, rather than all accumulated uncommitted watch work.

## Deployment

All 11 files below were checked against the runtime pre-edit baseline before copying.
Every deployed file has matching source/target SHA-256 in
`WATCH_BATTERY_deployment.csv`. Existing `bar_battery.dds` was already identical in
source/runtime and was not copied. Canonical root is
`C:/Games/NLC_OGSR_HE files/improved_to_he`; runtime root is `C:/Games/NLC_OGSR_HE`.

- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/he_watch_battery.script`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/he_watch_status.script`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/ogsr_shaders_control.script`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/ogse/ogse_signals_addons_list.script`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/kotovod/dsh_drag_drop.script`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/ui/ui_inv_descr.script`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/shaders/r3/model_clock.ps`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/shaders/r3/model_clock.s`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/config/system.ltx`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/config/text/eng/he_watch.xml`
- `C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/config/text/rus/he_watch.xml`

## Runtime acceptance still pending

Fully exit and restart the game: the new shader parameter export requires startup.

1. Inspect a charged watch on time/radiation pages, with/without an enabled alarm,
   and while editing. Confirm the small battery does not overlap digits/footer,
   retains its aspect, and has no rectangular background.
2. Confirm inventory percentage, six-day total consumption, accelerated sleep, and
   persistence after save/load and level transition. Native item serialization order
   and in-game timer delivery are not proven by the mocked save test.
3. Drag a battery onto a partly empty watch: exactly one is consumed, charge is full.
   Repeat with a full watch: no consumption. Check stacked batteries and foreign
   inventory transfers. Bring both items to actor inventory before recharging.
4. Test 15%/5% boundaries and empty. Empty means the ENTIRE electronic face is dark;
   an alarm due while empty must not ring or wake the actor, including after recharge.
5. Recharge during EMP: watch stays unavailable until the outage ends. Confirm normal
   alarm/editing resumes afterward, without a stale wake-up.
6. With two watches, only the selected watch drains; switching/removing/reacquiring
   items must not apply an old watch's time delta to the next item.

Recommendation after acceptance: tune icon readability and battery lifetime from
normal play before adding another subsystem. If the native switch is revisited,
manual OFF should stop drain, persist, and use the same alarm/blackout gate; recharging
should preserve OFF. That is a later design, not shipped behavior.
