# Early-game PDA correction: implementation and acceptance

Latest pass: **Luna dialogue review, trader prices, post-Radar Docent upgrade, introductory PDA fate and personal shielding implemented in source**. The final section is authoritative for these changes; older conflicting prices and deferrals below are historical. No deployment or commit was performed by the assistant.

Status: IMPLEMENT COMPLETE; engine acceptance pending. Canonical source only; no deployment or commit.

## Changes

- Saved transactions wait for the spawned PDA's client object before consuming payment. Confirmed purchases continue after Esc; unconfirmed Vergas selections are discarded on exit/re-entry. Interrupted commits retain their progress and retry rather than claiming a refund.
- Vergas selection identifies each donor by its existing serial number. One selected eligible donor plus one level-one artifact, or five selected eligible donors, pays for the reflash. Unselected donors survive. Broken donors are excluded.
- Petruha uses an exact eligible TOZ inventory record, including hidden inventory storage. Payment precedes the quest-completion flag; the reward includes two vodkas.
- Sidor and Barman sell the first ordinary PDA for 15,000 RUB through dialogue. Regular trade stock is removed. Acquisition history, existing PDA/proxies, charging custody and burn state close the ordinary purchase route. Their burn dialogues refer the player to Docent; Sidor's former replacement branch was removed.
- Dialogue graphs and Russian/English text were rebuilt. Final literary acceptance remains a human review item.
- Selected messages always enter one FIFO. Delays use real seconds, survive save/load without counting time outside the game, and cannot be bypassed by a newly arriving message. Apostol's paired messages retain a seven-second separation after a backlog. Presentation failures retain the head and log a bounded diagnostic.
- Upgrade ownership strictly gates radiation and custom markers; the old radiation-chip flag alone does not qualify. Battery capacity is 168 hours base, 336 upgraded. Existing artifact shielding still determines actual critical outages; there is no additional raw Electra-proximity reception cutoff.
- A retained burnt 3D PDA uses the existing black-screen shader path. Its background controls are disabled while burnt or in a critical outage. Alarm settings and key handling are preserved; closing the alarm cannot override the screen lock.
- Read-only transaction checkpoints log statistics, rank, reputation, money, PDA identity/charge, upgrade state and alarm fields. They do not initialize or reset those systems.

## Tool evidence

- Real Lua 5.1 parser: all 14 changed/new runtime scripts passed.
- XML parsing: all 10 changed/new XML documents/fragments passed.
- Six new dialogue graphs: unique/reachable phrase IDs and alternating speakers passed. Script references, both localizations, loaded include and NPC bindings passed.
- 29 mocked Lua behavior tests passed: `docs/tests/pda_state_tests.lua`. Run from the repository root with a Lua 5.1 interpreter. This run used the existing AXR Toolset Lua 5.1 DLL in a temporary 32-bit runner.
- Legacy config/localization CP1251 round-trip, no BOM, and existing line-ending styles checked. Existing mixed endings in `sivol_pda.script` were retained.
- `git diff --check` passed. See `reviews/PDA_IMPLEMENTATION.diff` for the complete runtime diff against HEAD, including the original implementation and these corrections. It is a UTF-8 review rendering, not an encoding-safe patch to apply to runtime files.

Mocks exercise state transitions and selected UI methods; they do not prove engine callback timing, actual save serialization, native control behavior, sound playback, or visual layout.

## Test setup

Use disposable saves after the approved files have been deployed separately. Run acquisition and stash tests on both Eased (0) and Author's (3). Keep a clean save without any previous PDA acquisition for each first-acquisition route. Removing a PDA from an already-used save does not reopen the ordinary shop by design.

Do not reset statistics, reputation, quest history, alarm state or acquisition flags to manufacture acceptance. Reload the appropriate baseline save instead. Capture the transaction log and note the actual observed result for each row.

Read-only checkpoint, before and after each route:

```lua
fl he_pda_access.diagnostics("manual_before")
fl he_pda_access.diagnostics("manual_after")
fl log2("PDA queue depth=%s", tostring(he_pda_news.get_queue_depth()))
fl log2("PDA transaction=%s", tostring(he_pda_access.is_busy()))
```

