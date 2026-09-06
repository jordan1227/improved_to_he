# Manual weapon breath and mass-handling test plan

Document class: focused runtime acceptance plan. Use `WORKFLOW.md` for
deployment gates and `EVIDENCE_TEMPLATE.md` to record each accepted or pending
result.

Acceptance status: all implemented steps were confirmed in-game, including the
final-direction camera rebase in step 16. Step 14 remains explicitly deferred
for a later, separately scoped investigation.

Deploy the changed runtime files to a safe test installation. Test a pistol,
an assault rifle, an LMG, a knife, binoculars, and a masked outfit.

1. Open Gameplay options. Confirm that **Toggle hold-breath input** is the only
   hold-breath setting. OFF is hold mode; ON is toggle mode. There must be no
   checkbox that disables the mechanic itself.
2. In hold mode, ADS and hold sprint for less than eight seconds. Stabilization,
   extra stamina drain, and extra zoom must start on iron sights; release must be silent.
3. Hold continuously for at least eight seconds, then release. Exactly one
   breath-out should play. Repeat through ADS exit and weapon switching after
   eight seconds: each must use the same result.
4. Drain stamina while holding breath. At 2.5% power, it must force-exit once,
   restore normal ADS FOV, and clear toggle/hold state. It may play breath-out
   only if the continuous hold had already reached eight seconds.
5. Compare a pistol, AK-pattern rifle, and PKM/RPK at full and low stamina.
   Heavy weapons should begin steadier but gain visibly more wobble as stamina
   falls; they should also use more ADS stamina. Walking while ADSing must
   continue to compound drain normally.
6. Equip a knife or knife_m1. It must receive no weapon-breath activation,
   no extra ADS drain, and no weapon wobble contribution. Binoculars retain
   their existing exemption behavior.
7. If mass diagnostics are needed, set
   `sivol_weapon.weapon_breath_diagnostics = true` in the script temporarily.
   On weapon changes, check `[weapon_mass]` for the live `inv_weight`, source,
   wobble, and stamina sensitivity. Leave it false for normal play.
8. Wear gas, military, and scientific masks while holding breath. Mask breathing
   sounds and condensation must pause during active hold and resume afterwards.
9. The artifact weapon-effect template intentionally has no assignments yet.
   Confirm normal play is unchanged with ordinary artifacts; no walking-drain
   compensation is implemented at this stage.
10. Hold breath continuously for 15 seconds. It must force-release through the
    normal FOV restore path, always play exactly one breath-out, start a roughly
    two-second activation cooldown, and apply a brief higher-wobble penalty.
    Confirm no key press can reactivate stabilization, FOV reduction, or breath-in
    sound during that cooldown.
11. With an attached `g28_scope`, `specter`, `wpn_addon_1p78`, `wpn_addon_1pso`,
    or `wpn_addon_pe` (and with binoculars), activate hold breath in ADS. Wobble
    stabilization, stamina cost, and breath cues must still work, but the extra
    hold-breath FOV reduction must not apply.
12. Enable diagnostics and compare a bare rifle with the same rifle carrying a
    scope, silencer, or launcher. `[weapon_mass]` must list the added sections
    and their summed addon mass after each attach/detach or weapon switch; ammo
    weight is intentionally excluded. Confirm the values do not log/recompute
    continuously while attachment state is unchanged.
13. At low stamina, activate hold breath with no mask and with gas, military,
    and scientific masks. No ambient low-stamina breathing sound may play while
    hold breath is active; it must resume after the hold ends.
14. **Deferred / open investigation.** The hold-breath drain remains part of
    the actor's aggregate current-power state. The existing `tiredness_add()`
    system converts that aggregate deficit into tiredness/max-power effects and
    has no per-drain attribution. A later session may investigate attribution
    and optional walking-drain compensation for the intentional ADS + walking
    drain combination; neither is changed by this implementation.
15. ADS with `wpn_sv98`, `wpn_awm`, `wpn_toz34_m1`, `wpn_gauss_auto`, and
    `wpn_gauss`. Hold breath must still stabilize, drain stamina, and play its
    cues, but it must not add temporary FOV reduction.
16. While ADSing, move the aim point noticeably, then leave ADS. The camera
    must retain the final aim direction; it must not snap back to the direction
    recorded on ADS entry. On exit, the final rendered direction is now copied
    once into the actor camera base; a no-callback lifecycle fallback uses the
    last rendered direction. With diagnostics enabled, a normal exit logs one
    `[weapon_wobble] rebased final ADS direction on zoom exit` line.
17. Enable diagnostics and inspect `wpn_akm_pso`, `wpn_lr300_m1`,
    `wpn_lr300_ac10632`, and `wpn_lr300_specter`. They must not log as
    unregistered. `wpn_lr300_m1` is an unscoped alias and should retain zero
    add-on mass. The other entries must list their concrete addon section and
    its nonzero `inv_weight`: `wpn_akm_pso` must include `wpn_addon_1pso`
    (+0.60 kg), while AC10632 and Specter variants list their configured scope
    mass. Hold breath on the PSO/Specter variants must stabilize normally but
    add no extra FOV reduction.
18. Set HUD FOV to a memorable value (the normal default is 0.70), ADS an
    unscoped rifle, then hold breath. After the short ADS-settle delay, the
    hands/weapon should smoothly enlarge as HUD FOV narrows by up to 10%; on
    release it must smoothly return to the exact value set before holding.
    Repeat a rapid press/release and an ADS exit: there must be no snap or
    persistent HUD-FOV change. Repeat with PSO, Specter, 1P78, PE, G28,
    binoculars, and the integral-optic exclusions: HUD FOV must remain
    unchanged while their stabilization/stamina behavior still works.

For failures, report weapon section, displayed weight, current power, input
mode, hold duration, movement state, mask type, and relevant log lines.
