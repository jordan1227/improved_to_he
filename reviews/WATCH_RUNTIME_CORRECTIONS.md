# Watch runtime corrections - 2026-09-13

Status: corrected source deployed; in-game acceptance pending. No commit.

Changes: register both custom shader exports; block native PDA alarm action and poll its binding; add editor timeout/menu cleanup; retain Use suppression through held-key release; shorten settings labels; use a dedicated verified 12x12 glyph atlas and round decoded digits.

Validation: Lua 5.1 syntax; focused real-Lua mocked input/lifecycle assertions; XML parse; CRLF/no BOM; git diff --check; 8/8 SHA-256 matches. Shader compilation and final visual/input acceptance require a complete game restart and manual test.

The Lua harness models blocked actor callbacks and polls physical key state. It does not execute the engine.

The dedicated glyph atlas is copied byte-for-byte from the supplied secondary Improved reference under a unique name; it does not replace the global packed digit texture.

## Deployed files

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\config\system.ltx`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\config\system.ltx`
  SHA-256: `882f56122762cdeceeb54cb49e2f10a747265a079a9adf4bd345939501d24e9a`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\config\text\eng\ui_st_keybinding.xml`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\config\text\eng\ui_st_keybinding.xml`
  SHA-256: `f50e9d2baa9f51ab13e7a43e42544424b43261bd8837bb76a2eb15587f3a3979`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\config\text\rus\ui_st_keybinding.xml`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\config\text\rus\ui_st_keybinding.xml`
  SHA-256: `af2a1ca529961dca60824a454c57793aba1deded552adc56cbb0ba8751ce2445`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\scripts\he_watch_ui.script`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\scripts\he_watch_ui.script`
  SHA-256: `ac42b640b5c6074d68cbe299f7a402d4dfbc6a37010f9a3453973b18bec583a0`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\scripts\binders\bind_stalker.script`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\scripts\binders\bind_stalker.script`
  SHA-256: `e8cb8b20f058dad0f0511df530146bd513729434cdef3336a08ed4e55dfed5c1`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\shaders\r3\model_clock.ps`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\shaders\r3\model_clock.ps`
  SHA-256: `2a3fdefbd62b12d38bb905eee6be03fc8ae4702bcf393cc5fc2cdaad199c0085`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\shaders\r3\model_clock.s`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\shaders\r3\model_clock.s`
  SHA-256: `a148eacfd45ac28e3a41d60bb31e90cf642591e95f4dc8acd0f34b0e976e5823`

- Source: `C:\Games\NLC_OGSR_HE files\improved_to_he\gamedata\textures\shaders\watch\he_clock_digits.dds`
  Target: `C:\Games\NLC_OGSR_HE\gamedata\textures\shaders\watch\he_clock_digits.dds`
  SHA-256: `a7c8c9e1ab851709e9cd432689fd428d4d7a9d2291f6f5c1b1555bde7586fd13`

## Retest

Restart the game fully. Check clock glyph spacing; F5 opens watch, second F5 cancels; wheel and Shift-wheel adjust; short Use sets, held Use disables without setting on release; menu closes editor and restores input; detector present plus nonzero radiation drives the ring; alarm-enabled marker appears. Thirst gameplay remains unchanged.

Focused text diff: WATCH_RUNTIME_CORRECTIONS.diff (UTF-8 review rendering, not a CP1251 patch).
