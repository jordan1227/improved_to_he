# Weapon timing editor

Run `weapon_timing.bat`. The tool edits the canonical repository only; it never deploys files to the game installation.

Recommended workflow:

1. Export the current timing profiles.
2. Edit `weapon_timing_profiles.csv` in a spreadsheet or text editor.
3. Validate, then preview.
4. Apply only after reviewing the preview and keeping the intended rows enabled.

Each row targets an exact LTX file and HUD section. `show_speed` and `hide_speed` also update their `_empty` companions when those keys exist. Values above `1.0` are faster.

The spread action writes `weapon_timing_profiles.candidate.csv`; it never replaces the active profile automatically. Its defaults widen draw differences more than holster differences and clamp the result to conservative limits.
