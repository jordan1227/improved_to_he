# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.30` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `9FCCF18989CAC0DD9038095D4E1FE06D8A5487CA330E74FB638AD1A4EC4DCFE4` |
| PDB | `nlc_tools/builds/20261007_205937_9fccf189` until the GitHub Release `nlc-3.589.30` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
