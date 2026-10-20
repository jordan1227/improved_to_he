# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.107` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `24244CE14DC708B4E06B2E770CE6B76810DC5D0FA708C86DFB30AE8532DA6C8D` |
| PDB | `nlc_tools/builds/20261020_041514_24244ce1` until the GitHub Release `nlc-3.589.107` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
