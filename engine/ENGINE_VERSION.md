# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.1` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `5A3F8D71BA5DA732BA597C7C6697A0607F27D9E907594AA814BFCAAAE07C0CF3` |
| PDB | attached to the GitHub Release `nlc-3.589.1` of this repository (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
