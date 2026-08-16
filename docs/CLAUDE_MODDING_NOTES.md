# Claude Modding Notes — NLC OGSR HE

This file distills reusable technical knowledge gathered across many modding/porting
sessions on this codebase. It's meant to complement `PROJECT MAP`, `MODDING NOTES`,
`TESTING`, and `BASELINE` — those cover current architecture and workflow; this one
is a "gotchas and confirmed facts" cheat sheet so a fresh LLM instance doesn't
re-derive things that have already been proven true or false the hard way.

Treat everything below as **confirmed by grep/source/in-game test** unless marked
"unconfirmed" or "hypothesis." When in doubt, grep the actual scripts before trusting
an assumption carried over from Anomaly/GAMMA — this is a SoC-lineage OGSR fork, not
a Monolith-engine fork, and a lot of Anomaly-isms simply don't exist here.

---

## 1. The signal system (`ogse_signals`) — read this before touching any script

This is the single dispatch mechanism for anything event-like in this fork.

- `self.am` (seen throughout `bind_stalker.script`, e.g. `self.am:call("on_actor_before_hit", ...)`)
  is **not** a separate "action manager" — it's the exact same `ogse_signals` manager
  instance that dispatches `on_update` and everything else.
- **There is no auto-discovery by function name.** A bare global function, even one
  named exactly like a signal, is never invoked on its own. `signals_mgr:call(name, ...)`
  only walks slots explicitly registered via `mgr:subscribe({signal=..., fun=...})`.
- Correct pattern for a new module:
  ```lua
  local function handler(...) ... end
  function attach(sm) sm:subscribe({signal="on_update", fun=handler}) end
  module.attach = attach
  ```
  Then add the module's filename (no extension) to `ogse_signals_addons_list.addons`
  — under the `-- [HE-PORT]` section for custom/ported modules — or `attach` never runs.
- Exception: `auto_attach = true` only works for files literally named `ogse_*.script`
  (filesystem glob), not as a general mechanism.
- **A module with no signals of its own still needs a no-op `attach(sm)`.** Missing
  `attach` throws "has no 'attach' function" when the module is listed in the addons
  table. Confirmed valid pattern (see `ballon_refill.script`) — the addons list is a
  general "load this module" registry, not purely for signal subscribers.
