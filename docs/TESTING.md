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
- Per-surface shell sounds use `fl_step_material()` at the firearm event, normalize
  `materials\\<name>` to `<name>`, and fall back to generic sounds for unavailable
  or unmapped materials. Confirmed in-game: mapped material sounds work and knife
  attacks queue no shell sound.
- Belt cache invalidation was confirmed immediate for one and two powered burn
  artifacts: `0.550 * 0.75 = 0.413`, then `0.550 * 0.75 * 0.75 = 0.310`.
- Unarmored proximity stalls were isolated to the native outfit-protection getter,
  not raycasts, result-table allocation, or `actor:hit`. Guarding slot 6 before both
  burn and chemical getter calls removes the thermal stall; Buzz uses the same guard.
- Periodic diagnostics are disabled by default. Optional GC diagnostics should be
  enabled only for targeted profiling, not ordinary gameplay.
- Manual weapon breath acceptance testing is complete; the detailed coverage and
  deferred follow-up items are retained in `WEAPON_BREATH_TEST_PLAN.md`.
- Confirmed weapon-breath behavior: hold and toggle input modes, 2.5% stamina
  floor, 15-second forced release, two-second reactivation cooldown, quiet
  short releases, and breath-out after holds of at least eight seconds.
- Confirmed weapon handling: real base-plus-addon mass drives ADS drain and
  low-stamina wobble; scoped/integral-optic and binocular exclusions suppress
  only the extra hold-breath FOV/HUD-FOV change, not stabilization or costs.
- Confirmed camera behavior: leaving ADS preserves the final visible aim
  direction rather than snapping back to the direction present at ADS entry.
- Confirmed Electra ARRIVAL VFX/SFX port: Core idle/blowout and sounds load for
  base/average/strong; weak retains the original NLC particles and sounds; strong
  also renders the isolated Aggressive-v2 idle sparks. PDA interference and torch
  flicker continued to trigger as regression checks.
- Confirmed Electra idle-light tiering: weak is off, average/base uses range 1.5,
  and strong uses range 3.0. Full clean level load and
  `sivol.spawn_anomaly_near("zone_witches_galantine_strong")` both completed
  without regression after the matching light animation was registered.
- Confirmed ARRIVAL Whirligig port: `zone_mincer` weak/average/strong render
  the separate gravitational tiers; the strong awakening group loads cleanly.
  Vortex (`zone_gravi_zone`) was returned to its original NLC presentation:
  weak is sound-only, average has leaves, and strong adds dust.

### Mutant/controller session

- Controller bind scrambling restores movement and fire/ADS mappings without
  permanent unbinding; repeated scramble ownership and death restoration are
  logged separately.
- Author difficulty uses 80 m normal and 100 m strong-controller ranges. Easy
  applies `0.9x` ability chance and `0.8x` range (64/80 m).
- Three-ray cover sampling was confirmed at 0/3 behind hard cover. Exact reticle
  targeting promotes targeted mechanics to effective 3/3 exposure while keeping
  sensed effects cover-reduced rather than eliminated.
- Authored magnification resolves correctly: APS is 1.00x/threat 1.00; SV-98 is
  approximately 7.94x and reaches the threat 2.00 cap. Reticle interception now
  uses the 16 ms weapon update instead of the irregular monster update.
- Fire reactions report explicit miss, proc/outcome, ray-loss, attempt, actor,
  and success-cooldown states. They no longer reject attempts solely because the
  ordinary controller target/camera pool is cooling down.
- `dog_shield` stacking and condition direction were confirmed. Three strong
  artifacts produced `dog_shield=0.275`, `weapon_factor=0.492`, and reduced an
  APS controller reaction from roughly 27% to roughly 11% in the tested gear set.
- Mask `psy_k` damage/camera scaling, sensed effects behind cover, close-range
  native plus scripted pressure, two-controller lockouts, tiered ranges, and the
  dog-shield condition fix were exercised in game.
- Native fake/hallucination fields produced no counted scripted phantoms in the
  standalone observation. Regular scripted phantoms spawn and expire; the
  experimental `psy_dog_phantom` spawned audibly but was invisible and is disabled.
- Controller locomotion was not modified or accepted in this session. Occasional
  stationary combat behavior appears native; do not treat movement as covered by
  the controller ability acceptance results above.

For anomaly reshuffling/emission tests, use the mod's supported console/script
command mechanism when the relevant function is known; exact command names and
log-file locations remain TBD.