Useful isolated setup commands (these deliberately alter a disposable save):

```lua
fl kotovod.give_info("bar_darklab_document_done")
fl kotovod.give_info("esc_vergas_new_gg")
fl kotovod.give_info("kvest_art_done")
fl sak.create_items("af_medusa_dyn4d", 1)
fl sak.create_items("af_fireball", 2)
fl sak.create_items("wpn_bm16_full", 1)
fl db.actor:give_money(15000)
```

The Vergas flags unlock his service conditions; they do not replace his normal introduction for narrative testing. Use legitimate looted NPC PDAs with initialized data for donor tests: a raw spawned `device_pda_npc` is not evidence of a valid serial/status. Setup commands were source-checked, not executed in the game.

## Acceptance matrix

| Test | Steps | Expected result |
| --- | --- | --- |
| Startup | New game, wait beyond 30 seconds, save/load | Starter PDA removed; first-acquisition offers available. No addon/attach error. |
| Sidor base | Have 15,000; inspect offer, exit with Esc before accepting, reopen and buy | First exit changes nothing. One PDA and exactly 15,000 deducted after confirmation; no repeat offer. |
| Barman base | Separate no-acquisition baseline; repeat purchase | Same result, with Barman's own dialogue. Both traders' ordinary trade windows contain no functional PDA. |
| Insufficient funds | Inspect either trader with less than 15,000 | Explanation/referral remains readable; payment option absent; nothing consumed. |
| Other acquisition | Acquire a real PDA through another route; revisit both traders, save/load | Neither ordinary purchase offer returns, including after later losing that PDA. |
| Esc timing | Exit immediately after final acceptance, then wait; repeat with immediate save/load where permitted | Confirmed transaction completes once. No double payment or stranded pending state. |
| Vergas one | Carry at least two eligible serials and a level-one artifact; select the second donor, confirm | Only the selected donor and one eligible artifact disappear. Other serial/data unchanged; real PDA arrives. |
| Vergas five | Carry six eligible donors plus a broken donor; select five | Exactly those five disappear; unselected/broken donors survive. No artifact payment. |
| Vergas cancellation | Select donors, Esc at selection and final-warning stages, reopen | Selection starts fresh; no consumed items, altered donor data, or busy transaction. |
| Petruha | Use his real TOZ reward branch; test TOZ in visible and hidden inventory storage | PDA option works only before first acquisition; exact TOZ removed once, two vodkas granted, quest completes. Existing reward alternatives still work. |
| Difficulty thresholds | Repeat with borderline condition items | TOZ minimum: 0.475 Eased / 0.95 Author's. Artifact minimum: 0.01 Eased / 0.5 Author's. |
| Upgrade | Before/after X-18 flag; carry two level-two artifacts; repeat with and without base PDA | Hidden before flag. Once purchased, consumes two eligible artifacts; gives/retains one fully charged PDA and persists upgraded ownership. |
| Marker tier | Double-click map with base, then upgraded PDA; save/load | Base cannot create custom markers; upgraded can. Native double-click enforcement is an explicit acceptance blocker until observed. |
| Radiation | Base, old chip flag alone, upgraded without/with an active detector | Base/old chip alone gives no radiation display; upgraded display follows existing active-detector requirements. |
| Battery | Compare discharge over the same in-game interval; charge with each existing service | Upgraded drain is half the base rate; charging custody blocks acquisition/news and return restores normal operation. |
| Stashes | Read legitimate NPC-PDA stash text on both difficulties, base and upgraded | Text and discovery remain; no automatic stash spot. Existing explicit quest-stash markers remain intact. |
| FIFO/recovery | Queue A then B without a PDA; acquire one and immediately queue C | A, B, C in order; backlog visibly marked delayed. No dropped/duplicated entries across save/load. |
| Delays/outages | Queue a seven-second message; enter dialogue, sleep, emission or critical outage before delivery | Message waits until reception is available. Real-second delay independent of game-time factor. |
| Apostol | Observe paired story tips and genuine wounded SOS with source logs | Paired messages remain seven seconds apart after backlog. Wounded SOS keeps Apostol provenance rather than substituting a relay's identity. |
| Burn | Trigger real burn function below with a working PDA; open/close it; visit both traders | Retained PDA opens black; content controls unusable; Esc/holster works. Both refer to Docent, with no ordinary replacement route. |
| Repair | Complete Docent's normal repair/replacement flow | Burn lock clears; controls recover; no duplicate PDA. Observe existing Docent article-reset semantics separately. |
| Alarm | Set alarm before acquisition/upgrade; open/close alarm during a critical outage; save/load; sleep until alarm | Stored settings persist; alarm/wake still works. Closing alarm does not unlock blacked-out content. |
| Statistics | Compare automatic before/after checkpoints and PDA statistics pages | No acquisition/upgrade-induced reset of points, rank, reputation or alarm. Genuine quest/time changes must be distinguished from resets. |

