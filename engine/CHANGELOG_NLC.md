# NLC engine changelog

One entry per shipped build (tag `nlc-<ogsr>.<n>` in `jordan1227/improved_to_he`).
Newest first. The engine source lives in this repository under `engine/`, next to the
game data of the same build. The exe and its PDB are attached to the GitHub Release
of the tag in this repository.

## nlc-3.589.150 (2026-10-24)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `6223D9A9B5DA199049E33D25FD122771CA399AB381A9204CAD3AD2AB36C78332` (archived with its PDB in `nlc_tools/builds/20261024_003319_6223d9a9`).
- **Status:** runtime pending. SSS 23 quality batch after a 3-agent review (ghosting, reprojection, upsampling).
- **Required game data:** `shaders/r3/ssfx_glass.ps` (reads `s_position`) and `shaders/r3/ssfx_sss_lights.ps` (decodes the new lamp flag); the rest of the batch is shader-only (`ssfx_sss*`, `ssfx_il*`, `ssfx_ssr*`, `ssfx_water_ssr.ps`, `ssfx_ao*`, `ssfx_volumetric_combine.ps`, `ssfx_fog_scattering_blur.ps`, `ssfx_upsample.h`, `combine_1.ps`).
- Lamp contact shadows: `ssfx_lights_data.w` gets +4 when the slot's light moves faster than ~0.3 m/s (frame-rate independent), so carried torches keep little shadow history.
- SSS 23 glass binds `$user$position`: refraction no longer pulls in objects standing in front of the glass.
- The OGSR SSR targets (`rt_ssr1/2`, two full-size RGBA8) are created only without the SSS 23 SSR; the OGSR fallback pass is skipped when they do not exist ("Hide new effects" in rain shows no SSR).
- **Rebuild 2026-10-24 (same version string), exe SHA-256** `C1A568F89A92C8236CD17E59A93851D9B4ABDEA8703A1A330AF747F54ED1BFF0` (PDB `nlc_tools/builds/20261024_145627_c1a568f8`): .150 plus audio fixes:
- Stereo files listed in `snd_occlusion.ltx` `spatial_stereo` (default `bullet_shells\`) stay 3D via `AL_SOURCE_SPATIALIZE_SOFT` instead of forced 2D (NPC casings played at the listener).
- Near-miss whine: own sound copy per bullet (the shared templates let one playing whine block all others), `snd_bullet_whine_vol` 1.5, `snd_bullet_whine_dist` 6, `[snd_whine]` lines with `snd_occ_log whine`.

## nlc-3.589.149 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A240D7660DB9893B0ED2F04FFD089327A239D9E1E1CED799240A69CB6149FE40` (archived with its PDB in `nlc_tools/builds/20261023_211634_a240d766`).
- **Status:** runtime pending. Audio batch 3 (`docs/AUDIO_DESIGN.md` 15-16).
- **Required game data:** `scripts/sivol/sivol_mask_audio.script` + its entry in `ogse_signals_addons_list.script`; `misc/snd_occlusion.ltx` (`hrtf_2d_gain`).
- With HRTF on, 2D sounds bypass it as direct stereo (`AL_DIRECT_CHANNELS_SOFT`, mono duplicated, level `hrtf_2d_gain`); `snd_hrtf_2d on` keeps them in HRTF.
- `snd_hrtf_set N`: HRTF data set by index (`ALC_HRTF_ID_SOFT`; one device reset at start for a non-default set); sets and the one in use are logged.
- Listener filter for scripts: `level.nlc_set_listener_filter(gain, hf, fade_sec)`, `level.nlc_set_listener_filter_exempt(parts)`; direct, room and echo paths of non-exempt sounds; reset on level unload. Used by `sivol_mask_audio` for gas masks and helmets.

## nlc-3.589.148 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F47BAF0F3E217B4E9F9E242256635563A8495AD27266A5AE6D23120AF9A78B4E` (archived with its PDB in `nlc_tools/builds/20261023_205354_f47baf0f`; first test build `109F259E` replaced, same version).
- **Status:** runtime pending. Audio batch 2 after a 4-agent review (`docs/AUDIO_DESIGN.md` 13-14).
- **Required game data:** `misc/snd_occlusion.ltx` (new keys), `ogg_comments_overrides.ltx`, `weapons/w_grenades.ltx`, `weapons/weapons.ltx` (explosion layers), `scripts/sivol/sivol_shell_sounds.script` (`on_npc_shot`).
- Occlusion moved from the voice gain to the direct-path filter gain: shots behind buildings keep their reverb/echo share and are no longer culled at the cap. Loud sounds soft-capped (18/0.25/24 dB), terrain-only paths 22 dB, general cap 30 dB; capped dB become HF loss.
- Ray budget per 60 fps frame scaled by frame time (`snd_occ_rays` 160); due sounds served by priority; loud starts always get leak rays; no refresh below the cull level; raw occlusion and source-enclosure caches.
- World-ambient sounds are occluded in mode 1 (exceptions: `no_occlude`); level statics no longer occluded twice.
- Reverb: tunnels keep their long axis, first reflections from the nearest walls, darker big rooms, near-dry outdoor preset, echo depth 0.5, unchanged commits skipped, probe at most 30 Hz.
- No room send for 2D sounds without an object (rain, music, UI); rain keeps a floor indoors and is dulled.
- `snd_hrtf on` requests stereo-HRTF output (surround endpoints); surround modes named in the log.
- `CExplosive`: optional `snd_explode_distant` / `snd_explode_far` layers by listener distance.
- Near-miss whine for non-actor bullets passing within `snd_bullet_whine_dist` (4.5 m).
- NPC shell casings: `CWeaponMagazined::OnShot` calls `sivol_shell_sounds.on_npc_shot` (NPCs within 30 m, ground point + material).
- After the first test: `snd_stats` crash fixed (`statistic()` under the sound lock); reverb decay from a room size (geometric mean of the trimmed and cube-mean probe distances; a tunnel read 3 m and decayed in 0.6 s); the Sound menu HRTF list triggers `snd_restart`.
- Fixes: 2D flag set at emitter start (no distance delay on 2D sounds), geometry setters under the sound lock (unload race), timer-jump resets, EFX ids cleared on release, coplanar face pairing, no AI pulses while a delayed sound is paused, `snd_occ_log` case-insensitive, ltx empty-value guard.

## nlc-3.589.147 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C8CEF90B5AB3D8A457E9661C192F9AD2724129D0A27D9218FEF1136DC5C84A2A` (archived with its PDB in `nlc_tools/builds/20261023_194625_c8cef90b`; first test build `717C8933` replaced, same version).
- **Status:** runtime pending. Audio redesign phases 1, 2, 3a, 4, 5 (`docs/AUDIO_DESIGN.md`).
- **Required game data:** `gamedata/config/misc/snd_occlusion.ltx` (built-in defaults and a log warning without it).
- **Occlusion** (`snd_occ_mode 1`, default): per-material, thickness-based dB loss along the listener-sound ray (all CFORM hits, entry/exit faces paired), up to 5 leak paths around the occluder (least lossy wins), per-voice EFX low-pass for muffling, 4-15 Hz refresh within a ray budget (`snd_occ_rays` 96), attack/release smoothing. `snd_occ_strength`, `snd_occ_log <name part>`, `snd_occ_reload`; `snd_stats_info_name` shows `[-dB hf]`. `snd_occ_mode 0` = old single ray.
- Static level sounds were occluded twice (`level_sounds.cpp`); fixed in mode 1.
- **Reverb** (`snd_reverb_mode 1`, needs `snd_efx on`): listener room probe (24 rays + sky, 3 per update) drives an EAX reverb (enclosure, mean distance, material absorption), blended with authored `snd_env` zones (`snd_reverb_zone_weight` 0.5); outdoor echo slot on send 1 for shots and explosions in the open (`snd_outdoor_echo`); room send reduced when listener and source enclosure differ. Commit throttled to 20 Hz; `snd_efx off` now detaches the effects. `snd_reverb_gain`, `snd_reverb_log`.
- **Air absorption:** `snd_air_absorption` dB per 100 m (3) on the direct low-pass.
- **Speed-of-sound delay** for 3D one-shot sounds beyond `snd_delay_min_dist` (35 m) at `snd_speed` (343); AI events stay on time (`snd_delay`).
- **Weather ambience** (`sm_EnvAmbient`, `CGamePersistent`) ducked and dulled by the listener's enclosure (`snd_amb_indoor_gain` 0.3) instead of ray occlusion.
- **OpenAL context:** 2 aux sends, HRTF from `snd_hrtf` (0 off, 1 on, 2 auto; applies on `snd_restart`), logged at start.
- `snd_dist_model 1`: engine distance curve with a fade to zero over the last 20% of `max_dist` (default 0 = OpenAL inverse clamped).
- Fix after the first test: the room probe and occlusion smoothing clamp the timer delta (a bad first delta after a load made every reverb value NaN, so no reverb at all); NaN-safe EFX clamps and a reset with a log line.
- Diagnostics: `snd_audio_report` (device, HRTF, reverb, probe, every rendering voice), `[snd_delay]` lines and `snd_occ_log *` for all sounds; HRTF status, output mode and available HRTF sets logged at start.
- `snd_occ_mode` is on/off and `snd_hrtf` takes `off / on / auto` (Sound menu controls).

## nlc-3.589.146 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `261943263873E129BE06A8888ADB2D9B33519AC50D67F6FA1D8002296DC98DF8` (archived with its PDB in `nlc_tools/builds/20261023_183809_26194326`).
- **Status:** runtime pending.
- DLSS preset lists no longer initialise NGX (opening Options cost ~2 s under TAA); availability comes from the DLL file version.

## nlc-3.589.145 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A47C74E99BC5A6118D057924227B0EBC539633AFBE3B0C1789463775775C027D` (archived with its PDB in `nlc_tools/builds/20261023_130146_a47c74e9`).
- **Status:** runtime pending. Scope pass 8 (`docs/SCOPE_PIP_DESIGN.md` 20); .144 and the data-only `3d_reticle_lock` accepted.
- Scope DLSS warm-up on any frame while lens/hybrid mode is on (first NGX load ~2 s with TAA, once per launch).
- `g_3dss_lens_fade_in` default 0.5 (user's value). New ltx key `3d_reticle_lock` (SV-98 = 0).

## nlc-3.589.144 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4AED5A9F5C879C8F2C3E2CA47A2229756DDC93467DDC61227433D6D3F075E726` (archived with its PDB in `nlc_tools/builds/20261023_122356_4aed5a9f`).
- **Status:** runtime pending. Scope pass 7 (`docs/SCOPE_PIP_DESIGN.md` 19).
- Shader: RGBA reticles in lens mode drop the exit-pupil circle (one edge circle) and keep the dark ring opaque (no halo).
- Lens CAS sharpening, `r_3dss_sharpen` (0.3, 0..1) via `s3ds_param_7.y`.

## nlc-3.589.143 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `71751D830BB3FDF28AC773D44134FD5527CDBF91C20E60952FB9539DDFECF5A9` (archived with its PDB in `nlc_tools/builds/20261023_121148_71751d83`).
- **Status:** runtime pending. Scope pass 6 (`docs/SCOPE_PIP_DESIGN.md` 18).
- `g_3dss_lens_fade_in` is now the share of the raise left after the switch (the ramp always completes; the dark lens).
- Scope DLSS warm-up runs every frame from the combine phase (it never ran in .142).
- Defaults from the user's tuning: lens_open 0.95, fade_in 0, ppe 0.5, ppe_speed 4 (range up to 8), close_ppe 0.1.
- Shader: lens mode keeps the exit pupil fully raised (no transition disc) and caps its eye offset.

## nlc-3.589.142 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A3599F66CDFC6D79C986345B89DE4EB18329CE71562D8AD9814AEA0ED2196B30` (archived with its PDB in `nlc_tools/builds/20261023_114707_a3599f66`).
- **Status:** runtime pending. Scope pass 5 (`docs/SCOPE_PIP_DESIGN.md` 17).
- Lens entry/exit cvars: `g_3dss_lens_open` (0.9), `g_3dss_lens_close` (0.1), `g_3dss_lens_fade_in`, `g_3dss_lens_exit_unzoom`, `g_3dss_lens_exit_dark`, `g_3dss_lens_close_ppe`; `g_3dss_lens_ppe` up to 2; entry blink after the fade-in.
- Exit unzooms the held magnification to 1x (crop dropped to 1 once).
- Scope DLSS features created when a lens scope is in hands (timing logged); failed-config check uses the scope preset.
- Shader: RGBA reticles split by scale (ring/vignette/dirt disc lens-attached, fine marks locked inside the ring).

## nlc-3.589.141 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `25CA8B6028D0B202045910457CDAD7B622565D61C32923CF825F76654D3444F2` (archived with its PDB in `nlc_tools/builds/20261023_112447_25ca8b60`).
- **Status:** runtime pending. Scope pass 4b (`docs/SCOPE_PIP_DESIGN.md` 16).
- Lens exit keeps its last magnification and crop while darkening from the release (the exit pop).
- Shader: RGBA reticles split into a lens-attached vignette (coarse mip) and locked fine marks (no doubled marks).

## nlc-3.589.140 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `B2B630B31B54ABD3E04A45F209445318CFECDCDA8E151BC0182BE4440D4F9A92` (archived with its PDB in `nlc_tools/builds/20261023_110715_b2b630b3`).
- **Status:** runtime pending. Scope pass 4 (`docs/SCOPE_PIP_DESIGN.md` 16).
- Scope DLSS: display-size output on TAA/DLSS, two features (Performance dynamic crop up to 2x, Ultra Performance fixed 3x), lens model default K; `r_3dss_scale_factor` is FSR3-only; new `r_3dss_crop_debug`.
- Lens PPE on its own clock: `g_3dss_lens_ppe_speed` (1.5), `g_3dss_lens_ppe_delay`; close blink at 2x on ADS exit.
- Lens entry/exit: no glass underlay, brightness ramp black -> image and back; reticle texture edge fade.

## nlc-3.589.139 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `045CA8B8281D216FECCD66D3AA2C07E1FEF74F974670DD070A3CB69EA81FC677` (archived with its PDB in `nlc_tools/builds/20261023_102929_045ca8b8`).
- **Status:** runtime pending. Scope pass 3 (`docs/SCOPE_PIP_DESIGN.md` 15).
- Crop upscaling: the scope DLSS feature (Ultra Performance range, dynamic sub-rectangle) upscales only the centre `1 / crop` of the frame that the lens shows (`CWeapon::LensCropMag`, widest zoom of the scope); output = display x `r_3dss_scale_factor` (works from 1). `s3ds_param_6/7` carry the request and the crop used.
- `r_3dss_dlss_preset` (default F): DLSS model of the scope feature, independent of `r_aa_dlss_preset`.
- Lens fade-in: glass and image drawn together while the image fades in over 0.3 of the raise.
- Lens-only post-process: `nlc_gunplay.lens_ppe(name)`; on lens open plays `[nlc_scope] lens_open_ppe` (default `deimos_short.ppe`) with strength `g_3dss_lens_ppe`; colour/gray/blur reach `3dss_reticle.ps` via `s3ds_param_8/9`.
- Main DLSS feature created only when DLSS is the AA mode; DLSS DLL version read from the file next to the exe, preset list logged only on change.
- Required game data: `shaders/r3/3dss_reticle.ps`, menu row `r_3dss_dlss_preset` (`ui/ui_mm_opt.xml`, `ui_mm_opt_video_adv.script`, `ui_mm_opt_main.script`, `text/*/ui_st_mm.xml`).

## nlc-3.589.138 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7680A89F2F74C834186EA26766FC8785A650B51E0741FF220365D66857869AF2` (archived with its PDB in `nlc_tools/builds/20261023_094411_7680a89f`).
- **Status:** runtime pending. Scope pass fixes after the first test of `.137` (`docs/SCOPE_PIP_DESIGN.md` 14).
- Hybrid camera/lens share continuous (log scale, 2x all lens -> 8x all camera); lens magnification and camera zoom eased over ~0.1 s (`CWeapon::UpdateLensSmoothing`).
- Lens image replaces the glass from mid-raise (`g_3dss_lens_open`, default 0.5; was the end of the raise).
- Scope source upscaling only while the lens magnifies (no cost in full-screen zoom); DLSS scope feature also with TAA.
- Required game data: `shaders/r3/3dss_reticle.ps` (lens flip fix, reticle edge clamp), menu rows moved to Advanced graphics (`ui/ui_mm_opt*.script`, `ui/ui_mm_opt.xml`, `text/*/ui_st_mm.xml`), `scripts/ogsr_shaders_control.script` (radius helper).

## nlc-3.589.137 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `AFF8F60E357AFB40E5A02A93F8DF310AA827274B49551E4CBDCF0D3747348730` (archived with its PDB in `nlc_tools/builds/20261023_091228_aff8f60e`).
- **Status:** runtime pending. 3D scope lens mode and scope upscaling (`docs/SCOPE_PIP_DESIGN.md` 13).
- `g_3dss_lens_mode` (full screen / lens / hybrid, default full screen): `CWeapon` splits the scope's zoom into a camera zoom and a lens magnification (`ComputeLensSplit`); wheel zoom steps the scope's zoom in lens modes; `3d_lens_camera_share` per weapon/scope. `s3ds_param_5` (lens magnification, zoom part, active, LOD factor) pushed by `CActor` for `3dss_reticle.ps`.
- `g_3dss_lens_sens` (0..1, default 1): mouse slowed by the lens magnification. `g_3dss_lens_lod` (default 0.5): render LOD/culling distance bias while looking through the lens.
- Scope source upscaling: second DLSS feature when DLSS is the AA mode (FSR3 fallback); `r_3dss_scale_factor` capped at 2.0.
- crookr fakescope pass re-enabled for 2D texture scopes (`fake_scope_radius` > 0, set by `ogsr_shaders_control.script`).
- Shot-punch FOV scaled by camera FOV / `g_fov`.
- Required game data: `shaders/r3/3dss_reticle.ps`, `scripts/ogsr_shaders_control.script`, `ui/ui_mm_opt.xml`, `ui/ui_mm_opt_gameplay.script`, `ui/ui_mm_opt_main.script`, `text/*/ui_st_mm.xml`.

## nlc-3.589.136 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `78F1869EBA8E8380436E4D51A4CAD127A670CE671332467D41BA06CD7D54E1E5` (archived with its PDB in `nlc_tools/builds/20261023_083103_78f1869e`).
- **Status:** runtime pending. Knife strike volume and attack buffering.
- `CWeaponKnife`: the actor's strike is resolved before its bullet: the exact crosshair ray wins on a living creature; otherwise sample rays walk the HUD key's blade line (`strike_<hud key> = travel_deg, along_deg, across_deg [, offset]`, `-1` = stab ellipse) from its entry end, the first living creature wins at its sample nearest the line, graze samples skip `knife_exact_only_bones` and scale the hit (`knife_graze_min`, `knife_graze_full_band`). The bullet follows the chosen ray and stops 5 cm past the contact (one target per strike). Downward strikes reach further (`knife_low_reach_start/_max`). Optional `snd_hit`. `[knife_strike]` log.
- `CWeaponKnife`: attack presses in the last `knife_buffer_ms` of a swing are buffered and start the next attack at the clip end (`knife_buffer_ttl_ms`); `[knife_buffer]` log. The strong-attack key no longer switches state while the knife is busy.
- `fl_hook` M1 combo: with `wpn_knife_m1_hud/knife_combo_window_ms` the window counts from the previous clip end.
- Lua `level.nlc_knife_strike_kind(weapon_id)` (0/1 slash/2 stab) and `level.nlc_knife_strike_k(weapon_id)`: last strike of a knife within 1.5 s.
- Required game data: `weapons/w_knife.ltx`, `sivol/sivol_backstab.script` (+ `ogse_signals_addons_list.script`).

## nlc-3.589.135 (2026-10-23)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `FFC92F56B1EF58A8CCA1FB677C009F72AE580969ADF9942D6BE6D23504024E04` (archived with its PDB in `nlc_tools/builds/20261023_064526_ffc92f56`).
- **Status:** runtime accepted 2026-10-23 (tests 1-9). Walking stamina regen.
- `CActorCondition::ConditionWalk`: a plain walk (not accelerated, not climbing/airborne/landing, not overweight) keeps `walk_regen_k` of the `stand_power` regen, bypassing outfit power loss like `ConditionStand`. `walk_regen_k` (`actor_condition`, default 0) is Lua read/write on the actor condition.
- Required game data: `creatures/actor.ltx` (`walk_regen_k = 0.5`), `sivol/sivol_weapon.script` (ADS drain scaled by game time).

## nlc-3.589.134 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `0105414CD7C1E03B64C9D535104DB48631F691C4E82EB6A6D37F40EA2345913E` (archived with its PDB in `nlc_tools/builds/20261022_201715_0105414c`).
- **Status:** runtime pending. Fixes for .133 (`docs/GUNPLAY_FEEL_DESIGN.md` 7d).
- `nlc_gunplay` clip player: a clip slot frees at the end of its clip (one-shot `CObjectAnimator` keeps `bPlay` set), a full pool reuses the clip nearest its end, and the pool resets on `CLevel` construction too. Fixes crouch clips stopping after a few uses.
- ADS pose spring `2.6 / ADS time` (was 4). Lua `nlc_gunplay.set_param(name, value)` for `ads_omega_k`, `pda_share`, `pda_turn_share`, `pda_use_share`, `missile_share`, `fx_omega`, `fx_zeta`, `fx_aim_k`.
- Required game data: `sivol/sivol_weapon_inertia.script` (sets the params; optional).

## nlc-3.589.133 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C4ACE6444B92300707F1CCA7A4ABDE5D314D46B20C6AD3CCBF4B3BF7BF365550` (archived with its PDB in `nlc_tools/builds/20261022_195706_c4ace644`).
- **Status:** runtime pending. Gunplay batch 3 (`docs/GUNPLAY_FEEL_DESIGN.md` 7c).
- `nlc_gunplay`: HUD clip player (8 stacking additive `.anm` clips, mirror, ADS share), engine-played crouch clips on stance changes, shared HUD impulse spring, live motion modifiers. Lua `hud_clip`, `hud_impulse`, `set_motion_mod`, `set_crouch_clips`; cleared in `CLevel::~CLevel`.
- `CHudItem`: rotational lag pivots near the stock (`hud_rot_inertia_pivot_z`, `hud_rot_inertia_shift`); PDA (fades while the 3D PDA is zoomed) and grenades get the motion layers; ADS pose (`hud_ads_damping`) and near-wall pose on springs; clip speed per item (`hud_clip_speed_k`). The batch-2 spring crouch kick is removed (`crouch_kick_hud_offset_*` unused).
- Required game data: `sivol/sivol_weapon_inertia.script`, `sivol/sivol_weapon_aim_speed.script`, `sivol/sivol_fuzz_recoil*.script`, `params/wpn_params.script`.

## nlc-3.589.132 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F9791723ACA4E3B6290DEAD410B8A24FB920357739E4CD245744E79E39AAAC28` (archived with its PDB in `nlc_tools/builds/20261022_192530_f9791723`).
- **Status:** runtime pending. Gunplay batch 2 (`docs/GUNPLAY_FEEL_DESIGN.md` 7b).
- `CHudItem`: strafe, lookout, stance and walk offsets use springs (`NlcBlend`, stiffness from the authored transition time, scaled by `hud_motion_freq`); crouch kick on stance changes (`crouch_kick_hud_offset_pos/rot`, `hud_crouch_kick_k`) and look-up dip (`lookup_hud_offset_pos/rot`, `hud_lookup_k`), both writable from Lua. Default `hud_motion_freq` 9.35 (was 11).
- `CEffectorZoomInertion`: a script-owned sway (`switch_zoom_osc(true)`) stays applied on frames with a large camera move.
- Required game data: `sivol/sivol_weapon_inertia.script` (crouch/lean/look-up .anm removed), `sivol/sivol_weapon_aim_speed.script`, `sivol/sivol_fuzz_recoil.script`.

## nlc-3.589.131 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `121D6903C37681D5B2FF294024EA26397F0EAB641E59FD7FBFB3106BA643DBBB` (archived with its PDB in `nlc_tools/builds/20261022_183734_121d6903`).
- **Status:** runtime pending. Gunplay feel pass (`docs/GUNPLAY_FEEL_DESIGN.md`).
- `xrGame/nlc_gunplay.cpp`: damped springs for HUD motion, actor landing events (every ground contact after a fall, with contact speed), mouse-look rate, and an aim-neutral shot punch effector (camera roll + FOV, `eCENlcShotPunch`). Lua `nlc_gunplay.shot_punch(roll_deg, fov_deg)`, `count, speed, time_ms = nlc_gunplay.land()`.
- `CHudItem`: jump/fall/landing offsets driven by a spring (`hud_motion_freq`, `hud_motion_damping`, `hud_landing_k`, `landing_spring[_aim]_hud_offset_pos/rot`) with a landing kick scaled by contact speed; `hud_motion_freq = 0` restores the old lerps. New rotational inertia (`hud_rot_inertia_k/freq/aim_k/roll`, from mouse turns only) and movement inertia (`hud_move_inertia_k/freq/aim_k`, from velocity changes). All readable from the item section and writable from Lua on `CWeapon`, plus `hud_inertion_tendto_speed[_aim]` / `hud_inertion_origin_offset[_aim]` for the native translation inertia.
- `g_weapon_look_factor` (default off, Gameplay menu): the per-weapon mouse slowdown (`control_inertion_factor` / `scope_inertion_factor`) applies only when on; off = one true mouse sensitivity.
- Required game data: `sivol/sivol_fuzz_recoil*.script`, `sivol/sivol_weapon*.script`, `params/wpn_params.script`, `ui/ui_inv_descr.script`, `ui/ui_mm_opt_*.script`, `ui/ui_mm_opt.xml`, `text/*/ui_st_mm.xml`, `text/*/ui_st_inventory.xml` (scripts check for `nlc_gunplay` and the new fields before use).

## nlc-3.589.130 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `BC368BD93C9D42A7022EFD0C0E9F5F7ED15F8239BDD1CC7B44F19679B9E832FB` (archived with its PDB in `nlc_tools/builds/20261022_130757_bc368bd9`).
- **Status:** runtime pending. Exo rebalance pass 2 (`docs/damage_model/DESIGN_exo_rebalance.md` 7).
- `[actor] nlc_speed_mult_once` (actor.ltx sets it true): `m_fMovementSpeedMultiplier` applied once per movement in `CActor::g_cl_CheckControls` instead of per run/sprint/crouch/climb/strafe factor (sprint had it squared, sprint-strafe cubed, plain walk not at all). Off = original behaviour.
- Item panel (`CUIItemInfo`): two optional bars from the xml nodes `nlc_bar1/2_static`, `nlc_bar1/2_progress`, `nlc_bars desc_cut/desc_shift`; values per item `CInventoryItem::m_nlc_ui_bar` set by Lua `obj:set_nlc_item_bars(a, b)` (< 0 hides, not saved; `sivol_exo.desc_lines` pushes exo servos / charge).
- Required game data: `creatures/actor.ltx`, `ui/inventory_item*.xml`, `ui/trade_item*.xml`, `ui/carbody_item*.xml`, `sivol/sivol_exo.script` (checks `set_nlc_item_bars` before use).

## nlc-3.589.129 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `3FE52DDB1E306BF896AD81EA68387E1131D9C30C3FD5B3F6A88488D0B14C66B0` (archived with its PDB in `nlc_tools/builds/20261022_104158_3fe52ddb`).
- **Status:** runtime pending. Recovery pass 36 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 36).
- `LegsSpeedK`: no slowdown above `legs_full`, linear to `legs_zero`, applied once (walk/run `legs_walk_k`, sprint `legs_sprint_k`); stamina cap max(`legs_min_power`, legs + `legs_power_add`).
- `AddGreen`: regeneration a topped-off part cannot take turns its green white (overflow); passive conversion keys set to 0.
- Inventory part bars only while the cursor is over the body window or the figure, an item is dragged or a use row hovered; Lua `body_health.set_part_bars_always(bool)`.

## nlc-3.589.128 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `AFAF56BDEEDFC496EC5A271261176650FC9F9F692124E1E6FC9D49ACE74F9BC2` (archived with its PDB in `nlc_tools/builds/20261022_100103_afaf56bd`).
- **Status:** runtime pending.

### NLC

- `nlc_flash_light`: light cap 8 -> 16 for overlapping Pulse strikes (Tesla near/far channels and clusters). Required game data: none.

## nlc-3.589.127 (2026-10-22)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CE0424C46CF515D1AA86458AC5BB5C4417857674454A517DBAB3B1759CBBC634` (archived with its PDB in `nlc_tools/builds/20261022_074354_ce0424c4`).
- **Status:** runtime pending.

### NLC

- `xrGame/nlc_flash_light.cpp`: Lua `flash_light.spawn(x, y, z, range, r, g, b, attack_ms, hold_ms, fade_ms [, flicker, shadow, volumetric])` / `stop(id)` / `count()`. Short-lived point lights owned by the engine (max 8), envelope updated in `CLevel::OnFrame`, all cleared in `CLevel::~CLevel`, not saved. Used by the Pulse anomaly (`sivol_pulse_anomaly.script`, `docs/PULSE_ANOMALY_DESIGN.md`).
- Also first built here: the Shaders tab master switch `ssfx_effects_off` (pending source from another session: `xrRender_console.cpp`, `ssfx_features.h`, `DetailManager_Decompress.cpp`, `xr_3da/Environment.h`, `Environment_misc.cpp`). Its menu row is in `gamedata/scripts/ui/ui_mm_opt_shaders.script`.
- Required game data: none (scripts check for `flash_light` before use).

## nlc-3.589.126 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `436E05E1BE5C921A5A9FE1F6E7550A31C27E48D337042B6F06945BE67E6D3A02` (archived with its PDB in `nlc_tools/builds/20261021_143919_436e05e1`).
- **Status:** runtime pending. Includes commit 8b47d725 (`stalker_animation_torso.cpp`), first built here.
- `CActorCondition::nlc_bullet_absorb` + Lua `body_health.part_armor(part [, power, ap])`: share of a reference bullet (AK 5.45 FMJ) the outfit absorbs on a part, for the inventory hover (`docs/damage_model/DESIGN_recovery_and_body_status.md` 35).

## nlc-3.589.125 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `0EA672BFE7D1CEF4C10938449A9B5C162B9133CB0373D2BDCAE0C834AD8EE7F7` (archived with its PDB in `nlc_tools/builds/20261021_120600_0ea672bf`).
- **Status:** runtime accepted for the shadow legs (tuned in game): `g_actor_shadow_legs_cut` 0.2, `g_actor_shadow_legs_dy` 0.1. Required game data as `.123`.

### NLC

- Defaults baked in: `g_actor_shadow_legs_cut` 0.2, `g_actor_shadow_legs_dy` 0.1 (`g_actor_shadow_legs_fwd` 0). `g_actor_shadow_legs_from_body` 1 is the default, 0 (the actor model's own legs) stays as the fallback.
- Removed the `.124` in-air pelvis alignment (`g_actor_shadow_legs_air`): no visible difference in game.

## nlc-3.589.124 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `72E2076E768D6604983D144576954404591912AF8E855165C3AD2113252E35F7` (archived with its PDB in `nlc_tools/builds/20261021_115144_72e2076e`).
- **Status:** runtime pending (visible body shadow). Required game data as `.123`.

### NLC

- **Shadow legs in jumps:** the puppet's jump clips tuck the hips about 0.4 m (pelvis 1.0 -> 0.62 m in `lancew_legs_jump_idle`) while the actor model's torso stays high, which opened a gap in the shadow.
  While the actor is in the air (`CPHMovementControl::Environment() == peInAir`, smoothed at 12/s) the puppet is raised so its pelvis meets the actor model's pelvis (up to 0.6 m, never lowered).
  `g_actor_shadow_legs_air` (0..1, default 1) scales or disables it.

## nlc-3.589.123 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `81A8E7B371BB67A62F822CA90B6EBDAC7410E40C28EC64F1D545164C9452FB5F` (archived with its PDB in `nlc_tools/builds/20261021_114142_81a8e7b3`).
- **Status:** runtime pending (visible body shadow).
- **Required game data:** `gamedata/shaders/r3/models_selflight_det_3.s`, `shadow_direct_model_legs.vs`, `shadow_legs.ps`, `gamedata/config/system.ltx` (`[shader_params_export]` `m_nlc_vb_shadow`), `gamedata/scripts/sivol/sivol_visible_body.script`.

### NLC

- **Visible-body shadow = actor torso + puppet legs:** the puppet is a full-body mesh (torso, head, backpack) that only the camera hides. In the shadow pass its pixels above its own
  pelvis are clipped (`shadow_legs.ps`, height from `shadow_direct_model_legs.vs`, clip height `m_nlc_vb_shadow.x` = puppet pelvis height + `g_actor_shadow_legs_cut`).
  The puppet is placed with its pelvis under the actor model's pelvis (x/z) instead of origin-to-origin.
- **Console:** `g_actor_shadow_legs_cut` (default 0.03), `g_actor_shadow_legs_dy`, `g_actor_shadow_legs_fwd` (live tuning); `g_actor_shadow_legs_from_body` is 0/1 now
  (modes 2 and 3 removed); `g_actor_shadow_debug` logs pelvis/feet heights.

## nlc-3.589.122 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D8F6F3CDBEDBAC3E2F09513EDF6D2FE54494064DD9363FF1817F2C3063405358` (archived with its PDB in `nlc_tools/builds/20261021_112427_d8f6f3cd`).
- **Status:** runtime pending (visible body shadow).
- **Required game data:** `gamedata/shaders/r3/models_selflight_det_3.s`, `gamedata/scripts/sivol/sivol_visible_body.script`.

### NLC

- **Why the puppet never reached the shadow maps:** its mesh shader `models\selflight_det_3` (all `sm\actor_legs\*.ogf`) is a script shader with the old
  `normal`/`l_special` workflow, which compiles no shadow elements (`E[2]`, `E[3]` stay null), so `rimp_select_sh_dynamic` returned null in every shadow phase.
  The shader now declares `element_0..4` explicitly, with `element_2`/`element_3` as the standard model shadow pass.
- **`CCustomHUD::SMAP_Skip`:** `build_subspace` skips the puppet in the shadow-phase scene traversal; `CHUDManager::Render_Actor_Shadow` adds it itself (`g_actor_shadow_legs_from_body` 1 / 2),
  so modes 0 and 3 do not get a second pair of legs.

## nlc-3.589.121 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `69CC97CE245822E26C1021C6821856A7F7BFED598F61BD5A72D77C84F1FAC62F` (archived with its PDB in `nlc_tools/builds/20261021_110855_69cc97ce`).
- **Status:** runtime pending (visible body shadow).

### NLC

- **Actor IK off in first person:** `CIKLimbsController::IKVisualCallback` runs after the bone callbacks and rebuilds the thigh/calf/foot bones with its
  own callbacks, which overwrote the visible-body shadow leg bones (`.72`, `.119`, `.120` modes) and readjusted one foot at a time (jittering shadow legs).
  The callback now returns for the actor while `active_cam() == eacFirstEye`; `g_actor_fpv_ik 1` restores it. Third person is unchanged.
  Logs from `.120` (`g_actor_shadow_debug`) showed the puppet itself grounded (toe -0.01 m) in every mode, so the geometry was not the cause.

## nlc-3.589.120 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `E843AA4C6DACE205EE445A50AC1C22491A2FFD6D18E5BAFF724441237F80396B` (archived with its PDB in `nlc_tools/builds/20261021_105748_e843aa4c`).
- **Status:** runtime pending (visible body shadow). `.119` looked worse in game (legs in midair, large hip motion); this build is diagnostic.

### NLC

- **`g_actor_shadow_legs_from_body` 0..3:** 0 actor legs, 1 thighs collapsed + puppet added at the actor's x/z (`.119`), 2 the same with the puppet at its own
  position, 3 the `.72` rotation copy onto the actor's leg bones. Switchable live to compare.
- **`g_actor_shadow_debug 1`:** logs `[nlc_shadow]` once a second (mode, leg callbacks skipped because the bone already had a callback, puppet vs actor height,
  foot/toe/pelvis heights above the actor's feet).

## nlc-3.589.119 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `AD7AF35DB1B57102F8D7BAE15016CBD103AC49DB4F9FBA7067C6B542776BF69D` (archived with its PDB in `nlc_tools/builds/20261021_104251_ad7af35d`).
- **Status:** runtime pending (visible body shadow).
- **Required game data:** `gamedata/scripts/sivol/sivol_visible_body.script` (unchanged API; script placement/turn-lag tuning shipped separately).

### NLC

- **Visible-body shadow, puppet legs:** the `.72` leg-bone rotation copy is replaced. While the puppet is shown, the actor model's
  thighs are collapsed into the hip joints (`bip01_l_thigh`/`bip01_r_thigh` callbacks, children are built from them) and
  `CHUDManager::Render_Actor_Shadow` adds the puppet itself to the same shadow pass (`nlc_render_puppet_shadow`), with its own
  yaw and pose, moved to the actor's x/z. Bones are calculated explicitly (the shadow phase does not). The actor's own pelvis,
  bob and strafe yaw no longer mix into the legs. `g_actor_shadow_legs_from_body 0` restores the actor model's own legs.

## nlc-3.589.118 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `93A96D97721DBE01CD5EB9F290C53BF901A08DCB2E803BE11B882EE550F96977` (archived with its PDB in `nlc_tools/builds/20261021_100321_93a96d97`).
- **Status:** runtime pending. Recovery pass 34 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 34).
- `CActorBodyHealth::AddGreen`: artifact regeneration fills the most damaged part first (level-filling), same total gain.

## nlc-3.589.117 (2026-10-21)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CA5AF5E0F3D2F9EB197D6FAB6693672E41E7309FA73D6DCC360BB85148175282` (archived with its PDB in `nlc_tools/builds/20261021_092932_ca5af5e0`).
- **Status:** runtime pending. Recovery pass 32 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 32).
- Inventory body window: optional `part_bars` (per-part bars: permanent, temporary, preview gain, number); the preview follows a hovered use row (`CUIPropertiesBox::GetHoveredItem`, `CUIInventoryWnd::UpdateBodyPreview`) or a body item dragged over a part.
- `CActorBodyHealth::Predict` (side-effect-free item prediction; `HealPart` and it share `spread_heal`).
- `body_health.set_injury` accepts level 3 (splinted), tinted blue in the window.

## nlc-3.589.116 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `46E0D3FBB9B7FE856EB8B3FCF9E7809C6216D5D93D30B8DC75A137D139A22C69` (archived with its PDB in `nlc_tools/builds/20261020_145047_46e0d3fb`).
- **Status:** runtime pending. Recovery pass 31 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 31).
- `CActorBodyHealth::DisplayHealth`: with body health on, the HUD bar, the watch (`m_actor_params.x`) and the inventory health bar show min(total, head, torso) (white + temporary). Display only.
- "Use on part" rows: "Use on the torso: 42% -> 76%" (part white + temporary now and after the treatment total), bleeding / fracture as words (`st_use_bleeding`, `st_use_fracture`) instead of `{..}` / `[..]`.

