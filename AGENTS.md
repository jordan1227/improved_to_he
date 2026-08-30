# Repository guidance

- Treat `C:\Games\NLC_OGSR_HE files\improved_to_he` as the canonical editable source. Edit the repository, never the runnable game installation as the source of truth.
- The runnable test installation is `C:\Games\NLC_OGSR_HE`.
- Default investigation scope is `gamedata/scripts` and the repository's config directory, `gamedata/config`. Inspect other `gamedata` folders only when the current task needs them.
- User-authored scripts are commonly under `gamedata/scripts/kotovod` and tagged with `sivol`, the user's username.
- Prefer existing project patterns over new frameworks or broad refactors.
- `docs/CLAUDE_MODDING_NOTES.md` is a high-value local reference. Search it selectively for the current task; never load, summarize, or process the entire archive unless explicitly requested.
- For dialogue work, read `docs/DIALOGUE_MODDING.md` first; trace the loaded dialogue root, character binding, infoportion declarations, localization keys, and runtime overrides rather than editing only a `dialogs_*.xml` file.
- For recoil or FL-hook HUD-transform work, read `docs/RECOIL_SYSTEM.md` first; preserve camera/HUD separation, hook-owned lifecycle cleanup, effective weapon-section personalities, and the cached artifact/equipment context.
- For legacy runtime UI/config files, follow `docs/ENCODING_AND_LINE_ENDINGS.md`: preserve Windows-1251/no-BOM encoding and existing line endings, and use explicit-encoding read/write methods when patch tooling cannot safely handle the file.
- When the user asks to test or deploy, copy only the relevant changed repository files to `C:\Games\NLC_OGSR_HE` and report exactly which files were deployed.
- For documentation-only, analysis-only, or setup tasks, do not deploy, modify, delete, link, or mirror files in the runnable installation.
- After implementation, show the changed-file diff. Do not commit unless the user explicitly asks.
