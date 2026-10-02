# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.10` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `AF014A0519F11A3313951C11BE4F07F00245465ACD3271A4ACDE4DBA96809EA3` |
| PDB | `nlc_tools/builds/20261002_225851_af014a05` until the GitHub Release `nlc-3.589.10` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
