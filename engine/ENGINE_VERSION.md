# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.153` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `DA16A6D5A3747407678A1B8F255237DAC752FB69B6B214A32FC06357B3D98049` |
| PDB | `nlc_tools/builds/20261025_114622_da16a6d5` until the GitHub Release `nlc-3.589.153` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
