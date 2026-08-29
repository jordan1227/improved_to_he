# FL Hook: transient HUD recoil API brief

> **Implementation status (2026-08-30): completed and accepted in game.** The
> hook now exports `fl_hud_recoil_available`, the backward-compatible
> `fl_hud_recoil_set`, six-axis `fl_hud_recoil_set6`, and
> `fl_hud_recoil_clear`. It applies an owner-checked, clamped, watchdog-protected
> transform around HUD rendering and restores the original matrix afterward.
> `sivol_fuzz_recoil.script` uses the six-axis API and clears it at neutral. The
> hook also invokes the optional `fl_on_actor_weapon_before_fire(is_gl)` global;
> no Lua definition of that pre-fire bridge is currently present. This file
> preserves the original implementation brief and acceptance contract; use
> `RECOIL_SYSTEM.md` for the current system architecture and balance.

## Goal

Give `sivol_fuzz_recoil.script` a safe, additive way to move the currently
attached first-person weapon model during recoil. Camera recoil already works
through `actor_camera(0)` and must remain independent.

This is not a request to port the complete Fuzz Recoil camera-effector system.
The immediate missing feature is weapon/HUD-model recoil.

## Confirmed target behavior and limitation

The Lua recoil port is active and tested. It replaces native camera recoil,
calculates a reversible pitch/yaw contribution, and applies that contribution
to the actor camera every update.

The optional HUD path currently tries these engine methods on the active weapon:

```lua
item:set_hud_offset(vector():set(0, position_y, 0))
item:set_hud_rotate(vector2():set(rotate_x, rotate_y))
```

A live probe on `wpn_lr300_specter` produced:

```text
is_hud_item=true offset_type=nil rotate_type=nil
offset_ok=false rotate_ok=false
```

`ogse_wpn_utils.script` contains wrappers with those names, but the wrappers
only call the absent object methods. The current generated `lua_help.script`
also omits the methods. The built-in `player_hud::tune`, `HUD_POS`, `HUD_ROT`,
and `hud_adjust_*` console settings are an interactive alignment editor, not a
transient per-frame Lua recoil channel.

The pre-hook Lua prototype therefore ran with `HUD_RECOIL_ENABLED = false` and
reported camera-only capability. The implemented hook and current controller
remove this historical limitation.

## Existing hook facilities to reuse

`fl_hook/flhook.cpp` already has:

- `g_player_hud` at `RVA_G_PLAYER_HUD`;
- offsets for the current script HUD part/item and hands models;
- actor HUD render/update detours;
- the current active inventory item path;
- `ensure_lua_api()`, which registers C closures as Lua globals;
- executable-version-specific RVAs and byte checks;
- engine logging through `Msg`;
- console diagnostic-command handling.

Use the same version guards and Lua-state re-registration pattern as
`fl_step_material`.

## Recommended API

Prefer hook-owned Lua globals instead of modifying the engine's luabind
`game_object` metatable:

```lua
fl_hud_recoil_available() -> boolean
fl_hud_recoil_set(position_y_m, pitch_rad, yaw_rad) -> boolean
fl_hud_recoil_clear() -> boolean
```

The minimal version can omit `available()` if the Lua script can check
`type(fl_hud_recoil_set) == "function"` and probe a neutral write.

### Contract

- `set` specifies the complete current recoil contribution, not a delta.
- Repeating the same call is idempotent; values must never accumulate.
- Position is metres and rotation is radians at the Lua boundary.
- The contribution is additive to the engine-computed weapon HUD transform.
- It must not edit weapon LTX values, `user.ltx`, `hud_adjust_mode`, or the
  persistent alignment edited by `player_hud::tune`.
- It acts only on the current first-person attached weapon/HUD item.
- If no valid actor, HUD, or attached item exists, it clears its state and
  returns false.
- `clear` removes only the hook's contribution and is safe to call repeatedly.
- Camera state must never be modified by these functions.

The existing Lua calculation currently supplies approximately:

```lua
position_y = recoil_pitch * 0.008 * class_scale
pitch_rad  = recoil_pitch * class.hud_pitch_scale
yaw_rad    = recoil_yaw   * class.hud_yaw_scale
```

## Where to apply the transform

