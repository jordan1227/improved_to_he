# Modding notes

These are concise observations from the thermal-anomaly and device-interference work.

- Prefer existing lifecycle signals (`on_anomaly_spawn` and
  `on_anomaly_net_destroy`) for static anomaly caches. Re-enumerating global anomaly
  lists every frame is more expensive and can return server-side objects where a
  client object is required.
- Cache stable anomaly data (client position, shape radius, section/tier) and do the
  inexpensive actor-distance calculation locally. Keep per-frame evaluation only
  where smooth visual or camera response matters.
- A shared, throttled proximity result is cleaner than separate PDA, NVD, and
  flashlight scans. The current Electra refresh interval is 200 ms, with consumer
  smoothing still running every frame.
- Save callbacks should perform persistence work only on the real save signal, not
  as an `on_update` callback. Runtime capability checks are safer than caching module
  availability at load time when addon order can vary.
- `log2` state-change and cache lifecycle messages are useful for in-game diagnosis;
  avoid per-frame logging except when deliberately debugging.
- For ambient actor damage, `hit.burn` with zero impulse, a valid draftsman, and
  `bip_none` preserves the anomaly burn behavior while avoiding localized blood
  decals and sprint interruption.
- The engine getter `db.actor:get_current_outfit_protection(hit.burn)` was observed
  to include the tested powered burn-protection artifact. The observed stacking is
  multiplicative, not additive. Keep manual artifact composition separate until a
  case is found where the getter omits an artifact source.
