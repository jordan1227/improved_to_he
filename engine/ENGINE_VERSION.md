# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.175` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `4BB2D7232535C1D12BD389A362B1234807BF9CD04C002FF8D26DA388DB059FB7` |
| PDB | `nlc_tools/builds/20261102_235737_4bb2d723` until the GitHub Release `nlc-3.589.175` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
