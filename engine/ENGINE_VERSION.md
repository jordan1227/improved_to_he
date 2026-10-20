# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.108` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `2B60E06ACE97CDF366DB1DD75302DE03230F515E5471ABEAC854B694AE5B6B4F` |
| PDB | `nlc_tools/builds/20261020_060505_2b60e06a` until the GitHub Release `nlc-3.589.108` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
