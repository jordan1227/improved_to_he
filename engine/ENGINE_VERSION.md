# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.136` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `78F1869EBA8E8380436E4D51A4CAD127A670CE671332467D41BA06CD7D54E1E5` |
| PDB | `nlc_tools/builds/20261023_083103_78f1869e` until the GitHub Release `nlc-3.589.136` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