Queue probes (local test text; not a substitute for the real quest routes):

```lua
fl sak.send_tip_pda("PDA test A", "PDA test", 0, 5, "default", "no", "gray", false, nil, "manual_test")
fl sak.send_tip_pda("PDA test B", "PDA test", 7, 5, "default", "no", "gray", false, nil, "manual_test")
fl sak.send_tip_pda("PDA test C", "PDA test", 0, 5, "default", "no", "gray", false, nil, "manual_test")
```

Temporary outage and permanent burn probes, on separate disposable saves:

```lua
fl sivol_critical_malfunction.trigger_from_hit(1)
fl sak.pda_will_brock()
```

The temporary outage starts according to the existing per-device stagger and ends automatically. The second command invokes the real burn transition; reload the baseline afterward or test Docent's legitimate recovery. Simply giving `pda_is_broken` would skip part of that transition.

## Limits and deferred work

- Source only: the original matrix is partially tested; see the September 10 observations below. Full black-screen visuals target the existing 3D PDA path; legacy 2D behavior needs separate observation.
- If a log reports `retained_transaction`, record its stage/error and preserve the save. Do not manually delete the saved transaction after partial payment. Spawn rejection/timeout and changed preflight payment cancel without consuming payment; unexpected engine exceptions retain the transaction for diagnosis/retry.
- Model differentiation remains deferred; no model, texture, shader asset or `items.ltx` change was made. The upgrade currently uses saved tier state with the existing item section.
- Broader notification classification remains deferred. Future rules: all thoughts aloud and immediate interaction-failure feedback are local, never PDA-gated. Campfire markers remain a future idea.
- No native engine patch was added. Native double-click marker gating, actual UI control propagation and save/load serialization remain the most important runtime checks.

## Complete runtime file set for a future deployment

This list includes retained original implementation changes plus corrections. Deployment still requires a separate explicit instruction.

```text
gamedata/config/gameplay/character_desc_bar.xml
gamedata/config/gameplay/character_desc_escape.xml
gamedata/config/gameplay/character_desc_nlc30_spawn.xml
gamedata/config/gameplay/dialogs_artmod.xml
gamedata/config/gameplay/dialogs_escape.xml
gamedata/config/gameplay/dialogs_he_pda_access.xml
gamedata/config/gameplay/dialogs_trading.xml
gamedata/config/gameplay/sak_dalogs_esc_gar.xml
gamedata/config/misc/shop_barman/barman_trade.ltx
gamedata/config/misc/shop_sidor/trade_sidor.ltx
gamedata/config/text/eng/script_strings.xml
gamedata/config/text/rus/script_strings.xml
gamedata/scripts/amk/amk.script
gamedata/scripts/he_pda_access.script
gamedata/scripts/he_pda_market.script
gamedata/scripts/he_pda_news.script
gamedata/scripts/news/news_main.script
gamedata/scripts/ogse/ogse_signals_addons_list.script
gamedata/scripts/ogsr_shaders_control.script
gamedata/scripts/pda_battery.script
gamedata/scripts/sak/sak.script
gamedata/scripts/sak/sak_dialog.script
gamedata/scripts/sivol/sivol_pda.script
gamedata/scripts/treasure_manager.script
gamedata/scripts/ui/ui_cheat_pda.script
gamedata/scripts/ui/ui_pda_addons.script
```

## September 10 runtime follow-up

