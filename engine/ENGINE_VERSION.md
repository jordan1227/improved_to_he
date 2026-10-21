# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.126` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `436E05E1BE5C921A5A9FE1F6E7550A31C27E48D337042B6F06945BE67E6D3A02` |
| PDB | `nlc_tools/builds/20261021_143919_436e05e1` until the GitHub Release `nlc-3.589.126` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
