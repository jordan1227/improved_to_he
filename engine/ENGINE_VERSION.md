# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.224` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `BD39B7E5FDCA71AC8BD1C7C4E47B48F507EB44A1C0B8EBF68B15CB3B48C8784F` |
| PDB | `nlc_tools/builds/20261125_193326_bd39b7e5` until the GitHub Release `nlc-3.589.224` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
