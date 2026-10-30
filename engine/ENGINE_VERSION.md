# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.162` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `0E331415885F53CDF1862198ECEE15BD1DD2056B63BC20F0B363F1DE1299F33B` |
| PDB | `nlc_tools/builds/20261027_205223_0e331415` until the GitHub Release `nlc-3.589.162` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
