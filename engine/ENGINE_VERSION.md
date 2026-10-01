# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.2` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `01026234925690EF58BE213DE2DE29558455F24F0C182FB99A8E74C5F11C1C0F` |
| PDB | attached to the GitHub Release `nlc-3.589.2` of this repository (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
