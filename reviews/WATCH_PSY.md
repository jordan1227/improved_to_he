# Watch psy outline

Status: implemented and deployed; in-game acceptance pending. No commit.

The existing upper HP-bar outline now shows actor psy-health independently of
the HP segments. Full psy-health preserves its original appearance exactly.
As health decreases, the visible portion recedes right-to-left, with a one-texel
soft edge, and smoothly changes from the original colour through yellow to red.
Zero psy-health removes the outline. The lower stamina frame is unchanged.
Existing EMP distortion and blackout apply after the psy rendering.

No new texture or model is required. The source is the upper frame of the
reference 1024x1024 `watch_ui.dds`, bounds x=205..883, y=469..574. A two-pixel
sampling margin covers filtered edges. Mesh UV correspondence was checked on
`wpn_hand_no_outfit` and `wpn_hand_loner_sunrise` during design.

`ogsr_shaders_control.watch_update` exports clamped `db.actor.psy_health` through
`m_he_watch_psy.x` every 200 ms. The parameter is registered in `system.ltx`.
The pixel shader replaces the original frame contribution rather than covering
it with a separate overlay, so the original outline cannot remain underneath.

The test installation uses English. Its missing English `device_watch_name`
and `device_watch_descr` entries were added to the already registered `he_watch`
string table. Inventory name: **Wristwatch**. Russian strings are unchanged.

Validation: Lua 5.1 parse, XML parse/unique local IDs, `git diff --check`, original
CP1251/no-BOM/CRLF preservation and focused diff review passed. The changed pixel
shader compiled with D3DCompiler_47 in SM5/non-MSAA against reference includes;
this is not target-engine visual proof. All four target files matched their
pre-edit canonical baseline before deployment. All four deployed SHA-256 hashes
match the source.

- Exact deployed paths: [WATCH_PSY_files.txt](WATCH_PSY_files.txt).
- Focused diff: [WATCH_PSY.diff](WATCH_PSY.diff).
- Hash proof: [WATCH_PSY_deployment.csv](WATCH_PSY_deployment.csv).

Fully restart the game for the shader export. Check full, intermediate, low and
zero psy-health: original full outline, progressively shorter coloured outline,
and no outline at zero. Check independent HP segments, unchanged stamina frame,
alarm mode, EMP/blackout, and the inventory name. No new battery behavior or
alternate/sliding display is implemented.
