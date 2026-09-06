# Repository guidance

## Authority and scope

- Treat `C:\Games\NLC_OGSR_HE files\improved_to_he` as the canonical editable source. Edit the repository, never the runnable game installation as the source of truth.
- The runnable test installation is `C:\Games\NLC_OGSR_HE`.
- Default investigation scope is `gamedata/scripts` and the repository's config directory, `gamedata/config`. Inspect other `gamedata` folders only when the current task needs them.
- User-authored scripts are commonly under `gamedata/scripts/kotovod` and tagged with `sivol`, the user's username.
- Prefer existing project patterns over new frameworks or broad refactors.
- Start with `docs/README.md` and `docs/WORKFLOW.md`; use `docs/PROJECT_MAP.md`, `docs/BASELINE.md`, `docs/TESTING.md`, and `docs/VALIDATION.md` to select the smallest relevant context.

## Task modes and approval gates

- `AUDIT` / `DESIGN`: inspect and report only. Do not edit, deploy, delete, or create helpers.
- `IMPLEMENT`: edit only the canonical source after the affected owners, lifecycle, and validation are clear. Do not deploy or commit.
- `IMPLEMENT_AND_TEST`: deploy only explicitly requested changed runtime files, then perform the focused manual test matrix.
- `RELEASE`: use only after acceptance is complete and the user explicitly requests deployment and commit actions.
- Cross-module, ambiguous, asset, animation, native-hook, or binary work requires a design or compatibility report before implementation. Localized obvious edits may proceed directly when their owner and validation are established.

## Task-specific routing

- `docs/CLAUDE_MODDING_NOTES.md` is a high-value local reference. Search it selectively for the current task; never load, summarize, or process the entire archive unless explicitly requested.
- For dialogue work, read `docs/DIALOGUE_MODDING.md` first; trace the loaded dialogue root, character binding, infoportion declarations, localization keys, and runtime overrides rather than editing only a `dialogs_*.xml` file.
- For recoil or FL-hook HUD-transform work, read `docs/RECOIL_SYSTEM.md` first; preserve camera/HUD separation, hook-owned lifecycle cleanup, effective weapon-section personalities, and the cached artifact/equipment context.
- For inventory, trade, corpse-looting, or `ui_inv_descr.script` layout work, read `docs/INVENTORY_UI.md` first. Keep 16:9 overrides, native panel coordinates, adaptive `ui_brokenline` construction, and row/text/icon offsets separate.
- For weapon balance or mechanics redesign, read `docs/WEAPON_BALANCE_REDESIGN.md` as an analysis document; do not treat proposals as implemented changes.
- For legacy runtime UI/config files, follow `docs/ENCODING_AND_LINE_ENDINGS.md`: preserve Windows-1251/no-BOM encoding and existing line endings, and use explicit-encoding read/write methods when patch tooling cannot safely handle the file.

## Validation, deployment, and completion

- Before implementation or deployment, inspect the focused diff, run `git diff --check`, and run the relevant checks in `docs/VALIDATION.md`.
- When the user asks to test or deploy, copy only the relevant changed repository files to `C:\Games\NLC_OGSR_HE` and report exactly which files were deployed.
- Documentation-only, analysis-only, and setup tasks must not deploy, modify, delete, link, or mirror files in the runnable installation.
- Separate static source evidence, tool-generated evidence, deployment/hash evidence, manual runtime evidence, hypotheses, and deferred work. Static checks and hashes do not prove engine behavior.
- After implementation, show the changed-file diff. Do not commit unless the user explicitly asks.
