# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.134` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `0105414CD7C1E03B64C9D535104DB48631F691C4E82EB6A6D37F40EA2345913E` |
| PDB | `nlc_tools/builds/20261022_201715_0105414c` until the GitHub Release `nlc-3.589.134` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
