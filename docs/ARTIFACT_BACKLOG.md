# Artifact backlog and Unknown persistence investigation

Updated: 2026-09-08. Tracking document; implementation is complete. Core Unknown runtime behavior is accepted from user-reported tests. Transition-autosave world generation and genuinely legacy magic-box migration remain pending; balance remains observational.

Canonical source: `C:\Games\NLC_OGSR_HE files\improved_to_he`.
Runnable installation: `C:\Games\NLC_OGSR_HE`.
Runtime observations below are user-reported unless stated otherwise.

## Current decisions

- Pudding insulation is runtime-accepted. Its condition-scaling switch and current wear presentation are retained. Say "less wear" rather than equating wear reduction with increased service life.
- Kolobok blocks eligible mutant melee attacks. The user accepts the remaining hit screen effect with the working `zwuk_short\kolo_block` sound. Pseudogiant stomp remaining unblocked is desirable: it is a ranged shockwave.
- Spirit uses stronger HP regeneration at low health; the user reports it working. Full diagnostic lifecycle acceptance is separate from that observation.
- Cry sprint reserve/application has log evidence and user testing. Preserve the user's removal of continuous Cry hunger; do not reintroduce it incidentally.
- No new HUD charge/cooldown display is currently wanted. Consider feedback only when a future mechanic needs it.
- Ordinary artifact properties remain visible from the start. Unknown identification is a separate, deferred exception proposal.
- Unknown specimens retain per-specimen payloads and remain independently selectable through the tested save/load, level-transition, drop/pickup, artifact-container and trade paths. Treat this as accepted evidence, while keeping transition-autosave world generation and genuinely legacy magic-box migration pending.
- The current generated Unknown distribution is accepted without numerical retuning: composition frequencies, weight-scaled magnitude, specialist selection, approved exception pairs and deterministic sampling all match the current direction. Healthy roll frequencies do not prove that heavy specialists feel worthwhile over a long playthrough.
- Pause Unknown feature expansion for now. Observe whether lightweight specimens become automatic choices, heavy specialists justify their carrying burden, and cooking remains an attractive alternative to equipping them.

## Work queue

| Priority | Work | Status / next action |
| --- | --- | --- |
| 1 | Persistent per-specimen Unknown properties | Implemented and accepted for the tested persistence/selection paths; transition-autosave world generation and genuinely legacy magic-box migration remain pending |
| 2 | Current generated Unknown profile system | Bounded compositions, ordinary/specialist properties, weight scaling, exception filtering and stable transfer metadata are implemented and runtime-accepted; observe long-play balance |
| 3 | Expand Unknown profile pool | Paused while observing the current version; no numerical retuning or feature expansion yet |
| Later | Paid Unknown identification by Saharov or Docent | Hidden but active properties before identification; no dialogue work yet |
| Later | Unique-artifact candidates for O2 extension | Locate actual oxygen resource/consumer, audit candidate ownership and acquisition |
| Later | Radiation-cleansing costs/efficiency; Flower/Star thirst | Parked concepts; no approved allocation or numerical design |
| Later | Reuse Kolobok-style protection for selected uniques | Preserve family roles; choose eligibility/stacking deliberately |
| Parked | Health/psy conversion | Distinct from implemented Spirit low-health healing |
| Parked | Escaping artifacts / pickup phantoms | Flavor concepts; no approved implementation |
| Parked | Blue Electra acquisition/encounters | Trace exact IDs and existing behavior before redesign |
| Parked | Extra Babka first-shot reduction | Alternative to stronger general recoil benefit, not automatically additive |
| No current work | General outfit wear comparison redesign; extra HUD meters | Current presentation and shield sound satisfy the user |

## Unique candidate register

Candidate status does not mean the artifact has no existing scripted ability.

| Artifact | Established role / user correction | Possible future direction |
| --- | --- | --- |
| `af_crist_new` / Призрачный кристалл | Radiation removal, stamina drain, heat vulnerability; accepted candidate | Radiation-cleansing specialist; other narrow utility after owner audit |
| `af_electra_blue_new` / Лунный блик | Accepted candidate; existing behavior/acquisition still need complete audit | Psy/environmental utility; O2 only if its actual mechanics and lore fit |
| `af_sul_new` | Extremely rare, high-end cooked apex mule artifact; recipe/ingredients can be out of reach for a playthrough | Optional complementary utility, not a vacant role or priority rebalance |
| `af_baloon` | Already anti-bloodsucker; relevant behavior in `bind_monster.script` | Not an unassigned O2 artifact merely because of its name |
| `af_medus_new` | Already anti-cat; relevant behavior in `bind_monster.script` | Preserve existing identity |

