# Modding documentation index

This is the entry point for repository and gameplay-modification work. Read
`WORKFLOW.md` before editing, then use the task table below to select only the
documents relevant to the feature.

## Read first

- `WORKFLOW.md` — task modes, approval gates, evidence classes, validation,
  deployment, and completion reports.
- `PROJECT_MAP.md` — current system owners and high-value source locations.
- `BASELINE.md` — repository/test-install boundary and observed invariants.
- `TESTING.md` — current manual checks and pending acceptance work.
- `VALIDATION.md` — static checks and the limits of each kind of evidence.

## Task routing

| Task | Read next | Primary proof required |
| --- | --- | --- |
| Dialogue or quest state | `DIALOGUE_MODDING.md` | Loaded-root/reference audit plus in-game dialogue test |
| Recoil, weapon control, or FL hook | `RECOIL_SYSTEM.md`; `FL_HOOK_HUD_RECOIL_BRIEF.md` for native details | Lua/config validation, hook capability checks, and focused runtime matrix |
| Inventory, trade, corpse loot, or item description UI | `INVENTORY_UI.md` | Encoding/XML checks plus separate inventory, trade, and corpse-looting tests |
| Legacy UI/config or gameplay option | `ENCODING_AND_LINE_ENDINGS.md`; `GAMEPLAY_MENU_OPTION_WORKFLOW.md` | CP1251/BOM/line-ending checks, XML parse, and targeted validator |
| Weapon balance or mechanics redesign | `WEAPON_BALANCE_REDESIGN.md` | Effective-section/source audit before any numerical change |
| Hold breath and weapon mass | `WEAPON_BREATH_TEST_PLAN.md` | Focused acceptance matrix and evidence record |
| Asset, animation, native, or binary work | `CLAUDE_MODDING_NOTES.md` selectively; relevant system document | Compatibility/design report before implementation and runtime proof |
| Artifact source catalog | `artifact_catalog.md` and `build_artifact_catalog.ps1` | Treat as generated source-reference data, not runtime proof |

## Evidence labels

Use these labels in new notes and reports:

- **Static** — directly confirmed by current source or configuration.
- **Tool-generated** — produced by a parser, validator, catalog builder, or
  hash comparison.
- **Runtime accepted** — observed in the target runnable installation under a
  recorded test condition.
- **Runtime pending** — plausible from source but not yet accepted in game.
- **Proposal** — design direction, not an implemented change.
- **Historical** — useful prior evidence that must be checked against the
  current build before reuse.

Static checks, deployment hashes, and logs do not replace a focused in-game
test when the claim concerns engine behavior, visual layout, AI perception,
animation compatibility, or timing.

## Document maintenance

New documents should state their status, scope, canonical owner, last verified
date or build, and whether they describe current behavior, historical evidence,
or a proposal. Keep generic workflow rules in `WORKFLOW.md`; keep system facts
in the appropriate task document. Do not copy Markdown documentation into the
runnable installation.
