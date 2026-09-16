# Regular-watch numeric radiation page and icon slide

Status: implemented and deployed; static tests complete; target-game acceptance pending.
Canonical source: `C:\Games\NLC_OGSR_HE files\improved_to_he`.
Runtime target: `C:\Games\NLC_OGSR_HE`. No commit.

## Behavior

While the ordinary watch is raised and its entry animation has finished, either
wheel direction toggles between time + bleeding and numeric radiation + psy.
The icon slides horizontally within its existing mesh over 250 ms, with smooth
acceleration/deceleration. Rapid reversal starts at the current position.
The main numeric display changes immediately; only the icon slides.

Every new raise starts on time + bleeding. Closing, menu/pause, death, loss of the
watch, conversation, load, shutdown or a functional outage clears the alternate
page and its hint. Regular-view input never edits the alarm. The alarm editor
retains wheel minutes, Shift+wheel hours and its accepted Use gesture; its clock
and bleeding icon are explicitly preserved.

An in-game non-modal hint identifies the current page and what the wheel selects.
No keybinding-menu text was lengthened. Both pages retain the radiation ring,
independent psy-health HP outline, HP/stamina/satiety/thirst displays, saved alarm
footer, and shared watch EMP/blackout effects.

Numeric radiation uses `m_he_watch_ring.x`, the actor's stored radiation value;
it is not ambient radiation per hour. Conversion follows the existing
`vergas_watch.set_radiation_textures` high-range convention: radiation * 10 R,
rounded to two decimal places, consistently across the whole range. The suffix
uses the addon's Cyrillic R (visually P) glyph. This is the game's display
convention, not a claim of calibrated real-world dosimetry.

| Actor radiation | Display |
| --- | --- |
| 0 | 0.00 R |
| 0.03 | 0.30 R |
| 0.0999 | 1.00 R (rounded) |
| 0.1 | 1.00 R |
| 0.153 | 1.53 R |
| 0.999 | 9.99 R |
| 1 | 10.00 R |

The numeric page uses the ring's existing detector-presence eligibility. Without
an eligible detector, it shows `--.-- R`, never zero. No detector battery or
working-state policy has been added or substituted for the accepted presence gate.

## Addon reuse and compatibility

Read-only donor: `C:\Games\NLC_OGSR_HE files\Addons\Часы`.
Only its `lut_digi_color.dds` artwork was copied, to the new uniquely named
`gamedata/textures/shaders/watch/he_radiation_digits.dds`.
The unchanged source SHA-256 is
`e426a144df436ed9da08052e62e637178f278d5d67745a0a9c4a7c37f1f6257c`.

The donor image is 1792x1536: 14 columns by 12 rows of 128-pixel tiles. The new
sampler uses those dimensions. The existing accepted 12x12 clock atlas and its
sampling/layout remain intact; the radiation sampler is separate.

The donor script's automatic radiation threshold switch, sleepiness write,
shared EMP overrides, inconsistent low-range scale and random near-maximum
digits were not imported. The existing `icon_thirst2.dds` is reused without
editing its artwork. Ten frames represent increasing psy damage, so the shader
selects them with `1 - m_he_watch_psy.x`.

## Owners

- `allcore_hide.get_watch_view_session()` exposes a read-only session ID only for
  the active ordinary-view phase, excluding raising/closing/alarm presentation.
- New `he_watch_display.script` owns the page, slide progress, hint and lifecycle.
  It alone writes `m_he_watch_page` (x: page 0/1; y: slide 0..1; z/w reserved).
  Writes occur on changes; stationary state does not write every frame.
- `system.ltx` exports the new parameter; the addons list registers the module.
- `he_watch_common.h` declares the parameter for consumers.
- `model_clock.ps/.s` binds the donor atlas and replaces only the main readout
  while the alternate page is selected. The saved alarm footer is unchanged.
- `model_blood_icon.ps/.s` samples either icon, clipping each in its local 0..1
  area before atlas remapping and composing a single background.
- English/Russian `he_watch.xml` supplies regular-view hints.

## Validation and deployment

- Lua 5.1 parser passed for the three scripts and two shader-material Lua files.
- English and Russian XML parsed, with unique IDs in each modified string table.
- Both changed pixel shaders compiled with D3DCompiler_47 in SM5/non-MSAA against
  available reference include headers. Target-engine compilation and appearance
  are still pending. Shared-header MSAA limitations documented in WATCH_EMP.md
  were not changed as part of this feature.
- 29 real-Lua, engine-mocked checks passed for the new page/slide lifecycle,
  including the real allcore presentation accessor and rapid scroll reversal.
- The existing 56-check EMP/alarm/editor harness passed as a regression check.
- Existing CP1251/no-BOM/CRLF and pre-existing localized content were preserved.
  The focused diff was reviewed and `git diff --check` passed.
- Before deployment, every existing runtime file in scope matched its pre-edit
  canonical baseline. New paths were absent.

Exact deployment scope is 13 files in [WATCH_SCROLL_files.txt](WATCH_SCROLL_files.txt),
including the unchanged existing psy artwork needed by the new shader binding.
All 13 files were deployed and source/target SHA-256 matched for every file.
See [WATCH_SCROLL.diff](WATCH_SCROLL.diff) for this pass's changes and
[WATCH_SCROLL_deployment.csv](WATCH_SCROLL_deployment.csv) for source/target hashes.
No review/test files belong in the runtime installation.

## Manual test checklist

Fully restart for the new shader export.

1. Raise the regular watch: original time/bleeding page and short hint. Wheel:
   radiation/psy page; another tick: time/bleeding. Try both directions and rapid
   reversal. No weapon cycling or other gameplay action should leak through.
2. Confirm icon slide direction/shape, no stretched brain, no adjacent atlas
   frames bleeding in, and no extra background rectangles. Both icons should
   retain their state-dependent artwork and flicker behavior.
3. Test zero, low, intermediate and maximum actor radiation. Check decimal point,
   suffix, leading digit at 10.00, and absence of clipping with the alarm footer
   both enabled and disabled. Compare numeric values with the table above.
4. Remove the eligible detector: dashes. Re-equip it: numeric reading returns.
5. HP segments, psy outline, radiation ring and all other surfaces must remain
   unchanged on both pages. EMP affects both; blackout blanks both.
6. Lower/re-raise: clock page. Open the alarm editor: no radiation readout, normal
   bleeding icon, existing wheel/hour carry/Use controls. Close: no stale hint.
7. Pause/menu, death, loading and an outage during a slide: no stuck hint/input
   or partial icon after recovery. New regular view starts on the clock page.

Battery drain/recharging remains deferred. With the watch visible in inventory,
the existing battery drag-and-drop route is the next useful feature to design.
