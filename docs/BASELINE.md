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
- `db.actor:get_current_outfit_protection(hit.burn)` reflects the tested outfit and
  powered artifact protection stack. Artifact values are multiplicative in the
  observed setup; no manual artifact composition is currently performed.
- Scripted ambient burn hits should use `hit.burn`, zero impulse, a valid draftsman,
  and `bip_none` rather than a localized spine bone.