Observed in the user's test: queue test C mostly passed; upgrade transaction worked; Sidor's burn referral and black screen worked. Native interactivity, generic wounded messages and G remain pending. Model differentiation is still deferred. These observations do not accept the whole matrix.

The game log `appdata/logs/xray_hawkl_10-09-26_06-40-44.log` records `queue=0 upgrade=false charge=100` twice. It also records identical total points, rank, reputation and alarm fields before/after the upgrade (07:44:28), with PDA charge rising to 1.

The same log confirms an update/late-init race: `startup_removal_cancelled` precedes `late_init skip=legitimate_acquisition`. The corrected market callback waits for startup initialization before observing acquisitions. Retest with a fresh new game on each difficulty; this fix intentionally does not erase acquisition history in existing saves.

Vergas's donor selector now reads full current inventory: usable NPC PDAs belong to `eatable`, not `rucksack`. Regression mocks now reproduce that classification. Artifact matching accepts dyn variants and retains existing tier/condition conventions. Exact transactional payment records are retained rather than calling a helper that chooses payment anew. A poor-condition artifact now exposes a refusal explanation when it is the obstacle to the one-PDA deal.

Wolf's PDA question now precedes Thanks and disappears after the answer, persisting across saves. Sidor/Barman referral replies stay visible until the player chooses to leave. No model changes or deployment were performed in this follow-up.

Focused source diff: `reviews/PDA_RUNTIME_FIX_2026-09-10.diff`. Six runtime files changed: `he_pda_access.script`, `he_pda_market.script`, `dialogs_he_pda_access.xml`, `dialogs_escape.xml`, and the Russian/English `script_strings.xml`. Parser/XML/graph/localization/encoding checks and 29 mocked tests passed. Runtime retest of these corrections remains pending.

### Docent recovery shortcuts

For screen/input recovery only, clear the same broken flag Docent clears, then restore access:

```lua
fl db.actor:disable_info_portion("pda_is_broken")
fl level.set_actor_allow_pda(true)
```

This bypasses payment and does not simulate his entire service. It leaves any broken inventory proxy in place. To test the replacement route's effects (which also clear PDA articles):

```lua
fl sak_inventory.release_actor_items("device_pda_npc_gg", 1)
fl sak.pda_new()
```

For an already-started timed repair, the existing completion dialogue instead calls:

```lua
fl sak_dialog.clear_ready_time("time_start_docent_pda")
fl sak.add_docent_self_pda()
fl db.actor:disable_info_portion("docent_self_pda_time_start")
```

That repair function also restores the existing saved-money portion when `gg_money` is present. These are source-verified actions, not commands executed by the assistant.

### Quest-message testing

`fl sak.esc_trader_start_quest()` invokes the real Sidor follow-up, provided `esc_serious_talk` is absent. It changes quest state and can kill Nimble if his rescue condition is unmet; use the disposable development save as authorized. If necessary, first use `fl db.actor:disable_info_portion("esc_serious_talk")`. Apostol's `apostol_strelok_tips` is the two-message special-AK story sequence, not a generic wounded SOS. Only the selected Apostol wounded route was moved to the new queue; arbitrary stalker wounded messages remain part of the deferred broader classification and are not covered by the FIFO acceptance claim.

## Advanced-model implementation (following the module design discussion)

Model differentiation is now implemented in source and supersedes the earlier deferral. No deployment, engine test, archive extraction, or commit was performed. The early notes-module and local-notification changes remain DESIGN proposals, not implementation.

`device_pda_upgraded` inherits the functional PDA section and uses the existing `dev_pda` world model plus `dev_pda_hud.ogf`, with a derived HUD section. The basic model retains `dev_pda_0.ogf` / `dev_pda_0_hud.ogf`. The upgraded variant is not offered through ordinary trade.

`he_pda_model` recognizes both variants and converts existing upgraded saves after dialogue closes and the PDA is holstered. It waits for the replacement client item, preserves charge, and only then removes the original. Saved conversion state supports interrupted attachment/commit. Battery, alarm, reception, shader ownership and affected story possession/removal/return checks recognize both variants. The normal charging return and Rashpil return select the saved tier's section. This changes the PDA object ID during conversion; alarm settings, articles and statistics are not reset.

