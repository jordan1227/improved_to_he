# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.135` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `FFC92F56B1EF58A8CCA1FB677C009F72AE580969ADF9942D6BE6D23504024E04` |
| PDB | `nlc_tools/builds/20261023_064526_ffc92f56` until the GitHub Release `nlc-3.589.135` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
