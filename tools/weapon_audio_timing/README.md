# Weapon audio timing tool

Run `weapon_audio_timing.bat`. Reload sounds are the default and recommended starting scope. Draw/holster scanning is available separately.

The tool reads originals from:

`C:\Games\NLC_OGSR_HE files\unpacked sounds\sounds\weapons`

Generated variants are written to the canonical repository under:

`gamedata\sounds\weapons\sivol_timing`

The unpacked originals are never overwritten. Shared sounds receive section-specific output names so weapons with different timing multipliers do not conflict.

Workflow:

1. Scan the desired sound class. Every generated CSV row starts disabled.
2. Enable only intended rows and set `speed`. A value of `1.10` makes the sound 10% faster while preserving pitch. Leave `output_sound` blank for an automatic section-specific name, or provide an explicit mod-relative sound name.
3. Validate and preview the expected durations.
4. Render with FFmpeg.
5. Listen to the generated OGG files, then patch the corresponding LTX sound references.

The config-patching step refuses to run until each enabled output exists. It edits the repository only and does not deploy files to the game installation.

Running another scan replaces the CSV, so keep a copy of any hand-edited profile you want to retain.
