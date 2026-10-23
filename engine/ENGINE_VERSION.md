# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.149` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `A240D7660DB9893B0ED2F04FFD089327A239D9E1E1CED799240A69CB6149FE40` |
| PDB | `nlc_tools/builds/20261023_211634_a240d766` until the GitHub Release `nlc-3.589.149` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
