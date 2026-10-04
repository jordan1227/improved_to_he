# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.25` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `6DA80D5FFF05401EC30D2A30BAF4EB01B9C4E9DD61549C148C667D004C0B9DDC` |
| PDB | `nlc_tools/builds/20261004_235240_6da80d5f` until the GitHub Release `nlc-3.589.25` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
