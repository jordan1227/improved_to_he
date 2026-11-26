# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.234` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `E2D712C07A7A4BC9F058E955A522C5AAF95DB734DB93F9EE85D972115A153C8A` |
| PDB | `nlc_tools/builds/20261126_132442_e2d712c0` until the GitHub Release `nlc-3.589.234` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
