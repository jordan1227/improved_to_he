# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.191` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `678E09CCC4EAAA251854DEC66D50C6202AEA4BDD1FAF9CE6F2B47C0ED2D8FED2` |
| PDB | `nlc_tools/builds/20261112_134857_678e09cc` until the GitHub Release `nlc-3.589.191` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
