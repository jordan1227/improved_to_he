# Testing workflow

For a gameplay change requested for testing:

1. Edit the canonical files in `C:\Games\NLC_OGSR_HE files\improved_to_he`.
2. Review the changed-file diff.
3. Copy only the relevant changed files to the matching paths under `C:\Games\NLC_OGSR_HE`.
4. Report exactly which files were deployed.
5. Launch the runnable game installation and manually test the affected behavior.
6. Record the observed result and inspect the repository diff again.
7. Commit only when explicitly requested.

Loose files in the runnable installation's `gamedata` override packed game files. Do not deploy anything for documentation-only, analysis-only, or setup tasks.

Automated test commands and log locations: TBD.

## Confirmed manual checks

- Thermal, gravity, and Electra effects activate and clean up correctly when
  approaching/leaving anomalies, including overlapping effects.
- Save/load near an anomaly restores the effects immediately.
- The shared Electra refresh interval of 200 ms feels smooth in game.
- NVD enabled state persists across save/load.
- The burn-protection cutoff can be tuned in `sivol_anom_effects.script`; at
  `0.50`, a 45% outfit plus a powered +25% artifact suppressed scripted damage
  while retaining the aura.
- Ambient thermal hits use `bip_none` to retain burn behavior without blood
  decals or sprint interruption.

For anomaly reshuffling/emission tests, use the mod's supported console/script
command mechanism when the relevant function is known; exact command names and
log-file locations remain TBD.
