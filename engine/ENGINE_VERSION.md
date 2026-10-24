# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.150` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `C1A568F89A92C8236CD17E59A93851D9B4ABDEA8703A1A330AF747F54ED1BFF0` |
| PDB | `nlc_tools/builds/20261024_145627_c1a568f8` until the GitHub Release `nlc-3.589.150` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
