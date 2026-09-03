# Inventory UI guidance

This document records the confirmed layout and implementation rules for the
item description UI shared by player inventory, trade, corpse looting, and the
standalone item popup.

## Source and runtime files

- `gamedata/scripts/ui/ui_inv_descr.script` builds the scripted popup and the
  supplemental bottom-panel headers.
- `gamedata/config/ui/ui_inv_descr.xml` and `ui_inv_descr_16.xml` define the
  scripted controls. The engine selects the `_16` variant for widescreen.
- `inventory_item*.xml`, `trade_item*.xml`, and `carbody_item*.xml` define the
  native item-description controls for inventory, trade, and corpse looting.
- Their containing panels are not named consistently: player inventory and
  corpse looting use `descr_static`, while trade uses `desc_static`. Inventory
  and trade expose dedicated level window getters; the corpse script uses the
  current main input receiver as its native window. Do not assume one lookup
  path works in every context.

The repository is the editable source. Deploy only changed runtime files to
`C:\Games\NLC_OGSR_HE` and compare source/target SHA-256 hashes.

## Coordinates and widescreen scaling

Authored UI coordinates are logical units, not guaranteed physical screen
pixels. During 16:9 testing in this project, small horizontal changes commonly
rendered at about twice their authored value. Treat this as an observed tuning
ratio, not a universal engine constant.

- Make 16:9-only corrections in the `_16.xml` variant whenever possible.
- Measure both left and right margins. To center an element, move it by half
  the difference between those margins, converted back to logical units.
- Keep the line and its text on the same center. If the visible pair is offset,
  move both together; use a line-only offset only when the line is demonstrably
  displaced relative to the text.
- Keep scroll-view bounds independent from title/header bounds. Narrowing a
  description list should trim its overflowing edge without moving unrelated
  weight, price, or condition controls.
- Static XML and hash checks cannot prove visual alignment; verify in game at
  the target aspect ratio.

## Adaptive item-name broken line

Use the same two-control pattern as the working popup header:

1. A blank `CUILabel` owns `<texture>ui_brokenline</texture>`.
2. A separate `CUIStatic` owns and draws the item-name text.
3. Measure the text with `AdjustWidthToText()`.
4. Add the intended side padding, clamp to the panel's maximum width, and
   calculate one centered rectangle.
5. Apply that rectangle to the text and use `SetWidth()` for the broken-line
   label. In this UI, `SetWndSize()` did not reliably resize the visible line.

Do not initialize `ui_brokenline` as a regular `CUIStatic` texture; that
produced the engine's purple missing-texture square. Do not write the item name
to both a `CUILabel` and its `GetTitleStatic()` child; that draws duplicate item
names. The line label must remain textless.

The supplemental inventory, trade, and corpse-looting title is updated from the
click-selected item, not from hover focus. Engine-managed native titles can
otherwise redraw on hover and compete with the scripted title. The old
`carbody_item*.xml` three-piece `ui_brokenline_b/back/e` assembly is not the
adaptive header; keep it removed when the scripted body header is active.

In the 16:9 corpse panel, move the description start together with vertical
title corrections and reduce its height by the same amount to preserve the
bottom boundary. The outer `ui_st_description` caption is intentionally hidden
because the centered adaptive item title already identifies the panel. Corpse
condition progress uses explicit red minimum, yellow midpoint, and green
maximum colors over the black `ui_mg_progress_efficiency_empty` track.

## Property rows and icons

`ShowPropFromIni` accepts independent row and icon offsets. Preserve this
separation:

- `x_offset` moves the complete row, including its text.
- `icon_x_offset` moves only the icon inside that row.

Ammo and compatible attachment rows currently use `x_offset = -3` and
`icon_x_offset = -5`. At the tested 16:9 scale, the icon-only adjustment is
approximately nine rendered pixels left. Do not change the row offset when the
request concerns only icon alignment.

`ShowProps` normalizes shorthand icon names through `get_property_icon`; the
current helper preserves names already beginning with `ui_inv_icon_` and adds
that prefix only to shorthand names. `ShowPropFromIni` follows a different
section-icon path through `CIconParams`, so verify which renderer consumes a
row before changing an icon key.

## Safe edit and verification checklist

These legacy UI XML and Lua files are normally Windows-1251 without a BOM and
use CRLF. Follow `docs/ENCODING_AND_LINE_ENDINGS.md`.

After an edit:

1. Run `git diff --check` on the affected files.
2. Parse every changed XML file.
3. Verify Windows-1251, BOM state, and line endings.
4. Count the intended call sites for shared Lua signature changes.
5. Inspect the focused diff for accidental coordinate or formatting changes.
6. Deploy only changed runtime files and compare SHA-256 hashes.
7. Fully restart the game after script or XML changes and test inventory,
   trade, and corpse-looting contexts separately.