Before assigning any unique: inspect effective config, localized description, `bind_monster.script`, other exact-ID script consumers, acquisition, recipes and exchanges. Never infer emptiness from config alone. Distinguish rare player-facing `_dummy_` artifacts from technical proxies.

## Unknown first-version contract

- Preserve actual `af_unknown_*` section identities, existing aliases, weights, prices, acquisition, quests, exchanges and cooking roles.
- Roll a bounded authored profile once per specimen, not every stat independently. Begin with properties already supported by the artifact runtime stat writer.
- Store resolved property values plus schema/profile version. A seed may accompany them, but future table changes must not silently reroll old specimens.
- No reroll on inspection, manual save/load, autosave/load, level transition, drop/pickup, trade, container insertion/extraction or object recreation.
- Specimen identity must survive replacement of engine object IDs. Do not key permanent identity solely by a reusable ID.
- Keep condition separate from rolled baseline values. Reapply from clean specimen baselines so load/reset cannot compound modifiers.
- UI and gameplay use the same payload interpretation. Previewing properties must not consume RNG or mutate gameplay.
- First version shows properties immediately. Identification is deferred; optional future metadata must not affect the first version's behavior.
- Preserve existing unrelated custom metadata, including capsule blocks, `xp={...}` data and logic data.

## Static investigation: persistence owners

Paths below are relative to canonical source.

| Owner | Verified source behavior | Design implication |
| --- | --- | --- |
| `gamedata/scripts/nlc_capsules.script` | `ensure_server()` reads existing capsule payload before choosing contents; `write_server()` updates server `custom_data` through `get_netpk` | Reuse the persistence pattern with a separate Unknown namespace, not capsule semantics |
| `gamedata/scripts/binders/bind_det_arts.script` | `net_spawn()` initializes capsule metadata then calls `scale_artefact_stats(item, true)` | Candidate fallback initialization/rehydration hook; must preserve rolled baselines through reset |
| `gamedata/scripts/dsh.script` | `get_next_random()` maintains an RNG queue in `get_value`/`set_value` | Existing saved-state random selection pattern; insufficient alone for object identity or pre-initialization autosaves |
| `gamedata/scripts/he_stor.script` | Reads/writes external `$app_data_root$/nlc_he_stor.ltx`; main `[stor]` section is reset when loaded game's `he_stor_uid` differs | Do not use as sole specimen authority; it has different rollback/profile semantics from a save |
| `gamedata/scripts/kotovod/ui_arc_container_menu.script` | Records section, condition and custom data; restores custom data on extraction; existing `xp` extension stores custom properties | Good transfer foundation, but initialization must not race metadata restoration |
| `gamedata/scripts/kotovod/arc_radiation.script` | Container encoder joins fields with commas, decoder splits by commas | New nested payload must contain no raw commas unless the container codec is explicitly extended; prefer controlled semicolon-separated numeric fields |
| `gamedata/scripts/he_magic_box.script` | Preserves custom data for artifacts and restores it after spawning | Test payload preservation and spawn-before-restore ordering |
| `gamedata/scripts/common/_g.script` | `scale_artefact_stats()` rebuilds runtime properties; `get_value`/`set_value` use `xvars` | Resolve per-specimen baseline here without competing writers; trace xvars serialization before relying on campaign counters |

Unknowns are not empty collectibles: `amk/amk_anoms.script` generates numbered sections, `amk/amk_mod.script` uses section weight for a cooking bonus, `mike.script` recognizes fixed Unknown sets, and `doc.script` gives/takes specific sections. Retain these contracts.

## Autosave design and runtime acceptance matrix

Static source:

- `bind_stalker.script` registers `callback.on_before_save`, dispatches `on_before_save_game`, and separately serializes actor/module state in `save(packet)`.
- `dsh.backup_autosave()` copies an existing autosave file. It does not initialize specimen metadata.
- This does not prove every engine-generated autosave invokes every Lua preparation callback in the desired order.

Preferred design: commit a new specimen's complete payload at its creation boundary, before exposure/inspection/container transfer. Before-save callbacks may verify state but must not be the only place that rolls or writes it. On load, read the saved payload and rehydrate properties without new random draws.

### Current runtime acceptance (2026-09-08)

User-reported tests accept the following for the current implementation:

- Per-specimen payloads survive save/load, level transition, dropping and pickup.
- Same-section specimens remain independently selectable and retain their own token/profile/properties rather than behaving as one grouped roll.
- Artifact-container storage and trade close/reopen preserve the resolved payloads.
- Generator composition, weight scaling, specialist selection, approved exception compatibility and deterministic sampling without consuming specimen tokens behave as intended.
- The neutral-immunity correction and strict malformed/incomplete-payload fallback are present in the tested source/runtime version; malformed-payload behavior still deserves a direct negative-case check if encountered in play.

