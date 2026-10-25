# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.151` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `ADB3A1A33506E3C8A1473EF5154E0EFFC36D8272294C93EE860C1B48854539B2` |
| PDB | `nlc_tools/builds/20261024_183710_adb3a1a3` until the GitHub Release `nlc-3.589.151` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
