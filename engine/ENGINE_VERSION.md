# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.161` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `C9E9637917C74D308B80D07D76F55F78A8E7E697B5C33C15A3542935E7A5A0E6` |
| PDB | `nlc_tools/builds/20261027_191810_c9e96379` until the GitHub Release `nlc-3.589.161` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
