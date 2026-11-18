# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.218` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `62B3004566D03F710816EB91B19127C866016E5CF465A823D6D7622344E8E38A` |
| PDB | `nlc_tools/builds/20261118_232231_62b30045` until the GitHub Release `nlc-3.589.218` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
