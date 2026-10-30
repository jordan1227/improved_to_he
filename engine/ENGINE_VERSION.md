# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.163` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `88E5DC7088EF37DABAB2F50C5C1FAE75788DCD5020BE5750CCD7C1C0E75EBFB8` |
| PDB | `nlc_tools/builds/20261030_162011_88e5dc70` until the GitHub Release `nlc-3.589.163` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
