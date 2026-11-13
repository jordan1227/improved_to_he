# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.192` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `E484312F734180C080605E41A3FEC3038D8D8FA78E6B9509E70A18797D6BC23F` |
| PDB | `nlc_tools/builds/20261112_223337_e484312f` until the GitHub Release `nlc-3.589.192` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
