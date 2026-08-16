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
