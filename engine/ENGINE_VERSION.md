# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.150` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `6223D9A9B5DA199049E33D25FD122771CA399AB381A9204CAD3AD2AB36C78332` |
| PDB | `nlc_tools/builds/20261024_003319_6223d9a9` until the GitHub Release `nlc-3.589.150` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
