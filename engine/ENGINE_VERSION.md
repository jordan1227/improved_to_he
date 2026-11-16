# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.208` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `85B761492793461F6540392C1836A96BA63D9EE8D747A9B731B4F73903920AFF` |
| PDB | `nlc_tools/builds/20261116_013030_85b76149` until the GitHub Release `nlc-3.589.208` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
