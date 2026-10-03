# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.16` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `64321D0632C68F12C95F556FA554C7825E18F81E9698D77E45FA98016DFACB49` |
| PDB | `nlc_tools/builds/20261003_213152_64321d06` until the GitHub Release `nlc-3.589.16` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