35 mocked tests passed (six model-specific cases added); real Lua 5.1 parsing, inheritance/registration and byte-format checks passed. These are not native model/UI acceptance.

Retest: buy the upgrade; close dialogue and holster the PDA, wait for `[he_pda_model] converted`, then draw it. Repeat with an already-upgraded save. Check hands/HUD alignment, buttons, glitch and burn effects, sound, retained charge, save/load, alarm, charging return, and relevant quest confiscation/return. The base PDA must keep its simple model. Native model loading and D_PDA slot behavior still require the game.

Focused review diff: `reviews/PDA_MODEL_VARIANT.diff` (UTF-8 review rendering, not a patch to apply to legacy files).

Runtime files changed for the model integration:

- `gamedata/scripts/death_manager.script`
- `gamedata/scripts/doc.script`
- `gamedata/scripts/he_pda_access.script`
- `gamedata/scripts/he_pda_market.script`
- `gamedata/scripts/he_pda_news.script`
- `gamedata/scripts/pda_battery.script`
- `gamedata/config/misc/items.ltx`
- `gamedata/config/misc/paket_classes.ltx`
- `gamedata/scripts/binders/bind_stalker.script`
- `gamedata/scripts/ogse/ogse_signals_addons_list.script`
- `gamedata/scripts/sak/sak.script`
- `gamedata/scripts/sak/sak_dialog.script`
- `gamedata/scripts/sivol/sivol_pda.script`
- `gamedata/scripts/vergas/vergas_watch.script`
- `gamedata/scripts/he_pda_model.script`

## Module installation, local feedback and 2D-fallback correction

IMPLEMENT COMPLETE in canonical source; runtime acceptance pending. No deployment/commit. The user's all.spawn edits and existing staged changes were preserved.

### Why the upgraded PDA fell back to 2D

The local engine source resolves `CInventoryOwner::GetPDA()` from inventory slot 7 (`InventoryOwner.cpp:215`). `UIGameSP.cpp:122` falls back to the 2D menu when that lookup is empty. The model transfer spawned the replacement while the old PDA occupied the slot and failed to slot the replacement after releasing the old item. The corrected transfer waits until the replacement occupies slot 7 before completing. Its regular update also repairs an already-converted upgraded PDA left in the rucksack, without displacing another slot occupant or changing the player's 3D setting.

After deploying the model fix, load the affected save, close dialogue/PDA windows, holster the PDA, and wait briefly. Draw it again. Check the log for `[he_pda_model] equipped slot=7`. Read-only diagnostic:

```lua
fl log2("PDA slot: %s", tostring(db.actor:item_in_slot(7) and db.actor:item_in_slot(7):section()))
```

Expected upgraded section: `device_pda_upgraded`. A new game or another upgrade purchase should not be needed. Native slot/model behavior still needs the user's retest; the source explanation and mocked regression tests are not engine acceptance.

### Repurposed pda_chip_rad

- Keep placing `pda_chip_rad` in all.spawn as planned; no scripted corpse injection was added.
- The existing icon/model/item class are retained. The name/description now describe a salvaged route-annotation board. Other chip variants keep their former names and behavior.
- First pickup shows one local suggestion to visit Vergas. Direct use is disabled and shows a hint; it neither consumes the chip nor grants the old radiation flag.
- Vergas's new module dialogue includes the surveying-board backstory, capabilities, price, a requirements explanation and confirmation. Sidor/Barman referrals no longer use the met-Vergas wording gate. Sidor also has an optional introductory-PDA/Strelok exchange, with no quest-state changes.
- Installation requires the chip, a working charged regular PDA, and one level-one artifact at **50% condition or higher on both difficulties**. The selected chip/artifact identities and condition are revalidated before consumption. Esc before acceptance changes nothing; after acceptance, the saved transaction completes independently.
- Installation unlocks the same custom map-annotation eligibility gate as the upgraded PDA. It does not add a separate editable notebook, double battery capacity, refill charge, or enable radiation. The upgraded PDA retains annotation eligibility automatically. Native marker enforcement remains a manual acceptance item.

On a disposable save with a legitimately acquired regular PDA:

```lua
fl sak.create_items("pda_chip_rad", 1)
fl sak.create_items("af_medusa_dyn4d", 1)
fl db.actor:object("af_medusa_dyn4d"):set_condition(0.49)
```

