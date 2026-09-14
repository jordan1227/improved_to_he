# Watch usability corrections and battery proposal

Corrections: implemented and deployed (two files, all SHA-256 hashes matching);
validation complete; gameplay acceptance pending.
Battery: DESIGN ONLY, not implemented. No commit.

Canonical source: `C:\Games\NLC_OGSR_HE files\improved_to_he`.
Test installation: `C:\Games\NLC_OGSR_HE`.

## Corrections

- `gamedata/scripts/he_watch_display.script` saves the wheel selection immediately
  in `get_value`/`set_value` storage as `he_watch_selected_page` (0 or 1). The
  existing storage is backed by `get_stored_vars()` in `common/_g.script`.
  Hiding, re-raising, temporary unavailability and load/stop cleanup do not change
  that saved preference. It also remains selected when the wrist is visible
  outside the dedicated viewing animation. Alarm editing temporarily shows the
  normal clock/bleeding page and restores the selection on exit.
- The regular-view hint appears once per save history, for at most 10 real-time
  seconds. `he_watch_view_hint_seen` is saved when it first appears. Scrolling
  changes its text without extending its deadline. Lowering the watch or entering
  a menu dismisses it early; it does not reappear. A save made before that first
  appearance naturally still has an unused hint. Alarm-editor help is unchanged.
- `gamedata/shaders/r3/model_clock.ps` moves the decimal glyph origin from x=0.30
  to x=0.265. The atlas dot occupies tile x=99..119, unlike the centrally placed
  digits, explaining its prior collision with the following digit. The new
  position leaves a gap without moving the digits or unit suffix.

Validation: 23 real-Lua/engine-mocked persistence, hint and lifecycle checks
passed. Lua 5.1 parsing and the clock shader's SM5 compilation against reference
headers passed. Original CP1251/no-BOM/CRLF and pre-existing non-ASCII bytes were
preserved. Focused diff reviewed; `git diff --check` passed.

Exact two-file scope: [WATCH_SCROLL_FIX_files.txt](WATCH_SCROLL_FIX_files.txt).
Diff: [WATCH_SCROLL_FIX.diff](WATCH_SCROLL_FIX.diff).
Deployment hashes: [WATCH_SCROLL_FIX_deployment.csv](WATCH_SCROLL_FIX_deployment.csv).
The prior reset-on-hide behavior and repeated hints described in WATCH_SCROLL.md
are superseded by this correction.

Manual checks: switch to radiation, hide/reopen, save/load; the radiation page
must return. Open/close alarm editing; preference must survive. Hold the first
view for ten seconds, scroll during that interval, then re-raise: no repeated
hint. Check decimal spacing at 1.00, 1.05, 1.50 and 10.00, with/without alarm footer.

## Battery: source evidence

- `gamedata/textures/act/watch/bar_battery.dds` exists: 22x18 RGBA, alpha fully
  opaque. It is a dark segmented battery graphic. No `bar_battery` reference was
  found in the searched canonical shader/script/UI sources or reference shaders;
  this does not prove absence from every packed asset.
- `gamedata/scripts/xr/xr_detector.script` uses item condition as charge, subtracting
  elapsed game seconds divided by a five-day capacity (two days for upgraded
  detectors). The time source is `game.get_game_time():diffSec(...)`.
- `gamedata/scripts/pda_battery.script` also uses condition, with saved game-minute
  timestamps, reset-clock behavior, and guards against time moving backward.
- `gamedata/scripts/kotovod/dsh_drag_drop.script` has a `batt_torch` branch for
  `device_torch`/`detector` targets. It consumes the battery and sets condition to 1.
  Its generic path can defer an action 50 ms for a non-actor source, so a watch
  recharge callback must revalidate live IDs and ownership when it executes.
- `gamedata/scripts/he_watch_status.script` already centralizes shutdown, editor
  abort, alarm silence, stale-wake invalidation and missed-occurrence handling.
- `gamedata/scripts/ui/ui_inv_descr.script:get_actual_cost` multiplies displayed
  non-trade value by condition; native trade prices come from the trader. Therefore
  condition-as-charge has a price/UI consequence, not merely a storage benefit.