Still pending:

1. Transition-autosave world generation: a newly generated world specimen saved by the transition-autosave path before first client `net_spawn`.
2. Genuinely legacy magic-box migration: a pre-feature specimen already stored in a magic box, including repeated extraction/load ordering.
3. Long-play balance observation: whether lightweight Unknowns become automatic choices, heavy specialists justify their burden, and cooking remains competitive.

The accepted tests compare specimen token/schema/resolved values rather than UI appearance alone. The two pending persistence cases must retain that same comparison standard; neither should reroll or consume a new specimen token.

## Deferred identification

Desired concept: Unknown properties are active but hidden until Saharov or Docent identifies that exact specimen for a small fee. Ordinary artifacts remain unchanged.

Likely script-level components are per-specimen identification metadata, UI visibility rules and existing NPC dialogue/payment flows. No engine patch is yet justified or proven necessary. Later investigation must cover every disclosure surface (inventory/trade/corpse popup, container/resonator inspection, comparison rows), fee rollback, identity after transfer, and localisation. Display gating must never deactivate the underlying properties or reroll them.

## Next handoff boundary

### Acquisition follow-up (2026-09-08)

User clarification: Unknowns are normally found loose in the world/stashes or awarded directly, not supplied inside artifact containers. Treat prefilled artifact-container acquisition as a low-priority compatibility case; player storage after acquisition still requires payload preservation.

Verified creation routes:

- World: `amk/amk_anoms.script` -> `get_new_section()` / `spawn_rand_arts()` -> `spawn_art()` -> `amk.spawn_item()` -> `alife():create()`. The returned server object is an initialization point before the spawn helper returns.
- Stashes: `treasure_manager.script`, `CTreasure:give_treasure()`, creates ordinary items directly with `alife():create()`. `misc/treasure_manager.ltx` contains exact Unknown sections. A stash inventory is distinct from serialized artifact-container contents.
- Ordinary rewards: `sak.create_items()` -> `sak.spawn_items()` -> `misc.spawn_multi()` -> `misc.spawn_obj()` -> `g_sim:create()`. `doc.script` and `sak_dialog.script` use this route.
- Reward exceptions: `sak_dialog.script` also has direct `g_sim:create()` calls for exact Unknown sections (including a five-item group around line 7906). A generic reward-helper change alone does not cover them.
- Trader lead: `trade_manager.refresh_mikhalych_arts()` explicitly spawns Unknowns via `misc.spawn_to()`. The function exists; confirm its calling context before claiming that all such items are currently available in normal trade.

Additional preservation requirement: `sak.bar_tp_bypass()` selects an Unknown section from `r_treas_items` and discharges it while bypassing the barrier. New specimen properties must not replace section identity or alter this behavior.

Recommended initialization API shape: distinguish creation of a genuinely new specimen from restoration of an existing payload. Do not indiscriminately initialize inside a generic spawn helper used by container extraction; it may create a temporary new roll before restoring the original metadata. Prefer explicit calls at genuine acquisition boundaries plus a carefully defined legacy/online fallback.

Remaining targeted leads: engine-backed `get_stored_vars()` serialization (the local `init_xvars()` is empty), autosave ordering relative to server creation, and the separate contract for rewinding to before a specimen was created. Neither a fresh wall-clock seed nor object ID alone is an acceptable migration solution.

## Implementation status (2026-09-08)

- Added deterministic, save-persistent world artifact-generation event state with stable vertex/weight traversal, deterministic dynamic tiers and capsule contents, replacement-vertex propagation, replay records and a shared disabled diagnostic switch.
- Added `sivol_unknown_arts.script` with bounded generated compositions, ordinary and specialist properties, grades `0.8 / 1.0 / 1.2`, weight-scaled magnitude, comma-free schema payloads, creation-boundary initialization, stable legacy/container migration, strict v2 validation, malformed-payload fallback and read-only diagnostics.
- Wired world, stash, ordinary reward, direct dialogue, trader and fixed-world Unknown creation paths, plus binder, artifact-container and magic-box restoration.
- Routed gameplay/stat/UI/radiation consumers through the same resolved nominal-property owner while preserving condition scaling and existing custom-property precedence.
- Static validation and source review are complete for the current pass. User-reported runtime evidence accepts deterministic sampling, profile persistence, independent selection, tested transfers and the current distribution. No balance retuning is planned now. Transition-autosave world generation, genuinely legacy magic-box migration and longer-play balance observation remain open; no deployment or commit is implied by this tracking update.
