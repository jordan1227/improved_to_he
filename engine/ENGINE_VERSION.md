# Engine version (read-only mirror)

`engine/` is a **read-only mirror** of the private engine repository
<https://github.com/AiramProvoker/NLC_OGSR_engine> (default branch `nlc`), updated in
the same commit as each new `bin_x64/xrEngine.exe`, so engine changes are visible in
this repository's history next to the game data.

**Do not edit engine source here.** Work in the engine repo (see its
`.github/README.md`); edits made here are overwritten by the next mirror update.

| | |
|---|---|
| Mirrored engine build | `nlc-3.589.1` (engine commit `40120fb`) |
| Upstream base | OGSR `main` `2021123` (2026-09-27) |
| `bin_x64/xrEngine.exe` SHA-256 | `5A3F8D71BA5DA732BA597C7C6697A0607F27D9E907594AA814BFCAAAE07C0CF3` |
| PDB | attached to the GitHub Release `nlc-3.589.1` of the engine repo |

Mirror update (on every engine release): replace `engine/` with
`git -C <engine repo> archive <tag> | tar -x -C engine`, keep this file updated,
commit together with the new exe.
