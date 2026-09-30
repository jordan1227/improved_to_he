# NLC OGSR engine

Engine for **NLC Improved (HE)**: [OGSR Engine](https://github.com/OGSR/OGSR-Engine)
(Shadow of Chernobyl) plus the NLC changes.

**This folder (`engine/` in `jordan1227/improved_to_he`) is the single source of truth.**
Engine source is edited and committed here, in the same repository and the same
commits as the scripts and the built `bin_x64/xrEngine.exe`, so the whole history is
visible in one place. There is no second engine repository to work in.

- Current build: [`ENGINE_VERSION.md`](../ENGINE_VERSION.md) and the top entry of
  [`CHANGELOG_NLC.md`](../CHANGELOG_NLC.md).
- Upstream README: <https://github.com/OGSR/OGSR-Engine>.
- Longer notes and evidence: `docs/ENGINE_BUILD.md` (internal docs).

## Build (Windows, Release x64)

Prerequisites:

- Visual Studio 2022 Build Tools, MSVC v143, plus the individual component
  **C++ ATL for latest v143 build tools (x86 & x64)** (`xr_3da/splash.cpp`).
- Windows 10/11 SDK, `git` on PATH, about 10 GB free disk space.

From `engine/`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File nlc_tools\fetch_deps.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File nlc_tools\build_release.ps1 -Rebuild
```

- `fetch_deps.ps1` checks out every third-party source at a pinned commit into
  `3rd_party\Src\...` (git-ignored). Do not use upstream `Update_Components.cmd`: it
  clones moving branch heads that stop matching the project files.
- `build_release.ps1` builds into `engine\bin_x64\`, writes a log to `nlc_tools\logs\`,
  prints size and SHA-256, and copies `xrEngine.exe` + `xrEngine.pdb` into
  `nlc_tools\builds\<date>_<sha8>\` so every exe keeps its matching PDB. Omit
  `-Rebuild` for incremental builds.
- Everything a build creates is git-ignored (`engine\bin_x64`, `_LIB`, `_TEMP`, the
  dependency checkouts, LuaJIT build files, `nlc_tools\logs`, `nlc_tools\builds`);
  `git status` stays clean after a build.
- The script sets `CL=/execution-charset:windows-1251` (the projects compile with
  `/source-charset:utf-8 /we4566`; OGSR and NLC are built on Russian Windows) and clears
  `NoDefaultCurrentDirectoryInExePath`. The `dbghelp.dll` post-build copy fails without
  the SDK debugging tools; that single error is expected and tolerated.

## Deploy and test

1. Copy `engine\bin_x64\xrEngine.exe` to the repository's `bin_x64\` (plus any changed
   runtime DLL, currently `nvngx_dlss.dll`).
2. Deploy to the game install with `docs\deploy_to_test.ps1` (copies named files and
   checks SHA-256).
3. Copy the matching `xrEngine.pdb` from `engine\nlc_tools\builds\...` next to the
   installed exe, so crash logs show function names. A PDB from another build is ignored.
4. Check the version in the main menu (`nlc-...`) and the build date in the log, then
   test the change in game plus a short regression (save load, `.sq` archive content,
   weapons, knife combo, inventory/PDA, options menu).

## Conventions

- Keep NLC code in NLC files where possible (`xrGame/fl_hook.cpp`/`.h` hold the features
  ported from the old `dinput8.dll` hook). Mark edits inside upstream files with
  `// NLC:` and keep them small. This keeps upstream merges manageable.
- Do not rename existing console commands or Lua functions; gamedata depends on them.
- Engine source files are UTF-8 (some with BOM) with CRLF; keep that.
- Commit an engine change together with the game data it needs (shaders, menu rows,
  strings) and the rebuilt exe, so every commit of the repository is consistent.

## Release a build

1. Set the version text in `ogsr_engine/xrCore/xrCore.cpp` (`nlc-<ogsr>.<n>`), rebuild,
   deploy, run the in-game smoke test.
2. Add a `CHANGELOG_NLC.md` entry (version, date, upstream base, NLC changes, required
   game data, dependency changes, exe SHA-256) and update `ENGINE_VERSION.md`.
3. Commit source, `bin_x64\xrEngine.exe`, changelog and version file together.
4. Tag the commit `nlc-<ver>` and create a GitHub Release of `jordan1227/improved_to_he`
   for that tag with the exe and PDB from `nlc_tools\builds\...` attached:
   `gh release create nlc-<ver> <exe> <pdb> --repo jordan1227/improved_to_he --title nlc-<ver> --notes-file <notes>`.
   PDBs (about 200 MB) never go into git.

## Upgrading upstream OGSR

Upstream history is not part of this repository, so merges are done in a separate
local workbench clone of OGSR (for example `C:\Games\NLC_OGSR_HE files\nlc_engine`,
remote `upstream` = `https://github.com/OGSR/OGSR-Engine`) and the result comes back
here as one commit:

1. In the workbench: make its NLC branch equal to the current `engine/` contents
   (copy `engine/` over the working tree and commit), `git fetch upstream`, then
   `git merge <upstream commit>` (prefer a pinned `main` commit over a release tag;
   OGSR release notes can describe commits after the tag).
2. Resolve conflicts file by file: keep NLC behaviour, take upstream fixes; record
   decisions in `nlc_tools/UPGRADE_CONFLICTS.md`. Ask when both sides changed the same
   behaviour.
3. Re-pin `nlc_tools/fetch_deps.ps1` to match the new `Update_Components.cmd`.
4. Copy the merged tree back into `engine/` here (`git archive <branch> | tar -x` into
   an emptied `engine/`), fetch deps, rebuild.
5. Port upstream game-data changes the engine needs: shaders from
   `engine/Game/Resources_SoC_1.0006/gamedata/shaders` file by file into
   `gamedata/shaders` (keeping NLC-only and NLC-edited shaders), menu rows and strings
   for new settings. Missing shaders crash at startup (for example `temporal_resolve`
   on the 3.589 upgrade).
6. Build, deploy, full regression, then one commit with engine source, exe and game
   data, and a release as above.
