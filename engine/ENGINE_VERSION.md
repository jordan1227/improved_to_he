# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.199` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `D8A6EF317B496F3AEA8808B5AA3BF77027AA5B294AF3C9E4566F7381DE405C2C` |
| PDB | `nlc_tools/builds/20261113_124145_d8a6ef31` until the GitHub Release `nlc-3.589.199` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
