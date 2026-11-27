# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.243` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `E18A647247B24D3B2B113A3097264620AE105C3A933D64064A16F067FC36CBA3` |
| PDB | `nlc_tools/builds/20261127_200812_e18a6472` until the GitHub Release `nlc-3.589.243` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