Find the final first-person item transform update used by the currently
attached `attachable_hud_item` or equivalent. Apply the hook-owned transform
after the engine has composed base hip/ADS/addon/inertia offsets, but before the
weapon model is rendered.

Conceptually:

```cpp
final_hud_transform = final_hud_transform * transient_recoil_transform;
```

Confirm the engine's matrix multiplication order rather than copying this
expression literally. The desired local behavior is:

- negative recoil pitch raises/tilts the weapon consistently with camera kick;
- yaw moves the weapon locally left/right;
- ADS alignment, alternate aim, grenade-launcher mode, scopes, lean and weapon
  inertia remain intact beneath the additive contribution.

Do not implement this by repeatedly calling `player_hud::tune` or changing its
stored HUD position/rotation fields. Those are calibration state and can drift
or leak between weapons.

## Safety requirements

1. Validate every Lua number with `isfinite`.
2. Clamp the contribution before storing it. Suggested initial hard limits:
   `position_y` +/-0.05 m and each rotation +/-0.25 rad.
3. Store an absolute hook-owned contribution initialized to identity/zero.
4. Clear on attached-item pointer change, actor/HUD disappearance, level exit,
   Lua-state recreation, and explicit Lua clear.
5. Add a short watchdog (about 250 ms of active game time) so a Lua exception
   cannot leave the weapon displaced. Do not let pause/menu wall time cause a
   false permanent offset.
6. Refuse to apply when the HUD item does not correspond to the actor's active
   weapon, unless the engine's attachment lifecycle proves that comparison is
   unreliable during normal draw/holster transitions.
7. Keep all writes on the game/render thread already used by the HUD hook.
8. Preserve the existing hook's RVA/byte-signature guards. If the expected
   code or layout is absent, log once, leave the feature unavailable, and do
   not patch.
9. The transform must never be serialized.
10. Removing or replacing `dinput8.dll` must restore completely vanilla HUD
    behavior without requiring config cleanup.

## Diagnostics for the hook developer

Add a console command such as `hudrc`:

```text
hudrc
```

Suggested output:

```text
[fl][hudrc] bound=1 player_hud=... attached_item=... active_item=...
enabled=1 pos_y=... pitch=... yaw=... age_ms=... applied_frames=...
clears=... rejects_nohud=... rejects_item_mismatch=... rejects_invalid=...
```

Also log once when the Lua globals are registered and once when the feature is
unavailable because a signature/layout check failed. Avoid per-frame logging.

For an isolated native test, optionally support:

```text
hudrc test 0.003 0.03 0.02
hudrc clear
```

This lets the developer verify transform direction and restoration without
loading the Lua recoil module.

## Historical Lua integration checklist

The Lua port should be changed to:

1. Set `HUD_RECOIL_ENABLED = true`.
2. Probe `fl_hud_recoil_set(0, 0, 0)` instead of the missing item methods.
3. Call `fl_hud_recoil_set(position_y, rotate_x, rotate_y)` in
   `write_hud_recoil`.
4. Call `fl_hud_recoil_clear()` on every existing HUD/native restoration
   boundary: weapon switch, non-weapon item, GL mode, actor loss, save, error,
   module stop and shutdown.
5. Keep camera and HUD failure states independent. A hook/HUD failure must fall
   back to camera-only recoil without disabling or altering native restoration.

The existing Lua-side method-probe code can remain temporarily for comparison,
then be removed once the hook API passes acceptance.

## Acceptance matrix

### Native/API checks

- `type(fl_hud_recoil_set) == "function"` after entering a loaded game.
- Neutral set and repeated clear return safely.
- Invalid/NaN/out-of-range inputs are rejected or clamped as specified.
- `rl` or Lua VM recreation re-registers the globals and clears old state.

### Visual behavior

- PM/Fort: three ADS single shots show small weapon kick and clean recovery.
- Automatic rifle: controlled burst shows increasing weapon movement without
  transform drift.
- Hip and ADS retain their original alignment after recoil reaches zero.
- Mouse correction remains responsive.
- Suppressed and unsuppressed recoil preserve the Lua-calculated difference.

### Lifecycle and compatibility

