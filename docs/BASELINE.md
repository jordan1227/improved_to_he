# Baseline

- Canonical editable repository: `C:\Games\NLC_OGSR_HE files\improved_to_he`
- Runnable test installation: `C:\Games\NLC_OGSR_HE`
- Loose files under the runnable installation's `gamedata` override packed game files.
- Repository edits are the source of truth; runnable-installation copies are deployment artifacts for requested testing only.

Known clean-game baseline and packed-file inventory: TBD.

## Observed implementation baseline

- Lifecycle-signal caches are suitable for static anomalies: cache the client
  object position and shape radius on spawn, remove it on network destroy, and
  calculate actor distance locally.
- Gravity effects still evaluate cached entries each frame for responsive camera
  behavior; this avoids repeated global anomaly enumeration and object lookup.
- Electra/PDA/NVD/flashlight proximity shares one cache and refreshes every 200 ms;
  visual smoothing remains per frame in the consumers.
- Observed during the Electra VFX port: `idle_light` is inherited from the Electra
  base section and is required by native zone initialization. Enabling
  `idle_light_anim = koster_01_electra` also requires that animation to be present
  in the compiled `lanims.xr` table; a missing entry caused a load failure in this
  build. This is an observed build-specific integration requirement, not an
  assumed engine law.
- `db.actor:get_current_outfit_protection(hit.burn)` reflects effective outfit
  protection but not the tested powered belt-artifact layer. Belt artifact immunity
  is composed separately and multiplicatively by `sivol_anom_effects`.
- The native outfit-protection getter has a confirmed pathological no-outfit path.
  Callers must check slot 6 and return zero before invoking it when unarmored.
- Thermal/Buzz winner results use persistent scratch tables, and all three anomaly
  scans share one actor-position read per update to avoid steady-state allocations.
- Outfit protection is refreshed at most every 200 ms; belt protection is lazily
  recomputed after spawn/equip/unequip/drop signal invalidation.
- Scripted ambient burn hits should use `hit.burn`, zero impulse, a valid draftsman,
  and `bip_none` rather than a localized spine bone.
- Manual weapon breath is manual-only: sprint while ADS activates it in hold or
  toggle mode; there is no automatic breath-debt or forced-breath-out system.
  The only player setting selects the input mode.
- Hold breath has a 2.5% power floor, a 15-second maximum duration, and a
  two-second reactivation cooldown. Breath-out is audible only after an
  eight-second continuous hold, except for the forced 15-second timeout.
- Weapon handling derives from the active weapon's live base mass plus attached
  scope, silencer, and launcher mass (ammo excluded). Knives remain outside the
  system; binoculars keep their existing exemption behavior.
- Scripted recoil replaces native camera recoil only for reviewed firearms and
  keeps camera and first-person HUD contributions separately reversible. The
  accepted global camera multipliers are pitch `2.90` and yaw `0.72`; HUD pitch
  is normalized against global camera pitch so camera-strength tuning does not
  automatically exaggerate the weapon model.
- Recoil context composes live low-health, stance, conditioned exoskeleton,
  cached powered-artifact, and the existing class/day weapon-mastery factors.
  Artifact recoil shares the existing
  condition-aware two-second belt pass with wobble; the shot path does not scan
  the belt or allocate a result table.
- The accepted FL hook exposes absolute, non-accumulating three- and six-axis HUD
  transforms with owner checks, input clamps, render-time restoration, and a
  watchdog. Lua clears the hook at neutral and leaves it idle after recovery.
- Detachable suppressors use caliber/profile-specific ballistic and handling
  modifiers. Their extra weapon wear is native OGSR behavior through
  `condition_shot_dec_silencer`, ranging from +7% to +20% per shot in the
  current profiles. Integral suppressors retain their weapon-authored baseline
  wear rather than receiving detachable-addon wear a second time.
- Suppressor diagnostics are disabled by default. Setting
  `sivol_weapon.suppressor_diagnostics = true` logs base/suppressed configured
  wear and condition deltas without changing weapon condition.
- The zoom effector applies wobble as a rendered-direction overlay. On zoom-out,
  `wpn_params` rebases the last rendered direction into the actor camera base so
  the camera does not return to the ADS-entry aim direction.
- Open follow-up ideas, not implemented: distinguish hold-breath drain from the
  aggregate tiredness/max-power path, and investigate optional compensation for
  the intentional combined ADS-plus-walking stamina drain.