## Recommended first battery version

1. One existing `batt_torch` restores the watch to 100%. No new battery item.
   Nominal capacity: **six game days = 8,640 game minutes**, configurable later.
   Charge decreases while the watch runs, whether raised or lowered, including
   accelerated sleep/wait time. Paused or closed-game wall-clock time does not count.
2. Use item condition as charge, consistent with detectors/PDA. The watch selected
   by the existing `db.actor:object("device_watch")` lookup is the running wristwatch;
   spares retain their own item charge. On active-item identity changes, establish
   a fresh drain timestamp instead of applying the previous watch's elapsed time.
   Do not reset condition on load. This scope avoids adding offline stash simulation.
3. New `gamedata/scripts/he_watch_battery.script` would own `refresh_charge()`,
   `get_charge()`, `is_powered()` and `recharge(battery_id, watch_id)`. Reuse the
   game-time delta pattern, clamp charge to 0..1, and flush charge/timestamp before
   saving. Reset on absent watch, changed watch identity or backward game time.
   Updating the charge must be independent of whether the editor/view is open.
4. Recharging must resolve both IDs again, require the exact `device_watch` target
   and an actor-owned battery/watch, and reject a full watch without consuming a
   battery. Synchronize pending drain before restoring charge, consume exactly
   one battery, reset the drain clock and refresh availability immediately. Put the
   watch-specific action in the existing battery branch; retain other device actions.
5. Make `he_watch_status.refresh()` consult the battery adapter synchronously, so
   an empty saved watch is unavailable from the first alarm/view query after load.
   Combine battery power with the existing EMP predicate; recharging during an EMP
   outage must not bypass the remaining outage. Zero charge blanks the complete
   electronic face and silences/cancels alarm output through the existing guards.
   The configured alarm remains saved; missed occurrences do not ring afterward.
6. Low charge: below **15%**, steady amber; below **5%**, a slow red brightness pulse.
   No extra sound. At six-day capacity, these thresholds leave roughly 21.6 and
   7.2 game hours. At zero, even the battery graphic is dark; the inventory charge
   description should identify the empty battery instead of drawing on an unpowered
   face. Add a watch-specific localized charge percentage in the item description.

Condition storage is the simplest compatible proposal, but its condition-based
value display/trading behavior must be accepted and tested before implementation.
If battery charge must not affect watch resale value, select separate per-item
charge storage instead; that requires a further identity/persistence design and
should not be hidden behind a cosmetic price-label change.

## Battery graphic proposal

Reuse `bar_battery.dds` in the lower-left portion of the clock surface. Proposed
raw clock UV origin **(0.20, 0.82)**, size **(0.1067, 0.16)**. The physical surface
aspect correction is 1.83288, so this preserves the asset's 22:18 aspect ratio.
All four corners were checked inside the no-outfit clock mesh. It sits left of
the saved-alarm footer; actual readability and spacing remain runtime-pending.

Bind `s_he_battery` in `gamedata/shaders/r3/model_clock.s`, sample it in
`model_clock.ps`, and register `m_he_watch_battery` in `gamedata/config/system.ltx`.
Proposed components: x=charge, y=watch present, z/w reserved. The existing shader
controller would write it. Use RGB contrast as coverage, not alpha (the texture
is fully opaque); preserve the battery outline while dimming internal segments
from right to left. Boost the very dark source artwork in the shader as necessary.
Render it on both normal pages and during alarm editing, before the shared EMP
output function, without moving the accepted digits initially.

No replacement artwork is needed for a first in-game trial. The 22x18 source is
small, so enlargement/filtering may warrant a higher-resolution authored version
after checking it in the intended location.

## Battery acceptance before release

Verify six game days of drain, accelerated sleep, save/load without refill or
catch-up errors, partial-charge replacement, full-charge rejection, exactly one
battery consumed, delayed drag safety, multiple watches, transfers, empty-watch
alarm suppression, recharging during EMP, inventory charge/value behavior and
icon placement at full/15%/5%/zero. Battery changes are not part of this deployment.