Check the refusal on both difficulties, then set the artifact condition to `0.5` and confirm installation. These single-object setup commands assume one matching artifact; do not use them to identify a specific specimen among duplicates. Verify that only one chip and one artifact disappear, the PDA's charge stays unchanged, annotation eligibility persists after save/load, radiation stays unavailable, and another installation is not offered. Also test direct use of the chip, pickup/drop/re-pickup, Esc before/after acceptance, and upgrading the PDA afterward.

Read-only eligibility probes:

```lua
fl log2("notes=%s upgrade=%s radiation=%s", tostring(he_pda_access.has_notes()), tostring(he_pda_access.has_upgrade()), tostring(he_pda_access.radiation_eligible()))
```

### Local feedback retest

Without a PDA, and again with a burnt/empty PDA:

1. Discover a new route: its existing local notification must appear.
2. Spawn a normal item through a path that normally reports receipt, and harvest a mutant part: existing receipt popups must appear.
3. Try antirad, medkit and bandage hotkeys with no matching item.
4. Drink from an empty flask; attempt mask cleaning with no water or no mask.
5. Test failed scope attachment/missing accumulator feedback where applicable.
6. Use the final medkit/bandage: do not show a false missing-item message for that successful use. A later failed use should show one; repeated presses are throttled.

`sak.send_tip_local` bypasses PDA reception checks. Item receipt/loss presentation in `sak.relocate_item` is also independent of PDA availability. The selected interaction-only call sites were routed locally; arbitrary radio/quest messages were not globally ungated. `misc.spawn_to` itself remains silent because it also populates NPCs/containers and internal temporary items.

### Validation and changed files

44 mocked behavior tests passed, including slot recovery, module payment, strict condition thresholds, direct-use prevention, missing-item feedback and the actual local presentation wrappers. Lua 5.1 parsing, XML/dialogue graphs and speaker alternation, script/localization/binding checks, preserved byte formats and `git diff --check` passed.

Focused review rendering: `reviews/PDA_MODULE_AND_SLOT_FIX.diff`. This pass changes 18 runtime files:

```text
gamedata/scripts/he_pda_model.script
gamedata/scripts/he_pda_access.script
gamedata/scripts/he_local_feedback.script
gamedata/scripts/ogse/ogse_signals_addons_list.script
gamedata/scripts/actor_take_item.script
gamedata/scripts/sak/sak.script
gamedata/scripts/amk/amk_mod.script
gamedata/scripts/allcore_hide.script
gamedata/scripts/binders/bind_stalker.script
gamedata/scripts/kotovod/monster_parts.script
gamedata/scripts/vergas/vergas_flask.script
gamedata/scripts/vergas/vergas_masks.script
gamedata/scripts/vergas/vergas_medical_elastic.script
gamedata/config/misc/items.ltx
gamedata/config/gameplay/dialogs_he_pda_access.xml
gamedata/config/gameplay/character_desc_nlc30_spawn.xml
gamedata/config/text/rus/script_strings.xml
gamedata/config/text/eng/script_strings.xml
```

Also changed: the mocked test harness and this document/review artifact. These are not game deployment files. Earlier PDA implementation files remain dependencies; this list is the latest correction set, not a clean-install package.

## Current pass: Luna review, Radar handover and PDA protection

Status: IMPLEMENT COMPLETE in canonical source. Static/mocked checks only; this pass is not deployed and its runtime acceptance is pending. The upgraded model itself was previously confirmed working by the user.

### Current contract and review fixes

