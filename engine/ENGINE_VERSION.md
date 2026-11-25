# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.225` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `A78D00283CA1C655B7AC2236C3C926995217381785D831AA3267AA5666733A02` |
| PDB | `nlc_tools/builds/20261125_195744_a78d0028` until the GitHub Release `nlc-3.589.225` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
