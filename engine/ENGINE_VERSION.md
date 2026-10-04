# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.17` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `8B3254F5CDC441C9E70F29A527B2D41B5008103375EB52043C0A158E5991D823` |
| PDB | `nlc_tools/builds/20261004_143703_8b3254f5` until the GitHub Release `nlc-3.589.17` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
