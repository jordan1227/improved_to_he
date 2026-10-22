# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.128` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `AFAF56BDEEDFC496EC5A271261176650FC9F9F692124E1E6FC9D49ACE74F9BC2` |
| PDB | `nlc_tools/builds/20261022_100103_afaf56bd` until the GitHub Release `nlc-3.589.128` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
