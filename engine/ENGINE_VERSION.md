# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.157` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `7B4278DA52E677D5F7160C9998EE07CE1C5C50B59F91F91083EEFCB9E6EF167F` |
| PDB | `nlc_tools/builds/20261025_211930_7b4278da` until the GitHub Release `nlc-3.589.157` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
