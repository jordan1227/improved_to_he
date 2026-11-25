# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.226` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `61BA01F646842ED45D2346BB4A8CCAA0696F72C5BC08881CABB5A508B288CD19` |
| PDB | `nlc_tools/builds/20261125_224628_61ba01f6` until the GitHub Release `nlc-3.589.226` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