- **Dedup pitfall:** `subscribe()`'s dedup check compares function identity (`rawequal`).
  If a module's `init_module()`/`late_init()` creates a fresh anonymous closure each
  call (common when it's re-invoked on every quickload), the dedup check never catches
  the repeat subscription — you get silent double-firing. Fix: a local
  `module_initialized` flag, early-return if already true, rather than relying on
  the manager to dedup for you.
- **`reschedule(ms)` caution:** calling `ogse_signals.get_mgr():reschedule(ms)` inside
  an `on_update` handler is documented/assumed to throttle only that handler's own
  slot — but at least one debugging session found evidence (not fully confirmed) that
  it may throttle the *whole manager's dispatch of that signal*, silently starving
  other independently-subscribed handlers on the same signal down to the same slow
  cadence. If a second `on_update` handler mysteriously stops firing after a sibling
  module starts calling `reschedule()`, suspect this first. Safer pattern for mixed
  cadences: one `on_update` handler per module with manual `time_global()`-based
  gating inside it, no `reschedule()` call at all.
- Confirmed working signal names and their quirks:
  - `on_update` — fires every frame. Do **not** throttle it for anything driving
    `play_hud_anm`/smooth blending — throttling causes visible choppiness.
  - `on_actor_before_hit` — `(obj, hit_data, binder)`.
  - `on_actor_weapon_zoom_in`/`on_actor_weapon_zoom_out` — exist, but polling
    `wpn_params.get_zie_obj() ~= nil` directly proved more reliable for aim-state
    detection in practice than trusting these signals.
  - `on_key`/`on_key_up` — `(key, bind)`, `bind` matches `key_bindings.*`
    (e.g. `key_bindings.kCROUCH`). Prefer these over `on_update` polling for any
    crisp one-shot reaction to a state transition — polling for e.g. stance changes
    races the engine's own animation state machine and one-shot anims get silently
    dropped before rendering.
  - `on_npc_death` / `on_monster_death` — `(object, who)`, fires at actual death.
  - `on_npc_net_destroy` / `on_monster_net_destroy` — fires on object removal from
    level (e.g. corpse decay), can be well **after** actual death. Not a substitute
    for the death signals.
  - No confirmed actor-specific death signal — workaround is edge-detecting
    `db.actor:alive()` transitioning true→false inside `on_update`.
  - `on_item_to_slot`, `on_item_to_belt`, `on_item_to_ruck` — equip/belt/unequip
    transitions; good hook points for stat-recompute-on-transition instead of pure
    polling. Confirmed forwarding is in `binders/bind_stalker.script`. A proven cache
    pattern is to mark outfit/belt values dirty in these handlers (plus actor spawn
    and `on_drop_after_all`) and lazily recompute on the next consumer read.
  - `on_take_before_all` / `on_take_after_all` (and drop equivalents) — **NOT a veto
    point.** By the time these fire, the engine has already completed the pickup/drop
    natively. "before/after" only brackets this function's own extra bookkeeping, not
    the actual transfer. There is no native "before pickup, can cancel" hook in this
    engine (unlike Anomaly's `actor_on_item_before_pickup`) — confirmed dead end for
    any port that needs to veto or delay a pickup.
- Native engine "before" callbacks are relayed into this same system via
  `self.object:set_callback(callback.X, self.X, self)` → handler does
  `self.am:call("mapped_name", ...)`. Confirmed IDs (from `lua_help.script`'s
  callback const table): `on_before_save`=52, `entity_alive_before_hit`=42,
  `on_before_use_item`=41, `on_before_switch_torch`=76, `on_before_switch_nv`=77.
  **`on_before_switch_torch` is a pre-event notification, not a cancelable gate** —
  see the torch section below, the native toggle fires regardless of what the
  handler does or returns.

---

## 2. HUD animation / motion system

- **OGSR ignores the `motion_refs` field inside LTX HUD sections.** Anomaly's engine
  reads `motion_refs = <path>` from a `.ltx` section and dynamically loads that
  `.omf`; OGSR silently ignores it. Fix: embed the motion reference directly into the
  target `.ogf`'s own internal "Motion References" list, edited via an OGF editor,
  not the ltx.
- **Bone-count mismatch is a fatal crash, not a soft failure.** Embedding an
  item-only motion track (e.g. 17 bones) into the player HAND mesh's motion
  references (42-bone hand skeleton) throws `motions_value::load ... Invalid remap
  index!`. Item-only tracks go on the item's own HUD `.ogf`; hand-appropriate tracks
  go on the player HAND `.ogf`. Don't mix which mesh gets which track.
- Per-source asset pipeline (FDDA/Anomaly ports), must ALL be ported together and
  kept in their split locations:
  - `anims\itemuse_anm_effects\*.anm` — camera effector clip
  - `meshes\anomaly_weapons\hud_animation\*.omf` — item-side motion track
  - `meshes\anomaly_weapons\hud_hands_animation\*.omf` — hand-side motion track
  - `meshes\anomaly_weapons\<category>\*.ogf` — per-faction HUD mesh variants
    (these exist because visible hands/sleeves must match the actor's current
    faction/outfit skin, not because the item itself differs)
  - matching `.ogg` under `sounds\actor\`
- `anim_items.ltx` gotcha: you can freely rename the ltx SECTION name and KEY name,
  but the animation-name **values** (e.g. `liz_backpack_hud_equip_hands`) must match
  the clip names physically baked inside the ported files exactly, or you get a fatal
  `motion [X] not found in section [Y]` crash.
- `game.play_hud_motion(slot, hud_section, key, bool, speed, bool)` — arg 3 is the
  LTX **key** name, not a raw clip name. The full working signature across every
  confirmed-good call site in this codebase is `(slot, section, key, true, speed,
  true)` — args 4 and 6 both `true`. Dropping either (esp. arg 4 → `false`) correlates
  with the animation locking player movement/camera for its whole duration, which is
  usually not what you want for a partial/cosmetic overlay.
- `game.play_hud_anm(name, partial_index, speed, power, loop, additive)` is the
  **partial/independent** animation call, distinct from `play_hud_motion`'s
  full-item-swap semantics. Confirmed working, matches Anomaly's own signature.
- **Confirmed engine limitation:** there is no working way to independently animate
  one hand while the other stays locked to a currently-held weapon. Every existing
  overlay in this codebase either fully replaces the HUD model or additively layers
  onto the *same* bound model — none animate a free hand independently of an equipped
  weapon. A pickup-animation port that needed exactly this was abandoned for this
  reason after exhausting the additive-flag route.
- `dsh.timeout(delay_ms, fn)` nested calls (calling `dsh.timeout` again from inside a
  callback that is itself currently executing as part of an active `ogse_qt` timer
  pass) can throw a Lua `invalid key to 'next'` fatal — a table-mutation-during-
  iteration hazard. Doesn't always trigger (existing code nests these without issue),
  but worth suspecting if a similarly-shaped crash appears.
- Script edits require a **full game restart** to take effect — a save/load reload
  is not sufficient.

---

## 3. Native engine bindings — confirmed quirks

- `CWeapon::scope_zoom_factor`, `zoom_factor`, `iron_sight_zoom_factor`, `scope_name`
  are exported as plain **readwrite fields**, not methods — `wpn.scope_zoom_factor = x`
  is valid directly. `GetZoomFactor()`, `IsScopeAttached()`, `IsGrenadeMode()` are
  **not** exposed to Lua at all.
- `m_fIronSightZoomFactor = m_fScopeZoomFactor` happens as a **one-time copy at
  weapon/addon-init time**, not per-ADS-press. The live ADS-press snap for unscoped
  weapons reads `iron_sight_zoom_factor`, not `scope_zoom_factor` — writing only the
  latter has zero effect on unscoped-weapon ADS FOV no matter how correctly it's
  computed/persisted. Scoped weapons (with an attached scope addon) genuinely do read
  `scope_zoom_factor` live, so the two cases need different write targets.
- `CActor::currentFOV()`: when zoomed (and the `ogse_wpn_zoom_system` feature flag is
  off, which is the confirmed state for this build), camera FOV =
  `scope_zoom_factor * 0.75`, fully independent of the player's FOV slider
  (`g_fov`, console var `"fov"`). Engine's hardcoded default `g_fov` is `67.5`.
- `game_object:get_camera_fov()` / `game_object:set_camera_fov(number)` are working
  native bindings in this build. `monster_parts.script` uses them for its harvest
  camera, and repeated `set_camera_fov()` calls can drive a smooth per-frame FOV
  transition from Lua. Writing `actor_camera(0).fov` is not an equivalent override.
- `set_pda_params(vector)` is native and takes exactly a **3-component** vector.
  Calling it with 4 args is a confirmed crash.
- `get_torch_obj(game_object*)` is a native binding — every call is a live engine
  read, not a cached Lua wrapper, so `torch.on` reflects true current state.
- `armor:get_outfit()` returns a **live mutable handle** in OGSR — writes to its
  fields persist and are reflected in a fresh `get_outfit()` call. (Contrast with the
  32-bit HE version's `outfit_params.outfit_prop()`, a raw memory-offset hack with no
  OGSR equivalent — don't try to port that approach.)
- **Critical performance trap:** do not call
  `actor:get_current_outfit_protection(hit.burn)` or its `hit.chemical_burn`
  counterpart when `actor:item_in_slot(6)` is nil. The native no-outfit path was
  measured at roughly 230 ms per call and caused repeatable periodic frame stalls.
  Guard slot 6 first and return/cache zero. With an outfit equipped, the getter is
  fast. The getter covers the outfit layer; tested powered belt-artifact immunity
  must be composed separately and stacks multiplicatively.
- `ini_file()` + `section_exist`/`line_exist` proved unreliable for finding known-good
  keys (silently returned nil with no error for real fields). The proven-reliable
  pattern is `get_float(section, param, default)` (same helper used in
  `sak_minigame.script`/`rx_ai.script`).
- `get_string(section, key)` **fails hard** (script error, not nil) if the section
  doesn't exist at all — useful as a fast way to confirm whether an ltx section is
  actually being loaded/parsed, but don't call it speculatively without a guard.
- New engine functionality is exposed to Lua via a consistent 3-file convention:
  1. Declare with `_DECLARE_FUNCTIONn` (numbered by arg count, macros in
     `script_bind_macroses.h`) in `script_game_object.h`.
  2. Implement in a `script_game_object*.cpp`, using the `m_game_object` accessor
     (confirmed via `CScriptGameObject::Center()`), not `object()`.
  3. Register via `.def("name", &CScriptGameObject::Func)` in
     `script_game_object_script3.cpp`.
- `CUIStatic` alpha control: `SetTextureColor(GetARGB(a,r,g,b))` — confirmed as a
  real call pattern (via Fatal Error mod usage) and is a base X-Ray/OGSR method, not
  an addon-specific one, so it's reasonably safe to assume it's available here too
  even where not yet directly exercised in this codebase.

---

## 4. Things confirmed **absent** in this fork (don't assume Anomaly/GAMMA parity)

Porting from Anomaly/GAMMA/FDDA-style addons repeatedly hits the same wrong
assumptions. Confirmed **not present** in this SoC-lineage OGSR fork:

- **MCM** (mod configuration menu) — doesn't exist at all. Settings become plain
  script locals instead of MCM reads.
- **`lam2`** (Anomaly's action-queue manager) — not used anywhere in this codebase.
- Generic `GUI_on_show`/`GUI_on_hide` signal watching for custom windows — not a
  pattern here (see UI conventions below).
- `ray_pick()` — not available (a call to it elsewhere in the fork is dead/commented
  out, replaced by `level.get_target_obj()`).
- `CreateTimeEvent` — crashes as a nil global. Use `dsh.timeout(delay_ms, fn)`
  instead (one-shot; repeat by recursively re-calling it from the callback).
- `utils_obj.safe_bone_pos` — doesn't exist. Use native `obj:bone_position(name)`.
- `wallmarks_manager()` — zero grep hits anywhere except an unrelated hit-flag
  constant. Anomaly/GAMMA's scriptable wallmark placement class has no equivalent
  binding here; placing new wallmarks (e.g. for a blood-pool-style effect) requires
  either a new C++ engine binding or a DLL hook, not a script-only port.
  `add_StaticWallmark` (the underlying engine call, C++-only) takes no texture
  parameter — the engine picks from the bound `IWallMarkArray` itself.
- `hit.wound2` — not in this engine's hit-type enum (an Anomaly-only addition).
  Build hit-type tables defensively if porting Anomaly data.
- No global `clamp()` function — inline with `math.max`/`math.min`.
- No pre-pickup veto hook (see `on_take_before_all` note above).
- `task_smart_upd`/`AddRegularTask` — this OGSR fork does not use this scheduler at
  all (that's a 32-bit-HE-only pattern); this fork's scheduler is `ogse_signals`.
  Grepping the wrong reference codebase (32-bit HE vs this OGSR fork) has caused
  real wasted-effort bugs before — always confirm which fork a grep hit came from.

---

## 5. Torch/flashlight architecture gotcha (generalizes beyond just the torch)

This is worth internalizing as a general pattern, not just a one-off torch fix:

- The engine's **native default toggle for a device fires independently of, and
  races against, any script-driven control of the same device**, on a single
  keypress. For the torch specifically: `on_before_switch_torch` fires a real
  unpaired native toggle event roughly 425-476ms after a script-driven change
  (animation-duration-timed), always with the *opposite* status of what the script
  just set.
- **`on_before_switch_torch` (and by extension any `on_before_*` callback wired the
  same way) is a pre-event notification, not a cancelable gate.** Returning `false`
  from the handler does **not** block the native call that follows — confirmed by
  100% reproduction with `return false` active. Don't rely on `return false` as a
  veto for these callbacks.
- **Working fix pattern:** don't try to fight the native call synchronously. Instead,
  wrap any corrective/reassert logic in a small deferred `dsh.timeout(20, ...)`
  *inside* the `on_before_*` handler, so it runs *after* the native toggle has
  already applied and overwrites it back to the script's desired state. A
  synchronous correction inside the handler gets immediately clobbered by the native
  call that runs right after the handler returns.
- If two competing scripts both implement the same device (e.g. an old and a new
  torch script both registered, or one registered and one not), expect them to fight
  over the same low-level state (`set_brightness`, etc.) whenever both happen to run
  in the same frame — always check `ogse_signals_addons_list` to confirm only the
  intended script's `attach(sm)` is actually being dispatched before debugging
  further symptoms as if only one script exists.

---

## 6. UI / window conventions

- No `lam2`, no generic `GUI_on_show`/`GUI_on_hide` watching for custom windows.
  Convention here: `CUIScriptWnd`-derived window classes own their own `on_quit`,
  and anything needing to react to open/close is called **directly** from that
  window class's own methods — or via `ogse_signals` only for genuinely cross-system
  events (e.g. `on_inventory_hide`). When porting a feature that ties an
  animation/effect to a UI opening/closing, expose plain `show()`/`hide()` trigger
  functions for the window class to call directly, rather than hooking a generic
  visibility signal.
- `ui_inv_descr.script`'s `ShowProps` (native loop) prepends `"ui_inv_icon_"` to the
  icon field itself. Porting in data/strings from a source that already includes the
  full prefixed icon name will double it and silently break rendering (this exact
  mismatch caused a full UI render failure once — worth checking first if a ported
  stat-display panel renders blank).

---

## 7. Reusable design patterns worth reapplying

- **Never mutate an already-scaled/decayed stored value and reapply scaling on top
  of it.** This is the root cause of every "runaway compounding" bug seen in this
  codebase (artifact stat decay, outfit stat drift). Always recompute the *effective*
  value fresh from an immutable ini/base source on every read, never
  mutate-and-reapply to a persisted field.
- **Route all consumers of a tunable/toggleable formula through one shared core
  function**, not through independent per-call-site `* condition` math. This is what
  made the artifact-scaling system's balance toggles (buff-formula-vs-linear-decay
  for debuffs) actually take effect everywhere at once instead of needing per-site
  patches.
- **When multiple independent systems write to the same shared stat fields** (e.g.
  outfit protection stats being touched by a mask system, a rain system, and an
  upgrade-decay system all at once), expect them to silently revert each other on
  whichever system's poll/tick runs last, unless all three route through one shared
  "composer" module that resolves named per-source contributions against a cached
  true base and writes the field exactly once. Pairwise reassert patches between two
  of the three systems will not survive a third system's addition.
- **Event-driven triggers beat `on_update` polling for one-shot reactions to a state
  transition** (e.g. crouch-kick animation on stance change). Polling races the
  engine's own animation/movement state machine and one-shot effects get silently
  dropped. Track toggle direction in a local boolean rather than reading live
  engine flags (`is_actor_crouch()` etc.) inside a keypress handler — those flags lag
  behind rapid key-spam and cause every-other-press misfires.
- **Keep per-frame proximity loops allocation-free.** Cache static anomaly data from
  spawn/destroy signals, reuse persistent winner-result tables, use numeric IDs for
  state identity instead of formatting strings, and share one actor-position read
  across related scans. Short-lived tables and formatted diagnostic strings create
  GC pressure that can become periodic hitches.

---

## 8. Debugging & workflow conventions

- `log2(...)` is the standard debug-logging call used throughout this codebase.
  Avoid per-frame logging except when actively isolating a bug — remove/gate it once
  a fix is confirmed.
- Grep the actual scripts before trusting a carried-over assumption from another
  engine fork or another LLM's inference — this project has repeatedly hit real bugs
  from assuming Anomaly/GAMMA/32-bit-HE conventions apply unchanged to this 64-bit
  OGSR fork.
- Findings from a collaborating LLM (other sessions have used GLM 5.2, Kimi K3,
  Gemini, Deepseek, Grok alongside Claude) are useful but not automatically correct —
  several confirmed-wrong claims from other models were only caught by grepping the
  actual source or an in-game test contradicting the claim (e.g. a claim that
  `return false` blocks a native callback, disproven by logged evidence). Treat any
  cross-LLM suggestion as a hypothesis to verify, not a fact to build on directly.
- A crash message's exact wording is usually diagnostic gold in this engine — e.g.
  "attempt to call method 'section' (a nil value)" pinpointed passing a raw `CWeapon`
  where a `CScriptGameObject` item was expected; "Invalid remap index!" pinpointed a
  bone-count mismatch; "has no 'attach' function" pinpointed a missing no-op
  `attach(sm)`. Read these literally before hypothesizing further.