## nlc-3.589.115 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `687D8780CCE1384843E66E511D10334C174AA38FB2634ABEE8A508CDF92FB7F3` (archived with its PDB in `nlc_tools/builds/20261020_141921_687d8780`).
- **Status:** runtime pending. Recovery pass 30 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 30).
- Item key `eat_green` (`CActorBodyHealth::AddGreenPart`): temporary health on the used part (no part and no bleeding: the most damaged one).
- Inventory body window: optional `pain_bar` / `pain_icon` children (pain / pain_max); `hint_text_inset` narrows only the hint text (default 10).
- "Use on part" rows show the treatment total (`vergas_medical_elastic.get_total_heal`).

## nlc-3.589.114 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7D863196A34E930EA90427D117FAE9A57D61C714776B59515B69E884DEDF37D0` (archived with its PDB in `nlc_tools/builds/20261020_134403_7d863196`).
- **Status:** runtime pending. Recovery pass test 4 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 29).
- Inventory body window: hovering the armoured figure (`figure_rect`, default 712,176,90,320) asks `sivol_body_text.part_text(0)`; the green layer is tinted bright green (alpha 0.35 + 0.65 x share); `CUIMapHint::SetBottomPadding` (10) so coloured multi-line hints do not clip.

## nlc-3.589.113 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4554F477434A98E3116D9F39AE5E6DF7E466126D6B296C16370C535C08973851` (archived with its PDB in `nlc_tools/builds/20261020_132426_4554f477`).
- **Status:** runtime pending. Recovery pass test 3 (`docs/damage_model/DESIGN_recovery_and_body_status.md` 28).
- `CActorBodyHealth::HealPart` (Lua `body_health.heal_part`): a heal worth a share of total health aimed at a part; the part takes up to 6x, the rest spreads, so the total gains the full value (medkits with body health used to raise it by 1/6).
- `CUIMapHint::SetHintWidth`; the inventory body hint is 270 wide (`hint_width` on `body_health`).

## nlc-3.589.112 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A5C66509CC23317319F95DE44DCBC2548D2D45597B9551CA0BFFF27244C55CCA` (archived with its PDB in `nlc_tools/builds/20261020_114854_a5c66509`).
- **Status:** runtime pending. Recovery pass (`docs/damage_model/DESIGN_recovery_and_body_status.md` 14).
- Body health green HP: `CActorBodyHealth` keeps green health per part; artifact / outfit regeneration (`CActor::UpdateArtefactsOnBelt`) adds green instead of health when body health is on; green counts for total health and death, hits and other losses take it first, it turns white at `green_convert_base` + `green_convert_factor` x regen per game hour (`[actor_body_health]`), and item key `green_convert` turns a share of every part's green white at once. `PartHealth` stays white, so leg speed, stamina cap and limb checks read white. Save version 2 (old saves: green 0).
- Lua `body_health.green`, `total`, `convert_green`, `set_injury`, `set_injury_system`.
- Inventory body window: shown with body health or the sivol injury system; green layer, injury tint on the fracture layer (light amber, severe red), multi-line hover text from `sivol_body_text.part_text` drawn above the inventory; the item use menu shows white + green.

## nlc-3.589.111 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4E2EADA8A9D3E170E29B384937EE9C9692521249E45EDE01697450CBACD44867` (archived with its PDB in `nlc_tools/builds/20261020_075443_4e2eada8`).
- **Status:** runtime pending. Staggers for every weapon type (`docs/damage_model/DESIGN_npc_damage_model.md` 21.2).
- `nlc_crit_anim_slot`: weapon animation types without critical-hit animations use the automatic set (2), pump shotguns (9) the rifle set (3); stock and NLC staggers both. `* [nlc_stagger]` prints the weapon and its type.

## nlc-3.589.110 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `870FE64FF81DCF80ECC4BC157C067A3BFB092183B502B040072C803FCA724150` (archived with its PDB in `nlc_tools/builds/20261020_073925_870fe64f`).
- **Status:** runtime pending. Stagger fixes after test 1 of the blunt pass (`docs/damage_model/DESIGN_npc_damage_model.md` 21.1).
- Forced staggers (impact and helmet concussion) go through `nlc_request_stagger`: `[nlc_npc_damage] stagger_log` prints `* [nlc_stagger]` with the block reason; transient blocks retry from `UpdateCL` for `nlc_stagger_retry` ms; an impact stagger that cannot animate keeps its aim penalty and cooldown.
- Ammo `nlc_blunt_head_k` (default `nlc_blunt_k`) for the blunt minimum on head bones, NPCs and player.
- `nlc_no_crouch` (stalker section): `CStalkerMovementManager::set_body_state` turns crouch into stand.

## nlc-3.589.109 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `01C8BDB7AB7E014615614E725E6A09D962090E636C9909DC5E1D2DCCFEFDCC73` (archived with its PDB in `nlc_tools/builds/20261020_071217_01c8bdb7`).
- **Status:** runtime pending. Blunt-impact minimum and impact stagger (`docs/damage_model/DESIGN_npc_damage_model.md` 21).
- `SBoneProtections`: optional third column `blunt` per bone line (`getBoneBlunt`, default 0).
- NPC (`CAI_Stalker::Hit`) and player (`CActorCondition::nlc_bullet_outfit`): `power = max(power, raw * blunt * nlc_blunt_k)`; ammo `nlc_blunt_k` read from the shooter weapon's current ammo type (`nlc_hit_blunt_k`); player head uses `[nlc_actor_damage] blunt_head` x mask coverage.
- Impact stagger: `nlc_impact_power` / `_window` / `_aim_time` / `_cooldown` in the stalker section; summed torso/pelvis hits stagger an armoured stalker, aim penalty and cooldown start when the stagger ends.

## nlc-3.589.108 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `2B60E06ACE97CDF366DB1DD75302DE03230F515E5471ABEAC854B694AE5B6B4F` (archived with its PDB in `nlc_tools/builds/20261020_060505_2b60e06a`).
- **Status:** runtime pending. Rebuild after merging the co-developer's `4c7ac302`: the `bin_x64/xrEngine.exe` committed with it lacked .99-.107, the installed .107 lacked its fix.
- Body health (`4c7ac302`, co-developer): healing first pays off the radiation health deficit (`CActorBodyHealth::ChangePartHealth`, `m_rad_deficit`).
- Otherwise identical to nlc-3.589.107.