- Sidor: 15,000 initially, 10,000 after either explicit first refusal. The discount survives leaving and loading. Esc alone does not count as selecting a refusal. Barman: 8,000, with a separate callback and acceptance text.
- Sidor says the introductory device was sent to Vergas. The optional question about Vergas uses only `esc_vergas_new_gg`, not the broader reflash eligibility predicate. Both branches return to the current price offer. The explanation remains one-time after its delivery.
- Every purchase/installation acceptance now has pending, failed AND completed NPC replies, so an update between actor acceptance and NPC response cannot leave the dialogue without an eligible continuation.
- Corrected the claim that the loose module would be returned: it stays installed. Removed the overt gameplay-rule sentence from Sidor's burn refusal and made his recurring discount wording independent of how recently the player arrived.
- Added 24 previously missing English charging-service lines for Vergas, Docent and Semyonov. The reference validator now covers PDA keys across gameplay fragments and literal script calls, including Wolf/Petruha and charging. This does not certify unrelated English localization tables.
- Before `bar_deactivate_radar_done`, Sidor's existing X-18 upgrade route remains. Afterwards he offers a sold-out/referral conversation and cannot start another upgrade transaction. A transaction already confirmed is honored.
- After Radar, a player without the upgrade can buy it from Docent for one `af_capsule`, with no condition or payload filter. `m_capsule` and the quest-specific `he_yan_af_capsule` are not substitutes for the explicitly requested section. Docent's established introduction (`bar_docent_gg_know`) must be completed.
- The new route may replace a burnt ordinary PDA directly, without an additional repair charge. It clears physical burn state/broken inventory tokens only after successful payment/delivery. It preserves break count, quest history, statistics, reputation and alarm settings. An already active repair (`docent_self_pda_time_start`), charging custody or pending burn event blocks acceptance. Existing upgraded-PDA repair/replacement continues through the established Docent service.
- Upgraded PDA has 0.05 personal EMP resistance: its interference/blackout exposure is multiplied by 0.95, and its critical-outage duration is shortened by 5%. Shared torch/NVD event eligibility, intensity and outage duration are unchanged. Ambient rain/location noise and emission-wide reception blocking are unchanged.
- Emission damage is countdown-based. At a due burn cycle, an owned upgraded PDA has a 10% chance to survive and reset that cycle. Survival does not increment `pda_breaks_count`. The existing three-failure quota, countdown ranges and difficulty modifiers remain. Already queued/forced story damage is not rerolled. This is not immunity or a ten-percentage-point reduction in a nonexistent per-emission roll.

### Repeatable static/mocked pipeline

Run from the repository in PowerShell. Both commands must finish successfully; a failed test now produces a failing process exit status.

```powershell
& 'C:/Users/hawkl/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' docs/tests/validate_pda_dialogues.py
& 'C:/Windows/SysWOW64/WindowsPowerShell/v1.0/powershell.exe' -NoProfile -ExecutionPolicy Bypass -File docs/tests/run_pda_state_tests.ps1
```

The validator checks nine graphs, references, speaker alternation, character bindings, RU/EN keys, info declarations, acceptance results, price callbacks and Lua 5.1 parsing. The behavior suite has 57 tests, including actual emission-scheduler and EMP-exposure code. These do not simulate native dialogue UI or prove visual behavior.

### Focused manual acceptance

Use separate disposable saves for first acquisition, upgrade and burn replacement. Do not clear acquisition history in a real save to make purchase options appear.

| Case | Setup/action | Expected |
| --- | --- | --- |
| Sidor first price | No prior legitimate PDA; at least 15,000. Accept immediately. | Exactly 15,000 charged, one working PDA. |
| Sidor bargaining | Same initial state. Select either first refusal, leave, save/load and return. | 10,000 remains available. Accepting charges exactly 10,000. Esc without a selected refusal leaves the initial price unchanged. |
| Intro and Vergas | Before meeting Vergas, ask both introductory-PDA questions. Follow the optional Vergas question. Repeat from a save after meeting him. | Location question appears only before `esc_vergas_new_gg`. Both paths return to the appropriate price. Delivered introduction does not repeat after load. |
| Barman | No prior legitimate PDA. Try with 7,999 and then 8,000. | Acceptance unavailable at 7,999; exactly 8,000 charged at acceptance. No Sidor discount state change. |
| Sidor upgrade cutoff | Give X-18 completion, check upgrade; then give Radar completion. | Upgrade sale before Radar, sold-out/referral afterwards, no artifact consumption from browsing. |
| Docent offer | Complete Docent introduction and Radar; no upgrade flag. Supply one `af_capsule`. | Capsule acceptance visible, one capsule consumed, upgraded PDA granted/converted, other capsules and cash untouched. |
| Docent burnt replacement | Same, but ordinary PDA burnt and broken-PDA token present. | Capsule buys the upgrade directly; physical burn/token clears, existing failure count and quest state remain. |
| Existing custody | Device is already charging or an existing Docent repair is underway. | No overlapping purchase acceptance; explanation available. |
| Esc/race/load | Esc immediately before acceptance, immediately after acceptance, and save/load after confirmation where the game permits. | Before: no payment. After: exactly one completed transaction, valid NPC reply if still talking, no duplicate reward. |
| Notes regression | Install early module with a half-condition level-one artifact. | Map annotations unlock; board is consumed; battery charge and tier remain unchanged. |
| Personal EMP | Compare ordinary and upgraded PDAs near the same Electra with identical belt artifacts/weather. | Slightly weaker PDA interference/shorter PDA outage; torch/NVD unaffected. A strong event can still black out the PDA. |
| Burn protection | Use mocked tests for probability/countdown boundaries; observe normal emissions in game. | No guarantee from a few random trials. Upgraded PDA can still burn; forced queued damage still occurs. |

