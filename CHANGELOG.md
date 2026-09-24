<!-- nlc-changelog-head: da95f16df35e854dc0183ff5ed31360ee71ad175 -->

# NLC Improved changelog

<!-- nlc-changelog-commit: da95f16df35e854dc0183ff5ed31360ee71ad175 -->
## 24.09.26 08:53 МСК - Add Sidor container map quest item

Commit: [da95f16](https://github.com/jordan1227/improved_to_he/commit/da95f16df35e854dc0183ff5ed31360ee71ad175)

### Description

Adds the Sidor container map as a new paper/quest item, including the dialogue reward, item definitions, localization strings, and inventory interaction hints. The paper reader now resolves a per-spawn HUD variant for the map and the new texture/model assets are wired into the item registry for use in the game.

### Game files changed

- M gamedata/config/gameplay/dialogs_escape.xml
- M gamedata/config/misc/items/anim_items.ltx
- M gamedata/config/misc/sak_quest_items.ltx
- M gamedata/config/text/eng/he_watch.xml
- M gamedata/config/text/eng/sak_strings.xml
- M gamedata/config/text/eng/string_table_items.xml
- M gamedata/config/text/eng/ui_st_inventory.xml
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/config/text/rus/string_table_items.xml
- M gamedata/config/text/rus/ui_st_inventory.xml
- M gamedata/config/text/rus/ui_st_mm.xml
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container1.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container2.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container3.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container4.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container5.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container6.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container7.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container8.ogf
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_sidor_container9.ogf
- M gamedata/scripts/alpet/nlc_container.script
- M gamedata/scripts/amk/amk_mod.script
- M gamedata/scripts/ui/ui_inv_descr.script
- M gamedata/scripts/ui/ui_paper_reader.script
- M gamedata/scripts/vergas/inv_buttons_use.script
- M gamedata/scripts/vergas/vergas_lib.script
- A gamedata/textures/liz/sidor_container1.dds
- A gamedata/textures/liz/sidor_container2.dds
- A gamedata/textures/liz/sidor_container3.dds
- A gamedata/textures/liz/sidor_container4.dds
- A gamedata/textures/liz/sidor_container5.dds
- A gamedata/textures/liz/sidor_container6.dds
- A gamedata/textures/liz/sidor_container7.dds
- A gamedata/textures/liz/sidor_container8.dds
- A gamedata/textures/liz/sidor_container9.dds

<!-- nlc-changelog-commit: 4182bb2119d595dcd8807baf7efdeb5e841abbdd -->
## 24.09.26 06:52 МСК - Add upgraded knife HUD animations, combo & Lua sounds

Commit: [4182bb2](https://github.com/jordan1227/improved_to_he/commit/4182bb2119d595dcd8807baf7efdeb5e841abbdd)

### Description

Introduce knife HUD combo handling and delegated draw/holster sounds.

- Hook flhook HUD playmotion (guarded by engine stamp/size) to support M1/M2 combo variants, return transitions, and logging.
- Delegate show/hide sound playback to a new Lua module instead of engine for the M1 draw/hide events via fl_on_knife_motion callback.
- Add sivol_knife.script implementing the Lua callback and register it in ogse_signals_addons_list.
- Update w_knife.ltx to map new animations and sound keys.
- Add multiple new camera animation and hud/hand assets and update UI texture/meshes.

Files: fl_hook/flhook.cpp, gamedata/config/weapons/w_knife.ltx, gamedata/scripts/sivol/sivol_knife.script, gamedata/anims/*, meshes and textures; plus many binary asset updates.

### Game files changed

- A gamedata/anims/camera_effects/weapon/liz_knife_bored.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_hide.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_hide_quick.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_hit1_start.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_hit2_start.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_hit3_start.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_hit4_start.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_idle1.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_show.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_show_quick.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_stab.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_stab2_variant.anm
- A gamedata/anims/camera_effects/weapon/liz_knife_stab3_variant.anm
- M gamedata/config/weapons/w_knife.ltx
- A gamedata/meshes/anomaly_weapons/hud_hands_animation/liz_knife_hud_hand_animations.omf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand__brown.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_bandit_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_bandit_novice.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_bandit_trench_gloves.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_cs1.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_cs3a.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_ddd.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_dolg_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_duty_psz9d.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_duty_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_ecologist_ssp99.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_ecologist_ssp99m.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_guard.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_guardian.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_wind.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_novice.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_novice_white.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_sunrise.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_radiation_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_military_berill.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_military_novice.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_military_skat9.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_radiation_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_no_outfit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_skat9m.ogf
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- A gamedata/scripts/sivol/sivol_knife.script
- M gamedata/scripts/vergas/vergas_art.script
- A gamedata/sounds/weapons/liz_knife_attack1.ogg
- A gamedata/sounds/weapons/liz_knife_attack2.ogg
- A gamedata/sounds/weapons/liz_knife_attack3.ogg
- A gamedata/sounds/weapons/liz_knife_hide.ogg
- A gamedata/sounds/weapons/liz_knife_show.ogg
- M gamedata/textures/ui/ui_icon_equipment_7.dds

### Other repository files changed

- M fl_hook/dinput8.dll
- M fl_hook/flhook.cpp

<!-- nlc-changelog-commit: 2cac18df852677e11e42abe46c9ce730cefa1e9f -->
## 23.09.26 23:29 МСК - Фикс входа на базу Свободы

Commit: [2cac18d](https://github.com/jordan1227/improved_to_he/commit/2cac18df852677e11e42abe46c9ce730cefa1e9f)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/gulag_mil_freedom1.ltx
- M gamedata/config/misc/gulag_mil_freedom_he.ltx
- M gamedata/scripts/xr/xr_gulag.script

<!-- nlc-changelog-commit: e8b480247c5c5c9772f3db251d7a536d0199c7a0 -->
## 23.09.26 20:04 МСК - Колбаса. Настоящие цены.

Commit: [e8b4802](https://github.com/jordan1227/improved_to_he/commit/e8b480247c5c5c9772f3db251d7a536d0199c7a0)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/trade_manager.script

<!-- nlc-changelog-commit: 71ca121fe5f67d848b098375d09a4fd3e13f9f87 -->
## 23.09.26 19:23 МСК - Награда за захват базы Свободы

Commit: [71ca121](https://github.com/jordan1227/improved_to_he/commit/71ca121fe5f67d848b098375d09a4fd3e13f9f87)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/character_desc_nlc30_spawn.xml
- M gamedata/config/gameplay/dialogs_bar.xml
- M gamedata/config/gameplay/dialogs_military.xml
- M gamedata/config/gameplay/info_portions_nlc30.xml
- M gamedata/config/gameplay/sak_dalogs_bar_yan.xml
- M gamedata/config/misc/gulag_bar.ltx
- M gamedata/config/misc/shop_petrenko/trade_petrenko.ltx
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/scripts/he_mil_dolg_base.script
- A gamedata/scripts/he_mil_dolg_reward.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/olr/olr_squads.script
- M gamedata/scripts/se/se_respawn.script

<!-- nlc-changelog-commit: aa1b5d86d7255359385915257b3dde3eb48ae41b -->
## 23.09.26 18:54 МСК - Цены Игнату

Commit: [aa1b5d8](https://github.com/jordan1227/improved_to_he/commit/aa1b5d86d7255359385915257b3dde3eb48ae41b)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/character_desc_nlc30_spawn.xml
- M gamedata/config/misc/trade_npc/trade_dan.ltx

<!-- nlc-changelog-commit: 83e8da686e3d627062026bb451cd89812ff7770f -->
## 23.09.26 18:33 МСК - Спавн новых свободовцев после активации квеста

Commit: [83e8da6](https://github.com/jordan1227/improved_to_he/commit/83e8da686e3d627062026bb451cd89812ff7770f)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/creatures/spawn_sections.ltx
- M gamedata/config/gameplay/info_portions_nlc30.xml
- A gamedata/config/misc/gulag_mil_freedom_he.ltx
- M gamedata/config/misc/gulag_military.ltx
- M gamedata/config/waypoints.ltx
- M gamedata/scripts/gulag/gulag_military.script
- M gamedata/scripts/he_mil_dolg_base.script
- A gamedata/scripts/he_mil_freedom_reinf.script
- M gamedata/scripts/olr/olr_protected.script

<!-- nlc-changelog-commit: 3ee1a89eeb495d9008166a5a9ddfd16f52e81aaa -->
## 23.09.26 18:04 МСК - Вернул правки sak.script и info_portions, затёртые штурмом Хутора

Commit: [3ee1a89](https://github.com/jordan1227/improved_to_he/commit/3ee1a89eeb495d9008166a5a9ddfd16f52e81aaa)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/info_portions_nlc30.xml
- M gamedata/scripts/sak/sak.script

<!-- nlc-changelog-commit: 7c23eea62cabd4135c8b247262e8efe2829fc6ba -->
## 23.09.26 15:21 МСК - Вернул штурм Хутора/Базы свободы из 3.0

Commit: [7c23eea](https://github.com/jordan1227/improved_to_he/commit/7c23eea62cabd4135c8b247262e8efe2829fc6ba)

### Description

Если ГГ убивает Лукаша в ходе штурма, то база Свободы переходит к долгу, Череп становится на место Лукаша, начинает работать миграция долговцев на базу свободы.

### Game files changed

- M gamedata/config/gameplay/dialogs_military.xml
- M gamedata/config/gameplay/info_portions_nlc30.xml
- M gamedata/config/misc/gulag_mil_dolg.ltx
- M gamedata/config/misc/gulag_mil_freedom1.ltx
- A gamedata/config/misc/gulag_mil_freedom3.ltx
- A gamedata/config/misc/gulag_mil_freedom4.ltx
- M gamedata/config/misc/gulag_military.ltx
- M gamedata/scripts/gulag/gulag_military.script
- A gamedata/scripts/he_mil_dolg_base.script
- M gamedata/scripts/logic/smart_terrain.script
- M gamedata/scripts/olr/olr_faction_policy.script
- M gamedata/scripts/sak/sak.script
- M gamedata/scripts/sak/sak_tasks.script
- M gamedata/scripts/se/se_respawn.script
- M gamedata/scripts/xr/xr_effects.script

<!-- nlc-changelog-commit: 56aba646880e060b5347edc29a969481be602847 -->
## 23.09.26 15:02 МСК - Убрал дебаг метки

Commit: [56aba64](https://github.com/jordan1227/improved_to_he/commit/56aba646880e060b5347edc29a969481be602847)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/kotovod/trade_arts_news.script

<!-- nlc-changelog-commit: 03b35e15048aa2a1653bbdb09b255e65e8e52987 -->
## 23.09.26 14:40 МСК - Крот на Агропроме: убран таймер выхода из укрытия, как в 3.0

Commit: [03b35e1](https://github.com/jordan1227/improved_to_he/commit/03b35e15048aa2a1653bbdb09b255e65e8e52987)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/scripts/agr/agr_krot.ltx

<!-- nlc-changelog-commit: 1a047a1a85e965ee0de7f4b8c2e8b2e430ee051b -->
## 23.09.26 14:40 МСК - Вернул полные звуки сталкерам на агро

Commit: [1a047a1](https://github.com/jordan1227/improved_to_he/commit/1a047a1a85e965ee0de7f4b8c2e8b2e430ee051b)

### Description

_(No additional description.)_

### Game files changed

- A gamedata/sounds/characters_voice/scenario/agroprom/stalker_help_1.ogg
- A gamedata/sounds/characters_voice/scenario/agroprom/stalker_help_2.ogg

<!-- nlc-changelog-commit: b2b5208a572a6d728d5fc7d24cd15d715d9bf985 -->
## 23.09.26 14:31 МСК - Умирающий затыкается после разговора с ГГ

Commit: [b2b5208](https://github.com/jordan1227/improved_to_he/commit/b2b5208a572a6d728d5fc7d24cd15d715d9bf985)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/scripts/mil/mil_dying.ltx

<!-- nlc-changelog-commit: adfd41d4a3aa028fb32d1a0355f2b950f3d59633 -->
## 23.09.26 14:23 МСК - Свобода больше не обижается за бандитов

Commit: [adfd41d](https://github.com/jordan1227/improved_to_he/commit/adfd41d4a3aa028fb32d1a0355f2b950f3d59633)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/xr/xr_gulag.script
- M gamedata/scripts/xr/xr_motivator.script

<!-- nlc-changelog-commit: f1f9125a0489cfee923c67faae784fe9a790a6f4 -->
## 23.09.26 11:26 МСК - Манифест

Commit: [f1f9125](https://github.com/jordan1227/improved_to_he/commit/f1f9125a0489cfee923c67faae784fe9a790a6f4)

### Description

This update fixes the Esc Shustryi tutorial timing and sleep/wake flow, tightens the dead novice energy drink spawn logic, reduces energy-drink effectiveness, adds the white loner jacket and PDA cleanup fixes, and bumps the patch/version metadata in the changelog and UI strings.

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml

### Other repository files changed

- M CHANGELOG.md
- M manifest.txt

<!-- nlc-changelog-commit: d732d3c2338b29051f706c800239a03e89cb2171 -->
## 23.09.26 10:42 МСК - Фикс Шустрого, должен идти спать после спасения

Commit: [d732d3c](https://github.com/jordan1227/improved_to_he/commit/d732d3c2338b29051f706c800239a03e89cb2171)

### Description

Rework esc_shustryi AI: move tutorial trigger earlier (on_info2 -> tutorial_wounded_give_info to walker6) and shift the original quest remark to on_info3. Update [walker6] to use shustryi_look, mark online, remove sprint movement overrides, use a 30s on_timer to wake the sleeper@esc_lager, set meet to meet@friend, and hide its show_spot. These changes make the NPC's movement/interaction timing and tutorial messaging more reliable.

Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>

### Game files changed

- M gamedata/config/scripts/esc/esc_shustryi.ltx

<!-- nlc-changelog-commit: 58ff2d4c266a52713f2d98232e72a351ac1cf2fe -->
## 23.09.26 10:12 МСК - Spawn energy drink once; tighten note spawn

Commit: [58ff2d4](https://github.com/jordan1227/improved_to_he/commit/58ff2d4c266a52713f2d98232e72a351ac1cf2fe)

### Description

Refactors ensure_dead_novice_letter: stop early-return on alife flag so the energy drink logic can still run; only spawn dead_novice_letter when it's not already in the corpse and esc_tutorial_dead_novice alife info is absent. Iterate_inventory now guards against nil items and detects existing energy_drink. Introduces a persistent key (sak.dead_novice_energy_drink_spawned) to ensure energy_drink is spawned at most once and record when one is already present. Prevents duplicate spawns and avoids nil dereference in inventory iteration.

### Game files changed

- M gamedata/scripts/sak/sak.script

<!-- nlc-changelog-commit: ee280f3b7400793014b085d5df8d0479ce023a5e -->
## 23.09.26 10:10 МСК - Меньше яркость энергосу, -20% эффекта, x5 длительность

Commit: [ee280f3](https://github.com/jordan1227/improved_to_he/commit/ee280f3b7400793014b085d5df8d0479ce023a5e)

### Description

Add new animation file gamedata/anims/brighten.ppe. Update gamedata/config/misc/items.ltx: increase mr_time from 2 to 10 and reduce mr_eat_power from 1.0 to 0.8 to slow recovery/eat effectiveness for balance.

### Game files changed

- A gamedata/anims/brighten.ppe
- M gamedata/config/misc/items.ltx

<!-- nlc-changelog-commit: 7a81f2245867c4ba21f70452a044f468ad6c8e20 -->
## 23.09.26 10:03 МСК - Centralize dialog actions, add prapor utilities

Commit: [7a81f22](https://github.com/jordan1227/improved_to_he/commit/7a81f2245867c4ba21f70452a044f468ad6c8e20)

### Description

Replaced inline dialog logic with centralized sivol functions and removed duplicate implementations.

- kill_stalker_dlg.xml: replaced direct sak.create_items call with sivol.give_mutant_parts_and_update_dan.
- sak_dalogs_esc_gar.xml: switched precondition/action to sivol.prapor_good_flame_have and sivol.prapor_good_flame_remove to handle fully-charged 'flame' weapon transfers.
- gamedata/scripts/sivol/sivol.script: added give_mutant_parts_and_update_dan (creates mutant parts, updates Dan's trade_manager/resupply time and schedules a tip), show_dan_tip, prapor_flame_is_fully_charged, prapor_good_flame_have, prapor_good_flame_have_not, transfer_prapor_flame, prapor_good_flame_remove, prapor_partial_flame_remove, and a flyto helper.
- gamedata/scripts/sivol/sivol_masks.script: removed the old duplicated implementations and adjusted section header to avoid redundancy.

Purpose: centralize inventory/transfer and trader-resupply logic, keep dialog XMLs thin, and avoid duplicated code paths for flame-weapon transfers and Dan resupply updates.

### Game files changed

- M gamedata/config/gameplay/kill_stalker_dlg.xml
- M gamedata/config/gameplay/sak_dalogs_esc_gar.xml
- M gamedata/scripts/sivol/sivol.script
- M gamedata/scripts/sivol/sivol_masks.script

<!-- nlc-changelog-commit: 269ec69fd3d98d8f596077c626011e6962a671ab -->
## 23.09.26 10:02 МСК - Add white loner jacket visual and UI icon

Commit: [269ec69](https://github.com/jordan1227/improved_to_he/commit/269ec69fd3d98d8f596077c626011e6962a671ab)

### Description

Introduce a new white loner jacket visual and update references.

- Added mesh gamedata/meshes/sm/actor_legs/jacket_loner_white.ogf
- Registered [actor_legs_jacket_loner_white] in gamedata/config/misc/sivol_visible_body.ltx
- Updated gamedata/scripts/sivol/sivol_visible_body.script to map neytral_novice_gaz_outfit_m1 to jacket_loner_white
- Replaced gamedata/textures/ui/ui_icons_npc_suits.dds (updated UI icons)
- Added /packaging to .gitignore

These changes wire the new asset into the visible-body system and update the UI icon texture.

### Game files changed

- M gamedata/config/misc/sivol_visible_body.ltx
- A gamedata/meshes/sm/actor_legs/jacket_loner_white.ogf
- M gamedata/scripts/sivol/sivol_visible_body.script
- M gamedata/textures/ui/ui_icons_npc_suits.dds

### Other repository files changed

- M .gitignore

<!-- nlc-changelog-commit: ad891c5375834d5ca3c864b79aed4f0ba9ec55f1 -->
## 23.09.26 06:34 МСК - Fix PDA startup removal and enable PDA

Commit: [ad891c5](https://github.com/jordan1227/improved_to_he/commit/ad891c5375834d5ca3c864b79aed4f0ba9ec55f1)

### Description

Tighten the PDA startup-removal check in he_pda_market.script: cache he_pda_model.get_pda(), ensure the PDA's section is a valid PDA section before calling cancel_startup_removal().

Also fix sak.script so the actor is allowed to use the PDA when the "pda_is_broken" alife info flag exists (change set_actor_allow_pda from false to true). These changes prevent incorrect startup-removal and restore PDA access when the broken flag is present.

### Game files changed

- M gamedata/scripts/he_pda_market.script
- M gamedata/scripts/sak/sak.script

<!-- nlc-changelog-commit: a70d1d9013c6a494be4d439311d8552c350627d7 -->
## 23.09.26 02:56 МСК - Adjust blood pool lifetime and size

Commit: [a70d1d9](https://github.com/jordan1227/improved_to_he/commit/a70d1d9013c6a494be4d439311d8552c350627d7)

### Description

Increase blood pool persistence from 200s to 600s and reduce the maximum final pool size from 1.2 to 1.0 to tune the visual footprint and duration of ground blood effects.

### Game files changed

- M gamedata/scripts/sivol/sivol_blood_pools.script

<!-- nlc-changelog-commit: 5004521d0a1ac326d4ecb6236f00d46c3aa9a782 -->
## 23.09.26 01:23 МСК - Цены торговли как в 3.0

Commit: [5004521](https://github.com/jordan1227/improved_to_he/commit/5004521d0a1ac326d4ecb6236f00d46c3aa9a782)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/shop_barman/barman_buy.ltx
- M gamedata/config/misc/shop_barman/barman_trade.ltx
- M gamedata/config/misc/shop_mikhalych/mikhalych_barter.ltx
- M gamedata/config/misc/shop_mikhalych/mikhalych_buy.ltx
- M gamedata/config/misc/shop_other/junkman_barter.ltx
- M gamedata/config/misc/shop_other/trade_junkman.ltx
- M gamedata/config/misc/shop_other/trade_silvestr.ltx
- M gamedata/config/misc/shop_pahom/pahom_buy.ltx
- M gamedata/config/misc/shop_pahom/pahom_trade.ltx
- M gamedata/config/misc/shop_sakharov/barter_saharov.ltx
- M gamedata/config/misc/shop_sakharov/trade_sakharov.ltx
- M gamedata/config/misc/shop_sidor/sidor_barter.ltx
- M gamedata/config/misc/shop_sidor/sidor_modificats.ltx
- M gamedata/config/misc/shop_sidor/trade_sidor.ltx
- M gamedata/config/misc/trade_logic/trade_generic_bnd.ltx
- M gamedata/config/misc/trade_logic/trade_generic_dlg.ltx
- M gamedata/config/misc/trade_logic/trade_generic_ecl.ltx
- M gamedata/config/misc/trade_logic/trade_generic_frd.ltx
- M gamedata/config/misc/trade_logic/trade_generic_grn.ltx
- M gamedata/config/misc/trade_logic/trade_generic_mil.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_exp.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_mas.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_nov.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_vet.ltx
- M gamedata/config/misc/trade_npc/trade_dsc_part_exchanger.ltx
- M gamedata/config/misc/trade_npc/trade_ignat.ltx
- M gamedata/config/misc/trade_npc/trade_jila.ltx
- M gamedata/config/misc/trade_npc/trade_kruglov.ltx
- M gamedata/config/misc/trade_npc/trade_sherstyk.ltx

<!-- nlc-changelog-commit: a6718f8dee3cea859e557a308c668c3979fe515d -->
## 23.09.26 00:30 МСК - Кондиция 95%

Commit: [a6718f8](https://github.com/jordan1227/improved_to_he/commit/a6718f8dee3cea859e557a308c668c3979fe515d)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_barter.script

<!-- nlc-changelog-commit: 3cddf6d710c0541b63d21129843bc4aa3d726dff -->
## 22.09.26 23:46 МСК - Кружка

Commit: [3cddf6d](https://github.com/jordan1227/improved_to_he/commit/3cddf6d710c0541b63d21129843bc4aa3d726dff)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/dialogs_escape.xml
- M gamedata/config/gameplay/dialogs_he_pda_access.xml
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/scripts/nlc_fixes.script
- M gamedata/scripts/vergas/vergas_krujka.script
- M gamedata/spawns/all.spawn

### Other repository files changed

- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_hospital.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/hospital.sections

<!-- nlc-changelog-commit: 334997fcc5e0f18c39b2b0ebcc729842a6b7f4c9 -->
## 22.09.26 23:45 МСК - Поменял цены

Commit: [334997f](https://github.com/jordan1227/improved_to_he/commit/334997fcc5e0f18c39b2b0ebcc729842a6b7f4c9)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/shop_barman/barman_trade.ltx
- M gamedata/config/misc/shop_sidor/trade_sidor.ltx

<!-- nlc-changelog-commit: 5e3016c85105ad25b31c72d1442d4981e18373a7 -->
## 22.09.26 21:17 МСК - Фикс рукавов на ТЧ руках

Commit: [5e3016c](https://github.com/jordan1227/improved_to_he/commit/5e3016c85105ad25b31c72d1442d4981e18373a7)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/kotovod/he_sleeves.script

<!-- nlc-changelog-commit: 275ad22702e8b56b7903d90e537b1e61c0899d39 -->
## 22.09.26 20:56 МСК - Снова скрыл часы с рук

Commit: [275ad22](https://github.com/jordan1227/improved_to_he/commit/275ad22702e8b56b7903d90e537b1e61c0899d39)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/watch_gate.script

<!-- nlc-changelog-commit: b9856fd9187c228b24e7b7234ae897243a757c4c -->
## 22.09.26 20:41 МСК - Убрал иконки при взломе пда

Commit: [b9856fd](https://github.com/jordan1227/improved_to_he/commit/b9856fd9187c228b24e7b7234ae897243a757c4c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/ui/ui_pda_addons.script

<!-- nlc-changelog-commit: d2ac3cd6bfad7068a5e2071849629b8243db1981 -->
## 22.09.26 20:31 МСК - Убрал нейтралов-военных с Кордона

Commit: [d2ac3cd](https://github.com/jordan1227/improved_to_he/commit/d2ac3cd6bfad7068a5e2071849629b8243db1981)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/olr/olr_faction_policy.script
- M gamedata/scripts/olr/olr_offline_task.script
- M gamedata/scripts/olr/olr_relations.script

<!-- nlc-changelog-commit: 7b87677460415608fb33a252abdf67eb23968569 -->
## 22.09.26 18:24 МСК - Окончательный фикс Лиса

Commit: [7b87677](https://github.com/jordan1227/improved_to_he/commit/7b87677460415608fb33a252abdf67eb23968569)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/scripts/esc/esc_dogs_fox_attack.ltx
- M gamedata/scripts/gnzi.script
- A gamedata/scripts/he_fox_dogs.script
- M gamedata/scripts/xr/xr_conditions.script

<!-- nlc-changelog-commit: 9e9d4d09665e5cc009774f4563c1effd98e00d7c -->
## 22.09.26 16:44 МСК - Пофиксил Лиса

Commit: [9e9d4d0](https://github.com/jordan1227/improved_to_he/commit/9e9d4d09665e5cc009774f4563c1effd98e00d7c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/gulag_escape.ltx
- M gamedata/scripts/gulag/gulag_escape.script

<!-- nlc-changelog-commit: e0b30a4dd26c92cc923869a409bdce570ae00d20 -->
## 22.09.26 16:16 МСК - Отравление просроченной едой из NLC 3.0

Commit: [e0b30a4](https://github.com/jordan1227/improved_to_he/commit/e0b30a4dd26c92cc923869a409bdce570ae00d20)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/binders/bind_stalker.script
- M gamedata/scripts/kotovod.script

<!-- nlc-changelog-commit: 064526caedd351b671b7bc543ca00ae2bebdf492 -->
## 22.09.26 14:06 МСК - Фикс переходов

Commit: [064526c](https://github.com/jordan1227/improved_to_he/commit/064526caedd351b671b7bc543ca00ae2bebdf492)

### Description

_(No additional description.)_

### Game files changed

- A gamedata/scripts/he_fix_lc_vertex.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script

<!-- nlc-changelog-commit: b6c3b56af7df29fd1c96fb3fcc3453ac73a4a4b4 -->
## 22.09.26 14:06 МСК - Фикс переходов

Commit: [b6c3b56](https://github.com/jordan1227/improved_to_he/commit/b6c3b56af7df29fd1c96fb3fcc3453ac73a4a4b4)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/spawns/all.spawn

### Other repository files changed

- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_aver.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_deadcity.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_generators.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_hospital.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_jupiter.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_k01_darkscape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l01_escape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l02_garbage.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l04u_labx18.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l05_bar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l06_rostok.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l07_military.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l08_yantar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l10_radar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l11_pripyat.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_la15_darkscape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_labx10.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_lost_village.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_marsh.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_mine.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_red_forest.ltx

<!-- nlc-changelog-commit: 25aacfa09919a7e509631b64ef672b4902a2010c -->
## 22.09.26 13:43 МСК - Лис.

Commit: [25aacfa](https://github.com/jordan1227/improved_to_he/commit/25aacfa09919a7e509631b64ef672b4902a2010c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/gulag_escape.ltx
- M gamedata/config/scripts/esc/esc_dogs_fox_attack.ltx

<!-- nlc-changelog-commit: 204241e5f18e421ed1ad01ffde6803d06ecc2337 -->
## 22.09.26 13:26 МСК - Серый/Лапоть в игнор.

Commit: [204241e](https://github.com/jordan1227/improved_to_he/commit/204241e5f18e421ed1ad01ffde6803d06ecc2337)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/sak_dalogs_esc_gar.xml
- M gamedata/config/misc/gulag_garbage.ltx
- M gamedata/scripts/dsh_enemies.script

<!-- nlc-changelog-commit: bf750c604c8869dcea595c105cb57b80562be37c -->
## 22.09.26 12:50 МСК - Сдвинул атаку бандитов.

Commit: [bf750c6](https://github.com/jordan1227/improved_to_he/commit/bf750c604c8869dcea595c105cb57b80562be37c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/sak/sak.script

<!-- nlc-changelog-commit: 273efe691a7800e3feb96aa89b3de17cd1d43479 -->
## 22.09.26 12:09 МСК - Manifest

Commit: [273efe6](https://github.com/jordan1227/improved_to_he/commit/273efe691a7800e3feb96aa89b3de17cd1d43479)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml

### Other repository files changed

- M manifest.txt

<!-- nlc-changelog-commit: e76d21e24d1594bac94ce180fd66566a907d1964 -->
## 22.09.26 09:45 МСК - Refresh Nazar stash map text

Commit: [e76d21e](https://github.com/jordan1227/improved_to_he/commit/e76d21e24d1594bac94ce180fd66566a907d1964)

### Description

Updated the Nazar stash map item description in the English and Russian string tables to reflect the current stash positions and revised map lore.

### Game files changed

- M gamedata/config/text/eng/string_table_items.xml
- M gamedata/config/text/rus/string_table_items.xml

<!-- nlc-changelog-commit: 2ec8a009759ebfb3f3a513cfdd04a10d7ccd746d -->
## 22.09.26 09:34 МСК - Fix paper reader close trigger

Commit: [2ec8a00](https://github.com/jordan1227/improved_to_he/commit/2ec8a009759ebfb3f3a513cfdd04a10d7ccd746d)

### Description

Prevent the paper reader from closing immediately when the quick-paper category is opened with the same mouse/fire binding. The close action is now armed only after the fire bind has been released, and the reader resets its state when opened. Also updates the patch version text and manifest metadata for this build.

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml
- M gamedata/scripts/ui/ui_paper_reader.script
- M gamedata/textures/liz/paper_tablet_document_diffuse_nazar.dds

### Other repository files changed

- M manifest.txt

<!-- nlc-changelog-commit: c8ab2b51bdee41596d8fed9d89cc1d50c1a18946 -->
## 22.09.26 09:06 МСК - Manifest

Commit: [c8ab2b5](https://github.com/jordan1227/improved_to_he/commit/c8ab2b51bdee41596d8fed9d89cc1d50c1a18946)

### Description

This patch updates several gameplay and UI systems: sprint FOV/hands handling, weapon zoom and aim tuning, inventory UI and paper-reader adjustments, plus related dialogue/text and manifest metadata refreshes. It also bumps the build version banner for the current release.

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml

### Other repository files changed

- M manifest.txt

<!-- nlc-changelog-commit: b6ccaa099f6110a65493ac7cb95db2b81bfcf453 -->
## 22.09.26 09:05 МСК - Add empty-hand sprint HUD & sprint-blocking

Commit: [b6ccaa0](https://github.com/jordan1227/improved_to_he/commit/b6ccaa099f6110a65493ac7cb95db2b81bfcf453)

### Description

Introduce a cosmetic empty-hand sprint HUD and support to avoid conflicts with other HUD animations.

Key changes:
- Added sivol/sivol_sprint_hands.script: plays bolt sprint hand cycle when sprinting empty-handed (test HUD).
- Added HUD section sivol_emptyhand_sprint_hud_test to anm_other.ltx.
- Implemented sprint block API in sivol.script (add/remove/reset blocks, request_sprint_stop, lifecycle reset, defensive release). Subscribed lifecycle signals.
- hud_animation_owner: relinquish cosmetic HUD before activating playback.
- allcore_hide.script: block sprint actions while consumable HUD plays and ensure removal on finish; safer activation and session checks.
- sivol_sprint_fov: consider scripted HUD blocking when suppressing sprint FOV.
- ogse_signals_addons_list: register sivol_sprint_hands addon.
- sivol_masks: prefixed several functions with sivol. to avoid globals.
- ui_cheat_inv_buttons: better full-backpack open cleanup, detach equipment icons, restore controls, watchdog and robust motion/open handling.

Purpose: enable a non-item empty-hand sprint animation while preventing input/HUD conflicts and improving robustness around inventory/backpack HUD transitions.

### Game files changed

- M gamedata/config/misc/items/anim_items/anm_other.ltx
- M gamedata/scripts/allcore_hide.script
- M gamedata/scripts/hud_animation_owner.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/sivol/sivol.script
- M gamedata/scripts/sivol/sivol_masks.script
- M gamedata/scripts/sivol/sivol_sprint_fov.script
- A gamedata/scripts/sivol/sivol_sprint_hands.script
- M gamedata/scripts/ui/ui_cheat_inv_buttons.script

<!-- nlc-changelog-commit: d08ccc8af322c6c62d3ee060b9528b246a2d872c -->
## 22.09.26 09:04 МСК - Диалог Волку о стрельбе. Прапору возможность дать незаряженый огнемёт по квесту за меньшую награду

Commit: [d08ccc8](https://github.com/jordan1227/improved_to_he/commit/d08ccc8af322c6c62d3ee060b9528b246a2d872c)

### Description

This commit expands the Wolf dialogue flow with a new weapon-handling / Zone advice branch and preserves the PDA hint route as a separate path. It also adds the missing info portion flag for the new branch and updates the Gar flame quest to handle partially-used flamethrowers with a different reward path, plus corresponding English/Russian text strings.

### Game files changed

- M gamedata/config/gameplay/dialogs_escape.xml
- M gamedata/config/gameplay/info_portions_nlc30.xml
- M gamedata/config/gameplay/sak_dalogs_esc_gar.xml
- M gamedata/config/text/eng/sak_strings_six.xml
- M gamedata/config/text/eng/script_strings.xml
- M gamedata/config/text/rus/sak_strings_six.xml
- M gamedata/config/text/rus/script_strings.xml

<!-- nlc-changelog-commit: 80b7ba5418482e5eb486ede1dff0574ca3dd3adc -->
## 22.09.26 03:57 МСК - Unify camera FOV ownership and empty-hands zoom

Commit: [80b7ba5](https://github.com/jordan1227/improved_to_he/commit/80b7ba5418482e5eb486ede1dff0574ca3dd3adc)

### Description

Consolidate temporary camera-FOV effects into sivol_sprint_fov and add a small API for other owners to release/restore the actor's base FOV. Move the empty-hands zoom into sprint_fov (shared owner), remove the duplicated implementation from sivol_weapon_zoom, and add release_camera_fov/release_hud_fov hooks. Ensure lifecycle events (actor death, load, level change, save) restore FOV. Call release_camera_fov from monster_parts before capturing FOV and release_hud_fov from ui_paper_reader. Also wire actor-death handling in bind_stalker.

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml
- M gamedata/scripts/binders/bind_stalker.script
- M gamedata/scripts/kotovod/monster_parts.script
- M gamedata/scripts/sivol/sivol_sprint_fov.script
- M gamedata/scripts/sivol/sivol_weapon_zoom.script
- M gamedata/scripts/ui/ui_paper_reader.script

### Other repository files changed

- M manifest.txt

<!-- nlc-changelog-commit: 1c0a8822ded20b194fa9f0c3078b1c5e1545d3fd -->
## 22.09.26 03:25 МСК - Add manifest tooling and menu patch stamp

Commit: [1c0a882](https://github.com/jordan1227/improved_to_he/commit/1c0a8822ded20b194fa9f0c3078b1c5e1545d3fd)

### Description

This update adds a Windows helper plus a stronger Python manifest generator to apply and verify the build manifest, with support for custom Moscow timestamps and optional menu-version updates. It also refreshes the main-menu patch text in both language files and updates the generated manifest to include the current content set.

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml
- M gamedata/textures/ui/ui_icon_equipment_14.dds

### Other repository files changed

- A make_manifest.bat
- M make_manifest.py
- M manifest.txt

<!-- nlc-changelog-commit: 66693f1c3ab467a440a38f108a456b8e9d40abad -->
## 22.09.26 02:40 МСК - Альтернативное разрешение Каналов Сбыта. Приближение при пустых руках

Commit: [66693f1](https://github.com/jordan1227/improved_to_he/commit/66693f1c3ab467a440a38f108a456b8e9d40abad)

### Description

Adds the Nazar stash-map quest branch with new dialog states, PDA gating, cleanup, and item spawns. This also introduces a reusable paper/document reader UI, custom HUD/model support for the stash map, new item strings and textures, and related quest-state tracking to keep the route and its spawns consistent.

### Game files changed

- M gamedata/config/gameplay/info_sak_way.xml
- M gamedata/config/gameplay/kill_stalker_dlg.xml
- M gamedata/config/misc/items/anim_items.ltx
- M gamedata/config/misc/sak_quest_items.ltx
- M gamedata/config/text/eng/he_watch.xml
- M gamedata/config/text/eng/string_table_items.xml
- M gamedata/config/text/rus/he_watch.xml
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/config/text/rus/string_table_items.xml
- A gamedata/config/ui/ui_paper_reader.xml
- A gamedata/meshes/anomaly_weapons/wpn_3ddocument/wpn_document_hud_nazar.ogf
- M gamedata/scripts/alpet/nlc_container.script
- M gamedata/scripts/binders/bind_stalker.script
- M gamedata/scripts/he_pda_access.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/sak/sak_dialog.script
- M gamedata/scripts/sak/sak_spawn.script
- M gamedata/scripts/sivol/sivol_paper.script
- M gamedata/scripts/sivol/sivol_weapon_zoom.script
- M gamedata/scripts/ui/ui_cheat_razgruzka.script
- M gamedata/scripts/ui/ui_cheat_razgruzka_new.script
- M gamedata/scripts/ui/ui_he_magic_box.script
- A gamedata/scripts/ui/ui_paper_reader.script
- M gamedata/scripts/vergas/inv_buttons_use.script
- M gamedata/scripts/vergas/vergas_lib.script
- A gamedata/textures/liz/paper_tablet_document_diffuse_nazar.dds

<!-- nlc-changelog-commit: 4f77a096dd2b2aa9eecb9562884db2a1044ebb9c -->
## 21.09.26 21:58 МСК - убрал мурку с бартера сидора

Commit: [4f77a09](https://github.com/jordan1227/improved_to_he/commit/4f77a096dd2b2aa9eecb9562884db2a1044ebb9c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/shop_sidor/sidor_barter.ltx

<!-- nlc-changelog-commit: 297c2ae6f42b39a883bf261231585efc61337cf4 -->
## 21.09.26 21:49 МСК - Фикс друга Петрухи

Commit: [297c2ae](https://github.com/jordan1227/improved_to_he/commit/297c2ae6f42b39a883bf261231585efc61337cf4)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/mike.script
- M gamedata/scripts/sak/sak.script

<!-- nlc-changelog-commit: a3d65445de4802c3db6e49c59c8c0bf4c9633c74 -->
## 21.09.26 21:18 МСК - Фикс секции

Commit: [a3d6544](https://github.com/jordan1227/improved_to_he/commit/a3d65445de4802c3db6e49c59c8c0bf4c9633c74)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/dialogs_artmod.xml
- M gamedata/scripts/kotovod.script
- M gamedata/scripts/kotovod/spawner.script
- M gamedata/scripts/vergas/vergas_krujka.script

<!-- nlc-changelog-commit: 59e8a1eca5bb6e97eb073019b36e0affcad0505a -->
## 21.09.26 20:21 МСК - фикс "Каналы сбыта"

Commit: [59e8a1e](https://github.com/jordan1227/improved_to_he/commit/59e8a1eca5bb6e97eb073019b36e0affcad0505a)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/kotovod.script

<!-- nlc-changelog-commit: 1f9f018453ce2c8fcdffbad56f2aa5c415780776 -->
## 21.09.26 19:06 МСК - Фикс Стаса.

Commit: [1f9f018](https://github.com/jordan1227/improved_to_he/commit/1f9f018453ce2c8fcdffbad56f2aa5c415780776)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/sak_dalogs_esc_gar.xml

<!-- nlc-changelog-commit: d49efe962108939cef987c57c986f3612a6fb4ad -->
## 21.09.26 18:06 МСК - Фикс снятия репы

Commit: [d49efe9](https://github.com/jordan1227/improved_to_he/commit/d49efe962108939cef987c57c986f3612a6fb4ad)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_guest_kill.script
- M gamedata/scripts/olr/olr_init.script
- M gamedata/scripts/olr/olr_relations.script

<!-- nlc-changelog-commit: a8f09f578fa4ca89a468c53c869009346ba76d2b -->
## 21.09.26 17:08 МСК - Добавил ПНВ в продажу торговцам

Commit: [a8f09f5](https://github.com/jordan1227/improved_to_he/commit/a8f09f578fa4ca89a468c53c869009346ba76d2b)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/shop_barman/barman_buy.ltx
- M gamedata/config/misc/shop_barman/barman_supl.ltx
- M gamedata/config/misc/shop_barman/barman_trade.ltx
- M gamedata/config/misc/shop_petrenko/trade_petrenko.ltx
- M gamedata/config/misc/shop_sakharov/trade_sakharov.ltx
- M gamedata/config/misc/shop_sidor/trade_sidor.ltx
- M gamedata/config/misc/trade_npc/trade_skriaga.ltx

<!-- nlc-changelog-commit: 1875132382d58e03f8d33c8d31fadda559819b8d -->
## 21.09.26 15:42 МСК - Пофиксил двойную иконку жажды, заменил на сонливость

Commit: [1875132](https://github.com/jordan1227/improved_to_he/commit/1875132382d58e03f8d33c8d31fadda559819b8d)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/ui/ui_hud.xml

<!-- nlc-changelog-commit: 7bf952f53986fb626ebd941c3dd48cda7faf370b -->
## 21.09.26 13:04 МСК - Починил руки на куртке

Commit: [7bf952f](https://github.com/jordan1227/improved_to_he/commit/7bf952f53986fb626ebd941c3dd48cda7faf370b)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/creatures/actor.ltx
- M gamedata/config/misc/all_outfits_nlc.ltx
- A gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_novice_white.ogf
- M gamedata/scripts/kotovod/he_sleeves.script
- M gamedata/scripts/watch_gate.script

### Other repository files changed

- M Опционально/оружие/WEAPONS_NLC30/gamedata/scripts/kotovod/he_sleeves.script
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/scripts/kotovod/he_sleeves.script

<!-- nlc-changelog-commit: 3507d4f926a9bbe0d5daabb940cb9fc9cbb1559f -->
## 21.09.26 12:23 МСК - Вернул старые иконки

Commit: [3507d4f](https://github.com/jordan1227/improved_to_he/commit/3507d4f926a9bbe0d5daabb940cb9fc9cbb1559f)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/textures/ui/ui_icon_equipment_1.dds
- M gamedata/textures/ui/ui_icon_equipment_2.dds
- A gamedata/textures/ui/ui_icon_equipment_7.dds

<!-- nlc-changelog-commit: 8ee6cc6523b23e85ff3481ef16fec5c1ea6b78cd -->
## 21.09.26 12:04 МСК - Группа торговцев: Уважаемый -> Торговец

Commit: [8ee6cc6](https://github.com/jordan1227/improved_to_he/commit/8ee6cc6523b23e85ff3481ef16fec5c1ea6b78cd)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/text/eng/string_table_general.xml
- M gamedata/config/text/rus/string_table_general.xml

<!-- nlc-changelog-commit: b941781f2cbe400f972ee55afe76748e1d238e76 -->
## 20.09.26 23:55 МСК - Фикс пропажи руки

Commit: [b941781](https://github.com/jordan1227/improved_to_he/commit/b941781f2cbe400f972ee55afe76748e1d238e76)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/watch_gate.script

### Other repository files changed

- M fl_hook/dinput8.dll
- M fl_hook/flhook.cpp

<!-- nlc-changelog-commit: 728340cbaa6c79d5f9655c6a886004bf6f60e5b7 -->
## 20.09.26 22:01 МСК - Кружка Вергаса

Commit: [728340c](https://github.com/jordan1227/improved_to_he/commit/728340cbaa6c79d5f9655c6a886004bf6f60e5b7)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/vergas/vergas_krujka.script

<!-- nlc-changelog-commit: 6b0532d7404c93e89efd91da501b02eb61d84a6f -->
## 20.09.26 19:44 МСК - Версия билда

Commit: [6b0532d](https://github.com/jordan1227/improved_to_he/commit/6b0532d7404c93e89efd91da501b02eb61d84a6f)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_other.xml

<!-- nlc-changelog-commit: e0e30d603aa04966ebc7d6460aa06a3e37af35d1 -->
## 20.09.26 19:18 МСК - Корректировка алайфа

Commit: [e0e30d6](https://github.com/jordan1227/improved_to_he/commit/e0e30d603aa04966ebc7d6460aa06a3e37af35d1)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/olr/olr_monster_squads.script
- M gamedata/scripts/olr/olr_offline_task.script
- M gamedata/scripts/olr/olr_offline_walk.script
- M gamedata/scripts/olr/olr_online_depart.script
- M gamedata/scripts/olr/olr_raids.script
- M gamedata/scripts/olr/olr_smart_index.script
- M gamedata/scripts/olr/olr_squads.script

<!-- nlc-changelog-commit: 8ca36653b439d26bca168315c41f37dc2f0163aa -->
## 20.09.26 15:35 МСК - Диалоги

Commit: [8ca3665](https://github.com/jordan1227/improved_to_he/commit/8ca36653b439d26bca168315c41f37dc2f0163aa)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/text/eng/sak_strings_trade.xml
- M gamedata/config/text/rus/amk_flame_dialogs.xml
- M gamedata/config/text/rus/doc_dialogs.xml
- M gamedata/config/text/rus/mike_strings.xml
- M gamedata/config/text/rus/sak_strings.xml
- M gamedata/config/text/rus/sak_strings_six.xml
- M gamedata/config/text/rus/sak_strings_six2.xml
- M gamedata/config/text/rus/sak_strings_trade.xml
- M gamedata/config/text/rus/stable_dialogs_agroprom.xml
- M gamedata/config/text/rus/stable_dialogs_amk.xml
- M gamedata/config/text/rus/stable_dialogs_bar.xml
- M gamedata/config/text/rus/stable_dialogs_darkvalley.xml
- M gamedata/config/text/rus/stable_dialogs_escape.xml
- M gamedata/config/text/rus/stable_dialogs_military.xml
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/config/text/rus/stable_task_manager.xml
- M gamedata/config/text/rus/story_dialogs.xml
- M gamedata/config/text/rus/string_table_vergas.xml

<!-- nlc-changelog-commit: bf08ac29888ffb6344000c91b2f6d6568771f39a -->
## 20.09.26 15:14 МСК - Удаляем дубликаты спавна мобам

Commit: [bf08ac2](https://github.com/jordan1227/improved_to_he/commit/bf08ac29888ffb6344000c91b2f6d6568771f39a)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/binders/bind_monster.script
- M gamedata/scripts/sak/sak_dialog.script

<!-- nlc-changelog-commit: 0a8cf8058689881a43b52fbc72fe7c053cfb0937 -->
## 20.09.26 15:06 МСК - Набор в слотовую флягу

Commit: [0a8cf80](https://github.com/jordan1227/improved_to_he/commit/0a8cf8058689881a43b52fbc72fe7c053cfb0937)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_cheat_flask.script
- M gamedata/scripts/vergas/vergas_masks.script

<!-- nlc-changelog-commit: 0c95c45bbca7f9097678be59f35ad6ff359413e7 -->
## 20.09.26 15:00 МСК - Фиксы дупликатов

Commit: [0c95c45](https://github.com/jordan1227/improved_to_he/commit/0c95c45bbca7f9097678be59f35ad6ff359413e7)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/creatures/m_dog.ltx
- M gamedata/config/misc/items.ltx
- M gamedata/config/text/eng/string_table_enc_weapons.xml
- M gamedata/config/text/eng/ui_st_mm.xml
- M gamedata/config/text/rus/string_table_enc_weapons.xml
- M gamedata/config/text/rus/ui_st_mm.xml
- M gamedata/config/text/rus/ui_st_other.xml
- M gamedata/config/ui/message_box.xml
- M gamedata/config/ui/ui_mm_opt.xml
- M gamedata/config/weapons/w_mp133_short.ltx
- M gamedata/config/weapons/w_mp153.ltx
- M gamedata/config/weapons/w_mp153_short.ltx
- M gamedata/config/weapons/w_oreh_shotgun.ltx
- M gamedata/config/weapons/w_remington870.ltx
- M gamedata/config/weapons/w_saiga12_mod.ltx
- M gamedata/config/weapons/w_saiga12c.ltx
- M gamedata/config/weapons/w_spas12.ltx
- M gamedata/config/weapons/w_toz34.ltx
- M gamedata/config/weapons/w_toz34_hunter.ltx
- M gamedata/config/weapons/w_toz66.ltx
- M gamedata/config/weapons/w_toz66_short.ltx
- M gamedata/config/weapons/w_usas12.ltx
- M gamedata/config/weapons/w_usas12_mod.ltx
- M gamedata/config/weapons/weapons.ltx
- M gamedata/scripts/dsh_wpn.script
- M gamedata/scripts/he_hipfire.script
- M gamedata/scripts/kotovod/game_options.script
- M gamedata/scripts/kotovod/spawner.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/ogsr_shaders_control.script
- M gamedata/scripts/sivol/sivol_cry_sprint.script
- M gamedata/scripts/sivol/sivol_fuzz_recoil_profiles.script
- M gamedata/scripts/sivol/sivol_shell_sounds.script
- M gamedata/scripts/sivol/sivol_weapon.script
- M gamedata/scripts/sivol/sivol_weapon_inertia_configs.script
- M gamedata/scripts/sivol/sivol_weapon_zoom.script
- M gamedata/scripts/ui/ui_inv_descr.script
- M gamedata/scripts/ui/ui_main_menu.script
- M gamedata/scripts/ui/ui_mm_opt_gameplay.script
- M gamedata/scripts/ui/ui_mm_opt_main.script

<!-- nlc-changelog-commit: ea7bec5b3a311ebd3321d9358018dcc0f7c845dd -->
## 20.09.26 14:24 МСК - Фиксы багов

Commit: [ea7bec5](https://github.com/jordan1227/improved_to_he/commit/ea7bec5b3a311ebd3321d9358018dcc0f7c845dd)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/creatures/stalkers.ltx
- M gamedata/config/gameplay/dialogs_escape.xml
- M gamedata/scripts/sak/sak_dialog.script

<!-- nlc-changelog-commit: b48a9b4f5e37e461d67ceee79ece509f718802cb -->
## 20.09.26 04:14 МСК - Enable corpses collision in external.ltx

Commit: [b48a9b4](https://github.com/jordan1227/improved_to_he/commit/b48a9b4f5e37e461d67ceee79ece509f718802cb)

### Description

Change corpses_collision from false to true in gamedata/config/external.ltx. This enables physical collisions for corpses so they interact with the world (can block movement or be pushed), which may affect gameplay and mod compatibility.

### Game files changed

- M gamedata/config/external.ltx

<!-- nlc-changelog-commit: 43fa3b1d33e70b19cb3960cb5b60c12de7b12e1e -->
## 20.09.26 03:48 МСК - Add AKS-47/MP-133, UI toggle, shotgun PDM tweaks

Commit: [43fa3b1](https://github.com/jordan1227/improved_to_he/commit/43fa3b1d33e70b19cb3960cb5b60c12de7b12e1e)

### Description

Adds two HE-ported weapons (wpn_aks47_he, wpn_mp133_full) and their HUD/mesh/texture/sound assets, plus many texture/hand mesh updates. Adds localization entries and includes the new configs in weapons.ltx. Introduces a main-menu option "Weapon firing penalties" (default ON) wired into game options and UI; dsh_wpn and he_hipfire respect it. Reduces PDM_disp movement/accel penalties for many shotguns (near-zero values) to change hipfire behavior. Updates recoil/inertia/zoom/shell-type registrations and spawner lists, removes he_pda_notes addon, adds a cosmetic watch battery icon toggle, and bumps mode/version string.
Add difficulty descriptions.

### Game files changed

- M gamedata/config/creatures/m_dog.ltx
- M gamedata/config/misc/items.ltx
- M gamedata/config/text/eng/string_table_enc_weapons.xml
- M gamedata/config/text/eng/ui_st_mm.xml
- M gamedata/config/text/rus/string_table_enc_weapons.xml
- M gamedata/config/text/rus/ui_st_mm.xml
- M gamedata/config/text/rus/ui_st_other.xml
- M gamedata/config/ui/message_box.xml
- M gamedata/config/ui/ui_mm_opt.xml
- A gamedata/config/weapons/w_aks47_he.ltx
- A gamedata/config/weapons/w_mp133_full.ltx
- M gamedata/config/weapons/w_mp133_short.ltx
- M gamedata/config/weapons/w_mp153.ltx
- M gamedata/config/weapons/w_mp153_short.ltx
- M gamedata/config/weapons/w_oreh_shotgun.ltx
- M gamedata/config/weapons/w_remington870.ltx
- M gamedata/config/weapons/w_saiga12_mod.ltx
- M gamedata/config/weapons/w_saiga12c.ltx
- M gamedata/config/weapons/w_spas12.ltx
- M gamedata/config/weapons/w_toz34.ltx
- M gamedata/config/weapons/w_toz34_hunter.ltx
- M gamedata/config/weapons/w_toz66.ltx
- M gamedata/config/weapons/w_toz66_short.ltx
- M gamedata/config/weapons/w_usas12.ltx
- M gamedata/config/weapons/w_usas12_mod.ltx
- M gamedata/config/weapons/weapons.ltx
- A gamedata/meshes/anomaly_weapons/hud_animation/wpn_akm_sed_ak12_gun_motions.omf
- A gamedata/meshes/anomaly_weapons/hud_animation/wpn_mp133_hud_animation.omf
- A gamedata/meshes/anomaly_weapons/hud_hands_animation/he_awp_mp133_hands.omf
- A gamedata/meshes/anomaly_weapons/hud_hands_animation/wpn_ak12_hands_motions.omf
- A gamedata/meshes/anomaly_weapons/hud_hands_animation/wpn_mp133_hud_hands_animation.omf
- A gamedata/meshes/anomaly_weapons/wpn_ak/wpn_aks_hud.ogf
- A gamedata/meshes/anomaly_weapons/wpn_mp133/wpn_mp133_hud.ogf
- A gamedata/meshes/dynamics/weapons/he_mp133_awp/wpn_mp133.ogf
- A gamedata/meshes/dynamics/weapons/he_mp133_awp/wpn_mp133_hud.ogf
- A gamedata/meshes/dynamics/weapons/he_mp133_awp/wpn_mp133_hud_animation.omf
- A gamedata/meshes/dynamics/weapons/he_mp133_awp/wpn_mp133_lod.ogf
- A gamedata/meshes/dynamics/weapons/wpn_ak/wpn_aks.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand__brown.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_bandit_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_bandit_novice.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_bandit_trench_gloves.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_cs1.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_cs3a.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_ddd.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_dolg_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_duty_psz9d.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_duty_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_ecologist_ssp99.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_ecologist_ssp99m.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_guard.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_guardian.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_freedom_wind.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_novice.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_loner_sunrise.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_radiation_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_merc_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_military_berill.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_military_novice.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_military_skat9.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_exo.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_radiation_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_seva.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_monolith_suit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_no_outfit.ogf
- M gamedata/meshes/dynamics/weapons/wpn_hand/wpn_hand_skat9m.ogf
- A gamedata/meshes/dynamics/weapons/wpn_mp133/wpn_mp133.ogf
- M gamedata/scripts/dsh_wpn.script
- M gamedata/scripts/he_hipfire.script
- M gamedata/scripts/kotovod/game_options.script
- M gamedata/scripts/kotovod/spawner.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/ogsr_shaders_control.script
- M gamedata/scripts/sivol/sivol_cry_sprint.script
- M gamedata/scripts/sivol/sivol_fuzz_recoil_profiles.script
- M gamedata/scripts/sivol/sivol_shell_sounds.script
- M gamedata/scripts/sivol/sivol_weapon.script
- M gamedata/scripts/sivol/sivol_weapon_inertia_configs.script
- M gamedata/scripts/sivol/sivol_weapon_zoom.script
- M gamedata/scripts/ui/ui_inv_descr.script
- M gamedata/scripts/ui/ui_main_menu.script
- M gamedata/scripts/ui/ui_mm_opt_gameplay.script
- M gamedata/scripts/ui/ui_mm_opt_main.script
- A gamedata/sounds/weapons/ak12/ak12_hide.ogg
- A gamedata/sounds/weapons/ak12/ak12_mis.ogg
- A gamedata/sounds/weapons/ak12/ak12_r.ogg
- A gamedata/sounds/weapons/ak12/ak12_re.ogg
- A gamedata/sounds/weapons/ak12/ak12_show.ogg
- A gamedata/sounds/weapons/he_mp133_awp/mp133_misfire.ogg
- A gamedata/sounds/weapons/he_mp133_awp/mp133_reload.ogg
- A gamedata/sounds/weapons/he_mp133_awp/mp133_reload_empty.ogg
- A gamedata/sounds/weapons/he_mp133_awp/mp133_reload_end.ogg
- A gamedata/sounds/weapons/he_mp133_awp/mp133_reload_start.ogg
- A gamedata/sounds/weapons/mossberg590a1/m590_close_ammo4.ogg
- A gamedata/sounds/weapons/mossberg590a1/m590_inspect.ogg
- A gamedata/sounds/weapons/mossberg590a1/m590_load_ammo3.ogg
- A gamedata/sounds/weapons/mp133/mp133_load_empty.ogg
- A gamedata/sounds/weapons/mp133/mp133_open_empty.ogg
- A gamedata/sounds/weapons/mp133/mp133_unjam.ogg
- A gamedata/textures/he_awp/mp133/wpn/mp133/barrel_mr133_bump.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/barrel_mr133_diff.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/barrel_mr133_diff.thm
- A gamedata/textures/he_awp/mp133/wpn/mp133/handguard_mr133_wood_bump.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/handguard_mr133_wood_diff.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/handguard_mr133_wood_diff.thm
- A gamedata/textures/he_awp/mp133/wpn/mp133/mag_lock.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/mag_lock.thm
- A gamedata/textures/he_awp/mp133/wpn/mp133/mag_lock_bump.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/mag_mr133_diff.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/mag_mr133_diff.thm
- A gamedata/textures/he_awp/mp133/wpn/mp133/mag_mr133_diff_bump.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/mr133_bump.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/mr133_diff.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/mr133_diff.thm
- A gamedata/textures/he_awp/mp133/wpn/mp133/stock_mr133_wood_bump.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/stock_mr133_wood_diff.dds
- A gamedata/textures/he_awp/mp133/wpn/mp133/stock_mr133_wood_diff.thm
- A gamedata/textures/he_awp/mp133/wpn/wpn_bullet1.dds
- A gamedata/textures/he_awp/mp133/wpn/wpn_bullet1.thm
- A gamedata/textures/he_awp/mp133/wpn/wpn_bullet1_blue.dds
- A gamedata/textures/he_awp/mp133/wpn/wpn_bullet1_blue.thm
- A gamedata/textures/he_awp/mp133/wpn/wpn_bullet1_bump.dds
- A gamedata/textures/ui/ui_icon_equipment_13.dds
- A gamedata/textures/ui/ui_icon_equipment_14.dds
- A gamedata/textures/wpn/cartridges/12ga_metal.dds
- A gamedata/textures/wpn/cartridges/12ga_metal.thm
- A gamedata/textures/wpn/cartridges/12ga_metal_bump#.dds
- A gamedata/textures/wpn/cartridges/12ga_metal_bump.dds
- A gamedata/textures/wpn/cartridges/12ga_metal_bump.thm
- A gamedata/textures/wpn/cartridges/12ga_plastic.dds
- A gamedata/textures/wpn/cartridges/12ga_plastic.thm
- A gamedata/textures/wpn/cartridges/12ga_plastic_bump#.dds
- A gamedata/textures/wpn/cartridges/12ga_plastic_bump.dds
- A gamedata/textures/wpn/cartridges/12ga_plastic_bump.thm
- A gamedata/textures/wpn/eft_aks/wpn_eft_ak74_gl_gp34_bump#.dds
- A gamedata/textures/wpn/eft_aks/wpn_eft_ak74_gl_gp34_bump.dds
- A gamedata/textures/wpn/eft_aks/wpn_eft_ak74_gl_gp34_bump.thm
- A gamedata/textures/wpn/eft_aks/wpn_eft_ak74_gl_gp34_diff.dds
- A gamedata/textures/wpn/eft_aks/wpn_eft_ak74_gl_gp34_diff.thm
- A gamedata/textures/wpn/wpn_eft_ak74_gl_gp34_bump#.dds
- A gamedata/textures/wpn/wpn_eft_ak74_gl_gp34_bump.dds
- A gamedata/textures/wpn/wpn_eft_ak74_gl_gp34_bump.thm
- A gamedata/textures/wpn/wpn_eft_ak74_gl_gp34_diff.dds
- A gamedata/textures/wpn/wpn_eft_ak74_gl_gp34_diff.thm
- A gamedata/textures/wpn/wpn_mp133/wpn_mp133.dds
- A gamedata/textures/wpn/wpn_mp133/wpn_mp133.thm
- A gamedata/textures/wpn/wpn_mp133/wpn_mp133_bump#.dds
- A gamedata/textures/wpn/wpn_mp133/wpn_mp133_bump.dds
- A gamedata/textures/wpn/wpn_mp133/wpn_mp133_bump.thm
- A gamedata/textures/wpn_cw/extra/launchers/gp25/gp25.dds
- A gamedata/textures/wpn_cw/extra/launchers/gp25/gp25.thm
- A gamedata/textures/wpn_cw/extra/launchers/gp25/gp25_bump.dds
- A gamedata/textures/wpn_cw/extra/launchers/gp25/gp25_bump.thm
- A gamedata/textures/wpn_cw/extra/launchers/gp25/grenade.dds
- A gamedata/textures/wpn_cw/extra/launchers/gp25/grenade.thm
- A gamedata/textures/wpn_cw/extra/launchers/gp25/grenade_bump.dds
- A gamedata/textures/wpn_cw/extra/launchers/gp25/grenade_bump.thm
- A gamedata/textures/wpn_cw/extra/silencers/silencer_pbs.dds
- A gamedata/textures/wpn_cw/extra/silencers/silencer_pbs.thm
- A gamedata/textures/wpn_cw/extra/silencers/silencer_pbs_bump.dds
- A gamedata/textures/wpn_cw/extra/silencers/silencer_pbs_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_bullet_rgb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_bullet_rgb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_bullet_rgb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_bullet_rgb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_case_rgb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_case_rgb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_case_rgb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/762_case_rgb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplate.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplate.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplate_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplate_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplateb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplateb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplateb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_buttplateb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stock.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stock.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stock_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stock_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stockb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stockb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stockb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKMS_stockb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_barrel_rgb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_barrel_rgb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_barrel_rgb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_barrel_rgb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_handguard_b.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_handguard_b.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_handguard_b_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_handguard_b_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_mag_rgb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_mag_rgb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_mag_rgb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_mag_rgb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_sights._rgb.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_sights._rgb.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_sights._rgb_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/AKM_sights._rgb_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/ak47_receiver.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/ak47_receiver.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/ak47_receiver_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/ak47_receiver_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_grip.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_grip.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_grip_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_grip_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_muzzle.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_muzzle.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_muzzle_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_muzzle_bump.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_sights.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_sights.thm
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_sights_bump.dds
- A gamedata/textures/wpn_cw/wpn_akm_cw/Type56/type56_sights_bump.thm

<!-- nlc-changelog-commit: 0c4c15f6a99fee08de576233571921709bf3179c -->
## 19.09.26 23:59 МСК - Хранить файлы репозитория байт в байт (CRLF), без конверсии переводов строк

Commit: [0c4c15f](https://github.com/jordan1227/improved_to_he/commit/0c4c15f6a99fee08de576233571921709bf3179c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/_prefetch.ltx
- M gamedata/config/alife.ltx
- M gamedata/config/balance_default.ltx
- M gamedata/config/balance_light.ltx
- M gamedata/config/cheat_list_info.ltx
- M gamedata/config/creatures/actor.ltx
- M gamedata/config/creatures/amk_spawn_sections.ltx
- M gamedata/config/creatures/assignseats.ltx
- M gamedata/config/creatures/damages.ltx
- M gamedata/config/creatures/game_relations.ltx
- M gamedata/config/creatures/helicopter.ltx
- M gamedata/config/creatures/immunities.ltx
- M gamedata/config/creatures/m_army.ltx
- M gamedata/config/creatures/m_bandit.ltx
- M gamedata/config/creatures/m_bloodsucker.ltx
- M gamedata/config/creatures/m_boar.ltx
- M gamedata/config/creatures/m_burer.ltx
- M gamedata/config/creatures/m_cat.ltx
- M gamedata/config/creatures/m_chimera.ltx
- M gamedata/config/creatures/m_controller.ltx
- M gamedata/config/creatures/m_crow.ltx
- M gamedata/config/creatures/m_dog.ltx
- M gamedata/config/creatures/m_flesh.ltx
- M gamedata/config/creatures/m_fracture.ltx
- M gamedata/config/creatures/m_giant.ltx
- M gamedata/config/creatures/m_karlik.ltx
- M gamedata/config/creatures/m_person.ltx
- M gamedata/config/creatures/m_phantom.ltx
- M gamedata/config/creatures/m_poltergeist.ltx
- M gamedata/config/creatures/m_pseudodog.ltx
- M gamedata/config/creatures/m_pseudodogfrend.ltx
- M gamedata/config/creatures/m_rat.ltx
- M gamedata/config/creatures/m_snork.ltx
- M gamedata/config/creatures/m_stalker.ltx
- M gamedata/config/creatures/m_stalker_antisniper.ltx
- M gamedata/config/creatures/m_stalker_monolith.ltx
- M gamedata/config/creatures/m_stalker_sniper.ltx
- M gamedata/config/creatures/m_stalker_zombied.ltx
- M gamedata/config/creatures/m_tushkano.ltx
- M gamedata/config/creatures/m_zombie.ltx
- M gamedata/config/creatures/monsters.ltx
- M gamedata/config/creatures/olr_spawn_sections.ltx
- M gamedata/config/creatures/spawn_sections.ltx
- M gamedata/config/creatures/stalkers.ltx
- M gamedata/config/creatures/vechicles.ltx
- M gamedata/config/default_controls.ltx
- M gamedata/config/defines.ltx
- M gamedata/config/environment/ambients.ltx
- M gamedata/config/environment/dynamic_weather_graphs.ltx
- M gamedata/config/environment/effects.ltx
- M gamedata/config/environment/environment.ltx
- M gamedata/config/environment/fog/agroprom_underground_area_01.ltx
- M gamedata/config/environment/fog/ts3.ltx
- M gamedata/config/environment/sound_channels.ltx
- M gamedata/config/environment/suns.ltx
- M gamedata/config/environment/thunderbolt_collections.ltx
- M gamedata/config/environment/thunderbolts.ltx
- M gamedata/config/environment/weather_effects/full_fx_surge_day.ltx
- M gamedata/config/environment/weather_effects/fx_surge_day_1.ltx
- M gamedata/config/environment/weather_effects/fx_surge_day_2.ltx
- M gamedata/config/environment/weather_effects/fx_surge_day_3.ltx
- M gamedata/config/environment/weather_effects/fx_surge_day_3_stancia.ltx
- M gamedata/config/environment/weather_effects/fx_surge_day_stancia_demo_1.ltx
- M gamedata/config/environment/weather_effects/fx_surge_day_stancia_demo_2.ltx
- M gamedata/config/environment/weather_effects/marsh_fx_surge_day_1.ltx
- M gamedata/config/environment/weather_effects/surge_day.ltx
- M gamedata/config/environment/weathers/default_katakomb.ltx
- M gamedata/config/environment/weathers/default_np_alpha.ltx
- M gamedata/config/environment/weathers/default_np_alpha2.ltx
- M gamedata/config/environment/weathers/default_np_alpha3.ltx
- M gamedata/config/environment/weathers/default_np_clear.ltx
- M gamedata/config/environment/weathers/default_np_clear2.ltx
- M gamedata/config/environment/weathers/default_np_clear3.ltx
- M gamedata/config/environment/weathers/default_np_clear4.ltx
- M gamedata/config/environment/weathers/default_np_cloudy.ltx
- M gamedata/config/environment/weathers/default_np_cloudy2.ltx
- M gamedata/config/environment/weathers/default_np_cloudy2_redmoon.ltx
- M gamedata/config/environment/weathers/default_np_cloudy3.ltx
- M gamedata/config/environment/weathers/default_np_cloudy4.ltx
- M gamedata/config/environment/weathers/default_np_cloudy5.ltx
- M gamedata/config/environment/weathers/default_np_foggy.ltx
- M gamedata/config/environment/weathers/default_np_foggy_dark.ltx
- M gamedata/config/environment/weathers/default_np_foggy_nosun.ltx
- M gamedata/config/environment/weathers/default_np_rain.ltx
- M gamedata/config/environment/weathers/default_np_thunder.ltx
- M gamedata/config/environment/weathers/default_pre_blowout.ltx
- M gamedata/config/environment/weathers/default_swtc_dark.ltx
- M gamedata/config/environment/weathers/default_swtc_etalon.ltx
- M gamedata/config/environment/weathers/indoor.ltx
- M gamedata/config/evaluation.ltx
- M gamedata/config/external.ltx
- M gamedata/config/extracontent.ltx
- M gamedata/config/fonts.ltx
- M gamedata/config/game.ltx
- M gamedata/config/game_graphs.ltx
- M gamedata/config/game_levels.ltx
- M gamedata/config/game_levels_music.ltx
- M gamedata/config/game_maps_mp.ltx
- M gamedata/config/game_maps_single.ltx
- M gamedata/config/game_spawn_story_ids.ltx
- M gamedata/config/game_story_ids.ltx
- M gamedata/config/gameplay/arc_diary_articles.xml
- M gamedata/config/gameplay/character_desc_agroprom.xml
- M gamedata/config/gameplay/character_desc_bar.xml
- M gamedata/config/gameplay/character_desc_darkvalley.xml
- M gamedata/config/gameplay/character_desc_escape.xml
- M gamedata/config/gameplay/character_desc_garbage.xml
- M gamedata/config/gameplay/character_desc_limansk.xml
- M gamedata/config/gameplay/character_desc_military.xml
- M gamedata/config/gameplay/character_desc_nlc30_spawn.xml
- M gamedata/config/gameplay/character_desc_pripyat.xml
- M gamedata/config/gameplay/character_desc_radar.xml
- M gamedata/config/gameplay/character_desc_sak.xml
- M gamedata/config/gameplay/character_desc_simulation.xml
- M gamedata/config/gameplay/character_desc_yantar.xml
- M gamedata/config/gameplay/character_dialogs.xml
- M gamedata/config/gameplay/character_dialogs_2.xml
- M gamedata/config/gameplay/character_dialogs_3.xml
- M gamedata/config/gameplay/character_dialogs_4.xml
- M gamedata/config/gameplay/character_drugs.xml
- M gamedata/config/gameplay/character_food.xml
- M gamedata/config/gameplay/character_items.xml
- M gamedata/config/gameplay/dialogs.xml
- M gamedata/config/gameplay/dialogs_agroprom.xml
- M gamedata/config/gameplay/dialogs_all_npces.xml
- M gamedata/config/gameplay/dialogs_amk.xml
- M gamedata/config/gameplay/dialogs_artmod.xml
- M gamedata/config/gameplay/dialogs_bar.xml
- M gamedata/config/gameplay/dialogs_cycle_task.xml
- M gamedata/config/gameplay/dialogs_darkscape.xml
- M gamedata/config/gameplay/dialogs_darkvalley.xml
- M gamedata/config/gameplay/dialogs_escape.xml
- M gamedata/config/gameplay/dialogs_flamequest.xml
- M gamedata/config/gameplay/dialogs_garbage.xml
- M gamedata/config/gameplay/dialogs_he_ballon_refill.xml
- M gamedata/config/gameplay/dialogs_he_pda_access.xml
- M gamedata/config/gameplay/dialogs_he_pda_charge.xml
- M gamedata/config/gameplay/dialogs_marsh.xml
- M gamedata/config/gameplay/dialogs_military.xml
- M gamedata/config/gameplay/dialogs_other.xml
- M gamedata/config/gameplay/dialogs_radar.xml
- M gamedata/config/gameplay/dialogs_story.xml
- M gamedata/config/gameplay/dialogs_teleport.xml
- M gamedata/config/gameplay/dialogs_trading.xml
- M gamedata/config/gameplay/dialogs_yantar.xml
- M gamedata/config/gameplay/doc_dialogs_new.xml
- M gamedata/config/gameplay/encyclopedia_amk_recipts.xml
- M gamedata/config/gameplay/encyclopedia_mutants.xml
- M gamedata/config/gameplay/encyclopedia_sak_way.xml
- M gamedata/config/gameplay/encyclopedia_social.xml
- M gamedata/config/gameplay/encyclopedia_tutorial.xml
- M gamedata/config/gameplay/encyclopedia_zone.xml
- M gamedata/config/gameplay/game_tasks.xml
- M gamedata/config/gameplay/game_tasks_by_vendor.xml
- M gamedata/config/gameplay/info_amk_recipts.xml
- M gamedata/config/gameplay/info_known_objects.xml
- M gamedata/config/gameplay/info_original.xml
- M gamedata/config/gameplay/info_portions_doc.xml
- M gamedata/config/gameplay/info_portions_nlc30.xml
- M gamedata/config/gameplay/info_sak_way.xml
- M gamedata/config/gameplay/info_stories.xml
- M gamedata/config/gameplay/kill_stalker_dlg.xml
- M gamedata/config/gameplay/mike_dialogs_new_story.xml
- M gamedata/config/gameplay/npc_profile.xml
- M gamedata/config/gameplay/npc_profile_nlc30_spawn.xml
- M gamedata/config/gameplay/sak_dalogs_agr_dv.xml
- M gamedata/config/gameplay/sak_dalogs_bar_yan.xml
- M gamedata/config/gameplay/sak_dalogs_esc_gar.xml
- M gamedata/config/gameplay/sak_dalogs_lim_atp.xml
- M gamedata/config/gameplay/sak_dialogs_dead_gen_prip.xml
- M gamedata/config/gameplay/stories.xml
- M gamedata/config/gameplay/storyline_info_amk_recipts.xml
- M gamedata/config/gameplay/storyline_info_doc.xml
- M gamedata/config/gameplay/storyline_info_mike.xml
- M gamedata/config/gameplay/storyline_info_pda.xml
- M gamedata/config/gameplay/storyline_info_sak_way.xml
- M gamedata/config/gameplay/storyline_info_taskmanager.xml
- M gamedata/config/gameplay/storyline_info_to_diary.xml
- M gamedata/config/gameplay/vergas_dialogs.xml
- M gamedata/config/localization.ltx
- M gamedata/config/misc/all_outfits_nlc.ltx
- M gamedata/config/misc/amk_items.ltx
- M gamedata/config/misc/arc.ltx
- M gamedata/config/misc/artefacts.ltx
- M gamedata/config/misc/artefacts_amk_mod.ltx
- M gamedata/config/misc/artefacts_cooking.ltx
- M gamedata/config/misc/artefacts_dynamic.ltx
- M gamedata/config/misc/artefacts_mayatnik.ltx
- M gamedata/config/misc/artefacts_unique.ltx
- M gamedata/config/misc/artefacts_unknown.ltx
- M gamedata/config/misc/barter_trade.ltx
- M gamedata/config/misc/death_generic.ltx
- M gamedata/config/misc/death_items_by_communities.ltx
- M gamedata/config/misc/death_items_by_levels.ltx
- M gamedata/config/misc/death_items_count.ltx
- M gamedata/config/misc/dialog_manager.ltx
- M gamedata/config/misc/dream.ltx
- M gamedata/config/misc/effectors.ltx
- M gamedata/config/misc/general_lager.ltx
- M gamedata/config/misc/general_lair.ltx
- M gamedata/config/misc/gps_habar.ltx
- M gamedata/config/misc/gulag_agroprom.ltx
- M gamedata/config/misc/gulag_amk.ltx
- M gamedata/config/misc/gulag_bar.ltx
- M gamedata/config/misc/gulag_dark_valley_nlc.ltx
- M gamedata/config/misc/gulag_escape.ltx
- M gamedata/config/misc/gulag_garbage.ltx
- M gamedata/config/misc/gulag_kishka.ltx
- M gamedata/config/misc/gulag_labx18.ltx
- M gamedata/config/misc/gulag_limansk.ltx
- M gamedata/config/misc/gulag_mil_dolg.ltx
- M gamedata/config/misc/gulag_mil_fblockpost.ltx
- M gamedata/config/misc/gulag_mil_freedom1.ltx
- M gamedata/config/misc/gulag_mil_freedom2.ltx
- M gamedata/config/misc/gulag_mil_mutants.ltx
- M gamedata/config/misc/gulag_mil_nblockpost.ltx
- M gamedata/config/misc/gulag_mil_village.ltx
- M gamedata/config/misc/gulag_mil_zombied.ltx
- M gamedata/config/misc/gulag_military.ltx
- M gamedata/config/misc/gulag_radar_u.ltx
- M gamedata/config/misc/gulag_sarcofag.ltx
- M gamedata/config/misc/gulag_tasks.ltx
- M gamedata/config/misc/gulag_yantar.ltx
- M gamedata/config/misc/he_short_story.ltx
- M gamedata/config/misc/items.ltx
- M gamedata/config/misc/items/anim_items.ltx
- M gamedata/config/misc/items/anim_items/anm_af_resonator.ltx
- M gamedata/config/misc/items/anim_items/anm_antirad.ltx
- M gamedata/config/misc/items/anim_items/anm_backpack.ltx
- M gamedata/config/misc/items/anim_items/anm_bints.ltx
- M gamedata/config/misc/items/anim_items/anm_bread.ltx
- M gamedata/config/misc/items/anim_items/anm_conserva.ltx
- M gamedata/config/misc/items/anim_items/anm_drug_booster.ltx
- M gamedata/config/misc/items/anim_items/anm_energy_drink.ltx
- M gamedata/config/misc/items/anim_items/anm_flask.ltx
- M gamedata/config/misc/items/anim_items/anm_kolbasa.ltx
- M gamedata/config/misc/items/anim_items/anm_medkit.ltx
- M gamedata/config/misc/items/anim_items/anm_other.ltx
- M gamedata/config/misc/items/anim_items/anm_outfit.ltx
- M gamedata/config/misc/items/anim_items/anm_tabletki.ltx
- M gamedata/config/misc/items/anim_items/anm_torch.ltx
- M gamedata/config/misc/items/anim_items/anm_vodka.ltx
- M gamedata/config/misc/monster_items.ltx
- M gamedata/config/misc/paket_classes.ltx
- M gamedata/config/misc/ph_box_generic.ltx
- M gamedata/config/misc/ph_box_items_by_communities.ltx
- M gamedata/config/misc/ph_box_items_by_levels.ltx
- M gamedata/config/misc/ph_box_items_count.ltx
- M gamedata/config/misc/postprocess.ltx
- M gamedata/config/misc/quest_items.ltx
- M gamedata/config/misc/rx_addons.ltx
- M gamedata/config/misc/rx_ai.ltx
- M gamedata/config/misc/rx_bandage.ltx
- M gamedata/config/misc/rx_facer.ltx
- M gamedata/config/misc/rx_ff.ltx
- M gamedata/config/misc/rx_gl.ltx
- M gamedata/config/misc/rx_help_wounded.ltx
- M gamedata/config/misc/rx_kill_wounded.ltx
- M gamedata/config/misc/rx_knife.ltx
- M gamedata/config/misc/rx_script_sound.ltx
- M gamedata/config/misc/sak_quest_items.ltx
- M gamedata/config/misc/script_sound.ltx
- M gamedata/config/misc/shop_barman/barman_buy.ltx
- M gamedata/config/misc/shop_barman/barman_supl.ltx
- M gamedata/config/misc/shop_barman/barman_trade.ltx
- M gamedata/config/misc/shop_barman/trade_barman.ltx
- M gamedata/config/misc/shop_hermit/hermit_barter.ltx
- M gamedata/config/misc/shop_hermit/trade_hermit.ltx
- M gamedata/config/misc/shop_mikhalych/mikhalych_barter.ltx
- M gamedata/config/misc/shop_mikhalych/mikhalych_buy.ltx
- M gamedata/config/misc/shop_mikhalych/mikhalych_supl.ltx
- M gamedata/config/misc/shop_mikhalych/mikhalych_trade.ltx
- M gamedata/config/misc/shop_mikhalych/trade_generic_defines.ltx
- M gamedata/config/misc/shop_mikhalych/trade_mikhalych.ltx
- M gamedata/config/misc/shop_other/junkman_barter.ltx
- M gamedata/config/misc/shop_other/trade_art_exchanger.ltx
- M gamedata/config/misc/shop_other/trade_junkman.ltx
- M gamedata/config/misc/shop_other/trade_kulinar.ltx
- M gamedata/config/misc/shop_other/trade_silvestr.ltx
- M gamedata/config/misc/shop_other/trade_zub.ltx
- M gamedata/config/misc/shop_pahom/pahom_buy.ltx
- M gamedata/config/misc/shop_pahom/pahom_supl.ltx
- M gamedata/config/misc/shop_pahom/pahom_trade.ltx
- M gamedata/config/misc/shop_pahom/trade_pahom.ltx
- M gamedata/config/misc/shop_petrenko/trade_generic_defines.ltx
- M gamedata/config/misc/shop_petrenko/trade_petrenko.ltx
- M gamedata/config/misc/shop_sakharov/barter_saharov.ltx
- M gamedata/config/misc/shop_sakharov/trade_generic_defines.ltx
- M gamedata/config/misc/shop_sakharov/trade_sakharov.ltx
- M gamedata/config/misc/shop_shinkar/trade_generic_defines.ltx
- M gamedata/config/misc/shop_shinkar/trade_shinkar.ltx
- M gamedata/config/misc/shop_sidor/sidor_barter.ltx
- M gamedata/config/misc/shop_sidor/sidor_modificats.ltx
- M gamedata/config/misc/shop_sidor/trade_generic_defines.ltx
- M gamedata/config/misc/shop_sidor/trade_sidor.ltx
- M gamedata/config/misc/sivol_visible_body.ltx
- M gamedata/config/misc/smart_terrain.ltx
- M gamedata/config/misc/smart_terrain_presets.ltx
- M gamedata/config/misc/spawn_items_by_levels.ltx
- M gamedata/config/misc/task_manager.ltx
- M gamedata/config/misc/trade_logic/trade_generic.ltx
- M gamedata/config/misc/trade_logic/trade_generic_bnd.ltx
- M gamedata/config/misc/trade_logic/trade_generic_dlg.ltx
- M gamedata/config/misc/trade_logic/trade_generic_ecl.ltx
- M gamedata/config/misc/trade_logic/trade_generic_frd.ltx
- M gamedata/config/misc/trade_logic/trade_generic_grn.ltx
- M gamedata/config/misc/trade_logic/trade_generic_mil.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_exp.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_mas.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_nov.ltx
- M gamedata/config/misc/trade_logic/trade_generic_stl_vet.ltx
- M gamedata/config/misc/trade_npc/barter_zub.ltx
- M gamedata/config/misc/trade_npc/trade_arny.ltx
- M gamedata/config/misc/trade_npc/trade_dan.ltx
- M gamedata/config/misc/trade_npc/trade_dsc_part_exchanger.ltx
- M gamedata/config/misc/trade_npc/trade_generic.ltx
- M gamedata/config/misc/trade_npc/trade_generic_defines.ltx
- M gamedata/config/misc/trade_npc/trade_ignat.ltx
- M gamedata/config/misc/trade_npc/trade_jila.ltx
- M gamedata/config/misc/trade_npc/trade_kruglov.ltx
- M gamedata/config/misc/trade_npc/trade_kuznetcov.ltx
- M gamedata/config/misc/trade_npc/trade_semenov.ltx
- M gamedata/config/misc/trade_npc/trade_sherstyk.ltx
- M gamedata/config/misc/trade_npc/trade_skriaga.ltx
- M gamedata/config/misc/trade_npc/trade_vahtang.ltx
- M gamedata/config/misc/trade_npc/trade_zub.ltx
- M gamedata/config/misc/treasure_manager.ltx
- M gamedata/config/misc/trigger.ltx
- M gamedata/config/misc/vergas_bag_on_belt.ltx
- M gamedata/config/misc/vergas_decoder.ltx
- M gamedata/config/misc/vergas_items.ltx
- M gamedata/config/misc/vergas_masks.ltx
- M gamedata/config/misc/vergas_pda.ltx
- M gamedata/config/misc/vergas_rykzack.ltx
- M gamedata/config/misc/zone_ameba.ltx
- M gamedata/config/misc/zone_base.ltx
- M gamedata/config/misc/zone_breathe.ltx
- M gamedata/config/misc/zone_bubble.ltx
- M gamedata/config/misc/zone_burningfuzz.ltx
- M gamedata/config/misc/zone_buzz.ltx
- M gamedata/config/misc/zone_field_radioactive.ltx
- M gamedata/config/misc/zone_gravi.ltx
- M gamedata/config/misc/zone_greedy.ltx
- M gamedata/config/misc/zone_ice.ltx
- M gamedata/config/misc/zone_kampfire.ltx
- M gamedata/config/misc/zone_mincer.ltx
- M gamedata/config/misc/zone_mine_acidic.ltx
- M gamedata/config/misc/zone_minefield.ltx
- M gamedata/config/misc/zone_monolith.ltx
- M gamedata/config/misc/zone_mosquitobald.ltx
- M gamedata/config/misc/zone_nogravity.ltx
- M gamedata/config/misc/zone_radioactive.ltx
- M gamedata/config/misc/zone_rustyhair.ltx
- M gamedata/config/misc/zone_sakbuzz.ltx
- M gamedata/config/misc/zone_smallrain.ltx
- M gamedata/config/misc/zone_sphere.ltx
- M gamedata/config/misc/zone_telepatic.ltx
- M gamedata/config/misc/zone_teleport.ltx
- M gamedata/config/misc/zone_torrid.ltx
- M gamedata/config/misc/zone_witchesgalantine.ltx
- M gamedata/config/misc/zone_zavesa.ltx
- M gamedata/config/misc/zone_zharka_static.ltx
- M gamedata/config/misc/zones.ltx
- M gamedata/config/models/bochka.ltx
- M gamedata/config/models/capture/armor_helmet_damage_med.ltx
- M gamedata/config/models/capture/burer_captures.ltx
- M gamedata/config/models/capture/c_dog.ltx
- M gamedata/config/models/capture/cat_captures.ltx
- M gamedata/config/models/capture/chimera_captures.ltx
- M gamedata/config/models/capture/chimera_captures_shoc.ltx
- M gamedata/config/models/capture/clear_sky_1.ltx
- M gamedata/config/models/capture/clear_sky_2.ltx
- M gamedata/config/models/capture/dog_captures.ltx
- M gamedata/config/models/capture/exo_helmet_damage_med.ltx
- M gamedata/config/models/capture/exo_helmet_damage_mon.ltx
- M gamedata/config/models/capture/exo_helmet_damage_zom.ltx
- M gamedata/config/models/capture/flesh_captures.ltx
- M gamedata/config/models/capture/gilet_antigas_damage_low.ltx
- M gamedata/config/models/capture/gilet_antigas_damage_med.ltx
- M gamedata/config/models/capture/gilet_mask_damage_low.ltx
- M gamedata/config/models/capture/gilet_mask_damage_no.ltx
- M gamedata/config/models/capture/human_captures.ltx
- M gamedata/config/models/capture/izlom_captures.ltx
- M gamedata/config/models/capture/kombez_antigas_damage_low.ltx
- M gamedata/config/models/capture/kombez_antigas_damage_med.ltx
- M gamedata/config/models/capture/kombez_damage_hunter1.ltx
- M gamedata/config/models/capture/kombez_helmet_damage_low.ltx
- M gamedata/config/models/capture/kombez_helmet_damage_med.ltx
- M gamedata/config/models/capture/kombez_hunter3_damage.ltx
- M gamedata/config/models/capture/kombez_kupol_damage_high.ltx
- M gamedata/config/models/capture/kombez_kupol_damage_med.ltx
- M gamedata/config/models/capture/kombez_kupol_damage_mon.ltx
- M gamedata/config/models/capture/kombez_mask_damage_low.ltx
- M gamedata/config/models/capture/kombez_mask_damage_mo.ltx
- M gamedata/config/models/capture/kombez_mask_damage_no.ltx
- M gamedata/config/models/capture/kombez_mask_damage_now.ltx
- M gamedata/config/models/capture/kontroler_captures.ltx
- M gamedata/config/models/capture/krovosos_captures.ltx
- M gamedata/config/models/capture/krysuk.ltx
- M gamedata/config/models/capture/kurtka_antigas_damage_low.ltx
- M gamedata/config/models/capture/kurtka_kupol_damage_high.ltx
- M gamedata/config/models/capture/kurtka_mask_damage_no.ltx
- M gamedata/config/models/capture/legs_captures.ltx
- M gamedata/config/models/capture/mutant_boar_captures.ltx
- M gamedata/config/models/capture/plasch_hunter2_damage.ltx
- M gamedata/config/models/capture/plasch_mask_damage_no.ltx
- M gamedata/config/models/capture/poltergeist_captures.ltx
- M gamedata/config/models/capture/pseudodog_captures.ltx
- M gamedata/config/models/capture/psevdogigant_captures.ltx
- M gamedata/config/models/capture/psevdogigant_captures_shoc.ltx
- M gamedata/config/models/capture/rat_captures.ltx
- M gamedata/config/models/capture/snork_captures.ltx
- M gamedata/config/models/capture/snork_captures_shoc.ltx
- M gamedata/config/models/capture/stalker_bandit_3.ltx
- M gamedata/config/models/capture/stalker_freedom_1.ltx
- M gamedata/config/models/capture/stalker_hero_captures.ltx
- M gamedata/config/models/capture/stalker_nebo_1.ltx
- M gamedata/config/models/capture/stalker_nebo_1_.ltx
- M gamedata/config/models/capture/stalker_nebo_2.ltx
- M gamedata/config/models/capture/stalker_nebo_2_.ltx
- M gamedata/config/models/capture/stalker_nebo_3.ltx
- M gamedata/config/models/capture/stalker_nebo_3_.ltx
- M gamedata/config/models/capture/stalker_neutral_1.ltx
- M gamedata/config/models/capture/stalker_serafim_1.ltx
- M gamedata/config/models/capture/stalker_trader_2.ltx
- M gamedata/config/models/capture/tushkano_captures.ltx
- M gamedata/config/models/capture/zombi_captures.ltx
- M gamedata/config/models/capture/zombied_damage_med.ltx
- M gamedata/config/models/capture/zombied_damage_zom.ltx
- M gamedata/config/models/niva_engine.ltx
- M gamedata/config/models/objects/aes_zabor.ltx
- M gamedata/config/models/objects/artefact_monolit.ltx
- M gamedata/config/models/objects/bochka.ltx
- M gamedata/config/models/objects/bottle_3l.ltx
- M gamedata/config/models/objects/box_ammo_01.ltx
- M gamedata/config/models/objects/box_gorb.ltx
- M gamedata/config/models/objects/box_lab_02.ltx
- M gamedata/config/models/objects/box_metal_01.ltx
- M gamedata/config/models/objects/box_metall_01.ltx
- M gamedata/config/models/objects/box_paper.ltx
- M gamedata/config/models/objects/box_wood_01.ltx
- M gamedata/config/models/objects/door_locker.ltx
- M gamedata/config/models/objects/hospital_arka_1.ltx
- M gamedata/config/models/objects/hospital_arka_2.ltx
- M gamedata/config/models/objects/hospital_arka_3.ltx
- M gamedata/config/models/objects/hospital_arka_4.ltx
- M gamedata/config/models/objects/hospital_arka_5.ltx
- M gamedata/config/models/objects/hospital_debris_01.ltx
- M gamedata/config/models/objects/hospital_fence_01.ltx
- M gamedata/config/models/objects/hospital_wooden_board_01.ltx
- M gamedata/config/models/objects/hospital_wooden_board_02.ltx
- M gamedata/config/models/objects/ignore_static.ltx
- M gamedata/config/models/objects/light_night.ltx
- M gamedata/config/models/objects/lut_box.ltx
- M gamedata/config/models/objects/medbox.ltx
- M gamedata/config/models/objects/military_wall_brick_in.ltx
- M gamedata/config/models/objects/monolith_generator.ltx
- M gamedata/config/models/objects/red_forest_rope_01.ltx
- M gamedata/config/models/objects/sarcofag_hologram.ltx
- M gamedata/config/models/objects/wood_board_br_01.ltx
- M gamedata/config/models/objects/wood_board_br_02.ltx
- M gamedata/config/models/objects/wood_board_br_03.ltx
- M gamedata/config/models/objects/wood_board_no_br.ltx
- M gamedata/config/models/objects/wood_fence.ltx
- M gamedata/config/models/objects/wooden_board_01.ltx
- M gamedata/config/models/objects/wooden_board_01_2m.ltx
- M gamedata/config/models/objects/wooden_board_02.ltx
- M gamedata/config/models/objects/wooden_board_03.ltx
- M gamedata/config/models/objects/wooden_board_04.ltx
- M gamedata/config/models/vehicles/niva_engine.ltx
- M gamedata/config/models/vehicles/veh_btr_script_u_01.ltx
- M gamedata/config/models/vehicles/veh_btr_u_01.ltx
- M gamedata/config/models/vehicles/veh_kamaz_u_01.ltx
- M gamedata/config/models/vehicles/veh_mi24_u_01.ltx
- M gamedata/config/models/vehicles/veh_mi2_01.ltx
- M gamedata/config/models/vehicles/veh_niva_u_01.ltx
- M gamedata/config/models/vehicles/veh_tr13_u_01.ltx
- M gamedata/config/models/vehicles/veh_uaz_u_01.ltx
- M gamedata/config/models/vehicles/veh_zaz_u_01.ltx
- M gamedata/config/models/vehicles/veh_zaz_u_01_part0.ltx
- M gamedata/config/models/vehicles/veh_zil_u_01.ltx
- M gamedata/config/ogg_comments_overrides.ltx
- M gamedata/config/prefetch.ltx
- M gamedata/config/prefetch/prefetch.ltx
- M gamedata/config/prefetch/prefetch_globals.ltx
- M gamedata/config/prefetch/prefetch_single.ltx
- M gamedata/config/rspec_default.ltx
- M gamedata/config/rspec_extreme.ltx
- M gamedata/config/rspec_high.ltx
- M gamedata/config/rspec_low.ltx
- M gamedata/config/rspec_maximum.ltx
- M gamedata/config/rspec_minimum.ltx
- M gamedata/config/script.ltx
- M gamedata/config/scripts/actorsleep.ltx
- M gamedata/config/scripts/agr/agr_blockpost_btr.ltx
- M gamedata/config/scripts/agr/agr_blood_growl.ltx
- M gamedata/config/scripts/agr/agr_codedoor1.ltx
- M gamedata/config/scripts/agr/agr_codedoor_0001.ltx
- M gamedata/config/scripts/agr/agr_codedoor_key_0001.ltx
- M gamedata/config/scripts/agr/agr_controller1.ltx
- M gamedata/config/scripts/agr/agr_defector_stalker1.ltx
- M gamedata/config/scripts/agr/agr_factory_assault_trigger.ltx
- M gamedata/config/scripts/agr/agr_factory_location.ltx
- M gamedata/config/scripts/agr/agr_factory_radio_spammer.ltx
- M gamedata/config/scripts/agr/agr_factory_skirmish_defence.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz1.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz2.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz3.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz4.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz5.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz6.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz7.ltx
- M gamedata/config/scripts/agr/agr_factory_spetsnaz8.ltx
- M gamedata/config/scripts/agr/agr_gigant1.ltx
- M gamedata/config/scripts/agr/agr_gigant2.ltx
- M gamedata/config/scripts/agr/agr_helicopter1.ltx
- M gamedata/config/scripts/agr/agr_helicopter2.ltx
- M gamedata/config/scripts/agr/agr_helicopter3.ltx
- M gamedata/config/scripts/agr/agr_helicopter4.ltx
- M gamedata/config/scripts/agr/agr_kirpich.ltx
- M gamedata/config/scripts/agr/agr_klyk_med1_monsters.ltx
- M gamedata/config/scripts/agr/agr_klyk_med2_monsters.ltx
- M gamedata/config/scripts/agr/agr_klyk_med3_monsters.ltx
- M gamedata/config/scripts/agr/agr_korovan_zone.ltx
- M gamedata/config/scripts/agr/agr_krash_btr.ltx
- M gamedata/config/scripts/agr/agr_krot.ltx
- M gamedata/config/scripts/agr/agr_krot_combat_trigger.ltx
- M gamedata/config/scripts/agr/agr_krot_help_trigger.ltx
- M gamedata/config/scripts/agr/agr_metal_small.ltx
- M gamedata/config/scripts/agr/agr_mil_commander.ltx
- M gamedata/config/scripts/agr/agr_mil_private_soldier.ltx
- M gamedata/config/scripts/agr/agr_monster_kirpich.ltx
- M gamedata/config/scripts/agr/agr_muller.ltx
- M gamedata/config/scripts/agr/agr_nii_heli.ltx
- M gamedata/config/scripts/agr/agr_nii_rupor.ltx
- M gamedata/config/scripts/agr/agr_pda_luber.ltx
- M gamedata/config/scripts/agr/agr_physic_destroyable_object.ltx
- M gamedata/config/scripts/agr/agr_ratcatcher.ltx
- M gamedata/config/scripts/agr/agr_sak_teleport_0001.ltx
- M gamedata/config/scripts/agr/agr_sak_teleport_0002.ltx
- M gamedata/config/scripts/agr/agr_sak_teleport_0003.ltx
- M gamedata/config/scripts/agr/agr_sherstyk.ltx
- M gamedata/config/scripts/agr/agr_space_restrictor.ltx
- M gamedata/config/scripts/agr/agr_space_restrictor_0001.ltx
- M gamedata/config/scripts/agr/agr_space_restrictor_0004.ltx
- M gamedata/config/scripts/agr/agr_space_restrictor_tower1.ltx
- M gamedata/config/scripts/agr/agr_stalker.ltx
- M gamedata/config/scripts/agr/agr_stalker_0006.ltx
- M gamedata/config/scripts/agr/agr_stalker_0007.ltx
- M gamedata/config/scripts/agr/agr_stalker_0008.ltx
- M gamedata/config/scripts/agr/agr_stalker_0009.ltx
- M gamedata/config/scripts/agr/agr_stalker_0010.ltx
- M gamedata/config/scripts/agr/agr_tower1_soldier.ltx
- M gamedata/config/scripts/agr/agr_tower2_soldier.ltx
- M gamedata/config/scripts/agr/agr_tower3_soldier.ltx
- M gamedata/config/scripts/agr/agr_zombied_0000.ltx
- M gamedata/config/scripts/agr/agr_zombied_0001.ltx
- M gamedata/config/scripts/agr/agr_zombied_0002.ltx
- M gamedata/config/scripts/agr/agro_tiski_repair.ltx
- M gamedata/config/scripts/agr/doktor_attantion.ltx
- M gamedata/config/scripts/agr/kat_m_bloodsucker_e.ltx
- M gamedata/config/scripts/agr/kat_soldier_0000.ltx
- M gamedata/config/scripts/agr/kat_soldier_0001.ltx
- M gamedata/config/scripts/agr/kat_soldier_0002.ltx
- M gamedata/config/scripts/agr/kat_stalker.ltx
- M gamedata/config/scripts/agr/klyk.ltx
- M gamedata/config/scripts/agr/logic_dumenko.ltx
- M gamedata/config/scripts/agr/rygyi.ltx
- M gamedata/config/scripts/agr/sak_military_stalker.ltx
- M gamedata/config/scripts/agr/u_agr_inventory_box_klyk.ltx
- M gamedata/config/scripts/agr/u_agr_space_restrictor.ltx
- M gamedata/config/scripts/agr/u_agr_space_restrictor_0000.ltx
- M gamedata/config/scripts/agr_und/agr_sak_teleport_0001.ltx
- M gamedata/config/scripts/agr_und/agr_sak_teleport_0002.ltx
- M gamedata/config/scripts/agr_und/agr_sak_teleport_0003.ltx
- M gamedata/config/scripts/agr_und/kat_stalker.ltx
- M gamedata/config/scripts/agr_und/klyk.ltx
- M gamedata/config/scripts/agr_und/u_agr_inventory_box_klyk.ltx
- M gamedata/config/scripts/amk/aes.ltx
- M gamedata/config/scripts/amk/agroprom.ltx
- M gamedata/config/scripts/amk/amk_respawns.ltx
- M gamedata/config/scripts/amk/anoms/anoms.ltx
- M gamedata/config/scripts/amk/anoms/arts_spawn_restr.ltx
- M gamedata/config/scripts/amk/bar.ltx
- M gamedata/config/scripts/amk/bunker.ltx
- M gamedata/config/scripts/amk/darkscape.ltx
- M gamedata/config/scripts/amk/darkvalley.ltx
- M gamedata/config/scripts/amk/deadcity.ltx
- M gamedata/config/scripts/amk/escape.ltx
- M gamedata/config/scripts/amk/garbage.ltx
- M gamedata/config/scripts/amk/generators.ltx
- M gamedata/config/scripts/amk/hospital.ltx
- M gamedata/config/scripts/amk/labx10.ltx
- M gamedata/config/scripts/amk/labx8.ltx
- M gamedata/config/scripts/amk/limansk.ltx
- M gamedata/config/scripts/amk/marsh.ltx
- M gamedata/config/scripts/amk/military.ltx
- M gamedata/config/scripts/amk/monolit.ltx
- M gamedata/config/scripts/amk/peshera.ltx
- M gamedata/config/scripts/amk/pripyat.ltx
- M gamedata/config/scripts/amk/radar.ltx
- M gamedata/config/scripts/amk/red_forest.ltx
- M gamedata/config/scripts/amk/rostok.ltx
- M gamedata/config/scripts/amk/sarcofag.ltx
- M gamedata/config/scripts/amk/st2.ltx
- M gamedata/config/scripts/amk/village.ltx
- M gamedata/config/scripts/amk/warlab.ltx
- M gamedata/config/scripts/amk/x16.ltx
- M gamedata/config/scripts/amk/x18.ltx
- M gamedata/config/scripts/amk/yantar.ltx
- M gamedata/config/scripts/av_peshera/av_peshera_physic_object_0002.ltx
- M gamedata/config/scripts/av_peshera/av_peshera_physic_object_0004.ltx
- M gamedata/config/scripts/av_peshera/av_peshera_physic_object_0006.ltx
- M gamedata/config/scripts/av_peshera/sak_priemnik.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0000.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0001.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0002.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0003.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0004.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0005.ltx
- M gamedata/config/scripts/aver/av_peshera_physic_object_0006.ltx
- M gamedata/config/scripts/aver/aver_doctor.ltx
- M gamedata/config/scripts/aver/aver_klyk.ltx
- M gamedata/config/scripts/aver/aver_matras_3.ltx
- M gamedata/config/scripts/aver/aver_mil_lchanger_teleport.ltx
- M gamedata/config/scripts/aver/aver_monolit.ltx
- M gamedata/config/scripts/aver/aver_monolit_zone.ltx
- M gamedata/config/scripts/aver/aver_rygyi.ltx
- M gamedata/config/scripts/aver/aver_sak_teleport_0002.ltx
- M gamedata/config/scripts/aver/dogfrend.ltx
- M gamedata/config/scripts/aver/doktor2.ltx
- M gamedata/config/scripts/aver/les_umuvalnik.ltx
- M gamedata/config/scripts/aver/marsh_monolit_0001.ltx
- M gamedata/config/scripts/aver/peschera_teleport_0001.ltx
- M gamedata/config/scripts/aver/peschera_teleport_0002.ltx
- M gamedata/config/scripts/aver/peschera_teleport_0003.ltx
- M gamedata/config/scripts/aver/peschera_teleport_0004.ltx
- M gamedata/config/scripts/aver/pesh_botan.ltx
- M gamedata/config/scripts/aver/pesh_botan_start.ltx
- M gamedata/config/scripts/aver/pesh_button1.ltx
- M gamedata/config/scripts/aver/pesh_button2.ltx
- M gamedata/config/scripts/aver/pesh_button3.ltx
- M gamedata/config/scripts/aver/pesh_prison_door.ltx
- M gamedata/config/scripts/aver/pesh_sleep.ltx
- M gamedata/config/scripts/aver/pesh_spec_hran.ltx
- M gamedata/config/scripts/aver/peshera_codedoor_0007.ltx
- M gamedata/config/scripts/aver/peshera_gas_zone.ltx
- M gamedata/config/scripts/aver/sak_priemnik.ltx
- M gamedata/config/scripts/aver/speakerphon_pesh_01.ltx
- M gamedata/config/scripts/aver/speakerphon_pesh_02.ltx
- M gamedata/config/scripts/bar/bagram.ltx
- M gamedata/config/scripts/bar/bar_arena_man.ltx
- M gamedata/config/scripts/bar/bar_arny.ltx
- M gamedata/config/scripts/bar/bar_attantion.ltx
- M gamedata/config/scripts/bar/bar_attantion_new.ltx
- M gamedata/config/scripts/bar/bar_bagram.ltx
- M gamedata/config/scripts/bar/bar_bar_door_guard.ltx
- M gamedata/config/scripts/bar/bar_bar_drunk_dolg.ltx
- M gamedata/config/scripts/bar/bar_bar_entry_zone.ltx
- M gamedata/config/scripts/bar/bar_bar_guard.ltx
- M gamedata/config/scripts/bar/bar_bar_guard_2.ltx
- M gamedata/config/scripts/bar/bar_bar_lisiy.ltx
- M gamedata/config/scripts/bar/bar_bar_osvedomitel.ltx
- M gamedata/config/scripts/bar/bar_bar_room_area.ltx
- M gamedata/config/scripts/bar/bar_bar_visitor_hunter.ltx
- M gamedata/config/scripts/bar/bar_bar_visitors_1_2.ltx
- M gamedata/config/scripts/bar/bar_bar_visitors_3_2.ltx
- M gamedata/config/scripts/bar/bar_bar_visitors_3_3.ltx
- M gamedata/config/scripts/bar/bar_bar_visitors_3_4.ltx
- M gamedata/config/scripts/bar/bar_barman.ltx
- M gamedata/config/scripts/bar/bar_barometr.ltx
- M gamedata/config/scripts/bar/bar_block_exit_border_01.ltx
- M gamedata/config/scripts/bar/bar_codelock1.ltx
- M gamedata/config/scripts/bar/bar_diler_zone.ltx
- M gamedata/config/scripts/bar/bar_docent.ltx
- M gamedata/config/scripts/bar/bar_dolg_barin.ltx
- M gamedata/config/scripts/bar/bar_dolg_brom.ltx
- M gamedata/config/scripts/bar/bar_dolg_bunker_0001.ltx
- M gamedata/config/scripts/bar/bar_dolg_bunker_0002.ltx
- M gamedata/config/scripts/bar/bar_dolg_general_kill_zone.ltx
- M gamedata/config/scripts/bar/bar_dolg_guard_door2.ltx
- M gamedata/config/scripts/bar/bar_dolg_ivancov.ltx
- M gamedata/config/scripts/bar/bar_dolg_leader.ltx
- M gamedata/config/scripts/bar/bar_dolg_lucky.ltx
- M gamedata/config/scripts/bar/bar_dolg_prapor.ltx
- M gamedata/config/scripts/bar/bar_dolg_territory.ltx
- M gamedata/config/scripts/bar/bar_dolg_veterans_bunker_kill_zone.ltx
- M gamedata/config/scripts/bar/bar_dolg_voronin.ltx
- M gamedata/config/scripts/bar/bar_dolgovazyi.ltx
- M gamedata/config/scripts/bar/bar_door1.ltx
- M gamedata/config/scripts/bar/bar_garik.ltx
- M gamedata/config/scripts/bar/bar_hunter.ltx
- M gamedata/config/scripts/bar/bar_ivar.ltx
- M gamedata/config/scripts/bar/bar_jorik.ltx
- M gamedata/config/scripts/bar/bar_kocherga.ltx
- M gamedata/config/scripts/bar/bar_lisaped.ltx
- M gamedata/config/scripts/bar/bar_lisiy.ltx
- M gamedata/config/scripts/bar/bar_petrenko_item_30.ltx
- M gamedata/config/scripts/bar/bar_physic_destroyable_object_0008.ltx
- M gamedata/config/scripts/bar/bar_radio.ltx
- M gamedata/config/scripts/bar/bar_sak_box_0001.ltx
- M gamedata/config/scripts/bar/bar_sak_box_0002.ltx
- M gamedata/config/scripts/bar/bar_sak_teleport_0001.ltx
- M gamedata/config/scripts/bar/bar_sak_teleport_0002.ltx
- M gamedata/config/scripts/bar/bar_sak_teleport_0003.ltx
- M gamedata/config/scripts/bar/bar_sak_teleport_0004.ltx
- M gamedata/config/scripts/bar/bar_seif_zone.ltx
- M gamedata/config/scripts/bar/bar_sleep_matras.ltx
- M gamedata/config/scripts/bar/bar_start_megafon.ltx
- M gamedata/config/scripts/bar/bar_stol_6_ph_door_0002.ltx
- M gamedata/config/scripts/bar/bar_stol_ph_door.ltx
- M gamedata/config/scripts/bar/bar_sultan.ltx
- M gamedata/config/scripts/bar/bar_tiski_repair.ltx
- M gamedata/config/scripts/bar/bar_umuvalnik.ltx
- M gamedata/config/scripts/bar/bar_zub.ltx
- M gamedata/config/scripts/bar/barin.ltx
- M gamedata/config/scripts/bar/dolg_dummy.ltx
- M gamedata/config/scripts/bar/dolg_megafon.ltx
- M gamedata/config/scripts/bar/dolg_trader_zone.ltx
- M gamedata/config/scripts/bar/dolgovazyi.ltx
- M gamedata/config/scripts/bar/door_metal_small_bar.ltx
- M gamedata/config/scripts/bar/sak.ltx
- M gamedata/config/scripts/bar/sanek_hran_door_01.ltx
- M gamedata/config/scripts/bar/sim_stalker_prapor.ltx
- M gamedata/config/scripts/bar/sultan.ltx
- M gamedata/config/scripts/brainlab/brain_system_message.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_01.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_02.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_03.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_04.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_05.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_06.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_07.ltx
- M gamedata/config/scripts/brainlab/x16_burer_normal_08.ltx
- M gamedata/config/scripts/brainlab/x16_burer_strong_01.ltx
- M gamedata/config/scripts/brainlab/x16_burer_strong_02.ltx
- M gamedata/config/scripts/brainlab/x16_burer_strong_03.ltx
- M gamedata/config/scripts/brainlab/x16_burer_strong_04.ltx
- M gamedata/config/scripts/brainlab/x16_burer_yan1.ltx
- M gamedata/config/scripts/brainlab/x16_burer_yan2.ltx
- M gamedata/config/scripts/brainlab/x16_codedoor1.ltx
- M gamedata/config/scripts/brainlab/x16_codelock1.ltx
- M gamedata/config/scripts/brainlab/x16_codelock2.ltx
- M gamedata/config/scripts/brainlab/x16_door_level_changer.ltx
- M gamedata/config/scripts/brainlab/x16_generator_sound_1.ltx
- M gamedata/config/scripts/brainlab/x16_generator_sound_2.ltx
- M gamedata/config/scripts/brainlab/x16_generator_sound_3.ltx
- M gamedata/config/scripts/brainlab/x16_home_snork_0000.ltx
- M gamedata/config/scripts/brainlab/x16_home_snork_0002.ltx
- M gamedata/config/scripts/brainlab/x16_home_snork_0013.ltx
- M gamedata/config/scripts/brainlab/x16_locked_door_x18_1.ltx
- M gamedata/config/scripts/brainlab/x16_locked_door_x18_2.ltx
- M gamedata/config/scripts/brainlab/x16_pult_sound_3.ltx
- M gamedata/config/scripts/brainlab/x16_snork_normal_0001.ltx
- M gamedata/config/scripts/brainlab/x16_snork_normal_0009.ltx
- M gamedata/config/scripts/brainlab/x16_snork_normal_0012.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_01.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_02.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_03.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_04.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_05.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_06.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_07.ltx
- M gamedata/config/scripts/brainlab/x16_switcher_08.ltx
- M gamedata/config/scripts/brainlab/x16_zombie_trup.ltx
- M gamedata/config/scripts/brainlab/yan_grate.ltx
- M gamedata/config/scripts/brainlab/yan_labx16_switcher1.ltx
- M gamedata/config/scripts/brainlab/yan_labx16_switcher2.ltx
- M gamedata/config/scripts/brainlab/yan_labx16_switcher3.ltx
- M gamedata/config/scripts/brainlab/yan_labx16_switcher_primary.ltx
- M gamedata/config/scripts/brainlab/yan_zasada_restrictor.ltx
- M gamedata/config/scripts/brainlab/yan_zasada_restrictor_0001.ltx
- M gamedata/config/scripts/brainlab/yantar_psi_radiotion_75.ltx
- M gamedata/config/scripts/brainlab/yantar_psi_radiotion_sound.ltx
- M gamedata/config/scripts/bunk/bun_codedoor4.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_door_0000.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_door_0003.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_door_0005.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0000.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0001.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0002.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0003.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0004.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0005.ltx
- M gamedata/config/scripts/bunk/bun_codedoor_key_0008.ltx
- M gamedata/config/scripts/bunk/bun_deactivate_radar.ltx
- M gamedata/config/scripts/bunk/bun_komp_klava_secr.ltx
- M gamedata/config/scripts/bunk/bun_primary_switcher_0000.ltx
- M gamedata/config/scripts/bunk/bun_pult.ltx
- M gamedata/config/scripts/bunk/bun_pult_rychag.ltx
- M gamedata/config/scripts/bunk/bun_space_restrictor_ognetushitel_2.ltx
- M gamedata/config/scripts/bunk/bun_spec_monolith_01.ltx
- M gamedata/config/scripts/bunk/x12_tiski_repair.ltx
- M gamedata/config/scripts/candle.ltx
- M gamedata/config/scripts/candle_logic.ltx
- M gamedata/config/scripts/cit/dead_chudishe.ltx
- M gamedata/config/scripts/cit/dead_codedoor_key_01.ltx
- M gamedata/config/scripts/cit/dead_ecolog_04.ltx
- M gamedata/config/scripts/cit/dead_hangry_zone.ltx
- M gamedata/config/scripts/cit/dead_prip_kill_zone.ltx
- M gamedata/config/scripts/cit/pesh_prison_door.ltx
- M gamedata/config/scripts/cit/vertolet_deadsity.ltx
- M gamedata/config/scripts/cmon/control_monolith_01.ltx
- M gamedata/config/scripts/cmon/control_monolith_02.ltx
- M gamedata/config/scripts/cmon/mon_big_fire_res_0001.ltx
- M gamedata/config/scripts/cmon/mon_gas_rychag_01.ltx
- M gamedata/config/scripts/cmon/mon_gas_rychag_02.ltx
- M gamedata/config/scripts/cmon/mon_gas_tank_door.ltx
- M gamedata/config/scripts/cmon/mon_m_poltergeist_normal_flame.ltx
- M gamedata/config/scripts/cmon/mon_particle_poltergeist.ltx
- M gamedata/config/scripts/cmon/mon_stalker.ltx
- M gamedata/config/scripts/cmon/mon_stalker_monolith_post3.ltx
- M gamedata/config/scripts/cmon/mon_tupik_zone.ltx
- M gamedata/config/scripts/cmon/sar_monolith_gen.ltx
- M gamedata/config/scripts/cmon/sar_monolith_gen_main.ltx
- M gamedata/config/scripts/dar/dar_door1.ltx
- M gamedata/config/scripts/dar/dar_door2.ltx
- M gamedata/config/scripts/dar/dar_door3.ltx
- M gamedata/config/scripts/dar/dar_gigant.ltx
- M gamedata/config/scripts/dar/komp_blok.ltx
- M gamedata/config/scripts/dar/vertolet_td.ltx
- M gamedata/config/scripts/dar/x18_X16_lev_chang_door.ltx
- M gamedata/config/scripts/dar/x18_codedoor1.ltx
- M gamedata/config/scripts/dar/x18_codedoor2.ltx
- M gamedata/config/scripts/dar/x18_codedoor_0003.ltx
- M gamedata/config/scripts/dar/x18_codedoor_control_room.ltx
- M gamedata/config/scripts/dar/x18_codedoor_control_room_key_out.ltx
- M gamedata/config/scripts/dar/x18_codedoor_key_0007.ltx
- M gamedata/config/scripts/dar/x18_codedoor_key_control_room_in.ltx
- M gamedata/config/scripts/dar/x18_codelock1.ltx
- M gamedata/config/scripts/dar/x18_codelock12.ltx
- M gamedata/config/scripts/dar/x18_door3_open_zone.ltx
- M gamedata/config/scripts/dar/x18_monsters.ltx
- M gamedata/config/scripts/dar/x18_switcher_01.ltx
- M gamedata/config/scripts/dar/x18_switcher_02.ltx
- M gamedata/config/scripts/dar/x18_switcher_03.ltx
- M gamedata/config/scripts/dar/x18_switcher_04.ltx
- M gamedata/config/scripts/dar/x18_switcher_05.ltx
- M gamedata/config/scripts/dar/x18_switcher_06.ltx
- M gamedata/config/scripts/dar/x18_switcher_07.ltx
- M gamedata/config/scripts/dar/x18_switcher_08.ltx
- M gamedata/config/scripts/dar/x18_switcher_09.ltx
- M gamedata/config/scripts/dcity/dead_codedoor_key_01.ltx
- M gamedata/config/scripts/dcity/dead_ecolog_04.ltx
- M gamedata/config/scripts/dcity/dead_hangry_zone.ltx
- M gamedata/config/scripts/dcity/dead_prip_kill_zone.ltx
- M gamedata/config/scripts/dcity/pesh_prison_door.ltx
- M gamedata/config/scripts/door_metal_large.ltx
- M gamedata/config/scripts/door_metal_small_sak.ltx
- M gamedata/config/scripts/door_metal_small_unlocked.ltx
- M gamedata/config/scripts/door_wood_large_unlocked.ltx
- M gamedata/config/scripts/door_wood_small_prip.ltx
- M gamedata/config/scripts/dsc/dar_sleep_matras.ltx
- M gamedata/config/scripts/dsc/dsc_1_monsters.ltx
- M gamedata/config/scripts/dsc/dsc_add_monsters_1_zone.ltx
- M gamedata/config/scripts/dsc/dsc_chimera1.ltx
- M gamedata/config/scripts/dsc/dsc_chimera2.ltx
- M gamedata/config/scripts/dsc/dsc_chimera3.ltx
- M gamedata/config/scripts/dsc/dsc_controller_ignat.ltx
- M gamedata/config/scripts/dsc/dsc_dar_lchanger_teleport.ltx
- M gamedata/config/scripts/dsc/dsc_diler.ltx
- M gamedata/config/scripts/dsc/dsc_ecolog_glavn.ltx
- M gamedata/config/scripts/dsc/dsc_ecolog_petrov.ltx
- M gamedata/config/scripts/dsc/dsc_kolonna_burer1.ltx
- M gamedata/config/scripts/dsc/dsc_kolonna_burer2.ltx
- M gamedata/config/scripts/dsc/dsc_kolonna_controller.ltx
- M gamedata/config/scripts/dsc/dsc_part_exchanger.ltx
- M gamedata/config/scripts/dsc/dsc_stalker_lager0.ltx
- M gamedata/config/scripts/dsc/dsc_tiski_repair.ltx
- M gamedata/config/scripts/dsc/dsc_trup_01.ltx
- M gamedata/config/scripts/dsc/dsc_umuvalnik.ltx
- M gamedata/config/scripts/dsc/dsc_zapravka_zasada1.ltx
- M gamedata/config/scripts/dsc/dsc_zapravka_zasada2.ltx
- M gamedata/config/scripts/dsc/dsc_zapravka_zasada3.ltx
- M gamedata/config/scripts/dsc/dsc_zasada1.ltx
- M gamedata/config/scripts/dsc/dsc_zasada2.ltx
- M gamedata/config/scripts/dsc/dsc_zasada3.ltx
- M gamedata/config/scripts/dsc/dsc_zombied_01.ltx
- M gamedata/config/scripts/dsc/dsc_zombied_02.ltx
- M gamedata/config/scripts/dsc/dsc_zombied_03.ltx
- M gamedata/config/scripts/dsc/dsk_sanek.ltx
- M gamedata/config/scripts/dsc/green_glavn.ltx
- M gamedata/config/scripts/dsc/green_guard.ltx
- M gamedata/config/scripts/dsc/val_bandit_krisyk.ltx
- M gamedata/config/scripts/dummy.ltx
- M gamedata/config/scripts/earthshake.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_01.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_02.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_03.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_04.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_05.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_06.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_07.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_08.ltx
- M gamedata/config/scripts/ecolog_apparat_rychag_09.ltx
- M gamedata/config/scripts/esc/atp_dinamit_restrictor.ltx
- M gamedata/config/scripts/esc/doktor1.ltx
- M gamedata/config/scripts/esc/door_metal_small_stas.ltx
- M gamedata/config/scripts/esc/esc_atp_bandits_0001.ltx
- M gamedata/config/scripts/esc/esc_attantion.ltx
- M gamedata/config/scripts/esc/esc_bandit_klava.ltx
- M gamedata/config/scripts/esc/esc_blockpost_btr.ltx
- M gamedata/config/scripts/esc/esc_blockpost_heli.ltx
- M gamedata/config/scripts/esc/esc_blockpost_patrol_restrictor.ltx
- M gamedata/config/scripts/esc/esc_blockpost_repairer.ltx
- M gamedata/config/scripts/esc/esc_bridge_kill_zone.ltx
- M gamedata/config/scripts/esc/esc_bridge_sr_zone.ltx
- M gamedata/config/scripts/esc/esc_diador.ltx
- M gamedata/config/scripts/esc/esc_doctor.ltx
- M gamedata/config/scripts/esc/esc_dogs_fox_attack.ltx
- M gamedata/config/scripts/esc/esc_door0001.ltx
- M gamedata/config/scripts/esc/esc_door0002.ltx
- M gamedata/config/scripts/esc/esc_door0008.ltx
- M gamedata/config/scripts/esc/esc_dram_restrictor.ltx
- M gamedata/config/scripts/esc/esc_exit_bandit_diador.ltx
- M gamedata/config/scripts/esc/esc_exit_bandit_restrictor.ltx
- M gamedata/config/scripts/esc/esc_fabrika_bandit_space_restrictor.ltx
- M gamedata/config/scripts/esc/esc_fluger.ltx
- M gamedata/config/scripts/esc/esc_fox_dogs.ltx
- M gamedata/config/scripts/esc/esc_heli.ltx
- M gamedata/config/scripts/esc/esc_heli_attack_zone.ltx
- M gamedata/config/scripts/esc/esc_heli_bridge_defense.ltx
- M gamedata/config/scripts/esc/esc_in_restrictor.ltx
- M gamedata/config/scripts/esc/esc_lager_guard_kill_zone.ltx
- M gamedata/config/scripts/esc/esc_lager_guard_warn_zone2.ltx
- M gamedata/config/scripts/esc/esc_matugalnik.ltx
- M gamedata/config/scripts/esc/esc_neumeha.ltx
- M gamedata/config/scripts/esc/esc_neumeha_go_zone.ltx
- M gamedata/config/scripts/esc/esc_oksana.ltx
- M gamedata/config/scripts/esc/esc_petruha.ltx
- M gamedata/config/scripts/esc/esc_physic_destroyable_object_0083.ltx
- M gamedata/config/scripts/esc/esc_physic_destroyable_object_0084.ltx
- M gamedata/config/scripts/esc/esc_physic_destroyable_object_0085.ltx
- M gamedata/config/scripts/esc/esc_physic_destroyable_object_0086.ltx
- M gamedata/config/scripts/esc/esc_physic_destroyable_object_0087.ltx
- M gamedata/config/scripts/esc/esc_physic_destroyable_object_0088.ltx
- M gamedata/config/scripts/esc/esc_pseudodog_strong.ltx
- M gamedata/config/scripts/esc/esc_return_from_dv.ltx
- M gamedata/config/scripts/esc/esc_return_from_gar.ltx
- M gamedata/config/scripts/esc/esc_return_from_hos.ltx
- M gamedata/config/scripts/esc/esc_return_restr.ltx
- M gamedata/config/scripts/esc/esc_sak_teleport_0000.ltx
- M gamedata/config/scripts/esc/esc_sak_teleport_0002.ltx
- M gamedata/config/scripts/esc/esc_shustryi.ltx
- M gamedata/config/scripts/esc/esc_sid_guard.ltx
- M gamedata/config/scripts/esc/esc_sid_guard_sub.ltx
- M gamedata/config/scripts/esc/esc_sleep_matras.ltx
- M gamedata/config/scripts/esc/esc_suicidal_flash_restrictor.ltx
- M gamedata/config/scripts/esc/esc_suicidal_flesh.ltx
- M gamedata/config/scripts/esc/esc_switcher_01.ltx
- M gamedata/config/scripts/esc/esc_tips_fox.ltx
- M gamedata/config/scripts/esc/esc_tiski_repair.ltx
- M gamedata/config/scripts/esc/esc_trader.ltx
- M gamedata/config/scripts/esc/esc_trader_door.ltx
- M gamedata/config/scripts/esc/esc_trader_pribor_0002.ltx
- M gamedata/config/scripts/esc/esc_trader_sound_zone.ltx
- M gamedata/config/scripts/esc/esc_trader_zone.ltx
- M gamedata/config/scripts/esc/esc_trader_zone_task.ltx
- M gamedata/config/scripts/esc/esc_umuvalnik.ltx
- M gamedata/config/scripts/esc/esc_vergas.ltx
- M gamedata/config/scripts/esc/esc_volk_docs_on.ltx
- M gamedata/config/scripts/esc/escape_blockpost_zone.ltx
- M gamedata/config/scripts/esc/escape_trader.ltx
- M gamedata/config/scripts/esc/marsh_oksana2.ltx
- M gamedata/config/scripts/esc/neumeha.ltx
- M gamedata/config/scripts/esc/space_restrictor_vagonchik.ltx
- M gamedata/config/scripts/esc/speakerphon_esc_01.ltx
- M gamedata/config/scripts/esc/store_stas_seif.ltx
- M gamedata/config/scripts/esc/sultan_inventory_box_0001.ltx
- M gamedata/config/scripts/esc/trader_pribor_0002.ltx
- M gamedata/config/scripts/esc_sleep.ltx
- M gamedata/config/scripts/fear_sound_forest.ltx
- M gamedata/config/scripts/fear_sound_hospital.ltx
- M gamedata/config/scripts/fear_sound_item.ltx
- M gamedata/config/scripts/fear_sound_limansk.ltx
- M gamedata/config/scripts/fear_sound_outdoors.ltx
- M gamedata/config/scripts/fear_sound_peshery.ltx
- M gamedata/config/scripts/fear_sound_pripyat.ltx
- M gamedata/config/scripts/fear_sound_rostok.ltx
- M gamedata/config/scripts/fear_sound_stancia.ltx
- M gamedata/config/scripts/fear_sound_underground.ltx
- M gamedata/config/scripts/gar/bag_gar_01.ltx
- M gamedata/config/scripts/gar/gar_actor_scenu_sorval.ltx
- M gamedata/config/scripts/gar/gar_apostol.ltx
- M gamedata/config/scripts/gar/gar_bandit_blokpost_online_rest.ltx
- M gamedata/config/scripts/gar/gar_bandit_blokpost_online_rest_from_bar.ltx
- M gamedata/config/scripts/gar/gar_bandit_blokpost_pass_zone.ltx
- M gamedata/config/scripts/gar/gar_bandit_blokpost_sign_restr.ltx
- M gamedata/config/scripts/gar/gar_bandit_oreh.ltx
- M gamedata/config/scripts/gar/gar_bandits_fire.ltx
- M gamedata/config/scripts/gar/gar_bandits_fire_2.ltx
- M gamedata/config/scripts/gar/gar_bandits_hos_restrictor.ltx
- M gamedata/config/scripts/gar/gar_bandits_restrictor.ltx
- M gamedata/config/scripts/gar/gar_bandits_shuher.ltx
- M gamedata/config/scripts/gar/gar_bandits_val_restrictor.ltx
- M gamedata/config/scripts/gar/gar_dm_bandit_1.ltx
- M gamedata/config/scripts/gar/gar_dm_bandit_2.ltx
- M gamedata/config/scripts/gar/gar_dm_bandit_3.ltx
- M gamedata/config/scripts/gar/gar_dm_novice.ltx
- M gamedata/config/scripts/gar/gar_dolg_pass_gates.ltx
- M gamedata/config/scripts/gar/gar_door_trader_box.ltx
- M gamedata/config/scripts/gar/gar_hellcar_monsters.ltx
- M gamedata/config/scripts/gar/gar_hellcar_zone.ltx
- M gamedata/config/scripts/gar/gar_monster_angar_zone.ltx
- M gamedata/config/scripts/gar/gar_monster_angar_zone_2.ltx
- M gamedata/config/scripts/gar/gar_monster_bandits.ltx
- M gamedata/config/scripts/gar/gar_monster_bandits_zone.ltx
- M gamedata/config/scripts/gar/gar_monster_stalkers_zone.ltx
- M gamedata/config/scripts/gar/gar_monster_tonel_zone.ltx
- M gamedata/config/scripts/gar/gar_monsters.ltx
- M gamedata/config/scripts/gar/gar_monsters_angar.ltx
- M gamedata/config/scripts/gar/gar_monsters_tonel.ltx
- M gamedata/config/scripts/gar/gar_sak_ataka_bandit.ltx
- M gamedata/config/scripts/gar/gar_sak_blok_post.ltx
- M gamedata/config/scripts/gar/gar_sak_teleport_0001.ltx
- M gamedata/config/scripts/gar/gar_seryi_factory.ltx
- M gamedata/config/scripts/gar/gar_seryi_factory_0000.ltx
- M gamedata/config/scripts/gar/gar_seryi_factory_tips.ltx
- M gamedata/config/scripts/gar/gar_seryi_igrok_restrictor.ltx
- M gamedata/config/scripts/gar/gar_space_restrictor_0003.ltx
- M gamedata/config/scripts/gar/gar_stalker_binoc.ltx
- M gamedata/config/scripts/gar/gar_yurik.ltx
- M gamedata/config/scripts/gar_tambovski.ltx
- M gamedata/config/scripts/gen/gen_codedoor_1.ltx
- M gamedata/config/scripts/gen/gen_codedoor_key.ltx
- M gamedata/config/scripts/gen/gen_codedoor_key_2.ltx
- M gamedata/config/scripts/gen/gen_crash_vertolet.ltx
- M gamedata/config/scripts/gen/gen_door_note_01.ltx
- M gamedata/config/scripts/gen/gen_hermit.ltx
- M gamedata/config/scripts/gen/gen_iskatel.ltx
- M gamedata/config/scripts/gen/gen_mil_group2_zone.ltx
- M gamedata/config/scripts/gen/gen_mil_group_zone.ltx
- M gamedata/config/scripts/gen/gen_sak_teleport_dead.ltx
- M gamedata/config/scripts/gen/gen_tele_zone.ltx
- M gamedata/config/scripts/gen/gen_xod_baze.ltx
- M gamedata/config/scripts/gen/matras_gen_01.ltx
- M gamedata/config/scripts/hos/hos_burer_hos.ltx
- M gamedata/config/scripts/hos/hos_junkman.ltx
- M gamedata/config/scripts/hos/hos_sak_teleport_0001.ltx
- M gamedata/config/scripts/hos/hos_sak_teleport_0002.ltx
- M gamedata/config/scripts/hos/in_hos_junkman.ltx
- M gamedata/config/scripts/inventory_box_no_use.ltx
- M gamedata/config/scripts/koster.ltx
- M gamedata/config/scripts/koster_logic.ltx
- M gamedata/config/scripts/labx10/door_metal_large.ltx
- M gamedata/config/scripts/labx10/door_wood_large_unlocked.ltx
- M gamedata/config/scripts/labx10/labx10_button.ltx
- M gamedata/config/scripts/labx10/labx10_button1.ltx
- M gamedata/config/scripts/labx10/labx10_button2.ltx
- M gamedata/config/scripts/labx10/labx10_button2_door.ltx
- M gamedata/config/scripts/labx10/labx10_button3.ltx
- M gamedata/config/scripts/labx10/labx10_button4.ltx
- M gamedata/config/scripts/labx10/labx10_button5.ltx
- M gamedata/config/scripts/labx10/labx10_button6.ltx
- M gamedata/config/scripts/labx10/labx10_button7.ltx
- M gamedata/config/scripts/labx10/labx10_button8.ltx
- M gamedata/config/scripts/labx10/labx10_button_gates.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor1.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor2.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor4.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_0005.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_door_0001.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_door_0003.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_door_0006.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_key_0002.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_key_0004.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_key_0005.ltx
- M gamedata/config/scripts/labx10/labx10_codedoor_key_0006.ltx
- M gamedata/config/scripts/labx10/labx10_codelock1.ltx
- M gamedata/config/scripts/labx10/labx10_codelock3.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0000.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0001.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0002.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0003.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0004.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0005.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0006.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0007.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0008.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0009.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0010.ltx
- M gamedata/config/scripts/labx10/labx10_komp_blok_0011.ltx
- M gamedata/config/scripts/labx10/labx10_physic_destroyable_object_0011.ltx
- M gamedata/config/scripts/labx10/labx10_physic_object_0018.ltx
- M gamedata/config/scripts/labx10/labx10_physic_object_0033.ltx
- M gamedata/config/scripts/labx10/labx10_physic_object_0101.ltx
- M gamedata/config/scripts/labx10/labx10_physic_object_0102.ltx
- M gamedata/config/scripts/labx10/labx10_prizrak_attantion.ltx
- M gamedata/config/scripts/labx10/labx10_rubilnik_rychag.ltx
- M gamedata/config/scripts/labx10/labx10_rubilnik_x12.ltx
- M gamedata/config/scripts/labx10/labx10_skelet_attantion.ltx
- M gamedata/config/scripts/labx10/labx10_small_door.ltx
- M gamedata/config/scripts/labx10/labx10_ventil.ltx
- M gamedata/config/scripts/labx10/pults_attantion.ltx
- M gamedata/config/scripts/labx10/x10_hard_disk.ltx
- M gamedata/config/scripts/labx10/x10_switcher_00.ltx
- M gamedata/config/scripts/labx10/x10_switcher_01.ltx
- M gamedata/config/scripts/labx10/x10_switcher_02.ltx
- M gamedata/config/scripts/labx10/x10_switcher_03.ltx
- M gamedata/config/scripts/labx10/x10_switcher_04.ltx
- M gamedata/config/scripts/labx10/x10_switcher_05.ltx
- M gamedata/config/scripts/labx10/x10_switcher_06.ltx
- M gamedata/config/scripts/labx10/x10_switcher_07.ltx
- M gamedata/config/scripts/labx10/x10_switcher_08.ltx
- M gamedata/config/scripts/labx10/x10_switcher_09.ltx
- M gamedata/config/scripts/labx10/x10_switcher_10.ltx
- M gamedata/config/scripts/labx10/x10_switcher_11.ltx
- M gamedata/config/scripts/labx10/x10_switcher_12.ltx
- M gamedata/config/scripts/labx10/x10_switcher_13.ltx
- M gamedata/config/scripts/labx12/bun_codedoor4.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_door_0000.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_door_0003.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_door_0005.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_door_0008.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_key_0000.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_key_0002.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_key_0004.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_key_0005.ltx
- M gamedata/config/scripts/labx12/bun_codedoor_key_0008.ltx
- M gamedata/config/scripts/labx12/bun_deactivate_radar.ltx
- M gamedata/config/scripts/labx12/bun_komp_blok_secr.ltx
- M gamedata/config/scripts/labx12/bun_komp_klava_secr.ltx
- M gamedata/config/scripts/labx12/bun_primary_switcher_0000.ltx
- M gamedata/config/scripts/labx12/bun_pult.ltx
- M gamedata/config/scripts/labx12/bun_pult_rychag.ltx
- M gamedata/config/scripts/labx12/bun_spec_monolith_01.ltx
- M gamedata/config/scripts/labx16/brain_system_message.ltx
- M gamedata/config/scripts/labx16/x16_codedoor1.ltx
- M gamedata/config/scripts/labx16/x16_door_level_changer.ltx
- M gamedata/config/scripts/labx16/x16_locked_door_x18.ltx
- M gamedata/config/scripts/labx16/x16_switcher_01.ltx
- M gamedata/config/scripts/labx16/x16_switcher_02.ltx
- M gamedata/config/scripts/labx16/x16_switcher_03.ltx
- M gamedata/config/scripts/labx16/x16_switcher_04.ltx
- M gamedata/config/scripts/labx16/x16_switcher_05.ltx
- M gamedata/config/scripts/labx16/x16_switcher_06.ltx
- M gamedata/config/scripts/labx16/x16_switcher_07.ltx
- M gamedata/config/scripts/labx16/x16_switcher_08.ltx
- M gamedata/config/scripts/labx16/x16_zombies.ltx
- M gamedata/config/scripts/labx16/yan_grate.ltx
- M gamedata/config/scripts/labx16/yan_labx16_switcher1.ltx
- M gamedata/config/scripts/labx16/yan_labx16_switcher2.ltx
- M gamedata/config/scripts/labx16/yan_labx16_switcher3.ltx
- M gamedata/config/scripts/labx16/yan_labx16_switcher_primary.ltx
- M gamedata/config/scripts/labx18/dar_door1.ltx
- M gamedata/config/scripts/labx18/dar_door2.ltx
- M gamedata/config/scripts/labx18/dar_door3.ltx
- M gamedata/config/scripts/labx18/dar_snork_strong.ltx
- M gamedata/config/scripts/labx18/dar_snork_strong_0000.ltx
- M gamedata/config/scripts/labx18/dar_space_restrictor_0001.ltx
- M gamedata/config/scripts/labx18/komp_blok_0000.ltx
- M gamedata/config/scripts/labx18/komp_blok_0001.ltx
- M gamedata/config/scripts/labx18/komp_blok_0002.ltx
- M gamedata/config/scripts/labx18/second_door_marker.ltx
- M gamedata/config/scripts/labx18/x18_X16_lev_chang_door.ltx
- M gamedata/config/scripts/labx18/x18_attantion.ltx
- M gamedata/config/scripts/labx18/x18_codedoor1.ltx
- M gamedata/config/scripts/labx18/x18_codedoor2.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_0003.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_control_room.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_control_room_key_out.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_key_0000.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_key_0002.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_key_0007.ltx
- M gamedata/config/scripts/labx18/x18_codedoor_key_control_room_in.ltx
- M gamedata/config/scripts/labx18/x18_codelock1.ltx
- M gamedata/config/scripts/labx18/x18_codelock12.ltx
- M gamedata/config/scripts/labx18/x18_codelock2.ltx
- M gamedata/config/scripts/labx18/x18_door3_open_zone.ltx
- M gamedata/config/scripts/labx18/x18_freezer.ltx
- M gamedata/config/scripts/labx18/x18_physic_destroyable_object_0003.ltx
- M gamedata/config/scripts/labx18/x18_physic_object.ltx
- M gamedata/config/scripts/labx18/x18_physic_object0031.ltx
- M gamedata/config/scripts/labx18/x18_poltergeist_door_closer.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0000.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0001.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0002.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0003.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0004.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0005.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0006.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0007.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0008.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0009.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0010.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0011.ltx
- M gamedata/config/scripts/labx18/x18_sound_restrictor_0012.ltx
- M gamedata/config/scripts/labx18/x18_spec_hran.ltx
- M gamedata/config/scripts/labx18/x18_switcher_01.ltx
- M gamedata/config/scripts/labx18/x18_switcher_02.ltx
- M gamedata/config/scripts/labx18/x18_switcher_03.ltx
- M gamedata/config/scripts/labx18/x18_switcher_04.ltx
- M gamedata/config/scripts/labx18/x18_switcher_05.ltx
- M gamedata/config/scripts/labx18/x18_switcher_06.ltx
- M gamedata/config/scripts/labx18/x18_switcher_07.ltx
- M gamedata/config/scripts/labx18/x18_switcher_08.ltx
- M gamedata/config/scripts/labx18/x18_switcher_09.ltx
- M gamedata/config/scripts/labx8/door_metal_large.ltx
- M gamedata/config/scripts/labx8/labx8_codedoor_01.ltx
- M gamedata/config/scripts/labx8/labx8_codedoor_key_01.ltx
- M gamedata/config/scripts/labx8/labx8_energopult_0001.ltx
- M gamedata/config/scripts/labx8/labx8_energopult_0002.ltx
- M gamedata/config/scripts/labx8/labx8_energopult_0003.ltx
- M gamedata/config/scripts/labx8/labx8_energopult_0004.ltx
- M gamedata/config/scripts/labx8/labx8_energopult_0005.ltx
- M gamedata/config/scripts/labx8/labx8_lamp_off_restrictor.ltx
- M gamedata/config/scripts/labx8/labx8_physic_object_0006.ltx
- M gamedata/config/scripts/labx8/labx8_rubilnik_rychag.ltx
- M gamedata/config/scripts/labx8/labx8_rubilnik_x12.ltx
- M gamedata/config/scripts/labx8/lx8_button.ltx
- M gamedata/config/scripts/labx8/lx8_lift_button.ltx
- M gamedata/config/scripts/labx8/lx8_rainbow_tips.ltx
- M gamedata/config/scripts/labx8/x8_codedoor1.ltx
- M gamedata/config/scripts/labx8/x8_codelock1.ltx
- M gamedata/config/scripts/labx8/x8_switcher_01.ltx
- M gamedata/config/scripts/labx8/x8_switcher_02.ltx
- M gamedata/config/scripts/labx8/x8_switcher_03.ltx
- M gamedata/config/scripts/labx8/x8_switcher_04.ltx
- M gamedata/config/scripts/les_umuvalnik.ltx
- M gamedata/config/scripts/lim/lim_codedoor_key_0001.ltx
- M gamedata/config/scripts/lim/lim_door.ltx
- M gamedata/config/scripts/lim/lim_electro_fence_switcher.ltx
- M gamedata/config/scripts/lim/lim_electro_pult.ltx
- M gamedata/config/scripts/lim/lim_guests_from_future.ltx
- M gamedata/config/scripts/lim/lim_mih_guard.ltx
- M gamedata/config/scripts/lim/lim_monstrs_enter.ltx
- M gamedata/config/scripts/lim/lim_podval_zone.ltx
- M gamedata/config/scripts/lim/lim_portal.ltx
- M gamedata/config/scripts/lim/lim_radio.ltx
- M gamedata/config/scripts/lim/lim_rashpil.ltx
- M gamedata/config/scripts/lim/lim_sleep_matras.ltx
- M gamedata/config/scripts/lim/lim_tiski_repair.ltx
- M gamedata/config/scripts/lim/lim_trader.ltx
- M gamedata/config/scripts/lim/lim_trader_zone.ltx
- M gamedata/config/scripts/lim/lim_umuvalnik_01.ltx
- M gamedata/config/scripts/lim/pesh_prison_door.ltx
- M gamedata/config/scripts/lim/speakerphon_lim_01.ltx
- M gamedata/config/scripts/lost/lost_esc_sleep_matras.ltx
- M gamedata/config/scripts/lost/lost_mikhas.ltx
- M gamedata/config/scripts/lost/lost_military_01.ltx
- M gamedata/config/scripts/lost/lost_military_02.ltx
- M gamedata/config/scripts/lost/sanek.ltx
- M gamedata/config/scripts/mar/mar__tiski_repair.ltx
- M gamedata/config/scripts/mar/mar_attantion.ltx
- M gamedata/config/scripts/mar/mar_baza_oksana_zone.ltx
- M gamedata/config/scripts/mar/mar_boryan.ltx
- M gamedata/config/scripts/mar/mar_cook.ltx
- M gamedata/config/scripts/mar/mar_ecolog_hutor.ltx
- M gamedata/config/scripts/mar/mar_esc_oksana_zone.ltx
- M gamedata/config/scripts/mar/mar_esc_sleep_matras.ltx
- M gamedata/config/scripts/mar/mar_factory_zone_1.ltx
- M gamedata/config/scripts/mar/mar_factory_zone_2.ltx
- M gamedata/config/scripts/mar/mar_factory_zone_3.ltx
- M gamedata/config/scripts/mar/mar_factory_zone_main.ltx
- M gamedata/config/scripts/mar/mar_gigant_zone.ltx
- M gamedata/config/scripts/mar/mar_oksana.ltx
- M gamedata/config/scripts/mar/mar_sak_teleport_0002.ltx
- M gamedata/config/scripts/mar/mar_umuvalnik.ltx
- M gamedata/config/scripts/mar/mar_umuvalnik_base.ltx
- M gamedata/config/scripts/mar/mar_xod_baze_0004.ltx
- M gamedata/config/scripts/mar/mar_xod_outbaze.ltx
- M gamedata/config/scripts/mar/mar_zone_kill_streck.ltx
- M gamedata/config/scripts/mar/marsh_botan1.ltx
- M gamedata/config/scripts/mar/marsh_priemnik.ltx
- M gamedata/config/scripts/mar/marsh_trader_meet_zone.ltx
- M gamedata/config/scripts/mar/pahom.ltx
- M gamedata/config/scripts/mar/sanek2.ltx
- M gamedata/config/scripts/mar/shersh.ltx
- M gamedata/config/scripts/marsh/fake_gluzov.ltx
- M gamedata/config/scripts/marsh/fracture_marsh.ltx
- M gamedata/config/scripts/marsh/fracture_marsh_ohota.ltx
- M gamedata/config/scripts/marsh/mar_attantion.ltx
- M gamedata/config/scripts/marsh/mar_boryan.ltx
- M gamedata/config/scripts/marsh/mar_eco_spawn_restr.ltx
- M gamedata/config/scripts/marsh/mar_esc_oksana_zone.ltx
- M gamedata/config/scripts/marsh/mar_factory_zone_1.ltx
- M gamedata/config/scripts/marsh/mar_factory_zone_2.ltx
- M gamedata/config/scripts/marsh/mar_factory_zone_3.ltx
- M gamedata/config/scripts/marsh/mar_factory_zone_main.ltx
- M gamedata/config/scripts/marsh/mar_gigant_look_zone.ltx
- M gamedata/config/scripts/marsh/mar_umuvalnik.ltx
- M gamedata/config/scripts/marsh/mar_umuvalnik_base.ltx
- M gamedata/config/scripts/marsh/marsh_botan1.ltx
- M gamedata/config/scripts/marsh/marsh_controller1.ltx
- M gamedata/config/scripts/marsh/marsh_controller2.ltx
- M gamedata/config/scripts/marsh/marsh_controller3.ltx
- M gamedata/config/scripts/marsh/marsh_cook.ltx
- M gamedata/config/scripts/marsh/marsh_crash_vertolet.ltx
- M gamedata/config/scripts/marsh/marsh_gigant.ltx
- M gamedata/config/scripts/marsh/marsh_muller.ltx
- M gamedata/config/scripts/marsh/marsh_oksana.ltx
- M gamedata/config/scripts/marsh/marsh_trader_meet_zone.ltx
- M gamedata/config/scripts/marsh/pahom.ltx
- M gamedata/config/scripts/marsh/sanek2.ltx
- M gamedata/config/scripts/marsh/shersh.ltx
- M gamedata/config/scripts/mil/bag_mil_01.ltx
- M gamedata/config/scripts/mil/door_metal_small_mil.ltx
- M gamedata/config/scripts/mil/gross_merc.ltx
- M gamedata/config/scripts/mil/krazy_mil.ltx
- M gamedata/config/scripts/mil/matugalnik.ltx
- M gamedata/config/scripts/mil/mil_arsenal_door.ltx
- M gamedata/config/scripts/mil/mil_barjer_monsters.ltx
- M gamedata/config/scripts/mil/mil_before_blockpost_restriktor.ltx
- M gamedata/config/scripts/mil/mil_block_teleports.ltx
- M gamedata/config/scripts/mil/mil_codedoor1.ltx
- M gamedata/config/scripts/mil/mil_codelock1.ltx
- M gamedata/config/scripts/mil/mil_controller.ltx
- M gamedata/config/scripts/mil/mil_cook_restrictor.ltx
- M gamedata/config/scripts/mil/mil_door_trader_box_0004.ltx
- M gamedata/config/scripts/mil/mil_dying.ltx
- M gamedata/config/scripts/mil/mil_freedom_crazy.ltx
- M gamedata/config/scripts/mil/mil_freedom_gulag_defense_res.ltx
- M gamedata/config/scripts/mil/mil_freedom_restrictor.ltx
- M gamedata/config/scripts/mil/mil_freedom_wall_restrictor.ltx
- M gamedata/config/scripts/mil/mil_freezer.ltx
- M gamedata/config/scripts/mil/mil_matugalnik.ltx
- M gamedata/config/scripts/mil/mil_matveev_zone.ltx
- M gamedata/config/scripts/mil/mil_merc1.ltx
- M gamedata/config/scripts/mil/mil_merc2.ltx
- M gamedata/config/scripts/mil/mil_mines_restrictor.ltx
- M gamedata/config/scripts/mil/mil_minigame_zone.ltx
- M gamedata/config/scripts/mil/mil_minigamer.ltx
- M gamedata/config/scripts/mil/mil_povar_door.ltx
- M gamedata/config/scripts/mil/mil_rush_starter.ltx
- M gamedata/config/scripts/mil/mil_ryaboi.ltx
- M gamedata/config/scripts/mil/mil_sak_teleport_0002.ltx
- M gamedata/config/scripts/mil/mil_sniper_kill_zone.ltx
- M gamedata/config/scripts/mil/mil_space_restrictor.ltx
- M gamedata/config/scripts/mil/mil_space_restrictor_0000.ltx
- M gamedata/config/scripts/mil/mil_spec_hran.ltx
- M gamedata/config/scripts/mil/mil_stalker_0004.ltx
- M gamedata/config/scripts/mil/mil_tiski_repair_01.ltx
- M gamedata/config/scripts/mil/mil_tiski_repair_02.ltx
- M gamedata/config/scripts/mil/mil_tiski_repair_03.ltx
- M gamedata/config/scripts/mil/mil_tiski_repair_05.ltx
- M gamedata/config/scripts/mil/mil_trader_zone.ltx
- M gamedata/config/scripts/mil/mil_umuvalnik.ltx
- M gamedata/config/scripts/mil/mil_umuvalnik_chainik.ltx
- M gamedata/config/scripts/mil/mil_vasilev.ltx
- M gamedata/config/scripts/mil/mil_zabor.ltx
- M gamedata/config/scripts/mil/store_lukash_seif.ltx
- M gamedata/config/scripts/minigame_inventory_box.ltx
- M gamedata/config/scripts/minigame_inventory_box2.ltx
- M gamedata/config/scripts/monsters/zombie_fc.ltx
- M gamedata/config/scripts/pri/bag_prip_01.ltx
- M gamedata/config/scripts/pri/door_metal_small_sak.ltx
- M gamedata/config/scripts/pri/door_wood_small_prip.ltx
- M gamedata/config/scripts/pri/pri_hotel_door.ltx
- M gamedata/config/scripts/pri/pri_stalker_n_2.ltx
- M gamedata/config/scripts/pri/pri_stalker_n_2_0000.ltx
- M gamedata/config/scripts/pri/prip_floor_door_01.ltx
- M gamedata/config/scripts/pri/prip_floor_door_02.ltx
- M gamedata/config/scripts/pri/prip_floor_door_03.ltx
- M gamedata/config/scripts/pri/prip_floor_door_04.ltx
- M gamedata/config/scripts/pri/prip_sak_teleport_0002.ltx
- M gamedata/config/scripts/pri/prip_teleport_0001.ltx
- M gamedata/config/scripts/pri/prip_teleport_0002.ltx
- M gamedata/config/scripts/pri/prip_teleport_0003.ltx
- M gamedata/config/scripts/pri/prip_teleport_0004.ltx
- M gamedata/config/scripts/pri/sak_trader_zone.ltx
- M gamedata/config/scripts/pri/store_ivar_seif.ltx
- M gamedata/config/scripts/proba.ltx
- M gamedata/config/scripts/rad/logic_vergas.ltx
- M gamedata/config/scripts/rad/new_killer_exo.ltx
- M gamedata/config/scripts/rad/new_killer_general1.ltx
- M gamedata/config/scripts/rad/new_killer_general2.ltx
- M gamedata/config/scripts/rad/new_killer_general3.ltx
- M gamedata/config/scripts/rad/new_killer_general4.ltx
- M gamedata/config/scripts/rad/new_killer_general5.ltx
- M gamedata/config/scripts/rad/new_killer_leader.ltx
- M gamedata/config/scripts/rad/new_killer_veteran1.ltx
- M gamedata/config/scripts/rad/new_killer_veteran2.ltx
- M gamedata/config/scripts/rad/new_killer_veteran3.ltx
- M gamedata/config/scripts/rad/new_killer_veteran4.ltx
- M gamedata/config/scripts/rad/new_killer_veteran5.ltx
- M gamedata/config/scripts/rad/new_killer_veteran6.ltx
- M gamedata/config/scripts/rad/new_killer_x12.ltx
- M gamedata/config/scripts/rad/rad2_antennas_found.ltx
- M gamedata/config/scripts/rad/rad2_stalker_zombied.ltx
- M gamedata/config/scripts/rad/rad2_stalker_zombied_0000.ltx
- M gamedata/config/scripts/rad/rad2_stalker_zombied_0001.ltx
- M gamedata/config/scripts/rad/rad_add_monsters_1_zone.ltx
- M gamedata/config/scripts/rad/rad_antenna_particle2.ltx
- M gamedata/config/scripts/rad/rad_antenna_particle3.ltx
- M gamedata/config/scripts/rad/rad_antenna_particle4.ltx
- M gamedata/config/scripts/rad/rad_antenna_particle_1.ltx
- M gamedata/config/scripts/rad/rad_antenna_rays_and_spot.ltx
- M gamedata/config/scripts/rad/rad_burer_hos1.ltx
- M gamedata/config/scripts/rad/rad_burer_strong2.ltx
- M gamedata/config/scripts/rad/rad_code_door.ltx
- M gamedata/config/scripts/rad/rad_codedoor_key_01.ltx
- M gamedata/config/scripts/rad/rad_codedoor_key_x7.ltx
- M gamedata/config/scripts/rad/rad_controller.ltx
- M gamedata/config/scripts/rad/rad_curve_road_high_guard1.ltx
- M gamedata/config/scripts/rad/rad_curve_road_high_guard2.ltx
- M gamedata/config/scripts/rad/rad_curve_road_low_guard.ltx
- M gamedata/config/scripts/rad/rad_curve_road_mid_guard1.ltx
- M gamedata/config/scripts/rad/rad_curve_road_mid_guard2.ltx
- M gamedata/config/scripts/rad/rad_door_trader_box_0000.ltx
- M gamedata/config/scripts/rad/rad_door_trader_box_0001.ltx
- M gamedata/config/scripts/rad/rad_door_trader_box_0003.ltx
- M gamedata/config/scripts/rad/rad_ivar_drug.ltx
- M gamedata/config/scripts/rad/rad_mil_level_changer_teleport.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0000.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0001.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0002.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0003.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0004.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0005.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0006.ltx
- M gamedata/config/scripts/rad/rad_physic_destroyable_object_0007.ltx
- M gamedata/config/scripts/rad/rad_pribor1.ltx
- M gamedata/config/scripts/rad/rad_pribor2.ltx
- M gamedata/config/scripts/rad/rad_pribor3.ltx
- M gamedata/config/scripts/rad/rad_prohod_restr.ltx
- M gamedata/config/scripts/rad/rad_psy_dog.ltx
- M gamedata/config/scripts/rad/rad_shkafdoor_0008.ltx
- M gamedata/config/scripts/rad/rad_shkafdoor_0009.ltx
- M gamedata/config/scripts/rad/rad_stalker_zombied_0010.ltx
- M gamedata/config/scripts/rad/rad_stalker_zombied_0011.ltx
- M gamedata/config/scripts/rad/rad_start_ambush_before_tramplin.ltx
- M gamedata/config/scripts/rad/rad_stol_6_ph_door_0000.ltx
- M gamedata/config/scripts/rad/rad_stol_6_ph_door_0002.ltx
- M gamedata/config/scripts/rad/rad_stol_8a_ph_door_0001.ltx
- M gamedata/config/scripts/rad/rad_tiski_repair.ltx
- M gamedata/config/scripts/rad/rad_tramplin_glavn.ltx
- M gamedata/config/scripts/rad/rad_tramplin_rpg_walker.ltx
- M gamedata/config/scripts/rad/rad_tramplin_sniper.ltx
- M gamedata/config/scripts/rad/rad_vergas.ltx
- M gamedata/config/scripts/rad/rad_wagon_restrictor.ltx
- M gamedata/config/scripts/rad/sak_radio.ltx
- M gamedata/config/scripts/rad/sak_trader_zone.ltx
- M gamedata/config/scripts/rad/stol_7_ph_door_0000.ltx
- M gamedata/config/scripts/rad/stol_8_ph_door_0000.ltx
- M gamedata/config/scripts/red/box_wooden_01_door_0000.ltx
- M gamedata/config/scripts/red/box_wooden_01_door_0001.ltx
- M gamedata/config/scripts/red/bridge_rest_zone.ltx
- M gamedata/config/scripts/red/door_red_forest_01.ltx
- M gamedata/config/scripts/red/red_add_monsters_1_zone.ltx
- M gamedata/config/scripts/red/red_basa_door.ltx
- M gamedata/config/scripts/red/red_bridge_door_bandit_l.ltx
- M gamedata/config/scripts/red/red_codedoor.ltx
- M gamedata/config/scripts/red/red_codedoor_key_0001.ltx
- M gamedata/config/scripts/red/red_forest_gate.ltx
- M gamedata/config/scripts/red/red_forest_gate2.ltx
- M gamedata/config/scripts/red/red_forest_lesnik.ltx
- M gamedata/config/scripts/red/red_forest_stalker_08.ltx
- M gamedata/config/scripts/red/red_forester_door.ltx
- M gamedata/config/scripts/red/red_freedom_glavn.ltx
- M gamedata/config/scripts/red/red_krest_01.ltx
- M gamedata/config/scripts/red/red_lim_kill_zone.ltx
- M gamedata/config/scripts/red/red_near_bridge_priemnik.ltx
- M gamedata/config/scripts/red/red_neitral_bandit_zone.ltx
- M gamedata/config/scripts/red/red_sak_teleport_0000.ltx
- M gamedata/config/scripts/red/red_scaut.ltx
- M gamedata/config/scripts/red/red_shtoln_inn.ltx
- M gamedata/config/scripts/red/red_smart_sos_die.ltx
- M gamedata/config/scripts/red/red_zone_kill_streck_02.ltx
- M gamedata/config/scripts/red/shelf_metal_160x200_door_r_0000.ltx
- M gamedata/config/scripts/red_forest/door_red_forest_01.ltx
- M gamedata/config/scripts/red_forest/red_add_monsters_1_zone.ltx
- M gamedata/config/scripts/red_forest/red_bloodsucker1.ltx
- M gamedata/config/scripts/red_forest/red_bloodsucker2.ltx
- M gamedata/config/scripts/red_forest/red_bridge_door_bandit_l.ltx
- M gamedata/config/scripts/red_forest/red_codedoor.ltx
- M gamedata/config/scripts/red_forest/red_codedoor_key_0001.ltx
- M gamedata/config/scripts/red_forest/red_forest_gate.ltx
- M gamedata/config/scripts/red_forest/red_forest_gate2.ltx
- M gamedata/config/scripts/red_forest/red_forest_lesnik.ltx
- M gamedata/config/scripts/red_forest/red_forester_door.ltx
- M gamedata/config/scripts/red_forest/red_freedom_glavn.ltx
- M gamedata/config/scripts/red_forest/red_krest_01.ltx
- M gamedata/config/scripts/red_forest/red_lim_kill_zone.ltx
- M gamedata/config/scripts/red_forest/red_sak_teleport_0000.ltx
- M gamedata/config/scripts/red_forest/red_scaut.ltx
- M gamedata/config/scripts/red_forest/red_zone_kill_streck_02.ltx
- M gamedata/config/scripts/red_forest/shelf_metal_160x200_door_l_0000.ltx
- M gamedata/config/scripts/red_forest/shelf_metal_160x200_door_r_0000.ltx
- M gamedata/config/scripts/ros/bar_ambush_killer_03.ltx
- M gamedata/config/scripts/ros/bar_ecolog_crush_1.ltx
- M gamedata/config/scripts/ros/bar_ecolog_crush_2.ltx
- M gamedata/config/scripts/ros/bar_ecolog_crush_3.ltx
- M gamedata/config/scripts/ros/dt_mns_restr.ltx
- M gamedata/config/scripts/ros/ros2_add_monsters.ltx
- M gamedata/config/scripts/ros/ros3_add_monsters.ltx
- M gamedata/config/scripts/ros/ros_add_monsters.ltx
- M gamedata/config/scripts/ros/ros_block_ground_zone_01.ltx
- M gamedata/config/scripts/ros/ros_block_roof_zone.ltx
- M gamedata/config/scripts/ros/ros_ecolog_kruglov.ltx
- M gamedata/config/scripts/ros/ros_heli.ltx
- M gamedata/config/scripts/ros/ros_sak_teleport_0001.ltx
- M gamedata/config/scripts/ros/ros_sak_teleport_0002.ltx
- M gamedata/config/scripts/ros/ros_sak_teleport_0003.ltx
- M gamedata/config/scripts/ros/ros_sak_teleport_0004.ltx
- M gamedata/config/scripts/ros/ros_space_restrictor.ltx
- M gamedata/config/scripts/ros/ros_space_restrictor_0001.ltx
- M gamedata/config/scripts/ros/ros_stalker_0001.ltx
- M gamedata/config/scripts/ros/ros_stalker_0002.ltx
- M gamedata/config/scripts/ros/ros_stalker_0003.ltx
- M gamedata/config/scripts/ros/ros_stalker_0004.ltx
- M gamedata/config/scripts/ros/ros_stalker_zombied.ltx
- M gamedata/config/scripts/ros/ros_stalker_zombied_0000.ltx
- M gamedata/config/scripts/ros/ros_stalker_zombied_0001.ltx
- M gamedata/config/scripts/ros/ros_stalker_zombied_0003.ltx
- M gamedata/config/scripts/ros/ros_stalker_zombied_0004.ltx
- M gamedata/config/scripts/sar_mon/control_monolith_02.ltx
- M gamedata/config/scripts/sar_mon/mon_big_fire_res_0001.ltx
- M gamedata/config/scripts/sar_mon/mon_gas_rychag_01.ltx
- M gamedata/config/scripts/sar_mon/mon_gas_rychag_02.ltx
- M gamedata/config/scripts/sar_mon/mon_tupik_zone.ltx
- M gamedata/config/scripts/sar_mon/sar_monolith_gen.ltx
- M gamedata/config/scripts/sar_mon/sar_monolith_gen_main.ltx
- M gamedata/config/scripts/sarc/sar_door.ltx
- M gamedata/config/scripts/sarc/sar_in_rest.ltx
- M gamedata/config/scripts/sarc/sar_monolith.ltx
- M gamedata/config/scripts/sarc/sar_monolith_call.ltx
- M gamedata/config/scripts/sarc/sar_monolith_noise.ltx
- M gamedata/config/scripts/sarc/sar_small_door.ltx
- M gamedata/config/scripts/sarc/sar_stalker_monolith_high.ltx
- M gamedata/config/scripts/sarc/sar_stalker_monolith_high_2.ltx
- M gamedata/config/scripts/sarc/sar_stalker_monolith_high_3.ltx
- M gamedata/config/scripts/sarc/sar_teleport_sphere_0000.ltx
- M gamedata/config/scripts/sarc/sar_trader_door.ltx
- M gamedata/config/scripts/sarcofag/sar_door.ltx
- M gamedata/config/scripts/sarcofag/sar_monolith.ltx
- M gamedata/config/scripts/sarcofag/sar_monolith_noise.ltx
- M gamedata/config/scripts/sarcofag/sar_teleport_sphere_0000.ltx
- M gamedata/config/scripts/self_inv_box.ltx
- M gamedata/config/scripts/self_rep_box.ltx
- M gamedata/config/scripts/sincos.ltx
- M gamedata/config/scripts/sr_agro_undrgrnd.ltx
- M gamedata/config/scripts/st1/aes_space_restrictor_eartquake.ltx
- M gamedata/config/scripts/st1/stancia_perehod_door.ltx
- M gamedata/config/scripts/st1/stancia_perehod_rychag.ltx
- M gamedata/config/scripts/st1/stancia_stena_door.ltx
- M gamedata/config/scripts/st2/st2_perehod_door.ltx
- M gamedata/config/scripts/st2/st2_perehod_rychag.ltx
- M gamedata/config/scripts/st2/st2_space_restrictor_eartquake.ltx
- M gamedata/config/scripts/st2/stancia_1_space_restrictor.ltx
- M gamedata/config/scripts/st2/stancia_2_space_restrictor.ltx
- M gamedata/config/scripts/st2/stancia_3_space_restrictor.ltx
- M gamedata/config/scripts/st2/stancia_4_space_restrictor.ltx
- M gamedata/config/scripts/st2/stancia_5_space_restrictor.ltx
- M gamedata/config/scripts/st2/stancia_6_space_restrictor.ltx
- M gamedata/config/scripts/stan1/aes_space_restrictor_eartquake.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_01.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_02.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_03.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_04.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_05.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_06.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_07.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_08.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_09.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_10.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_11.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_12.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_13.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_14.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_15.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_16.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_17.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_18.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_19.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_20.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_21.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_22.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_23.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_24.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_25.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_26.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_27.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_28.ltx
- M gamedata/config/scripts/stan1/aes_spec_monolith_29.ltx
- M gamedata/config/scripts/stan1/stancia_perehod_rychag.ltx
- M gamedata/config/scripts/stan2/st2_esc_sleep_matras.ltx
- M gamedata/config/scripts/stan2/st2_mike_space_restrictor.ltx
- M gamedata/config/scripts/stan2/st2_perehod_door.ltx
- M gamedata/config/scripts/stan2/st2_perehod_rychag.ltx
- M gamedata/config/scripts/stan2/st2_space_restrictor_eartquake.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_01.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_02.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_03.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_04.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_05.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_06.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_07.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_08.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_09.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_10.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_11.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_12.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_13.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_14.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_15.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_16.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_17.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_18.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_19.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_20.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_21.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_22.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_23.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_24.ltx
- M gamedata/config/scripts/stan2/st2_spec_monolith_25.ltx
- M gamedata/config/scripts/stan2/st2_tiski_repair.ltx
- M gamedata/config/scripts/store_dolg_seif.ltx
- M gamedata/config/scripts/store_inventory_box.ltx
- M gamedata/config/scripts/store_ivar_seif.ltx
- M gamedata/config/scripts/store_jila_seif.ltx
- M gamedata/config/scripts/store_lukash_seif.ltx
- M gamedata/config/scripts/store_my_seif.ltx
- M gamedata/config/scripts/store_stas_seif.ltx
- M gamedata/config/scripts/stories.ltx
- M gamedata/config/scripts/teleport.ltx
- M gamedata/config/scripts/tiski.ltx
- M gamedata/config/scripts/treasure_inventory_box.ltx
- M gamedata/config/scripts/val/amk_mini_bar_voron.ltx
- M gamedata/config/scripts/val/borov_radio.ltx
- M gamedata/config/scripts/val/dark_trader_zone.ltx
- M gamedata/config/scripts/val/door_door_01_0006.ltx
- M gamedata/config/scripts/val/door_metal_small_jila.ltx
- M gamedata/config/scripts/val/dv_door_1.ltx
- M gamedata/config/scripts/val/dv_esc_sleep_matras.ltx
- M gamedata/config/scripts/val/metal_small.ltx
- M gamedata/config/scripts/val/small_seif_door.ltx
- M gamedata/config/scripts/val/store_jila_seif.ltx
- M gamedata/config/scripts/val/val2_add_monsters.ltx
- M gamedata/config/scripts/val/val2_monsters_zone.ltx
- M gamedata/config/scripts/val/val_actor_behind_gate_zone.ltx
- M gamedata/config/scripts/val/val_add_monsters.ltx
- M gamedata/config/scripts/val/val_bandit_trader.ltx
- M gamedata/config/scripts/val/val_bandits_enemy_zone.ltx
- M gamedata/config/scripts/val/val_bloodsucker1.ltx
- M gamedata/config/scripts/val/val_bloodsucker2.ltx
- M gamedata/config/scripts/val/val_bloodsucker3.ltx
- M gamedata/config/scripts/val/val_borov_vertolet_zone.ltx
- M gamedata/config/scripts/val/val_bs_killer1.ltx
- M gamedata/config/scripts/val/val_bs_killer2.ltx
- M gamedata/config/scripts/val/val_bs_victim.ltx
- M gamedata/config/scripts/val/val_chase_heli.ltx
- M gamedata/config/scripts/val/val_chase_start_zone.ltx
- M gamedata/config/scripts/val/val_door_nedostroyka.ltx
- M gamedata/config/scripts/val/val_dram_dialog_zone.ltx
- M gamedata/config/scripts/val/val_dram_start_zone.ltx
- M gamedata/config/scripts/val/val_escort_ambush_zone.ltx
- M gamedata/config/scripts/val/val_escort_captive.ltx
- M gamedata/config/scripts/val/val_escort_nap1.ltx
- M gamedata/config/scripts/val/val_escort_start_zone.ltx
- M gamedata/config/scripts/val/val_exit_gate.ltx
- M gamedata/config/scripts/val/val_flame_polter1.ltx
- M gamedata/config/scripts/val/val_flame_polter2.ltx
- M gamedata/config/scripts/val/val_lager_bandits_leader_company_restr.ltx
- M gamedata/config/scripts/val/val_lager_bandits_nedostroyka_company_restr.ltx
- M gamedata/config/scripts/val/val_lager_bandits_restrictor.ltx
- M gamedata/config/scripts/val/val_prisoner_button.ltx
- M gamedata/config/scripts/val/val_prisoner_cage_zone.ltx
- M gamedata/config/scripts/val/val_prisoner_captive.ltx
- M gamedata/config/scripts/val/val_prisoner_jail_door.ltx
- M gamedata/config/scripts/val/val_raid_go1_zone.ltx
- M gamedata/config/scripts/val/val_raid_go2_zone.ltx
- M gamedata/config/scripts/val/val_raid_go3_zone.ltx
- M gamedata/config/scripts/val/val_reshetka_baza.ltx
- M gamedata/config/scripts/val/val_reshetka_truba.ltx
- M gamedata/config/scripts/val/val_sacrifice_danger_zone.ltx
- M gamedata/config/scripts/val/val_sacrifice_dialog_start_zone.ltx
- M gamedata/config/scripts/val/val_sacrifice_guard1.ltx
- M gamedata/config/scripts/val/val_sacrifice_guard2.ltx
- M gamedata/config/scripts/val/val_sos_bs_zone.ltx
- M gamedata/config/scripts/val/val_space_restrictor.ltx
- M gamedata/config/scripts/val/val_stalker.ltx
- M gamedata/config/scripts/val/val_stalker_0000.ltx
- M gamedata/config/scripts/val/val_stena_podval.ltx
- M gamedata/config/scripts/val/val_strelok_door.ltx
- M gamedata/config/scripts/val/val_tiski_repair.ltx
- M gamedata/config/scripts/val/val_voron.ltx
- M gamedata/config/scripts/val/val_x18_exit_restr.ltx
- M gamedata/config/scripts/val/val_zaval_x18_zone.ltx
- M gamedata/config/scripts/val/voron_transiver.ltx
- M gamedata/config/scripts/warlab/war_codedoor_0001.ltx
- M gamedata/config/scripts/warlab/war_codedoor_key_0000.ltx
- M gamedata/config/scripts/warlab/war_inventory_box_0008.ltx
- M gamedata/config/scripts/warlab/war_rubilnik_rychag.ltx
- M gamedata/config/scripts/warlab/war_rubilnik_x12.ltx
- M gamedata/config/scripts/warlab/war_spec_hran.ltx
- M gamedata/config/scripts/warlab/war_switcher_0001.ltx
- M gamedata/config/scripts/warlab/war_switcher_0002.ltx
- M gamedata/config/scripts/warlab/war_zone_sleep.ltx
- M gamedata/config/scripts/yan/assistant.ltx
- M gamedata/config/scripts/yan/assistant2.ltx
- M gamedata/config/scripts/yan/door_trader_0000.ltx
- M gamedata/config/scripts/yan/factory_m_snork_e_0000.ltx
- M gamedata/config/scripts/yan/factory_m_snork_e_0001.ltx
- M gamedata/config/scripts/yan/factory_m_snork_e_0002.ltx
- M gamedata/config/scripts/yan/factory_m_snork_e_0004.ltx
- M gamedata/config/scripts/yan/factory_m_snork_e_0005.ltx
- M gamedata/config/scripts/yan/factory_m_snork_e_0013.ltx
- M gamedata/config/scripts/yan/he_yan_bunker_r1_switcher.ltx
- M gamedata/config/scripts/yan/he_yan_bunker_r2_switcher.ltx
- M gamedata/config/scripts/yan/he_yan_bunker_r3_switcher.ltx
- M gamedata/config/scripts/yan/he_yan_ecolog_r_01.ltx
- M gamedata/config/scripts/yan/he_yan_ecolog_r_02.ltx
- M gamedata/config/scripts/yan/logic_prizrak.ltx
- M gamedata/config/scripts/yan/speakerphon_yan_01.ltx
- M gamedata/config/scripts/yan/speakerphon_yan_02.ltx
- M gamedata/config/scripts/yan/yan_bunker_door_2_zone.ltx
- M gamedata/config/scripts/yan/yan_burer_yan.ltx
- M gamedata/config/scripts/yan/yan_controller.ltx
- M gamedata/config/scripts/yan/yan_door_in_bunker.ltx
- M gamedata/config/scripts/yan/yan_door_trader.ltx
- M gamedata/config/scripts/yan/yan_ecolog1.ltx
- M gamedata/config/scripts/yan/yan_ecolog2.ltx
- M gamedata/config/scripts/yan/yan_ecolog3.ltx
- M gamedata/config/scripts/yan/yan_ecolog_kruglov.ltx
- M gamedata/config/scripts/yan/yan_ecolog_matveev.ltx
- M gamedata/config/scripts/yan/yan_ecolog_saharov.ltx
- M gamedata/config/scripts/yan/yan_ecolog_semenov.ltx
- M gamedata/config/scripts/yan/yan_ecolog_spam_space_restrictor.ltx
- M gamedata/config/scripts/yan/yan_helicopter_1.ltx
- M gamedata/config/scripts/yan/yan_helicopter_2.ltx
- M gamedata/config/scripts/yan/yan_home_snork_0005.ltx
- M gamedata/config/scripts/yan/yan_home_snork_0006.ltx
- M gamedata/config/scripts/yan/yan_home_snork_0009.ltx
- M gamedata/config/scripts/yan/yan_home_snork_0011.ltx
- M gamedata/config/scripts/yan/yan_home_snork_0017.ltx
- M gamedata/config/scripts/yan/yan_home_snork_01.ltx
- M gamedata/config/scripts/yan/yan_izm_spot1.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_0009.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_1_0012.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_1_03.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_2_001.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_4_0001.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_5_0009.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_5_0011.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_6_001.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_7_0007.ltx
- M gamedata/config/scripts/yan/yan_jump_snork_7_0008.ltx
- M gamedata/config/scripts/yan/yan_kill_exit_tonnel.ltx
- M gamedata/config/scripts/yan/yan_many_restrictor.ltx
- M gamedata/config/scripts/yan/yan_prizrak.ltx
- M gamedata/config/scripts/yan/yan_sak_box_0001.ltx
- M gamedata/config/scripts/yan/yan_sak_teleport_0002.ltx
- M gamedata/config/scripts/yan/yan_scientist.ltx
- M gamedata/config/scripts/yan/yan_sleep_restrictor.ltx
- M gamedata/config/scripts/yan/yan_spawn_snork_3.ltx
- M gamedata/config/scripts/yan/yan_tiski_repair.ltx
- M gamedata/config/scripts/yan/yan_ucheniy_talk.ltx
- M gamedata/config/scripts/yan/yan_umuvalnik_01.ltx
- M gamedata/config/scripts/yan/yan_zombied_respawn_05.ltx
- M gamedata/config/scripts/yan/yantar_burer_tunnel.ltx
- M gamedata/config/scripts/yan/yantar_psi_radiotion_30.ltx
- M gamedata/config/scripts/yan/yantar_psi_radiotion_5.ltx
- M gamedata/config/scripts/yan/yantar_psi_radiotion_55.ltx
- M gamedata/config/scripts/yan/yantar_room_switcher.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0000.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0001.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0002.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0003.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0004.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0005.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0006.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0007.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0008.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0009.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0010.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0011.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0012.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0013.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0014.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0015.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0016.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0017.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0018.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0019.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0020.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0021.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0022.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0023.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0024.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0025.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0026.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0027.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0029.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0030.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0031.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0032.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0033.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0043.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0044.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0046.ltx
- M gamedata/config/scripts/yan/yantar_stalker_zombied_0049.ltx
- M gamedata/config/scripts/yan/yantar_switcher_machinery.ltx
- M gamedata/config/scripts/yan/yantar_tunnel_ass.ltx
- M gamedata/config/text/eng/amk_flame_dialogs.xml
- M gamedata/config/text/eng/arc_string_table.xml
- M gamedata/config/text/eng/doc_dialogs.xml
- M gamedata/config/text/eng/he_watch.xml
- M gamedata/config/text/eng/mike_strings.xml
- M gamedata/config/text/eng/sak_strings.xml
- M gamedata/config/text/eng/sak_strings_attention.xml
- M gamedata/config/text/eng/sak_strings_six.xml
- M gamedata/config/text/eng/sak_strings_six2.xml
- M gamedata/config/text/eng/sak_strings_trade.xml
- M gamedata/config/text/eng/sak_strings_treasure.xml
- M gamedata/config/text/eng/script_strings.xml
- M gamedata/config/text/eng/stable_bio_name.xml
- M gamedata/config/text/eng/stable_dialog_manager.xml
- M gamedata/config/text/eng/stable_dialog_manager_uni.xml
- M gamedata/config/text/eng/stable_dialogs.xml
- M gamedata/config/text/eng/stable_dialogs_agroprom.xml
- M gamedata/config/text/eng/stable_dialogs_amk.xml
- M gamedata/config/text/eng/stable_dialogs_bar.xml
- M gamedata/config/text/eng/stable_dialogs_darkvalley.xml
- M gamedata/config/text/eng/stable_dialogs_escape.xml
- M gamedata/config/text/eng/stable_dialogs_garbage.xml
- M gamedata/config/text/eng/stable_dialogs_military.xml
- M gamedata/config/text/eng/stable_dialogs_yantar.xml
- M gamedata/config/text/eng/stable_game_credits.xml
- M gamedata/config/text/eng/stable_generate_fnames.xml
- M gamedata/config/text/eng/stable_generate_snames.xml
- M gamedata/config/text/eng/stable_statistic_caption.xml
- M gamedata/config/text/eng/stable_stories.xml
- M gamedata/config/text/eng/stable_storyline_info_pda.xml
- M gamedata/config/text/eng/stable_task_manager.xml
- M gamedata/config/text/eng/stable_treasure_manager.xml
- M gamedata/config/text/eng/story_dialogs.xml
- M gamedata/config/text/eng/string_actor_bio.xml
- M gamedata/config/text/eng/string_news_mikelik.xml
- M gamedata/config/text/eng/string_table_amk_arts_desc.xml
- M gamedata/config/text/eng/string_table_amk_metka.xml
- M gamedata/config/text/eng/string_table_enc_equipment.xml
- M gamedata/config/text/eng/string_table_enc_mutants.xml
- M gamedata/config/text/eng/string_table_enc_social.xml
- M gamedata/config/text/eng/string_table_enc_vehicles.xml
- M gamedata/config/text/eng/string_table_enc_weapons.xml
- M gamedata/config/text/eng/string_table_enc_zone.xml
- M gamedata/config/text/eng/string_table_general.xml
- M gamedata/config/text/eng/string_table_he_magic_box.xml
- M gamedata/config/text/eng/string_table_includes.xml
- M gamedata/config/text/eng/string_table_items.xml
- M gamedata/config/text/eng/string_table_level_tips.xml
- M gamedata/config/text/eng/string_table_outfit.xml
- M gamedata/config/text/eng/string_table_pda_bandit.xml
- M gamedata/config/text/eng/string_table_pda_dolg.xml
- M gamedata/config/text/eng/string_table_pda_ecolog.xml
- M gamedata/config/text/eng/string_table_pda_freedom.xml
- M gamedata/config/text/eng/string_table_pda_killer.xml
- M gamedata/config/text/eng/string_table_pda_stalker.xml
- M gamedata/config/text/eng/string_table_pda_treasure.xml
- M gamedata/config/text/eng/string_table_tasks_standart.xml
- M gamedata/config/text/eng/string_table_tips.xml
- M gamedata/config/text/eng/string_table_tutorial.xml
- M gamedata/config/text/eng/string_table_ui.xml
- M gamedata/config/text/eng/string_table_vergas.xml
- M gamedata/config/text/eng/ui_st_inventory.xml
- M gamedata/config/text/eng/ui_st_keybinding.xml
- M gamedata/config/text/eng/ui_st_mapdesc.xml
- M gamedata/config/text/eng/ui_st_mm.xml
- M gamedata/config/text/eng/ui_st_other.xml
- M gamedata/config/text/eng/ui_st_pda.xml
- M gamedata/config/text/rus/amk_flame_dialogs.xml
- M gamedata/config/text/rus/arc_string_table.xml
- M gamedata/config/text/rus/doc_dialogs.xml
- M gamedata/config/text/rus/he_watch.xml
- M gamedata/config/text/rus/mike_strings.xml
- M gamedata/config/text/rus/sak_strings.xml
- M gamedata/config/text/rus/sak_strings_attention.xml
- M gamedata/config/text/rus/sak_strings_six.xml
- M gamedata/config/text/rus/sak_strings_six2.xml
- M gamedata/config/text/rus/sak_strings_trade.xml
- M gamedata/config/text/rus/sak_strings_treasure.xml
- M gamedata/config/text/rus/script_strings.xml
- M gamedata/config/text/rus/stable_bio_name.xml
- M gamedata/config/text/rus/stable_dialog_manager.xml
- M gamedata/config/text/rus/stable_dialog_manager_uni.xml
- M gamedata/config/text/rus/stable_dialogs.xml
- M gamedata/config/text/rus/stable_dialogs_agroprom.xml
- M gamedata/config/text/rus/stable_dialogs_amk.xml
- M gamedata/config/text/rus/stable_dialogs_bar.xml
- M gamedata/config/text/rus/stable_dialogs_darkvalley.xml
- M gamedata/config/text/rus/stable_dialogs_escape.xml
- M gamedata/config/text/rus/stable_dialogs_garbage.xml
- M gamedata/config/text/rus/stable_dialogs_military.xml
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/config/text/rus/stable_dialogs_yantar.xml
- M gamedata/config/text/rus/stable_game_credits.xml
- M gamedata/config/text/rus/stable_generate_fnames.xml
- M gamedata/config/text/rus/stable_generate_snames.xml
- M gamedata/config/text/rus/stable_statistic_caption.xml
- M gamedata/config/text/rus/stable_stories.xml
- M gamedata/config/text/rus/stable_storyline_info_pda.xml
- M gamedata/config/text/rus/stable_task_manager.xml
- M gamedata/config/text/rus/stable_task_manager.xml.bak_kanaly_sbyta
- M gamedata/config/text/rus/stable_treasure_manager.xml
- M gamedata/config/text/rus/story_dialogs.xml
- M gamedata/config/text/rus/string_actor_bio.xml
- M gamedata/config/text/rus/string_news_mikelik.xml
- M gamedata/config/text/rus/string_table_amk_arts_desc.xml
- M gamedata/config/text/rus/string_table_amk_metka.xml
- M gamedata/config/text/rus/string_table_enc_equipment.xml
- M gamedata/config/text/rus/string_table_enc_mutants.xml
- M gamedata/config/text/rus/string_table_enc_social.xml
- M gamedata/config/text/rus/string_table_enc_vehicles.xml
- M gamedata/config/text/rus/string_table_enc_weapons.xml
- M gamedata/config/text/rus/string_table_enc_zone.xml
- M gamedata/config/text/rus/string_table_general.xml
- M gamedata/config/text/rus/string_table_he_magic_box.xml
- M gamedata/config/text/rus/string_table_includes.xml
- M gamedata/config/text/rus/string_table_items.xml
- M gamedata/config/text/rus/string_table_level_tips.xml
- M gamedata/config/text/rus/string_table_outfit.xml
- M gamedata/config/text/rus/string_table_pda_bandit.xml
- M gamedata/config/text/rus/string_table_pda_dolg.xml
- M gamedata/config/text/rus/string_table_pda_ecolog.xml
- M gamedata/config/text/rus/string_table_pda_freedom.xml
- M gamedata/config/text/rus/string_table_pda_killer.xml
- M gamedata/config/text/rus/string_table_pda_stalker.xml
- M gamedata/config/text/rus/string_table_pda_treasure.xml
- M gamedata/config/text/rus/string_table_pda_treasure_old.xml
- M gamedata/config/text/rus/string_table_tasks_standart.xml
- M gamedata/config/text/rus/string_table_tips.xml
- M gamedata/config/text/rus/string_table_tutorial.xml
- M gamedata/config/text/rus/string_table_ui.xml
- M gamedata/config/text/rus/string_table_vergas.xml
- M gamedata/config/text/rus/ui_st_inventory.xml
- M gamedata/config/text/rus/ui_st_keybinding.xml
- M gamedata/config/text/rus/ui_st_mapdesc.xml
- M gamedata/config/text/rus/ui_st_mm.xml
- M gamedata/config/text/rus/ui_st_other.xml
- M gamedata/config/text/rus/ui_st_pda.xml
- M gamedata/config/ui/actor_diary.xml
- M gamedata/config/ui/actor_statistic.xml
- M gamedata/config/ui/af_params.xml
- M gamedata/config/ui/af_params_16.xml
- M gamedata/config/ui/amk_ui_base.xml
- M gamedata/config/ui/amk_ui_base_16.xml
- M gamedata/config/ui/arc_tool.xml
- M gamedata/config/ui/car_panel.xml
- M gamedata/config/ui/car_panel_16.xml
- M gamedata/config/ui/carbody_item.xml
- M gamedata/config/ui/carbody_item_16.xml
- M gamedata/config/ui/carbody_new.xml
- M gamedata/config/ui/carbody_new_16.xml
- M gamedata/config/ui/color_defs.xml
- M gamedata/config/ui/contracts.xml
- M gamedata/config/ui/contracts_character.xml
- M gamedata/config/ui/encyclopedia.xml
- M gamedata/config/ui/encyclopedia_item.xml
- M gamedata/config/ui/events_new.xml
- M gamedata/config/ui/game_tutorials.xml
- M gamedata/config/ui/global_map.xml
- M gamedata/config/ui/grenade.xml
- M gamedata/config/ui/hint_item.xml
- M gamedata/config/ui/ingame_msglog_sp.xml
- M gamedata/config/ui/inventory_item.xml
- M gamedata/config/ui/inventory_item_16.xml
- M gamedata/config/ui/inventory_new.xml
- M gamedata/config/ui/inventory_new_16.xml
- M gamedata/config/ui/job_item.xml
- M gamedata/config/ui/jobs.xml
- M gamedata/config/ui/maingame.xml
- M gamedata/config/ui/maingame_16.xml
- M gamedata/config/ui/maingame_character.xml
- M gamedata/config/ui/maingame_pda_msg.xml
- M gamedata/config/ui/maingame_pda_msg_16.xml
- M gamedata/config/ui/map.xml
- M gamedata/config/ui/map_character.xml
- M gamedata/config/ui/map_desc.xml
- M gamedata/config/ui/map_desc_16.xml
- M gamedata/config/ui/map_spots.xml
- M gamedata/config/ui/map_spots_lchangers.xml
- M gamedata/config/ui/map_spots_mp.xml
- M gamedata/config/ui/map_spots_relations.xml
- M gamedata/config/ui/message_box.xml
- M gamedata/config/ui/messages_window.xml
- M gamedata/config/ui/messages_window_16.xml
- M gamedata/config/ui/motion_icon.xml
- M gamedata/config/ui/motion_icon_16.xml
- M gamedata/config/ui/news.xml
- M gamedata/config/ui/pda.xml
- M gamedata/config/ui/pda_contacts.xml
- M gamedata/config/ui/pda_contacts_new.xml
- M gamedata/config/ui/pda_dialog_character.xml
- M gamedata/config/ui/pda_dialog_character_16.xml
- M gamedata/config/ui/pda_events.xml
- M gamedata/config/ui/pda_map.xml
- M gamedata/config/ui/pda_spot.xml
- M gamedata/config/ui/scopes.xml
- M gamedata/config/ui/scopes_16.xml
- M gamedata/config/ui/scroll_bar.xml
- M gamedata/config/ui/stalkers_ranking.xml
- M gamedata/config/ui/stalkers_ranking_character.xml
- M gamedata/config/ui/stalkers_ranking_character_16.xml
- M gamedata/config/ui/statisticwnd.xml
- M gamedata/config/ui/talk.xml
- M gamedata/config/ui/talk_16.xml
- M gamedata/config/ui/textures_descr/ui_actor_pda_icons.xml
- M gamedata/config/ui/textures_descr/ui_characteristics_icons.xml
- M gamedata/config/ui/textures_descr/ui_ingame.xml
- M gamedata/config/ui/textures_descr/ui_ingame2_commonn.xml
- M gamedata/config/ui/textures_descr/ui_radial.xml
- M gamedata/config/ui/trade.xml
- M gamedata/config/ui/trade_16.xml
- M gamedata/config/ui/trade_character.xml
- M gamedata/config/ui/trade_character_16.xml
- M gamedata/config/ui/trade_item.xml
- M gamedata/config/ui/trade_item_16.xml
- M gamedata/config/ui/ui_art_box1.xml
- M gamedata/config/ui/ui_art_box1_16.xml
- M gamedata/config/ui/ui_art_box1_16_10.xml
- M gamedata/config/ui/ui_art_box1_21_9.xml
- M gamedata/config/ui/ui_art_box3.xml
- M gamedata/config/ui/ui_art_box3_16.xml
- M gamedata/config/ui/ui_art_box3_16_10.xml
- M gamedata/config/ui/ui_art_box3_21_9.xml
- M gamedata/config/ui/ui_art_box8.xml
- M gamedata/config/ui/ui_art_box8_16.xml
- M gamedata/config/ui/ui_art_box8_16_10.xml
- M gamedata/config/ui/ui_art_box8_21_9.xml
- M gamedata/config/ui/ui_asus_intro.xml
- M gamedata/config/ui/ui_attention_blank.xml
- M gamedata/config/ui/ui_attention_comp.xml
- M gamedata/config/ui/ui_barter.xml
- M gamedata/config/ui/ui_cheat_bag_on_belt.xml
- M gamedata/config/ui/ui_cheat_bag_on_belt_n.xml
- M gamedata/config/ui/ui_cheat_decoder.xml
- M gamedata/config/ui/ui_cheat_flask.xml
- M gamedata/config/ui/ui_cheat_inv_buttons.xml
- M gamedata/config/ui/ui_cheat_notebook.xml
- M gamedata/config/ui/ui_cheat_pda_msg.xml
- M gamedata/config/ui/ui_cheat_pda_npc.xml
- M gamedata/config/ui/ui_cheat_pda_npc_eng.xml
- M gamedata/config/ui/ui_cheat_podsumok.xml
- M gamedata/config/ui/ui_cheat_razgruzka.xml
- M gamedata/config/ui/ui_cheat_razgruzka_new.xml
- M gamedata/config/ui/ui_cheat_soc.xml
- M gamedata/config/ui/ui_cheat_wnd.xml
- M gamedata/config/ui/ui_common.xml
- M gamedata/config/ui/ui_credits.xml
- M gamedata/config/ui/ui_custom_msgs.xml
- M gamedata/config/ui/ui_custom_msgs_16.xml
- M gamedata/config/ui/ui_death.xml
- M gamedata/config/ui/ui_detector_artefact.xml
- M gamedata/config/ui/ui_dg_inventory_arc.xml
- M gamedata/config/ui/ui_dots_wnd.xml
- M gamedata/config/ui/ui_dsh_remkit.xml
- M gamedata/config/ui/ui_dsh_remkit_16.xml
- M gamedata/config/ui/ui_dynamic_hud.xml
- M gamedata/config/ui/ui_dynamic_hud_16.xml
- M gamedata/config/ui/ui_find_by_name.xml
- M gamedata/config/ui/ui_game_over.xml
- M gamedata/config/ui/ui_gps_trade_wnd.xml
- M gamedata/config/ui/ui_he_infogiver.xml
- M gamedata/config/ui/ui_he_magic_box.xml
- M gamedata/config/ui/ui_hud.xml
- M gamedata/config/ui/ui_hud_mask.xml
- M gamedata/config/ui/ui_hud_mm.xml
- M gamedata/config/ui/ui_hud_thirst_he.xml
- M gamedata/config/ui/ui_icons_equ_1.xml
- M gamedata/config/ui/ui_icons_equ_2.xml
- M gamedata/config/ui/ui_icons_he.xml
- M gamedata/config/ui/ui_icons_map.xml
- M gamedata/config/ui/ui_icons_notebook_parts.xml
- M gamedata/config/ui/ui_icons_npc.xml
- M gamedata/config/ui/ui_icons_npc_16.xml
- M gamedata/config/ui/ui_icons_npc_suits.xml
- M gamedata/config/ui/ui_icons_npc_suits_16.xml
- M gamedata/config/ui/ui_icons_repair.xml
- M gamedata/config/ui/ui_icons_rykzack.xml
- M gamedata/config/ui/ui_iconstotal.xml
- M gamedata/config/ui/ui_info_menu.xml
- M gamedata/config/ui/ui_ingame.xml
- M gamedata/config/ui/ui_inv_descr.xml
- M gamedata/config/ui/ui_inv_descr_16.xml
- M gamedata/config/ui/ui_inventory.xml
- M gamedata/config/ui/ui_inventory2.xml
- M gamedata/config/ui/ui_keybinding.xml
- M gamedata/config/ui/ui_mainmenu.xml
- M gamedata/config/ui/ui_map_description.xml
- M gamedata/config/ui/ui_medkit_use.xml
- M gamedata/config/ui/ui_mm_load_dlg.xml
- M gamedata/config/ui/ui_mm_loading_screen.xml
- M gamedata/config/ui/ui_mm_main.xml
- M gamedata/config/ui/ui_mm_main_16.xml
- M gamedata/config/ui/ui_mm_opt.xml
- M gamedata/config/ui/ui_mm_save_dlg.xml
- M gamedata/config/ui/ui_movies.xml
- M gamedata/config/ui/ui_nlc_hud.xml
- M gamedata/config/ui/ui_notebook_arts.xml
- M gamedata/config/ui/ui_npc_monster.xml
- M gamedata/config/ui/ui_npc_monster_new.xml
- M gamedata/config/ui/ui_npc_u_actor_nlc.xml
- M gamedata/config/ui/ui_npc_unique.xml
- M gamedata/config/ui/ui_numpad.xml
- M gamedata/config/ui/ui_numpad_wnd.xml
- M gamedata/config/ui/ui_old_textures.xml
- M gamedata/config/ui/ui_pda.xml
- M gamedata/config/ui/ui_pda_add.xml
- M gamedata/config/ui/ui_pda_pust.xml
- M gamedata/config/ui/ui_pda_pust_eng.xml
- M gamedata/config/ui/ui_pda_steklo_bad.xml
- M gamedata/config/ui/ui_pda_steklo_ok.xml
- M gamedata/config/ui/ui_sharper.xml
- M gamedata/config/ui/ui_sleep.xml
- M gamedata/config/ui/ui_spawner.xml
- M gamedata/config/ui/ui_statistics.xml
- M gamedata/config/ui/ui_vergas_decoder.xml
- M gamedata/config/ui/ui_vergas_mask.xml
- M gamedata/config/ui/ui_vergas_zayaz.xml
- M gamedata/config/ui/ui_vergas_zayaz_new.xml
- M gamedata/config/ui/ui_vergas_zayaz_timer.xml
- M gamedata/config/ui/zone_map.xml
- M gamedata/config/ui/zone_map_16.xml
- M gamedata/config/waypoints.ltx
- M gamedata/config/weapons/add_gl.ltx
- M gamedata/config/weapons/add_scopes.ltx
- M gamedata/config/weapons/add_silencers.ltx
- M gamedata/config/weapons/delayed_action_fuse.ltx
- M gamedata/config/weapons/gl.ltx
- M gamedata/config/weapons/grenadier.ltx
- M gamedata/config/weapons/w_1911.ltx
- M gamedata/config/weapons/w_abakan.ltx
- M gamedata/config/weapons/w_ak103.ltx
- M gamedata/config/weapons/w_ak104.ltx
- M gamedata/config/weapons/w_ak74.ltx
- M gamedata/config/weapons/w_ak74u.ltx
- M gamedata/config/weapons/w_ak_strelok.ltx
- M gamedata/config/weapons/w_ak_strelok_rem.ltx
- M gamedata/config/weapons/w_akm.ltx
- M gamedata/config/weapons/w_apb.ltx
- M gamedata/config/weapons/w_awm.ltx
- M gamedata/config/weapons/w_beretta92.ltx
- M gamedata/config/weapons/w_binoc.ltx
- M gamedata/config/weapons/w_bizon-2.ltx
- M gamedata/config/weapons/w_bolt.ltx
- M gamedata/config/weapons/w_colt_m45a1.ltx
- M gamedata/config/weapons/w_desert_eagle.ltx
- M gamedata/config/weapons/w_flame.ltx
- M gamedata/config/weapons/w_fn2000.ltx
- M gamedata/config/weapons/w_fn_fnx.ltx
- M gamedata/config/weapons/w_fn_scar.ltx
- M gamedata/config/weapons/w_fort12.ltx
- M gamedata/config/weapons/w_g36.ltx
- M gamedata/config/weapons/w_gauss_pistol.ltx
- M gamedata/config/weapons/w_gauss_rifle.ltx
- M gamedata/config/weapons/w_glock17.ltx
- M gamedata/config/weapons/w_grenades.ltx
- M gamedata/config/weapons/w_groza.ltx
- M gamedata/config/weapons/w_hk_m320.ltx
- M gamedata/config/weapons/w_knife.ltx
- M gamedata/config/weapons/w_m4a1.ltx
- M gamedata/config/weapons/w_mg36.ltx
- M gamedata/config/weapons/w_mosin.ltx
- M gamedata/config/weapons/w_mp133_short.ltx
- M gamedata/config/weapons/w_mp153.ltx
- M gamedata/config/weapons/w_mp153_short.ltx
- M gamedata/config/weapons/w_mp5.ltx
- M gamedata/config/weapons/w_mp5_mod.ltx
- M gamedata/config/weapons/w_oreh_shotgun.ltx
- M gamedata/config/weapons/w_pb.ltx
- M gamedata/config/weapons/w_pernach.ltx
- M gamedata/config/weapons/w_pkm.ltx
- M gamedata/config/weapons/w_pkm_mod.ltx
- M gamedata/config/weapons/w_pm.ltx
- M gamedata/config/weapons/w_remington870.ltx
- M gamedata/config/weapons/w_rg6.ltx
- M gamedata/config/weapons/w_rpg7.ltx
- M gamedata/config/weapons/w_rpk16.ltx
- M gamedata/config/weapons/w_rpk74.ltx
- M gamedata/config/weapons/w_sa58.ltx
- M gamedata/config/weapons/w_saiga12_mod.ltx
- M gamedata/config/weapons/w_saiga12c.ltx
- M gamedata/config/weapons/w_sig220.ltx
- M gamedata/config/weapons/w_sig552.ltx
- M gamedata/config/weapons/w_sks.ltx
- M gamedata/config/weapons/w_spas12.ltx
- M gamedata/config/weapons/w_sv98.ltx
- M gamedata/config/weapons/w_svd.ltx
- M gamedata/config/weapons/w_svu.ltx
- M gamedata/config/weapons/w_taurus.ltx
- M gamedata/config/weapons/w_toz34.ltx
- M gamedata/config/weapons/w_toz34_hunter.ltx
- M gamedata/config/weapons/w_toz66.ltx
- M gamedata/config/weapons/w_toz66_short.ltx
- M gamedata/config/weapons/w_ump45.ltx
- M gamedata/config/weapons/w_usas12.ltx
- M gamedata/config/weapons/w_usas12_mod.ltx
- M gamedata/config/weapons/w_usp.ltx
- M gamedata/config/weapons/w_val.ltx
- M gamedata/config/weapons/w_vector.ltx
- M gamedata/config/weapons/w_vintorez.ltx
- M gamedata/config/weapons/w_vintorez_mod.ltx
- M gamedata/config/weapons/w_vityaz.ltx
- M gamedata/config/weapons/w_vpo101_hunter.ltx
- M gamedata/config/weapons/w_walther.ltx
- M gamedata/config/weapons/weapons.ltx
- M gamedata/levels/fake_start/level.ltx
- M gamedata/levels/l08_yantar/level.ps_static.ltx
- M gamedata/levels/l08_yantar/level_stat.txt
- M gamedata/scripts/_sdata.script
- M gamedata/scripts/actor_proxy.script
- M gamedata/scripts/actor_take_item.script
- M gamedata/scripts/ai_additions/rx_addons.script
- M gamedata/scripts/ai_additions/rx_ai.script
- M gamedata/scripts/ai_additions/rx_bandage.script
- M gamedata/scripts/ai_additions/rx_facer.script
- M gamedata/scripts/ai_additions/rx_ff.script
- M gamedata/scripts/ai_additions/rx_gl.script
- M gamedata/scripts/ai_additions/rx_kill_wounded.script
- M gamedata/scripts/ai_additions/rx_knife.script
- M gamedata/scripts/ai_additions/rx_reload.script
- M gamedata/scripts/ai_additions/rx_sound.script
- M gamedata/scripts/ai_additions/rx_utils.script
- M gamedata/scripts/ai_additions/xr_help_wounded.script
- M gamedata/scripts/allcore_hide.script
- M gamedata/scripts/alpet/balance.script
- M gamedata/scripts/alpet/const.script
- M gamedata/scripts/alpet/list_tools.script
- M gamedata/scripts/alpet/misc.script
- M gamedata/scripts/alpet/nlc_container.script
- M gamedata/scripts/alpet/schedule.script
- M gamedata/scripts/alpet/statics.script
- M gamedata/scripts/alpet/texman.script
- M gamedata/scripts/amk/amk.script
- M gamedata/scripts/amk/amk_anoms.script
- M gamedata/scripts/amk/amk_hideouts.script
- M gamedata/scripts/amk/amk_mod.script
- M gamedata/scripts/amk/amk_names_lists.script
- M gamedata/scripts/amk/amk_offline_alife.script
- M gamedata/scripts/amk/amk_particle.script
- M gamedata/scripts/amk/amk_uniq_news_lists.script
- M gamedata/scripts/amk/utils.script
- M gamedata/scripts/anagramm_flash.script
- M gamedata/scripts/anomaly_detector.script
- M gamedata/scripts/ansi_functions.script
- M gamedata/scripts/ar_sleep.script
- M gamedata/scripts/arts/arc_arts.script
- M gamedata/scripts/arts/arc_diary.script
- M gamedata/scripts/ballon_refill.script
- M gamedata/scripts/binders/bind_bubble.script
- M gamedata/scripts/binders/bind_det_arts.script
- M gamedata/scripts/binders/bind_greedy.script
- M gamedata/scripts/binders/bind_heli.script
- M gamedata/scripts/binders/bind_monster.script
- M gamedata/scripts/binders/bind_mteleport.script
- M gamedata/scripts/binders/bind_pda.script
- M gamedata/scripts/binders/bind_respawn.script
- M gamedata/scripts/binders/bind_restrictor.script
- M gamedata/scripts/binders/bind_smart_terrain.script
- M gamedata/scripts/binders/bind_stalker.script
- M gamedata/scripts/binders/bind_upg_outfit.script
- M gamedata/scripts/blowout_scheme.script
- M gamedata/scripts/ce_switcher.script
- M gamedata/scripts/class_registrator.script
- M gamedata/scripts/common/_g.script
- M gamedata/scripts/common/db.script
- M gamedata/scripts/common/net_utils_new.script
- M gamedata/scripts/common/timers.script
- M gamedata/scripts/death_manager.script
- M gamedata/scripts/debug/profiler.script
- M gamedata/scripts/debug/version.script
- M gamedata/scripts/den1s_test.script
- M gamedata/scripts/dialogs/bar_dialogs.script
- M gamedata/scripts/dialogs/dialog_manager.script
- M gamedata/scripts/dialogs/dialogs.script
- M gamedata/scripts/dialogs/escape_dialog.script
- M gamedata/scripts/doc.script
- M gamedata/scripts/dream.script
- M gamedata/scripts/dsh.script
- M gamedata/scripts/dsh_battle_radius.script
- M gamedata/scripts/dsh_enemies.script
- M gamedata/scripts/dsh_hud_icons.script
- M gamedata/scripts/dsh_monster_attack.script
- M gamedata/scripts/dsh_remkit.script
- M gamedata/scripts/dsh_torch_backup.script
- M gamedata/scripts/dsh_ui.script
- M gamedata/scripts/dsh_ui_simple_craft.script
- M gamedata/scripts/dsh_wpn.script
- M gamedata/scripts/engine_callbacks.script
- M gamedata/scripts/figure.script
- M gamedata/scripts/flamethrower.script
- M gamedata/scripts/fx_sound.script
- M gamedata/scripts/game_stats.script
- M gamedata/scripts/gnzi.script
- M gamedata/scripts/gps_habar.script
- M gamedata/scripts/gulag/gulag_agroprom.script
- M gamedata/scripts/gulag/gulag_agroprom_underground.script
- M gamedata/scripts/gulag/gulag_amk.script
- M gamedata/scripts/gulag/gulag_bar.script
- M gamedata/scripts/gulag/gulag_dark_valley.script
- M gamedata/scripts/gulag/gulag_dark_valley_nlc.script
- M gamedata/scripts/gulag/gulag_escape.script
- M gamedata/scripts/gulag/gulag_garbage.script
- M gamedata/scripts/gulag/gulag_general.script
- M gamedata/scripts/gulag/gulag_general_prip.script
- M gamedata/scripts/gulag/gulag_kishka.script
- M gamedata/scripts/gulag/gulag_labx18.script
- M gamedata/scripts/gulag/gulag_limansk.script
- M gamedata/scripts/gulag/gulag_military.script
- M gamedata/scripts/gulag/gulag_pripyat.script
- M gamedata/scripts/gulag/gulag_radar.script
- M gamedata/scripts/gulag/gulag_radar_u.script
- M gamedata/scripts/gulag/gulag_sarcofag.script
- M gamedata/scripts/gulag/gulag_tasks.script
- M gamedata/scripts/gulag/gulag_yantar.script
- M gamedata/scripts/he_bai_compat.script
- M gamedata/scripts/he_exo_charge.script
- M gamedata/scripts/he_gluzov_art.script
- M gamedata/scripts/he_guest_kill.script
- M gamedata/scripts/he_hipfire.script
- M gamedata/scripts/he_info_giver.script
- M gamedata/scripts/he_info_list.script
- M gamedata/scripts/he_kruglov.script
- M gamedata/scripts/he_local_feedback.script
- M gamedata/scripts/he_magic_box.script
- M gamedata/scripts/he_no_monsters.script
- M gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/he_pda_access.script
- M gamedata/scripts/he_pda_market.script
- M gamedata/scripts/he_pda_model.script
- M gamedata/scripts/he_pda_news.script
- M gamedata/scripts/he_red_water.script
- M gamedata/scripts/he_semenov.script
- M gamedata/scripts/he_short_story.script
- M gamedata/scripts/he_sidor_scam.script
- M gamedata/scripts/he_stor.script
- M gamedata/scripts/he_watch_battery.script
- M gamedata/scripts/he_watch_display.script
- M gamedata/scripts/he_watch_status.script
- M gamedata/scripts/he_watch_ui.script
- M gamedata/scripts/jack.script
- M gamedata/scripts/kotovod.script
- M gamedata/scripts/kotovod/arc_radiation.script
- M gamedata/scripts/kotovod/art_collections.script
- M gamedata/scripts/kotovod/cleaning_masks_if_raining.script
- M gamedata/scripts/kotovod/collect_info_portions.script
- M gamedata/scripts/kotovod/detectors_art.script
- M gamedata/scripts/kotovod/dsh_coordinator.script
- M gamedata/scripts/kotovod/dsh_drag_drop.script
- M gamedata/scripts/kotovod/dsh_reload.script
- M gamedata/scripts/kotovod/dsh_switch_addons.script
- M gamedata/scripts/kotovod/game_options.script
- M gamedata/scripts/kotovod/garbage_arts.script
- M gamedata/scripts/kotovod/he_sleeves.script
- M gamedata/scripts/kotovod/monster_parts.script
- M gamedata/scripts/kotovod/nlc_torch.script
- M gamedata/scripts/kotovod/on_body_item_drop.script
- M gamedata/scripts/kotovod/on_lc.script
- M gamedata/scripts/kotovod/parts_decay.script
- M gamedata/scripts/kotovod/pda_actions.script
- M gamedata/scripts/kotovod/spawner.script
- M gamedata/scripts/kotovod/special_container_drop.script
- M gamedata/scripts/kotovod/trade_arts_news.script
- M gamedata/scripts/kotovod/ui_arc_container_menu.script
- M gamedata/scripts/kotovod/ui_sharper.script
- M gamedata/scripts/kotovod/undergrounds_hits.script
- M gamedata/scripts/kotovod_restrictors.script
- M gamedata/scripts/level_weathers.script
- M gamedata/scripts/levels/darkvalley_tasks.script
- M gamedata/scripts/levels/escape_tasks.script
- M gamedata/scripts/levels/garbage_tasks.script
- M gamedata/scripts/levels/level_tasks.script
- M gamedata/scripts/logic/smart_terrain.script
- M gamedata/scripts/logic/smart_terrain_params.script
- M gamedata/scripts/logic/stalker_generic.script
- M gamedata/scripts/lua_help.script
- M gamedata/scripts/machines/heli_combat.script
- M gamedata/scripts/machines/heli_func.script
- M gamedata/scripts/machines/heli_move.script
- M gamedata/scripts/machines/heli_snd.script
- M gamedata/scripts/mike.script
- M gamedata/scripts/mob/mob_alife_mgr.script
- M gamedata/scripts/mob/mob_camp.script
- M gamedata/scripts/mob/mob_combat.script
- M gamedata/scripts/mob/mob_death.script
- M gamedata/scripts/mob/mob_eluder.script
- M gamedata/scripts/mob/mob_fake_death.script
- M gamedata/scripts/mob/mob_home.script
- M gamedata/scripts/mob/mob_jump.script
- M gamedata/scripts/mob/mob_kicker.script
- M gamedata/scripts/mob/mob_panic.script
- M gamedata/scripts/mob/mob_remark.script
- M gamedata/scripts/mob/mob_sound.script
- M gamedata/scripts/mob/mob_state_mgr.script
- M gamedata/scripts/mob/mob_trade.script
- M gamedata/scripts/mob/mob_trader.script
- M gamedata/scripts/mob/mob_walker.script
- M gamedata/scripts/modules.script
- M gamedata/scripts/move_mgr.script
- M gamedata/scripts/new_compass_tp.script
- M gamedata/scripts/new_quests.script
- M gamedata/scripts/news/news_data.script
- M gamedata/scripts/news/news_main.script
- M gamedata/scripts/news/news_manager.script
- M gamedata/scripts/news/news_mikelik.script
- M gamedata/scripts/nlc_capsules.script
- M gamedata/scripts/nlc_corpse_decay.script
- M gamedata/scripts/nlc_fixes.script
- M gamedata/scripts/ogse/ogse_benchmark.script
- M gamedata/scripts/ogse/ogse_campfire.script
- M gamedata/scripts/ogse/ogse_debug.script
- M gamedata/scripts/ogse/ogse_qt.script
- M gamedata/scripts/ogse/ogse_signals.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/ogse/ogse_wpn_utils.script
- M gamedata/scripts/ogse_critical_hit.script
- M gamedata/scripts/ogsr_exo_step_sound.script
- M gamedata/scripts/ogsr_memory_control.script
- M gamedata/scripts/ogsr_shaders_control.script
- M gamedata/scripts/olr/olr_ai_tasks.script
- M gamedata/scripts/olr/olr_faction_policy.script
- M gamedata/scripts/olr/olr_init.script
- M gamedata/scripts/olr/olr_monster_squads.script
- M gamedata/scripts/olr/olr_monsters_sim.script
- M gamedata/scripts/olr/olr_offline_task.script
- M gamedata/scripts/olr/olr_offline_walk.script
- M gamedata/scripts/olr/olr_online_depart.script
- M gamedata/scripts/olr/olr_post_watch.script
- M gamedata/scripts/olr/olr_protected.script
- M gamedata/scripts/olr/olr_raids.script
- M gamedata/scripts/olr/olr_relations.script
- M gamedata/scripts/olr/olr_smart_index.script
- M gamedata/scripts/olr/olr_smart_tuning.script
- M gamedata/scripts/olr/olr_squads.script
- M gamedata/scripts/olr/olr_timers.script
- M gamedata/scripts/oxy_wnp.script
- M gamedata/scripts/oxyr.script
- M gamedata/scripts/params/actor_params.script
- M gamedata/scripts/params/coord_params.script
- M gamedata/scripts/params/item_params.script
- M gamedata/scripts/params/nvd_params.script
- M gamedata/scripts/params/params.script
- M gamedata/scripts/params/wpn_params.script
- M gamedata/scripts/pda_battery.script
- M gamedata/scripts/pda_noise.script
- M gamedata/scripts/ph/ph_appforce.script
- M gamedata/scripts/ph/ph_button.script
- M gamedata/scripts/ph/ph_camera.script
- M gamedata/scripts/ph/ph_car.script
- M gamedata/scripts/ph/ph_code.script
- M gamedata/scripts/ph/ph_death.script
- M gamedata/scripts/ph/ph_door.script
- M gamedata/scripts/ph/ph_gate.script
- M gamedata/scripts/ph/ph_hit.script
- M gamedata/scripts/ph/ph_idle.script
- M gamedata/scripts/ph/ph_impulse.script
- M gamedata/scripts/ph/ph_on_hit.script
- M gamedata/scripts/ph/ph_oscillate.script
- M gamedata/scripts/ph/ph_sound.script
- M gamedata/scripts/phantom_manager.script
- M gamedata/scripts/postprocess.script
- M gamedata/scripts/projectors.script
- M gamedata/scripts/ranks.script
- M gamedata/scripts/registry.script
- M gamedata/scripts/sak/black_tip.script
- M gamedata/scripts/sak/sak.script
- M gamedata/scripts/sak/sak_artmods.script
- M gamedata/scripts/sak/sak_box_move.script
- M gamedata/scripts/sak/sak_dialog.script
- M gamedata/scripts/sak/sak_inventory.script
- M gamedata/scripts/sak/sak_minigame.script
- M gamedata/scripts/sak/sak_off_corpses.script
- M gamedata/scripts/sak/sak_rand_nagrad.script
- M gamedata/scripts/sak/sak_repair_dialog.script
- M gamedata/scripts/sak/sak_spawn.script
- M gamedata/scripts/sak/sak_tasks.script
- M gamedata/scripts/se/se_artefact.script
- M gamedata/scripts/se/se_car.script
- M gamedata/scripts/se/se_item.script
- M gamedata/scripts/se/se_legs.script
- M gamedata/scripts/se/se_monster.script
- M gamedata/scripts/se/se_respawn.script
- M gamedata/scripts/se/se_stalker.script
- M gamedata/scripts/se/se_switcher.script
- M gamedata/scripts/se/se_zones.script
- M gamedata/scripts/set_far_dist_vision.script
- M gamedata/scripts/sim_statistic.script
- M gamedata/scripts/sivol/sivol.script
- M gamedata/scripts/sivol/sivol_anom_effects.script
- M gamedata/scripts/sivol/sivol_blood_pools.script
- M gamedata/scripts/sivol/sivol_critical_malfunction.script
- M gamedata/scripts/sivol/sivol_cry_sprint.script
- M gamedata/scripts/sivol/sivol_emp_config.script
- M gamedata/scripts/sivol/sivol_emp_sources.script
- M gamedata/scripts/sivol/sivol_fuzz_recoil.script
- M gamedata/scripts/sivol/sivol_fuzz_recoil_profiles.script
- M gamedata/scripts/sivol/sivol_hit_shock.script
- M gamedata/scripts/sivol/sivol_kolobok.script
- M gamedata/scripts/sivol/sivol_masks.script
- M gamedata/scripts/sivol/sivol_outfit_animations.script
- M gamedata/scripts/sivol/sivol_outfit_stats.script
- M gamedata/scripts/sivol/sivol_paper.script
- M gamedata/scripts/sivol/sivol_pda.script
- M gamedata/scripts/sivol/sivol_psy_phantoms.script
- M gamedata/scripts/sivol/sivol_pudding_outfit.script
- M gamedata/scripts/sivol/sivol_rain.script
- M gamedata/scripts/sivol/sivol_shell_sounds.script
- M gamedata/scripts/sivol/sivol_spirit_regen.script
- M gamedata/scripts/sivol/sivol_sprint_fov.script
- M gamedata/scripts/sivol/sivol_unknown_arts.script
- M gamedata/scripts/sivol/sivol_visible_body.script
- M gamedata/scripts/sivol/sivol_weapon.script
- M gamedata/scripts/sivol/sivol_weapon_aim_speed.script
- M gamedata/scripts/sivol/sivol_weapon_inertia.script
- M gamedata/scripts/sivol/sivol_weapon_inertia_configs.script
- M gamedata/scripts/sivol/sivol_weapon_zoom.script
- M gamedata/scripts/sleep_manager.script
- M gamedata/scripts/slot_keeper.script
- M gamedata/scripts/sound_theme.script
- M gamedata/scripts/spawn_teleport.script
- M gamedata/scripts/sr/sr_aes_deadzone.script
- M gamedata/scripts/sr/sr_cutscene.script
- M gamedata/scripts/sr/sr_danger.script
- M gamedata/scripts/sr/sr_idle.script
- M gamedata/scripts/sr/sr_light.script
- M gamedata/scripts/sr/sr_mapspot.script
- M gamedata/scripts/sr/sr_no_weapon.script
- M gamedata/scripts/sr/sr_particle.script
- M gamedata/scripts/sr/sr_postprocess.script
- M gamedata/scripts/sr/sr_psy_antenna.script
- M gamedata/scripts/sr/sr_sleep.script
- M gamedata/scripts/sr/sr_sound.script
- M gamedata/scripts/sr/sr_sound2d.script
- M gamedata/scripts/sr/sr_teleport.script
- M gamedata/scripts/sr/sr_territory.script
- M gamedata/scripts/sr/sr_timer.script
- M gamedata/scripts/sr/sr_tip.script
- M gamedata/scripts/standalone_tele.script
- M gamedata/scripts/state/state_lib.script
- M gamedata/scripts/state/state_mgr.script
- M gamedata/scripts/state/state_mgr_animation.script
- M gamedata/scripts/state/state_mgr_animation_list.script
- M gamedata/scripts/state/state_mgr_animstate.script
- M gamedata/scripts/state/state_mgr_bodystate.script
- M gamedata/scripts/state/state_mgr_direction.script
- M gamedata/scripts/state/state_mgr_goap.script
- M gamedata/scripts/state/state_mgr_mental.script
- M gamedata/scripts/state/state_mgr_movement.script
- M gamedata/scripts/state/state_mgr_weapon.script
- M gamedata/scripts/task_manager.script
- M gamedata/scripts/test.script
- M gamedata/scripts/thirst.script
- M gamedata/scripts/trade_manager.script
- M gamedata/scripts/treasure_manager.script
- M gamedata/scripts/ui/ui_barter.script
- M gamedata/scripts/ui/ui_body_add.script
- M gamedata/scripts/ui/ui_cheat.script
- M gamedata/scripts/ui/ui_cheat_bag_on_belt.script
- M gamedata/scripts/ui/ui_cheat_bag_on_belt_n.script
- M gamedata/scripts/ui/ui_cheat_flask.script
- M gamedata/scripts/ui/ui_cheat_flask_quickly.script
- M gamedata/scripts/ui/ui_cheat_inv_buttons.script
- M gamedata/scripts/ui/ui_cheat_notebook.script
- M gamedata/scripts/ui/ui_cheat_pda.script
- M gamedata/scripts/ui/ui_cheat_pda_menu.script
- M gamedata/scripts/ui/ui_cheat_podsumok.script
- M gamedata/scripts/ui/ui_cheat_razgruzka.script
- M gamedata/scripts/ui/ui_cheat_razgruzka_new.script
- M gamedata/scripts/ui/ui_death.script
- M gamedata/scripts/ui/ui_dots.script
- M gamedata/scripts/ui/ui_find_by_name.script
- M gamedata/scripts/ui/ui_gps_trade.script
- M gamedata/scripts/ui/ui_he_magic_box.script
- M gamedata/scripts/ui/ui_hud_attention_blank.script
- M gamedata/scripts/ui/ui_hud_attention_comp.script
- M gamedata/scripts/ui/ui_info_menu.script
- M gamedata/scripts/ui/ui_inv_add.script
- M gamedata/scripts/ui/ui_inv_descr.script
- M gamedata/scripts/ui/ui_main_menu.script
- M gamedata/scripts/ui/ui_mm_opt_controls.script
- M gamedata/scripts/ui/ui_mm_opt_gameplay.script
- M gamedata/scripts/ui/ui_mm_opt_main.script
- M gamedata/scripts/ui/ui_mm_opt_sound.script
- M gamedata/scripts/ui/ui_mm_opt_video.script
- M gamedata/scripts/ui/ui_mm_opt_video_adv.script
- M gamedata/scripts/ui/ui_mm_save_load_dialog.script
- M gamedata/scripts/ui/ui_notebook_arts.script
- M gamedata/scripts/ui/ui_numpad.script
- M gamedata/scripts/ui/ui_params.script
- M gamedata/scripts/ui/ui_pda_addons.script
- M gamedata/scripts/ui/ui_trade_add.script
- M gamedata/scripts/ui/ui_wpn_params.script
- M gamedata/scripts/ui/windows.script
- M gamedata/scripts/vergas/bag_on_belt_use.script
- M gamedata/scripts/vergas/inv_buttons_use.script
- M gamedata/scripts/vergas/razgruzka.script
- M gamedata/scripts/vergas/sleeping_bag.script
- M gamedata/scripts/vergas/used_items.script
- M gamedata/scripts/vergas/vergas_art.script
- M gamedata/scripts/vergas/vergas_decoder.script
- M gamedata/scripts/vergas/vergas_flask.script
- M gamedata/scripts/vergas/vergas_krujka.script
- M gamedata/scripts/vergas/vergas_lib.script
- M gamedata/scripts/vergas/vergas_masks.script
- M gamedata/scripts/vergas/vergas_medical_elastic.script
- M gamedata/scripts/vergas/vergas_pda.script
- M gamedata/scripts/vergas/vergas_rucksack.script
- M gamedata/scripts/vergas/vergas_scrambling.script
- M gamedata/scripts/vergas/vergas_watch.script
- M gamedata/scripts/watch_gate.script
- M gamedata/scripts/watcher_act.script
- M gamedata/scripts/weight_upgrade.script
- M gamedata/scripts/xr/xr_abuse.script
- M gamedata/scripts/xr/xr_actions_id.script
- M gamedata/scripts/xr/xr_assistance.script
- M gamedata/scripts/xr/xr_attendant.script
- M gamedata/scripts/xr/xr_box.script
- M gamedata/scripts/xr/xr_camper.script
- M gamedata/scripts/xr/xr_combat.script
- M gamedata/scripts/xr/xr_combat_camper.script
- M gamedata/scripts/xr/xr_combat_ignore.script
- M gamedata/scripts/xr/xr_combat_monolith.script
- M gamedata/scripts/xr/xr_combat_zombied.script
- M gamedata/scripts/xr/xr_companion.script
- M gamedata/scripts/xr/xr_conditions.script
- M gamedata/scripts/xr/xr_danger.script
- M gamedata/scripts/xr/xr_death.script
- M gamedata/scripts/xr/xr_detector.script
- M gamedata/scripts/xr/xr_effects.script
- M gamedata/scripts/xr/xr_evaluators_id.script
- M gamedata/scripts/xr/xr_gulag.script
- M gamedata/scripts/xr/xr_heli_hunter.script
- M gamedata/scripts/xr/xr_hit.script
- M gamedata/scripts/xr/xr_info.script
- M gamedata/scripts/xr/xr_kamp.script
- M gamedata/scripts/xr/xr_kamp_data.script
- M gamedata/scripts/xr/xr_logic.script
- M gamedata/scripts/xr/xr_meet.script
- M gamedata/scripts/xr/xr_motivator.script
- M gamedata/scripts/xr/xr_patrol.script
- M gamedata/scripts/xr/xr_remark.script
- M gamedata/scripts/xr/xr_rest.script
- M gamedata/scripts/xr/xr_sleeper.script
- M gamedata/scripts/xr/xr_sound.script
- M gamedata/scripts/xr/xr_spawner.script
- M gamedata/scripts/xr/xr_state.script
- M gamedata/scripts/xr/xr_statistic.script
- M gamedata/scripts/xr/xr_use.script
- M gamedata/scripts/xr/xr_walker.script
- M gamedata/scripts/xr/xr_wounded.script
- M gamedata/scripts/xr/xr_zones.script
- M gamedata/scripts/xray_vars.script
- M gamedata/scripts/xrs/xrs_ai.script
- M gamedata/scripts/xrs/xrs_armor.script
- M gamedata/scripts/xrs/xrs_battle_ai.script
- M gamedata/scripts/xrs/xrs_battle_ai_data.script
- M gamedata/scripts/xrs/xrs_grenade.script
- M gamedata/scripts/xrs/xrs_medic.script
- M gamedata/scripts/xrs/xrs_utils.script
- M gamedata/shaders/r1/models_selflight_det_3.s
- M gamedata/shaders/r1/models_selflight_det_3_aref.s
- M gamedata/shaders/r1/models_xleerinspect.s
- M gamedata/shaders/r2/models_selflight_det_3.s
- M gamedata/shaders/r2/models_selflight_det_3_aref.s
- M gamedata/shaders/r2/models_xleerinspect.s
- M gamedata/shaders/r3/he_watch_common.h
- M gamedata/shaders/r3/model_blood_icon.ps
- M gamedata/shaders/r3/model_blood_icon.s
- M gamedata/shaders/r3/model_clock.ps
- M gamedata/shaders/r3/model_clock.s
- M gamedata/shaders/r3/model_compass_icon.ps
- M gamedata/shaders/r3/model_compass_icon.s
- M gamedata/shaders/r3/model_health_icon.ps
- M gamedata/shaders/r3/model_healthbar.ps
- M gamedata/shaders/r3/model_power_icon.ps
- M gamedata/shaders/r3/model_powerbar.ps
- M gamedata/shaders/r3/model_satiety_icon.ps
- M gamedata/shaders/r3/model_thirst_icon.ps
- M gamedata/shaders/r3/model_watch_ui_backlight.ps
- M gamedata/shaders/r3/model_watch_ui_backlight_emissive.ps
- M gamedata/shaders/r3/models_pda_screen.ps
- M gamedata/shaders/r3/models_selflight_det_3.s
- M gamedata/shaders/r3/models_selflight_det_3_aref.s
- M gamedata/shaders/r3/models_watch_ui.ps
- M gamedata/shaders/r3/models_xleerinspect.s
- M gamedata/shaders/r3/pda_overlay_0.s
- M gamedata/shaders/r3/pda_screen.ps
- M gamedata/shaders/r3/pda_screen.s
- M gamedata/shaders/r3/pda_screen.vs
- M gamedata/textures/animated/flame/flame.seq
- M gamedata/textures/ballon/desktop.ini
- M gamedata/textures/textures.ltx
- M gamedata/textures/ui/ui_pda_broken.seq
- M gamedata/textures/ui/ui_pda_noise.seq

### Other repository files changed

- M .gitattributes
- M .gitignore
- M README.md
- M all.spawn.improved_3.9.1_ogsr/unpacked/_level_unknown.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife__level_unknown.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_av_peshera.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_aver.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_dead_city.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_deadcity.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_generators.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_hospital.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_jupiter.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_jupiter_underground.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_k01_darkscape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l01_escape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l02_garbage.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l03_agroprom.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l03u_agr_underground.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l04_darkvalley.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l04u_labx18.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l05_bar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l06_rostok.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l07_military.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l08_yantar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l08u_brainlab.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l10_radar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l10u_bunker.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l11_pripyat.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l12_stancia_2_new.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l12_stancia_new.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l12u_control_monolith.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_l12u_sarcofag.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_la15_darkscape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_labx10.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_labx8.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_limansk.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_lost_village.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_marsh.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_mine.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_pripyat.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_red_forest.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_warlab.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/alife_zaton.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/all.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/av_peshera.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/aver.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/dead_city.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/deadcity.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/generators.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/hospital.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/jupiter.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/jupiter_underground.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/k01_darkscape.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l01_escape.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l02_garbage.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l03_agroprom.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l03u_agr_underground.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l04_darkvalley.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l04u_labx18.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l05_bar.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l06_rostok.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l07_military.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l08_yantar.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l08u_brainlab.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l10_radar.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l10u_bunker.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l11_pripyat.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l12_stancia_2_new.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l12_stancia_new.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l12u_control_monolith.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/l12u_sarcofag.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/la15_darkscape.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/labx10.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/labx8.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/limansk.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/lost_village.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/marsh.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/mine.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/pripyat.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/red_forest.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/warlab.sections
- M all.spawn.improved_3.9.1_ogsr/unpacked/way__level_unknown.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_av_peshera.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_aver.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_dead_city.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_deadcity.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_generators.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_hospital.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_jupiter.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_jupiter_underground.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_k01_darkscape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l01_escape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l02_garbage.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l03_agroprom.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l03u_agr_underground.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l04_darkvalley.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l04u_labx18.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l05_bar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l06_rostok.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l07_military.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l08_yantar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l08u_brainlab.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l10_radar.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l10u_bunker.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l11_pripyat.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l12_stancia_2.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l12_stancia_2_new.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l12_stancia_new.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l12u_control_monolith.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_l12u_sarcofag.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_la15_darkscape.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_labx10.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_labx8.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_limansk.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_lost_village.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_marsh.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_mine.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_pripyat.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_red_forest.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_warlab.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/way_zaton.ltx
- M all.spawn.improved_3.9.1_ogsr/unpacked/zaton.sections
- M all.spawn_NLC.HE/unpacked/_level_unknown.sections
- M all.spawn_NLC.HE/unpacked/alife__level_unknown.ltx
- M all.spawn_NLC.HE/unpacked/alife_av_peshera.ltx
- M all.spawn_NLC.HE/unpacked/alife_aver.ltx
- M all.spawn_NLC.HE/unpacked/alife_dead_city.ltx
- M all.spawn_NLC.HE/unpacked/alife_generators.ltx
- M all.spawn_NLC.HE/unpacked/alife_hospital.ltx
- M all.spawn_NLC.HE/unpacked/alife_jupiter.ltx
- M all.spawn_NLC.HE/unpacked/alife_jupiter_underground.ltx
- M all.spawn_NLC.HE/unpacked/alife_l01_escape.ltx
- M all.spawn_NLC.HE/unpacked/alife_l02_garbage.ltx
- M all.spawn_NLC.HE/unpacked/alife_l03_agroprom.ltx
- M all.spawn_NLC.HE/unpacked/alife_l03u_agr_underground.ltx
- M all.spawn_NLC.HE/unpacked/alife_l04_darkvalley.ltx
- M all.spawn_NLC.HE/unpacked/alife_l04u_labx18.ltx
- M all.spawn_NLC.HE/unpacked/alife_l05_bar.ltx
- M all.spawn_NLC.HE/unpacked/alife_l06_rostok.ltx
- M all.spawn_NLC.HE/unpacked/alife_l07_military.ltx
- M all.spawn_NLC.HE/unpacked/alife_l08_yantar.ltx
- M all.spawn_NLC.HE/unpacked/alife_l08u_brainlab.ltx
- M all.spawn_NLC.HE/unpacked/alife_l10_radar.ltx
- M all.spawn_NLC.HE/unpacked/alife_l10u_bunker.ltx
- M all.spawn_NLC.HE/unpacked/alife_l11_pripyat.ltx
- M all.spawn_NLC.HE/unpacked/alife_l12u_control_monolith.ltx
- M all.spawn_NLC.HE/unpacked/alife_l12u_sarcofag.ltx
- M all.spawn_NLC.HE/unpacked/alife_la15_darkscape.ltx
- M all.spawn_NLC.HE/unpacked/alife_labx8.ltx
- M all.spawn_NLC.HE/unpacked/alife_limansk.ltx
- M all.spawn_NLC.HE/unpacked/alife_lost_village.ltx
- M all.spawn_NLC.HE/unpacked/alife_marsh.ltx
- M all.spawn_NLC.HE/unpacked/alife_mine.ltx
- M all.spawn_NLC.HE/unpacked/alife_pripyat.ltx
- M all.spawn_NLC.HE/unpacked/alife_red_forest.ltx
- M all.spawn_NLC.HE/unpacked/alife_warlab.ltx
- M all.spawn_NLC.HE/unpacked/alife_zaton.ltx
- M all.spawn_NLC.HE/unpacked/all.ltx
- M all.spawn_NLC.HE/unpacked/av_peshera.sections
- M all.spawn_NLC.HE/unpacked/aver.sections
- M all.spawn_NLC.HE/unpacked/dead_city.sections
- M all.spawn_NLC.HE/unpacked/generators.sections
- M all.spawn_NLC.HE/unpacked/hospital.sections
- M all.spawn_NLC.HE/unpacked/jupiter.sections
- M all.spawn_NLC.HE/unpacked/jupiter_underground.sections
- M all.spawn_NLC.HE/unpacked/l01_escape.sections
- M all.spawn_NLC.HE/unpacked/l02_garbage.sections
- M all.spawn_NLC.HE/unpacked/l03_agroprom.sections
- M all.spawn_NLC.HE/unpacked/l03u_agr_underground.sections
- M all.spawn_NLC.HE/unpacked/l04_darkvalley.sections
- M all.spawn_NLC.HE/unpacked/l04u_labx18.sections
- M all.spawn_NLC.HE/unpacked/l05_bar.sections
- M all.spawn_NLC.HE/unpacked/l06_rostok.sections
- M all.spawn_NLC.HE/unpacked/l07_military.sections
- M all.spawn_NLC.HE/unpacked/l08_yantar.sections
- M all.spawn_NLC.HE/unpacked/l08u_brainlab.sections
- M all.spawn_NLC.HE/unpacked/l10_radar.sections
- M all.spawn_NLC.HE/unpacked/l10u_bunker.sections
- M all.spawn_NLC.HE/unpacked/l11_pripyat.sections
- M all.spawn_NLC.HE/unpacked/l12u_control_monolith.sections
- M all.spawn_NLC.HE/unpacked/l12u_sarcofag.sections
- M all.spawn_NLC.HE/unpacked/la15_darkscape.sections
- M all.spawn_NLC.HE/unpacked/labx8.sections
- M all.spawn_NLC.HE/unpacked/limansk.sections
- M all.spawn_NLC.HE/unpacked/lost_village.sections
- M all.spawn_NLC.HE/unpacked/marsh.sections
- M all.spawn_NLC.HE/unpacked/mine.sections
- M all.spawn_NLC.HE/unpacked/pripyat.sections
- M all.spawn_NLC.HE/unpacked/red_forest.sections
- M all.spawn_NLC.HE/unpacked/warlab.sections
- M all.spawn_NLC.HE/unpacked/way__level_unknown.ltx
- M all.spawn_NLC.HE/unpacked/way_av_peshera.ltx
- M all.spawn_NLC.HE/unpacked/way_aver.ltx
- M all.spawn_NLC.HE/unpacked/way_dead_city.ltx
- M all.spawn_NLC.HE/unpacked/way_generators.ltx
- M all.spawn_NLC.HE/unpacked/way_hospital.ltx
- M all.spawn_NLC.HE/unpacked/way_jupiter.ltx
- M all.spawn_NLC.HE/unpacked/way_jupiter_underground.ltx
- M all.spawn_NLC.HE/unpacked/way_l01_escape.ltx
- M all.spawn_NLC.HE/unpacked/way_l02_garbage.ltx
- M all.spawn_NLC.HE/unpacked/way_l03_agroprom.ltx
- M all.spawn_NLC.HE/unpacked/way_l03u_agr_underground.ltx
- M all.spawn_NLC.HE/unpacked/way_l04_darkvalley.ltx
- M all.spawn_NLC.HE/unpacked/way_l04u_labx18.ltx
- M all.spawn_NLC.HE/unpacked/way_l05_bar.ltx
- M all.spawn_NLC.HE/unpacked/way_l06_rostok.ltx
- M all.spawn_NLC.HE/unpacked/way_l07_military.ltx
- M all.spawn_NLC.HE/unpacked/way_l08_yantar.ltx
- M all.spawn_NLC.HE/unpacked/way_l08u_brainlab.ltx
- M all.spawn_NLC.HE/unpacked/way_l10_radar.ltx
- M all.spawn_NLC.HE/unpacked/way_l10u_bunker.ltx
- M all.spawn_NLC.HE/unpacked/way_l11_pripyat.ltx
- M all.spawn_NLC.HE/unpacked/way_l12_stancia_2.ltx
- M all.spawn_NLC.HE/unpacked/way_l12u_control_monolith.ltx
- M all.spawn_NLC.HE/unpacked/way_l12u_sarcofag.ltx
- M all.spawn_NLC.HE/unpacked/way_la15_darkscape.ltx
- M all.spawn_NLC.HE/unpacked/way_labx8.ltx
- M all.spawn_NLC.HE/unpacked/way_limansk.ltx
- M all.spawn_NLC.HE/unpacked/way_lost_village.ltx
- M all.spawn_NLC.HE/unpacked/way_marsh.ltx
- M all.spawn_NLC.HE/unpacked/way_mine.ltx
- M all.spawn_NLC.HE/unpacked/way_pripyat.ltx
- M all.spawn_NLC.HE/unpacked/way_red_forest.ltx
- M all.spawn_NLC.HE/unpacked/way_warlab.ltx
- M all.spawn_NLC.HE/unpacked/way_zaton.ltx
- M all.spawn_NLC.HE/unpacked/zaton.sections
- M Опционально/arrival_anomaly_vfx/README.md
- M Опционально/arrival_anomaly_vfx/gamedata/config/misc/zone_buzz.ltx
- M Опционально/arrival_anomaly_vfx/gamedata/config/misc/zone_mincer.ltx
- M Опционально/arrival_anomaly_vfx/gamedata/config/misc/zone_mine_acidic.ltx
- M Опционально/arrival_anomaly_vfx/gamedata/config/misc/zone_mosquitobald.ltx
- M Опционально/arrival_anomaly_vfx/gamedata/config/misc/zone_witchesgalantine.ltx
- M Опционально/arrival_anomaly_vfx/gamedata/config/misc/zone_zharka_static.ltx
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/acidic_mine_hit.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/acidic_mine_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/hit/zone_acidic_mine_hit_1.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/hit/zone_acidic_mine_hit_1_08.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/hit/zone_acidic_mine_hit_1_09.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/hit/zone_acidic_mine_hit_1_10.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/hit/zone_acidic_mine_hit_1_11.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/acidic_mine_smok.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/anomaly_toxic_particle_test_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/studen_idle_bottom_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/studen_idle_bottom_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/studen_idle_bottom_02.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/studen_idle_bottom_12.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/studen_idle_bottom_glow_02.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic/effects/idle/studen_idle_bottom_shader.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic_more_smoke/acidic_more_smoke_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic_more_smoke/anomaly_toxic_particle_test_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic_more_smoke/nograv_dust_03.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic_more_smoke/zone_acidic_idle_trail_03.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic_more_smoke/zone_acidic_idle_trail_05_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/acidic_more_smoke/zone_acidic_idle_trail_05_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/anomaly_teleport_sphere_graviti_distort_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/body_tear_blood_polter_flash_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_06.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_08.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_1_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_new_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_new_03.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_new_04.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_new_05.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/electra2_flash_new_07.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/hit/gold_shield_dist_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/anomaly_teleport_sphere_graviti_distort_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/electra2_flash_new_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/expl_new_sparks_bottom_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/gold_idle_smoke_big_puff_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/studen_idle_bottom_04.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/studen_idle_bottom_glow_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/wish_granter_electric_smoke_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/zone_electric_mine_idle_disk.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/zone_electric_mine_idle_sparks.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/zone_electric_mine_idle_sparks_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/effects/idle/zone_electric_mine_idle_sparks_aggressive.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/electra_blowout.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/electra_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/electra/electra_idle_aggressive.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/hit/gravitational_blowout_leaves.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/hit/gravitational_we_rocks_burs,_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/hit/gravitational_we_rocks_burst2_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/hit/gravitational_we_rocks_outburst_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/hit/gravity_dust_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/hit/gravity_dust_0022.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_av_leaves.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_av_leaves_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_av_leaves_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_av_leaves_02.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_av_leaves_04.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_av_leaves_05.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/gravitational_we_rocks_burst2_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/nograv_dust_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/effects/idle/zone_graviti_mine_smoke.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/gravitational_blowout.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational/gravitational_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood1.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood2.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood4.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood5.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood6.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood7.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood7_child_od_dead.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood7_child_on_play.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_blood8.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blood_splash/sb_dust_burst.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_av_rocks_rise_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_distort_12.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_distort_3.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_distort_6.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_distort_9.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_shield_wave.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_vaccuuum.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_vacuum_12.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_vacuum_6.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_vacuum_dust_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_vacuum_dust_10.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_wave_distort.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravi_zaxvat_otbrosy00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravitational_leaves_burst.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravitational_we_rocks_burst2.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravitational_we_rocks_outburst.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dist_average_additional_05.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dust_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dust_21.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dust_22.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dust_23.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dust_24.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_dust_25.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/gravity_leaves_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/zone_graviti_mine_idle_leaves_06.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/blowout/zone_graviti_mine_idle_leaves_07.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_ground_dist.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_ground_dust.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_leaves_small.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_leaves_small_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_leaves_small_new.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_leaves_small_new_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/gravi_we_leaves_small_new_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/zone_graviti_mine_idle_leaves_,_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/effects/idle/zone_graviti_mine_idle_leaves_,_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/gravitational_average_blood_splash.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/gravitational_average_blowout.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_average/gravitational_average_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/awake/gold_dist_blast_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/awake/gold_shield_dist_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood1.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood2.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood4.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood5.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood6.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood7.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood7_child_od_dead.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood7_child_on_play.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_blood8.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blood_splash/sb_dust_burst.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_hit_rocks_outburst.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_hit_rocks_upper.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_hit_rocks_vacuum_hight10.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat-400_leaves_dist.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat004_leaves_00_dist.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat400_leaves_dist.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat_leaves_inner.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat_leaves_outter.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat_otbrosy.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat_wave_dist.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat_yadro_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravi_zaxvat_yadro_02.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravitat_ground_leaves_lift.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravitat_inner_sphere_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/graviti_steam_trigger_strong_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravity_dist_strong_04.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravity_dist_strong_08.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravity_dust_superstrong.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravity_dust_superstrong_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravity_dust_superstrong_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/gravity_dust_superstrong_02.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/blowout/zone_graviti_mine_idle_leaves_09.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravi_rocks1.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravi_rocks2.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravi_we_leaves_small_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravitat_bottom_dist_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravitat_dist_ring_single.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravitational_core_bubble2.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravitational_core_bubble_small.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/gravitational_ripples.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/nyah_idle_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/radar_leaves_damping_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/zone_graviti_leaves_pulsing.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/zone_graviti_mine_idle_leaves_,_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/effects/idle/zone_graviti_mine_idle_leaves_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/gravitational_strong_awake.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/gravitational_strong_blood_splash.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/gravitational_strong_blowout.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/gravitational_strong/gravitational_strong_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravi_hit_rocks_outburst_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravi_zaxvat_otbrosy_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravi_zaxvat_wave_dist_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravity_dist_strong_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravity_dist_strong_001.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravity_dust_superstrong_03.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravity_dust_superstrong_04.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravity_dust_superstrong_05.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/gravity_dust_superstrong_06.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/blowout/heat_sparks_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/anomaly_spiky_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/gravi_rocks2.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/gravi_rocks2_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/gravitational_ripples.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/heat_sparks_idle_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/radar_leaves_damping.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/zone_graviti_leaves_pulsing.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/effects/idle/zone_graviti_mine_idle_leaves.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/springboard_blowout.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/springboard/springboard_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/buble_distort_0222.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/buble_distort_03.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/campfire_big_sparks_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/campfire_flame_hd_smoke_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/campfire_flame_hd_smoke_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_blue_bottom.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_bottom_varible_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_line_dist_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_line_side_fire_01.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_line_side_fire_blue.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_line_side_fire_lol.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_line_side_fire_small.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_line_up_steam_07.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_thermal_idle_point_emiter.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_burst_thermal_idle_point_emiter1.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_line_core1_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_line_core1_bottm_blue.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/flame_start_ground_spread.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/graviti_steam_trigger_weak_232_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/monsters_fire_flame_burst_line_044.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/monsters_fire_flame_burst_line_06.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/monsters_fire_flame_burst_line_09.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/monsters_fire_flame_burst_line_aaaaa.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/nograv_dust_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/nograv_dust_03.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/zharka_big_smoke.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/zharka_dust.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/effects/zharka_ground_smoke_after.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/zharka_blowout.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka/zharka_idle.pg
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka_strong/smoke/campfire_hot_sparks_00.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka_strong/smoke/expl_hd_gasbarrel_new_26.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka_strong/smoke/expl_hd_handgrenade_31.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka_strong/smoke/zone_thermal_idle_point_emiter.pe
- M Опционально/arrival_anomaly_vfx/gamedata/particles/semitone/anomalies/zharka_strong/zharka_strong_idle.pg
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/ui/textures_descr/ui_watch.xml
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/ui/textures_descr/ui_watch_part.xml
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/ui/ui_cheat_watch.xml
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_1911.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_abakan.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_ak103.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_ak74.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_ak74u.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_ak_strelok.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_ak_strelok_rem.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_akm.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_apb.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_awm.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_beretta92.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_binoc.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_bizon-2.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_desert_eagle.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_flame.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_fn2000.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_fn_fnx.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_fn_scar.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_fort12.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_g36.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_gauss_pistol.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_gauss_rifle.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_glock17.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_groza.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_hk_m320.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_knife.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_m4a1.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mg36.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mosin.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mp133_short.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mp153.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mp153_short.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mp5.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_mp5_mod.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_oreh_shotgun.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_pb.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_pernach.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_pkm.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_pm.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_remington870.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_rpg7.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_rpk16.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_rpk74.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_saiga12_mod.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_saiga12c.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_sig220.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_sig552.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_sks.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_spas12.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_sv98.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_svd.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_svu.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_taurus.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_toz34.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_toz34_hunter.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_toz66.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_toz66_short.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_ump45.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_usp.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_val.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_vector.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_vintorez.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_vintorez_mod.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_vityaz.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_vpo101_hunter.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/w_walther.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/config/weapons/weapons.ltx
- M Опционально/оружие/WEAPONS_NLC30/gamedata/scripts/allcore_hide.script
- M Опционально/оружие/WEAPONS_NLC30/gamedata/scripts/kotovod/he_sleeves.script
- M Опционально/оружие/WEAPONS_NLC30/gamedata/scripts/kotovod/he_watch_ui.script
- M Опционально/оружие/WEAPONS_NLC30/gamedata/scripts/ogse/ogse_signals_addons_list.script
- M Опционально/оружие/WEAPONS_NLC30/gamedata/scripts/ui/ui_cheat_watch.script
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/ui/textures_descr/ui_watch.xml
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/ui/textures_descr/ui_watch_part.xml
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/ui/ui_cheat_watch.xml
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_1911.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_abakan.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_ak103.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_ak74.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_ak74u.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_ak_strelok.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_ak_strelok_rem.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_akm.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_apb.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_awm.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_beretta92.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_binoc.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_bizon-2.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_desert_eagle.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_flame.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_fn2000.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_fn_fnx.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_fn_scar.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_fort12.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_g36.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_gauss_pistol.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_gauss_rifle.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_groza.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_hk_m320.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_knife.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_m4a1.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_mg36.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_mp133_short.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_mp153.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_mp5.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_mp5_mod.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_oreh_shotgun.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_pb.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_pernach.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_pkm.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_pm.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_rpg7.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_rpk16.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_rpk74.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_saiga12_mod.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_saiga12c.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_sig220.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_sig552.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_sks.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_spas12.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_svd.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_svu.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_taurus.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_toz34.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_toz34_hunter.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_toz66.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_toz66_short.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_ump45.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_usp.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_val.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_vector.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_vintorez.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_vintorez_mod.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_vityaz.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_vpo101_hunter.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/w_walther.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/config/weapons/weapons.ltx
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/scripts/allcore_hide.script
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/scripts/kotovod/he_sleeves.script
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/scripts/kotovod/he_watch_ui.script
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/scripts/ogse/ogse_signals_addons_list.script
- M Опционально/оружие/WEAPONS_NLC_ALT/gamedata/scripts/ui/ui_cheat_watch.script
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/ambients.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/dynamic_weather_graphs.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/effects.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/sound_channels.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/suns.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/thunderbolt_collections.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/gamedata/config/environment/thunderbolts.ltx
- M Опционально/погода/WEATHER_CGIM_Origin/user.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/ambients.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/dynamic_weather_graphs.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/effects.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/sound_channels.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/suns.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/thunderbolt_collections.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/thunderbolts.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_katakomb.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_clear.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_clear_depressive.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_clear_mist.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_clear_no_far.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_clear_storm.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_default.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_depressive.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_groza.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_groza_depressive.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_groza_mist.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_groza_no_far.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_groza_storm.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_indoor.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_katakomb.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_mist.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_no_far.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_pasmurno.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_pasmurno_depressive.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_pasmurno_mist.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_pasmurno_no_far.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_pasmurno_storm.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_pripyat.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_radar.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_rain.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_rain_depressive.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_rain_level.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_rain_mist.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_rain_no_far.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_rain_storm.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_sarkofag.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_stancia.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_stancia2.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_storm.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_yantar.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_n7_yantar_indoor.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_np_thunder.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/default_pre_blowout.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/environment/weathers/indoor.ltx
- M Опционально/погода/WEATHER_N7_Original/gamedata/config/game_maps_single.ltx
- M Опционально/погода/WEATHER_N7_Original/user.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/ambients.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/dynamic_weather_graphs.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/effects.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/sound_channels.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/suns.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/thunderbolt_collections.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/thunderbolts.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_katakomb.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_np_thunder.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_pre_blowout.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_beta.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_clear.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_clear_depressive.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_dark.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_default.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_dry.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_foggy.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_grey.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_groza_depressive.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_indoor_x18.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_no_far.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_ogsm.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_pasmurno.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_pasmurno_depressive.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_pasmurno_october.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_pasmurno_storm.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_pripyat.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_radar.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_rain.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_rain_depressive.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_rainy.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_sarkofag.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_stancia.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_stancia2.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_stancia2_x.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_stancia_x.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_stormy.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_thunder.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_yantar.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/default_sa_yantar_indoor.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/environment/weathers/indoor.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/gamedata/config/game_maps_single.ltx
- M Опционально/погода/WEATHER_SA_NLC_Style/user.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/ambients.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/dynamic_weather_graphs.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/effects.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/sound_channels.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/suns.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/thunderbolt_collections.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/gamedata/config/environment/thunderbolts.ltx
- M Опционально/погода/WEATHER_SWTC_Call_Of_Pripyat/user.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/ambients.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/dynamic_weather_graphs.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/effects.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/sound_channels.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/suns.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/thunderbolt_collections.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/thunderbolts.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_katakomb.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_np_thunder.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_pre_blowout.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_vv_vivid.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_vv_vivid_cloudy.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_vv_vivid_foggy.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_vv_vivid_pure.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/default_vv_vivid_stormy.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/environment/weathers/indoor.ltx
- M Опционально/погода/WEATHER_VIVID/gamedata/config/game_maps_single.ltx
- M Опционально/погода/WEATHER_VIVID/user.ltx

<!-- nlc-changelog-commit: fcd632c83942f6e03115d656133441119898caf9 -->
## 19.09.26 23:31 МСК - Набор воды во фляжку

Commit: [fcd632c](https://github.com/jordan1227/improved_to_he/commit/fcd632c83942f6e03115d656133441119898caf9)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/vergas_items.ltx
- M gamedata/scripts/vergas/vergas_masks.script

<!-- nlc-changelog-commit: 50cf1e990b27504854c8d06b0d7022635d4aa9a7 -->
## 19.09.26 23:01 МСК - Разжигание костров

Commit: [50cf1e9](https://github.com/jordan1227/improved_to_he/commit/50cf1e990b27504854c8d06b0d7022635d4aa9a7)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ogse/ogse_campfire.script

<!-- nlc-changelog-commit: 08a6f674e6d1bd5b02f851b92a3ceafc59cd9121 -->
## 19.09.26 22:25 МСК - Правка диалога

Commit: [08a6f67](https://github.com/jordan1227/improved_to_he/commit/08a6f674e6d1bd5b02f851b92a3ceafc59cd9121)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/text/eng/sak_strings_six.xml
- M gamedata/config/text/rus/sak_strings_six.xml

<!-- nlc-changelog-commit: 4d82730ee637da8775f913bb85df039298657928 -->
## 19.09.26 19:22 МСК - Игнор для 2х экологов на Янтаре

Commit: [4d82730](https://github.com/jordan1227/improved_to_he/commit/4d82730ee637da8775f913bb85df039298657928)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/creatures/stalkers.ltx

<!-- nlc-changelog-commit: 71c48fc4431574b35ec4b85bc1e8283a249c9262 -->
## 19.09.26 18:53 МСК - Короткий сюжет

Commit: [71c48fc](https://github.com/jordan1227/improved_to_he/commit/71c48fc4431574b35ec4b85bc1e8283a249c9262)

### Description

Опция «Короткий сюжет» в настройках геймплея: после сюжетного сна у выхода
из лаборатории Х-6 игрок сразу переносится в Саркофаг и завершает игру
у Монолита. Сюжет между Радаром и ЧАЭС в прохождении не участвует.

Переключатель game_options.ShortStory, по умолчанию выключен.
Точка прибытия настраивается в config/misc/he_short_story.ltx.

### Game files changed

- A gamedata/config/misc/he_short_story.ltx
- M gamedata/config/text/eng/ui_st_mm.xml
- M gamedata/config/text/rus/ui_st_mm.xml
- M gamedata/config/ui/ui_mm_opt.xml
- A gamedata/scripts/he_short_story.script
- M gamedata/scripts/kotovod/game_options.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/ui/ui_mm_opt_gameplay.script
- M gamedata/scripts/ui/ui_mm_opt_main.script

<!-- nlc-changelog-commit: 70324a3aa540100142b4c9eecefa217a0f9a5332 -->
## 19.09.26 15:33 МСК - Не нравится  реализация, потом переделать

Commit: [70324a3](https://github.com/jordan1227/improved_to_he/commit/70324a3aa540100142b4c9eecefa217a0f9a5332)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/dialogs_yantar.xml
- M gamedata/config/text/rus/stable_dialogs_nlc30.xml
- M gamedata/scripts/kotovod.script
- M gamedata/scripts/kotovod/dsh_drag_drop.script
- M gamedata/scripts/kotovod/spawner.script
- M gamedata/scripts/trade_manager.script

<!-- nlc-changelog-commit: 4696458e708e19a6cc54be7b88ff5d32490f4034 -->
## 19.09.26 11:57 МСК - девайсы в спавнер

Commit: [4696458](https://github.com/jordan1227/improved_to_he/commit/4696458e708e19a6cc54be7b88ff5d32490f4034)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/kotovod/spawner.script
- M gamedata/scripts/sak/sak.script

<!-- nlc-changelog-commit: 864e7939a81b835a8817e0ef17b13653b9fc1b1d -->
## 19.09.26 06:58 МСК - Откат, все там ок было

Commit: [864e793](https://github.com/jordan1227/improved_to_he/commit/864e7939a81b835a8817e0ef17b13653b9fc1b1d)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_model.script
- M gamedata/scripts/ui/ui_cheat_pda.script

<!-- nlc-changelog-commit: d215b2776686ebf8a7503e445954abc359e4ddd0 -->
## 19.09.26 02:07 МСК - Upgrade PDA emission handling

Commit: [d215b27](https://github.com/jordan1227/improved_to_he/commit/d215b2776686ebf8a7503e445954abc359e4ddd0)

### Description

Allow upgraded PDAs to remain readable during emissions with reduced live-screen glitching while keeping ordinary PDA and early notes blocked. This preserves critical outage checks, legacy blowout gating, and adds regression coverage for upgraded versus ordinary PDA behavior.

### Game files changed

- M gamedata/scripts/binders/bind_stalker.script
- M gamedata/scripts/sivol/sivol_emp_config.script
- M gamedata/scripts/sivol/sivol_pda.script

### Other repository files changed

- M docs/PDA_CORRECTION_TEST_PLAN.md
- A docs/reviews/PDA_EMISSION_USABILITY.diff
- M docs/tests/pda_state_tests.lua

<!-- nlc-changelog-commit: 3899e011038ad4af4af31ada4233ec8e99af41f7 -->
## 18.09.26 20:27 МСК - Снятие ПНВ с костюмов

Commit: [3899e01](https://github.com/jordan1227/improved_to_he/commit/3899e011038ad4af4af31ada4233ec8e99af41f7)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/all_outfits_nlc.ltx
- M gamedata/scripts/binders/bind_stalker.script
- M gamedata/scripts/params/nvd_params.script

<!-- nlc-changelog-commit: 689c5d89c432020a72600f6a4732fef89f3b4ac0 -->
## 18.09.26 20:27 МСК - Приборы ночного видения как предметы

Commit: [689c5d8](https://github.com/jordan1227/improved_to_he/commit/689c5d89c432020a72600f6a4732fef89f3b4ac0)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/items.ltx
- M gamedata/config/text/eng/string_table_enc_equipment.xml
- M gamedata/config/text/rus/string_table_enc_equipment.xml

<!-- nlc-changelog-commit: a1cd126720f677dedc9c3625851cd31ae8b5c3d5 -->
## 18.09.26 20:27 МСК - Силуэт и иконки ПНВ

Commit: [a1cd126](https://github.com/jordan1227/improved_to_he/commit/a1cd126720f677dedc9c3625851cd31ae8b5c3d5)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/textures/ui/amk_ui_base_textur0.dds
- M gamedata/textures/ui/amk_ui_base_textur2.dds
- M gamedata/textures/ui/amk_ui_base_textur3.dds
- M gamedata/textures/ui/amk_ui_base_textur4.dds
- M gamedata/textures/ui/amk_ui_base_textur5.dds
- M gamedata/textures/ui/amk_ui_base_textur6.dds
- M gamedata/textures/ui/amk_ui_base_texture.dds
- A gamedata/textures/ui/ui_icon_equipment_12.dds

<!-- nlc-changelog-commit: 5a4b4f0fb3802de5a76cec76480a1b06a8d13296 -->
## 18.09.26 20:27 МСК - Слот ПНВ в инвентаре

Commit: [5a4b4f0](https://github.com/jordan1227/improved_to_he/commit/5a4b4f0fb3802de5a76cec76480a1b06a8d13296)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/ui/inventory_new.xml
- M gamedata/config/ui/inventory_new_16.xml

<!-- nlc-changelog-commit: 2e09614ef02924c4351b783e1892222ce3fcd06c -->
## 18.09.26 20:26 МСК - Локальные сообщения в pda_battery и vergas_lib

Commit: [2e09614](https://github.com/jordan1227/improved_to_he/commit/2e09614ef02924c4351b783e1892222ce3fcd06c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/pda_battery.script
- M gamedata/scripts/vergas/vergas_lib.script

<!-- nlc-changelog-commit: ffccc32b696644ba01af25f3412eced94007c538 -->
## 18.09.26 20:07 МСК - Зачет прочитанного ПДА

Commit: [ffccc32](https://github.com/jordan1227/improved_to_he/commit/ffccc32b696644ba01af25f3412eced94007c538)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_cheat_pda.script

<!-- nlc-changelog-commit: 3d1a83d96c2a5f9880fe9835875d74d066568e9b -->
## 18.09.26 19:59 МСК - Счетчик записей

Commit: [3d1a83d](https://github.com/jordan1227/improved_to_he/commit/3d1a83d96c2a5f9880fe9835875d74d066568e9b)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_cheat_pda.script

<!-- nlc-changelog-commit: b2f6df371b09a60bba0779e946d7436c3ef4ecdc -->
## 18.09.26 15:31 МСК - Временный пда

Commit: [b2f6df3](https://github.com/jordan1227/improved_to_he/commit/b2f6df371b09a60bba0779e946d7436c3ef4ecdc)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/items.ltx
- M gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/he_pda_market.script

<!-- nlc-changelog-commit: 88e58287f865ede81357792e96b2d0f313b67033 -->
## 18.09.26 15:18 МСК - Восстановление

Commit: [88e5828](https://github.com/jordan1227/improved_to_he/commit/88e58287f865ede81357792e96b2d0f313b67033)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/slot_keeper.script

<!-- nlc-changelog-commit: f1c204c84cc7f3e78723327194a586466a5858d3 -->
## 18.09.26 15:15 МСК - Восстановление

Commit: [f1c204c](https://github.com/jordan1227/improved_to_he/commit/f1c204c84cc7f3e78723327194a586466a5858d3)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/slot_keeper.script

<!-- nlc-changelog-commit: ba5865e9bc1b532a1e3c03b610451a07bdbad02d -->
## 18.09.26 15:10 МСК - Восстановление

Commit: [ba5865e](https://github.com/jordan1227/improved_to_he/commit/ba5865e9bc1b532a1e3c03b610451a07bdbad02d)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_3d.script

<!-- nlc-changelog-commit: 7251e5268cebbcf5b5dbf1ba466cfff5ff77f50b -->
## 18.09.26 14:31 МСК - Восстановление

Commit: [7251e52](https://github.com/jordan1227/improved_to_he/commit/7251e5268cebbcf5b5dbf1ba466cfff5ff77f50b)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_3d.script

<!-- nlc-changelog-commit: 3edac2134285511ac2a5e78d6e23b140c71b8d32 -->
## 18.09.26 14:11 МСК - Осталось поправить восстановление оружие и его доставание из 4 мода рюкзака

Commit: [3edac21](https://github.com/jordan1227/improved_to_he/commit/3edac2134285511ac2a5e78d6e23b140c71b8d32)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_cheat_pda.script

<!-- nlc-changelog-commit: 738afce9b537450ecfed7e2568b88436da32d0e1 -->
## 18.09.26 13:53 МСК - Update ui_cheat_pda.script

Commit: [738afce](https://github.com/jordan1227/improved_to_he/commit/738afce9b537450ecfed7e2568b88436da32d0e1)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_cheat_pda.script

<!-- nlc-changelog-commit: d8917622d60082da3a00b34cce865fb33e2a2eb3 -->
## 18.09.26 13:44 МСК - Пока так с ПДА, нужно много дорабатывать с переключением, восстановлением оружия

Commit: [d891762](https://github.com/jordan1227/improved_to_he/commit/d8917622d60082da3a00b34cce865fb33e2a2eb3)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/kotovod/dsh_drag_drop.script
- M gamedata/scripts/slot_keeper.script
- M gamedata/scripts/trade_manager.script
- M gamedata/scripts/ui/ui_cheat_pda.script
- M gamedata/scripts/ui/ui_cheat_pda_menu.script

<!-- nlc-changelog-commit: c5da435a6414a2cade4a8daea8a22c7ecd94ace5 -->
## 17.09.26 21:15 МСК - pda

Commit: [c5da435](https://github.com/jordan1227/improved_to_he/commit/c5da435a6414a2cade4a8daea8a22c7ecd94ace5)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/ui/ui_cheat_pda_msg.xml
- M gamedata/config/ui/ui_cheat_pda_npc.xml
- M gamedata/config/ui/ui_cheat_pda_npc_eng.xml
- M gamedata/scripts/ui/ui_cheat_pda.script

<!-- nlc-changelog-commit: 18ce007d5793bc92143c75d70e71eea33ab14ec1 -->
## 16.09.26 15:42 МСК - 3Д взлом

Commit: [18ce007](https://github.com/jordan1227/improved_to_he/commit/18ce007d5793bc92143c75d70e71eea33ab14ec1)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/ui/ui_cheat_pda_msg.xml
- M gamedata/config/ui/ui_cheat_pda_npc.xml
- M gamedata/config/ui/ui_cheat_pda_npc_eng.xml
- A gamedata/scripts/he_pda_3d.script
- M gamedata/scripts/ui/ui_cheat_pda.script
- M gamedata/scripts/ui/ui_cheat_pda_menu.script
- M gamedata/scripts/vergas/vergas_lib.script
- M gamedata/scripts/vergas/vergas_pda.script

<!-- nlc-changelog-commit: 07fef4d526fa5dd46184be83b6957ad50b717704 -->
## 12.09.26 01:39 МСК - Update ui_inv_descr.script

Commit: [07fef4d](https://github.com/jordan1227/improved_to_he/commit/07fef4d526fa5dd46184be83b6957ad50b717704)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_inv_descr.script

<!-- nlc-changelog-commit: 403450ddb05058fd3178f9603a14c81959c355de -->
## 12.09.26 01:20 МСК - Tune trade UI and aim speed

Commit: [403450d](https://github.com/jordan1227/improved_to_he/commit/403450ddb05058fd3178f9603a14c81959c355de)

### Description

This change repositions the trade cost label in the 16:9 item panel, slightly speeds up red-dot scope aim times, and disables the base-cost line in the inventory description while trading. The goal is to keep the trade UI aligned and improve weapon responsiveness during aimed firing.

### Game files changed

- M gamedata/config/ui/trade_item_16.xml
- M gamedata/scripts/sivol/sivol_weapon_aim_speed.script
- M gamedata/scripts/ui/ui_inv_descr.script

<!-- nlc-changelog-commit: f274b55d47a870150a72e2decb6577143a57df80 -->
## 11.09.26 23:30 МСК - ogse_signals

Commit: [f274b55](https://github.com/jordan1227/improved_to_he/commit/f274b55d47a870150a72e2decb6577143a57df80)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ogse/ogse_signals_addons_list.script

<!-- nlc-changelog-commit: 00e5e710557fa1bf7fb0f64de6fdc942f74a36aa -->
## 11.09.26 23:30 МСК - bind_physic_object

Commit: [00e5e71](https://github.com/jordan1227/improved_to_he/commit/00e5e710557fa1bf7fb0f64de6fdc942f74a36aa)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/binders/bind_physic_object.script

<!-- nlc-changelog-commit: 9c89fc27ebf6bfbb0a44984bb3ed7b903ae9b488 -->
## 11.09.26 23:30 МСК - Диалоги

Commit: [9c89fc2](https://github.com/jordan1227/improved_to_he/commit/9c89fc27ebf6bfbb0a44984bb3ed7b903ae9b488)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/text/eng/sak_strings_six.xml
- M gamedata/config/text/rus/sak_strings_six.xml

<!-- nlc-changelog-commit: dd11e4764c10d77cbc7dc6af451db1abee95bae6 -->
## 11.09.26 23:30 МСК - Поршни

Commit: [dd11e47](https://github.com/jordan1227/improved_to_he/commit/dd11e4764c10d77cbc7dc6af451db1abee95bae6)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/sak_dalogs_bar_yan.xml
- M gamedata/config/gameplay/sak_dalogs_esc_gar.xml

<!-- nlc-changelog-commit: 4be4cb97964638a20f9b4213750948d1d70979c2 -->
## 11.09.26 23:30 МСК - box, dialogs.script

Commit: [4be4cb9](https://github.com/jordan1227/improved_to_he/commit/4be4cb97964638a20f9b4213750948d1d70979c2)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/dialogs/dialogs.script
- M gamedata/scripts/ui/ui_he_magic_box.script

<!-- nlc-changelog-commit: 0ce78bb9154636f8e052540a42e6358848ccc9c8 -->
## 11.09.26 23:29 МСК - he_sidor_scam

Commit: [0ce78bb](https://github.com/jordan1227/improved_to_he/commit/0ce78bb9154636f8e052540a42e6358848ccc9c8)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/he_info_list.script
- M gamedata/scripts/he_magic_box.script
- A gamedata/scripts/he_sidor_scam.script

<!-- nlc-changelog-commit: 99e2d03e0b844df55c073165e60d0e9611f4275d -->
## 11.09.26 23:26 МСК - Поршни

Commit: [99e2d03](https://github.com/jordan1227/improved_to_he/commit/99e2d03e0b844df55c073165e60d0e9611f4275d)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/info_portions_nlc30.xml

<!-- nlc-changelog-commit: 1a32f073369baf3370711a3b922fe849d84b799e -->
## 11.09.26 23:26 МСК - Диалоги

Commit: [1a32f07](https://github.com/jordan1227/improved_to_he/commit/1a32f073369baf3370711a3b922fe849d84b799e)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/gameplay/dialogs_escape.xml
- M gamedata/config/gameplay/dialogs_trading.xml

<!-- nlc-changelog-commit: 1515b22af4512d47ff0f1cae141f7f6f62e7f14d -->
## 11.09.26 20:10 МСК - Update ui_inv_descr.script

Commit: [1515b22](https://github.com/jordan1227/improved_to_he/commit/1515b22af4512d47ff0f1cae141f7f6f62e7f14d)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/ui/ui_inv_descr.script

<!-- nlc-changelog-commit: 31293e99ea7a7c9e6796f8b9da61fd48d5109f08 -->
## 11.09.26 20:09 МСК - Improve inventory help and layout

Commit: [31293e9](https://github.com/jordan1227/improved_to_he/commit/31293e99ea7a7c9e6796f8b9da61fd48d5109f08)

### Description

Refine the inventory description UX by fixing localized hyphen characters, adding drag-and-drop interaction hints for reloads, battery swaps, sharpening, flask use, and related actions, and widening the description panels to fit longer help text. The patch also adjusts compact outfit property layout, makes LAlt help labels more readable, and cleans up duplicate/legacy hint handling in the inventory script.

### Game files changed

- M gamedata/config/text/eng/ui_st_inventory.xml
- M gamedata/config/text/rus/ui_st_inventory.xml
- M gamedata/config/ui/inventory_item_16.xml
- M gamedata/config/ui/ui_inv_descr.xml
- M gamedata/config/ui/ui_inv_descr_16.xml
- M gamedata/scripts/ui/ui_inv_descr.script

<!-- nlc-changelog-commit: 8a241d99eb8287ffd322bd4821e02629378d61f7 -->
## 11.09.26 15:58 МСК - Tune weapon ready-motion speeds

Commit: [8a241d9](https://github.com/jordan1227/improved_to_he/commit/8a241d99eb8287ffd322bd4821e02629378d61f7)

### Description

This change adjusts draw/holster and ready-state animation speed multipliers across the weapon config set for smoother HUD behavior and more consistent timing. It also hardens inventory description weapon metrics by resolving ready-motion aliases across empty/misfire/grenade states, caching effective metrics to avoid repeated recomputation, and guarding against stale item references when swapping addons or item instances.

### Game files changed

- M gamedata/config/weapons/w_1911.ltx
- M gamedata/config/weapons/w_abakan.ltx
- M gamedata/config/weapons/w_ak103.ltx
- M gamedata/config/weapons/w_ak104.ltx
- M gamedata/config/weapons/w_ak74.ltx
- M gamedata/config/weapons/w_ak74u.ltx
- M gamedata/config/weapons/w_ak_strelok.ltx
- M gamedata/config/weapons/w_ak_strelok_rem.ltx
- M gamedata/config/weapons/w_akm.ltx
- M gamedata/config/weapons/w_apb.ltx
- M gamedata/config/weapons/w_awm.ltx
- M gamedata/config/weapons/w_beretta92.ltx
- M gamedata/config/weapons/w_bizon-2.ltx
- M gamedata/config/weapons/w_colt_m45a1.ltx
- M gamedata/config/weapons/w_desert_eagle.ltx
- M gamedata/config/weapons/w_flame.ltx
- M gamedata/config/weapons/w_fn2000.ltx
- M gamedata/config/weapons/w_fn_fnx.ltx
- M gamedata/config/weapons/w_fn_scar.ltx
- M gamedata/config/weapons/w_fort12.ltx
- M gamedata/config/weapons/w_g36.ltx
- M gamedata/config/weapons/w_gauss_rifle.ltx
- M gamedata/config/weapons/w_glock17.ltx
- M gamedata/config/weapons/w_groza.ltx
- M gamedata/config/weapons/w_hk_m320.ltx
- M gamedata/config/weapons/w_m4a1.ltx
- M gamedata/config/weapons/w_mg36.ltx
- M gamedata/config/weapons/w_mosin.ltx
- M gamedata/config/weapons/w_mp133_short.ltx
- M gamedata/config/weapons/w_mp153.ltx
- M gamedata/config/weapons/w_mp153_short.ltx
- M gamedata/config/weapons/w_mp5.ltx
- M gamedata/config/weapons/w_mp5_mod.ltx
- M gamedata/config/weapons/w_oreh_shotgun.ltx
- M gamedata/config/weapons/w_pb.ltx
- M gamedata/config/weapons/w_pernach.ltx
- M gamedata/config/weapons/w_pkm.ltx
- M gamedata/config/weapons/w_pkm_mod.ltx
- M gamedata/config/weapons/w_pm.ltx
- M gamedata/config/weapons/w_remington870.ltx
- M gamedata/config/weapons/w_rg6.ltx
- M gamedata/config/weapons/w_rpg7.ltx
- M gamedata/config/weapons/w_rpk16.ltx
- M gamedata/config/weapons/w_rpk74.ltx
- M gamedata/config/weapons/w_sa58.ltx
- M gamedata/config/weapons/w_saiga12_mod.ltx
- M gamedata/config/weapons/w_saiga12c.ltx
- M gamedata/config/weapons/w_sig220.ltx
- M gamedata/config/weapons/w_sig552.ltx
- M gamedata/config/weapons/w_sks.ltx
- M gamedata/config/weapons/w_spas12.ltx
- M gamedata/config/weapons/w_sv98.ltx
- M gamedata/config/weapons/w_svd.ltx
- M gamedata/config/weapons/w_svu.ltx
- M gamedata/config/weapons/w_taurus.ltx
- M gamedata/config/weapons/w_toz34.ltx
- M gamedata/config/weapons/w_toz34_hunter.ltx
- M gamedata/config/weapons/w_toz66.ltx
- M gamedata/config/weapons/w_toz66_short.ltx
- M gamedata/config/weapons/w_ump45.ltx
- M gamedata/config/weapons/w_usp.ltx
- M gamedata/config/weapons/w_val.ltx
- M gamedata/config/weapons/w_vector.ltx
- M gamedata/config/weapons/w_vintorez.ltx
- M gamedata/config/weapons/w_vintorez_mod.ltx
- M gamedata/config/weapons/w_vityaz.ltx
- M gamedata/config/weapons/w_vpo101_hunter.ltx
- M gamedata/config/weapons/w_walther.ltx
- M gamedata/scripts/ui/ui_inv_descr.script

<!-- nlc-changelog-commit: 276055ecd3aad923af9a727b65e3a73958c66436 -->
## 11.09.26 11:06 МСК - Schedule kontainer_8 reward; pass artefact count

Commit: [276055e](https://github.com/jordan1227/improved_to_he/commit/276055ecd3aad923af9a727b65e3a73958c66436)

### Description

Add a scheduled reward for add_kontainer_8 by registering a delayed call to sak_dialog.add_kontainer_8_reward(), and move item creation into the new add_kontainer_8_reward function. Also adjust sidor_amk_art_not_have to pass an explicit count (1) to kotovod.gg_inventory_artefacts to ensure the artefact presence check is performed with the correct parameter.

### Game files changed

- M gamedata/scripts/sak/sak_dialog.script

<!-- nlc-changelog-commit: 01c3c465693771631c41a433483e3e3cbaf01349 -->
## 11.09.26 08:03 МСК - Initialize unknown artifacts on spawn

Commit: [01c3c46](https://github.com/jordan1227/improved_to_he/commit/01c3c465693771631c41a433483e3e3cbaf01349)

### Description

Adds a shared helper in sak_dialog.script to create unknown artifacts through sivol_unknown_arts initialization when treasure objects are spawned. This keeps generated af_unknown_* items consistent with the custom artifact setup instead of creating bare objects.

### Game files changed

- M gamedata/scripts/sak/sak_dialog.script

<!-- nlc-changelog-commit: 5b3570e5d8e0ef74c55ff12639c7521edb5f04ad -->
## 10.09.26 22:03 МСК - Репа.

Commit: [5b3570e](https://github.com/jordan1227/improved_to_he/commit/5b3570e5d8e0ef74c55ff12639c7521edb5f04ad)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/sak/sak_dialog.script
- M gamedata/scripts/task_manager.script
- M gamedata/scripts/xr/xr_motivator.script

<!-- nlc-changelog-commit: e02fa1888a706c1ce127e11c92cdc833cc74636a -->
## 10.09.26 16:14 МСК - Фикс

Commit: [e02fa18](https://github.com/jordan1227/improved_to_he/commit/e02fa1888a706c1ce127e11c92cdc833cc74636a)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/death_manager.script

<!-- nlc-changelog-commit: fa142fa01517034ac15c5a5a98c14c176342810e -->
## 10.09.26 00:51 МСК - Ящики из he

Commit: [fa142fa](https://github.com/jordan1227/improved_to_he/commit/fa142fa01517034ac15c5a5a98c14c176342810e)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/ph_box_generic.ltx
- M gamedata/config/misc/ph_box_items_by_communities.ltx
- M gamedata/config/misc/ph_box_items_by_levels.ltx
- M gamedata/config/misc/ph_box_items_count.ltx
- M gamedata/scripts/xr/xr_box.script

<!-- nlc-changelog-commit: 363bf0ca031ea2f43426297b32e022d0028bbf2c -->
## 10.09.26 00:25 МСК - esc_specnaz

Commit: [363bf0c](https://github.com/jordan1227/improved_to_he/commit/363bf0ca031ea2f43426297b32e022d0028bbf2c)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/config/misc/gulag_escape.ltx
- M gamedata/scripts/gulag/gulag_escape.script

<!-- nlc-changelog-commit: d4ad2c2227d58463058fee9cf43047ee157edf30 -->
## 09.09.26 23:39 МСК - Спец. конт на ДТ.

Commit: [d4ad2c2](https://github.com/jordan1227/improved_to_he/commit/d4ad2c2227d58463058fee9cf43047ee157edf30)

### Description

_(No additional description.)_

### Game files changed

- A gamedata/scripts/kotovod/special_container_drop.script
- M gamedata/scripts/nlc_fixes.script
- M gamedata/scripts/ogse/ogse_signals_addons_list.script
- M gamedata/scripts/vergas/used_items.script
- M gamedata/scripts/vergas/vergas_lib.script

<!-- nlc-changelog-commit: 9c280b15b9afe0927d78bd2adc91efe2e3feeb73 -->
## 22.09.26 20:14 МСК - Вылет в диалоге при снятии гранат у Свободы

Commit: [9c280b1](https://github.com/jordan1227/improved_to_he/commit/9c280b15b9afe0927d78bd2adc91efe2e3feeb73)

### Description

_(No additional description.)_

### Game files changed

- M gamedata/scripts/kotovod.script