## nlc-3.589.107 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `24244CE14DC708B4E06B2E770CE6B76810DC5D0FA708C86DFB30AE8532DA6C8D` (archived with its PDB in `nlc_tools/builds/20261020_041514_24244ce1`).
- **Status:** runtime pending. Arm-injury handling penalties (`docs/damage_model/DESIGN_actor_damage_model.md` 18.6).
- Actor weapon handling speed per kind (`player_hud.cpp` `nlc_set_hud_speed_factor`, `NLC_SPEED_RELOAD` / `NLC_SPEED_DRAW`): new `level.nlc_set_draw_speed_factor(name, k)`, `nlc_get_draw_speed_factor(name)`, `nlc_get_draw_speed_k()` for show / hide motions (`*show*`, `*hide*`, `*draw*`, `*holster*`, pistol `anim_close`) and the weapon's `snd_draw` / `snd_holster` (`HUD_SOUND::m_nlc_speed_kind`). Reload API unchanged.
- Fix (.106): the pistol's empty holster motion `anim_close` was scaled as a reload.

## nlc-3.589.106 (2026-10-20)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4479038DE44043499998F23A9F53A3CF4485D22BE8E5D3B6EC22DC32A5C0BA38` (archived with its PDB in `nlc_tools/builds/20261020_033948_4479038d`).
- **Status:** runtime pending. Combined build: NPC damage model follow-ups, aim review fixes, reload speed factors.
- NPC damage (`docs/damage_model/DESIGN_npc_damage_model.md`): `[nlc_npc_damage] legacy_communities` (comma list, `zombied`) keeps the stock armour formula (no AP, no bone koeff) for those communities; the concussion stagger fires only on bullet hits the stalker survives (after `inherited::Hit`, alive, not wounded, health left); no death line when the stalker dies within 1 s of a head hit (`m_nlc_last_hit_head`).
- Aim vs mutants (`docs/damage_model/DESIGN_npc_aim_vs_mutants.md` 6h): `nlc_mut_fire_target` kind 1 requires the sight point within 2 m of the mutant (no scope leak onto actor/stalker shots); the eye branch of `SetFirePointLookAngles` aims from `Position() + m_nlc_eye_offset` for mutant fire targets; the ready-gate cap no longer applies to `anim_sleep`.
- Actor reload speed (`docs/damage_model/DESIGN_actor_damage_model.md` 18.6): named factors `level.nlc_set_reload_speed_factor(name, k)` / `nlc_get_reload_speed_factor(name)` / `nlc_get_reload_speed_k()` (product clamped 0.3-2; `nlc_set_reload_speed_k(k)` = the `"injury"` factor). Covers reload and shotgun tube-loading motions (`*reload*`, `*_open*`, `*add_cartridge*`, `*_close*`) of the actor's weapon, third-person state timing (`CHudItem::PlayHUDMotion_noCB`), and the actor's reload sounds (frequency x k, delay / k; `HUD_SOUND::m_nlc_reload`).

## nlc-3.589.105 (2026-10-19)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `E9775051036CC137C2A7374F6D460BB8D207A0596CD5DB52FB19F472409A9020` (archived with its PDB in `nlc_tools/builds/20261019_035649_e9775051`).
- **Status:** runtime pending. Melee flinch after mutant-aim test 3 (`docs/damage_model/DESIGN_npc_aim_vs_mutants.md` 6g).
- `[nlc_npc_mutant_aim] flinch_*`: a mutant hit from within `flinch_dist` (`CAI_Stalker::HitSignal`) adds `flinch_cone_deg` x rank factor to the dispersion against mutants for `flinch_ms`; `nlc_mut_get("flinch_until")`.

## nlc-3.589.104 (2026-10-19)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `801C96BD795BB6D296B70DEF7847356AE2C68EB35CA49EA185BDC980D08D36D1` (archived with its PDB in `nlc_tools/builds/20261019_032851_801c96bd`).
- **Status:** runtime pending. Mutant-aim tuning after test 2 (`docs/damage_model/DESIGN_npc_aim_vs_mutants.md` 6e-6f).
- Psy cone: `[nlc_npc_mutant_aim] psy_cone_deg` / `psy_cone_full`: psy pressure adds dispersion against mutants (the old `nlc_psy_dispersion_k` only scaled the ~0.01 deg stance spread).
- Diagnostics: a hit on another mutant while the fight target is alive counts as a stray (`nlc_mut_get("strays")`) instead of opening a new engagement.

## nlc-3.589.103 (2026-10-19)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F884368890781B8E7A690C95467A2456FF24969EBE5D3924FF4CA3D611EE9299` (archived with its PDB in `nlc_tools/builds/20261019_030511_f8843688`).
- **Status:** runtime pending. Fixes after the first mutant-aim test (`docs/damage_model/DESIGN_npc_aim_vs_mutants.md` 6c-6d).
- `SetFirePointLookAngles`: mutant targets are aimed from the eye, not from the stalker centre (`use_center_to_aim` targets: bloodsucker, controller, zombie had every shot ~0.7 m high).
- `nlc_mut_fire_target`: A1/A3/A4/A5 also while xrs looks at a mutant through a torso position sight (not seen); `rounds_nosight` / `nosight_type` count shots at a selected mutant with any other sight.
- Diagnostics: hits counted once per shot, every pellet in `nlc_mut_pellets(band)` (new export).

## nlc-3.589.102 (2026-10-19)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CC3D293BACF2DBF5A648E2E7D6B26AA0606E680E5A4AEF19059F0938AD5B91B2` (archived with its PDB in `nlc_tools/builds/20261019_023820_cc3d293b`).
- **Status:** runtime pending. Stalker aim and engagement against mutants (`docs/damage_model/DESIGN_npc_aim_vs_mutants.md` 6, 6b); `[nlc_npc_mutant_aim]` in `game_relations.ltx`, `xrGame/nlc_mutant_aim.h`. Only while the sight target is a `CBaseMonster`; aim at the actor and at stalkers is unchanged.
- A1: `g_fireParams` shoots along the current head angles and position (`eye_matrix` is refreshed only every second scheduler tick, 250-725 ms by camera distance). A2: `CSightAction::execute_object` aims where the mutant was a rank-dependent lag ago (`lag_ms`, jitter per burst). A3: the bloodsucker cloak factor also scales the lag, plus `cloak_cone_deg` / `xray_cone_deg`; the cloak factor follows the mutant aimed at. A4: `head_speed` within `snap_dist`. A5: `spread_deg`.
- B1/B2: `CEnemyManager::useful` keeps mutants beyond `max_ignore_distance` inside their `nlc_npc_threat_dist` (new monster key) or within `approach_dist` when they attack the stalker, a friendly stalker, or the actor while the stalker is his friend / neutral (`help_dist_friend` / `help_dist_neutral`).
- Diagnostics (collected always, printed by `sivol_mut_aim_log.script`): Lua `npc:nlc_mut_get(key)`, `nlc_mut_rounds(band)`, `nlc_mut_hits(band)`, `nlc_mut_far_section()`, `nlc_mut_reset()`; `level.nlc_mut_aim_param(name, value)`, `level.nlc_mut_aim_get(name)`. Bullet hits on monsters by stalkers counted in `CBaseMonster::Hit`; fire-check reason codes from `can_fire_to_enemy`; far hits in `CAI_Stalker::HitSignal`.

## nlc-3.589.101 (2026-10-19)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `5EA5C680C98B108479CA30E0CC0062C064C717D77057C1D937464DA545F2C88D` (archived with its PDB in `nlc_tools/builds/20261019_013604_5ea5c680`).
- **Status:** runtime pending. God mode (`g_god`) still fills the actor last-hit record (`CActorCondition::nlc_dry_hit`: bullet model, hit scale, immunity, would-be guard, no outfit wear, nothing applied), so the hit log works while testing in god mode.

## nlc-3.589.100 (2026-10-19)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C48329836629870EAF68584813EFEAB4D77938CD1F0F3BC9900AAC5C20B66FAD` (archived with its PDB in `nlc_tools/builds/20261019_010809_c4832983`).
- **Status:** runtime pending. Actor bullet damage model (`docs/damage_model/DESIGN_actor_damage_model.md` 3-4, 11-15).
- `[nlc_actor_damage]` (`xrGame/nlc_npc_damage.h`): with `enabled`, a suit with `bullet_protection_sect` takes bullets through `pen = max(p - A*cf*(1-k_ap), p*hit_fraction)`, `K = 1 - cov*(1 - (1 - F*cf)^fw_protection_exp)` lifted by `k_ap*ap_koeff_bypass`; `F` is the live `fire_wound_protection`, `cf = min(1, condition/cond_full)`. Head-part bones take coverage and armour from `nlc_set_head_cover` (the worn mask) and `bullet_head_scale` instead of the damage-table hit scale. Other hit types and suits without the key keep the original outfit path. `CEntityCondition::HitOutfitEffect` is virtual (overridden by `CActorCondition`).
- One-shot guard (`nlc_body_health.cpp`, `[actor_body_health] guard_*`): a head or body part above `guard_above` when a hit window opens (`guard_window_ms`, the pellets of one shot) cannot drop below its floor in that window, for the hit types in `guard_hit_types`; with the body-health option off it guards the single health pool. Missing keys = off.
- Lua (actor): `nlc_last_hit_raw/post/dealt/k/armour/bone/type/time/guarded/bullet_model()`, `nlc_set_head_cover(cov, armour)`, `nlc_head_cover()`; `bone_armour`/`bone_koeff`/`bone_hit_fraction`/`bone_protection_section` return the suit's bullet table when it has one.
- Stalker concussion: a concussion still running when the critical-wound stagger ends restarts its full window from there.

## nlc-3.589.99 (2026-10-18)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `69AB60E0B2B70DE1E936F31302F3AFDD55B16F7C19700090E046D1BA2D281FDF` (archived with its PDB in `nlc_tools/builds/20261018_063819_69ab60e0`).
- **Status:** runtime pending. NPC bullet damage model, phase 2 (`docs/damage_model/DESIGN_npc_damage_model.md` 14-15).
- `[nlc_npc_damage]` (`xrGame/nlc_npc_damage.h`): `apply_ap` (ammo `k_ap` reduces NPC bone armour, replaces the `APPLY_ARMOR_PIERCING_TO_NPC` define), `use_bone_koeff` (the bone-protection koeff column absorbs a share of the post-armour bullet power, `ap_koeff_bypass` lifts it by `k_ap`), `apply_ap_actor` (actor outfit armour and `k_ap`). Missing keys keep the old behaviour.
- Critical wound (stagger): `critical_wound_raw_part` (share of the pre-armour bullet power that also charges it), `critical_wound_keep_charge` (a full charge waits for suitable conditions instead of being dropped).
- Stalker head-hit concussion: a surviving head hit where armour stopped at least `nlc_concussion_min_absorb` of the bullet multiplies weapon dispersion by `nlc_concussion_disp_k` for `nlc_concussion_time` ms; `nlc_concussion_force_anim` plays the head critical-hit animation on a new concussion.
- Lua: `bone_armour(bone)`, `bone_koeff(bone)`, `bone_hit_fraction()`, `bone_protection_section()` (stalker profile, actor outfit).

## nlc-3.589.98 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `45E01248889FFACD1C28053505A8F45C0AE696D2FA68F4EAC9EB08AE3DF12B68` (archived with its PDB in `nlc_tools/builds/20261013_233846_45e01248`).
- **Status:** runtime pending. `.97` accepted overall (`xray_hawkl_13-10-26_23-22-37.log`: retreats 16-42 m, perch pounces follow "not seen" within 0.4 s); melee dodges still stalled (13 of 18).
- Cause: `CControlAnimationBase::select_animation` keeps an attack animation until it ends, so a dodge started mid-swing timed out before the run animation could start. A melee dodge decided mid-swing now starts its clock, start point and boost when the swing ends (`melee_dodge_swing_wait_ms`, 700; dropped as `dodge end (swing)` past that); log `dodge waited N ms for the swing`.

## nlc-3.589.97 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C371233E5D68C88BCF1C5E86E81FDE880C7088C99BADC62FB1436151FC93F4A1` (archived with its PDB in `nlc_tools/builds/20261013_231645_c371233e`).
- **Status:** runtime pending. From `xray_hawkl_13-10-26_22-55-20.log`: melee dodges still stalled in `.96` (41 steps, 0.33 m average) - the target lay 120-150 degrees off its heading while it faced the player; two perch strikes waited on "enemy not seen" with the player in plain view; pack retreats started at 6-20% health to cover points 1-3 m away.
- Melee dodge steps circle the player at its own distance (forward diagonal); a step needing more than `melee_dodge_max_turn` (100 degrees) or covering under `melee_dodge_min_clear` (60%) of its length is skipped.
- Perch spot: unseen for `pounce_perch_blind_ms` (400) but seen within `pounce_perch_blind_mem_ms` (5 s): pounces anyway; logs `perch pounce: enemy not seen (N deg up, ...)` and `perch pounce blind`.
- Dodge growl on its own sound channel (`eDodgeGrowl`).
- Retreat threshold floor `tac_recover_pack_min` (0.3); retreat points closer than `tac_recover_min_move` (6 m) are skipped (`recover: <src> point only N m away, looking farther`).

## nlc-3.589.96 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `6542F557BFD0C1213E5996BFFD5299F59B3D674C2341BC3CC849F07DC5A1D474` (archived with its PDB in `nlc_tools/builds/20261013_225359_6542f557`).
- **Status:** runtime pending. `.95` dodges accepted overall (`xray_hawkl_13-10-26_22-40-24.log`); melee steps that switched sides stalled (28 of 48 under 0.3 m: a 180 degree turn eats the step).
- Melee dodge bursts keep one side (`melee_dodge_alternate = false`); a step that still switches side soon after the last one (blocked side, strafe) also backs off `melee_dodge_reverse_back` (1 m) so the turn is about 90 degrees; log `, reversal, backs off`.

## nlc-3.589.95 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F1DA679E1AAEF815325F46D3BB932B52861A665C8A84C8C8C326C0BC13A43640` (archived with its PDB in `nlc_tools/builds/20261013_223951_f1da679e`).
- **Status:** runtime pending. Bloodsucker dodge rework (`xray_hawkl_13-10-26_22-25-54.log`: small steps).

### NLC

- **Dodge:** step time from its length at run speed (+150 ms, `melee_dodge_max_ms` cap; a fixed 400 ms ran out after
  about 1 m from standstill), a start boost (`melee_dodge_start_k`); closing steps into swing range from the gap
  between swing and lunge range (`melee_dodge_close_range`), also while the player reloads; a counter-strike chance
  after a burst (`melee_dodge_counter`); a burst ends when the player stops aiming; the side goes against the player's
  strafe; a blocked side (wall) takes the other one; a growl on the first step (`melee_dodge_sound`); a `dodge end`
  log line with distance, angle around the player and duration.

## nlc-3.589.94 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D9291A54668CD45D2BC89D60D1BC3A45EB04D57CD07B7DCE6E33829948B4EE70` (archived with its PDB in `nlc_tools/builds/20261013_222506_d9291a54`).
- **Status:** runtime pending. After the `.93` test (`xray_hawkl_13-10-26_22-09-09.log`).

### NLC

- **Bloodsucker dodge:** a fixed step length (`melee_dodge_step` 2.2-2.8 m, the angle follows; an angle alone moved it
  1.6 m close in), at least `melee_dodge_min_ms`; no lunge for `melee_dodge_lunge_ms` after a step.
- **Hit and run:** shot with no other hiding point: a speed burst and an early zigzag switch (`tac_recover_shot_boost_*`).
- **Config:** the retreat zigzag, the blocked-retreat time, the pack radius and the dodge gap are keys now.

## nlc-3.589.93 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7D845120EC6984B6EC9C962ABC927F6DA0527EF46663AD1C364859F3285B4707` (archived with its PDB in `nlc_tools/builds/20261013_220456_7d845120`).
- **Status:** runtime pending. After the `.92` test (`xray_hawkl_13-10-26_21-47-32.log`).

### NLC

- **Bloodsucker perch:** 0.8 m or more above its ground point counts as a perch without the off-map reason (a 1.1 m
  rock was "accessible" by the 1.2 m rule; a weak one stood below biting).
- **Config:** invisible speed -20% (7.2 / 7.56 / 7.92); the run attack's second hit at 40% of the clip and +-0.7 rad
  (at 60% it landed after the run past the player); cats zigzag from 5 m with `zz_min_leg_k` 1.0.

## nlc-3.589.92 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `0642698D602237C6554AD57BC3404F866889F9CF66F875DCD904F06ED3765C4E` (archived with its PDB in `nlc_tools/builds/20261013_214706_0642698d`).
- **Status:** runtime pending. After the `.91` test (`xray_hawkl_13-10-26_21-33-53.log`, crash).

### NLC

- **Crash fix:** a bloodsucker retreat closed its wounds with `ClearWounds()`, which deleted wounds that
  `CEntityAlive::m_BloodWounds` still pointed to (FATAL in `LL_GetBoneInstance` from `UpdateBloodDrops`, bone 1478).
  It uses the stock healing path `ChangeBleeding(1)` now.
- **Bloodsucker dodge:** also while charging in (attack run, `melee_dodge_range` 6 m; the run target override takes
  the step), also when the player's shots hit it; steps around the player at `melee_dodge_dist` (a step capped at
  the current distance went to 1 m of the player); faster and more often (cooldown 1.5 s, strong 3 steps).
- **Zigzag:** from 4 m (`zz_dist`, was 8: it stopped zigzagging 8-10 m out); cats from 6 m (attack jump).

## nlc-3.589.91 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `460876987FFEF6572C3EDC654A9EEFA3038126E23939AC7331669854B523CB15` (archived with its PDB in `nlc_tools/builds/20261013_211208_46087698`).
- **Status:** runtime pending. Bloodsucker toughness and tactics pass after tester feedback ("too easy").

### NLC

- **Mutants and anomalies:** the anomaly detector ran only at rest, so fights and retreats went through anomalies
  (Red Forest bloodsuckers died in them); `anomaly_detect_combat` keeps it on while there is an enemy, with a log line.
- **Zigzag variety (all species):** random leg angle (`zz_angle_rand`), fake legs (`zz_fake_chance`), speed bursts
  (`zz_boost_chance`), short pauses (`zz_pause_chance`) and an early switch when the crosshair is on it
  (`zz_aim_switch`); the fixed angle and rhythm were easy to lead.
- **Impact slow (all species, experiment):** health lost within `impact_slow_window_ms` above `impact_slow_damage`
  slows the monster briefly (`impact_slow_k`, `_k2`); 0 switches it off.
- **Bloodsucker hit and run:** earlier (45/55/65%), more often, back at 90%; a retreat finds its point through the
  ring, the cover finder, an ai-map walk (tunnels) or a straight line, and logs which; extra regeneration rolled per
  retreat, wounds closed; a retreat that makes no way ends in a fight; bolder with mates near; relentless when hurt
  and the retreat is on cooldown; a bait charges when the player reloads.
- **Bloodsucker perch pounce:** always at a reachable perch; a perch latch against flickers; it turns to face the
  player; the spot is re-picked after 1 s.
- **Bloodsucker melee dodge:** diagonal steps around the player while aimed at in melee range, sped up, with a turn
  boost (`melee_dodge_*`); the melee substate hands movement over (`nlc_melee_override`).
- **Bloodsucker grab:** camera turn 2-6 rad/s; the struggle counts at half while the camera still turns.
- **Bloodsucker breathing tell** while cloaked within 6 m (`cloak_breath_dist`, `Sound_Cloak_Breath`).
- **NPCs against a cloaked bloodsucker:** weapon dispersion x3 while invisible, x1.8 in x-ray (`npc_disp_k_*`, in
  `CAI_Stalker::GetWeaponAccuracy` next to the psy factor).
- **Config:** normal +5% and strong +10% speed; weak and normal `health_fire_wound_hit_part` 0.4 (was 0.5).
- **Scripts:** buckshot pellets reach the bloodsucker temple bonus (`sivol_bloodsucker_head.on_pellet`, per shot).

## nlc-3.589.90 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `685E3AD474AF5A79367A7B094F31498E2D543855A2FAEBE91F9FBA572266D7DB` (archived with its PDB in `nlc_tools/builds/20261013_191025_685e3ad4`).
- **Status:** runtime pending ( review fixes ).
- **Required game data:** `rain_patch_normal.ps`, `rain_apply_gloss.ps`, `ssfx_il.ps`.

### NLC

- Review: rain porosity / gloss packing safe in FP16; no NaN in the flashlight IL term; fog scattering not skipped
  while ground mist is on; HUD prefetch flag only for file names ending in `_hud`.

## nlc-3.589.89 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D97B4E39B3111E724DD57736F9B762F62B6094C41E3D19A8CEEAFC4A2C96FC85` (archived with its PDB in `nlc_tools/builds/20261013_185528_d97b4e39`).
- **Status:** runtime pending.
- **Required game data:** `ui_mm_opt_shaders.script`.

### NLC

- `ssfx_compare` is an on / off command ( the Shaders tab checkbox could not switch the integer command ).
- `ssfx_settings_version`: one-time migration of changed defaults for players who update ( grass slope 0.5, player
  grass bend radius 1.2 ), in `CRender::create`.

## nlc-3.589.88 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F981FB64611B1CB9A86BBD89655911D3B505939729B1719E5D97C719C886FB45` (archived with its PDB in `nlc_tools/builds/20261013_184136_f981fb64`).
- **Status:** runtime pending.

### NLC

- **HUD visuals in `prefetch_visuals_*`** ( NLC `prefetch_globals.ltx`, ~375 weapon `*_hud` models ) are prefetched as
  HUD visuals ( `CModelPool::Prefetch` ); they were loaded with world shaders and reused for the HUD, so many weapons
  had no HUD rain drops. `.87` `[hudvis]` diagnostic removed.

## nlc-3.589.87 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `55BB943798F4249CFAC0CA67341B57B58AD214F326960BDB246F4B8F921CB282` (archived with its PDB in `nlc_tools/builds/20261013_183047_55bb9437`).
- **Status:** diagnostic build.

### NLC

- `~ [hudvis]` log line per HUD model part ( shader, texture, compiled pixel shader ): HUD rain drops missing on some
  weapons.

## nlc-3.589.86 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `74C3A1464BB8CC81BBE7F558D680E3BC548E3087A9D4233A3183C56566372160` (archived with its PDB in `nlc_tools/builds/20261013_173701_74c3a146`).
- **Status:** runtime pending.
- **Required game data:** `ssfx_rain.ps`, `effects_rain.ps`, menu files.

### NLC

- `ssfx_look` default NLC; automatic sun shafts also for a higher sun ( weaker ), floor 0.06.
- Rain lit by the player's torch and by volumetric beams ( shader side, `ssfx_torch` constants ).

## nlc-3.589.85 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `006CC5819BDD6BB80CB641A2532C71E094C1DA258EFDF307EB6F8F737F41FC4A` (archived with its PDB in `nlc_tools/builds/20261013_170720_006cc581`).
- **Status:** runtime pending.
- **Required game data:** `ssfx_rain.s/.ps`, `effects_rain.s/.ps`, `img_corrections.h`, presets, menu files.

### NLC

- **Rain refraction buffer** ( `ssfx_rain_beams`, `SSFX_RAIN_PASS`, SSS 22 ): 1/8 HDR scene + volumetric light before
  the forward pass; rain no longer refracts the previous tone-mapped frame.