Useful development commands (quest-state changes are intentional on these disposable saves):

```text
fl kotovod.give_info("bar_darklab_document_done")
fl kotovod.give_info("bar_deactivate_radar_done")
fl kotovod.give_info("bar_docent_gg_know")
fl sak.create_items("af_capsule",1)
fl he_pda_access.diagnostics("before_docent_upgrade")
fl he_pda_access.diagnostics("after_docent_upgrade")
```

Prefer an actual capsule when checking payload preservation of the unspent ones. The spawner section is sufficient for transaction eligibility tests. For forced burn testing, `fl sak.pda_will_brock()` deliberately bypasses the new survival chance; do not use it to evaluate the 10% roll. Run it once on a disposable save with a working PDA.

### Runtime files changed in this pass

```text
gamedata/scripts/he_pda_access.script
gamedata/scripts/sak/sak.script
gamedata/scripts/sivol/sivol_emp_config.script
gamedata/scripts/sivol/sivol_pda.script
gamedata/scripts/sivol/sivol_critical_malfunction.script
gamedata/config/gameplay/dialogs_he_pda_access.xml
gamedata/config/gameplay/character_desc_escape.xml
gamedata/config/gameplay/character_desc_nlc30_spawn.xml
gamedata/config/text/rus/script_strings.xml
gamedata/config/text/eng/script_strings.xml
```

This is a correction subset atop the existing PDA implementation, not a clean-install package. Review artifacts: `reviews/PDA_LUNA_REVIEW_AND_RADAR.diff` and `reviews/PDA_CURRENT_RUSSIAN_DIALOGUES.md`. No files were deployed or committed.


## September 19: upgraded PDA emission usability

Source implemented; deployment and runtime acceptance pending. Supersedes the earlier emission-wide reception-blocking rule for the upgraded tier only.

- Upgraded PDA uses the live-screen glitch shader during emissions: X=0, Y=0.20, Z=0.25, with reduced noise volume. Tune `UPGRADED_PDA_EMISSION_INTERFERENCE` in `sivol_emp_config.script` after visual acceptance. No shader asset changes.
- Its selected-message queue can deliver during emissions. Battery, custody, dialogue, broken/pending-burn and critical-outage checks still apply. Ordinary PDA and early notes module keep heavy emission static and queued messages.
- Legacy 2D PDA no longer opens its blocking noise window solely because of an emission when upgraded; other location/anomaly restrictions remain.
- Existing emission burn probability is unchanged: usability is not burn immunity.
- Runtime files: `sivol/sivol_pda.script`, `sivol/sivol_emp_config.script`, `binders/bind_stalker.script`.

Manual check: from matched disposable saves, compare ordinary and upgraded PDAs during a real emission in shelter, away from Electras. Read map/tasks and use controls; upgraded should remain readable with light/moderate glitching, ordinary should retain heavy static. Enqueue a test PDA message and confirm only upgraded delivers during the emission. Repeat with 3D PDA disabled. Check emission ending, holster/reopen and save/load; no lingering emission effect. A broken, empty or separately EMP-disabled upgraded PDA must remain unusable. Run the existing static and mocked pipeline before deployment.
