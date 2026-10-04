# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.19` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `F3B6C843EA4ED42781E9614C71920A18526335D7CBFDE8992A9AB6D0837AB60D` |
| PDB | `nlc_tools/builds/20261004_160008_f3b6c843` until the GitHub Release `nlc-3.589.19` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
