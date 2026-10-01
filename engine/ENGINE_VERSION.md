# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.3` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `E145FBA5C3671C8C4E8FCE113547E63B951C43D78B288ABA4D42453DA5877F5A` |
| PDB | attached to the GitHub Release `nlc-3.589.3` of this repository (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
