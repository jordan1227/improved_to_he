# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.111` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `4E2EADA8A9D3E170E29B384937EE9C9692521249E45EDE01697450CBACD44867` |
| PDB | `nlc_tools/builds/20261020_075443_4e2eada8` until the GitHub Release `nlc-3.589.111` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