- **Look presets** ( `ssfx_look` Neutral / NLC / Cold / Classic ), `ssfx_contrast`, `ssfx_color_r/g/b` ( LDR grading in
  `img_corrections.h`, constants `ssfx_look`, `ssfx_color_balance` ).
- **Automatic sun shafts** ( `ssfx_sunshafts` ): floor for a low bright sun in `CEnvDescriptorMixer::lerp`.
- `ssfx_terrain_grass_slope` default 1 -> 0.5; light SSS slots keyed by `light::ssfx_uid`.

## nlc-3.589.84 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CFCBC75C214F4170DB5D81F07ABC204141635A8838ED87322A8ECFEE2D2748DC` (archived with its PDB in `nlc_tools/builds/20261013_054500_cfcbc75c`).
- **Status:** runtime pending (tuning + performance batch).
- **Required game data:** shaders ( `shadow.h`, `combine_1.ps`, `ssfx_il.ps`, `ssfx_mist.h`, `ssfx_extra.h`, `ssfx_fog_sun.h`,
  `ssfx_fog_scattering.ps`, `ssfx_water.ps`, `ssfx_ssr.s`, `ssfx_ssr_combine.ps`, `ssfx_volumetric.s`,
  `ssfx_volumetric_combine.ps`, `ssfx_sss_lights.ps`, `rain_patch_normal.ps`, `rain_apply_gloss.ps`,
  `settings_screenspace_FLORA.h`, HUD rain drops / watch files ), presets, menu files.

### NLC

- **Performance:** grass / tree colour variation behind `SSFX_FLORA_VARIATION` ( off on Minimum / Low ); light SSS
  passes skipped when their 4 slots are empty; SSR combined by blending into `rt_Generic_0` ( no scratch copy ); glass
  copy only after an SSR pass; fog scattering skipped without visible fog; sun SSS skipped at night; sun and spot
  volumetrics share one reduced target; volumetric spot fade only at reduced resolution, from the light's range edge.
- **New:** `ssfx_ao_direct` ( AO on direct light, default 0.3 ), `ssfx_ground_mist` ( `SSFX_GROUND_MIST`, default 0.5 ),
  constant `ssfx_extra2`. Defaults: `ssfx_cloud_shadows` 1, `ssfx_il_flashlight` 1 ( range 0 ~ 2 ).

## nlc-3.589.83 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A14ECCFD6DEBFD9869BDBD40506DD48BD320C496FD77D0F3ABDBAF02B8417267` (archived with its PDB in `nlc_tools/builds/20261013_041832_a14eccfd`).
- **Status:** runtime pending.
- **Required game data:** `combine_1.ps`.

### NLC

- **Wetness reset on same-level save loads** ( they skip `Load_GameSpecific_After` ): armed in
  `CALifeStorageManager::load` too.
- **Flashlight bounce:** normalised torch colour ( brightness = `ssfx_il_flashlight` ), shader multiplies by albedo.

## nlc-3.589.82 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A0A783CC9AFE0548E4A883FFC2A2C3363B29A7A94E1FF9EDAC493558D0A5D06F` (archived with its PDB in `nlc_tools/builds/20261013_035126_a0a783cc`).
- **Status:** runtime pending (`.81` test fixes).
- **Required game data:** `ssfx_il.ps`, `ssfx_il_blur.ps`, `combine_1.ps`, `screenspace_wind.h`,
  `deffer_terrain_high_flat.ps`, menu files.

### NLC

- **Flashlight indirect light** is gathered in the IL alpha and added as light in the torch colour ( was a multiplier
  of the ambient: nothing underground, sky-blue indoors ); `ssfx_il_flashlight` is now the strength ( default 1.1 ).
- **Cloud shadows** default 0.85.
- **Wetness reset log lines** ( `~ [wetness] ...` ) at level load, weather restore and reset.

## nlc-3.589.81 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `E6F4E7B5C84F67C3324920232805F31F06C288FFDFCEA2F893F52949145CF5D1` (archived with its PDB in `nlc_tools/builds/20261013_031355_e6f4e7b5`).
- **Status:** runtime pending (`.80` test fixes, review fixes).
- **Required game data:** `ssfx_cloud_shadows.h`, `accum_sun_cascade0/2.ps`, `screenspace_wind.h`, `ssfx_il.ps`,
  `water_puddles.ps`, `effects_water_puddles.s`, `deffer_terrain_high_flat.ps`, `ssfx_ssr.ps`, `ssfx_water_ssr.ps`,
  `ssfx_ao.ps`, menu files.

### NLC

