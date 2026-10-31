# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.168` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `858A484A80569954093B39A52382E082EC0A90C3E0AB3A9650459A78A42CBB2B` |
| PDB | `nlc_tools/builds/20261031_144745_858a484a` until the GitHub Release `nlc-3.589.168` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
