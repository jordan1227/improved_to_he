# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.29` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `9D6F0A194F42DC54A3D09A19AD43360BA7616F1D82B0423BCDA8244274E6A948` |
| PDB | `nlc_tools/builds/20261007_153704_9d6f0a19` until the GitHub Release `nlc-3.589.29` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
