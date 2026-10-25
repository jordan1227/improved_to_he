# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.156` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `F0E04E4C5E6EF25BAC468A44CA71EFE4E0FE9FF92AAA57C6378F8F207B188AD1` |
| PDB | `nlc_tools/builds/20261025_173751_f0e04e4c` until the GitHub Release `nlc-3.589.156` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
