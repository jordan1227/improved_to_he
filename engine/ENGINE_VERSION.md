# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.130` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `BC368BD93C9D42A7022EFD0C0E9F5F7ED15F8239BDD1CC7B44F19679B9E832FB` |
| PDB | `nlc_tools/builds/20261022_130757_bc368bd9` until the GitHub Release `nlc-3.589.130` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
