# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.158` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `7EE84CC90C4E5CAB86933AA2661C0051BB921D48B91B82998A0830EFCB2FAEFD` |
| PDB | `nlc_tools/builds/20261026_015239_7ee84cc9` until the GitHub Release `nlc-3.589.158` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