- Switch between two firearms during recovery.
- Holster to detector/bolt/consumable and return.
- Enter and leave grenade-launcher mode.
- Reload, misfire and empty click do not leave an offset.
- Save/load, level transition, death and main-menu exit restore identity.
- Lean, sprint, crouch, alternate aim, 2D scope and 3D/PiP scope keep their
  normal base offsets.
- Existing item-use/hit/cutscene camera effectors remain unaffected.
- Temporarily disable/remove the recoil Lua module: watchdog returns the HUD to
  identity.

## Optional later work: camera-effector backend

The original Fuzz add-on uses `onerad.anm` with dynamic camera-effector factor
control and `oneshove.anm` for positional/FOV punch. This target does not expose
the required Fuzz-style `check_cam_effector` and `set_cam_effector_factor` Lua
APIs. The accepted controller instead has an on/off experiment using safely
pre-scaled `sivol_onerad_006.anm` and `sivol_oneshove_004.anm` copies with fixed
effector IDs. Procedural actor-camera recoil remains authoritative; this is not
a dynamic-factor port of the original backend.

## Follow-up API: full transient HUD transform

The first hook implementation provides vertical translation plus pitch/yaw:

```lua
fl_hud_recoil_set(position_y_m, pitch_rad, yaw_rad)
```

That is enough for a coherent first pass, but the original Fuzz HUD response
also uses lateral displacement, rearward shoulder shove and optional roll. Add
a backward-compatible six-axis entry point rather than changing the existing
function's contract:

```lua
fl_hud_recoil_set6(pos_x_m, pos_y_m, pos_z_m,
                   pitch_rad, yaw_rad, roll_rad) -> boolean
```

Use the same absolute/non-accumulating state, owner check, watchdog, render-only
application and restoration rules as `fl_hud_recoil_set`. Suggested hard
translation limits are +/-0.05 m per axis; retain the current +/-0.25 rad
rotation limits. Extend `hudrc test` and `hudrc` status output to cover all six
values.

The useful missing channel is local Z: a small negative impulse creates the
weapon's rearward shoulder shove while its faster recovery separates the model
kick from camera climb. Local X supports restrained horizontal weapon movement.
Roll should default to zero and remain subtle.

Before finalizing axis names, validate matrix order and signs with isolated
native tests for one non-zero axis at a time. Document the tested local-space
convention in `flhook.cpp`; Lua should not need weapon-specific sign swaps.

## Follow-up hook: actor weapon before-fire callback

Add a reusable pre-discharge callback for future ballistics, dispersion, ammo,
heat and weapon-condition systems. It is not required for the current
post-shot camera/HUD impulse, but it is required when Lua must alter values
before the engine reads them for the shot.

### Required timing and semantics

- Invoke once per real actor weapon discharge.
- Invoke after ammo/state/misfire checks have accepted the shot.
- Invoke before the engine reads shot dispersion and before it creates the
  projectile(s).
- Automatic fire invokes once per cartridge discharged.
- A shotgun discharge invokes once, not once per pellet.
- Empty clicks, rejected trigger attempts and misfires do not invoke it.
- Document whether grenade-launcher mode is included; preferably include it and
  let Lua subscribers filter by mode.
- The callback is observational/mutating only in its first version. Do not let
  a Lua return value cancel the shot until cancellation semantics are designed
  separately.

Hook the per-cartridge weapon-discharge site, not the lower-level per-projectile
or per-pellet bullet creation routine.

### Suggested bridge

Have the hook invoke one well-known optional Lua global:

```lua
fl_on_actor_weapon_before_fire()
```

The Lua bridge can obtain `db.actor:active_item()` at callback time and dispatch
the existing OGSE-style signal:

```lua
ogse_signals.get_mgr():call("on_actor_weapon_before_fire", item)
```

The native caller must preserve/restore the Lua stack, guard against re-entry,
skip cleanly when the VM/global is unavailable, and report callback errors
without corrupting or cancelling the engine shot. Avoid loading/compiling a Lua
chunk per shot; cache a registry reference or perform a guarded global lookup.
Reset the reference when the Lua state changes, using the same lifecycle as
`ensure_lua_api()`.

Add bounded diagnostics or a console status command with invocation count,
skipped-no-actor count, skipped-no-callback count, Lua-error count and the last
weapon/mode when practical.
