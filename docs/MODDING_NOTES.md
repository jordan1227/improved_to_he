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
- `bind_stalker.script` forwards `on_item_to_slot`, `on_item_to_belt`, and
  `on_item_to_ruck` through `ogse_signals`; `on_drop_before_all` and
  `on_drop_after_all` bracket its drop bookkeeping. These are reliable cache
  invalidation hooks for outfit and belt-dependent values. Recompute lazily after
  invalidation instead of scanning inventory every frame.
- `log2` state-change and cache lifecycle messages are useful for in-game diagnosis;
  avoid per-frame or periodic damage-tick logging except when deliberately debugging.
  Synchronous log output can itself produce repeatable frame spikes.
- For ambient actor damage, `hit.burn` with zero impulse, a valid draftsman, and
  `bip_none` preserves the anomaly burn behavior while avoiding localized blood
  decals and sprint interruption.
- Never call `db.actor:get_current_outfit_protection(...)` while slot 6 is empty.
  Its native no-outfit path was measured taking roughly 230 ms and caused periodic
  stalls near thermal and chemical anomalies. Check `actor:item_in_slot(6)` first
  and return/cache zero protection when it is nil.
- `get_current_outfit_protection(hit.burn)` supplies the effective outfit layer;
  tested powered belt artifacts are not included by this getter. Their immunity
  factors stack multiplicatively (`outfit_damage_mult * art_1 * art_2`) and are
  maintained separately by the event-invalidated belt cache.
- In per-frame proximity loops, reuse persistent result tables and mutate their
  fields. Avoid table literals, `string.format`, sorting, and concatenation in the
  steady-state path. Obtain the actor position once and share it across scans.