- **Wetness after a save load:** the weather script restores the saved weather with a forced `set_weather` some seconds
  after the level load; the first forced set inside a 120 s window re-arms the wetness reset ( `.79` / `.80` reset from
  the previous save's weather ).
- **Cloud shadows:** the cloud texture redirection no longer releases a reference it never took ( `surface_get` does
  not AddRef ); `ssfx_debug_clouds 1` debug view.
- **Flashlight indirect light** ( `ssfx_il_flashlight`, default 2 ): `CTorch` publishes the player's torch
  ( `IGame_Persistent::actor_torch` ), binder `ssfx_torch` ( view space ).
- **Review fixes:** SSS / water / SSR histories cleared when their pass resumes after skipped frames; SSFX histories
  reset on camera cuts ( with `ResetTemporalHistory` ).

## nlc-3.589.80 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `61D2F63FDA2DB90AE1BF00E442DDBA4A413F5CA3AC3D0F9760DB9686C324DC64` (archived with its PDB in `nlc_tools/builds/20261013_023139_61d2f63f`).
- **Status:** runtime pending (`.79` test fixes).
- **Required game data:** `accum_sun_cascade0.s`, `accum_sun_cascade2.s`, `screenspace_wind.h`, `water_puddles.ps`,
  `deffer_terrain_high_flat.ps`.

### NLC

- **Grass benders sized by the ground footprint** ( visual box xz half-extent, radius 0.25 ~ 1.5 m ); the bounding
  radius ( height included ) made boars push grass like an aura. Player bend radius default 2 -> 1.2 m
  ( `ssfx_int_grass_params_1.x` ).
- **Footstep ripples fade** over ~1.5 s after the walker stops ( was cut the same frame ).

## nlc-3.589.79 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `6B00994FC890321F120C7BDCE24BC5D7BA54EA02DE35431261279958381004BC` (archived with its PDB in `nlc_tools/builds/20261013_015421_6b00994f`).
- **Status:** runtime pending (wetness fix, Build C extras).

### NLC

- **Wetness:** `wetness_factor` changes per second ( was per frame: soak / dry time depended on the FPS; defaults keep the
  100 FPS timing ) and restarts from the current weather after every level / save load ( a rainy save no longer leaves the
  next one wet ).
- **Grass benders sized by the creature** ( `CMovementManager::on_frame`: radius and strength from the bounding radius ).
- **Cloud shadows** ( `ssfx_cloud_shadows`, `SSFX_CLOUD_SHADOWS` ): the current weather's cloud textures, bound as
  `$user$ssfx_clouds0/1` every frame, projected along the sun in the sun cascades.
- **Footstep ripples** ( `ssfx_step_ripples`, `SSFX_STEP_RIPPLES`, `ssfx_steps` binder: player + nearest moving benders ).
- **Volumetric distance fade** ( spot lights, 20 -> 120 m, skipped beyond ).
- **Specular AA** ( `ssfx_spec_aa`, `SSFX_SPEC_AA` ) and **object motion blur** ( `ssfx_mblur_objects`, `SSFX_MBLUR_OBJECTS` ).
- SSS 23 temporal history reset on level load.

## nlc-3.589.78 (2026-10-13)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A21DE08CD950A3DEF650DB9CDA5B1596798EAD57C3DECAEE27556BD663EAC5E3` (archived with its PDB in `nlc_tools/builds/20261013_004518_a21de08c`).
- **Status:** runtime pending (SSS 23 Build C).
- **Required game data:** `screenspace_wind.h`, `combine_1.ps`, `ssfx_hmodel.h`, `ssfx_fog_sun.h`, `ssfx_extra.h`,
  `ogsr_bloom.ps/.s`, `ssfx_water.ps`, `ssfx_ssr.ps/.s`, `ssfx_ssr_combine.ps`, menu files, presets, anomaly configs.

### NLC

- **Water prepass:** camera matrices are set before the element ( binders such as `m_texgen` run at `set_Element` );
  `.77` traced the water reflections from the last drawn object's matrix ( shader workaround already deployed ).
- **`ssfx_extra` constant** ( x wind gusts, y HUD bloom, z sun-coloured fog, w specular occlusion; neutral with
  `ssfx_compare` ) and cvars `ssfx_wind_gusts`, `ssfx_bloom_hud`, `ssfx_fog_sun`, `ssfx_spec_occlusion`;
  macros `SSFX_WIND_GUSTS`, `SSFX_FOG_SUN` when the value is > 0 at game start.
- **`ssfx_ssr_resolution` 1 / 2:** SSS 23 SSR targets at full or half size ( `vid_restart` ).

## nlc-3.589.77 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `AF1F470576390FE8F748A4C265B235C04216D52B5599F90EA5F23D8B286F552F` (archived with its PDB in `nlc_tools/builds/20261012_205227_af1f4705`).
- **Status:** runtime pending (SSS 23 Build B, `docs/SSS_UPGRADE_DESIGN.md`).
- **Required game data:** Build B shaders ( `ssfx_water*`, `ssfx_fog_scattering*`, `ssfx_volumetric*`, `ssfx_ssr*`,
  `ssfx_glass*`, `ssfx_tree_branch.h`, `deffer_tree_branch_aref_*`, overrides of `effects_water*.s`,
  `accum_volumetric_sun.ps`, `deffer_tree_branch_bump-hq.vs`, `deffer_base_aref_bump-hq.ps` ), textures
  `fx\ssfx_water_*`, `water\ssfx_water_perlin`, `fx\ssfx_glass_normal`, menu files.

### NLC

- **SSS 23 water:** water surfaces ( `effects_water*`, not puddles ) are flagged at shader creation
  ( `ShaderElement::flags.isWater` ) and collected per frame ( `R_dsgraph_structure::waterNodes` ). A reflection prepass
  draws them into reduced-size targets ( `ssfx_water_resolution` ), accumulates over frames and blurs; a 512x512 wave
  height target feeds the parallax waves. The surfaces still render in the normal graph with their own `.s`; their
  pixel shaders fall back to the OGSR `water.ps` when the feature is off. `ssfx_water_enable`, `ssfx_water_quality`,
  `ssfx_water_parallax` ( game restart ), `ssfx_water_blur`, `ssfx_water_reflection`.
- **Fog scattering:** `ssfx_fog_scattering` ( 0 = off ), HDR pass before bloom with its own 1/4 and 1/2 targets.
- **Volumetric lights at reduced resolution:** `ssfx_volumetric_resolution` 1-4 ( 1 = old full-resolution path ).
  Spot and sun volumetrics render into reduced targets without a depth buffer ( both shaders have no depth test ),
  combined with a depth-aware 4x4 filter ( `ssfx_volumetric_combine.ps` ).
- **SSR:** `ssfx_ssr_enable` ( game restart, needs `r_sslr_enable` ) replaces the OGSR SSR pass: reflected-point
  reprojection, disocclusion and HUD rejection, 16-bit history, gloss-based blur ( `ssfx_ssr_blur` ); `ssfx_ssr_dry`
  > 0 also reflects on dry glossy surfaces.
- **Glass:** `ssfx_glass_enable` ( game restart ) gives transparent `Model_EbB` glass ( not HUD ) refraction, chromatic
  aberration and fresnel ( `ssfx_glass` ).
- **Foliage:** with `ssfx_tree_variation` > 0 or `ssfx_foliage_mask` at game start, tree branches use their own pixel
  shaders ( `deffer_tree_branch_aref_*` ): always foliage, per-tree colour variation. `ssfx_foliage_mask 1` gives other
  alpha-tested surfaces ( fences, nets ) their own material instead of the foliage material.
- `ssfx_compare 1` also shows the old water, glass, SSR and full-resolution volumetrics, skips fog scattering and
  removes grass / tree colour variation and the foliage mask.

## nlc-3.589.76 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C33500C66E62B7185665EB840F9829D175FCB6DD9D1706CF74A460B4A0394783` (archived with its PDB in `nlc_tools/builds/20261012_195119_c33500c6`).
- **Status:** runtime pending (SSS 23 Build A menu fixes after the `.75` test).
- **Required game data:** as `.75` ( menu files changed: `ui_mm_opt.xml`, `ui_mm_opt_main.script`,
  `ui_mm_opt_shaders.script`, `ui_mm_opt_video_adv.script`, `ui_st_mm.xml` eng/rus ).

### NLC

- **`ssfx_compare` is session-only** ( `CCC_SsfxCompare`, never written to `user.ltx` ), so a forgotten comparison cannot
  hide the SSS 23 passes in later sessions.
- Menu ( game data ): the effect switches ( AO type/quality, indirect light, contact shadows ) moved into the Shaders
  panel and its Defaults button turns the effects on; option group renamed `mm_opt_video_shaders` because OGSR clears
  the `vid_restart` flag only for `mm_opt_video*` groups ( `.75` restarted the renderer on every Accept ).

## nlc-3.589.75 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `5112ACABCD1AC64AFFA3974C53BD314645EBD43C33D0AC4DE96A27F45558031B` (archived with its PDB in `nlc_tools/builds/20261012_193448_5112acab`).
- **Status:** runtime pending (SSS 23 Build A fixes after the first `.74` test).
- **Required game data:** as `.74`, plus `ui_mm_opt_video.script`, `ui_mm_opt_video_adv.script`.

### NLC

- **SSS 23 passes, session snapshot:** `.74` read the pass switches at every `vid_restart`, but a renderer restart does
  not recompile shaders, so passes enabled in the menu never reached the compiled `combine_1` / cascade shaders ( and a
  pass disabled in-session would have left the shaders sampling a released target ). The switches, quality levels and
  lamp shadows are now a snapshot taken once per game session ( `ssfx_features.h`, logged as `* [SSFX] session state` );
  they apply after a game restart. Resolution dividers still apply on `vid_restart`.
- **`ssfx_compare` ( 0/1 ):** skips all SSS 23 passes and makes their consumers neutral, without a restart, for
  screenshot and FPS comparison; history restarts when it is switched off.
- **`ssfx_sss_length` 0** turns screen-space shadows off ( range 0-2 ).

## nlc-3.589.74 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A85D465F88815A3D7E6A2FB7B7AF26D6ED6EAF40049A761A51A5D6E18195C5E5` (archived with its PDB in `nlc_tools/builds/20261012_184726_a85d465f`).
- **Status:** runtime pending (SSS 23 Build A, `docs/SSS_UPGRADE_DESIGN.md`).
- **Required game data:** `gamedata/shaders/r3/ssfx_*.{ps,s,h}`, `accum_base.ps`, `accum_sun_cascade0/2.{ps,s}`,
  `accum_spot.s`, `accum_omni.s`, `combine.s`, `combine_1.ps`, `combine_2_naa.ps`, `settings_screenspace_SSS.h`,
  `deffer_grass.{vs,ps}`, `settings_screenspace_FLORA.h`; options menu: `ui_mm_opt.xml`, `ui_mm_opt_shaders.script`,
  `ui_mm_opt_main.script`, `ui_st_mm.xml` ( eng/rus ); presets `rspec_*.ltx`.

### NLC

- **SSS 23 temporal passes ( `r4_rendertarget_phase_ssfx.cpp`, `ssfx_features.h` ):** ports of the Screen Space Shaders
  23 AO, indirect light and screen-space shadows as separate passes that reproject their previous result with the OGSR
  velocity buffer and reject it with a copy of the previous position buffer.
  - Sun screen-space shadows before sun accumulation ( `ssfx_sss_enable` ), read by the cascade shaders.
  - Light screen-space shadows for up to 8 shadowed spot/omni lights ( `ssfx_sss_lights` ), slot per light in
    `light::ssfx_sss_id`, read by `accum_base.ps` through the `ssfx_sss_id` constant.
  - AO as a new `r_ao_mode` token `st_ssfx_ao` at `ssfx_ao_resolution` ( 1-4 divider ), quality from `r2_ssao`.
  - Indirect light at `ssfx_il_resolution` ( 2-8 divider ) when `ssfx_indirect_light` is on; replaces the inline
    Update 14.5 IL.
  - HUD mask from the depth copy ( HUD drawn in `[0, 0.02]` depth ) instead of an extra HUD render pass.
  - Targets exist only for enabled passes; toggles and resolution/quality changes take effect on `vid_restart`
    ( `UIOptionsItem.cpp` list ). New commands: `ssfx_ao_resolution`, `ssfx_ao_intensity`, `ssfx_ao_radius`,
    `ssfx_il_resolution`, `ssfx_il_quality`, `ssfx_il_intensity`, `ssfx_sss_quality`, `ssfx_sss_lights`,
    `ssfx_sss_length`, `ssfx_flora_variation`.
- **Grass colour variation constant:** `ssfx_flora_variation` ( 0 = off ) bound for `deffer_grass.ps`; changes apply
  immediately.
- **Deterministic grass placement:** `CDetailManager::cache_Decompress` seeds a local `CRandom` per detail slot
  ( new `CRandom(u32 seed)` ). Placement no longer depends on decompression order or thread, so the same spot gets the
  same grass on every load.

## nlc-3.589.73 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `B8C594361C9CC1A3CAE922C1A8CC20972CDD46E74AE04DCB3CD07C7C0CF0831F` (archived with its PDB in `nlc_tools/builds/20261012_014144_b8c59436`).
- **Status:** runtime pending.

### NLC

- **Actor shadow legs, rotation only:** the `.72` leg-bone callbacks copied the puppet's model-space bone matrices, so the
  actor mesh was pulled between joints of another rig and the boot soles stretched in the shadow. The callbacks now take
  the puppet bone's rotation relative to its parent and keep the actor bone's own local offset
  (`new = actor_parent x [puppet_local_rotation | actor_local_translation]`).

## nlc-3.589.72 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `495BB151F05F319CD56AF28D19C03F07DF695662DE080F970A860AFAEB3A2FD1` (archived with its PDB in `nlc_tools/builds/20261012_012649_495bb151`).
- **Status:** runtime pending (visible body / kick pass).
- **Required game data:** `gamedata/scripts/sivol/sivol_visible_body.script`, `sivol_kick.script`, `sivol_kick_configs.script`.

### NLC

- **Kick kills are ragdoll kills:** `CCharacterPhysicsSupport::KillHit` treats `eHitTypePhysicStrike` (used only by the
  actor's kick) like the pseudogiant stomp: no death animation. A death animation took 90 % of the impulse
  (`H.impulse *= 0.1`) and the body collapsed on the spot (tushkanos: about 1 m/s from a 250 impulse).
- **Actor shadow follows the visible body:** the visible-body puppet never reaches the shadow maps and the actor model's
  shadow (`g_actor_shadow`) had another animation set, so it desynced from the legs the player sees. While the script
  reports the puppet (`level.nlc_visible_body_active(active, id)`), bone callbacks on the actor model's eight leg bones
  copy the puppet's current model-space leg pose; the shadow is the actor's torso/arms/weapon on the visible legs. The
  callbacks are installed on demand, never replace another callback, and are removed when the puppet goes away.
  `g_actor_shadow_legs_from_body 0` disables it. The explicit puppet render and `g_actor_shadow_with_body` of `.70`/`.71`
  are gone.

## nlc-3.589.71 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1FCEEAAA622FA8879BC2310F54376AD04CEBF12A5FAD6542A536FDC0A4E326AA` (archived with its PDB in `nlc_tools/builds/20261012_010810_1fceeaaa`).
- **Status:** runtime pending (visible body / kick pass).
- **Required game data:** `gamedata/scripts/sivol/sivol_visible_body.script` (calls `level.nlc_visible_body_active(active, id)`).

### NLC

- **Visible-body shadow:** `level.nlc_visible_body_active` takes the puppet's object id. With `g_actor_shadow` on and the
  puppet shown, `CHUDManager::Render_Actor_Shadow` renders the puppet into the shadow maps in place of the actor model
  (the puppet did not reach them through the normal traversal, and the actor model's shadow desynced from the legs).
  `g_actor_shadow_with_body 1` keeps the actor model's shadow.

## nlc-3.589.70 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7D9ABC443163B8F69FDD7A3CC2FEEE891720DF318A45E82499823FE98A5177C1` (archived with its PDB in `nlc_tools/builds/20261012_005213_7d9abc44`).
- **Status:** runtime pending (visible body / kick pass).
- **Required game data:** `gamedata/scripts/sivol/sivol_visible_body.script` (calls `level.nlc_visible_body_active`).

### NLC

- **Actor shadow vs visible body:** `g_actor_shadow` renders the actor model, which has another animation set and sits
  behind the visible-body puppet, so its shadow doubled and desynced the puppet's own. The script reports the puppet through
  `level.nlc_visible_body_active(bool)`; while it is shown the actor model's shadow is skipped
  (`g_actor_shadow_with_body 1` restores both).
- **Stealth/burer ignore all body variants:** `nlc_stealth::is_visible_body` compared the section with `actor_legs`
  exactly, so the outfit variants (`actor_legs_<mesh>`) were treated as ordinary monsters; it matches the prefix now.

## nlc-3.589.69 (2026-10-12)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4111D70FF47050B490C4EC56BC92C7903266F626C6A3C55D405ABCA5A428A3AE` (archived with its PDB in `nlc_tools/builds/20261012_003546_4111d70f`).
- **Status:** runtime pending (visible body / kick pass).
- **Required game data:** `gamedata/scripts/sivol/sivol_visible_body.script`, `sivol_kick.script`, `sivol_kick_configs.script`
  (all three feature-detect the new bindings and keep a fallback on older exes).

### NLC

- **`game_object:nlc_set_npc_transform(pos, dir)`:** position and heading of a script-driven monster in one call
  (`script_game_object2.cpp`). `set_npc_position` keeps the rotation and the heading otherwise comes from the look action,
  which the AI processes only in its 100-250 ms schedule tick, so the visible-body puppet slid to its position first and
  turned later. The heading is written into XFORM, `movement().m_body` and the monster direction controls
  (`CControlDirection::nlc_force_heading`) so `update_frame()` does not lerp it back.
- **`game_object:nlc_push(dir, speed, up)`:** sets the velocity of a living character directly (the burer's
  `CBurer::nlc_push`, without the actor hold). ApplyImpulse on a character is cancelled by the ground controller within a
  few steps. Used by the kick for tushkanos (`push_speed`/`push_up` in `sivol_kick_configs.script`).
- **`game_object:nlc_set_cycle_speed(scale)`:** playback speed of the running (non-FX) cycles as a multiple of their
  authored motion-def speed; the visible body scales walk/run/sprint clips by the measured speed.

## nlc-3.589.68 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `0F0A5396657BC6195622CBCC4E809753EABC69B899841BD16F071419F5A6F64C` (archived with its PDB in `nlc_tools/builds/20261011_212245_0f0a5396`).
- **Status:** runtime pending (`docs/BLOOD_POOLS.md`).
- **Required game data:** `gamedata/shaders/r3/effects_wallmark_blood_soft.{s,ps}` (without the `.s` the blood marks
  cannot be created), `gamedata/config/misc/blood_pool_surfaces.ltx`, `gamedata/scripts/sivol/sivol_blood_pools.script`,
  `gamedata/scripts/se/se_monster.script` (extra saved string per monster).

### NLC

- **Soft-ground blood:** blood wallmarks (`wm_blood_*`, pools, splats, drops) use a second shader on earth/grass/sand:
  the blood is multiplied into the ground colour (texture shows through, no gloss). The class comes from the hit
  material name (`CGameMtlLibrary::NLCSoftSurface`, `misclood_pool_surfaces.ltx`); `bpm mtl` lists the materials.
- **Rain wash-off:** open-air pools (upward ray at stamp time) fade gradually while it rains
  (`ps_wm_wash_seconds`, default 240 s of full rain; `bpm wash <s>`). A washed pool leaves an empty marker so the
  script's refresh cannot bring it back; it returns after a reload.
- **Faint old stains:** `bpm` argument `faint` takes a share of the opacity away for good; owner ids go up to 16777215
  (one per pool of a corpse).

## nlc-3.589.67 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `BD24EE72D01CF83ABE54457726B58B1C7135C874CCBA1A7E3E4D7EE4E3D2A1FA` (archived with its PDB in `nlc_tools/builds/20261011_192548_bd24ee72`).
- **Status:** runtime pending (`docs/BLOOD_POOLS.md` section 6). Build `.66` (giant buff, high-jump AI fix) has no entry here.
- **Required game data:** `gamedata/scripts/sivol/sivol_blood_pools.script`, `gamedata/shaders/r3/effects_wallmark_blood.ps`
  (loose override: texture-window mask, drying tint), `gamedata/textures/wm/wm_blood_pool_*.dds` (mip chains).

### NLC

- **Static wallmark parameters (`wm_static_params`, `IRender_interface::add_StaticWallmark` overload):** per-mark life
  (overrides `r__wallmark_ttl`; the mark stays solid and fades over the last quarter), spread time, drying tint, age,
  explicit rotation and an owner id. A mark with an owner replaces that owner's earlier mark in any slot and keeps its age;
  owned marks never merge by the 2 cm position test. Vanilla callers are unchanged.
- **`bpm x y z dist size [tex [life [grow [rot_deg [owner [dry [age]]]]]]]`:** drives the above. Spreading no longer
  rebuilds the geometry: it is built once at full size and `static_wm_render` shrinks the texture window per frame.
  Inputs are clamped; bad arguments are counted (`last_reason=9`).
- **`ps_wm_rotfix` now defaults to 0:** blood pools pass an explicit rotation, so bullet holes and splats get their vanilla
  random rotation back (`bpm rot 1` still forces the fixed one).
- **`CWallmarksEngine::Render`:** slots without marks no longer lock the vertex buffer (twice per slot per frame).
- **Blood pools script:** corpse-driven (restored after load, refreshed while the player is near, moved with the body,
  amount from the hits taken, no pool for fire/shock/radiation/psy deaths); see `docs/BLOOD_POOLS.md`.

## nlc-3.589.65 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `61A1737E3239F6C95BDE6D54017CBF3EA0BC3D34C98315AFF4BA5CEC8B0D7EAC` (archived with its PDB in `nlc_tools/builds/20261011_184422_61a1737e`).
- **Status:** runtime pending.

### NLC

- **Explosion blame (stock bug):** `CExplosiveItem::Hit` tested `Initiator() == u16(-1)`, which never holds, so a shot
  canister / gas cylinder (`amk_kanistra`, `amk_ballon`, explosive barrels) credited nobody and friendly NPCs could be
  killed unpunished. It now records whoever set it off; `CExplosive::nlc_blame` passes the owner of a hitting
  explosive on, so chains (shot canister -> grenade) blame the first shooter (also in `CGrenade::Hit`).

## nlc-3.589.64 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `36CFB24A2FE97AFD6F992DD52CE6E471635D1DB7EDC21539B08014968F0A7A8E` (archived with its PDB in `nlc_tools/builds/20261011_174621_36cfb24a`).
- **Status:** runtime pending (`docs/BURER_REDESIGN.md` 5.21).

### NLC

- **Burer grudge hunt:** with no peek point nearby it walks toward the enemy's last known position, stopping 8 m short
  (`move search`); it stood still behind a large rock before.

## nlc-3.589.63 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8D3C5E6CE0CFA03A8F8D8C40D4B51DE9E09A35A15A674F4E2E026CAA4CC0250E` (archived with its PDB in `nlc_tools/builds/20261011_173347_8d3c5e6c`).
- **Status:** runtime pending. After the `.62` tests (`docs/BURER_REDESIGN.md` 5.20).

### NLC

- **Burer puppet gun:** a gun is thrown only when empty, at the actor, in sight (a loaded gun or one landing at a
  stalker re-armed the enemy); put down after `nlc_puppet_blind_ms` without a shot or line of fire.
- **Burer tele:** at the end, held objects are thrown only at an enemy in sight; guns thrown faster
  (`nlc_tele_weapon_speed_k`); every telekinetic throw flies without air drag (`CTelekineticObject::fire_t`).
- **Barricade:** spring gain and acceleration per object (`CTelekineticObject::m_nlc_gain`, `nlc_barricade_gain` /
  `_accel`); faster broad-side turn.
- **Grudge:** `grudge: hunting` diagnostic.

## nlc-3.589.62 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `E7B4CF256436F00295C809FD8B6E1982E558F31A7FB7A14A38A44D4B159D773D` (archived with its PDB in `nlc_tools/builds/20261011_152135_e7b4cf25`).
- **Status:** runtime pending. Spare pistol for disarmed stalkers (`docs/BURER_REDESIGN.md` 5.19).

### NLC

- **Burer NPC disarm:** a stalker without another gun it can fire gets a worn spare pistol and a box at the disarm
  (`nlc_npc_disarm_spare*`, `CBurer::nlc_npc_give_spare`); with a spare configured every main-gun stalker can be disarmed.

## nlc-3.589.61 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `133A6B8E226A7160372421DA4420D5A5F2A5032F507F5DEBA1292C17D3F91B15` (archived with its PDB in `nlc_tools/builds/20261011_150322_133a6b8e`).
- **Status:** runtime pending. Fixes from the review of the burer redesign (`docs/BURER_REDESIGN.md` 5.17-5.18).

### NLC

- **Crash fix:** `CExplosiveRocket::net_Relcase` clears a raw owner that is being destroyed (caught / puppet rounds).
- **Burer:** an interrupted tactical move ends its bookkeeping (`critical_finalize`); objects held by any burer are
  skipped by every search (`CBurer::nlc_held_by_any`); gaze effect switched off on `net_Destroy`; launchers are not
  disarmed; `m_nlc_busy` reset outside the attack state.
- **Performance:** failed tactical picks retried after 750 ms; round scan every 150 ms; barricade extents every 100 ms.
- `Grenade.h` default of `m_nlc_detonation_explosion_hit` matches `Load` (5).

## nlc-3.589.60 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D0C73EE29F65878F77619710BE5F9A980CC579A03A4555302613B540AD828D19` (archived with its PDB in `nlc_tools/builds/20261011_050830_d0c73ee2`).
- **Status:** runtime pending. Sidearms, grudge, tells, hospital guns (`docs/BURER_REDESIGN.md` 5.16).

### NLC

- **Burer NPC disarm:** only stalkers left with another usable gun (`nlc_npc_disarm_need_sidearm`).
- **Burer grudge** (`nlc_grudge_ms`): the enemy that hurt it stays pinned in memory; it peeks for a line of sight.
- **Burer overwatch:** settle time after arriving, no points under 3 m.
- **Burer shield catch tell:** flare at every caught bullet before the volley, faster glow pulse near the shield's end.

## nlc-3.589.59 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `53DF1355A180445C4E566BB688817744883F6B4D6CF74143CD9D88DC93D21CB8` (archived with its PDB in `nlc_tools/builds/20261011_041056_53df1355`).
- **Status:** runtime pending. Burers vs stalkers (`docs/BURER_REDESIGN.md` 5.15).

### NLC

- **Burer tele:** no guns lifted or thrown while fighting a stalker (`CBurer::nlc_tele_skip`); cooldown also on
  `critical_finalize` (a tele cut short by damage restarted every few frames); empty object searches throttled.
- **Burer NPC disarm:** timing (magazine share, not reloading), gun flung toward the burer and locked for pickup, short
  seize delay for it, weapons-left log; stalkers aiming at the burer fill the threat meter.
- **Burer shield catch:** the shield's bullet glow replayed at each caught bullet (`nlc_catch_glow`).
- **Burer:** overwatch ring from the enemy's position; a grenade held 4 s is thrown at a target from 5 m.

## nlc-3.589.58 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C3450A98850DF81A16CEAA65ED149D038D06CDC28415BFC973AD272CB75FEF0D` (archived with its PDB in `nlc_tools/builds/20261011_025201_c3450a98`).
- **Status:** runtime pending. After the `.57` test (`docs/BURER_REDESIGN.md` 5.14).

### NLC

- **Crash fix (stock):** `CUISequencer` forwarded input to the receiver stored at its start; a tutorial started during
  `demo_record` crashed in `IR_OnKeyboardHold` once the demo recorder was gone. It now forwards only while that
  receiver is still on the input stack (`CInput::nlc_has_receiver`).
- **Burer push:** the character is woken before the velocity is set (a standing actor's sleeping body ignored it);
  logs read the real body velocity.
- **Burer barricade:** height from the physics shell's extents (a corpse's bind-pose box lifted it above the burer);
  `barricade hold` log of bottom / top height.
- **Burer:** no stare window during a tactical move; god mode logged when it skips the stamina attack.

## nlc-3.589.57 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7F94708B6E4FCFBD680E6A306EE5271916269386C7917ADFF068EED4725404B7` (archived with its PDB in `nlc_tools/builds/20261011_023939_7f94708b`).
- **Status:** runtime pending. Burer pass 5 (`docs/BURER_REDESIGN.md` 5.13).

### NLC

- **Burer barricade fit:** box centre at chest height over the ground under the post, lowest point >= 0.2 m, broad side
  to the enemy (`CTelekineticObject::m_nlc_axis` / `m_nlc_axis_twoway` / turn rate), effort tremor; experimental
  one-bone grip for corpses (`m_nlc_grip`, `nlc_barricade_grip`).
- **Gravi push** the frame after the hit, away from the burer; push velocity logs.
- **NPC disarm** state (stalkers aiming a main weapon at the burer), lead for any moving target, puppet-gun taker log.
- **Grenade snatch** (`CGrenade::nlc_snatch_arm`): the actor's grenade is dropped, armed and held by the burer.
- **Shield bullet catch:** blocked bullets held and returned through the bullet manager, dropped on a break.
- **Pull** state against an elevated enemy; **elevation** tracking with a memory pin and overwatch / siege-hide moves.
- **Gaze meter** with a stare-window state, forcing the stock anti-aim stamina attack; script effects
  (`bind_monster.burer_gaze`); the script stare is removed.

## nlc-3.589.56 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1BEFB583DD0EFD3F0402CAC5AC4A90EE629A938D90DBBE1F877767B6D9171B1C` (archived with its PDB in `nlc_tools/builds/20261011_005924_1befb583`).
- **Status:** runtime pending. After the `.55` test (`docs/BURER_REDESIGN.md` 5.11).

### NLC

- **Burer barricade:** dropped only after 2 s beyond `nlc_barricade_leash` from its post (a corpse pulled in from 6-10 m
  was dropped at once), held object speed `nlc_barricade_speed` 9 m/s (5 could not keep up with a running burer).
- **Puppet gun:** keep-away moves at most once a second; seize log shows launcher rounds and the attached round object.

## nlc-3.589.55 (2026-10-11)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `ABAFDDB2ACDFEAD8536B1B1554326005838D9CC546DCC11C7F992188DDDBACFE` (archived with its PDB in `nlc_tools/builds/20261011_003936_abafddb2`).
- **Status:** runtime pending. After the `.54` test (`docs/BURER_REDESIGN.md` 5.10).

### NLC

- **Telekinetic spring steering** (`CTelekineticObject::nlc_steer`, `m_nlc_max_accel`): eased moves and turns.
- **Burer puppet gun:** one flow for both gun modes (launcher mode switched to the rifle at the seize); launched round's
  owner = the gun (it exploded on its own launcher); capped jams with sounds and a 3 s unjam
  (`CWeaponMagazined::nlc_puppet_shot(..., jam_cap)`, `nlc_puppet_sound`); hover points kept 2.5 s, moved after 0.6 s
  without a line of fire, rising first.
- **Burer barricade** as a free action in its own container (`nlc_barricade_mode` 1 / 2); own gravi / repulse skip
  objects it holds.
- **Knockback:** gravi hit pushes (`nlc_gravi_push_*`), repulse falloff / hold keys, shared `CBurer::nlc_push` with a
  displacement log.

## nlc-3.589.54 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F52C7AF20ABB6D40BA7BCAC3BAA8574FD0AD15C516B80C09054305A1ADDB9852` (archived with its PDB in `nlc_tools/builds/20261010_230618_f52c7af2`).
- **Status:** runtime pending. After the `.53` test and crash (`docs/BURER_REDESIGN.md` 5.9).

### NLC

- **Crash fix:** `CPHSimpleCharacter::HitCallback` looks the touched object's callback up again by id instead of
  returning the pointer cached at contact time (a telekinetic re-throw, a release or the object's destruction freed
  it before `CPHMovementControl::Calculate` called it: access violation in `Calculate`, line 168).
- **Burer puppet UBGL:** works in launcher mode (round first, then `PerformSwitchGL` to the rifle) and in rifle mode;
  retried until the target is `nlc_puppet_ubgl_min_dist` away (was spent silently when closer).
- **Puppet gun:** stronger two-wave bob, vertical bounce per shot.

## nlc-3.589.53 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `0F41710BC4CA6416E4EA202D3076BB258B4D02566B8290F7C3EA94A9751660E4` (archived with its PDB in `nlc_tools/builds/20261010_225057_0f41710b`).
- **Status:** runtime pending. Burer round after the `.52` test (`docs/BURER_REDESIGN.md` 5.8).

### NLC

- **Burer puppet gun:** UBGL rifles qualify (only RPG-7 / RG-6 excluded); wobble, tumble, overshoot, sag, pre-burst
  lean; one underbarrel round after 6-8 s (`CWeaponMagazinedWGrenade::nlc_puppet_launch`).
- **Burer shield:** strain in health (`nlc_shield_strain_hp`), graded leak, break reaction (psi pulse, cover, wrench);
  passive deflection (`nlc_deflect_chance`). `CEntityCondition::nlc_health_hit_part`.
- **Burer round counter:** VOG / RPG rounds caught, held and returned (`CCustomRocket::nlc_fuse_pause / nlc_set_owner`,
  `CExplosiveRocket::nlc_clear_safe_dist`).
- **Burer brain:** move arrival handled on completion (holds now stick), cover retry, peek only when ready, straight
  advance after a failed cover advance. Eased camera wrench with pitch, overshoot and shake.

## nlc-3.589.52 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1CB7ACD29959D4830B5259FEB85AD3CB17F73C574926F1EE113E92CA51D20E90` (archived with its PDB in `nlc_tools/builds/20261010_220324_1cb7acd2`).
- **Status:** runtime pending. Burer passes 2-4 combined (`docs/BURER_REDESIGN.md` 5.7).

### NLC

- **Burer puppet gun:** loaded guns lying around are raised, steered, aimed and fired at the enemy
  (`CWeaponMagazined::nlc_puppet_shot`, shots credited to the burer via `CWeapon::m_nlc_shot_owner`); thrown when
  empty or jammed. Held items cannot be taken (`CInventoryItem::m_nlc_tele_held`, lock after release).
- **Telekinetic steering:** `CTelekineticObject::nlc_steer` (target point + barrel axis), `CTelekinesis::find_object`.
- **Burer defenses:** threat meter (crosshair on the burer) raises the shield without a state change; shield strain;
  camera wrench for strong / hos; a size-checked barricade object (or an allowed corpse) held in front while it
  channels tele.
- **Burer brain:** health phases, `eStateBurerAttack_Tactical` cover / advance / peek moves, gravi only along a clear
  line, tele vs NPC enemies.
- New file `xrGame/ai/monsters/burer/burer_nlc.cpp` (project updated).

## nlc-3.589.51 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `ACBA64D93D183AC430AC2AE12EF2D9E39506B1B23F868549C860DF5691D9E64A` (archived with its PDB in `nlc_tools/builds/20261010_212836_acba64d9`).
- **Status:** runtime pending. Burer pass 1 round 5 (`xray_hawkl_10-10-26_21-11-16.log`, `docs/BURER_REDESIGN.md` 5.6).

### NLC

- **Eaten grenades fixed:** a throw interrupted after release left `CGrenade::m_thrown` set and `Throw` skipped every
  later throw; any state change after release now runs the `eThrowEnd` cleanup (`nlc_finish_thrown`).
- **Thrown inventory items hurt:** `CPhysicsShellHolder` stores a collision-hit callback by default (was a no-op).
- **Burer repulse** sets the target's velocity (`nlc_repulse_push_*`) instead of `ApplyImpulse`, which the character
  controller turns downward or cancels; **burer tele** pauses `nlc_tele_cooldown` between tele states.

## nlc-3.589.50 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `3EEE07B9BA8EDB5C4197018974B80922CC3CA10BDD10827D5B0B14A9C1164D13` (archived with its PDB in `nlc_tools/builds/20261010_211021_3eee07b9`).
- **Status:** runtime pending. Burer pass 1 round 4 after the `.49` test (`xray_hawkl_10-10-26_20-51-18.log`, `docs/BURER_REDESIGN.md` 5.5).

### NLC

- **Burer gravi:** inventory items (a just-disarmed gun) get `nlc_gravi_item_impulse` x mass instead of
  `Gravi_Impulse_To_Objects`; no wave within `nlc_gravi_after_disarm_ms` of any burer disarm
  (`bind_monster.burer_disarm_age`).
- **Grenades:** explosion detonation threshold default 0.5 -> 5 (`nlc_detonation_explosion_hit`); the burer's own
  thrown-back grenade set off the next held one from about 12 m.
- **Diagnostics:** `top state` lines from `CStateManagerBurer`; `~ [grenade]` lines for the actor's grenade
  states, throw and release (same `nlc_burer_debug` switch).

## nlc-3.589.49 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `6418EF03812E6E837CC3B703AD56C1C1823208C5FED6926A4DB44D8A0975A041` (archived with its PDB in `nlc_tools/builds/20261010_204831_6418ef03`).
- **Status:** runtime pending. Burer pass 1 round 3 after the `.48` test (`xray_hawkl_10-10-26_20-32-40.log`, `docs/BURER_REDESIGN.md` 5.4).

### NLC

- **Grenades detonate on hits:** any parentless grenade that is not exploding yet explodes when a bullet hits it
  (`nlc_detonate_by_bullet`, default true) or an explosion hits it with at least `nlc_detonation_explosion_hit` (0.5);
  the shooter / blast source is the initiator. Stock tested `CExplosive::Initiator() == u16(-1)`, which never holds.
- **Burer gravi wave** no longer shoves grenades (500 x mass launched thrown grenades out of sight: the "vanishing"
  grenades near an aggroed burer).
- **Tele damage on bone pieces:** `CPhysicsSkeletonObject` (`ph_skeleton_object`) owns a collision-hit callback slot;
  `nlc_tele_set_hit_callback` attaches to any object that keeps it (read back, freed otherwise).
- **Burer repulse** skips the visible-body helper (`actor_legs`); stronger push in `m_burer.ltx`.
- `tele end` logged once per tele state.

## nlc-3.589.48 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `10E577E3CE618F82B83B711351A22EDD302DB72733D056604B60BC7357E1B953` (archived with its PDB in `nlc_tools/builds/20261010_203122_10e577e3`).
- **Status:** runtime pending. Burer pass 1 round 2 after the user's `.47` test (`docs/BURER_REDESIGN.md` 5.3).

### NLC

- **Burer grenades:** `nlc_grenade_fuse_extra_ms` (airburst fuse = flight + 100 ms + extra; 1500 in `m_burer.ltx`);
  hold time from `nlc_grenade_min_hold_ms` (2000 in the config).
- **Burer repulse push:** `nlc_repulse_impulse` is now x the target's movement mass (the first version passed a raw 300
  to `ApplyImpulse`, about 1/140 of the stock controller pull), plus `nlc_repulse_lift`.
- **Burer diagnostics:** `nlc_burer_debug` enables `~ [burer]` lines (attack states, tele, gravi, shield, anti-aim,
  grenades, repulse, scan).

## nlc-3.589.47 (2026-10-10)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `627C82A9C148F15A11C9B3508C145D26D3457555C0A9935F6C73363E87E4AD17` (archived with its PDB in `nlc_tools/builds/20261010_193131_627c82a9`).
- **Status:** runtime pending. Burer pass 1 (`docs/BURER_REDESIGN.md` 5.2).

### NLC

- **Burer tele throws hurt:** the poltergeist's collision-damage callback (`SCollisionHitCallback`) moved to the shared
  `ai/monsters/telekinesis_hit_callback.h` (poltergeist unchanged); burer throws attach it with a mass-scaled hit
  (`nlc_tele_damage_min/max`, `nlc_tele_damage_mass_min/max`, props only, expires 3 s after the throw), lead a moving
  actor (`nlc_tele_lead`), and a kept object is only thrown along a clear path to the head. Slower throws via
  `tele_fly_velocity` in `m_burer.ltx`.
- **Burer grenade catch:** own telekinesis container in `CBurer`, active in every state (`HandleGrenades` removed
  from the tele state). Catches live grenades (`nlc_grenade_*`), pauses the fuse (`CMissile::nlc_fuse_pause/resume`),
  stops them dead, holds them (`CGrenade::m_nlc_tele_held`: not takeable), throws them back at the thrower or the
  enemy when visible and at least `nlc_grenade_safe_dist` away, fuse set to the flight time (`nlc_grenade_airburst`),
  initiator = the burer. A held grenade explodes when shot (initiator = the shooter). Death/destroy drops held
  grenades with their stored fuse.
- **Burer repulse:** new attack substate `eStateBurerAttack_Repulse` (before gravi): springboard blast within
  `nlc_repulse_radius` when the enemy is close (`nlc_repulse_*`): strike hit + push (movement-control impulse) on
  entities, radial impulse on props and bodies, `bind_monster.burer_repulse_actor` raises the camera inertia floor.
- **Burer scan feedback:** the old `scan_sound` + `scan_effector_section` play when the M2 psy sense level crosses
  `nlc_sense_notice` (`nlc_scan_fx`, `nlc_scan_fx_cooldown`; one burer at a time).
- **Burer disarm grace:** `CBurer::StaminaHit` asks `bind_monster.burer_disarm_allowed` before a weapon drop and
  reports it with `bind_monster.burer_on_disarm` (`nlc_disarm_grace`), shared with the script stare.

### Required game data

- `gamedata/config/creatures/m_burer.ltx` (new keys, head 1.5 / neck 1.2),
  `gamedata/scripts/binders/bind_monster.script` (stare keys, shared disarm grace, repulse camera hook, unload roll
  removed). The script functions are optional for the exe (missing functors are skipped).

## nlc-3.589.46 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D17BB1BDB6E088D51144D8465CE8B2F0E6E90533A190646297A3D306850B2639` (archived with its PDB in `nlc_tools/builds/20261008_184236_d17bb1bd`).
- **Status:** runtime pending. Design after the `.45` test (`xray_hawkl_08-10-26_18-25-29.log`, open ground).

### NLC

- **Perch:** a perch counts after the enemy stood 0.4 m or more above its ground point for `pounce_perch_hold_ms`, not
  jumping, and off the ai-map as the siege sees it (weak ones "perch" pounced on flat ground at a jumping player).
- **Pair:** `tac_pair_rank` (strong 2, normal 1): the highest rank baits, so a normal + weak pair forms too. The
  flanker moves cloaked on an arc (`tac_flank_arc_dist`) until `tac_flank_arc_angle` off the player's view, then
  strikes when the player watches the bait or after `tac_flank_wait_ms` (`tac_flank_arc_max_ms` cap). The bait makes
  mock charges (`tac_bait_mock`), roars when the flanker strikes and charges for real once the flanker is within
  `tac_bait_sync_dist` (3 s cap). Both charged at once from the front before (the flanker struck 0.2 s into its stalk).
- **Open ground:** `cloak_counts_hidden`: cloaked beyond the x-ray radius and outside the view cone counts as hidden
  (siege, stalk, feint, hit and run found no cover in a field); hit and run runs zigzag legs while watched
  (`tac_recover_zz`); a feint aimed at vanishes sideways (`tac_feint_lateral`).

## nlc-3.589.45 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `879DD54560C94C78B5BFBF0E7EDC6A0A20E6B93FA9B0B1B482575BB69745EAE8` (archived with its PDB in `nlc_tools/builds/20261008_182500_879dd545`).
- **Status:** runtime pending. After the `.44` test (`xray_hawkl_08-10-26_18-12-38.log`); the relaunch fixed the
  standing pounces (Runtime accepted).

### NLC

- **Bait:** starts only beyond `tac_bait_dist` (it entered at 2-3 m in melee and stopped for 0.2 s, again and again);
  in place it walks slowly at the player (`tac_bait_advance`) instead of standing until its time ran out.

## nlc-3.589.44 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `2B56763F9A336785A4990E8696766B02C2129BEC51E0B7E026FFB2D11A29DBF8` (archived with its PDB in `nlc_tools/builds/20261008_165525_2b56763f`).
- **Status:** runtime pending. After the `.43` test (`xray_hawkl_08-10-26_16-44-30.log`).

### NLC

- **Pounce relaunch:** "on the ground" is less than half the expected flight progress, checked from 120 ms through the
  first half of the jump; the jump measures from the relaunch point (a jump started at a run kept the run's momentum,
  passed the fixed 0.5 m test and still died, moved 0.5 m).
- **Config:** `jump_max_height` 4.0 (a perch seen from a dip is 3.2-3.4 m to the target centre: "jump not possible"),
  `pounce_perch_block_ms` 8 s (a blocked perch strike left it holding a visible fallback beside the rock).

## nlc-3.589.43 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `49741AC1102173306FC4E7CA3493EA3769F2245FC212144D3DD4D3ECCF9F2679` (archived with its PDB in `nlc_tools/builds/20261008_164043_49741ac1`).
- **Status:** runtime pending. After the `.42` test (`xray_hawkl_08-10-26_16-28-12.log`).

### NLC

- **Pounce:** "baby jumps" also at the stock factor (moved 0.2-0.6 m, flat and perch, at random): the character kept its
  ground contact over the launch step, took ground control back and the speed limit ate the jump velocity.
  `CControlJump::update_frame` launches a jump that is still on the ground 120 ms after its start again, at most twice
  (species with an NLC jump only).
- **Perch strike:** the spot is chosen out of 12 directions around the enemy, preferring its back and sides (away
  from its view) over a long way around.

## nlc-3.589.42 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `D7E1D3CA141E85A1D18E6404D889F41EBF2158C27B4C05E7D6205D40C4FA3531` (archived with its PDB in `nlc_tools/builds/20261008_162742_d7e1d3ca`).
- **Status:** runtime pending. After the `.41` test (`xray_hawkl_08-10-26_16-17-02.log`).

### NLC

- **Flat pounce:** still died in place at `pounce_flat_factor` 1.9 although the takeoff passed 6 m/s upward (the target
  is the enemy's centre, ~1 m up); 3 of 4 failed, every flat pounce at the stock factor flew. Config: factor 0
  (= `jump_factor`); the engine jump cannot launch that flat and fast.
- **Pounce:** the sight gate also accepts the visual memory's "visible now" (it waited 2 m from a visible player:
  "visible right now" flickers between vision updates).

## nlc-3.589.41 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `890DD254CA9B59C6E1BEB65F695AA55A28D8F5BA96F3F6915646FB0554400DC3` (archived with its PDB in `nlc_tools/builds/20261008_161632_890dd254`).
- **Status:** runtime pending. After the `.40` test.

### NLC

- **Flat pounce:** with `pounce_flat_factor` 1.9 the takeoff was too flat: the character stayed in ground contact,
  regained ground control and the pounce died in place (the running-attack clip played standing, "moved 0.3 m").
  `CControlJump::calculate_jump_time` lowers a forced factor until the takeoff is at least `nlc_jump_min_vy()` upward
  (bloodsucker `pounce_min_vy` 6 m/s; world gravity 19.62).

## nlc-3.589.40 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `093B74820D81AB2C18FF77659C72220B0143533CD6E23E43ED2E6EF166F61743` (archived with its PDB in `nlc_tools/builds/20261008_160140_093b7482`).
- **Status:** runtime pending. After the `.39` test (`xray_hawkl_08-10-26_15-46-50.log`).

### NLC

- **Flat pounce:** `pounce_flat_factor` never applied (`CControlManagerCustom::jump` resets `force_factor`); it is set
  after the start and reset when the pounce ends.
- **Perch pounce:** the facing tolerance follows `jump_max_angle` (0.6 rad; it waited 24-28 deg off, the siege turns
  only beyond 30); the perch spot tries other sides around the enemy when the straight one is off the ai-map.
- **Tactics:** a re-entry of the tactic state no longer drops the running tactic (a recover ended after 1 s, idle).
- **Logs:** every bloodsucker pounce logs how it ended, after how long and how far it moved.

## nlc-3.589.39 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CFF14B93350028F169121244F092CC8D65C2C6F55F6FC790340847C534A698EA` (archived with its PDB in `nlc_tools/builds/20261008_154404_cff14b93`).
- **Status:** runtime pending. After the `.38` test (`xray_hawkl_08-10-26_15-11-26.log`).

### NLC

- **Perch strike:** the spot is placed around the enemy itself, at the xz distance that puts the 3D jump range mid band
  (it stood 7-9 m away, out of range, when the enemy's ai vertex lay off on the rock); out of range at the spot, it
  picks a new one.
- **Pounce:** separate ranges for a perch (`pounce_perch_dist`) and flat ground (`pounce_flat_dist`); flat ground is
  lower, faster and longer (`pounce_flat_factor`); the glide clip is configurable (`pounce_anim`, now the running
  attack); an own hit test, centre to centre (`pounce_hit_dist`), as the stock one missed perched players; one hit per pounce.
- **Cloak:** never fully invisible within `cloak_xray_radius` of the enemy (weak 3, normal 2.5, strong 2 m).
- **Tactics:** hit and run for all variants (35/45/55% health, `tac_recover_cooldown_rand`); weak bloodsuckers stalk
  (closer, shorter); normal ones feint (rarer); a pair forms when one member has `tac_pair_lead` (strong), who baits.

## nlc-3.589.38 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `69190B60364EC350DA37B4FEA85A8FC8E59D8C1B74F7CC9CCAA3B24896C37F93` (archived with its PDB in `nlc_tools/builds/20261008_133415_69190b60`).
- **Status:** runtime pending. Fixes after the `.37` tests (`xray_hawkl_08-10-26_13-10-00.log`, `13-20-54`).

### NLC

- **Pounce:** 3 of 5 pounces ended one frame after takeoff (a velocity bounce at the start); `CControlJump` ignores
  bounces for `nlc_jump_bounce_grace()` ms (bloodsucker `pounce_bounce_grace_ms` 150, other species 0 = stock). The
  upward-jump log names why and after how long a jump ended; a ready perch pounce that does not start logs why.
- **Perch strike:** shot at the perch-strike spot, the bloodsucker leaves it for `pounce_perch_block_ms` (it re-picked the
  same spot after every shot when no retreat point existed).
- **Bait and flank:** a bait no longer restarts while a flanker's strike is still fresh (every later bait ended after
  0.2 s).

## nlc-3.589.37 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `34CB9B7F2F65E5C42B9D2133BA458CE8ED05AF261854BD88C917BE2F6A1B3766` (archived with its PDB in `nlc_tools/builds/20261008_130858_34cb9b7f`).
- **Status:** runtime pending. Fixes after the `.36` test (`xray_hawkl_08-10-26_12-47-40.log`) and a code review.

### NLC

- **Back-hit grab:** it broke 220 ms after its start (the stock start distance, `MinAttackDist`, was also the break
  distance); the ambush grab now breaks only beyond `vampire_break_dist` (3.5 m) and logs why and when it ends. A grab
  that breaks early lets the held-back rest of the hit land; the rest keeps its world direction. The back hit is rolled
  only when a grab can start from there, and the actor is checked again (jumping, climbing, facing it) before the grab.
- **Cloak:** fully visible while its grab holds the actor (a strike's x-ray window could swap the model mid-grab).
- **Pounce:** range measured as `CControlJump::can_jump` does (3D, feet to the enemy's centre).
- **Zigzag:** the `zz_react_ms` reaction time restarts after the enemy left sight or range.
- **Stalk:** the flank preference restarts with each stalk and each new enemy; `pounce_perch_block_ms` at least 2 s.

## nlc-3.589.36 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `86154D49B50015A6CC36D172A65C60CECCDFFDE846F6D69E00933A75CF9D07CB` (archived with its PDB in `nlc_tools/builds/20261008_124644_86154d49`).
- **Status:** runtime pending. Round 6 after the `.35` test (`xray_hawkl_08-10-26_12-09-38.log`), design doc 21.

### NLC

- **Perch strike:** no lunge during a siege or at a perched enemy; the perch-strike spot stays cloaked until the pounce
  is ready; a refused perch roll or no pounce within `pounce_perch_wait_ms` gives the spot up for
  `pounce_perch_block_ms` (it hides instead of standing there).
- **Pounce:** all variants at a perched enemy (`pounce_perch_chance`, `pounce_perch_max_h`); flat ground only with
  `pounce_flat` (strong). The rolls happen per cycle before the reveal; a failed jump is logged.
- **Cloak:** a strike reveal without a strike ends after `strike_reveal_max_ms`, then no strike reveal for
  `strike_reveal_block_ms`.
- **Back-hit grab:** a hit from behind may turn into the grab (`vampire_backhit_chance`); `vampire_backhit_damage_k` of
  it lands at once, the rest only when no grab starts within `vampire_backhit_window_ms`.
- **Flanks:** stalk points on the player's flank first (`tac_stalk_flank`); an unwatched charge curves toward the flank
  or back (`charge_flank`, bloodsucker `nlc_run_target_override`).
- **Zigzag trigger:** `zz_need_gun 3` (aiming down sights at it or the crosshair on it) and `zz_react_ms`.

## nlc-3.589.35 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `56DA4D67D582E71170C8A2D7E11B1C24FB4B4D197AFCA0AA8BFF38F83F1EE649` (archived with its PDB in `nlc_tools/builds/20261008_014521_56da4d67`).
- **Status:** runtime pending. After the `.34` test (`xray_hawkl_08-10-26_01-17-40.log`).

### NLC

- **Bloodsucker cloak:** the strike reveal applies only while charging (attack run, facing within 45 degrees, enemy
  seen) or at a siege perch-strike spot (margin now 2 m); every other siege hold or move stays cloaked; a feint ends with
  a cloak window (`tac_feint_cloak_ms`).
- **Perch strike:** a strong bloodsucker besieging a player on a perch it can jump to goes to pounce distance and
  pounces (`nlc_siege_perch_strike`, `CBaseMonster` hook).
- **Feint:** shows itself while charging (no stop); with no hidden point it charges on cloaked.
- **Stalk:** points on its own side of the player (the route across ended the stalk at once).
- **Hit and run:** `tac_recover_break_health` breaks away even at point blank; with no hidden point it takes a point
  out of the enemy's view, else runs straight away.
- **Turning back after a pass:** the rotation jump also starts above the normal run speed (a cloaked bloodsucker
  never turned quickly); `Run_Attack_Overshoot` ends the lunge line shortly past the enemy. The rotation jump
  releases only the controls it owns.
- **Cats:** the stock attack jump was never wired (`nlc_jump_attack`), and the `run_turn_180_r` running turnaround
  (`nlc_turn180`); both on in `m_cat.ltx`.
- **Logs:** every lunge (`lunge at`).
- **Config:** bloodsucker `jump_factor 0.9` (strong), `tac_feint_show_ms 700`, break health 0.2 / 0.3; cats
  `zz_max_angle 14`; dogs `zz_min_leg_k 2`.

## nlc-3.589.34 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `3B287996E5A2FF6C811E2BFD6D91EC51EF270A7B9A568C5A9D530FCEBF4DBAB1` (archived with its PDB in `nlc_tools/builds/20261008_011344_3b287996`).
- **Status:** runtime pending. After the `.33` test (`xray_hawkl_08-10-26_00-49-32.log`).

### NLC

- **Bloodsucker strike tell:** `strike_reveal_margin` (3 m): fully visible within the start distance of a ready lunge or
  pounce plus the margin, else within `cloak_cooldown_radius` (5 m); a strike needs `strike_reveal_ms` (300) fully
  visible first. The pounce only opens the after-strike window (no model swap under a running jump).
  `Run_Attack_Cooldown` (stock 2200 ms) and `pounce_chance` (strong 0.5: else the lunge) are config keys.
- **Perch:** a besieging bloodsucker may pounce at a perched enemy it can reach (`jump_max_height`); bloodsuckers hold
  the low-perch bite window 0.8 s (`elev_reach_timeout`); a side-guard species tries the retreat ring before holding a
  visible spot, and holds one standing and facing the enemy. The attack-run state faces the enemy when it stands in
  place (all species; it kept its arrival heading, side-on).
- **Zigzag turning:** the heading speed follows the dodge factor (the turn radius the path was planned with holds);
  leg angle capped by the run turn rate (`zz_turn_share`), legs shorter than `zz_min_leg_k` x the turn radius run
  straight (no walk-speed curves), no new leg while heading more than `zz_heading_gate` off the enemy (cats circled).
- **Config:** cats `zz_max_angle 20`, legs 700-1000 ms, `zz_seen_grace 1500`; bloodsucker `lunge_reveal 2`,
  `vampire_struggle_need 2500`, `Melee_Rotation_Factor 3.0`.

## nlc-3.589.33 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8D411AAFC272EBF79D586492A2FFE77DA7FFAE00118CDECCF0088D21D218BB29` (archived with its PDB in `nlc_tools/builds/20261008_004826_8d411aaf`).
- **Status:** runtime pending. Fixes after the `.32` test (`xray_hawkl_08-10-26_00-39-43.log`).

### NLC

- **Crash fix (control capture steal):** a lunge started during a pounce (NLC had dropped the run attack's stock
  "nothing else captured" check for the boar ram); `capture_pure` took the jump's controls, the jump ended and
  `CControlJump::on_release` read the direction data it no longer owned. The run attack now refuses while a jump runs or
  a non-base control holds the animation (jump, vampire grab, stagger); the jump and the run attack release and unlock
  only the elements they still own (`CControl_Manager::release_pure_owned`, `unlock_owned`).
- **Grab from behind:** never starts while a lunge or jump plays or in the x-ray window after a strike (a grab started
  0.35 s into a lunge was carried past the player and ended at once: a camera twitch, no hold).

## nlc-3.589.32 (2026-10-08)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `782D95F57F65A2A7C7ECE0E21DE4A0BC598F097C38DEABEA11B28D33AB7C705B` (archived with its PDB in `nlc_tools/builds/20261008_003651_782d95f5`).
- **Status:** runtime pending. Fixes after the first `.31` test (`xray_hawkl_08-10-26_00-13-52.log`).

### NLC

- **Crash fix (bloodsucker pounce):** `CControlJump::on_event` ended in a null dereference on the first glide frame;
  the pounce decloaked to full visibility in the same tick, which swaps the model and restarts its animations. The
  jump now ends (logged) instead of crashing when its animation capture, direction capture or blend is missing; the
  pounce reveals only to x-ray (same predator visual); the bloodsucker's visibility is frozen while a jump or lunge
  plays; the lunge reveals right before the control activates, not during it.
- **Strike reveal:** `lunge_reveal` (1 x-ray, 2 full) and `lunge_reveal_ms`: after a lunge or pounce the bloodsucker
  stays in that state until the first melee swing (which decloaks at once).
- **Grab from behind reachable:** `vampire_intent_dist`: when a grab from behind is planned (cloaked, unseen, behind
  the player, chance rolled), the lunge and pounce stand down and the cloak holds until grab range. The grab start
  decloaks at once. The pounce also never fires in a siege.
- **Stalking:** the enemy is pinned in memory during stalk, hit-and-run and feint (memory 40 s vs. stalks up to 60 s);
  hiding and stalk points are picked around the last known position, not the true one.
- **Tuning keys:** `tac_recover_min_time`, `tac_feint_vis`, `tac_bait_vis` (1 x-ray, 2 full).
- **Zigzag:** `zz_seen_grace` (the monster's own sight flickers between vision updates and stopped legs), per-variant
  frequency `zz_chance`, `zz_burst`, `zz_rest`; dog, pseudodog and cat variants tuned (weak dogs least, pseudodogs
  and cats most).
- **Game data:** `game_relations.ltx` (new zigzag keys; `evade_debug_log` / `elev_debug_log` true for the test
  round), `m_bloodsucker.ltx`, `m_dog.ltx`, `m_pseudodog.ltx`, `m_cat.ltx`.

## nlc-3.589.31 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `764E22AE64F1EA523025F0D1F277D4FC1B19DEE90A0B7A557428FD55B9C56E3E` (engine/bin_x64; archived with its PDB in `nlc_tools/builds/20261007_235838_764e22ae`).
- **Status:** runtime pending. Design: `docs/DESIGN_monster_movement_under_fire.md`; handoff:
  `docs/HANDOFF_monster_movement_under_fire.md`. Rides on top of the uncommitted siege pass (`nlc-3.589.30`).

### NLC

- **Evasion pass (`base_monster_evade.cpp`, `[monster_evasion]` in `game_relations.ltx`):** every feature has its own
  key and defaults to off; `evade_enabled = false` (shared or per section) turns all of it off for a species;
  `evade_debug_log` writes `~ [evade]` lines.
  - **Threat test:** `nlc_threat()` (cached 200 ms): watched (view cone `evade_watch_cone` + one static ray), armed
    (a gun, no knife or binoculars), aimed (`evade_aim_angle`), locked (crosshair ray, actor only).
  - **Zigzag approach:** `zz_*` keys. Charging species leave the straight line in legs of `acos(1/zz_speed_k)` degrees
    (capped by `zz_max_angle`) with a dodge speed factor (`nlc_set_dodge`, separate from the giant's haste), from
    `zz_dist.y` to `zz_dist.x`, when watched (`zz_need_gun`: 0 watched, 1 armed, 2 aimed); shot sounds, near misses
    and hits switch the side. Hooks in `CStateMonsterAttackRun::execute` and the far branch of the attack-on-move state.
    On in: dog, pseudodog, cat, bloodsucker.
  - **Side guard (`side_guard`, bloodsucker):** a watched guarded monster ignores the squad encircle orientation; siege
    goto / retreat / flee picks skip points that would cross the line of sight (`side_guard_angle`,
    `side_guard_short`); with none left, a radial escape straight away from the enemy; lateral-run diagnostics.
  - **Run attack:** the 2.2 s cooldown is armed when the lunge really starts (it was armed before the species gate could
    refuse it); `run_attack_haste_k` speeds up the lunge. **Found:** `CAI_Bloodsucker` and `CAI_Boar` add the run attack
    after `control().load()`, so `Run_Attack_Dist` / `Run_Attack_Delay` were never read (code floors: 2.5-5.5 m, no
    delay). New bloodsucker key `run_attack_cfg` loads them (off by default; the boar is unchanged).
  - **Siege placement:** `elev_pick_near_anchor` ranks ambush points by their distance to the enemy's ground point.
  - **Dog sneak:** `anim_steal` (dog, `stand_steal_`); upward-jump diagnostics in `CControlJump`.
- **Bloodsucker behaviour set (`bloodsucker_tactics.cpp`, `eStateAttack_NlcTactic`):** cloak hold at the siege ambush
  (`amb_cloak_hold`), full visibility only inside `cloak_lunge_radius` while the lunge is ready, lunge from cloak with
  decloak (`run_attack_decloak`), melee reveal, faint steps while cloaked (`CStepManager::step_volume_k`), edge sneak
  (`sneak_near_dist`), hit and run (`tac_recover_*`), stalk for openings (`tac_stalk_*`, `tac_open_*`), feint
  (`tac_feint_*`), bait and flank pair (`tac_pair_enabled`), night boldness (`tac_night_*`), vampire grab as a true
  ambush from behind with a chance per variant and a mouse-shake struggle (`vampire_*`, `CActorInputHandler::on_mouse_move`),
  experimental pounce (`amb_pounce`, strong). Variants in `m_bloodsucker.ltx`; the phantom is excluded.
- **Game data:** `game_relations.ltx` (`[monster_evasion]`), `m_bloodsucker.ltx`, `m_dog.ltx`, `m_pseudodog.ltx`,
  `m_cat.ltx`. Vampire numbers and the variant table are untuned starting values.

## nlc-3.589.30 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `9FCCF18989CAC0DD9038095D4E1FE06D8A5487CA330E74FB638AD1A4EC4DCFE4`
- **Status:** runtime pending. Design: `docs/DESIGN_monster_elevation_siege.md`.

### NLC

- **Siege of an elevated enemy:** an enemy out of reach because of height (roof, ledge, vehicle) is besieged
  instead of the stock home-point shuffle (`eStateAttack_Siege`, `base_monster_siege.cpp`): monsters wait at points
  the enemy cannot see near its ground point (ambush ring, then a looser watch ring with one peeking watcher per
  squad), move when exposed or near-missed, retreat out of sight when shot, rest per species. Config
  `[monster_elevation]` in `game_relations.ltx` (`elev_enabled` master switch, `elev_vs_npc`), per-section overrides.
- **Anti-bait:** an enemy coming down within `elev_reengage_close` is attacked at once; farther away the re-engage
  delay grows with each bait (3 / 6 / 10 s), and the break-off grace drops to 1 s after the first.
- **Hostility kept (lock):** noise or sight pins the enemy in monster memory (`CMonsterEnemyMemory::pin`) for the
  species patience, then the stock 20 s memory; faint (suppressed outer-tier) shots renew up to
  `elev_faint_patience`; losing the enemy this way starts the stealth "lost the trail" alert.
- **Exits:** a squad that loses `elev_flee_losses` (default `hunt_flee_losses`) members during a siege abandons it;
  a bolt far from the enemy pulls one distractible besieger (boar, flesh) away; a low perch (up to
  `elev_reach_height`) is attacked until no bite was tried for `elev_reach_timeout`.
- **Moved helpers:** `nlc_point_on_map` / `nlc_los_to` from `CController` to `CBaseMonster` (controller unchanged).
- **After the first test rounds:** hidden = three static rays from the enemy's eyes (head, both body ends) all
  blocked; an outer ring before a visible fallback (lies down there); a still-hidden spot is kept; the sneak gait
  only from out of sight; the low-perch attack never inside a siege, dropped when shot without biting, 15 s block;
  dogs stay in the attack state during a siege (`dog_state_manager`: the 6 m / mid-home / 8 s danger gate let them
  idle); at most 4 point searches per frame over all monsters.
  The lock survives an interrupted siege (hit reaction, sound states) and a restart keeps the patience running;
  a peeking watcher gets the vision factor `elev_peek_detect_k` (dogs 2, peek 4 s).
- **Opt-outs:** controller, poltergeist, snork, burer, chimera.
- **Game data:** `game_relations.ltx` (`[monster_elevation]`), species keys in `m_dog`, `m_pseudodog`,
  `m_bloodsucker`, `m_boar`, `m_flesh`, `m_giant`, `m_zombie`; `dsh_battle_radius.script` releases the gunfire
  pursuit 25 m out when the actor is elevated.

## nlc-3.589.29 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `9D6F0A194F42DC54A3D09A19AD43360BA7616F1D82B0423BCDA8244274E6A948`
- **Status:** runtime pending. Fixes from the multi-agent review of the controller work.

### NLC

- **Crash fix:** `CController::net_Relcase` clears a destroyed enemy from its thralls' task object (thralls hold
  the controller's enemy; an offline switch or release of that enemy left a dangling pointer).
- **Controller vs non-actor enemies:** the cover / peek loop only runs against the actor (the tube targets only
  the actor); stalkers and monsters get the stock fight instead of an endless peek without attacking.
- **Thrall eyes:** the strike no longer silences the hum of a real tube that started during the wind-up; a
  wind-up dies with the controller.
- **Staggers:** a pending thrall stagger that cannot start within 1.5 s is dropped (no late, random staggers);
  thralls restored after a load do not stagger.
- **Recruit channel:** stops only the sequencer it started. **Herd call:** reports 0 when thralls do not attack.
- **Stalker aim:** psy health 0 (never regenerates for NPCs) no longer means permanent x4 spread.
- **Input scramble:** a key scrambled onto a blocked action is blocked too, and a recorded fire / zoom release
  is never blocked; a consumed fire press never swallows a click meant for an open UI window.

## nlc-3.589.28 (2026-10-07)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8D8B793A437871A0DC1ACD8CAFDDD4D94D22D6B46828DF288516DB8F0E7A6229`
- **Status:** runtime pending.

### NLC

- **Controller input scramble without binds:** `level.nlc_scramble_input(group, ms)` (0 movement, 1 fire/zoom)
  remaps the game actions in `CLevel` input dispatch for a while (random derangement); a key keeps the action it
  was pressed as until release, so fire/zoom cannot stick. The binds in `user.ltx` are never touched, so a load,
  quit or crash cannot leave them scrambled. `level.nlc_clear_input_scramble()` ends it.
- **Thrall eyes as a quick tube:** the thrall's head flares with the tube particles and the tube hums for
  `nlc_ctrl_thrall_eyes_windup` ms, then the tube hit sounds and the psy jolt land (cancelled if the thrall loses
  sight of the actor).
- **Script access:** `nlc_controller_phase()` (ranged phase name) and `nlc_controller_herd_call(ms)` (every thrall
  attacks the controller's enemy, posts and flanks dropped) on `CCustomMonster`.

## nlc-3.589.27 (2026-10-06)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `43EB91F4A9394AC1510A5F7957977A9A12865381A8F1EA8DF14BFC2201074C61`
- **Status:** runtime pending.

### NLC

- **Controller thralls attack:** monsters taken under control now hunt the controller's enemy
  (`nlc_ctrl_thrall_attack`, `nlc_ctrl_thrall_memory`) instead of only following it and sitting down
  (stock code never called `set_controlled_task`).
- **Controller reposition:** when its enemy is inaccessible (roof, crate, vehicle, outside home, restrictor)
  the controller walks to cover (tube recharging or recently hit) or to a tube-range point
  (`nlc_ctrl_reposition`, `nlc_ctrl_cover_dist`, `nlc_ctrl_range_dist`, `nlc_ctrl_hold_time`,
  `nlc_ctrl_move_timeout`) instead of standing in the open.
- **Controller ignores its thralls:** `game_relations.ltx` keeps the species hostile, so the controller
  forgets a thrall as enemy and attacker on capture and `is_relation_enemy` is false for its thralls.
- **Controller cover loop:** hides in cover while the tube recharges, peeks to a point with line of sight when
  it is ready, stock melee when close, stock hunt when the enemy is lost (`nlc_ctrl_cover_loop`, `nlc_ctrl_close_dist`,
  `nlc_ctrl_lost_time`, `nlc_ctrl_peek_radius`, `nlc_ctrl_aim_time`); real run to cover with `nlc_ctrl_run_anim`
  when the visual has the clip.
- **Controller thralls:** species roles and weights (`nlc_ctrl_thrall_table`): hunt, flank, guard (new
  `eTaskMove` / `CStateMonsterControlledMove` for every controllable species); weight budget, leash, release
  stagger and slow; active recruitment channel (`nlc_ctrl_recruit_*`) that any hit breaks.
- **Thrall release:** repeated, slowed stagger (`nlc_ctrl_release_stagger = count, speed`; new
  `CBaseMonster::nlc_stagger_repeat`, critical-wound playback speed); thralls ignore their home while controlled
  (`CMonsterHome::nlc_suspend/nlc_resume`).
- **Controller tube:** while aiming from a peek point with a clear ray to the actor seen within 3 s, the tube
  does not wait for the vision build-up (`CController::nlc_psy_sense`).
- **Controller Mirage decoy:** on reaching cover a controller may spawn a decoy controller (`nlc_ctrl_decoy_section`,
  `_chance`, `_cooldown`) and stays hidden while it lives; a decoy (`nlc_ctrl_decoy`) has no tube, thralls or
  recruits and is destroyed on any hit or after `nlc_ctrl_decoy_life` (no corpse, loot or kill).
- **Controller guards:** posts resolved without misusing `accessible_nearest` (it requires an inaccessible
  point), closer fallbacks, one guard plus flank/hunt overflow, guard promotion, 3 s thrall status log.
- **Stalker psy aim:** `nlc_psy_dispersion_k` scales weapon dispersion with lost psy health (`CAI_Stalker`).
- **Controller flushed reaction** (`nlc_ctrl_flush_*`): an enemy close to its cover gets a psy jolt, then the
  controller repositions nearby. **Thrall eyes** (`nlc_ctrl_thrall_eyes*`): a weak psy jab through a thrall that
  sees the actor while the controller does not.
- **Script hooks on `CCustomMonster`:** `nlc_set_phantom` (invisible to AI, no grenades), `nlc_add_enemy`,
  `nlc_controller_thralls`, `nlc_controller_take` (thrall persistence across save/load).
- **Stalker smart-terrain evaluator:** no crash when a script released the server object this frame.
- **Script:** `level.consume_key_press()` skips native handling of the current key press (controller fire reaction).
- **Diagnostics:** `CBaseMonster::nlc_inaccessible_reason()`; `nlc_ctrl_debug` logs `~ [controller]` lines.

### Required game data

- `m_controller.ltx` `nlc_ctrl_*` keys, `[nlc_controller_thralls]`, `[m_controller_shepherd]` (all keys optional;
  missing keys keep stock behavior); `sivol_weapon.script` (consume the fire press), `rx_gl.script` (no launcher for phantoms), `sivol_controller_psy.script` (stalker domination and guards, Mirage swap and tells, thrall persistence),
  `m_stalker.ltx` `nlc_psy_dispersion_k`.

## nlc-3.589.26 (2026-10-06)

- Co-dev build: body-part health / skeleton system from A.R.E.A (`nlc_body_health`, `UIBodyHealthWnd`),
  console command history; see commit `7090f09b` and `19db1c3c`. Changelog entry not written by the co-dev.

## nlc-3.589.25 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `6DA80D5FFF05401EC30D2A30BAF4EB01B9C4E9DD61549C148C667D004C0B9DDC`
- **Status:** runtime pending.

### NLC

- **Pseudogiant charge rework:** the rush paths to a point locked `Charge_Overshoot` m past the target's spot
  (`CBaseMonster::nlc_run_target_override`, used by the attack run substate), so sidestepping escapes; reaching
  that point without contact counts as dodged. Wind-up plays `Charge_Windup_Anim` in place at
  `Charge_Windup_Anim_Speed` through the animation sequencer; the charge requires facing the target
  (`Charge_Face_Yaw`). The stumble is a forced damaged walk (limp clip and `Velocity_WalkFwdDamaged`) for
  `Charge_Stumble_Time` instead of a speed multiplier. `Charge_Max_Turn` is no longer used.
- **Animation sequencer:** optional playback speed (`seq_run(motion, speed_k)`) and early stop (`seq_stop`).
- **Forced damaged walk:** `CBaseMonster::nlc_force_damaged` on top of the `DamagedThreshold` health rule.

### Required game data

- `m_giant.ltx` `Charge_Windup_Anim*`, `Charge_Face_Yaw`, `Charge_Overshoot`, `Charge_Stumble_Time 2500`.

## nlc-3.589.24 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `B2DD779B6A2A2B8D843993A808D3EDEFC1B3D38DAD395B4E6DB23FDEF288A7F9`
- **Status:** runtime pending.

### NLC

- **Species speed:** `move_speed_k` in a monster section scales movement and walk clip speed together
  (pseudogiant 0.83); rage and charge stack on it.
- **Pseudogiant charge:** wind-up telegraph (`Charge_Windup`: attack sound, near stop while aiming), locked rush
  line (`Charge_Max_Turn`: a target that sidesteps out of it makes the rush abort with a stumble), ram hit on touch
  (`Charge_Contact_*`) instead of waiting for an attack animation.

### Required game data

- `m_giant.ltx` `move_speed_k`, `Charge_Contact_*`, `Charge_Windup`, `Charge_Max_Turn`.

## nlc-3.589.23 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A5B2E5121060BDBEA0E7D446920B7A7CCB9E53E86A84E7D346BB29A062D45378`
- **Status:** runtime pending.

### NLC

- **Melee phase 2:** `[monster_melee] trace_ignore_objects` (the actor line-of-hit trace is blocked only by
  level geometry, not pack mates or loose objects); `close_yaw_dist` / `close_yaw_half` (wider bite yaw window
  at body contact).
- **Monster speed multipliers:** persistent bonus and temporary haste next to the stomp slow; movement clips
  without an accel chain now scale their speed with the multiplier (no foot sliding).
- **Pseudogiant moves** (each with its own switch in `m_giant.ltx`): rage below a health fraction (faster,
  wider, more frequent stomps, entry stomp), recovery stomp on burst damage, charge (rush on a clear line,
  empowered first strike, stumble on a miss).

### Required game data

- `game_relations.ltx` `[monster_melee]` keys; `m_giant.ltx` `HugeKick_Rage_*`, `HugeKick_Recover_*`,
  `Charge_*`, `stand_attack_1` impulse 400.

## nlc-3.589.22 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `813BE01F917A2F0B2F6507A8E18071D98A5FFFA43064C8E086DB8F086407E056`
- **Status:** runtime pending.

### NLC

- **Surface-aware melee distance (opt-in):** `melee_surface_distance` / `melee_surface_offset` in a monster
  section make `CMeleeChecker::distance_to_enemy` and the bite range check in `check_hit` measure to the
  target's collision surface (+ offset) instead of centre to centre (the original ray test queried the
  attacker's own collision form). Large targets such as the pseudogiant now count as close.
- **Melee diagnostics:** `[monster_melee] debug_log` logs group-attack decisions (enemy within 4 m) and every
  bite attempt (distance, yaw/pitch window, hit/miss).
- **Rotation jump guard:** animation sets whose names are missing in the model are dropped with a log line
  (pseudogiant placeholders `"1".."4"`); no rotation jump starts without a valid set.

### Required game data

- `game_relations.ltx` `[monster_melee]`; `m_dog.ltx`, `m_pseudodog.ltx` (`melee_surface_*`).

## nlc-3.589.21 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `C371FCC9D14F42DC8BF5F4918AD696E368B74E749BE27450A02C05A52ECD3023`
- **Status:** runtime accepted (quick tests): target switching, damage threat, back-stab aggro, crowd stomp,
  splash damage, stagger, mutant slow, ragdoll kills, jump dodge, grenade-deflection stomp. Includes the
  unreleased `nlc-3.589.20`.

### NLC

- **Monster target selection:** `[monster_target_selection]` in `game_relations.ltx` (per-section overrides):
  player bias, current-target stickiness, recent-hit bonus, damage-weighted threat (per-attacker health lost,
  fading over `damage_memory_time`), back-stab provocation (`back_hit_threat`), `debug_log` for target switches.
  Missing keys keep the original danger formula.
- **Pseudogiant stomp:** crowd trigger, splash on enemies (optionally neutrals) with falloff, height gate,
  configurable hit type/bone per victim kind, knockback, stagger (stalker leg-hit / mutant critical-hit anims),
  mutant movement slow, actor hit inside the splash, precise jump dodge (feet clearance at impact),
  grenade-deflection stomp at a faster animation speed. All `HugeKick_*` keys optional.
- **Stomp kills** skip the CoP death animation, so the body is thrown as a ragdoll.
- **Run attack guard:** monsters whose model lacks `stand_attack_run_0` (pseudogiant) no longer start the run
  attack (null blend crash in `CControlRunAttack::on_event`).

### Required game data

- `game_relations.ltx` `[monster_target_selection]`, `m_giant.ltx` (`HugeKick_*`, damage table, `health_hit_part`),
  `m_flesh.ltx` (`target_stickiness`).

## nlc-3.589.19 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F3B6C843EA4ED42781E9614C71920A18526335D7CBFDE8992A9AB6D0837AB60D`
- **Status:** runtime pending.

### NLC

- **wpn_knife_m1 walking return:** optional `knife_return_moving_speed` in `wpn_knife_m1_hud` scales the
  `anm_hitN2idle` return clip when it replaces a moving idle (default 1.0; tuning experiment).

### Required game data

- `w_knife.ltx` key `knife_return_moving_speed` (optional).

## nlc-3.589.18 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1DB407E47AD7ED1343CE05F9DE60FE3EB5F3BAEFA363A41FDA76CF70A634D361`
- **Status:** runtime pending.

### NLC

- **wpn_knife_m1 walking return:** the swing-to-idle return clip (`anm_hitN2idle`) is now also used when the
  next idle request is a moving idle (`anm_idle_moving*`), so the return no longer cuts straight into the walk cycle.

### Required game data

- None (the torch rework and `liz_knife_flash` ship as script/config/asset changes).

## nlc-3.589.17 (2026-10-04)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `8B3254F5CDC441C9E70F29A527B2D41B5008103375EB52043C0A158E5991D823`
- **Status:** runtime pending. Foliage and muzzle flash (docs STEALTH_DESIGN 20.15); first test accepted.

### NLC

- **Close-range foliage:** for stalkers looking at the actor the ray cutoff fades from the vision
  section `transparency_threshold` to `foliage_near_threshold` inside `foliage_near_range` (0 = off).
  New `Feel::Vision` virtuals `feel_vision_threshold` and `feel_vision_mtl_transp_for` (vanilla defaults);
  NPC targets and mutants unchanged.
- **Foliage table:** optional `[nlc_vis_transparency]` (material name substring = see-through, first match
  wins) and `foliage_k` (vis^k) for stalker rays towards the actor; every partly see-through material is
  logged once ("vis mtl").
- **Muzzle-flash reveal:** an unsuppressed actor shot gives stalkers in range that already have the actor
  in view a one-time sum bump of `flash_reveal` x darkness x threshold (`flash_reveal_range`,
  `flash_reveal_gap_ms`; 0 = off).
- **Lua:** `nlc_stealth_ray(id)`: see-through of the stalker's ray to the actor, cutoff, distance, last
  see-through material (debug overlay).
- **Follow-ups after the first test:** `nolos` log lines show the effective (near-faded) cutoff;
  no flash reveal for a stalker already fighting the actor. Game data: `bush_sux = 0.5` in
  `[nlc_vis_transparency]` (one thin-bush face no longer blocks sight at any range).

## nlc-3.589.16 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `64321D0632C68F12C95F556FA554C7825E18F81E9698D77E45FA98016DFACB49`
- **Status:** runtime pending. Fixes from a code review of the stealth base (nlc-3.589.4 to .10).

### NLC

- **NPC-vs-NPC sight restored:** vision sections get `nlc_actor_rate_k` (actor-only rate factor);
  `stalker_vision_danger` is back to vanilla `time_quant` 0.001 with `nlc_actor_rate_k` 0.35 (same actor
  detection as tested). Before, the slower danger vision applied to every target (NPCs re-acquired
  NPC enemies about 2.9x slower).
- **Monster gates actor-only:** pack enemy sharing, the impact range gate and the near-hit gate apply
  only to the actor; NPC enemies and shooters as vanilla.
- **Near misses** use the segment the bullet really flew (clipped at its hit, capped at 60 m), not the
  whole step (NPCs behind a wall got near misses).
- **Pack far notices** are queued and handled on the main thread.
- **Squad share skip** (`squad_dying_share`) only for a dying member whose enemy is the actor.
- **Wall muffle cost:** listeners within 60 m only, 500 ms staggered cache, 3 m position tolerance.
- **Surface table** read in file order (`Ordered_Data`; first match wins as documented).
- **Psy/smell senses:** actor speed clamped (10 m/s); a jump over 5 m (teleport) is not movement.
- Outfit key caches keyed by id and section; species `pack_share_delay_min` and `nlc_rain_k` clamped.

## nlc-3.589.15 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `30B28B98AF2C159417A643BF11C34D06B6C7AA222F5EAC64D7BA52E2BCE81D98`
- **Status:** runtime pending. Fixes from a code review of the stealth pass 5 engine changes.

### NLC

- **Wall muffle:** passable hits (bushes) are stepped over instead of ending the check, and the ray runs
  both ways (the static pick culls back faces, so single-sided walls blocked in one direction only).
- **Psy auras:** `actor_psy_k` defaults to -1 = the actor's telepathic immunity (vanilla) until the
  script pushes the gear factor; before, psy gear stopped helping when the script had not run.
- **Queues:** an atomic flag replaces the unlocked `empty()` checks; queued bolt contacts and near-miss
  segments carry their level and are dropped after a level change (ids are reused).
- **Bolt contact** position from the contact geometry (the hook runs on the physics thread).
- **Monster hunts:** search and flank points are never swallowed by the "no retarget within 8 m" rule;
  hunt, alert, concern and wall-cache state reset on reinit; species hunt keys clamped.
- **Planner dump** relies on `CPlanner::update`'s own `__except` (a C++ try is not allowed there).

## nlc-3.589.14 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `F6AAE6CE16302CC05DFDEA8B7A915E4F4602DB11DCB6AC2D14D0D2F1B60B2154`
- **Status:** runtime pending.

### NLC

- **Squad enemy share** (`agent_enemy_manager.cpp`, `squad_dying_share`): the squad agent shares the
  enemies of its combat members with the squad (`make_object_visible_somewhen`, memory masks). A
  member killed by a hit stays alive for a frame or two with the attacker as his enemy, so a silent
  one-shot kill told the whole squad (all Agroprom military) where the actor was. With
  `squad_dying_share` 0 a member at 0 health contributes no enemies; living members share as before.

## nlc-3.589.13 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `CBBCEF9E8FB60240EB8503CF250537DB3EE4501CF2024FD265BEEC48E2AD499D`
- **Status:** runtime pending. Diagnostics only on top of `nlc-3.589.12`.

### NLC

- **Planner freeze diagnostics** (`action_planner_inline.h`): when an action planner finds no
  solution (the NPC freezes, `[CPlanner::update]: <name> has solution().empty()`), the engine also
  logs, once per freeze, every evaluator's value (`world:`), the goal (`goal:`) and each action's
  unmet preconditions (`unmet:`). Freezes of Agroprom soldiers and others predate the stealth work
  (logs of 30.09 and 02.10 before the suspicion scheme); this finds the blocking property.

## nlc-3.589.12 (2026-10-03)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7131F07152D3DE0DB4290B47F6D6956660D0055F10ADB446D91417E055135FB9`
- **Status:** runtime pending (stealth pass 5, `docs/STEALTH_DESIGN.md` 20). Replaces the
  unshipped test build `nlc-3.589.11` (`C30A6943`), which crashed on the second bolt hit: the
  queued bolt contacts were handled in `CPHWorld::OnFrame`, which runs on the second thread
  (`seqFrameMT`), and called Lua from there. Now handled in `CLevel::OnFrame` (main thread).
  Actor bullet near misses (M3, from the bullet manager's parallel update) are queued and handled
  there too, so AI memory is no longer written from the parallel thread.

### NLC: stealth pass 5 (`xrGame/nlc_stealth.cpp/.h`)

New parameters are identity by default; `sivol_stealth_cfg.script` switches them on.

- **Monster hunt** (`hunt`; species `hunt_time`, `hunt_error_k`, `hunt_detect_k`, `hunt_alert_ms`,
  `hunt_flankers`, `hunt_flee_losses`): a bold monster whose third concern finds the actor out of
  `concern_enemy_range` commits to a hunt of the guessed shooter point (half the error), searches
  around it, sends pack flankers to points beside it, and afterwards stays alert ("lost the
  trail"); vision and aura senses run faster (`m_nlc_rate_k`, now applied to monsters too).
  A pack that lost `hunt_flee_losses` members flees instead.
- **Investigate impulses:** every repeated concern halves the point error; the gait is chosen
  once per episode and only upgrades; no retarget to a point within 8 m of the current target;
  the creature a bullet passes through gets no near miss.
- **Psy auras** use the live `actor_psy_k` (pushed by script from suit and belt) instead of the
  actor's telepathic immunity, which scales with difficulty.
- **Wall muffle** (`wall_mute`): actor sounds (not bullet impacts) heard through solid static
  geometry are scaled by `1 - wall_mute`; one static ray per listener, cached 250 ms; passable
  materials (bushes) do not count.
- **Bolts** (`bolt_range`): the first hard contact of a bolt thrown by the actor
  (`CMissile::ExitContactCallback`, physics thread, queued; handled in `CLevel::OnFrame`) is a faint noise
  (stalkers: heard kind 4; monsters: investigate) within `bolt_range` x the
  `[nlc_step_surface]` factor; a direct hit on a stalker calls `sivol_stealth_bolt.on_hit`.

## nlc-3.589.10 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `AF014A0519F11A3313951C11BE4F07F00245465ACD3271A4ACDE4DBA96809EA3`
- **Status:** runtime pending (stealth P4 build, `docs/STEALTH_DESIGN.md` 18.1 and 19).

### NLC

- **Monster investigate retarget** (`monster_state_hear_int_sound`): every accepted investigate
  impulse bumps a serial and the hear-interesting-sound state restarts toward the new point
  (`check_force_state`); a corpse check does not override a more urgent impulse (near miss,
  faint shot, sight) still in progress. Before, a monster walking to a corpse ignored later near
  misses.

## nlc-3.589.9 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `5A326042C868CFD609666D557ED97A7EE2A975511BC2C81D06E31B50F26CE19E`
- **Status:** runtime pending (stealth M4, `docs/STEALTH_DESIGN.md` section 18).

### NLC: stealth M4 (`xrGame/nlc_stealth.cpp/.h`)

New globals are identity by default; the script values switch them on.

- **Concern escalation** (`concern_escalate`, `concern_window_ms`; species `concern_bold`,
  `concern_enemy_range`): repeated near misses or faint shots make a monster hurry on the second
  event, then attack within range (bold) or flee (timid, an ownerless dangerous sound).
- **No relay chains** (`firsthand_ms`): pack mates pass the actor on only if they sensed him
  themselves recently (`CMonsterEnemyMemory::add_enemy` marks own-sense acquisitions).
- **Two-tier footsteps** (`step_alert_pow`; `sound_memory_manager.cpp`): actor footsteps weaker than
  the threshold are a heard suspicion (kind 3) for stalkers, not an enemy sound.
- **Impacts near stalkers:** an actor bullet hit sound within `near_miss_range` of a stalker counts
  like a near miss (any relation; scripts give non-hostiles a short worried reaction).
- **Footstep noise:** outfit key `stealth_noise_k` (`outfit_noise`), ground material table
  `[nlc_step_surface]` (`surface_noise`), rain masking of quiet sounds (`rain_mask`); applied to the
  actor's sounds for stalkers and monsters.
- **Dynamic lamps** (`lamp_k`, `lamp_period_ms`; `CHangingLamp::nlc_light`, lamp registry): lamps and
  spot lamps light the actor for AI (range falloff, spot cone, static-geometry ray), computed once per
  period for all observers; `vis` lines show `+lamp`.
- **Lua:** `nlc_stealth_actor_surface()` (ground material, its factor, outfit noise factor).

### Required game data

- Scripts: `sivol_stealth_suspicion.script` (worried reaction, corpse investigation, torch hook),
  `sivol_stealth_cfg.script`, `sivol_stealth_test.script`; hooks in `xr/xr_danger.script` (corpses),
  `xrs/xrs_battle_ai.script` (knife and any suppressed weapon count as a silent kill),
  `sr/sr_light.script` (torch on while investigating).
- Config: `creatures/m_stalker.ltx` (`[nlc_step_surface]`), species keys in `m_dog`, `m_pseudodog`,
  `m_boar`, `m_bloodsucker`, `m_snork`, `m_chimera`, `m_flesh`, `m_tushkano`, `m_rat`;
  `misc/all_outfits_nlc.ltx` (`stealth_noise_k`).

## nlc-3.589.8 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `1668F9477BB2EFF7B5BE8F62C33252825C787D523F3F08B7969EBFB23051F69C`
- **Status:** runtime pending (stealth M3, `docs/STEALTH_DESIGN.md` section 17).

### NLC: shots, near misses, corpses (`xrGame/nlc_stealth.cpp/.h`)

New globals are identity by default; the script values switch them on.

- **Two-tier shot hearing:** a heard actor shot weaker than `shot_alert_pow` (stalkers, weighted
  power; `sound_memory_manager.cpp`) or `monster_shot_alert_pow` / species `shot_alert_pow`
  (monsters; `base_monster_feel.cpp`) is concern only: stalkers get a heard suspicion value at a
  blurred point (suspicion stages run, no `attack_sound` danger), monsters investigate a blurred
  point. Louder shots work as before.
- **Near misses** (`Level_bullet_manager.cpp` `CalcBullet`): an actor bullet segment passing within
  `near_miss_range` of a creature gives concern toward a shooter guessed back along the bullet
  (distance error): monsters investigate, stalkers get a heard suspicion (kind "near miss") that the
  script turns into ducking or taking cover by temperament. Per-creature cooldown 1.5 s.
- **Corpse checks** (`CBaseMonster::Die`, `corpse_check`): the nearest calm squad mates within the
  species `corpse_check_radius` (up to `corpse_check_count`) investigate the corpse, the rest watch it.
- **Investigate styles** (`monster_state_hear_int_sound_inline.h`): monsters sent to investigate by
  the stealth layer walk, sneak (`ACT_STEAL`), hold and watch, or run, by a stable roll with the
  species `investigate_style_weights`; `mnotice` lines show `why` and `style`.
- **Lua:** `nlc_stealth_suspicion` returns the heard kind (0 sight, 1 faint shot, 2 near miss).
- **Diagnostics:** `macq src=hit_neutral` for the neutral-attacker hit path.

### Required game data

- `config/ogg_comments_overrides.ltx`: suppressed `ai_dist` raised to the outer radii (PB 8,
  pistols 13, 9x39 13, 5.45/5.56 20, 7.62x39 25, full power 28, 12 gauge 32).
- `config/creatures/m_dog.ltx`, `m_pseudodog.ltx`, `m_boar.ltx`, `m_flesh.ltx`, `m_snork.ltx`,
  `m_bloodsucker.ltx`: corpse check and investigate style keys.
- `gamedata/scripts/sivol/sivol_stealth_cfg.script` (new values),
  `sivol_stealth_suspicion.script` (shot-at reaction by temperament and rank).

## nlc-3.589.7 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `4C48BA7921655C02A6EA1760D937913BFD33CA290BD40D3FCA449E091C7D0B83`
- **Status:** runtime pending (stealth M2, mutant senses, `docs/STEALTH_DESIGN.md` section 16).

### NLC: mutant senses (`xrGame/nlc_stealth.cpp/.h`)

New globals are identity by default; species keys in `config/creatures/m_*.ltx` are absent
(vanilla) unless set.

- **Light model:** species `nlc_light_k` (share of the AI sky and near-range light terms for
  monster observers) and `nlc_dark_floor` (night vision); `visual_memory_manager.cpp`.
- **Partial cover and decay for monsters:** `monster_ray_resample`, `monster_mem_hold_ms`,
  `monster_mem_decay_s`.
- **Monster suspicion:** past `monster_notice_v` of its detection threshold a monster gets an
  "interesting" actor sound at the actor's position, so its own investigate states run
  (`monster_notice_ms` between impulses).
- **Aura sense** (generalised from `CPoltergeist::update_detection`; `CBaseMonster::shedule_Update`):
  species `nlc_sense_*` keys (range, near/far factors, actor-speed exponent and minimum, rate,
  loss, notice and success levels, psy scaling by the actor's telepathic immunity, rain, through
  walls); notice sends the monster to investigate, success makes the actor its enemy (sources
  `psy` / `smell`). Global `sense_mult`.
- **Pack sharing gate** (`CBaseMonster::feel_vision_isRelevant`): with `pack_gate`, a monster copies
  a pack mate's enemy only within the species `pack_share_range` and after a random delay
  (`pack_share_delay_min/max`); beyond the range it only investigates. Monsters already hunting
  the enemy refresh freely. The visible-body helper (`actor_legs`) never gives or receives (always).
- **Impact and near-miss gates:** species `feel_enemy_who_made_impact_max_distance`
  (`CMonsterEnemyMemory::update`, bullet impact and whine sounds) and
  `near_hit_shooter_max_distance` (`CBaseMonster::feel_sound_new`, the 2 m near-miss hit).
- **Rain:** species `nlc_rain_k` scales monster detection.
- **Lua:** `nlc_stealth_env_light()` (environment light: total, ambient, sky, sun) for the NPC
  night-vision darkness switch.
- **Diagnostics:** `mnotice` and `msense` lines; `macq` sources `psy`, `smell`.

### Required game data

- `config/creatures/m_dog.ltx`, `m_pseudodog.ltx`, `m_snork.ltx`, `m_bloodsucker.ltx`,
  `m_controller.ltx`, `m_burer.ltx`, `m_poltergeist.ltx`, `m_boar.ltx`, `m_flesh.ltx`: species
  keys and calm vision sections.
- `gamedata/scripts/sivol/sivol_stealth_cfg.script` (monster globals),
  `sivol_stealth_nvd.script` (darkness mode), `sivol_stealth_test.script` (monster `hold()`,
  environment light in `ctx()`).

## nlc-3.589.6 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `60BE35C44604D1C2A8C2EC1F95FAA224089774B28248B87FF83B219AC9A9163C`
- **Status:** runtime pending (stealth pass 3, `docs/STEALTH_DESIGN.md` section 15).

### NLC: stealth and perception (`xrGame/nlc_stealth.cpp/.h`)

New parameters are identity by default, so the exe alone behaves like nlc-3.589.5.

- **Partial cover** (`ray_resample`; `xr_3da/Feel_Vision.cpp` virtual
  `feel_vision_resample`, `CCustomMonster` override): stalkers re-trace the actor every
  update with a new sample point on the body, so detection scales with the visible part
  of the body. Before, one clear point was kept while nobody moved.
- **Suspicion exports** (`CVisualMemoryManager`): the actor's position at the last
  evaluation that added to the sum, a per-NPC rate factor; Lua `nlc_stealth_suspicion`,
  `nlc_stealth_set_rate_k`, `nlc_stealth_force_suspicion` (harness).
- **Night-vision devices:** per-NPC flag (`nlc_stealth_set_nvd`) and light floor
  `nvd_floor`; eye glow particle placed in front of the eyes every frame
  (`nlc_stealth_eye_glow`; `CAI_Stalker::UpdateCL`, stopped on death and net_Destroy).
- **Outfit and rain** (`outfit_vis`, `rain_k`): the actor outfit's hidden
  `stealth_visibility_k` and the rain density scale the detection rate for stalker
  observers.
- **NPC targets** (`npc_light_k`, `npc_sky_vis`, `npc_sun_k`): optional light model for
  stalkers seeing stalkers (environment light cached per frame, target torch flag cached
  for 0.5 s, muzzle flash after unsuppressed NPC shots, near term, night-vision floor),
  blended from the vanilla "always lit"; 0 keeps vanilla.
- **Diagnostics:** shot sounds are labelled with the weapon that fired (the actor's
  `wpn_pkp` logged `wpn=-` before); `vis` lines add `out`, `rain`, `nvd`, `ek`.

### Required game data

- `gamedata/scripts/sivol/sivol_stealth_suspicion.script` (new scheme; hooks in
  `modules.script`, `xr/xr_logic.script`, `xr/xr_motivator.script`),
  `sivol_stealth_nvd.script` (new; hook in `sr/sr_light.script`), both registered in
  `ogse/ogse_signals_addons_list.script`; `sivol_stealth_cfg.script` values.
- `config/misc/all_outfits_nlc.ltx`: `stealth_visibility_k` keys.
- `particles/stealth_nvg/nvg_dot.pe` (from the Anomaly addon "Stealth" 2.31, with
  permission).

## nlc-3.589.5 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `A241D7FC2E7EEBA27A51440D1E6587953FA164A78F383FA764DF30EE84ED33A9`
- **Status:** runtime pending (stealth batch 2, `docs/STEALTH_DESIGN.md` section 14).

### NLC: stealth and perception (`xrGame/nlc_stealth.cpp/.h`)

New parameters are identity by default, so the exe alone behaves like nlc-3.589.4.

- **Suspicion decay** (`mem_hold_ms`, `mem_decay_s`; `visual_memory_manager.cpp`
  `update`): a stalker that stops evaluating the actor (out of its view cone, ray
  blocked) keeps its partial detection sum for `mem_hold_ms`, then drains it over
  `mem_decay_s` from the full threshold, instead of resetting it at once. Monsters and
  other targets keep the instant reset. `nolos` lines show `sum` and `left`.
- **Heard-shot hit record gate** (`fakehit_min_pow`; `sound_memory_manager.cpp`): a
  heard actor shot adds the actor to the listener's hit memory (an amount-0 record that
  turns into an enemy candidate once the squad is in combat) only at this weighted power
  or more. `snd` lines show `gated=1`.

### Required game data

- `config/creatures/m_stalker.ltx`: new `[stalker_vision_tower_sniper]`.
- `gamedata/scripts/sivol/sivol_stealth_cfg.script`: the new parameters and
  `vision_profiles` (applies the tower section to the Agroprom tower snipers on spawn).

## nlc-3.589.4 (2026-10-02)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `7C6BA53A0470FAB27DC01BF96CE15B113010BFEC3151178EA41CC3C21017589A`
- **Status:** runtime tested in the test install (2026-10-02, stealth checks 1 to 5 of
  `docs/STEALTH_DESIGN.md` 13.1); design and measurements in `docs/STEALTH_DESIGN.md`.

### NLC: stealth and perception (`xrGame/nlc_stealth.cpp/.h`)

All terms are parameters set from Lua (`sivol_stealth_cfg.script`); every engine default
is identity, so the exe alone behaves like nlc-3.589.3. The shipped script values are
listed under game data.

- **Vision terms for the actor as target** (`visual_memory_manager.cpp`):
  - AI sky light (`sky_k`, `sky_pow`, `sky_scale`): env hemisphere brightness x sky
    visibility added to the actor's luminosity for stalker observers. The renderer scales
    its sky term by `ps_r2_dhemi_sky_scale` (0.08), so before this, shade at noon read like
    night and only direct sun or static lamps mattered.
  - Near-range contrast (`near_k`, `near_range`), observer flashlight cone (`torch_k`,
    `torch_cone`; NPC torches are found in the torch slot or among attached items),
    muzzle flash after an unsuppressed actor shot (`flash_k`, `flash_ms`; hook in
    `CWeaponMagazined::OnShot`).
  - Rate factors: observer rank (`rank_k_novice/experienced/veteran/master`, thresholds
    from `[game_relations] rating`) and actor stance (`crouch_k`, `creep_k`); never inside
    `always_visible_distance`.
  - Actor speed from physics movement (`vel_physics`) instead of the position history,
    which alternated between real and doubled values.
  - Live overrides of the existing formula: `free_rate_mult`, `danger_rate_mult`,
    `monster_rate_mult`, `lum_factor_override`, `transparency_factor_override`.
- **Per-NPC vision sections:** `nlc_stealth_set_vision(id, free, danger)` reloads an
  observer's `CVisionParameters` (`CVisualMemoryManager::nlc_set_vision_sections`); lasts
  until the NPC reloads its sections.
- **Diagnostics** (off by default): per-observer `~ [stealth]` lines for vision factors
  (`vis`, `nolos`, `seen`), actor sounds heard by stalkers (`snd`, with the actor's weapon
  and suppressor state) and monsters (`msnd`), stalker enemy selection context (`enemy`),
  monster enemy acquisition source (`macq`); once per episode for `enemy` / `macq`; the
  visible-body helper (`actor_legs`) is skipped. Hooks in `sound_memory_manager.cpp`,
  `base_monster_feel.cpp`, `enemy_manager.cpp`, `monster_enemy_memory.cpp`,
  `script_game_object2/3.cpp` (script-induced awareness timestamps).
- **Lua globals:** `nlc_stealth_set/get/dump/watch/unwatch/unwatch_all/reset/last_seen/
  set_vision` (registered in `COMMON_AI/script_engine_export.cpp`). Debug parameter
  `force_profile` forces free or danger vision on watched observers.

### NLC: sound loader (`xrSound/SoundRender_Source_loader.cpp`)

- A `[path.with.dots]` section in the system ini now overrides only the keys it sets on
  top of the `.ogg` comment. Before, any such section skipped the comment, so unset keys
  (range, volume, sound type) fell back to engine defaults. The false "Missing
  ogg-comment" warning for overridden sounds is gone.

### Required game data

- `gamedata/scripts/sivol/sivol_stealth_cfg.script` (tuning values, registered in
  `ogse_signals_addons_list.script`), `sivol_stealth_test.script` (measurement harness).
- `config/creatures/m_stalker.ltx`: stalker vision sections (`always_visible_distance`
  2 when calm, `luminocity_factor` 0.8, danger `time_quant` 0.00286, `transparency_factor`
  1) and the new `[actor_step_manager]` with landing rows; `config/creatures/actor.ltx`
  points `step_params` at it.
- `config/ogg_comments_overrides.ltx`: `ai_dist` for every suppressed shot sound.

## nlc-3.589.3 (2026-10-01)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), unchanged.
- **exe SHA-256:** `E145FBA5C3671C8C4E8FCE113547E63B951C43D78B288ABA4D42453DA5877F5A`

### NLC

- Monster target selection (`monster_enemy_memory.cpp`, `monster_hit_memory.cpp`):
  `[monster_target_selection]` in `game_relations.ltx` sets `actor_bias`,
  `target_stickiness`, `hit_bonus` and `hit_bonus_time`, with optional per-monster
  overrides. Missing keys keep the original danger formula. `debug_log = true`
  logs every target switch.
- Duplet (`WeaponShotgun.cpp`): `duplet_on_alt_aim = true` fires both barrels from
  the alt-aim key on weapons without alt-aim zoom. Both shots raise the actor
  weapon-fire callback, so script recoil sees two cartridges. Optional
  `anm_shots_both` / `anm_shots_both_aim` HUD motions.
- Wound model (`EntityCondition.cpp`, `Wound.cpp`): `wound_model = 2` in a
  creature's condition section sums wound sizes and heals wounds in proportion to
  their size, so total bleeding is a fixed share of the damage dealt. Default 1
  keeps the original behaviour (actor and stalkers unchanged).
- Attack-on-move monsters (`base_monster`, attack states): `aom_close_melee_dist`
  switches to the standing melee state when the enemy is that close, until it
  moves past `MaxAttackDist`. Default 0 (off).
- Version text `nlc-3.589.3, OGSR main 2021123`.

### Required game data (same repository)

- `game_relations.ltx`: `[monster_target_selection]`; actor relation `-2` -> `-1`
  for flesh, dog, cat, chimera, giant, zombie, snork, fracture and zombi.
- Weapon configs: `duplet_on_alt_aim` and duplet HUD motions on the TOZ-66, the
  sawn-off TOZ-66, the TOZ-34 and the Ruzhye Orekha.
- Creature configs: `wound_model = 2` on dogs, cats, pseudodogs, boars and flesh;
  `aom_close_melee_dist = 1.6` on dogs.

## nlc-3.589.2 (2026-09-30)

- Server object destruction (`xrServer_process_event_destroy.cpp`): destroy
  events are flushed into a new event pack before the pack would exceed
  `NET_PacketSizeLimit` (fixed lost items when many objects were destroyed at
  once, e.g. Sidorovich's stock).
- **exe SHA-256:** `01026234925690EF58BE213DE2DE29558455F24F0C182FB99A8E74C5F11C1C0F`

## nlc-3.589.1 (2026-09-30)

- **Upstream base:** OGSR `main` `2021123` (2026-09-27), newer than release
  "3.589" (tag `3.548`). Previous NLC build: "3.526" (OGSR tag `3.525` + NLC,
  codev build of 2026-09-28).
- **exe SHA-256:** `5A3F8D71BA5DA732BA597C7C6697A0607F27D9E907594AA814BFCAAAE07C0CF3`

### From upstream OGSR

- DLSS quality presets (`r_aa_dlss_quality`: DLAA, Quality, Balanced,
  Performance, Ultra Performance) and DLSS model presets (`r_aa_dlss_preset`).
- FSR3 replaces FSR2 (`r_aa_mode st_opt_fsr3`, `r_aa_fsr3_quality`); FSR3 is
  linked into the exe, the `ffx_fsr2_*.dll` files are no longer used. DLSS SDK
  v310.7.0 (`nvngx_dlss.dll`).
- `g_weapon_bobbing`, `r_lens_flare_mode`, simulated fullscreen (`rs_fullscreen`),
  `input_exclusive_mode` setting, sun-shadow and water fixes, weather fixes,
  ImGui update, and more (103 upstream commits after `3.525`).

### NLC

- All NLC engine changes of the previous build carried over: `fl_hook` features
  (console commands, Lua API, knife combo), `.sq` archive support, gameplay, UI
  and script changes, `script_random`.
- Settings file stays `appdata/user.ltx` (upstream renamed it to `user_ogsr.ltx`).
- Active task is not switched by priority when `dont_switch_active_task_by_prio`
  is set in `external.ltx` (NLC sets it to `true`).
- Scripted aim sway: `CEffectorZoomInertion` honours `switch_zoom_osc(true)`
  instead of replacing the script's target with a random point (fixed a fast
  ADS camera shake).
- Weapon bobbing: `g_weapon_bobbing_ads_only` (bob only while aiming, regular
  walk animations otherwise) and `g_weapon_bobbing_mode` (off/always/aim) for
  the menu; fade-in restarts after a pause; walk animation is re-selected when
  leaving ADS while moving. Default: while aiming.
- Fresh-start defaults (no settings file): crosshair, dynamic crosshair and NPC
  info off; hard crosshair, status icons, 3D scopes and the dynamic item
  description window on.
- Version text `nlc-3.589.1, OGSR main 2021123` in the log and main menu.

### Required game data (same repository)

- Upstream 3.589 shaders as loose overrides in `gamedata/shaders/r3` (the engine
  crashes at startup without `temporal_resolve`), merged
  `models_selflight_det_3*.s`.
- Menu rows and strings for DLSS/FSR3 quality, DLSS model, sun flare style,
  fullscreen, weapon bobbing mode; `external.ltx`
  `dont_switch_active_task_by_prio = true`.

### Build

- Dependencies re-pinned in `nlc_tools/fetch_deps.ps1` to upstream's
  2026-09-27 set (mimalloc v3.5.1, DLSS v310.7.0, FidelityFX-SDK for FSR3).
- `nlc_tools/build_release.ps1` archives every exe with its PDB in
  `nlc_tools/builds/`.
