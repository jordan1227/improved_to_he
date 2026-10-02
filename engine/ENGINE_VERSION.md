# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.4` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `7C6BA53A0470FAB27DC01BF96CE15B113010BFEC3151178EA41CC3C21017589A` |
| PDB | `nlc_tools/builds/20261002_142159_7c6ba53a` until the GitHub Release `nlc-3.589.4` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
