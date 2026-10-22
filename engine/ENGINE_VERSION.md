# Engine version

`engine/` in this repository is the **source of truth** for the NLC engine. Engine
source is edited and committed here, together with the scripts and
`bin_x64/xrEngine.exe` built from it. Workflow: `engine/.github/README.md`.
Build history: `engine/CHANGELOG_NLC.md`.

| | |
|---|---|
| Engine build in `bin_x64/xrEngine.exe` | `nlc-3.589.129` |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| exe SHA-256 | `3FE52DDB1E306BF896AD81EA68387E1131D9C30C3FD5B3F6A88488D0B14C66B0` |
| PDB | `nlc_tools/builds/20261022_104158_3fe52ddb` until the GitHub Release `nlc-3.589.129` exists (never in git) |

Update this file whenever a new engine exe is committed to `bin_x64/`.
