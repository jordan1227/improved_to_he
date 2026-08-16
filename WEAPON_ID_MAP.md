# Weapon / Add-on ID Map

| Inventory field | Value |
| :-- | :-- |
| Repository source | `C:\\Games\\NLC_OGSR_HE files\\improved_to_he` |
| Weapon config scope | `gamedata/config/weapons/*.ltx` |
| Requested basename glob | `wpn_*.ltx` — 0 matching filenames found; all `.ltx` files in the weapon directory were scanned for weapon/add-on section headers |
| String source | `gamedata/config/text/rus/string_table_enc_weapons.xml` |
| Spawner source | `gamedata/scripts/kotovod/spawner.script` |
| `.ltx` files scanned | 82 |
| `wpn_*` section headers | 553 |
| `wpn_*_hud` support sections excluded from spawn map | 258 |
| Non-`_hud` `wpn_*` sections mapped | 295 |
| Supplemental non-`wpn_` scope sections mapped | 6 (`ac10632`, `microt1`, `specter`, `okp`, `rakurs`, `g28_scope`) |
| Spawner weapon/add-on references | 122 occurrences / 120 unique IDs |


## Step 1 — Section inventory

| base_id | variant_id | addon_type | source file | relationship / ambiguity note |
| :-- | :-- | :-- | :-- | :-- |
| — | ac10632 | scope | gamedata/config/weapons/add_scopes.ltx:167 | — |
| — | g28_scope | scope | gamedata/config/weapons/add_scopes.ltx:274 | — |
| — | microt1 | scope | gamedata/config/weapons/add_scopes.ltx:189 | — |
| — | okp | scope | gamedata/config/weapons/add_scopes.ltx:232 | — |
| — | rakurs | scope | gamedata/config/weapons/add_scopes.ltx:253 | — |
| — | specter | scope | gamedata/config/weapons/add_scopes.ltx:210 | — |
| — | wpn_addon_1p78 | scope | gamedata/config/weapons/add_scopes.ltx:24 | — |
| — | wpn_addon_1pso | scope | gamedata/config/weapons/add_scopes.ltx:2 | — |
| — | wpn_addon_cobra | scope | gamedata/config/weapons/add_scopes.ltx:47 | — |
| — | wpn_addon_eotech | scope | gamedata/config/weapons/add_scopes.ltx:97 | — |
| — | wpn_addon_g36l | scope | gamedata/config/weapons/add_scopes.ltx:69 | — |
| — | wpn_addon_grenade_launcher | underbarrel | gamedata/config/weapons/add_gl.ltx:2 | — |
| — | wpn_addon_grenade_launcher_ag36 | underbarrel | gamedata/config/weapons/add_gl.ltx:48 | — |
| — | wpn_addon_grenade_launcher_groza | underbarrel | gamedata/config/weapons/add_gl.ltx:68 | — |
| — | wpn_addon_grenade_launcher_m203 | underbarrel | gamedata/config/weapons/add_gl.ltx:26 | — |
| — | wpn_addon_pe | scope | gamedata/config/weapons/add_scopes.ltx:119 | — |
| — | wpn_addon_po | scope | gamedata/config/weapons/add_scopes.ltx:143 | — |
| — | wpn_addon_sil_45_p | silencer | gamedata/config/weapons/add_silencers.ltx:191 | — |
| — | wpn_addon_sil_545_ar | silencer | gamedata/config/weapons/add_silencers.ltx:30 | — |
| — | wpn_addon_sil_556_ar | silencer | gamedata/config/weapons/add_silencers.ltx:57 | — |
| — | wpn_addon_sil_762_ar | silencer | gamedata/config/weapons/add_silencers.ltx:84 | — |
| — | wpn_addon_sil_762_sr1 | silencer | gamedata/config/weapons/add_silencers.ltx:138 | — |
| — | wpn_addon_sil_762_sr2 | silencer | gamedata/config/weapons/add_silencers.ltx:111 | — |
| — | wpn_addon_sil_9_p | silencer | gamedata/config/weapons/add_silencers.ltx:2 | — |
| — | wpn_addon_sil_apb | silencer | gamedata/config/weapons/add_silencers.ltx:242 | — |
| — | wpn_addon_sil_co_salvo | silencer | gamedata/config/weapons/add_silencers.ltx:297 | — |
| — | wpn_addon_sil_glock | silencer | gamedata/config/weapons/add_silencers.ltx:216 | — |
| — | wpn_addon_sil_groza | silencer | gamedata/config/weapons/add_silencers.ltx:322 | — |
| — | wpn_addon_sil_mosin | silencer | gamedata/config/weapons/add_silencers.ltx:165 | — |
| — | wpn_addon_sil_vsk | silencer | gamedata/config/weapons/add_silencers.ltx:269 | — |
| — | wpn_fake_missile | internal projectile | gamedata/config/weapons/weapons.ltx:1488 | — |
| — | wpn_fake_missile1 | internal projectile | gamedata/config/weapons/weapons.ltx:1589 | — |
| — | wpn_fake_missile2 | internal projectile | gamedata/config/weapons/weapons.ltx:1705 | — |
| — | wpn_rpg7_missile | internal projectile | gamedata/config/weapons/weapons.ltx:1360 | — |
| wpn_abakan | — | base weapon | gamedata/config/weapons/w_abakan.ltx:2 | — |
| wpn_abakan | wpn_abakan_1p78 | scope | gamedata/config/weapons/w_abakan.ltx:360 | — |
| wpn_abakan | wpn_abakan_ekp | scope | gamedata/config/weapons/w_abakan.ltx:299 | — |
| wpn_abakan | wpn_abakan_okp | scope | gamedata/config/weapons/w_abakan.ltx:320 | — |
| wpn_abakan | wpn_abakan_pso | scope | gamedata/config/weapons/w_abakan.ltx:386 | — |
| wpn_abakan | wpn_abakan_rakurs | scope | gamedata/config/weapons/w_abakan.ltx:341 | — |
| wpn_aek971 | — | base weapon | gamedata/config/weapons/w_ak104.ltx:2 | — |
| wpn_aek971 | wpn_ak74u_snag_1p78 | scope | gamedata/config/weapons/w_ak104.ltx:391 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_ac10632 | scope | gamedata/config/weapons/w_ak104.ltx:249 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_ekp | scope | gamedata/config/weapons/w_ak104.ltx:321 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_eot | scope | gamedata/config/weapons/w_ak104.ltx:273 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_microt1 | scope | gamedata/config/weapons/w_ak104.ltx:297 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_okp | scope | gamedata/config/weapons/w_ak104.ltx:346 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_pso | scope | gamedata/config/weapons/w_ak104.ltx:425 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_aek971 | wpn_ak74u_snag_rakurs | scope | gamedata/config/weapons/w_ak104.ltx:366 | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` |
| wpn_ak104 | — | base weapon | gamedata/config/weapons/w_ak103.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_ak104 | wpn_ak103_1p78 | scope | gamedata/config/weapons/w_ak103.ltx:337 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` |
| wpn_ak104 | wpn_ak103_ekp | scope | gamedata/config/weapons/w_ak103.ltx:269 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` |
| wpn_ak104 | wpn_ak103_okp | scope | gamedata/config/weapons/w_ak103.ltx:293 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` |
| wpn_ak104 | wpn_ak103_pso | scope | gamedata/config/weapons/w_ak103.ltx:366 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` |
| wpn_ak104 | wpn_ak103_rakurs | scope | gamedata/config/weapons/w_ak103.ltx:312 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` |
| wpn_ak108 | — | base weapon | gamedata/config/weapons/w_rpk16.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_ak108 | wpn_rpk16_ac10632 | scope | gamedata/config/weapons/w_rpk16.ltx:162 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` |
| wpn_ak108 | wpn_rpk16_eot | scope | gamedata/config/weapons/w_rpk16.ltx:181 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` |
| wpn_ak108 | wpn_rpk16_microt1 | scope | gamedata/config/weapons/w_rpk16.ltx:201 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` |
| wpn_ak108 | wpn_rpk16_specter | scope | gamedata/config/weapons/w_rpk16.ltx:221 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` |
| wpn_ak47 | — | base weapon | gamedata/config/weapons/w_sks.ltx:2 | — |
| wpn_ak47 | wpn_ak47_old | named variant | gamedata/config/weapons/w_sks.ltx:373 | — |
| wpn_ak47 | wpn_sks_1p78 | scope | gamedata/config/weapons/w_sks.ltx:347 | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` |
| wpn_ak47 | wpn_sks_ekp | scope | gamedata/config/weapons/w_sks.ltx:259 | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` |
| wpn_ak47 | wpn_sks_okp | scope | gamedata/config/weapons/w_sks.ltx:280 | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` |
| wpn_ak47 | wpn_sks_pso | scope | gamedata/config/weapons/w_sks.ltx:322 | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` |
| wpn_ak47 | wpn_sks_rakurs | scope | gamedata/config/weapons/w_sks.ltx:301 | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` |
| wpn_ak74_m1 | — | base weapon | gamedata/config/weapons/w_ak_strelok.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74_m2` |
| wpn_ak74_m1 | wpn_ak74_m1_ac10632 | scope | gamedata/config/weapons/w_ak_strelok.ltx:79 | — |
| wpn_ak74_m1 | wpn_ak74_m1_eot | scope | gamedata/config/weapons/w_ak_strelok.ltx:87 | — |
| wpn_ak74_m1 | wpn_ak74_m1_microt1 | scope | gamedata/config/weapons/w_ak_strelok.ltx:93 | — |
| wpn_ak74_m1 | wpn_ak74_m1_specter | scope | gamedata/config/weapons/w_ak_strelok.ltx:99 | — |
| wpn_ak74_m2 | — | base weapon | gamedata/config/weapons/w_ak_strelok_rem.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_ak74_m2 | wpn_ak12_ac10632 | scope | gamedata/config/weapons/w_ak_strelok_rem.ltx:319 | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` |
| wpn_ak74_m2 | wpn_ak12_eot | scope | gamedata/config/weapons/w_ak_strelok_rem.ltx:338 | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` |
| wpn_ak74_m2 | wpn_ak12_microt1 | scope | gamedata/config/weapons/w_ak_strelok_rem.ltx:358 | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` |
| wpn_ak74_m2 | wpn_ak12_specter | scope | gamedata/config/weapons/w_ak_strelok_rem.ltx:378 | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` |
| wpn_ak74_m3 | — | base weapon | gamedata/config/weapons/w_colt_m45a1.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_colt1911` |
| wpn_ak74 | — | base weapon | gamedata/config/weapons/w_ak74.ltx:2 | — |
| wpn_ak74 | wpn_ak74_old | named variant | gamedata/config/weapons/w_ak74.ltx:433 | — |
| wpn_ak74m | — | base weapon | gamedata/config/weapons/w_ak74.ltx:302 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_ak74m | wpn_ak74m_1p78 | scope | gamedata/config/weapons/w_ak74.ltx:380 | — |
| wpn_ak74m | wpn_ak74m_ekp | scope | gamedata/config/weapons/w_ak74.ltx:318 | — |
| wpn_ak74m | wpn_ak74m_okp | scope | gamedata/config/weapons/w_ak74.ltx:338 | — |
| wpn_ak74m | wpn_ak74m_pso | scope | gamedata/config/weapons/w_ak74.ltx:405 | — |
| wpn_ak74m | wpn_ak74m_rakurs | scope | gamedata/config/weapons/w_ak74.ltx:359 | — |
| wpn_ak74m | wpn_ak74pbn | named variant | gamedata/config/weapons/w_ak74.ltx:431 | ambiguous relationship: ID stem `wpn_ak74pbn`, inheritance-based base `wpn_ak74m` |
| wpn_ak74u | — | base weapon | gamedata/config/weapons/w_ak74u.ltx:7 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_ak74u | wpn_ak74u_1p78 | scope | gamedata/config/weapons/w_ak74u.ltx:275 | — |
| wpn_ak74u | wpn_ak74u_ekp | scope | gamedata/config/weapons/w_ak74u.ltx:210 | — |
| wpn_ak74u | wpn_ak74u_okp | scope | gamedata/config/weapons/w_ak74u.ltx:233 | — |
| wpn_ak74u | wpn_ak74u_old | named variant | gamedata/config/weapons/w_ak74u.ltx:301 | — |
| wpn_ak74u | wpn_ak74u_rakurs | scope | gamedata/config/weapons/w_ak74u.ltx:254 | — |
| wpn_akm | — | base weapon | gamedata/config/weapons/w_akm.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_akm | wpn_akm_1p78 | scope | gamedata/config/weapons/w_akm.ltx:284 | — |
| wpn_akm | wpn_akm_ekp | scope | gamedata/config/weapons/w_akm.ltx:222 | — |
| wpn_akm | wpn_akm_okp | scope | gamedata/config/weapons/w_akm.ltx:242 | — |
| wpn_akm | wpn_akm_pso | scope | gamedata/config/weapons/w_akm.ltx:310 | — |
| wpn_akm | wpn_akm_rakurs | scope | gamedata/config/weapons/w_akm.ltx:263 | — |
| wpn_aks47 | — | base weapon | gamedata/config/weapons/w_vector.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_colt1911` |
| wpn_aks47 | wpn_vector_ac10632 | scope | gamedata/config/weapons/w_vector.ltx:158 | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` |
| wpn_aks47 | wpn_vector_eot | scope | gamedata/config/weapons/w_vector.ltx:177 | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` |
| wpn_aks47 | wpn_vector_microt1 | scope | gamedata/config/weapons/w_vector.ltx:196 | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` |
| wpn_aks47 | wpn_vector_specter | scope | gamedata/config/weapons/w_vector.ltx:216 | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` |
| wpn_aps | — | base weapon | gamedata/config/weapons/w_apb.ltx:2 | — |
| wpn_awm | — | base weapon | gamedata/config/weapons/w_awm.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_svd` |
| wpn_beretta | — | base weapon | gamedata/config/weapons/w_beretta92.ltx:2 | — |
| wpn_beretta93 | — | base weapon | gamedata/config/weapons/w_fn_fnx.ltx:2 | — |
| wpn_binoc | — | base device | gamedata/config/weapons/w_binoc.ltx:2 | — |
| wpn_bizon | — | base weapon | gamedata/config/weapons/w_bizon-2.ltx:2 | — |
| wpn_bizon | wpn_bizon_ekp | scope | gamedata/config/weapons/w_bizon-2.ltx:230 | — |
| wpn_bizon | wpn_bizon_okp | scope | gamedata/config/weapons/w_bizon-2.ltx:255 | — |
| wpn_bizon | wpn_bizon_rakurs | scope | gamedata/config/weapons/w_bizon-2.ltx:280 | — |
| wpn_bm16_full | — | base weapon | gamedata/config/weapons/w_toz66.ltx:2 | — |
| wpn_bm16 | — | base weapon | gamedata/config/weapons/w_toz66_short.ltx:2 | — |
| wpn_colt1911 | — | base weapon | gamedata/config/weapons/w_1911.ltx:2 | — |
| wpn_desert_eagle | — | base weapon | gamedata/config/weapons/w_desert_eagle.ltx:2 | — |
| wpn_desert_eagle | wpn_desert_eagle_po | named variant | gamedata/config/weapons/w_desert_eagle.ltx:230 | — |
| wpn_eagle_m1 | — | base weapon | gamedata/config/weapons/w_taurus.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_desert_eagle` |
| wpn_flame | — | base weapon | gamedata/config/weapons/w_flame.ltx:2 | — |
| wpn_fn2000 | — | base weapon | gamedata/config/weapons/w_fn2000.ltx:2 | — |
| wpn_fn2000 | wpn_fn2000_nimble_ac10632 | scope | gamedata/config/weapons/w_fn2000.ltx:300 | — |
| wpn_fn2000 | wpn_fn2000_nimble_eot | scope | gamedata/config/weapons/w_fn2000.ltx:318 | — |
| wpn_fn2000 | wpn_fn2000_nimble_microt1 | scope | gamedata/config/weapons/w_fn2000.ltx:336 | — |
| wpn_fn2000 | wpn_fn2000_nimble_specter | scope | gamedata/config/weapons/w_fn2000.ltx:354 | — |
| wpn_fort_m1 | — | base weapon | gamedata/config/weapons/w_pernach.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_fort` |
| wpn_fort | — | base weapon | gamedata/config/weapons/w_fort12.ltx:2 | — |
| wpn_g36 | — | base weapon | gamedata/config/weapons/w_g36.ltx:2 | — |
| wpn_g36c | — | base weapon | gamedata/config/weapons/w_mg36.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_g36` |
| wpn_g36c | wpn_g36_nimble_ac10632 | scope | gamedata/config/weapons/w_mg36.ltx:162 | — |
| wpn_g36c | wpn_g36_nimble_eot | scope | gamedata/config/weapons/w_mg36.ltx:179 | — |
| wpn_g36c | wpn_g36_nimble_microt1 | scope | gamedata/config/weapons/w_mg36.ltx:197 | — |
| wpn_g36c | wpn_g36_nimble_specter | scope | gamedata/config/weapons/w_mg36.ltx:215 | — |
| wpn_gauss_auto | — | base weapon | gamedata/config/weapons/w_gauss_rifle.ltx:219 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_gauss` |
| wpn_gauss | — | base weapon | gamedata/config/weapons/w_gauss_rifle.ltx:1 | — |
| wpn_glock_m1 | — | base weapon | gamedata/config/weapons/w_glock17.ltx:222 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_glock` |
| wpn_glock_m2 | — | base weapon | gamedata/config/weapons/w_glock17.ltx:281 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_glock` |
| wpn_glock_m2 | wpn_mp9_ac10632 | scope | gamedata/config/weapons/w_glock17.ltx:424 | ambiguous relationship: ID stem `wpn_mp9`, inheritance-based base `wpn_glock_m2` |
| wpn_glock_m2 | wpn_mp9_eot | scope | gamedata/config/weapons/w_glock17.ltx:444 | ambiguous relationship: ID stem `wpn_mp9`, inheritance-based base `wpn_glock_m2` |
| wpn_glock_m2 | wpn_mp9_microt1 | scope | gamedata/config/weapons/w_glock17.ltx:463 | ambiguous relationship: ID stem `wpn_mp9`, inheritance-based base `wpn_glock_m2` |
| wpn_glock | — | base weapon | gamedata/config/weapons/w_glock17.ltx:2 | — |
| wpn_gm94 | — | base weapon | gamedata/config/weapons/w_hk_m320.ltx:2 | — |
| wpn_groza_nimble | — | base weapon | gamedata/config/weapons/w_groza.ltx:306 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_groza` |
| wpn_groza_nimble | wpn_groza_nimble_ac10632 | scope | gamedata/config/weapons/w_groza.ltx:344 | — |
| wpn_groza_nimble | wpn_groza_nimble_eot | scope | gamedata/config/weapons/w_groza.ltx:364 | — |
| wpn_groza_nimble | wpn_groza_nimble_microt1 | scope | gamedata/config/weapons/w_groza.ltx:383 | — |
| wpn_groza_nimble | wpn_groza_nimble_specter | scope | gamedata/config/weapons/w_groza.ltx:402 | — |
| wpn_groza | — | base weapon | gamedata/config/weapons/w_groza.ltx:2 | — |
| wpn_gungauss | — | base weapon | gamedata/config/weapons/w_gauss_pistol.ltx:1 | — |
| wpn_hunters_toz_new | — | base weapon | gamedata/config/weapons/w_oreh_shotgun.ltx:1 | — |
| wpn_knife_m1 | — | base weapon | gamedata/config/weapons/w_knife.ltx:134 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_knife` |
| wpn_knife | — | base weapon | gamedata/config/weapons/w_knife.ltx:2 | — |
| wpn_m110 | — | base weapon | gamedata/config/weapons/w_fn_scar.ltx:2 | — |
| wpn_m110 | wpn_scarh_ac10632 | scope | gamedata/config/weapons/w_fn_scar.ltx:351 | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` |
| wpn_m110 | wpn_scarh_eot | scope | gamedata/config/weapons/w_fn_scar.ltx:369 | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` |
| wpn_m110 | wpn_scarh_microt1 | scope | gamedata/config/weapons/w_fn_scar.ltx:386 | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` |
| wpn_m110 | wpn_scarh_specter | scope | gamedata/config/weapons/w_fn_scar.ltx:403 | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` |
| wpn_m16a4 | — | base weapon | gamedata/config/weapons/w_m4a1.ltx:2 | — |
| wpn_m16a4 | wpn_lr300 | named variant | gamedata/config/weapons/w_m4a1.ltx:452 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` |
| wpn_m16a4 | wpn_lr300_ac10632 | scope | gamedata/config/weapons/w_m4a1.ltx:364 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` |
| wpn_m16a4 | wpn_lr300_eot | scope | gamedata/config/weapons/w_m4a1.ltx:382 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` |
| wpn_m16a4 | wpn_lr300_m1 | named variant | gamedata/config/weapons/w_m4a1.ltx:455 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` |
| wpn_m16a4 | wpn_lr300_microt1 | scope | gamedata/config/weapons/w_m4a1.ltx:401 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` |
| wpn_m16a4 | wpn_lr300_specter | scope | gamedata/config/weapons/w_m4a1.ltx:420 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` |
| wpn_mosin | — | base weapon | gamedata/config/weapons/w_mosin.ltx:2 | — |
| wpn_mosin | wpn_mosin_pu | named variant | gamedata/config/weapons/w_mosin.ltx:237 | — |
| wpn_mp153_m1 | — | base weapon | gamedata/config/weapons/w_mp153_short.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_winchester_m1` |
| wpn_mp153 | — | base weapon | gamedata/config/weapons/w_remington870.ltx:2 | — |
| wpn_mp153 | wpn_wincheaster1300_trapper_ac10632 | scope | gamedata/config/weapons/w_remington870.ltx:210 | ambiguous relationship: ID stem `wpn_wincheaster1300`, inheritance-based base `wpn_mp153` |
| wpn_mp153 | wpn_wincheaster1300_trapper_eot | scope | gamedata/config/weapons/w_remington870.ltx:228 | ambiguous relationship: ID stem `wpn_wincheaster1300`, inheritance-based base `wpn_mp153` |
| wpn_mp153 | wpn_wincheaster1300_trapper_microt1 | scope | gamedata/config/weapons/w_remington870.ltx:246 | ambiguous relationship: ID stem `wpn_wincheaster1300`, inheritance-based base `wpn_mp153` |
| wpn_mp5_m1 | — | base weapon | gamedata/config/weapons/w_mp5_mod.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_mp5` |
| wpn_mp5_m1 | wpn_mp5_m1_ac10632 | scope | gamedata/config/weapons/w_mp5_mod.ltx:45 | — |
| wpn_mp5_m1 | wpn_mp5_m1_eot | scope | gamedata/config/weapons/w_mp5_mod.ltx:52 | — |
| wpn_mp5_m1 | wpn_mp5_m1_microt1 | scope | gamedata/config/weapons/w_mp5_mod.ltx:58 | — |
| wpn_mp5_m1 | wpn_mp5_m1_specter | scope | gamedata/config/weapons/w_mp5_mod.ltx:64 | — |
| wpn_mp5_m2 | — | base weapon | gamedata/config/weapons/w_mp5_mod.ltx:73 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_mp5` |
| wpn_mp5_m2 | wpn_sr2m_ac10632 | scope | gamedata/config/weapons/w_mp5_mod.ltx:203 | ambiguous relationship: ID stem `wpn_sr2m`, inheritance-based base `wpn_mp5_m2` |
| wpn_mp5_m2 | wpn_sr2m_eot | scope | gamedata/config/weapons/w_mp5_mod.ltx:223 | ambiguous relationship: ID stem `wpn_sr2m`, inheritance-based base `wpn_mp5_m2` |
| wpn_mp5_m2 | wpn_sr2m_microt1 | scope | gamedata/config/weapons/w_mp5_mod.ltx:243 | ambiguous relationship: ID stem `wpn_sr2m`, inheritance-based base `wpn_mp5_m2` |
| wpn_mp5 | — | base weapon | gamedata/config/weapons/w_mp5.ltx:2 | — |
| wpn_mp5 | wpn_mp5_ac10632 | scope | gamedata/config/weapons/w_mp5.ltx:214 | — |
| wpn_mp5 | wpn_mp5_eot | scope | gamedata/config/weapons/w_mp5.ltx:234 | — |
| wpn_mp5 | wpn_mp5_microt1 | scope | gamedata/config/weapons/w_mp5.ltx:254 | — |
| wpn_mp5 | wpn_mp5_specter | scope | gamedata/config/weapons/w_mp5.ltx:273 | — |
| wpn_mr308 | — | base weapon | gamedata/config/weapons/w_fn_scar.ltx:438 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_m110` |
| wpn_mr308 | wpn_mr308_g28_scope | scope | gamedata/config/weapons/w_fn_scar.ltx:572 | — |
| wpn_pb | — | base weapon | gamedata/config/weapons/w_pb.ltx:2 | — |
| wpn_pkm | — | base weapon | gamedata/config/weapons/w_pkm.ltx:2 | — |
| wpn_pkp | — | base weapon | gamedata/config/weapons/w_pkm_mod.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_pkm` |
| wpn_pkp | wpn_pkm_zulus_ac10632 | scope | gamedata/config/weapons/w_pkm_mod.ltx:147 | ambiguous relationship: ID stem `wpn_pkm`, inheritance-based base `wpn_pkp` |
| wpn_pkp | wpn_pkm_zulus_eot | scope | gamedata/config/weapons/w_pkm_mod.ltx:165 | ambiguous relationship: ID stem `wpn_pkm`, inheritance-based base `wpn_pkp` |
| wpn_pkp | wpn_pkm_zulus_microt1 | scope | gamedata/config/weapons/w_pkm_mod.ltx:185 | ambiguous relationship: ID stem `wpn_pkm`, inheritance-based base `wpn_pkp` |
| wpn_pm | — | base weapon | gamedata/config/weapons/w_pm.ltx:2 | — |
| wpn_pp19 | — | base weapon | gamedata/config/weapons/w_vityaz.ltx:2 | — |
| wpn_pp19 | wpn_vityaz_ac10632 | scope | gamedata/config/weapons/w_vityaz.ltx:229 | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` |
| wpn_pp19 | wpn_vityaz_eot | scope | gamedata/config/weapons/w_vityaz.ltx:253 | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` |
| wpn_pp19 | wpn_vityaz_microt1 | scope | gamedata/config/weapons/w_vityaz.ltx:281 | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` |
| wpn_pp19 | wpn_vityaz_specter | scope | gamedata/config/weapons/w_vityaz.ltx:305 | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` |
| wpn_rg6 | — | base weapon | gamedata/config/weapons/w_rg6.ltx:1 | — |
| wpn_rpg7 | — | base weapon | gamedata/config/weapons/w_rpg7.ltx:2 | — |
| wpn_rpk | — | base weapon | gamedata/config/weapons/w_rpk74.ltx:311 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_rpk74` |
| wpn_rpk | wpn_rpk74m_1p78 | scope | gamedata/config/weapons/w_rpk74.ltx:488 | — |
| wpn_rpk | wpn_rpk74m_ekp | scope | gamedata/config/weapons/w_rpk74.ltx:426 | — |
| wpn_rpk | wpn_rpk74m_okp | scope | gamedata/config/weapons/w_rpk74.ltx:447 | — |
| wpn_rpk | wpn_rpk74m_pso | scope | gamedata/config/weapons/w_rpk74.ltx:513 | — |
| wpn_rpk | wpn_rpk74m_rakurs | scope | gamedata/config/weapons/w_rpk74.ltx:467 | — |
| wpn_rpk74 | — | base weapon | gamedata/config/weapons/w_rpk74.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` |
| wpn_rpk74 | wpn_rpk74_1p78 | scope | gamedata/config/weapons/w_rpk74.ltx:258 | — |
| wpn_rpk74 | wpn_rpk74_ekp | scope | gamedata/config/weapons/w_rpk74.ltx:198 | — |
| wpn_rpk74 | wpn_rpk74_okp | scope | gamedata/config/weapons/w_rpk74.ltx:216 | — |
| wpn_rpk74 | wpn_rpk74_pso | scope | gamedata/config/weapons/w_rpk74.ltx:283 | — |
| wpn_rpk74 | wpn_rpk74_rakurs | scope | gamedata/config/weapons/w_rpk74.ltx:236 | — |
| wpn_saiga12c_m1 | — | base weapon | gamedata/config/weapons/w_saiga12_mod.ltx:1 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_saiga12c` |
| wpn_saiga12c_m1 | wpn_saiga12m_ac10632 | scope | gamedata/config/weapons/w_saiga12_mod.ltx:154 | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` |
| wpn_saiga12c_m1 | wpn_saiga12m_eot | scope | gamedata/config/weapons/w_saiga12_mod.ltx:172 | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` |
| wpn_saiga12c_m1 | wpn_saiga12m_microt1 | scope | gamedata/config/weapons/w_saiga12_mod.ltx:190 | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` |
| wpn_saiga12c_m1 | wpn_saiga12m_specter | scope | gamedata/config/weapons/w_saiga12_mod.ltx:210 | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` |
| wpn_saiga12c_m2 | — | base weapon | gamedata/config/weapons/w_vpo101_hunter.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak47` |
| wpn_saiga12c_m2 | wpn_vpo101_1p78 | scope | gamedata/config/weapons/w_vpo101_hunter.ltx:281 | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` |
| wpn_saiga12c_m2 | wpn_vpo101_ekp | scope | gamedata/config/weapons/w_vpo101_hunter.ltx:212 | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` |
| wpn_saiga12c_m2 | wpn_vpo101_okp | scope | gamedata/config/weapons/w_vpo101_hunter.ltx:235 | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` |
| wpn_saiga12c_m2 | wpn_vpo101_pso | scope | gamedata/config/weapons/w_vpo101_hunter.ltx:185 | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` |
| wpn_saiga12c_m2 | wpn_vpo101_rakurs | scope | gamedata/config/weapons/w_vpo101_hunter.ltx:258 | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` |
| wpn_saiga12c | — | base weapon | gamedata/config/weapons/w_saiga12c.ltx:2 | — |
| wpn_saiga12c | wpn_saiga12_ekp | scope | gamedata/config/weapons/w_saiga12c.ltx:200 | — |
| wpn_saiga12c | wpn_saiga12_okp | scope | gamedata/config/weapons/w_saiga12c.ltx:227 | — |
| wpn_saiga12c | wpn_saiga12_rakurs | scope | gamedata/config/weapons/w_saiga12c.ltx:248 | — |
| wpn_sig220 | — | base weapon | gamedata/config/weapons/w_sig220.ltx:2 | — |
| wpn_sig550 | — | base weapon | gamedata/config/weapons/w_sig552.ltx:2 | — |
| wpn_sig550 | wpn_sig550_ac10632 | scope | gamedata/config/weapons/w_sig552.ltx:249 | — |
| wpn_sig550 | wpn_sig550_eot | scope | gamedata/config/weapons/w_sig552.ltx:266 | — |
| wpn_sig550 | wpn_sig550_microt1 | scope | gamedata/config/weapons/w_sig552.ltx:285 | — |
| wpn_sig550 | wpn_sig550_specter | scope | gamedata/config/weapons/w_sig552.ltx:304 | — |
| wpn_soc308_m1 | — | base weapon | gamedata/config/weapons/w_sa58.ltx:214 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_soc308` |
| wpn_soc308_m1 | wpn_soc308_m1_ac10632 | scope | gamedata/config/weapons/w_sa58.ltx:252 | — |
| wpn_soc308_m1 | wpn_soc308_m1_eot | scope | gamedata/config/weapons/w_sa58.ltx:269 | — |
| wpn_soc308_m1 | wpn_soc308_m1_microt1 | scope | gamedata/config/weapons/w_sa58.ltx:286 | — |
| wpn_soc308_m1 | wpn_soc308_m1_specter | scope | gamedata/config/weapons/w_sa58.ltx:303 | — |
| wpn_soc308 | — | base weapon | gamedata/config/weapons/w_sa58.ltx:4 | — |
| wpn_spas12_m1 | — | base weapon | gamedata/config/weapons/w_spas12.ltx:207 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_spas12` |
| wpn_spas12_m1 | wpn_spas12_nimble_ac10632 | scope | gamedata/config/weapons/w_spas12.ltx:343 | — |
| wpn_spas12_m1 | wpn_spas12_nimble_eot | scope | gamedata/config/weapons/w_spas12.ltx:362 | — |
| wpn_spas12_m1 | wpn_spas12_nimble_microt1 | scope | gamedata/config/weapons/w_spas12.ltx:381 | — |
| wpn_spas12 | — | base weapon | gamedata/config/weapons/w_spas12.ltx:2 | — |
| wpn_spas15_m1 | — | base weapon | gamedata/config/weapons/w_usas12_mod.ltx:2 | — |
| wpn_spas15_m1 | wpn_usas12_m1_ac10632 | scope | gamedata/config/weapons/w_usas12_mod.ltx:217 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` |
| wpn_spas15_m1 | wpn_usas12_m1_eot | scope | gamedata/config/weapons/w_usas12_mod.ltx:236 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` |
| wpn_spas15_m1 | wpn_usas12_m1_microt1 | scope | gamedata/config/weapons/w_usas12_mod.ltx:255 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` |
| wpn_spas15_m1 | wpn_usas12_m1_specter | scope | gamedata/config/weapons/w_usas12_mod.ltx:275 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` |
| wpn_spas15 | — | base weapon | gamedata/config/weapons/w_usas12.ltx:2 | — |
| wpn_sv98 | — | base weapon | gamedata/config/weapons/w_sv98.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_svd` |
| wpn_svd | — | base weapon | gamedata/config/weapons/w_svd.ltx:1 | — |
| wpn_svd | wpn_svd_1p78 | scope | gamedata/config/weapons/w_svd.ltx:302 | — |
| wpn_svd | wpn_svd_ekp | scope | gamedata/config/weapons/w_svd.ltx:232 | — |
| wpn_svd | wpn_svd_okp | scope | gamedata/config/weapons/w_svd.ltx:256 | — |
| wpn_svd | wpn_svd_pso | scope | gamedata/config/weapons/w_svd.ltx:329 | — |
| wpn_svd | wpn_svd_rakurs | scope | gamedata/config/weapons/w_svd.ltx:279 | — |
| wpn_svu | — | base weapon | gamedata/config/weapons/w_svu.ltx:1 | — |
| wpn_svu | wpn_svu_1p78 | scope | gamedata/config/weapons/w_svu.ltx:284 | — |
| wpn_svu | wpn_svu_ekp | scope | gamedata/config/weapons/w_svu.ltx:227 | — |
| wpn_svu | wpn_svu_okp | scope | gamedata/config/weapons/w_svu.ltx:246 | — |
| wpn_svu | wpn_svu_pso | scope | gamedata/config/weapons/w_svu.ltx:308 | — |
| wpn_svu | wpn_svu_rakurs | scope | gamedata/config/weapons/w_svu.ltx:265 | — |
| wpn_toz34_m1 | — | base weapon | gamedata/config/weapons/w_toz34_hunter.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_toz34` |
| wpn_toz34 | — | base weapon | gamedata/config/weapons/w_toz34.ltx:2 | — |
| wpn_ump45 | — | base weapon | gamedata/config/weapons/w_ump45.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_mp5` |
| wpn_ump45 | wpn_mp5_nimble_ac10632 | scope | gamedata/config/weapons/w_ump45.ltx:226 | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` |
| wpn_ump45 | wpn_mp5_nimble_eot | scope | gamedata/config/weapons/w_ump45.ltx:245 | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` |
| wpn_ump45 | wpn_mp5_nimble_microt1 | scope | gamedata/config/weapons/w_ump45.ltx:264 | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` |
| wpn_ump45 | wpn_mp5_nimble_specter | scope | gamedata/config/weapons/w_ump45.ltx:284 | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` |
| wpn_usp | — | base weapon | gamedata/config/weapons/w_usp.ltx:2 | — |
| wpn_val_nimble | — | base weapon | gamedata/config/weapons/w_val.ltx:331 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_val` |
| wpn_val_nimble | wpn_val_nimble_1p78 | scope | gamedata/config/weapons/w_val.ltx:498 | — |
| wpn_val_nimble | wpn_val_nimble_ac10632 | scope | gamedata/config/weapons/w_val.ltx:551 | — |
| wpn_val_nimble | wpn_val_nimble_ekp | scope | gamedata/config/weapons/w_val.ltx:421 | — |
| wpn_val_nimble | wpn_val_nimble_eot | scope | gamedata/config/weapons/w_val.ltx:576 | — |
| wpn_val_nimble | wpn_val_nimble_microt1 | scope | gamedata/config/weapons/w_val.ltx:600 | — |
| wpn_val_nimble | wpn_val_nimble_okp | scope | gamedata/config/weapons/w_val.ltx:448 | — |
| wpn_val_nimble | wpn_val_nimble_pso | scope | gamedata/config/weapons/w_val.ltx:525 | — |
| wpn_val_nimble | wpn_val_nimble_rakurs | scope | gamedata/config/weapons/w_val.ltx:473 | — |
| wpn_val | — | base weapon | gamedata/config/weapons/w_val.ltx:2 | — |
| wpn_val | wpn_val_1p78 | scope | gamedata/config/weapons/w_val.ltx:280 | — |
| wpn_val | wpn_val_ekp | scope | gamedata/config/weapons/w_val.ltx:205 | — |
| wpn_val | wpn_val_okp | scope | gamedata/config/weapons/w_val.ltx:230 | — |
| wpn_val | wpn_val_pso | scope | gamedata/config/weapons/w_val.ltx:306 | — |
| wpn_val | wpn_val_rakurs | scope | gamedata/config/weapons/w_val.ltx:255 | — |
| wpn_vintorez_m1 | — | base weapon | gamedata/config/weapons/w_vintorez_mod.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_vintorez` |
| wpn_vintorez_m1 | wpn_vintorez_nimble_ac10632 | scope | gamedata/config/weapons/w_vintorez_mod.ltx:100 | — |
| wpn_vintorez_m1 | wpn_vintorez_nimble_eot | scope | gamedata/config/weapons/w_vintorez_mod.ltx:123 | — |
| wpn_vintorez_m1 | wpn_vintorez_nimble_microt1 | scope | gamedata/config/weapons/w_vintorez_mod.ltx:147 | — |
| wpn_vintorez_m1 | wpn_vintorez_nimble_specter | scope | gamedata/config/weapons/w_vintorez_mod.ltx:171 | — |
| wpn_vintorez | — | base weapon | gamedata/config/weapons/w_vintorez.ltx:2 | — |
| wpn_vintorez | wpn_vintorez_1p78 | scope | gamedata/config/weapons/w_vintorez.ltx:292 | — |
| wpn_vintorez | wpn_vintorez_ekp | scope | gamedata/config/weapons/w_vintorez.ltx:216 | — |
| wpn_vintorez | wpn_vintorez_okp | scope | gamedata/config/weapons/w_vintorez.ltx:241 | — |
| wpn_vintorez | wpn_vintorez_pso | scope | gamedata/config/weapons/w_vintorez.ltx:318 | — |
| wpn_vintorez | wpn_vintorez_rakurs | scope | gamedata/config/weapons/w_vintorez.ltx:267 | — |
| wpn_walther | — | base weapon | gamedata/config/weapons/w_walther.ltx:2 | — |
| wpn_wincheaster1300 | — | base weapon | gamedata/config/weapons/w_mp133_short.ltx:2 | — |
| wpn_winchester_m1 | — | base weapon | gamedata/config/weapons/w_mp153.ltx:2 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_wincheaster1300` |


## Step 2 — Name and string resolution

### Effective `inv_name` references

| section_id | string_id | in-game name | mismatch flag | source file |
| :-- | :-- | :-- | :-- | :-- |
| ac10632 | st_addon_scope_ac10632 | Прицел Barska ELECTRO SIGHT AC10632 | no | gamedata/config/weapons/add_scopes.ltx:167 |
| g28_scope | st_addon_scope_g28 | Прицел HK Schmidt Bender | no | gamedata/config/weapons/add_scopes.ltx:274 |
| microt1 | st_addon_scope_micro-t1 | Прицел Aimpoint Micro T-1 | no | gamedata/config/weapons/add_scopes.ltx:189 |
| okp | st_addon_scope_okp | Прицел ОКП-7 | no | gamedata/config/weapons/add_scopes.ltx:232 |
| rakurs | st_addon_scope_rakurs | Прицел «Ракурс-А» | no | gamedata/config/weapons/add_scopes.ltx:253 |
| specter | st_addon_scope_specter | Прицел ELCAN SpecterDR | no | gamedata/config/weapons/add_scopes.ltx:210 |
| wpn_addon_1p78 | st_addon_scope_1p78 | Прицел 1П78 «Каштан» | no | gamedata/config/weapons/add_scopes.ltx:24 |
| wpn_addon_1pso | st_addon_scope_pso1 | Прицел ПСО-1М2 | no | gamedata/config/weapons/add_scopes.ltx:2 |
| wpn_addon_cobra | st_addon-ekp-8-02 | Прицел «Кобра» ЭКП-8-02 | no | gamedata/config/weapons/add_scopes.ltx:47 |
| wpn_addon_eotech | st_addon_scope_eotech | Прицел EOTech | no | gamedata/config/weapons/add_scopes.ltx:97 |
| wpn_addon_g36l | wpn-addon-g36l | Прицел G36 Red Dot | no | gamedata/config/weapons/add_scopes.ltx:69 |
| wpn_addon_grenade_launcher | wpn-addon-grenade-launcher | Подствольный гранатомет ГП-25 | no | gamedata/config/weapons/add_gl.ltx:2 |
| wpn_addon_grenade_launcher_ag36 | wpn_addon_grenade_launcher-ag36 | Подствольный гранатомет AG36 | no | gamedata/config/weapons/add_gl.ltx:48 |
| wpn_addon_grenade_launcher_groza | st_grenade_launcher_groza | Подствольный гранатомет ГП-5 | no | gamedata/config/weapons/add_gl.ltx:68 |
| wpn_addon_grenade_launcher_m203 | wpn-addon-grenade-launcher-m203 | Подствольный гранатомет M203 | no | gamedata/config/weapons/add_gl.ltx:26 |
| wpn_addon_pe | st_addon_scope_pu | Прицел ПУ | no | gamedata/config/weapons/add_scopes.ltx:119 |
| wpn_addon_po | wpn-addon-po | Прицел Aimpoint Micro | no | gamedata/config/weapons/add_scopes.ltx:143 |
| wpn_addon_sil_45_p | ars-sil_45_p | Глушитель 11,43 мм | no | gamedata/config/weapons/add_silencers.ltx:191 |
| wpn_addon_sil_545_ar | ars-sil_545_ar | Глушитель 5,45х39 | no | gamedata/config/weapons/add_silencers.ltx:30 |
| wpn_addon_sil_556_ar | ars-sil_556_ar | Глушитель 5,56х45 | no | gamedata/config/weapons/add_silencers.ltx:57 |
| wpn_addon_sil_762_ar | ars-sil_762_ar | Глушитель 7,62х39 | no | gamedata/config/weapons/add_silencers.ltx:84 |
| wpn_addon_sil_762_sr1 | ars-sil_762_sr1 | Глушитель 7,62х54 | no | gamedata/config/weapons/add_silencers.ltx:138 |
| wpn_addon_sil_762_sr2 | ars-sil_762_sr2 | Глушитель 7,62х51 | no | gamedata/config/weapons/add_silencers.ltx:111 |
| wpn_addon_sil_9_p | ars-sil_9_p | Глушитель 9 мм | no | gamedata/config/weapons/add_silencers.ltx:2 |
| wpn_addon_sil_apb | ars-sil_apb | Глушитель АПБ | no | gamedata/config/weapons/add_silencers.ltx:242 |
| wpn_addon_sil_co_salvo | inv_sil_co_salvo | Глушитель Salvo 12 | no | gamedata/config/weapons/add_silencers.ltx:297 |
| wpn_addon_sil_glock | ars-sil_glock | Глушитель Glock 18C | no | gamedata/config/weapons/add_silencers.ltx:216 |
| wpn_addon_sil_groza | st_addon_sil_groza | Глушитель 9х39 мм «Гроза» | no | gamedata/config/weapons/add_silencers.ltx:322 |
| wpn_addon_sil_mosin | ars-sillenser_mosin | Глушитель 7,62х54 R | no | gamedata/config/weapons/add_silencers.ltx:165 |
| wpn_addon_sil_vsk | ars-sil_vsk | Глушитель 9х39 мм «Вихрь» | no | gamedata/config/weapons/add_silencers.ltx:269 |
| wpn_fake_missile | ammo-vog-25 | Заряд ВОГ-25 | no | gamedata/config/weapons/weapons.ltx:1488 |
| wpn_fake_missile1 | ammo-vog-25p | Заряд ВГ-40И | no | gamedata/config/weapons/weapons.ltx:1589 |
| wpn_fake_missile2 | ammo-m209 | Заряд M406 | no | gamedata/config/weapons/weapons.ltx:1705 |
| wpn_rpg7_missile | "Og-7b" | UNRESOLVED | yes — string ID `"Og-7b"` is not present in string_table_enc_weapons.xml | gamedata/config/weapons/weapons.ltx:1360 |
| wpn_abakan | wpn-abakan | Автомат АН-94 | no | gamedata/config/weapons/w_abakan.ltx:2 |
| wpn_abakan_1p78 | wpn-abakan | Автомат АН-94 | no | gamedata/config/weapons/w_abakan.ltx:360 |
| wpn_abakan_ekp | wpn-abakan | Автомат АН-94 | no | gamedata/config/weapons/w_abakan.ltx:299 |
| wpn_abakan_okp | wpn-abakan | Автомат АН-94 | no | gamedata/config/weapons/w_abakan.ltx:320 |
| wpn_abakan_pso | wpn-abakan | Автомат АН-94 | no | gamedata/config/weapons/w_abakan.ltx:386 |
| wpn_abakan_rakurs | wpn-abakan | Автомат АН-94 | no | gamedata/config/weapons/w_abakan.ltx:341 |
| wpn_aek971 | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:2 |
| wpn_ak74u_snag_1p78 | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:391 |
| wpn_ak74u_snag_ac10632 | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:249 |
| wpn_ak74u_snag_ekp | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:321 |
| wpn_ak74u_snag_eot | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:273 |
| wpn_ak74u_snag_microt1 | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:297 |
| wpn_ak74u_snag_okp | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:346 |
| wpn_ak74u_snag_pso | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:425 |
| wpn_ak74u_snag_rakurs | wpn-aek971 | Автомат АК-104 «Тактик» | yes — section ID says AEK-971, but the resolved name is AK-104 «Тактик» | gamedata/config/weapons/w_ak104.ltx:366 |
| wpn_ak104 | wpn_ak103 | Автомат АК-103 | yes — section ID says AK-104, but the resolved name is AK-103 | gamedata/config/weapons/w_ak103.ltx:2 |
| wpn_ak103_1p78 | wpn_ak103 | Автомат АК-103 | yes — section ID says AK-104, but the resolved name is AK-103 | gamedata/config/weapons/w_ak103.ltx:337 |
| wpn_ak103_ekp | wpn_ak103 | Автомат АК-103 | yes — section ID says AK-104, but the resolved name is AK-103 | gamedata/config/weapons/w_ak103.ltx:269 |
| wpn_ak103_okp | wpn_ak103 | Автомат АК-103 | yes — section ID says AK-104, but the resolved name is AK-103 | gamedata/config/weapons/w_ak103.ltx:293 |
| wpn_ak103_pso | wpn_ak103 | Автомат АК-103 | yes — section ID says AK-104, but the resolved name is AK-103 | gamedata/config/weapons/w_ak103.ltx:366 |
| wpn_ak103_rakurs | wpn_ak103 | Автомат АК-103 | yes — section ID says AK-104, but the resolved name is AK-103 | gamedata/config/weapons/w_ak103.ltx:312 |
| wpn_ak108 | wpn_ak101 | Пулемёт РПК-16 | yes — section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | gamedata/config/weapons/w_rpk16.ltx:2 |
| wpn_rpk16_ac10632 | wpn_ak101 | Пулемёт РПК-16 | yes — section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | gamedata/config/weapons/w_rpk16.ltx:162 |
| wpn_rpk16_eot | wpn_ak101 | Пулемёт РПК-16 | yes — section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | gamedata/config/weapons/w_rpk16.ltx:181 |
| wpn_rpk16_microt1 | wpn_ak101 | Пулемёт РПК-16 | yes — section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | gamedata/config/weapons/w_rpk16.ltx:201 |
| wpn_rpk16_specter | wpn_ak101 | Пулемёт РПК-16 | yes — section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | gamedata/config/weapons/w_rpk16.ltx:221 |
| wpn_ak47 | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:2 |
| wpn_ak47_old | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:373 |
| wpn_sks_1p78 | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:347 |
| wpn_sks_ekp | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:259 |
| wpn_sks_okp | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:280 |
| wpn_sks_pso | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:322 |
| wpn_sks_rakurs | wpn-ak47 | Карабин Молот ОП-СКС | yes — section ID says AK-47, but the resolved name is Molot OP-SKS | gamedata/config/weapons/w_sks.ltx:301 |
| wpn_ak74_m1 | wpn-ak_strelok | Автомат Стрелка | no | gamedata/config/weapons/w_ak_strelok.ltx:2 |
| wpn_ak74_m1_ac10632 | wpn-ak_strelok | Автомат Стрелка | no | gamedata/config/weapons/w_ak_strelok.ltx:79 |
| wpn_ak74_m1_eot | wpn-ak_strelok | Автомат Стрелка | no | gamedata/config/weapons/w_ak_strelok.ltx:87 |
| wpn_ak74_m1_microt1 | wpn-ak_strelok | Автомат Стрелка | no | gamedata/config/weapons/w_ak_strelok.ltx:93 |
| wpn_ak74_m1_specter | wpn-ak_strelok | Автомат Стрелка | no | gamedata/config/weapons/w_ak_strelok.ltx:99 |
| wpn_ak74_m2 | wpn-ak_strelok1 | Автомат Стрелка М | no | gamedata/config/weapons/w_ak_strelok_rem.ltx:2 |
| wpn_ak12_ac10632 | wpn-ak_strelok1 | Автомат Стрелка М | no | gamedata/config/weapons/w_ak_strelok_rem.ltx:319 |
| wpn_ak12_eot | wpn-ak_strelok1 | Автомат Стрелка М | no | gamedata/config/weapons/w_ak_strelok_rem.ltx:338 |
| wpn_ak12_microt1 | wpn-ak_strelok1 | Автомат Стрелка М | no | gamedata/config/weapons/w_ak_strelok_rem.ltx:358 |
| wpn_ak12_specter | wpn-ak_strelok1 | Автомат Стрелка М | no | gamedata/config/weapons/w_ak_strelok_rem.ltx:378 |
| wpn_ak74_m3 | st_wpn_coltm45a1 | Пистолет Colt M45A1 | yes — section ID says AK-74 M3, but the resolved name is Colt M45A1 | gamedata/config/weapons/w_colt_m45a1.ltx:2 |
| wpn_ak74 | wpn-ak74 | Автомат АКС-74 | no | gamedata/config/weapons/w_ak74.ltx:2 |
| wpn_ak74_old | wpn-ak74 | Автомат АКС-74 | no | gamedata/config/weapons/w_ak74.ltx:433 |
| wpn_ak74m | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:302 |
| wpn_ak74m_1p78 | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:380 |
| wpn_ak74m_ekp | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:318 |
| wpn_ak74m_okp | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:338 |
| wpn_ak74m_pso | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:405 |
| wpn_ak74m_rakurs | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:359 |
| wpn_ak74pbn | wpn-ak74m | Автомат АКС-74Н | no | gamedata/config/weapons/w_ak74.ltx:431 |
| wpn_ak74u | wpn-ak74u | Автомат АКС-74УН | no | gamedata/config/weapons/w_ak74u.ltx:7 |
| wpn_ak74u_1p78 | wpn-ak74u | Автомат АКС-74УН | no | gamedata/config/weapons/w_ak74u.ltx:275 |
| wpn_ak74u_ekp | wpn-ak74u | Автомат АКС-74УН | no | gamedata/config/weapons/w_ak74u.ltx:210 |
| wpn_ak74u_okp | wpn-ak74u | Автомат АКС-74УН | no | gamedata/config/weapons/w_ak74u.ltx:233 |
| wpn_ak74u_old | wpn-ak74u | Автомат АКС-74УН | no | gamedata/config/weapons/w_ak74u.ltx:301 |
| wpn_ak74u_rakurs | wpn-ak74u | Автомат АКС-74УН | no | gamedata/config/weapons/w_ak74u.ltx:254 |
| wpn_akm | wpn-akm | Автомат АКМН | no | gamedata/config/weapons/w_akm.ltx:2 |
| wpn_akm_1p78 | wpn-akm | Автомат АКМН | no | gamedata/config/weapons/w_akm.ltx:284 |
| wpn_akm_ekp | wpn-akm | Автомат АКМН | no | gamedata/config/weapons/w_akm.ltx:222 |
| wpn_akm_okp | wpn-akm | Автомат АКМН | no | gamedata/config/weapons/w_akm.ltx:242 |
| wpn_akm_pso | wpn-akm | Автомат АКМН | no | gamedata/config/weapons/w_akm.ltx:310 |
| wpn_akm_rakurs | wpn-akm | Автомат АКМН | no | gamedata/config/weapons/w_akm.ltx:263 |
| wpn_aks47 | wpn-aks47 | Пистолет-пулемёт TDI KRISS Vector | yes — section/string ID says AKS47, but the resolved name is TDI KRISS Vector | gamedata/config/weapons/w_vector.ltx:2 |
| wpn_vector_ac10632 | wpn-aks47 | Пистолет-пулемёт TDI KRISS Vector | yes — section/string ID says AKS47, but the resolved name is TDI KRISS Vector | gamedata/config/weapons/w_vector.ltx:158 |
| wpn_vector_eot | wpn-aks47 | Пистолет-пулемёт TDI KRISS Vector | yes — section/string ID says AKS47, but the resolved name is TDI KRISS Vector | gamedata/config/weapons/w_vector.ltx:177 |
| wpn_vector_microt1 | wpn-aks47 | Пистолет-пулемёт TDI KRISS Vector | yes — section/string ID says AKS47, but the resolved name is TDI KRISS Vector | gamedata/config/weapons/w_vector.ltx:196 |
| wpn_vector_specter | wpn-aks47 | Пистолет-пулемёт TDI KRISS Vector | yes — section/string ID says AKS47, but the resolved name is TDI KRISS Vector | gamedata/config/weapons/w_vector.ltx:216 |
| wpn_aps | wpn-aps | Пистолет АПБ | yes — section ID says APS, but the resolved name is APB | gamedata/config/weapons/w_apb.ltx:2 |
| wpn_awm | wpn_awm | Винтовка AWM-F | no | gamedata/config/weapons/w_awm.ltx:2 |
| wpn_beretta | wpn-beretta | Пистолет Beretta M9A1 | no | gamedata/config/weapons/w_beretta92.ltx:2 |
| wpn_beretta93 | wpn-beretta93 | Пистолет FN FNX-45 | yes — section ID says Beretta 93, but the resolved name is FN FNX-45 | gamedata/config/weapons/w_fn_fnx.ltx:2 |
| wpn_binoc | binocular | UNRESOLVED | yes — string ID `binocular` is not present in string_table_enc_weapons.xml | gamedata/config/weapons/w_binoc.ltx:2 |
| wpn_bizon | wpn_bizon | Пистолет-пулемёт ПП-19 «Бизон» | no | gamedata/config/weapons/w_bizon-2.ltx:2 |
| wpn_bizon_ekp | wpn_bizon | Пистолет-пулемёт ПП-19 «Бизон» | no | gamedata/config/weapons/w_bizon-2.ltx:230 |
| wpn_bizon_okp | wpn_bizon | Пистолет-пулемёт ПП-19 «Бизон» | no | gamedata/config/weapons/w_bizon-2.ltx:255 |
| wpn_bizon_rakurs | wpn_bizon | Пистолет-пулемёт ПП-19 «Бизон» | no | gamedata/config/weapons/w_bizon-2.ltx:280 |
| wpn_bm16_full | wpn-bm16_full | Ружьё ТОЗ-66 | no | gamedata/config/weapons/w_toz66.ltx:2 |
| wpn_bm16 | wpn-bm16 | Обрез ТОЗ-66 | no | gamedata/config/weapons/w_toz66_short.ltx:2 |
| wpn_colt1911 | wpn-colt1911 | Пистолет Colt M1911A1 | no | gamedata/config/weapons/w_1911.ltx:2 |
| wpn_desert_eagle | wp_desert_eagle | Пистолет IMI Desert Eagle | no | gamedata/config/weapons/w_desert_eagle.ltx:2 |
| wpn_desert_eagle_po | wp_desert_eagle | Пистолет IMI Desert Eagle | no | gamedata/config/weapons/w_desert_eagle.ltx:230 |
| wpn_eagle_m1 | wpn-eagle_m1 | Револьвер Taurus Raging Bull | yes — section/string ID says Eagle M1, but the resolved name is Taurus Raging Bull | gamedata/config/weapons/w_taurus.ltx:2 |
| wpn_flame | wpn-flame_name | Огнемёт | no | gamedata/config/weapons/w_flame.ltx:2 |
| wpn_fn2000 | wpn-fn2000 | Автомат FN F2000 Tactical (FDE) | no | gamedata/config/weapons/w_fn2000.ltx:2 |
| wpn_fn2000_nimble_ac10632 | wpn-fn2000 | Автомат FN F2000 Tactical (FDE) | no | gamedata/config/weapons/w_fn2000.ltx:300 |
| wpn_fn2000_nimble_eot | wpn-fn2000 | Автомат FN F2000 Tactical (FDE) | no | gamedata/config/weapons/w_fn2000.ltx:318 |
| wpn_fn2000_nimble_microt1 | wpn-fn2000 | Автомат FN F2000 Tactical (FDE) | no | gamedata/config/weapons/w_fn2000.ltx:336 |
| wpn_fn2000_nimble_specter | wpn-fn2000 | Автомат FN F2000 Tactical (FDE) | no | gamedata/config/weapons/w_fn2000.ltx:354 |
| wpn_fort_m1 | wpn-fort_m1 | Пистолет ОЦ-33 «Пернач» | yes — section/string ID says Fort M1, but the resolved name is OTs-33 Pernach | gamedata/config/weapons/w_pernach.ltx:2 |
| wpn_fort | wpn-fort | Пистолет Форт-12 | no | gamedata/config/weapons/w_fort12.ltx:2 |
| wpn_g36 | wpn-g36 | Автомат G36 | no | gamedata/config/weapons/w_g36.ltx:2 |
| wpn_g36c | wpn-g36c | Пулемёт HK MG36 | yes — section ID says G36C, but the resolved name is HK MG36 | gamedata/config/weapons/w_mg36.ltx:2 |
| wpn_g36_nimble_ac10632 | wpn-g36c | Пулемёт HK MG36 | yes — section ID says G36C, but the resolved name is HK MG36 | gamedata/config/weapons/w_mg36.ltx:162 |
| wpn_g36_nimble_eot | wpn-g36c | Пулемёт HK MG36 | yes — section ID says G36C, but the resolved name is HK MG36 | gamedata/config/weapons/w_mg36.ltx:179 |
| wpn_g36_nimble_microt1 | wpn-g36c | Пулемёт HK MG36 | yes — section ID says G36C, but the resolved name is HK MG36 | gamedata/config/weapons/w_mg36.ltx:197 |
| wpn_g36_nimble_specter | wpn-g36c | Пулемёт HK MG36 | yes — section ID says G36C, but the resolved name is HK MG36 | gamedata/config/weapons/w_mg36.ltx:215 |
| wpn_gauss_auto | wpn-gauss | Гаусс-винтовка | no | gamedata/config/weapons/w_gauss_rifle.ltx:219 |
| wpn_gauss | wpn-gauss | Гаусс-винтовка | no | gamedata/config/weapons/w_gauss_rifle.ltx:1 |
| wpn_glock_m1 | wpn-glock17_m1 | Пистолет Glock 18C | yes — string ID says Glock 17 M1, but the resolved name is Glock 18C | gamedata/config/weapons/w_glock17.ltx:222 |
| wpn_glock_m2 | wpn-glock18 | Пистолет-пулемёт BT MP9 | yes — string ID says Glock 18, but the resolved name is BT MP9 | gamedata/config/weapons/w_glock17.ltx:281 |
| wpn_mp9_ac10632 | wpn-glock18 | Пистолет-пулемёт BT MP9 | yes — string ID says Glock 18, but the resolved name is BT MP9 | gamedata/config/weapons/w_glock17.ltx:424 |
| wpn_mp9_eot | wpn-glock18 | Пистолет-пулемёт BT MP9 | yes — string ID says Glock 18, but the resolved name is BT MP9 | gamedata/config/weapons/w_glock17.ltx:444 |
| wpn_mp9_microt1 | wpn-glock18 | Пистолет-пулемёт BT MP9 | yes — string ID says Glock 18, but the resolved name is BT MP9 | gamedata/config/weapons/w_glock17.ltx:463 |
| wpn_glock | wpn-glock17 | Пистолет Glock 17 | no | gamedata/config/weapons/w_glock17.ltx:2 |
| wpn_gm94 | wpn-gm94 | Гранатомёт HK M320 | no | gamedata/config/weapons/w_hk_m320.ltx:2 |
| wpn_groza_nimble | st_wpn_groza_nimble | Автомат ОЦ-14-4А «Шторм» | yes — string ID says Groza Nimble, but the resolved name is Groza «Шторм» | gamedata/config/weapons/w_groza.ltx:306 |
| wpn_groza_nimble_ac10632 | st_wpn_groza_nimble | Автомат ОЦ-14-4А «Шторм» | yes — string ID says Groza Nimble, but the resolved name is Groza «Шторм» | gamedata/config/weapons/w_groza.ltx:344 |
| wpn_groza_nimble_eot | st_wpn_groza_nimble | Автомат ОЦ-14-4А «Шторм» | yes — string ID says Groza Nimble, but the resolved name is Groza «Шторм» | gamedata/config/weapons/w_groza.ltx:364 |
| wpn_groza_nimble_microt1 | st_wpn_groza_nimble | Автомат ОЦ-14-4А «Шторм» | yes — string ID says Groza Nimble, but the resolved name is Groza «Шторм» | gamedata/config/weapons/w_groza.ltx:383 |
| wpn_groza_nimble_specter | st_wpn_groza_nimble | Автомат ОЦ-14-4А «Шторм» | yes — string ID says Groza Nimble, but the resolved name is Groza «Шторм» | gamedata/config/weapons/w_groza.ltx:402 |
| wpn_groza | wpn-groza | Автомат ОЦ-14-4А «Гроза» | no | gamedata/config/weapons/w_groza.ltx:2 |
| wpn_gungauss | wpn-gungauss | Гаусс-пистолет | no | gamedata/config/weapons/w_gauss_pistol.ltx:1 |
| wpn_hunters_toz_new | wpn_orex | Ружьё Ореха | yes — section ID says Hunters TOZ New, but the resolved name is Orex shotgun | gamedata/config/weapons/w_oreh_shotgun.ltx:1 |
| wpn_knife_m1 | st_kabar |  Нож KM2000 | no | gamedata/config/weapons/w_knife.ltx:134 |
| wpn_knife | st_knife | Нож 6Х4 | no | gamedata/config/weapons/w_knife.ltx:2 |
| wpn_m110 | wpn-m110 | Винтовка FN SCAR-H | yes — string ID says M110, but the resolved name is FN SCAR-H | gamedata/config/weapons/w_fn_scar.ltx:2 |
| wpn_scarh_ac10632 | wpn-m110 | Винтовка FN SCAR-H | yes — string ID says M110, but the resolved name is FN SCAR-H | gamedata/config/weapons/w_fn_scar.ltx:351 |
| wpn_scarh_eot | wpn-m110 | Винтовка FN SCAR-H | yes — string ID says M110, but the resolved name is FN SCAR-H | gamedata/config/weapons/w_fn_scar.ltx:369 |
| wpn_scarh_microt1 | wpn-m110 | Винтовка FN SCAR-H | yes — string ID says M110, but the resolved name is FN SCAR-H | gamedata/config/weapons/w_fn_scar.ltx:386 |
| wpn_scarh_specter | wpn-m110 | Винтовка FN SCAR-H | yes — string ID says M110, but the resolved name is FN SCAR-H | gamedata/config/weapons/w_fn_scar.ltx:403 |
| wpn_m16a4 | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:2 |
| wpn_lr300 | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:452 |
| wpn_lr300_ac10632 | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:364 |
| wpn_lr300_eot | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:382 |
| wpn_lr300_m1 | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:455 |
| wpn_lr300_microt1 | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:401 |
| wpn_lr300_specter | wpn-m4a1 | Автомат M4A1 | yes — section ID says M16A4, but the resolved name is M4A1 | gamedata/config/weapons/w_m4a1.ltx:420 |
| wpn_mosin | wpn_mosin | Винтовка Мосина | no | gamedata/config/weapons/w_mosin.ltx:2 |
| wpn_mosin_pu | wpn_mosin | Винтовка Мосина | no | gamedata/config/weapons/w_mosin.ltx:237 |
| wpn_mp153_m1 | wpn_mp153_m1 | Короткое ружьё МР-153 | no | gamedata/config/weapons/w_mp153_short.ltx:2 |
| wpn_mp153 | wpn-winchester_m1 | Ружьё Remington 870 (FD) | yes — string ID says Winchester M1, but the resolved name is Remington 870 | gamedata/config/weapons/w_remington870.ltx:2 |
| wpn_wincheaster1300_trapper_ac10632 | wpn-winchester_m1 | Ружьё Remington 870 (FD) | yes — string ID says Winchester M1, but the resolved name is Remington 870 | gamedata/config/weapons/w_remington870.ltx:210 |
| wpn_wincheaster1300_trapper_eot | wpn-winchester_m1 | Ружьё Remington 870 (FD) | yes — string ID says Winchester M1, but the resolved name is Remington 870 | gamedata/config/weapons/w_remington870.ltx:228 |
| wpn_wincheaster1300_trapper_microt1 | wpn-winchester_m1 | Ружьё Remington 870 (FD) | yes — string ID says Winchester M1, but the resolved name is Remington 870 | gamedata/config/weapons/w_remington870.ltx:246 |
| wpn_mp5_m1 | wpn-mp5_m1 | Пистолет-пулемёт HK MP5 9x18 | no | gamedata/config/weapons/w_mp5_mod.ltx:2 |
| wpn_mp5_m1_ac10632 | wpn-mp5_m1 | Пистолет-пулемёт HK MP5 9x18 | no | gamedata/config/weapons/w_mp5_mod.ltx:45 |
| wpn_mp5_m1_eot | wpn-mp5_m1 | Пистолет-пулемёт HK MP5 9x18 | no | gamedata/config/weapons/w_mp5_mod.ltx:52 |
| wpn_mp5_m1_microt1 | wpn-mp5_m1 | Пистолет-пулемёт HK MP5 9x18 | no | gamedata/config/weapons/w_mp5_mod.ltx:58 |
| wpn_mp5_m1_specter | wpn-mp5_m1 | Пистолет-пулемёт HK MP5 9x18 | no | gamedata/config/weapons/w_mp5_mod.ltx:64 |
| wpn_mp5_m2 | wpn-mp5_m2 | Пистолет-пулемёт СР-2МП «Вереск» | no | gamedata/config/weapons/w_mp5_mod.ltx:73 |
| wpn_sr2m_ac10632 | wpn-mp5_m2 | Пистолет-пулемёт СР-2МП «Вереск» | no | gamedata/config/weapons/w_mp5_mod.ltx:203 |
| wpn_sr2m_eot | wpn-mp5_m2 | Пистолет-пулемёт СР-2МП «Вереск» | no | gamedata/config/weapons/w_mp5_mod.ltx:223 |
| wpn_sr2m_microt1 | wpn-mp5_m2 | Пистолет-пулемёт СР-2МП «Вереск» | no | gamedata/config/weapons/w_mp5_mod.ltx:243 |
| wpn_mp5 | wpn-mp5 | Пистолет-пулемёт HK MP5 | no | gamedata/config/weapons/w_mp5.ltx:2 |
| wpn_mp5_ac10632 | wpn-mp5 | Пистолет-пулемёт HK MP5 | no | gamedata/config/weapons/w_mp5.ltx:214 |
| wpn_mp5_eot | wpn-mp5 | Пистолет-пулемёт HK MP5 | no | gamedata/config/weapons/w_mp5.ltx:234 |
| wpn_mp5_microt1 | wpn-mp5 | Пистолет-пулемёт HK MP5 | no | gamedata/config/weapons/w_mp5.ltx:254 |
| wpn_mp5_specter | wpn-mp5 | Пистолет-пулемёт HK MP5 | no | gamedata/config/weapons/w_mp5.ltx:273 |
| wpn_mr308 | wpn-mr308 | Винтовка HК MR308 HK G28 | no | gamedata/config/weapons/w_fn_scar.ltx:438 |
| wpn_mr308_g28_scope | wpn-mr308 | Винтовка HК MR308 HK G28 | no | gamedata/config/weapons/w_fn_scar.ltx:572 |
| wpn_pb | wpn-pb | Пистолет ПБ | no | gamedata/config/weapons/w_pb.ltx:2 |
| wpn_pkm | wpn-pkm_name | Пулемёт ПКМ | no | gamedata/config/weapons/w_pkm.ltx:2 |
| wpn_pkp | wpn-pkp | Пулемёт ПКМ Тактический | yes — string ID says PKP, but the resolved name is tactical PKM | gamedata/config/weapons/w_pkm_mod.ltx:2 |
| wpn_pkm_zulus_ac10632 | wpn-pkp | Пулемёт ПКМ Тактический | yes — string ID says PKP, but the resolved name is tactical PKM | gamedata/config/weapons/w_pkm_mod.ltx:147 |
| wpn_pkm_zulus_eot | wpn-pkp | Пулемёт ПКМ Тактический | yes — string ID says PKP, but the resolved name is tactical PKM | gamedata/config/weapons/w_pkm_mod.ltx:165 |
| wpn_pkm_zulus_microt1 | wpn-pkp | Пулемёт ПКМ Тактический | yes — string ID says PKP, but the resolved name is tactical PKM | gamedata/config/weapons/w_pkm_mod.ltx:185 |
| wpn_pm | wpn-pm | Пистолет ПМ | no | gamedata/config/weapons/w_pm.ltx:2 |
| wpn_pp19 | wpn-pp19 | Пистолет-пулемёт "Витязь-СН" | no | gamedata/config/weapons/w_vityaz.ltx:2 |
| wpn_vityaz_ac10632 | wpn-pp19 | Пистолет-пулемёт "Витязь-СН" | no | gamedata/config/weapons/w_vityaz.ltx:229 |
| wpn_vityaz_eot | wpn-pp19 | Пистолет-пулемёт "Витязь-СН" | no | gamedata/config/weapons/w_vityaz.ltx:253 |
| wpn_vityaz_microt1 | wpn-pp19 | Пистолет-пулемёт "Витязь-СН" | no | gamedata/config/weapons/w_vityaz.ltx:281 |
| wpn_vityaz_specter | wpn-pp19 | Пистолет-пулемёт "Витязь-СН" | no | gamedata/config/weapons/w_vityaz.ltx:305 |
| wpn_rg6 | wpn-rg-6 | Гранатомёт РГ-6 «Гном» | no | gamedata/config/weapons/w_rg6.ltx:1 |
| wpn_rpg7 | wpn-rpg7 | РПГ-7 | no | gamedata/config/weapons/w_rpg7.ltx:2 |
| wpn_rpk | wpn-rpk | Пулемёт РПК | no | gamedata/config/weapons/w_rpk74.ltx:311 |
| wpn_rpk74m_1p78 | wpn-rpk | Пулемёт РПК | no | gamedata/config/weapons/w_rpk74.ltx:488 |
| wpn_rpk74m_ekp | wpn-rpk | Пулемёт РПК | no | gamedata/config/weapons/w_rpk74.ltx:426 |
| wpn_rpk74m_okp | wpn-rpk | Пулемёт РПК | no | gamedata/config/weapons/w_rpk74.ltx:447 |
| wpn_rpk74m_pso | wpn-rpk | Пулемёт РПК | no | gamedata/config/weapons/w_rpk74.ltx:513 |
| wpn_rpk74m_rakurs | wpn-rpk | Пулемёт РПК | no | gamedata/config/weapons/w_rpk74.ltx:467 |
| wpn_rpk74 | wpn_rpk74 | Пулемёт РПК-74Н | no | gamedata/config/weapons/w_rpk74.ltx:2 |
| wpn_rpk74_1p78 | wpn_rpk74 | Пулемёт РПК-74Н | no | gamedata/config/weapons/w_rpk74.ltx:258 |
| wpn_rpk74_ekp | wpn_rpk74 | Пулемёт РПК-74Н | no | gamedata/config/weapons/w_rpk74.ltx:198 |
| wpn_rpk74_okp | wpn_rpk74 | Пулемёт РПК-74Н | no | gamedata/config/weapons/w_rpk74.ltx:216 |
| wpn_rpk74_pso | wpn_rpk74 | Пулемёт РПК-74Н | no | gamedata/config/weapons/w_rpk74.ltx:283 |
| wpn_rpk74_rakurs | wpn_rpk74 | Пулемёт РПК-74Н | no | gamedata/config/weapons/w_rpk74.ltx:236 |
| wpn_saiga12c_m1 | wpn-saiga12c_m1 | Карабин Сайга-12М | no | gamedata/config/weapons/w_saiga12_mod.ltx:1 |
| wpn_saiga12m_ac10632 | wpn-saiga12c_m1 | Карабин Сайга-12М | no | gamedata/config/weapons/w_saiga12_mod.ltx:154 |
| wpn_saiga12m_eot | wpn-saiga12c_m1 | Карабин Сайга-12М | no | gamedata/config/weapons/w_saiga12_mod.ltx:172 |
| wpn_saiga12m_microt1 | wpn-saiga12c_m1 | Карабин Сайга-12М | no | gamedata/config/weapons/w_saiga12_mod.ltx:190 |
| wpn_saiga12m_specter | wpn-saiga12c_m1 | Карабин Сайга-12М | no | gamedata/config/weapons/w_saiga12_mod.ltx:210 |
| wpn_saiga12c_m2 | wpn_soc94 | Карабин ВПО-101 «Вепрь Хантер» | yes — section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | gamedata/config/weapons/w_vpo101_hunter.ltx:2 |
| wpn_vpo101_1p78 | wpn_soc94 | Карабин ВПО-101 «Вепрь Хантер» | yes — section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | gamedata/config/weapons/w_vpo101_hunter.ltx:281 |
| wpn_vpo101_ekp | wpn_soc94 | Карабин ВПО-101 «Вепрь Хантер» | yes — section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | gamedata/config/weapons/w_vpo101_hunter.ltx:212 |
| wpn_vpo101_okp | wpn_soc94 | Карабин ВПО-101 «Вепрь Хантер» | yes — section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | gamedata/config/weapons/w_vpo101_hunter.ltx:235 |
| wpn_vpo101_pso | wpn_soc94 | Карабин ВПО-101 «Вепрь Хантер» | yes — section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | gamedata/config/weapons/w_vpo101_hunter.ltx:185 |
| wpn_vpo101_rakurs | wpn_soc94 | Карабин ВПО-101 «Вепрь Хантер» | yes — section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | gamedata/config/weapons/w_vpo101_hunter.ltx:258 |
| wpn_saiga12c | wpn-saiga12c | Карабин Сайга-12К | no | gamedata/config/weapons/w_saiga12c.ltx:2 |
| wpn_saiga12_ekp | wpn-saiga12c | Карабин Сайга-12К | no | gamedata/config/weapons/w_saiga12c.ltx:200 |
| wpn_saiga12_okp | wpn-saiga12c | Карабин Сайга-12К | no | gamedata/config/weapons/w_saiga12c.ltx:227 |
| wpn_saiga12_rakurs | wpn-saiga12c | Карабин Сайга-12К | no | gamedata/config/weapons/w_saiga12c.ltx:248 |
| wpn_sig220 | wpn-sig220 | Пистолет SIG-Sauer P220 | no | gamedata/config/weapons/w_sig220.ltx:2 |
| wpn_sig550 | wpn-sig550 | Автомат SIG SG 552 Commando | no | gamedata/config/weapons/w_sig552.ltx:2 |
| wpn_sig550_ac10632 | wpn-sig550 | Автомат SIG SG 552 Commando | no | gamedata/config/weapons/w_sig552.ltx:249 |
| wpn_sig550_eot | wpn-sig550 | Автомат SIG SG 552 Commando | no | gamedata/config/weapons/w_sig552.ltx:266 |
| wpn_sig550_microt1 | wpn-sig550 | Автомат SIG SG 552 Commando | no | gamedata/config/weapons/w_sig552.ltx:285 |
| wpn_sig550_specter | wpn-sig550 | Автомат SIG SG 552 Commando | no | gamedata/config/weapons/w_sig552.ltx:304 |
| wpn_soc308_m1 | wpn-soc308_m1 | Автомат DS Arms SA-58 custom | no | gamedata/config/weapons/w_sa58.ltx:214 |
| wpn_soc308_m1_ac10632 | wpn-soc308_m1 | Автомат DS Arms SA-58 custom | no | gamedata/config/weapons/w_sa58.ltx:252 |
| wpn_soc308_m1_eot | wpn-soc308_m1 | Автомат DS Arms SA-58 custom | no | gamedata/config/weapons/w_sa58.ltx:269 |
| wpn_soc308_m1_microt1 | wpn-soc308_m1 | Автомат DS Arms SA-58 custom | no | gamedata/config/weapons/w_sa58.ltx:286 |
| wpn_soc308_m1_specter | wpn-soc308_m1 | Автомат DS Arms SA-58 custom | no | gamedata/config/weapons/w_sa58.ltx:303 |
| wpn_soc308 | wpn-soc308 | Автомат DS Arms SA-58 | no | gamedata/config/weapons/w_sa58.ltx:4 |
| wpn_spas12_m1 | wpn-spas12_m1 | Ружьё Benelli M4 Super 90 (M1014) | yes — section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | gamedata/config/weapons/w_spas12.ltx:207 |
| wpn_spas12_nimble_ac10632 | wpn-spas12_m1 | Ружьё Benelli M4 Super 90 (M1014) | yes — section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | gamedata/config/weapons/w_spas12.ltx:343 |
| wpn_spas12_nimble_eot | wpn-spas12_m1 | Ружьё Benelli M4 Super 90 (M1014) | yes — section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | gamedata/config/weapons/w_spas12.ltx:362 |
| wpn_spas12_nimble_microt1 | wpn-spas12_m1 | Ружьё Benelli M4 Super 90 (M1014) | yes — section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | gamedata/config/weapons/w_spas12.ltx:381 |
| wpn_spas12 | wpn-spas12 | Ружьё Franchi SPAS-12 | no | gamedata/config/weapons/w_spas12.ltx:2 |
| wpn_spas15_m1 | wpn-spas15_m1 | Ружьё Daewoo USAS-12 М1 | yes — section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | gamedata/config/weapons/w_usas12_mod.ltx:2 |
| wpn_usas12_m1_ac10632 | wpn-spas15_m1 | Ружьё Daewoo USAS-12 М1 | yes — section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | gamedata/config/weapons/w_usas12_mod.ltx:217 |
| wpn_usas12_m1_eot | wpn-spas15_m1 | Ружьё Daewoo USAS-12 М1 | yes — section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | gamedata/config/weapons/w_usas12_mod.ltx:236 |
| wpn_usas12_m1_microt1 | wpn-spas15_m1 | Ружьё Daewoo USAS-12 М1 | yes — section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | gamedata/config/weapons/w_usas12_mod.ltx:255 |
| wpn_usas12_m1_specter | wpn-spas15_m1 | Ружьё Daewoo USAS-12 М1 | yes — section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | gamedata/config/weapons/w_usas12_mod.ltx:275 |
| wpn_spas15 | wpn-spas15 | Ружьё Daewoo USAS-12 | yes — section/string ID says SPAS-15, but the resolved name is Daewoo USAS-12 | gamedata/config/weapons/w_usas12.ltx:2 |
| wpn_sv98 | wpn_sv98 | Винтовка СВ-98М | no | gamedata/config/weapons/w_sv98.ltx:2 |
| wpn_svd | wpn-svd | Винтовка СВД | no | gamedata/config/weapons/w_svd.ltx:1 |
| wpn_svd_1p78 | wpn-svd | Винтовка СВД | no | gamedata/config/weapons/w_svd.ltx:302 |
| wpn_svd_ekp | wpn-svd | Винтовка СВД | no | gamedata/config/weapons/w_svd.ltx:232 |
| wpn_svd_okp | wpn-svd | Винтовка СВД | no | gamedata/config/weapons/w_svd.ltx:256 |
| wpn_svd_pso | wpn-svd | Винтовка СВД | no | gamedata/config/weapons/w_svd.ltx:329 |
| wpn_svd_rakurs | wpn-svd | Винтовка СВД | no | gamedata/config/weapons/w_svd.ltx:279 |
| wpn_svu | wpn-svu | Винтовка СВУ-А | no | gamedata/config/weapons/w_svu.ltx:1 |
| wpn_svu_1p78 | wpn-svu | Винтовка СВУ-А | no | gamedata/config/weapons/w_svu.ltx:284 |
| wpn_svu_ekp | wpn-svu | Винтовка СВУ-А | no | gamedata/config/weapons/w_svu.ltx:227 |
| wpn_svu_okp | wpn-svu | Винтовка СВУ-А | no | gamedata/config/weapons/w_svu.ltx:246 |
| wpn_svu_pso | wpn-svu | Винтовка СВУ-А | no | gamedata/config/weapons/w_svu.ltx:308 |
| wpn_svu_rakurs | wpn-svu | Винтовка СВУ-А | no | gamedata/config/weapons/w_svu.ltx:265 |
| wpn_toz34_m1 | wpn_hunters | Ружьё Охотника | yes — section ID says TOZ-34 M1, but the resolved name is Hunter shotgun | gamedata/config/weapons/w_toz34_hunter.ltx:2 |
| wpn_toz34 | wpn-toz34 | Ружьё ТОЗ-34 | no | gamedata/config/weapons/w_toz34.ltx:2 |
| wpn_ump45 | wpn-ump45 | Пистолет-пулемёт HK UMP45 | no | gamedata/config/weapons/w_ump45.ltx:2 |
| wpn_mp5_nimble_ac10632 | wpn-ump45 | Пистолет-пулемёт HK UMP45 | no | gamedata/config/weapons/w_ump45.ltx:226 |
| wpn_mp5_nimble_eot | wpn-ump45 | Пистолет-пулемёт HK UMP45 | no | gamedata/config/weapons/w_ump45.ltx:245 |
| wpn_mp5_nimble_microt1 | wpn-ump45 | Пистолет-пулемёт HK UMP45 | no | gamedata/config/weapons/w_ump45.ltx:264 |
| wpn_mp5_nimble_specter | wpn-ump45 | Пистолет-пулемёт HK UMP45 | no | gamedata/config/weapons/w_ump45.ltx:284 |
| wpn_usp | wpn_usp | Пистолет HK USP | no | gamedata/config/weapons/w_usp.ltx:2 |
| wpn_val_nimble | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:331 |
| wpn_val_nimble_1p78 | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:498 |
| wpn_val_nimble_ac10632 | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:551 |
| wpn_val_nimble_ekp | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:421 |
| wpn_val_nimble_eot | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:576 |
| wpn_val_nimble_microt1 | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:600 |
| wpn_val_nimble_okp | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:448 |
| wpn_val_nimble_pso | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:525 |
| wpn_val_nimble_rakurs | st_wpn_val_nimble | Автомат СР-3М «Вихрь» | yes — string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | gamedata/config/weapons/w_val.ltx:473 |
| wpn_val | wpn-val | Автомат АС «Вал» | no | gamedata/config/weapons/w_val.ltx:2 |
| wpn_val_1p78 | wpn-val | Автомат АС «Вал» | no | gamedata/config/weapons/w_val.ltx:280 |
| wpn_val_ekp | wpn-val | Автомат АС «Вал» | no | gamedata/config/weapons/w_val.ltx:205 |
| wpn_val_okp | wpn-val | Автомат АС «Вал» | no | gamedata/config/weapons/w_val.ltx:230 |
| wpn_val_pso | wpn-val | Автомат АС «Вал» | no | gamedata/config/weapons/w_val.ltx:306 |
| wpn_val_rakurs | wpn-val | Автомат АС «Вал» | no | gamedata/config/weapons/w_val.ltx:255 |
| wpn_vintorez_m1 | wpn-vintorez_m1 | Винтовка "ВССМ" | no | gamedata/config/weapons/w_vintorez_mod.ltx:2 |
| wpn_vintorez_nimble_ac10632 | wpn-vintorez_m1 | Винтовка "ВССМ" | no | gamedata/config/weapons/w_vintorez_mod.ltx:100 |
| wpn_vintorez_nimble_eot | wpn-vintorez_m1 | Винтовка "ВССМ" | no | gamedata/config/weapons/w_vintorez_mod.ltx:123 |
| wpn_vintorez_nimble_microt1 | wpn-vintorez_m1 | Винтовка "ВССМ" | no | gamedata/config/weapons/w_vintorez_mod.ltx:147 |
| wpn_vintorez_nimble_specter | wpn-vintorez_m1 | Винтовка "ВССМ" | no | gamedata/config/weapons/w_vintorez_mod.ltx:171 |
| wpn_vintorez | wpn-vintorez | Винтовка ВСС "Винторез" | no | gamedata/config/weapons/w_vintorez.ltx:2 |
| wpn_vintorez_1p78 | wpn-vintorez | Винтовка ВСС "Винторез" | no | gamedata/config/weapons/w_vintorez.ltx:292 |
| wpn_vintorez_ekp | wpn-vintorez | Винтовка ВСС "Винторез" | no | gamedata/config/weapons/w_vintorez.ltx:216 |
| wpn_vintorez_okp | wpn-vintorez | Винтовка ВСС "Винторез" | no | gamedata/config/weapons/w_vintorez.ltx:241 |
| wpn_vintorez_pso | wpn-vintorez | Винтовка ВСС "Винторез" | no | gamedata/config/weapons/w_vintorez.ltx:318 |
| wpn_vintorez_rakurs | wpn-vintorez | Винтовка ВСС "Винторез" | no | gamedata/config/weapons/w_vintorez.ltx:267 |
| wpn_walther | wpn-sig226 | Пистолет Walther P99 | no | gamedata/config/weapons/w_walther.ltx:2 |
| wpn_wincheaster1300 | wpn_mp133 | Короткое ружьё MP-133 | yes — section ID says Wincheaster 1300, but the resolved name is MP-133 | gamedata/config/weapons/w_mp133_short.ltx:2 |
| wpn_winchester_m1 | wpn_mp153 | Ружьё МР-153 | yes — section ID says Winchester M1, but the resolved name is MP-153 | gamedata/config/weapons/w_mp153.ltx:2 |


### Effective `description` references

| description string_id | actual `<text>` description | referenced section IDs |
| :-- | :-- | :-- |
| enc_arsenal_m16a4 | M4 - автоматический карабин, созданный в США на основе винтовки М16А2 и изначально предназначенный для вооружения экипажей боевых машин и расчётов вооружения и военной техники. Несмотря на это, Командование специальных операций США приняло этот карабин в качестве единого для всех американских сил специальных операций. В настоящее время на него перешла и вся Армия США. Основными отличиями М4 от М16А2 являются ствол меньшей длины и выдвижной телескопический приклад. Вариант Colt Model 921 (M4A1) отличается от М4 режимом стрельбы: SAFE/SEMI/AUTO, по всем остальным параметрам полностью идентичен М4. | wpn_m16a4, wpn_lr300_ac10632, wpn_lr300_eot, wpn_lr300_microt1, wpn_lr300_specter, wpn_lr300, wpn_lr300_m1 |
| enc_arsenal_sil_45_p | Глушитель для оружия под патрон 45 ACP. | wpn_addon_sil_45_p |
| enc_arsenal_sil_545_ar | Глушитель-пламегаситель тактический автоматный (ТГП-А) для оружия калибра 5,45х39 мм. | wpn_addon_sil_545_ar |
| enc_arsenal_sil_556_ar | Глушитель тактический автоматный стандарта NATO для оружия калибра 5,56х45 мм. | wpn_addon_sil_556_ar |
| enc_arsenal_sil_762_ar | Прибор для беззвучной и беспламенной стрельбы (ПБС-1) для оружия калибра из 7,62х39 мм | wpn_addon_sil_762_ar |
| enc_arsenal_sil_762_sr1 | Глушитель-пламегаситель тактический винтовочный (ТГП-В) для оружия калибра 7.62х54 мм. | wpn_addon_sil_762_sr1 |
| enc_arsenal_sil_762_sr2 | Глушитель тактический автоматный стандарта NATO для оружия калибра 7,62х51 мм. | wpn_addon_sil_762_sr2 |
| enc_arsenal_sil_9_p | Глушитель для оружия калибра 9 мм. | wpn_addon_sil_9_p |
| enc_arsenal_sil_apb | Глушитель специальный для автоматического пистолета Стечкина АПБ. | wpn_addon_sil_apb |
| enc_arsenal_sil_glock | Глушитель для пистолета Glock 18C. | wpn_addon_sil_glock |
| enc_arsenal_sil_vsk | Глушитель для СР-3М «Вихрь». | wpn_addon_sil_vsk |
| enc_arsenal_sillenser_mosin | Глушитель для винтовки Мосина. | wpn_addon_sil_mosin |
| enc_equipment_devic_binocul1 | UNRESOLVED — not present in string_table_enc_weapons.xml | wpn_binoc |
| enc_sil_co_salvo | Модератор звука выстрела от компании "SilencerCo" для карабинов и ружей 12 калибра. Излишне тяжел, но при этом - достаточно эффективен. Для установки требуется соответствующий переходник. | wpn_addon_sil_co_salvo |
| enc_weapons_wpn_ak103 | АК-103 - это производная версия АК-100 от АК-74М под патрон М43 7,62x39 мм, аналогичный АКМ. На АК-103 можно установить различные прицелы, в том числе ночные и оптические, а также штык-нож или гранатомет. Более новые версии могут устанавливаться на планки Пикатинни, что позволяет устанавливать больше аксессуаров. Там, где это возможно, в нем используются пластиковые компоненты вместо дерева или металла, такими компонентами являются пистолетная рукоятка, цевье, складной приклад и, в зависимости от типа, магазин. | wpn_ak104, wpn_ak103_ekp, wpn_ak103_okp, wpn_ak103_rakurs, wpn_ak103_1p78, wpn_ak103_pso |
| enc_weapons_wpn_awm | Accuracy International AWM-F (Arctic Warfare Magnum-Folding). Данная винтовка разработана для стрельбы на дистанцию 1500 метров патроном «магнум» (в данном случае - .338 Lapua Magnum) оборудована складным прикладом и съёмным магазином на пять патронов. Под индексом L115A3 эта снайперская винтовка состоит на вооружении британской армии. И именно британскому снайперу Крэйгу Харрисону принадлежит рекорд поражения противника на дальней дистанции. | wpn_awm |
| enc_weapons_wpn_bizon | Пистолет-пулемёт, разработанный по заказу МВД России в начале 90х годов. В основе конструкции, как и у многих других ижевских образцов, лежат узлы автомата Калашникова, благодаря чему многие детали взаимозаменяемымы с ним. В процессе переделки под пистолетный патрон конструкторы в угоду компактности изменили схему автоматики, и в ПП-19 она работает за счёт энергии отдачи свободного затвора. Оружие оснащено компактным и вместительным шнековым магазином, также имеется крепление типа «ласточкин хвост» для установки прицелов. | wpn_bizon, wpn_bizon_ekp, wpn_bizon_okp, wpn_bizon_rakurs |
| enc_weapons_wpn_hunters | Старая семейная реликвия. | wpn_toz34_m1 |
| enc_weapons_wpn_orex | По слухам, некогда принадлежало соратнику Стрелка - Клыку. | wpn_hunters_toz_new |
| enc_weapons_wpn_soc94 | Самозарядный охотничий карабин, созданный на базе Автомата Калашникова (АКМ), имеет идентичный вид, массу, габаритные размеры. Предназначен для промысловой и любительской охоты на среднего и крупного зверя, а также спортивного использования. | wpn_saiga12c_m2, wpn_vpo101_pso, wpn_vpo101_ekp, wpn_vpo101_okp, wpn_vpo101_rakurs, wpn_vpo101_1p78 |
| enc_weapons_wpn_sv98 | СВ-98М - модернизация винтовки СВ-98, в ходе которой полностью заменен корпус и ствол, за счет чего был уменьшен вес, улучшенна эргономика, увеличена дальность и точность стрельбы, а также установлен новый прицел с большей кратностью. | wpn_sv98 |
| enc_weapons_wpn-beretta93 | Самозарядный пистолет совместного бельгийско-американского производства, давший название одноимённой серии пистолетов. Производится в Колумбии (штат Южная Каролина, США) на заводах FN America (FN USA) - американском отделении бельгийской оружейной компании FN Herstal. Оружие производится с пометкой на затворе «Fredericksburg, VA», что соответствует месту, где была выдана лицензия на производство данного оружия. | wpn_beretta93 |
| enc_weapons_wpn-mp153 | Самозарядный карабин МР-153 российского производства. Был первым полуавтоматическим дробовиком, разработанным на Ижевском Механическом Заводе. Является одним из самых популярных российских полуавтоматических ружей и широко используется охотниками по всей территории России. Также эффективен для охоты на мутантов. | wpn_winchester_m1 |
| enc_weapons_wpn-mp153_m1 | Компактное ружьё с коротким стволом и пистолетной рукояткой без приклада.<br> <br>%c[default]Боеприпасы:<br>%c[255,160,160,160]• 12х70 дробь<br>• 12x76 картечь<br>• 12x76 пуля<br>%c[default]Вид боепитания:<br>%c[255,160,160,160]• трубчатый подствольный магазин на 5 патронов<br>%c[default]Режим огня:<br>%c[255,160,160,160]• одиночный | wpn_mp153_m1 |
| enc_weapons1_wpn_addon_grenade_launcher-ag36 | Однозарядный 40-мм подствольный гранатомёт. Гранатомёт применяется в комплекте с винтовкой G36. | wpn_addon_grenade_launcher_ag36 |
| enc_weapons1_wpn_rpk74 | 5,45-мм ручной пулемёт Калашникова (РПК-74, Индекс ГРАУ - 6П18) - ручной пулемёт, разработанный для замены в войсках пулемёта РПК под патрон 7,62х39 мм в рамках системы стрелкового оружия под малоимпульсный патрон 5,45х39 мм. Принят на вооружение в 1974 году вместе с автоматом АК74. | wpn_rpk74, wpn_rpk74_ekp, wpn_rpk74_okp, wpn_rpk74_rakurs, wpn_rpk74_1p78, wpn_rpk74_pso |
| enc_weapons1_wpn-abakan | Современный автомат, разработанный на замену устаревающему АКС-74. Одна из главных особенностей - возможность ведения огня очередями по 2 выстрела с высокой точностью, так что на расстоянии ста метров обе пули ложатся в одну точку. Это существенно увеличивает шансы поразить цель благодаря совокупному останавливающему и пробивающему действию пуль. | wpn_abakan, wpn_abakan_ekp, wpn_abakan_okp, wpn_abakan_rakurs, wpn_abakan_1p78, wpn_abakan_pso |
| enc_weapons1_wpn-addon-g36l | Прицел коллиматорный G36 Red Dot закрытого типа. Устанавливается на верхнюю направляющую винтовки G36. | wpn_addon_g36l |
| enc_weapons1_wpn-addon-grenade-launcher | Однозарядный 40-мм подствольный гранатомёт. Гранатомёт применяется в комплексе с автоматами Калашникова, а так же с АН-94. | wpn_addon_grenade_launcher |
| enc_weapons1_wpn-addon-grenade-launcher-m203 | Однозарядный 40-мм подствольный гранатомёт. Гранатомёт применяется в комплексе с винтовками стран NATO. | wpn_addon_grenade_launcher_m203 |
| enc_weapons1_wpn-addon-po | Компактный коллиматорный прицел Aimpoint. Устанавливается на планку Пикатинни. | wpn_addon_po |
| enc_weapons1_wpn-aek971 | АК-104 - компактный вариант классического автомата Калашникова в современном исполнении под патрон 7,62х39 мм. АК-104 является укороченным вариантом автомата АК-103. Данный экземпляр имеет магазин на 45 патронов, более удобную рукоять и цевье с планкой пикатинни для крепления прицелов. | wpn_aek971, wpn_ak74u_snag_ac10632, wpn_ak74u_snag_eot, wpn_ak74u_snag_microt1, wpn_ak74u_snag_ekp, wpn_ak74u_snag_okp, wpn_ak74u_snag_rakurs, wpn_ak74u_snag_1p78, wpn_ak74u_snag_pso |
| enc_weapons1_wpn-ak_strelok | Тот самый. | wpn_ak74_m1, wpn_ak74_m1_ac10632, wpn_ak74_m1_eot, wpn_ak74_m1_microt1, wpn_ak74_m1_specter |
| enc_weapons1_wpn-ak_strelok1 | Тот самый. | wpn_ak74_m2, wpn_ak12_ac10632, wpn_ak12_eot, wpn_ak12_microt1, wpn_ak12_specter |
| enc_weapons1_wpn-ak101 | Российский единый ручной пулемет - РПК-16 под калибр 5.45x39, который разрабатывался на замену устаревшему РПК-74. Ключевыми особенностями данного образца вооружения являются - быстросменные стволы, улучшенная эргономика, а так же - цевье и крышка ствольной коробки с направляющими типа «Weaver». | wpn_ak108, wpn_rpk16_ac10632, wpn_rpk16_eot, wpn_rpk16_microt1, wpn_rpk16_specter |
| enc_weapons1_wpn-ak47 | Молот ОП-СКС самозарядный охотничий карабин калибра 7,62х39 мм создан на базе известного самозарядного карабина Симонова (СКС) и имеет идентичный вид, массу, габаритные размеры. Карабин ОП-СКС предназначен для промысловой и любительской охоты на среднего и крупного зверя. | wpn_ak47, wpn_sks_ekp, wpn_sks_okp, wpn_sks_rakurs, wpn_sks_pso, wpn_sks_1p78, wpn_ak47_old |
| enc_weapons1_wpn-ak74 | Автомат образца 1974 года, с появлением которого армии стран Варшавского договора начали переходить на малокалиберный и малоимпульсный патрон. Массивный компенсатор позволяет ещё более снизить и так не слишком значительную отдачу, что положительно сказывается на точности и кучности боя во всех режимах ведения огня. Кроме того, это простое в устройстве и надёжное оружие. | wpn_ak74, wpn_ak74_old |
| enc_weapons1_wpn-ak74m | Автомат образца 1974 года, с появлением которого армии стран Варшавского договора начали переходить на малокалиберный и малоимпульсный патрон. Массивный компенсатор позволяет ещё более снизить и так не слишком значительную отдачу, что положительно сказывается на точности и кучности боя во всех режимах ведения огня. Кроме того, это простое в устройстве и надёжное оружие. АКС-74Н - «ночной» вариант базовой модели с боковой планкой для крепления прицелов. | wpn_ak74m, wpn_ak74m_ekp, wpn_ak74m_okp, wpn_ak74m_rakurs, wpn_ak74m_1p78, wpn_ak74m_pso, wpn_ak74pbn |
| enc_weapons1_wpn-ak74u | По своим габаритам и массе близок к пистолетам-пулеметам, однако устройство механизма почти полностью идентично АКС-74. Малые габариты оружия позволяют успешно использовать его в условиях городской застройки и в стесненных условиях, а пробивающее действие пули довольно велико. Среди недостатков - малая прицельная дальность и склонность к быстрому перегреву. | wpn_ak74u, wpn_ak74u_ekp, wpn_ak74u_okp, wpn_ak74u_rakurs, wpn_ak74u_1p78, wpn_ak74u_old |
| enc_weapons1_wpn-akm | Автомат Калашникова модернизированный (АКМ) является автоматическим оружием с газовым двигателем автоматики, магазинным питанием и воздушным охлаждением ствола. В 1959г АК был модифицирован и принят на вооружение под индексом АКМ - ствольная коробка стала изготавливаться штамповкой, уменьшилась на 700г масса, был введён новый штык-нож, изменена штамповка крышки ствольной коробки, введено замедление срабатывания курка. На ствол стал устанавливаться специальный компенсатор, уменьшающий увод ствола вверх и влево при автоматической стрельбе. | wpn_akm, wpn_akm_ekp, wpn_akm_okp, wpn_akm_rakurs, wpn_akm_1p78, wpn_akm_pso |
| enc_weapons1_wpn-aks47 | Пистолет-пулемёт KRISS Vector - компактная оружейная система для ведения боя в условиях замкнутых пространств, станет идеальным выбором для сотрудников силовых ведомств и вооружённых сил. Низкое расположение канала ствола, а также система компенсирования отдачи Super V позволяют получить наилучший результат как при стрельбе в автоматическом режиме, так и беглым одиночным огнём. | wpn_aks47, wpn_vector_ac10632, wpn_vector_eot, wpn_vector_microt1, wpn_vector_specter |
| enc_weapons1_wpn-aps | Бесшумный вариант знаменитого пистолета Стечкина, доработанный по заказу сил особого назначения. Принят на вооружение в 1972 году, через 14 лет после снятия с производства своего прародителя. Что удивительно, при этом АПБ практически ничем не отличается от него - из значимых изменений выделяются только ствол с резьбой под глушитель. | wpn_aps |
| enc_weapons1_wpn-beretta | Cамозарядный пистолет под патрон 9х19 мм Парабеллум, состоящий на вооружении всех видов вооружённых сил, родов войск и служб войск США. Создан на основе итальянского пистолета Beretta 92F. Изготовляется в США дочерним подразделением фирмы Beretta. От пистолетов серии 92F итальянского производства отличается упрощённым и более коротким по времени производства технологическим циклом, автоматизированным производством ряда деталей, которые в Италии изготавливаются мастерами-оружейниками вручную. | wpn_beretta |
| enc_weapons1_wpn-bm16 | Обрез обычного охотничьего ружья с горизонтальным расположением стволов, значительно более компактный и лёгкий, чем обычная двустволка. Один из самых распространённых видов оружия у бандитов, поскольку удобен для скрытого ношения и очень эффективен в ближнем бою. | wpn_bm16 |
| enc_weapons1_wpn-bm16_full | Двуствольное ружье ТОЗ 66 12-го калибра выпускалось с 1968 по 1974 гг. Имеет усиленную ствольную коробку, хромированные стволы длиной 700-720 мм со стандартными дульными сужениями. Ложа березовая пистолетной формы. Кучность боя для правого ствола 50%, для левого - 60%, доходит до 65%. | wpn_bm16_full |
| enc_weapons1_wpn-colt1911 | Усовершенствованная модель пистолета, который стал классикой автоматического оружия и с честью прошёл все вооружённые конфликты XX века. Как в исходном, так и в модифицированном варианте он до сих пор выпускается десятками, если не сотнями производителей. Единственным недостатком можно считать невысокую ёмкость магазина, но в определённой степени это компенсируется мощным патроном. | wpn_colt1911 |
| enc_weapons1_wpn-desert-eagle | Настоящая «карманная пушка» - большая, тяжёлая и обладающая чрезвычайно высокой убойной силой. Разрабатывался как специализированное оружие для охоты на среднюю и крупную дичь, а также для спортивной стрельбы по силуэтным мишеням на дальностях от пятидесяти метров. Из-за цены, массы и внушительных габаритов особого распространения в Зоне не получил. | wpn_desert_eagle, wpn_desert_eagle_po |
| enc_weapons1_wpn-eagle_m1 | Raging Bull - револьвер, созданный бразильской компанией по производству огнестрельного оружия Taurus International. Это оружие позиционируется как охотничье, благодаря своему калибру. Достаточно плотно лежит в руке, имеет красную резиновую прокладку, снижающую отдачу, что позволяет стрелку чувствовать себя комфортнее. | wpn_eagle_m1 |
| enc_weapons1_wpn-fn2000 | FN F2000 Tactical - аналогичен стандартной модели, однако лишён оптического прицела, вместо которого установлена дополнительная направляющая типа планка Пикатинни, открытые прицельные приспособления, магазин на 40 патронов и окрашен в FDE расцветку. | wpn_fn2000, wpn_fn2000_nimble_ac10632, wpn_fn2000_nimble_eot, wpn_fn2000_nimble_microt1, wpn_fn2000_nimble_specter |
| enc_weapons1_wpn-fort | Пистолет украинского производства, предназначенный в основном для вооружения правоохранительных органов и силовых структур. Обладает лучшими по сравнению с ПМ точностью боя, приёмистостью и ёмкостью магазина. В основном попадает в Зону среди широко реализуемой здесь отбраковки (в основном это ранние образцы со всеми «детскими болезнями»). Тем не менее, пользуется спросом у сталкеров из-за достаточно высоких боевых характеристик. | wpn_fort |
| enc_weapons1_wpn-fort_m1 | ОЦ-33 «Пернач» - российский автоматический пистолет, разработанный в середине 1990-х годов коллективом конструкторов ЦКИБ СОО под руководством И. Я. Стечкина для замены пистолета АПС. | wpn_fort_m1 |
| enc_weapons1_wpn-g36 | Воплощение новых тенденций в автоматическом оружии западного производства: принцип работы автоматики основан не на полусвободном затворе, а на коротком ходе газового поршня. Нужно сказать, изменения явно пошли на пользу: данная штурмовая винтовка представляет собой первоклассный образец современного оружия - лёгкого, надёжного и эргономичного. | wpn_g36 |
| enc_weapons1_wpn-g36c | HK MG36 - ручной пулемёт на базе G36. Имеет утяжелённый ствол возле патронника и сошки. Произведено несколько сотен в 1995 году и в том же году был отменен серийный выпуск, по причине неудовлетворенности солдат Бундесвера продолжительностью непрерывного огня. | wpn_g36c, wpn_g36_nimble_ac10632, wpn_g36_nimble_eot, wpn_g36_nimble_microt1, wpn_g36_nimble_specter |
| enc_weapons1_wpn-gauss | Сверхточное высокотехнологичное оружие, известное как «изделие №62», было разработано конструкторами секретных лабораторий внутри Зоны. Принцип его действия основан на разгоне пули системой электромагнитов; пуля при этом способна развивать сверхскорости, что обеспечивает колоссальную пробивную способность при почти полном отсутствии отдачи. Для обеспечения достаточной энергоёмкости аккумуляторов в последних используются частицы определённых артефактов. | wpn_gauss, wpn_gauss_auto |
| enc_weapons1_wpn-glock | Glock 17 - австрийский пистолет, разработанный фирмой Glock для нужд австрийской армии. Благодаря своим боевым качествам и надёжности получил широкое распространение в качестве полицейского, спортивного, а так же гражданского оружия самообороны. | wpn_glock |
| enc_weapons1_wpn-glock_m1 | Glock 18C модификация пистолета Glock 17 с возможностью ведения автоматического огня. Данный экземпляр укомлектован магазином на 33 патрона. | wpn_glock_m1 |
| enc_weapons1_wpn-glock_m2 | MP9 - пистолет-пулемёт, созданный компанией Brugger + Thomet AG на базе TMP. Основные отличия: наличие складного вправо приклада и планки Пикатинни для установки дополнительных прицельных приспособлений Находится на вооружении подразделений спецназа, а также широко применяется спецслужбами и компаниями, которые отвечают за сохранность первых лиц государства, высокопоставленных фигур. | wpn_glock_m2, wpn_mp9_ac10632, wpn_mp9_eot, wpn_mp9_microt1 |
| enc_weapons1_wpn-gm94 | Подствольный гранатомёт M320 был создан для замены подствольного гранатомёта M203. M320 был произведен на основе Heckler Koch AG36, но они не полностью идентичны. Производство было начато в ноябре 2008 года. Его можно использовать двумя способами. Он может быть смонтирован под стволом на винтовки M16 и M4, или может использоваться сам по себе. | wpn_gm94 |
| enc_weapons1_wpn-groza | Автоматно-гранатометный комплекс, на 70% унифицированный с автоматами типа АКМ-74 (основные отличия - калибр 9х39 мм, компоновочная схема «булл-пап» и более широкие возможности для модернизации). Разработан для вооружения подразделений быстрого реагирования. Очень ценим военными и сталкерами за способность надёжно поражать защищённого бронежилетом или расположенного в укрытии противника. | wpn_groza |
| enc_weapons1_wpn-kabar | KM2000 (от нем. Kampfmesser - «боевой нож») - немецкий боевой нож. Нож производится немецкой компанией Eickhorn-Solingen Ltd. Принят на вооружение германским Бундесвером. Клинок выполнен в стиле «american tanto». | wpn_knife_m1 |
| enc_weapons1_wpn-knife | Общевойсковой штык-нож 6Х4. | wpn_knife |
| enc_weapons1_wpn-m110 | FN SCAR-H штурмовая винтовка, разработанная компанией FN Herstal для командования специальных операций США (SOCOM). В данной модели используется калибр 7.62х51 мм НАТО (отъёмные коробчатые магазины на 20 патронов). FN SCAR обладает большим уроном среди штурмовых винтовок. Эффективна на любой дистанции. Против больших групп врагов не хватает вместимости маленького магазина. | wpn_m110, wpn_scarh_ac10632, wpn_scarh_eot, wpn_scarh_microt1, wpn_scarh_specter |
| enc_weapons1_wpn-mp5 | Данный пистолет-пулемёт заслуженно пользуется огромной популярностью благодаря надёжности, удобству в обращении и высокой точности стрельбы. Принятый на вооружение армейских и полицейских подразделений многих стран, он появился и на чёрном рынке и со временем добрался до Зоны. | wpn_mp5, wpn_mp5_ac10632, wpn_mp5_eot, wpn_mp5_microt1, wpn_mp5_specter |
| enc_weapons1_wpn-mp5_m1 | Модификация стандартного пистолет-пулемета под более распространённый патрон 9x18. | wpn_mp5_m1, wpn_mp5_m1_ac10632, wpn_mp5_m1_eot, wpn_mp5_m1_microt1, wpn_mp5_m1_specter |
| enc_weapons1_wpn-mp5_m2 | СР-2 «Вереск» («специальная разработка 2») - российский пистолет-пулемёт, предназначенный для поражения в ближнем бою (на дистанции до 200 м) живой силы противника, использующей индивидуальные средства бронезащиты. А этот вариант, это усовершенствованный СР-2М «Вереск» с планками Пикатинни на крышке ствольной коробки, цевье. Также теперь имеется возможность установки глушителя и приклада изменённой формы. | wpn_mp5_m2, wpn_sr2m_ac10632, wpn_sr2m_eot, wpn_sr2m_microt1 |
| enc_weapons1_wpn-mr308 | Снайперская винтовка Heckler - Koch HK G28 разработана и производится германской компанией Хеклер-Кох по заказу Бундесвера (Армии ФРГ). Сама винтовка G28 создана на базе спортивно-охотничьей самозарядной винтовки HK MR308, которая, в свою очередь, является гражданским вариантом автоматической винтовки HK 417. | wpn_mr308, wpn_mr308_g28_scope |
| enc_weapons1_wpn-pb_lt | Пистолет был разработан во второй половине ХХ века на базе пистолета ПМ для вооружения разведывательных групп и персонала спецслужб. Данная модель считается морально устаревшей, но в умелых руках это по-прежнему весьма эффективное оружие. | wpn_pb |
| enc_weapons1_wpn-pkm | Ручная модификация армейского пулемёта с ленточным питанием. Несмотря на высокую массу и неудовлетворительную точность боя, нашёл себе достаточно широкое применение в Зоне. В основном используется для подавления превосходящего по численности противника и при зачистках логов мутантов. | wpn_pkm |
| enc_weapons1_wpn-pkp | Модернизация ПКМа по заказам Сталкеров-мастеров для особо опасных рейдов вглубь зоны. В ходе модернизации были заменены приклад, рукоятка, пламегаситель, а также добавлены тканевый кожух на магазин и планка пикатинни с возможностью крепления прицелов. | wpn_pkp, wpn_pkm_zulus_ac10632, wpn_pkm_zulus_eot, wpn_pkm_zulus_microt1 |
| enc_weapons1_wpn-pm | Наиболее распространённый в зоне пистолет, наследие советской эпохи, ПМ - один из лучших представителей класса компактного оружия самообороны. Он отличается небольшими размерами, исключительно надёжен, прост в использовании и обслуживании. Среди недостатков - малая дальность и точность стрельбы, невысокая эффективность пули и - в сравнении с современными пистолетами того же класса - довольно большая масса при недостаточной ёмкости магазина. | wpn_pm |
| enc_weapons1_wpn-pp19 | 9 мм пистолет-пулемет ПП 19-01 «Витязь» разработан на базе конструкции автомата Калашникова АК-105 под пистолетный патрон 9х19 мм по техническому заданию специальных подразделений МВД России. Благодаря удачной конструкции пистолет-пулемёт «Витязь» подходит для вооружения специалистов любого профиля от специальных подразделений до военнослужащих отдельных специальностей для решения задач по обеспечению их личной безопасности. | wpn_pp19, wpn_vityaz_ac10632, wpn_vityaz_eot, wpn_vityaz_microt1, wpn_vityaz_specter |
| enc_weapons1_wpn-rg-6 | Гранатомёт револьверного типа, в котором каморы барабана образованы шестью нарезными стволами. Предназначен для борьбы с противником, находящимся в укрытиях, и ведению огня по площадям. Полезен также при нападении на хорошо охраняемые объекты или в бою с особо живучими тварями. | wpn_rg6 |
| enc_weapons1_wpn-rpg7 | Гранатомёт образца 1960-х, в огромном количестве оставшийся на армейских складах, откуда и попал в Зону. Представляет собой гладкоствольное однозарядное пусковое устройство безоткатного типа, укомплектованное оптическим прицелом ПГО-7 кратностью 2,7 диоптрии. Внушительные габариты и большой вес делают это оружие не слишком распространённым среди сталкеров, но ввиду огромной разрушительной силы и простоты использования оно нашло себе верных почитателей в Зоне. | wpn_rpg7 |
| enc_weapons1_wpn-rpk | 7,62-мм ручной пулемёт Калашникова (РПК, Индекс ГРАУ - 6П2) - советский ручной пулемёт, созданный на основе автомата АКМ. Принят на вооружение советской армии в 1961 году. Он заменил ручной пулемёт Дегтярёва как более предпочтительный с точки зрения унификации с другим принятым на вооружение стрелковым оружием. | wpn_rpk, wpn_rpk74m_ekp, wpn_rpk74m_okp, wpn_rpk74m_rakurs, wpn_rpk74m_1p78, wpn_rpk74m_pso |
| enc_weapons1_wpn-sig220 | Единственная оставшаяся в серийном производстве модель популярного во всём мире пистолета. Данное оружие характерно прекрасной эргономикой и отличной точностью боя. Малая ёмкость магазина компенсируется мощным патроном и надёжностью швейцарских часов. Неудивительно, что пистолет пользуется неизменным спросом у бывалых сталкеров и наёмников. | wpn_sig220 |
| enc_weapons1_wpn-sig226 | Отличный пистолет западного производства, отличающийся высокой надёжностью, ёмким магазином, хорошей точностью боя и, пожалуй, лучшей в своем классе эргономикой. Попав в Зону, стал одним из самых распространенных пистолетов как среди профессионалов, так и среди новичков при деньгах. | wpn_walther |
| enc_weapons1_wpn-sig550 | SIG SG 552 - компактный автомат, предназначенный для специальных полицейских подразделений. В отличие от SG 550 возвратная пружина позади затворной рамы и действует непосредственно на неё, а не на шток газового поршня. Автомат имеет изменённый пламегаситель и может оснащаться планками Пикатинни для установки дополнительных аксессуаров. | wpn_sig550, wpn_sig550_ac10632, wpn_sig550_eot, wpn_sig550_microt1, wpn_sig550_specter |
| enc_weapons1_wpn-soc308 | SA-58 OSW (Operations Specialist Weapon) производимая американской компанией DSA (David Selveggio Arms) на той же производственной линии, что и StG-58, и является официальной копии винтовок FAL. Гражданская версия производится только в полу-автоматическом варианте, так же для полиции или армейских подразделений производится модификация с возможностью ведения автоматического огня. DSA-58 полностью совместима с любыми метрическими магазинами для FN FAL. | wpn_soc308 |
| enc_weapons1_wpn-soc308_m1 | Модифицированный вариант DS Arms SA-58 с планкой Пикатинни и увеличенным магазином. | wpn_soc308_m1, wpn_soc308_m1_ac10632, wpn_soc308_m1_eot, wpn_soc308_m1_microt1, wpn_soc308_m1_specter |
| enc_weapons1_wpn-spas12 | Гладкоствольный автоматический дробовик специального назначения с возможностью переключения режимов ведения огня. Разработанный ещё во второй половине ХХ столетия, продолжает применяться в качестве универсального боевого оружия полиции и штурмовых подразделений армии. Отличается высокой надёжностью и тактической гибкостью. Несмотря на большую массу, сложное устройство и высокую цену, довольно популярен в Зоне как эффективное средство борьбы против мутантов. | wpn_spas12 |
| enc_weapons1_wpn-spas12_m1 | Benelli M4 Super 90 - гладкоствольное полуавтоматическое магазинное самозарядное ружьё, разработанное в Италии фирмой Benelli Armi S.p.A.. Cверху на ствольной коробке смонтирована универсальная планка типа «Пикатинни», позволяющая крепить коллиматорные или ночные прицелы. Оружие снабжено пистолетной рукояткой и отъемным телескопическим прикладом с резиновым затыльником и «щекой», пластмассовым цевьем. Дробовики Benelli M4 состоят на вооружении в Австралии, США, Словении и Великобритании (как L128A1), принимали участие в войне в Ираке и в Афганистане. | wpn_spas12_m1, wpn_spas12_nimble_ac10632, wpn_spas12_nimble_eot, wpn_spas12_nimble_microt1 |
| enc_weapons1_wpn-spas15 | USAS-12 - гражданская версия ружья, разработанного Джоном Тревором в конце 1980-х годов на основе идей другого американского конструктора Максвелла Атчиссона, создавшего Atchisson Assault Shotgun (AA-12). | wpn_spas15 |
| enc_weapons1_wpn-spas15_m1 | Модификация стандартного USAS-12. Увеличена кучность стрельбы, скорострельность и убойность. | wpn_spas15_m1, wpn_usas12_m1_ac10632, wpn_usas12_m1_eot, wpn_usas12_m1_microt1, wpn_usas12_m1_specter |
| enc_weapons1_wpn-svd | Данный тип снайперской винтовки находится на рынке вооружений вот уже добрых полвека и за это время успел зарекомендовать себя как исключительно надёжное и удобное в обращении оружие. В войсках она служит в основном для поражения удалённых целей, которые невозможно достать из автомата, но сталкеры оценили СВД во многом за то, что пуля из неё позволяет пробить даже самую толстую шкуру мутанта или хороший бронежилет. | wpn_svd, wpn_svd_ekp, wpn_svd_okp, wpn_svd_rakurs, wpn_svd_1p78, wpn_svd_pso |
| enc_weapons1_wpn-svu | Укороченная снайперская винтовка с увеличенным темпом стрельбы, анатомической рукояткой, дальнейшее развитие идеи СВУ. | wpn_svu, wpn_svu_ekp, wpn_svu_okp, wpn_svu_rakurs, wpn_svu_1p78, wpn_svu_pso |
| enc_weapons1_wpn-toz34 | Широко распространённое охотничье ружьё-«вертикалка» благодаря достаточной точности и хорошему останавливающему действию обеспечивает более надёжную защиту от мутантов, чем пистолет. Очень дёшево и доступно в сравнении с большинством видов оружия, поэтому активно применяется новичками и бандитами на окраинах Зоны. | wpn_toz34 |
| enc_weapons1_wpn-ump45 | Heckler Koch UMP - пистолет-пулемет 45-го калибра, разработанный как дополнение к семейству 9-миллиметровых пистолетов-пулеметов HK MP-5. Низкая скорострельность объясняется тем, что первоначально пистолет-пулемёт разрабатывался под крупнокалиберные патроны .45 ACP. Имеется магазин только на 25 патронов. | wpn_ump45, wpn_mp5_nimble_ac10632, wpn_mp5_nimble_eot, wpn_mp5_nimble_microt1, wpn_mp5_nimble_specter |
| enc_weapons1_wpn-usp | Главные отличительные черты этого оружия - высочайшие надёжность и живучесть при отличной точности боя. Несмотря на некоторые недостатки (например, значительные размеры, высоко расположенный центр тяжести и массивный затвор, затрудняющий скрытое ношение) пользуется значительной популярностью среди ветеранов Зоны. | wpn_usp |
| enc_weapons1_wpn-val | Это оружие было создано на основе специальной снайперской винтовки «Винторез» в качестве бесшумного оружия для войск специального назначения. Оно позволяет стрелку вести эффективный огонь по защищённому бронежилетом противнику, при этом не демаскируя своей позиции. Одна из главных особенностей - использование интегрированного глушителя и патронов калибра 9х39 мм с тяжёлой пулей. | wpn_val, wpn_val_ekp, wpn_val_okp, wpn_val_rakurs, wpn_val_1p78, wpn_val_pso |
| enc_weapons1_wpn-vintorez | «Винтовка снайперская специальная» предназначена для бесшумной и беспламенной снайперской стрельбы при проведении спецопераций, когда необходимо сохранить в тайне позицию стрелка. Её боеприпас обеспечивает высокие бронебойное и останавливающее действия; кроме того, данное оружие снабжено интегрированным глушителем. Несмотря на магазин небольшой ёмкости, очень ценится сталкерами. | wpn_vintorez, wpn_vintorez_ekp, wpn_vintorez_okp, wpn_vintorez_rakurs, wpn_vintorez_1p78, wpn_vintorez_pso |
| enc_weapons1_wpn-vintorez_m1 | «Винторез-М» - это усовершенствованный вариант снайперской винтовки «Винторез», принятой в российской армии еще в 1987 году. Особенностью обновленного оружия являются улучшенные характеристики по точности и кучности. А для самого стрелка «Винторез» стал еще более удобен благодаря улучшенному прикладу: отдача при стрельбе почти не чувствуется. Буква «М» обозначает - модернизированная. | wpn_vintorez_m1, wpn_vintorez_nimble_ac10632, wpn_vintorez_nimble_eot, wpn_vintorez_nimble_microt1, wpn_vintorez_nimble_specter |
| enc_weapons1_wpn-wincheaster1300_lt | MP-133 - российское гладкоствольное ружьё, которое применяется для различных видов охоты, спортивной стрельбы, охраны и самообороны. Ружьё разработано на базе ружья ИЖ-81 и серийно выпускавшееся Ижевским механическим заводом с 2000 года. У данного экземпляра укорочен ствол и отстутствует приклад. | wpn_wincheaster1300 |
| enc_weapons1_wpn-winchester_m1 | Remington 870 - американское ружьё, впервые представленное в 1950 году компанией Remington Arms. Remington 870 благодаря своей характерной перезарядке является одним из самых знаменитых помповых ружей в мире. Remington 870 выпускался в различных модификациях, он подходит как для военных и полиции, так и для охотников и спортсменов, а также для самообороны. FD версия оснащена пистолетной рукояткой, прикладом и цевьем от фирмы FAB Defense, а также имеет магазин на 11 патронов и чок от Remington. | wpn_mp153, wpn_wincheaster1300_trapper_ac10632, wpn_wincheaster1300_trapper_eot, wpn_wincheaster1300_trapper_microt1 |
| enc_wpn_mosin | Винтовка Мосина, которую также называют просто «Мосинкой» или трехлинейкой, является самой известной винтовкой на территории постсоветского пространства. Принятая на вооружение в далеком 1891 году, она продолжала массово использоваться до конца 50-х годов ХХ века. Калибр винтовки Мосина равен 7.62 мм. Название трехлинейка происходит от калибра винтовки, который равен трем линиям, старинной мерой длины равной одной десятой дюйма или 2,54 мм. | wpn_mosin, wpn_mosin_pu |
| enc_wpn-gungauss | %c[default]Боеприпасы:<br>%c[255,160,160,160]• пистолетный электромагнитный картридж | wpn_gungauss |
| enc_wpn-saiga12c | Сайга-12К - самозарядное ружьё, разработанное на Ижевском машиностроительном заводе на базе автомата Калашникова и предназначенное для промысловой и любительской охоты на мелкого, среднего зверя и птицу в районах с любыми климатическими условиями. | wpn_saiga12c, wpn_saiga12_ekp, wpn_saiga12_okp, wpn_saiga12_rakurs |
| enc_wpn-saiga12c_m1 | Образец модернизированой Сайга-12К по заказу одного из специальных военных отрядов на территории зоны, кто они и чем занимаются остается загадкой. В ходе модернизации были установлены: магазин на 10 патронов, анатомическая рукоять, планка пикатинни, улучшенный затвор и дульный тормоз-компенсатор ГК-02. | wpn_saiga12c_m1, wpn_saiga12m_ac10632, wpn_saiga12m_eot, wpn_saiga12m_microt1, wpn_saiga12m_specter |
| st_addon_scope_1p78_descr | Прицел 1П78 предназначен для оснащения автоматов и ручных пулемётов и ведения прицельной стрельбы на средние и дальние дистанции. Прицел имеет кратность 2,7X. Разработан ЦКБ «Точприбор» (г. Новосибирск), главный конструктор - С.И. Михаленко. | wpn_addon_1p78 |
| st_addon_scope_ac10632_descr | Коллиматорный прицел ELECTRO SIGHT 1x22x33 для быстрого прицеливания. Ударопрочный, подходит под все виды оружия с планкой Пикатинни. Выбор уровня яркости подсветки прицельной марки (7 ступеней) позволяет использовать прицел при различных уровнях освещённости. | ac10632 |
| st_addon_scope_eotech_descr | EOTech - это мировой лидер и эксклюзивный производитель голографических прицельных систем для стрелкового оружия. | wpn_addon_eotech |
| st_addon_scope_g28_descr | Оптический прицел немецкого производства переменной кратности, до 8,8X. В комплекте идёт коллиматорный прицел Aimpoint Micro T-1, который крепится на планку, расположенную схверху. | g28_scope |
| st_addon_scope_micro-t1_descr | Aimpoint Micro T-1 - тактический коллиматорный прицел, самый компактный из всей линейки Aimpoint, для тех, кому необходим небольшой вес и размеры при таких же великолепных характеристиках и качестве, как и у полноразмерных прицелов Aimpoint. | microt1 |
| st_addon_scope_okp_descr | Оригинальная конструкция прицелов ОКП даёт возможность вести прицельный огонь из неустойчивых положений и в условиях быстро меняющейся обстановки, снижает утомляемость стрелка, а также, при необходимости, позволяет использовать штатное механическое приспособление. | okp |
| st_addon_scope_pso1_descr | Широко распространённый оптический прицел советского производства фиксированной кратности 4,5X. Оснащён шкалой для определения дальности до цели. Для установки на оружие используется стандартное в странах - участницах Варшавского договора крепление типа «ласточкин хвост». | wpn_addon_1pso |
| st_addon_scope_pu_descr | ПУ (от Прицел Укороченный) - советский оптический прицел, изначально созданный в 1940 году, и предназначенный для установки на винтовку Мосина. Видимое увеличение - 3.6 крат. | wpn_addon_pe |
| st_addon_scope_rakurs_descr | Прицел коллиматорный «Ракурс-А» - широкоугольный коллиматорный прицел однократного увеличения предназначен для ведения прицельной стрельбы на дальность прямого выстрела. | rakurs |
| st_addon_scope_specter_descr | Оптический прицел с изменяемой кратностью, призванный прийти на замену морально устаревшему прицелу ACOG от компании Trijicon. Даёт увеличение до 4,5 крат. | specter |
| st_addon_sil_groza_descr | Глушитель из комплекта поставки автоматно-гранатомётного комплекса ОЦ-14 «Гроза». | wpn_addon_sil_groza |
| st_addon-ekp-8-02_descr | ЭКП-8-02 Кобра является открытым коллиматорным прицелом типа «красная точка». Предназначен для поражения быстро движущихся целей и решения скоротечных задач на близком расстоянии в городских условиях при различном освещении. Обеспечивает чёткое прицеливание с одного или двух глаз, очень эффективен при стрельбе из движущихся транспортных средств, а также идеален для поражения быстро движущихся, малых и спонтанно появляющихся целей. | wpn_addon_cobra |
| st_grenade_launcher_groza_descr | Гранатомётный модуль, предназначенный для использования совместно с автоматно-гранатомётным комплексом ОЦ-14 «Гроза». Создан на базе подствольного гранатомёта ГП-25, в конструкцию которого внесён ряд изменений и доработок. Наиболее существенной из них является единый спусковой крючок, используемый как при стрельбе гранатами, так и для ведения огня обычными боеприпасами. Из-за изменённой схемы крепления гранатомёта к стволу данный модуль нельзя использовать с другими видами оружия. | wpn_addon_grenade_launcher_groza |
| st_wpn_coltm45a1_descr | Усовершенствованный M45A1 содержит несколько изменений оригинального дизайна M1911A1. Одной из особенностей является система двойной возвратной пружины, которая распределяет силу отдачи патрона .45 ACP за счет снижения пиковой силы импульса отдачи. У него также есть тритиевые ночные прицелы с 3 точками, 5-дюймовый ствол, двусторонний предохранитель, планка Пикатинни и отделка Cerakote цвета пустынно-коричневого цвета. | wpn_ak74_m3 |
| st_wpn_groza_nimble_descr | Модификация стандартного «ОЦ-14-4А», созданная специально для действующих в Зоне спецподразделений. Отличается наличием планок пикатинни, тритиевых вставок и анатомической рукоятки. | wpn_groza_nimble, wpn_groza_nimble_ac10632, wpn_groza_nimble_eot, wpn_groza_nimble_microt1, wpn_groza_nimble_specter |
| st_wpn_val_nimble_descr | СР-3 «Вихрь» - компактный российский автомат, разработанный на основе бесшумного автомата АС «Вал». СР-3М отличается возможностью использования глушителя, металлических магазинов на 30 патронов, установки оптических и ночных прицелов, складным влево рамочным прикладом (позаимствованным у АС «Вал»), флажковым предохранителем, выполненным по типу СВД, и новым цевьём со складывающейся дополнительной рукояткой. | wpn_val_nimble, wpn_val_nimble_ekp, wpn_val_nimble_okp, wpn_val_nimble_rakurs, wpn_val_nimble_1p78, wpn_val_nimble_pso, wpn_val_nimble_ac10632, wpn_val_nimble_eot, wpn_val_nimble_microt1 |
| wpn-flame_readme | Самодельный огнемёт конструкции свободовца Шурупа. Заправляется двухкомпонентной смесью из коллоидного газа "Ф" и нефтепродуктов. Смесь самовозгорается на воздухе, поэтому конструкция предельно проста. Однако из-за высокой химической активности смеси в конструкции применены детали, изготовленные из высокотехнологичных антикоррозийных материалов, получаемых в особых лабораторных условиях. К тому же секретом производства смеси владеет только сам Шуруп. По этим причинам огнемёт не стал массовым изделием, несмотря на весьма серьёзные преимущества перед обычным огнестрельным оружием при встрече с некоторыми обитателями Зоны. | wpn_flame |


### Additional name-like references where `inv_name_short` differs

| section_id | field | string_id | actual `<text>` | mismatch flag | source file |
| :-- | :-- | :-- | :-- | :-- | :-- |
| wpn_g36c | inv_name_short | g36c | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_mg36.ltx:2 |
| wpn_g36_nimble_ac10632 | inv_name_short | g36c | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_mg36.ltx:162 |
| wpn_g36_nimble_eot | inv_name_short | g36c | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_mg36.ltx:179 |
| wpn_g36_nimble_microt1 | inv_name_short | g36c | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_mg36.ltx:197 |
| wpn_g36_nimble_specter | inv_name_short | g36c | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_mg36.ltx:215 |
| wpn_ump45 | inv_name_short | ump45 | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_ump45.ltx:2 |
| wpn_mp5_nimble_ac10632 | inv_name_short | ump45 | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_ump45.ltx:226 |
| wpn_mp5_nimble_eot | inv_name_short | ump45 | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_ump45.ltx:245 |
| wpn_mp5_nimble_microt1 | inv_name_short | ump45 | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_ump45.ltx:264 |
| wpn_mp5_nimble_specter | inv_name_short | ump45 | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/w_ump45.ltx:284 |
| wpn_fake_missile2 | inv_name_short | "M209" | UNRESOLVED — not present in string_table_enc_weapons.xml | yes — unresolved | gamedata/config/weapons/weapons.ltx:1705 |


## Step 3 — Merged map

| base_id | variant_id | addon_type | in-game name | mismatch note | relationship note | source file |
| :-- | :-- | :-- | :-- | :-- | :-- | :-- |
| — | ac10632 | scope | Прицел Barska ELECTRO SIGHT AC10632 | — | — | gamedata/config/weapons/add_scopes.ltx:167 |
| — | g28_scope | scope | Прицел HK Schmidt Bender | — | — | gamedata/config/weapons/add_scopes.ltx:274 |
| — | microt1 | scope | Прицел Aimpoint Micro T-1 | — | — | gamedata/config/weapons/add_scopes.ltx:189 |
| — | okp | scope | Прицел ОКП-7 | — | — | gamedata/config/weapons/add_scopes.ltx:232 |
| — | rakurs | scope | Прицел «Ракурс-А» | — | — | gamedata/config/weapons/add_scopes.ltx:253 |
| — | specter | scope | Прицел ELCAN SpecterDR | — | — | gamedata/config/weapons/add_scopes.ltx:210 |
| — | wpn_addon_1p78 | scope | Прицел 1П78 «Каштан» | — | — | gamedata/config/weapons/add_scopes.ltx:24 |
| — | wpn_addon_1pso | scope | Прицел ПСО-1М2 | — | — | gamedata/config/weapons/add_scopes.ltx:2 |
| — | wpn_addon_cobra | scope | Прицел «Кобра» ЭКП-8-02 | — | — | gamedata/config/weapons/add_scopes.ltx:47 |
| — | wpn_addon_eotech | scope | Прицел EOTech | — | — | gamedata/config/weapons/add_scopes.ltx:97 |
| — | wpn_addon_g36l | scope | Прицел G36 Red Dot | — | — | gamedata/config/weapons/add_scopes.ltx:69 |
| — | wpn_addon_grenade_launcher | underbarrel | Подствольный гранатомет ГП-25 | — | — | gamedata/config/weapons/add_gl.ltx:2 |
| — | wpn_addon_grenade_launcher_ag36 | underbarrel | Подствольный гранатомет AG36 | — | — | gamedata/config/weapons/add_gl.ltx:48 |
| — | wpn_addon_grenade_launcher_groza | underbarrel | Подствольный гранатомет ГП-5 | — | — | gamedata/config/weapons/add_gl.ltx:68 |
| — | wpn_addon_grenade_launcher_m203 | underbarrel | Подствольный гранатомет M203 | — | — | gamedata/config/weapons/add_gl.ltx:26 |
| — | wpn_addon_pe | scope | Прицел ПУ | — | — | gamedata/config/weapons/add_scopes.ltx:119 |
| — | wpn_addon_po | scope | Прицел Aimpoint Micro | — | — | gamedata/config/weapons/add_scopes.ltx:143 |
| — | wpn_addon_sil_45_p | silencer | Глушитель 11,43 мм | — | — | gamedata/config/weapons/add_silencers.ltx:191 |
| — | wpn_addon_sil_545_ar | silencer | Глушитель 5,45х39 | — | — | gamedata/config/weapons/add_silencers.ltx:30 |
| — | wpn_addon_sil_556_ar | silencer | Глушитель 5,56х45 | — | — | gamedata/config/weapons/add_silencers.ltx:57 |
| — | wpn_addon_sil_762_ar | silencer | Глушитель 7,62х39 | — | — | gamedata/config/weapons/add_silencers.ltx:84 |
| — | wpn_addon_sil_762_sr1 | silencer | Глушитель 7,62х54 | — | — | gamedata/config/weapons/add_silencers.ltx:138 |
| — | wpn_addon_sil_762_sr2 | silencer | Глушитель 7,62х51 | — | — | gamedata/config/weapons/add_silencers.ltx:111 |
| — | wpn_addon_sil_9_p | silencer | Глушитель 9 мм | — | — | gamedata/config/weapons/add_silencers.ltx:2 |
| — | wpn_addon_sil_apb | silencer | Глушитель АПБ | — | — | gamedata/config/weapons/add_silencers.ltx:242 |
| — | wpn_addon_sil_co_salvo | silencer | Глушитель Salvo 12 | — | — | gamedata/config/weapons/add_silencers.ltx:297 |
| — | wpn_addon_sil_glock | silencer | Глушитель Glock 18C | — | — | gamedata/config/weapons/add_silencers.ltx:216 |
| — | wpn_addon_sil_groza | silencer | Глушитель 9х39 мм «Гроза» | — | — | gamedata/config/weapons/add_silencers.ltx:322 |
| — | wpn_addon_sil_mosin | silencer | Глушитель 7,62х54 R | — | — | gamedata/config/weapons/add_silencers.ltx:165 |
| — | wpn_addon_sil_vsk | silencer | Глушитель 9х39 мм «Вихрь» | — | — | gamedata/config/weapons/add_silencers.ltx:269 |
| — | wpn_fake_missile | internal projectile | Заряд ВОГ-25 | — | — | gamedata/config/weapons/weapons.ltx:1488 |
| — | wpn_fake_missile1 | internal projectile | Заряд ВГ-40И | — | — | gamedata/config/weapons/weapons.ltx:1589 |
| — | wpn_fake_missile2 | internal projectile | Заряд M406 | — | — | gamedata/config/weapons/weapons.ltx:1705 |
| — | wpn_rpg7_missile | internal projectile | UNRESOLVED | string ID `"Og-7b"` is not present in string_table_enc_weapons.xml | — | gamedata/config/weapons/weapons.ltx:1360 |
| wpn_abakan | — | base weapon | Автомат АН-94 | — | — | gamedata/config/weapons/w_abakan.ltx:2 |
| wpn_abakan | wpn_abakan_1p78 | scope | Автомат АН-94 | — | — | gamedata/config/weapons/w_abakan.ltx:360 |
| wpn_abakan | wpn_abakan_ekp | scope | Автомат АН-94 | — | — | gamedata/config/weapons/w_abakan.ltx:299 |
| wpn_abakan | wpn_abakan_okp | scope | Автомат АН-94 | — | — | gamedata/config/weapons/w_abakan.ltx:320 |
| wpn_abakan | wpn_abakan_pso | scope | Автомат АН-94 | — | — | gamedata/config/weapons/w_abakan.ltx:386 |
| wpn_abakan | wpn_abakan_rakurs | scope | Автомат АН-94 | — | — | gamedata/config/weapons/w_abakan.ltx:341 |
| wpn_aek971 | — | base weapon | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | — | gamedata/config/weapons/w_ak104.ltx:2 |
| wpn_aek971 | wpn_ak74u_snag_1p78 | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:391 |
| wpn_aek971 | wpn_ak74u_snag_ac10632 | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:249 |
| wpn_aek971 | wpn_ak74u_snag_ekp | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:321 |
| wpn_aek971 | wpn_ak74u_snag_eot | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:273 |
| wpn_aek971 | wpn_ak74u_snag_microt1 | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:297 |
| wpn_aek971 | wpn_ak74u_snag_okp | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:346 |
| wpn_aek971 | wpn_ak74u_snag_pso | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:425 |
| wpn_aek971 | wpn_ak74u_snag_rakurs | scope | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» | ambiguous relationship: ID stem `wpn_ak74u`, inheritance-based base `wpn_aek971` | gamedata/config/weapons/w_ak104.ltx:366 |
| wpn_ak104 | — | base weapon | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_ak103.ltx:2 |
| wpn_ak104 | wpn_ak103_1p78 | scope | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` | gamedata/config/weapons/w_ak103.ltx:337 |
| wpn_ak104 | wpn_ak103_ekp | scope | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` | gamedata/config/weapons/w_ak103.ltx:269 |
| wpn_ak104 | wpn_ak103_okp | scope | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` | gamedata/config/weapons/w_ak103.ltx:293 |
| wpn_ak104 | wpn_ak103_pso | scope | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` | gamedata/config/weapons/w_ak103.ltx:366 |
| wpn_ak104 | wpn_ak103_rakurs | scope | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 | ambiguous relationship: ID stem `wpn_ak103`, inheritance-based base `wpn_ak104` | gamedata/config/weapons/w_ak103.ltx:312 |
| wpn_ak108 | — | base weapon | Пулемёт РПК-16 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_rpk16.ltx:2 |
| wpn_ak108 | wpn_rpk16_ac10632 | scope | Пулемёт РПК-16 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` | gamedata/config/weapons/w_rpk16.ltx:162 |
| wpn_ak108 | wpn_rpk16_eot | scope | Пулемёт РПК-16 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` | gamedata/config/weapons/w_rpk16.ltx:181 |
| wpn_ak108 | wpn_rpk16_microt1 | scope | Пулемёт РПК-16 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` | gamedata/config/weapons/w_rpk16.ltx:201 |
| wpn_ak108 | wpn_rpk16_specter | scope | Пулемёт РПК-16 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 | ambiguous relationship: ID stem `wpn_rpk16`, inheritance-based base `wpn_ak108` | gamedata/config/weapons/w_rpk16.ltx:221 |
| wpn_ak47 | — | base weapon | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | — | gamedata/config/weapons/w_sks.ltx:2 |
| wpn_ak47 | wpn_ak47_old | named variant | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | — | gamedata/config/weapons/w_sks.ltx:373 |
| wpn_ak47 | wpn_sks_1p78 | scope | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` | gamedata/config/weapons/w_sks.ltx:347 |
| wpn_ak47 | wpn_sks_ekp | scope | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` | gamedata/config/weapons/w_sks.ltx:259 |
| wpn_ak47 | wpn_sks_okp | scope | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` | gamedata/config/weapons/w_sks.ltx:280 |
| wpn_ak47 | wpn_sks_pso | scope | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` | gamedata/config/weapons/w_sks.ltx:322 |
| wpn_ak47 | wpn_sks_rakurs | scope | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS | ambiguous relationship: ID stem `wpn_sks`, inheritance-based base `wpn_ak47` | gamedata/config/weapons/w_sks.ltx:301 |
| wpn_ak74_m1 | — | base weapon | Автомат Стрелка | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74_m2` | gamedata/config/weapons/w_ak_strelok.ltx:2 |
| wpn_ak74_m1 | wpn_ak74_m1_ac10632 | scope | Автомат Стрелка | — | — | gamedata/config/weapons/w_ak_strelok.ltx:79 |
| wpn_ak74_m1 | wpn_ak74_m1_eot | scope | Автомат Стрелка | — | — | gamedata/config/weapons/w_ak_strelok.ltx:87 |
| wpn_ak74_m1 | wpn_ak74_m1_microt1 | scope | Автомат Стрелка | — | — | gamedata/config/weapons/w_ak_strelok.ltx:93 |
| wpn_ak74_m1 | wpn_ak74_m1_specter | scope | Автомат Стрелка | — | — | gamedata/config/weapons/w_ak_strelok.ltx:99 |
| wpn_ak74_m2 | — | base weapon | Автомат Стрелка М | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_ak_strelok_rem.ltx:2 |
| wpn_ak74_m2 | wpn_ak12_ac10632 | scope | Автомат Стрелка М | — | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` | gamedata/config/weapons/w_ak_strelok_rem.ltx:319 |
| wpn_ak74_m2 | wpn_ak12_eot | scope | Автомат Стрелка М | — | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` | gamedata/config/weapons/w_ak_strelok_rem.ltx:338 |
| wpn_ak74_m2 | wpn_ak12_microt1 | scope | Автомат Стрелка М | — | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` | gamedata/config/weapons/w_ak_strelok_rem.ltx:358 |
| wpn_ak74_m2 | wpn_ak12_specter | scope | Автомат Стрелка М | — | ambiguous relationship: ID stem `wpn_ak12`, inheritance-based base `wpn_ak74_m2` | gamedata/config/weapons/w_ak_strelok_rem.ltx:378 |
| wpn_ak74_m3 | — | base weapon | Пистолет Colt M45A1 | section ID says AK-74 M3, but the resolved name is Colt M45A1 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_colt1911` | gamedata/config/weapons/w_colt_m45a1.ltx:2 |
| wpn_ak74 | — | base weapon | Автомат АКС-74 | — | — | gamedata/config/weapons/w_ak74.ltx:2 |
| wpn_ak74 | wpn_ak74_old | named variant | Автомат АКС-74 | — | — | gamedata/config/weapons/w_ak74.ltx:433 |
| wpn_ak74m | — | base weapon | Автомат АКС-74Н | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_ak74.ltx:302 |
| wpn_ak74m | wpn_ak74m_1p78 | scope | Автомат АКС-74Н | — | — | gamedata/config/weapons/w_ak74.ltx:380 |
| wpn_ak74m | wpn_ak74m_ekp | scope | Автомат АКС-74Н | — | — | gamedata/config/weapons/w_ak74.ltx:318 |
| wpn_ak74m | wpn_ak74m_okp | scope | Автомат АКС-74Н | — | — | gamedata/config/weapons/w_ak74.ltx:338 |
| wpn_ak74m | wpn_ak74m_pso | scope | Автомат АКС-74Н | — | — | gamedata/config/weapons/w_ak74.ltx:405 |
| wpn_ak74m | wpn_ak74m_rakurs | scope | Автомат АКС-74Н | — | — | gamedata/config/weapons/w_ak74.ltx:359 |
| wpn_ak74m | wpn_ak74pbn | named variant | Автомат АКС-74Н | — | ambiguous relationship: ID stem `wpn_ak74pbn`, inheritance-based base `wpn_ak74m` | gamedata/config/weapons/w_ak74.ltx:431 |
| wpn_ak74u | — | base weapon | Автомат АКС-74УН | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_ak74u.ltx:7 |
| wpn_ak74u | wpn_ak74u_1p78 | scope | Автомат АКС-74УН | — | — | gamedata/config/weapons/w_ak74u.ltx:275 |
| wpn_ak74u | wpn_ak74u_ekp | scope | Автомат АКС-74УН | — | — | gamedata/config/weapons/w_ak74u.ltx:210 |
| wpn_ak74u | wpn_ak74u_okp | scope | Автомат АКС-74УН | — | — | gamedata/config/weapons/w_ak74u.ltx:233 |
| wpn_ak74u | wpn_ak74u_old | named variant | Автомат АКС-74УН | — | — | gamedata/config/weapons/w_ak74u.ltx:301 |
| wpn_ak74u | wpn_ak74u_rakurs | scope | Автомат АКС-74УН | — | — | gamedata/config/weapons/w_ak74u.ltx:254 |
| wpn_akm | — | base weapon | Автомат АКМН | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_akm.ltx:2 |
| wpn_akm | wpn_akm_1p78 | scope | Автомат АКМН | — | — | gamedata/config/weapons/w_akm.ltx:284 |
| wpn_akm | wpn_akm_ekp | scope | Автомат АКМН | — | — | gamedata/config/weapons/w_akm.ltx:222 |
| wpn_akm | wpn_akm_okp | scope | Автомат АКМН | — | — | gamedata/config/weapons/w_akm.ltx:242 |
| wpn_akm | wpn_akm_pso | scope | Автомат АКМН | — | — | gamedata/config/weapons/w_akm.ltx:310 |
| wpn_akm | wpn_akm_rakurs | scope | Автомат АКМН | — | — | gamedata/config/weapons/w_akm.ltx:263 |
| wpn_aks47 | — | base weapon | Пистолет-пулемёт TDI KRISS Vector | section/string ID says AKS47, but the resolved name is TDI KRISS Vector | classified as a named base from its own inv_name; technical inheritance parent is `wpn_colt1911` | gamedata/config/weapons/w_vector.ltx:2 |
| wpn_aks47 | wpn_vector_ac10632 | scope | Пистолет-пулемёт TDI KRISS Vector | section/string ID says AKS47, but the resolved name is TDI KRISS Vector | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` | gamedata/config/weapons/w_vector.ltx:158 |
| wpn_aks47 | wpn_vector_eot | scope | Пистолет-пулемёт TDI KRISS Vector | section/string ID says AKS47, but the resolved name is TDI KRISS Vector | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` | gamedata/config/weapons/w_vector.ltx:177 |
| wpn_aks47 | wpn_vector_microt1 | scope | Пистолет-пулемёт TDI KRISS Vector | section/string ID says AKS47, but the resolved name is TDI KRISS Vector | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` | gamedata/config/weapons/w_vector.ltx:196 |
| wpn_aks47 | wpn_vector_specter | scope | Пистолет-пулемёт TDI KRISS Vector | section/string ID says AKS47, but the resolved name is TDI KRISS Vector | ambiguous relationship: ID stem `wpn_vector`, inheritance-based base `wpn_aks47` | gamedata/config/weapons/w_vector.ltx:216 |
| wpn_aps | — | base weapon | Пистолет АПБ | section ID says APS, but the resolved name is APB | — | gamedata/config/weapons/w_apb.ltx:2 |
| wpn_awm | — | base weapon | Винтовка AWM-F | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_svd` | gamedata/config/weapons/w_awm.ltx:2 |
| wpn_beretta | — | base weapon | Пистолет Beretta M9A1 | — | — | gamedata/config/weapons/w_beretta92.ltx:2 |
| wpn_beretta93 | — | base weapon | Пистолет FN FNX-45 | section ID says Beretta 93, but the resolved name is FN FNX-45 | — | gamedata/config/weapons/w_fn_fnx.ltx:2 |
| wpn_binoc | — | base device | UNRESOLVED | string ID `binocular` is not present in string_table_enc_weapons.xml | — | gamedata/config/weapons/w_binoc.ltx:2 |
| wpn_bizon | — | base weapon | Пистолет-пулемёт ПП-19 «Бизон» | — | — | gamedata/config/weapons/w_bizon-2.ltx:2 |
| wpn_bizon | wpn_bizon_ekp | scope | Пистолет-пулемёт ПП-19 «Бизон» | — | — | gamedata/config/weapons/w_bizon-2.ltx:230 |
| wpn_bizon | wpn_bizon_okp | scope | Пистолет-пулемёт ПП-19 «Бизон» | — | — | gamedata/config/weapons/w_bizon-2.ltx:255 |
| wpn_bizon | wpn_bizon_rakurs | scope | Пистолет-пулемёт ПП-19 «Бизон» | — | — | gamedata/config/weapons/w_bizon-2.ltx:280 |
| wpn_bm16_full | — | base weapon | Ружьё ТОЗ-66 | — | — | gamedata/config/weapons/w_toz66.ltx:2 |
| wpn_bm16 | — | base weapon | Обрез ТОЗ-66 | — | — | gamedata/config/weapons/w_toz66_short.ltx:2 |
| wpn_colt1911 | — | base weapon | Пистолет Colt M1911A1 | — | — | gamedata/config/weapons/w_1911.ltx:2 |
| wpn_desert_eagle | — | base weapon | Пистолет IMI Desert Eagle | — | — | gamedata/config/weapons/w_desert_eagle.ltx:2 |
| wpn_desert_eagle | wpn_desert_eagle_po | named variant | Пистолет IMI Desert Eagle | — | — | gamedata/config/weapons/w_desert_eagle.ltx:230 |
| wpn_eagle_m1 | — | base weapon | Револьвер Taurus Raging Bull | section/string ID says Eagle M1, but the resolved name is Taurus Raging Bull | classified as a named base from its own inv_name; technical inheritance parent is `wpn_desert_eagle` | gamedata/config/weapons/w_taurus.ltx:2 |
| wpn_flame | — | base weapon | Огнемёт | — | — | gamedata/config/weapons/w_flame.ltx:2 |
| wpn_fn2000 | — | base weapon | Автомат FN F2000 Tactical (FDE) | — | — | gamedata/config/weapons/w_fn2000.ltx:2 |
| wpn_fn2000 | wpn_fn2000_nimble_ac10632 | scope | Автомат FN F2000 Tactical (FDE) | — | — | gamedata/config/weapons/w_fn2000.ltx:300 |
| wpn_fn2000 | wpn_fn2000_nimble_eot | scope | Автомат FN F2000 Tactical (FDE) | — | — | gamedata/config/weapons/w_fn2000.ltx:318 |
| wpn_fn2000 | wpn_fn2000_nimble_microt1 | scope | Автомат FN F2000 Tactical (FDE) | — | — | gamedata/config/weapons/w_fn2000.ltx:336 |
| wpn_fn2000 | wpn_fn2000_nimble_specter | scope | Автомат FN F2000 Tactical (FDE) | — | — | gamedata/config/weapons/w_fn2000.ltx:354 |
| wpn_fort_m1 | — | base weapon | Пистолет ОЦ-33 «Пернач» | section/string ID says Fort M1, but the resolved name is OTs-33 Pernach | classified as a named base from its own inv_name; technical inheritance parent is `wpn_fort` | gamedata/config/weapons/w_pernach.ltx:2 |
| wpn_fort | — | base weapon | Пистолет Форт-12 | — | — | gamedata/config/weapons/w_fort12.ltx:2 |
| wpn_g36 | — | base weapon | Автомат G36 | — | — | gamedata/config/weapons/w_g36.ltx:2 |
| wpn_g36c | — | base weapon | Пулемёт HK MG36 | section ID says G36C, but the resolved name is HK MG36 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_g36` | gamedata/config/weapons/w_mg36.ltx:2 |
| wpn_g36c | wpn_g36_nimble_ac10632 | scope | Пулемёт HK MG36 | section ID says G36C, but the resolved name is HK MG36 | — | gamedata/config/weapons/w_mg36.ltx:162 |
| wpn_g36c | wpn_g36_nimble_eot | scope | Пулемёт HK MG36 | section ID says G36C, but the resolved name is HK MG36 | — | gamedata/config/weapons/w_mg36.ltx:179 |
| wpn_g36c | wpn_g36_nimble_microt1 | scope | Пулемёт HK MG36 | section ID says G36C, but the resolved name is HK MG36 | — | gamedata/config/weapons/w_mg36.ltx:197 |
| wpn_g36c | wpn_g36_nimble_specter | scope | Пулемёт HK MG36 | section ID says G36C, but the resolved name is HK MG36 | — | gamedata/config/weapons/w_mg36.ltx:215 |
| wpn_gauss_auto | — | base weapon | Гаусс-винтовка | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_gauss` | gamedata/config/weapons/w_gauss_rifle.ltx:219 |
| wpn_gauss | — | base weapon | Гаусс-винтовка | — | — | gamedata/config/weapons/w_gauss_rifle.ltx:1 |
| wpn_glock_m1 | — | base weapon | Пистолет Glock 18C | string ID says Glock 17 M1, but the resolved name is Glock 18C | classified as a named base from its own inv_name; technical inheritance parent is `wpn_glock` | gamedata/config/weapons/w_glock17.ltx:222 |
| wpn_glock_m2 | — | base weapon | Пистолет-пулемёт BT MP9 | string ID says Glock 18, but the resolved name is BT MP9 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_glock` | gamedata/config/weapons/w_glock17.ltx:281 |
| wpn_glock_m2 | wpn_mp9_ac10632 | scope | Пистолет-пулемёт BT MP9 | string ID says Glock 18, but the resolved name is BT MP9 | ambiguous relationship: ID stem `wpn_mp9`, inheritance-based base `wpn_glock_m2` | gamedata/config/weapons/w_glock17.ltx:424 |
| wpn_glock_m2 | wpn_mp9_eot | scope | Пистолет-пулемёт BT MP9 | string ID says Glock 18, but the resolved name is BT MP9 | ambiguous relationship: ID stem `wpn_mp9`, inheritance-based base `wpn_glock_m2` | gamedata/config/weapons/w_glock17.ltx:444 |
| wpn_glock_m2 | wpn_mp9_microt1 | scope | Пистолет-пулемёт BT MP9 | string ID says Glock 18, but the resolved name is BT MP9 | ambiguous relationship: ID stem `wpn_mp9`, inheritance-based base `wpn_glock_m2` | gamedata/config/weapons/w_glock17.ltx:463 |
| wpn_glock | — | base weapon | Пистолет Glock 17 | — | — | gamedata/config/weapons/w_glock17.ltx:2 |
| wpn_gm94 | — | base weapon | Гранатомёт HK M320 | — | — | gamedata/config/weapons/w_hk_m320.ltx:2 |
| wpn_groza_nimble | — | base weapon | Автомат ОЦ-14-4А «Шторм» | string ID says Groza Nimble, but the resolved name is Groza «Шторм» | classified as a named base from its own inv_name; technical inheritance parent is `wpn_groza` | gamedata/config/weapons/w_groza.ltx:306 |
| wpn_groza_nimble | wpn_groza_nimble_ac10632 | scope | Автомат ОЦ-14-4А «Шторм» | string ID says Groza Nimble, but the resolved name is Groza «Шторм» | — | gamedata/config/weapons/w_groza.ltx:344 |
| wpn_groza_nimble | wpn_groza_nimble_eot | scope | Автомат ОЦ-14-4А «Шторм» | string ID says Groza Nimble, but the resolved name is Groza «Шторм» | — | gamedata/config/weapons/w_groza.ltx:364 |
| wpn_groza_nimble | wpn_groza_nimble_microt1 | scope | Автомат ОЦ-14-4А «Шторм» | string ID says Groza Nimble, but the resolved name is Groza «Шторм» | — | gamedata/config/weapons/w_groza.ltx:383 |
| wpn_groza_nimble | wpn_groza_nimble_specter | scope | Автомат ОЦ-14-4А «Шторм» | string ID says Groza Nimble, but the resolved name is Groza «Шторм» | — | gamedata/config/weapons/w_groza.ltx:402 |
| wpn_groza | — | base weapon | Автомат ОЦ-14-4А «Гроза» | — | — | gamedata/config/weapons/w_groza.ltx:2 |
| wpn_gungauss | — | base weapon | Гаусс-пистолет | — | — | gamedata/config/weapons/w_gauss_pistol.ltx:1 |
| wpn_hunters_toz_new | — | base weapon | Ружьё Ореха | section ID says Hunters TOZ New, but the resolved name is Orex shotgun | — | gamedata/config/weapons/w_oreh_shotgun.ltx:1 |
| wpn_knife_m1 | — | base weapon |  Нож KM2000 | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_knife` | gamedata/config/weapons/w_knife.ltx:134 |
| wpn_knife | — | base weapon | Нож 6Х4 | — | — | gamedata/config/weapons/w_knife.ltx:2 |
| wpn_m110 | — | base weapon | Винтовка FN SCAR-H | string ID says M110, but the resolved name is FN SCAR-H | — | gamedata/config/weapons/w_fn_scar.ltx:2 |
| wpn_m110 | wpn_scarh_ac10632 | scope | Винтовка FN SCAR-H | string ID says M110, but the resolved name is FN SCAR-H | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` | gamedata/config/weapons/w_fn_scar.ltx:351 |
| wpn_m110 | wpn_scarh_eot | scope | Винтовка FN SCAR-H | string ID says M110, but the resolved name is FN SCAR-H | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` | gamedata/config/weapons/w_fn_scar.ltx:369 |
| wpn_m110 | wpn_scarh_microt1 | scope | Винтовка FN SCAR-H | string ID says M110, but the resolved name is FN SCAR-H | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` | gamedata/config/weapons/w_fn_scar.ltx:386 |
| wpn_m110 | wpn_scarh_specter | scope | Винтовка FN SCAR-H | string ID says M110, but the resolved name is FN SCAR-H | ambiguous relationship: ID stem `wpn_scarh`, inheritance-based base `wpn_m110` | gamedata/config/weapons/w_fn_scar.ltx:403 |
| wpn_m16a4 | — | base weapon | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | — | gamedata/config/weapons/w_m4a1.ltx:2 |
| wpn_m16a4 | wpn_lr300 | named variant | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` | gamedata/config/weapons/w_m4a1.ltx:452 |
| wpn_m16a4 | wpn_lr300_ac10632 | scope | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` | gamedata/config/weapons/w_m4a1.ltx:364 |
| wpn_m16a4 | wpn_lr300_eot | scope | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` | gamedata/config/weapons/w_m4a1.ltx:382 |
| wpn_m16a4 | wpn_lr300_m1 | named variant | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` | gamedata/config/weapons/w_m4a1.ltx:455 |
| wpn_m16a4 | wpn_lr300_microt1 | scope | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` | gamedata/config/weapons/w_m4a1.ltx:401 |
| wpn_m16a4 | wpn_lr300_specter | scope | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 | ambiguous relationship: ID stem `wpn_lr300`, inheritance-based base `wpn_m16a4` | gamedata/config/weapons/w_m4a1.ltx:420 |
| wpn_mosin | — | base weapon | Винтовка Мосина | — | — | gamedata/config/weapons/w_mosin.ltx:2 |
| wpn_mosin | wpn_mosin_pu | named variant | Винтовка Мосина | — | — | gamedata/config/weapons/w_mosin.ltx:237 |
| wpn_mp153_m1 | — | base weapon | Короткое ружьё МР-153 | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_winchester_m1` | gamedata/config/weapons/w_mp153_short.ltx:2 |
| wpn_mp153 | — | base weapon | Ружьё Remington 870 (FD) | string ID says Winchester M1, but the resolved name is Remington 870 | — | gamedata/config/weapons/w_remington870.ltx:2 |
| wpn_mp153 | wpn_wincheaster1300_trapper_ac10632 | scope | Ружьё Remington 870 (FD) | string ID says Winchester M1, but the resolved name is Remington 870 | ambiguous relationship: ID stem `wpn_wincheaster1300`, inheritance-based base `wpn_mp153` | gamedata/config/weapons/w_remington870.ltx:210 |
| wpn_mp153 | wpn_wincheaster1300_trapper_eot | scope | Ружьё Remington 870 (FD) | string ID says Winchester M1, but the resolved name is Remington 870 | ambiguous relationship: ID stem `wpn_wincheaster1300`, inheritance-based base `wpn_mp153` | gamedata/config/weapons/w_remington870.ltx:228 |
| wpn_mp153 | wpn_wincheaster1300_trapper_microt1 | scope | Ружьё Remington 870 (FD) | string ID says Winchester M1, but the resolved name is Remington 870 | ambiguous relationship: ID stem `wpn_wincheaster1300`, inheritance-based base `wpn_mp153` | gamedata/config/weapons/w_remington870.ltx:246 |
| wpn_mp5_m1 | — | base weapon | Пистолет-пулемёт HK MP5 9x18 | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_mp5` | gamedata/config/weapons/w_mp5_mod.ltx:2 |
| wpn_mp5_m1 | wpn_mp5_m1_ac10632 | scope | Пистолет-пулемёт HK MP5 9x18 | — | — | gamedata/config/weapons/w_mp5_mod.ltx:45 |
| wpn_mp5_m1 | wpn_mp5_m1_eot | scope | Пистолет-пулемёт HK MP5 9x18 | — | — | gamedata/config/weapons/w_mp5_mod.ltx:52 |
| wpn_mp5_m1 | wpn_mp5_m1_microt1 | scope | Пистолет-пулемёт HK MP5 9x18 | — | — | gamedata/config/weapons/w_mp5_mod.ltx:58 |
| wpn_mp5_m1 | wpn_mp5_m1_specter | scope | Пистолет-пулемёт HK MP5 9x18 | — | — | gamedata/config/weapons/w_mp5_mod.ltx:64 |
| wpn_mp5_m2 | — | base weapon | Пистолет-пулемёт СР-2МП «Вереск» | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_mp5` | gamedata/config/weapons/w_mp5_mod.ltx:73 |
| wpn_mp5_m2 | wpn_sr2m_ac10632 | scope | Пистолет-пулемёт СР-2МП «Вереск» | — | ambiguous relationship: ID stem `wpn_sr2m`, inheritance-based base `wpn_mp5_m2` | gamedata/config/weapons/w_mp5_mod.ltx:203 |
| wpn_mp5_m2 | wpn_sr2m_eot | scope | Пистолет-пулемёт СР-2МП «Вереск» | — | ambiguous relationship: ID stem `wpn_sr2m`, inheritance-based base `wpn_mp5_m2` | gamedata/config/weapons/w_mp5_mod.ltx:223 |
| wpn_mp5_m2 | wpn_sr2m_microt1 | scope | Пистолет-пулемёт СР-2МП «Вереск» | — | ambiguous relationship: ID stem `wpn_sr2m`, inheritance-based base `wpn_mp5_m2` | gamedata/config/weapons/w_mp5_mod.ltx:243 |
| wpn_mp5 | — | base weapon | Пистолет-пулемёт HK MP5 | — | — | gamedata/config/weapons/w_mp5.ltx:2 |
| wpn_mp5 | wpn_mp5_ac10632 | scope | Пистолет-пулемёт HK MP5 | — | — | gamedata/config/weapons/w_mp5.ltx:214 |
| wpn_mp5 | wpn_mp5_eot | scope | Пистолет-пулемёт HK MP5 | — | — | gamedata/config/weapons/w_mp5.ltx:234 |
| wpn_mp5 | wpn_mp5_microt1 | scope | Пистолет-пулемёт HK MP5 | — | — | gamedata/config/weapons/w_mp5.ltx:254 |
| wpn_mp5 | wpn_mp5_specter | scope | Пистолет-пулемёт HK MP5 | — | — | gamedata/config/weapons/w_mp5.ltx:273 |
| wpn_mr308 | — | base weapon | Винтовка HК MR308 HK G28 | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_m110` | gamedata/config/weapons/w_fn_scar.ltx:438 |
| wpn_mr308 | wpn_mr308_g28_scope | scope | Винтовка HК MR308 HK G28 | — | — | gamedata/config/weapons/w_fn_scar.ltx:572 |
| wpn_pb | — | base weapon | Пистолет ПБ | — | — | gamedata/config/weapons/w_pb.ltx:2 |
| wpn_pkm | — | base weapon | Пулемёт ПКМ | — | — | gamedata/config/weapons/w_pkm.ltx:2 |
| wpn_pkp | — | base weapon | Пулемёт ПКМ Тактический | string ID says PKP, but the resolved name is tactical PKM | classified as a named base from its own inv_name; technical inheritance parent is `wpn_pkm` | gamedata/config/weapons/w_pkm_mod.ltx:2 |
| wpn_pkp | wpn_pkm_zulus_ac10632 | scope | Пулемёт ПКМ Тактический | string ID says PKP, but the resolved name is tactical PKM | ambiguous relationship: ID stem `wpn_pkm`, inheritance-based base `wpn_pkp` | gamedata/config/weapons/w_pkm_mod.ltx:147 |
| wpn_pkp | wpn_pkm_zulus_eot | scope | Пулемёт ПКМ Тактический | string ID says PKP, but the resolved name is tactical PKM | ambiguous relationship: ID stem `wpn_pkm`, inheritance-based base `wpn_pkp` | gamedata/config/weapons/w_pkm_mod.ltx:165 |
| wpn_pkp | wpn_pkm_zulus_microt1 | scope | Пулемёт ПКМ Тактический | string ID says PKP, but the resolved name is tactical PKM | ambiguous relationship: ID stem `wpn_pkm`, inheritance-based base `wpn_pkp` | gamedata/config/weapons/w_pkm_mod.ltx:185 |
| wpn_pm | — | base weapon | Пистолет ПМ | — | — | gamedata/config/weapons/w_pm.ltx:2 |
| wpn_pp19 | — | base weapon | Пистолет-пулемёт "Витязь-СН" | — | — | gamedata/config/weapons/w_vityaz.ltx:2 |
| wpn_pp19 | wpn_vityaz_ac10632 | scope | Пистолет-пулемёт "Витязь-СН" | — | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` | gamedata/config/weapons/w_vityaz.ltx:229 |
| wpn_pp19 | wpn_vityaz_eot | scope | Пистолет-пулемёт "Витязь-СН" | — | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` | gamedata/config/weapons/w_vityaz.ltx:253 |
| wpn_pp19 | wpn_vityaz_microt1 | scope | Пистолет-пулемёт "Витязь-СН" | — | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` | gamedata/config/weapons/w_vityaz.ltx:281 |
| wpn_pp19 | wpn_vityaz_specter | scope | Пистолет-пулемёт "Витязь-СН" | — | ambiguous relationship: ID stem `wpn_vityaz`, inheritance-based base `wpn_pp19` | gamedata/config/weapons/w_vityaz.ltx:305 |
| wpn_rg6 | — | base weapon | Гранатомёт РГ-6 «Гном» | — | — | gamedata/config/weapons/w_rg6.ltx:1 |
| wpn_rpg7 | — | base weapon | РПГ-7 | — | — | gamedata/config/weapons/w_rpg7.ltx:2 |
| wpn_rpk | — | base weapon | Пулемёт РПК | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_rpk74` | gamedata/config/weapons/w_rpk74.ltx:311 |
| wpn_rpk | wpn_rpk74m_1p78 | scope | Пулемёт РПК | — | — | gamedata/config/weapons/w_rpk74.ltx:488 |
| wpn_rpk | wpn_rpk74m_ekp | scope | Пулемёт РПК | — | — | gamedata/config/weapons/w_rpk74.ltx:426 |
| wpn_rpk | wpn_rpk74m_okp | scope | Пулемёт РПК | — | — | gamedata/config/weapons/w_rpk74.ltx:447 |
| wpn_rpk | wpn_rpk74m_pso | scope | Пулемёт РПК | — | — | gamedata/config/weapons/w_rpk74.ltx:513 |
| wpn_rpk | wpn_rpk74m_rakurs | scope | Пулемёт РПК | — | — | gamedata/config/weapons/w_rpk74.ltx:467 |
| wpn_rpk74 | — | base weapon | Пулемёт РПК-74Н | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak74` | gamedata/config/weapons/w_rpk74.ltx:2 |
| wpn_rpk74 | wpn_rpk74_1p78 | scope | Пулемёт РПК-74Н | — | — | gamedata/config/weapons/w_rpk74.ltx:258 |
| wpn_rpk74 | wpn_rpk74_ekp | scope | Пулемёт РПК-74Н | — | — | gamedata/config/weapons/w_rpk74.ltx:198 |
| wpn_rpk74 | wpn_rpk74_okp | scope | Пулемёт РПК-74Н | — | — | gamedata/config/weapons/w_rpk74.ltx:216 |
| wpn_rpk74 | wpn_rpk74_pso | scope | Пулемёт РПК-74Н | — | — | gamedata/config/weapons/w_rpk74.ltx:283 |
| wpn_rpk74 | wpn_rpk74_rakurs | scope | Пулемёт РПК-74Н | — | — | gamedata/config/weapons/w_rpk74.ltx:236 |
| wpn_saiga12c_m1 | — | base weapon | Карабин Сайга-12М | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_saiga12c` | gamedata/config/weapons/w_saiga12_mod.ltx:1 |
| wpn_saiga12c_m1 | wpn_saiga12m_ac10632 | scope | Карабин Сайга-12М | — | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` | gamedata/config/weapons/w_saiga12_mod.ltx:154 |
| wpn_saiga12c_m1 | wpn_saiga12m_eot | scope | Карабин Сайга-12М | — | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` | gamedata/config/weapons/w_saiga12_mod.ltx:172 |
| wpn_saiga12c_m1 | wpn_saiga12m_microt1 | scope | Карабин Сайга-12М | — | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` | gamedata/config/weapons/w_saiga12_mod.ltx:190 |
| wpn_saiga12c_m1 | wpn_saiga12m_specter | scope | Карабин Сайга-12М | — | ambiguous relationship: ID stem `wpn_saiga12m`, inheritance-based base `wpn_saiga12c_m1` | gamedata/config/weapons/w_saiga12_mod.ltx:210 |
| wpn_saiga12c_m2 | — | base weapon | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | classified as a named base from its own inv_name; technical inheritance parent is `wpn_ak47` | gamedata/config/weapons/w_vpo101_hunter.ltx:2 |
| wpn_saiga12c_m2 | wpn_vpo101_1p78 | scope | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` | gamedata/config/weapons/w_vpo101_hunter.ltx:281 |
| wpn_saiga12c_m2 | wpn_vpo101_ekp | scope | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` | gamedata/config/weapons/w_vpo101_hunter.ltx:212 |
| wpn_saiga12c_m2 | wpn_vpo101_okp | scope | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` | gamedata/config/weapons/w_vpo101_hunter.ltx:235 |
| wpn_saiga12c_m2 | wpn_vpo101_pso | scope | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` | gamedata/config/weapons/w_vpo101_hunter.ltx:185 |
| wpn_saiga12c_m2 | wpn_vpo101_rakurs | scope | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» | ambiguous relationship: ID stem `wpn_vpo101`, inheritance-based base `wpn_saiga12c_m2` | gamedata/config/weapons/w_vpo101_hunter.ltx:258 |
| wpn_saiga12c | — | base weapon | Карабин Сайга-12К | — | — | gamedata/config/weapons/w_saiga12c.ltx:2 |
| wpn_saiga12c | wpn_saiga12_ekp | scope | Карабин Сайга-12К | — | — | gamedata/config/weapons/w_saiga12c.ltx:200 |
| wpn_saiga12c | wpn_saiga12_okp | scope | Карабин Сайга-12К | — | — | gamedata/config/weapons/w_saiga12c.ltx:227 |
| wpn_saiga12c | wpn_saiga12_rakurs | scope | Карабин Сайга-12К | — | — | gamedata/config/weapons/w_saiga12c.ltx:248 |
| wpn_sig220 | — | base weapon | Пистолет SIG-Sauer P220 | — | — | gamedata/config/weapons/w_sig220.ltx:2 |
| wpn_sig550 | — | base weapon | Автомат SIG SG 552 Commando | — | — | gamedata/config/weapons/w_sig552.ltx:2 |
| wpn_sig550 | wpn_sig550_ac10632 | scope | Автомат SIG SG 552 Commando | — | — | gamedata/config/weapons/w_sig552.ltx:249 |
| wpn_sig550 | wpn_sig550_eot | scope | Автомат SIG SG 552 Commando | — | — | gamedata/config/weapons/w_sig552.ltx:266 |
| wpn_sig550 | wpn_sig550_microt1 | scope | Автомат SIG SG 552 Commando | — | — | gamedata/config/weapons/w_sig552.ltx:285 |
| wpn_sig550 | wpn_sig550_specter | scope | Автомат SIG SG 552 Commando | — | — | gamedata/config/weapons/w_sig552.ltx:304 |
| wpn_soc308_m1 | — | base weapon | Автомат DS Arms SA-58 custom | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_soc308` | gamedata/config/weapons/w_sa58.ltx:214 |
| wpn_soc308_m1 | wpn_soc308_m1_ac10632 | scope | Автомат DS Arms SA-58 custom | — | — | gamedata/config/weapons/w_sa58.ltx:252 |
| wpn_soc308_m1 | wpn_soc308_m1_eot | scope | Автомат DS Arms SA-58 custom | — | — | gamedata/config/weapons/w_sa58.ltx:269 |
| wpn_soc308_m1 | wpn_soc308_m1_microt1 | scope | Автомат DS Arms SA-58 custom | — | — | gamedata/config/weapons/w_sa58.ltx:286 |
| wpn_soc308_m1 | wpn_soc308_m1_specter | scope | Автомат DS Arms SA-58 custom | — | — | gamedata/config/weapons/w_sa58.ltx:303 |
| wpn_soc308 | — | base weapon | Автомат DS Arms SA-58 | — | — | gamedata/config/weapons/w_sa58.ltx:4 |
| wpn_spas12_m1 | — | base weapon | Ружьё Benelli M4 Super 90 (M1014) | section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_spas12` | gamedata/config/weapons/w_spas12.ltx:207 |
| wpn_spas12_m1 | wpn_spas12_nimble_ac10632 | scope | Ружьё Benelli M4 Super 90 (M1014) | section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | — | gamedata/config/weapons/w_spas12.ltx:343 |
| wpn_spas12_m1 | wpn_spas12_nimble_eot | scope | Ружьё Benelli M4 Super 90 (M1014) | section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | — | gamedata/config/weapons/w_spas12.ltx:362 |
| wpn_spas12_m1 | wpn_spas12_nimble_microt1 | scope | Ружьё Benelli M4 Super 90 (M1014) | section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 | — | gamedata/config/weapons/w_spas12.ltx:381 |
| wpn_spas12 | — | base weapon | Ружьё Franchi SPAS-12 | — | — | gamedata/config/weapons/w_spas12.ltx:2 |
| wpn_spas15_m1 | — | base weapon | Ружьё Daewoo USAS-12 М1 | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | — | gamedata/config/weapons/w_usas12_mod.ltx:2 |
| wpn_spas15_m1 | wpn_usas12_m1_ac10632 | scope | Ружьё Daewoo USAS-12 М1 | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` | gamedata/config/weapons/w_usas12_mod.ltx:217 |
| wpn_spas15_m1 | wpn_usas12_m1_eot | scope | Ружьё Daewoo USAS-12 М1 | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` | gamedata/config/weapons/w_usas12_mod.ltx:236 |
| wpn_spas15_m1 | wpn_usas12_m1_microt1 | scope | Ружьё Daewoo USAS-12 М1 | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` | gamedata/config/weapons/w_usas12_mod.ltx:255 |
| wpn_spas15_m1 | wpn_usas12_m1_specter | scope | Ружьё Daewoo USAS-12 М1 | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 | ambiguous relationship: ID stem `wpn_usas12`, inheritance-based base `wpn_spas15_m1` | gamedata/config/weapons/w_usas12_mod.ltx:275 |
| wpn_spas15 | — | base weapon | Ружьё Daewoo USAS-12 | section/string ID says SPAS-15, but the resolved name is Daewoo USAS-12 | — | gamedata/config/weapons/w_usas12.ltx:2 |
| wpn_sv98 | — | base weapon | Винтовка СВ-98М | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_svd` | gamedata/config/weapons/w_sv98.ltx:2 |
| wpn_svd | — | base weapon | Винтовка СВД | — | — | gamedata/config/weapons/w_svd.ltx:1 |
| wpn_svd | wpn_svd_1p78 | scope | Винтовка СВД | — | — | gamedata/config/weapons/w_svd.ltx:302 |
| wpn_svd | wpn_svd_ekp | scope | Винтовка СВД | — | — | gamedata/config/weapons/w_svd.ltx:232 |
| wpn_svd | wpn_svd_okp | scope | Винтовка СВД | — | — | gamedata/config/weapons/w_svd.ltx:256 |
| wpn_svd | wpn_svd_pso | scope | Винтовка СВД | — | — | gamedata/config/weapons/w_svd.ltx:329 |
| wpn_svd | wpn_svd_rakurs | scope | Винтовка СВД | — | — | gamedata/config/weapons/w_svd.ltx:279 |
| wpn_svu | — | base weapon | Винтовка СВУ-А | — | — | gamedata/config/weapons/w_svu.ltx:1 |
| wpn_svu | wpn_svu_1p78 | scope | Винтовка СВУ-А | — | — | gamedata/config/weapons/w_svu.ltx:284 |
| wpn_svu | wpn_svu_ekp | scope | Винтовка СВУ-А | — | — | gamedata/config/weapons/w_svu.ltx:227 |
| wpn_svu | wpn_svu_okp | scope | Винтовка СВУ-А | — | — | gamedata/config/weapons/w_svu.ltx:246 |
| wpn_svu | wpn_svu_pso | scope | Винтовка СВУ-А | — | — | gamedata/config/weapons/w_svu.ltx:308 |
| wpn_svu | wpn_svu_rakurs | scope | Винтовка СВУ-А | — | — | gamedata/config/weapons/w_svu.ltx:265 |
| wpn_toz34_m1 | — | base weapon | Ружьё Охотника | section ID says TOZ-34 M1, but the resolved name is Hunter shotgun | classified as a named base from its own inv_name; technical inheritance parent is `wpn_toz34` | gamedata/config/weapons/w_toz34_hunter.ltx:2 |
| wpn_toz34 | — | base weapon | Ружьё ТОЗ-34 | — | — | gamedata/config/weapons/w_toz34.ltx:2 |
| wpn_ump45 | — | base weapon | Пистолет-пулемёт HK UMP45 | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_mp5` | gamedata/config/weapons/w_ump45.ltx:2 |
| wpn_ump45 | wpn_mp5_nimble_ac10632 | scope | Пистолет-пулемёт HK UMP45 | — | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` | gamedata/config/weapons/w_ump45.ltx:226 |
| wpn_ump45 | wpn_mp5_nimble_eot | scope | Пистолет-пулемёт HK UMP45 | — | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` | gamedata/config/weapons/w_ump45.ltx:245 |
| wpn_ump45 | wpn_mp5_nimble_microt1 | scope | Пистолет-пулемёт HK UMP45 | — | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` | gamedata/config/weapons/w_ump45.ltx:264 |
| wpn_ump45 | wpn_mp5_nimble_specter | scope | Пистолет-пулемёт HK UMP45 | — | ambiguous relationship: ID stem `wpn_mp5`, inheritance-based base `wpn_ump45` | gamedata/config/weapons/w_ump45.ltx:284 |
| wpn_usp | — | base weapon | Пистолет HK USP | — | — | gamedata/config/weapons/w_usp.ltx:2 |
| wpn_val_nimble | — | base weapon | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | classified as a named base from its own inv_name; technical inheritance parent is `wpn_val` | gamedata/config/weapons/w_val.ltx:331 |
| wpn_val_nimble | wpn_val_nimble_1p78 | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:498 |
| wpn_val_nimble | wpn_val_nimble_ac10632 | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:551 |
| wpn_val_nimble | wpn_val_nimble_ekp | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:421 |
| wpn_val_nimble | wpn_val_nimble_eot | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:576 |
| wpn_val_nimble | wpn_val_nimble_microt1 | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:600 |
| wpn_val_nimble | wpn_val_nimble_okp | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:448 |
| wpn_val_nimble | wpn_val_nimble_pso | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:525 |
| wpn_val_nimble | wpn_val_nimble_rakurs | scope | Автомат СР-3М «Вихрь» | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» | — | gamedata/config/weapons/w_val.ltx:473 |
| wpn_val | — | base weapon | Автомат АС «Вал» | — | — | gamedata/config/weapons/w_val.ltx:2 |
| wpn_val | wpn_val_1p78 | scope | Автомат АС «Вал» | — | — | gamedata/config/weapons/w_val.ltx:280 |
| wpn_val | wpn_val_ekp | scope | Автомат АС «Вал» | — | — | gamedata/config/weapons/w_val.ltx:205 |
| wpn_val | wpn_val_okp | scope | Автомат АС «Вал» | — | — | gamedata/config/weapons/w_val.ltx:230 |
| wpn_val | wpn_val_pso | scope | Автомат АС «Вал» | — | — | gamedata/config/weapons/w_val.ltx:306 |
| wpn_val | wpn_val_rakurs | scope | Автомат АС «Вал» | — | — | gamedata/config/weapons/w_val.ltx:255 |
| wpn_vintorez_m1 | — | base weapon | Винтовка "ВССМ" | — | classified as a named base from its own inv_name; technical inheritance parent is `wpn_vintorez` | gamedata/config/weapons/w_vintorez_mod.ltx:2 |
| wpn_vintorez_m1 | wpn_vintorez_nimble_ac10632 | scope | Винтовка "ВССМ" | — | — | gamedata/config/weapons/w_vintorez_mod.ltx:100 |
| wpn_vintorez_m1 | wpn_vintorez_nimble_eot | scope | Винтовка "ВССМ" | — | — | gamedata/config/weapons/w_vintorez_mod.ltx:123 |
| wpn_vintorez_m1 | wpn_vintorez_nimble_microt1 | scope | Винтовка "ВССМ" | — | — | gamedata/config/weapons/w_vintorez_mod.ltx:147 |
| wpn_vintorez_m1 | wpn_vintorez_nimble_specter | scope | Винтовка "ВССМ" | — | — | gamedata/config/weapons/w_vintorez_mod.ltx:171 |
| wpn_vintorez | — | base weapon | Винтовка ВСС "Винторез" | — | — | gamedata/config/weapons/w_vintorez.ltx:2 |
| wpn_vintorez | wpn_vintorez_1p78 | scope | Винтовка ВСС "Винторез" | — | — | gamedata/config/weapons/w_vintorez.ltx:292 |
| wpn_vintorez | wpn_vintorez_ekp | scope | Винтовка ВСС "Винторез" | — | — | gamedata/config/weapons/w_vintorez.ltx:216 |
| wpn_vintorez | wpn_vintorez_okp | scope | Винтовка ВСС "Винторез" | — | — | gamedata/config/weapons/w_vintorez.ltx:241 |
| wpn_vintorez | wpn_vintorez_pso | scope | Винтовка ВСС "Винторез" | — | — | gamedata/config/weapons/w_vintorez.ltx:318 |
| wpn_vintorez | wpn_vintorez_rakurs | scope | Винтовка ВСС "Винторез" | — | — | gamedata/config/weapons/w_vintorez.ltx:267 |
| wpn_walther | — | base weapon | Пистолет Walther P99 | — | — | gamedata/config/weapons/w_walther.ltx:2 |
| wpn_wincheaster1300 | — | base weapon | Короткое ружьё MP-133 | section ID says Wincheaster 1300, but the resolved name is MP-133 | — | gamedata/config/weapons/w_mp133_short.ltx:2 |
| wpn_winchester_m1 | — | base weapon | Ружьё МР-153 | section ID says Winchester M1, but the resolved name is MP-153 | classified as a named base from its own inv_name; technical inheritance parent is `wpn_wincheaster1300` | gamedata/config/weapons/w_mp153.ltx:2 |


## Step 4 — `spawner.script` reconciliation

### Current weapon/add-on IDs

| id | spawner table | line(s) | merge status | resolved name | section alias / candidate | mismatch |
| :-- | :-- | :-- | :-- | :-- | :-- | :-- |
| st_wpn_b725 | weapons | 386 | missing section — no match | Ружьё Browning B725 | — | — |
| st_wpn_groza_nimble | weapons | 426 | missing section — string_id alias | Автомат ОЦ-14-4А «Шторм» | wpn_groza_nimble | string ID says Groza Nimble, but the resolved name is Groza «Шторм» |
| st_wpn_val_nimble | weapons | 427 | missing section — string_id alias | Автомат СР-3М «Вихрь» | wpn_val_nimble | string ID says Val Nimble, but the resolved name is SR-3M «Вихрь» |
| wpn_abakan | weapons | 406 | present | Автомат АН-94 | — | — |
| wpn_addon_1p78 | addons | 897 | present | Прицел 1П78 «Каштан» | — | — |
| wpn_addon_1pso | addons | 896 | present | Прицел ПСО-1М2 | — | — |
| wpn_addon_acog | addons | 901 | missing section — no match | — | — | — |
| wpn_addon_cobra | addons | 898 | present | Прицел «Кобра» ЭКП-8-02 | — | — |
| wpn_addon_eotech | addons | 902 | present | Прицел EOTech | — | — |
| wpn_addon_g36l | addons | 899 | present | Прицел G36 Red Dot | — | — |
| wpn_addon_grenade_launcher | addons | 906 | present | Подствольный гранатомет ГП-25 | — | — |
| wpn_addon_grenade_launcher_m203 | addons | 907 | present | Подствольный гранатомет M203 | — | — |
| wpn_addon_mt | addons | 900 | missing section — non-wpn/legacy alias | Прицел Aimpoint Micro T-1 | microt1 | — |
| wpn_addon_pe | addons | 903 | present | Прицел ПУ | — | — |
| wpn_addon_po | addons | 904 | present | Прицел Aimpoint Micro | — | — |
| wpn_addon_sil_545_ar | addons | 910 | present | Глушитель 5,45х39 | — | — |
| wpn_addon_sil_556_ar | addons | 911 | present | Глушитель 5,56х45 | — | — |
| wpn_addon_sil_762_ar | addons | 912 | present | Глушитель 7,62х39 | — | — |
| wpn_addon_sil_762_sr1 | addons | 913 | present | Глушитель 7,62х54 | — | — |
| wpn_addon_sil_762_sr2 | addons | 914 | present | Глушитель 7,62х51 | — | — |
| wpn_addon_sil_9_p | addons | 909 | present | Глушитель 9 мм | — | — |
| wpn_addon_sil_mosin | addons | 915 | present | Глушитель 7,62х54 R | — | — |
| wpn_aek971 | weapons | 422 | present — mismatch flagged | Автомат АК-104 «Тактик» | — | section ID says AEK-971, but the resolved name is AK-104 «Тактик» |
| wpn_ak_strelok | weapons | 423 | missing section — string_id alias | Автомат Стрелка | wpn_ak74_m1 | — |
| wpn_ak_strelok1 | weapons | 424 | missing section — string_id alias | Автомат Стрелка М | wpn_ak74_m2 | — |
| wpn_ak_strelok2 | weapons | 425 | missing section — no match | Автомат тюнингованный Шурупом | — | — |
| wpn_ak101 | weapons | 420 | missing section — string_id alias | Пулемёт РПК-16 | wpn_ak108 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 |
| wpn_ak103 | weapons | 421 | missing section — string_id alias | Автомат АК-103 | wpn_ak104 | section ID says AK-104, but the resolved name is AK-103 |
| wpn_ak104 | weapons | 442 | present — mismatch flagged | Автомат АК-103 | — | section ID says AK-104, but the resolved name is AK-103 |
| wpn_ak108 | weapons | 443 | present — mismatch flagged | Пулемёт РПК-16 | — | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 |
| wpn_ak47 | weapons | 400 | present — mismatch flagged | Карабин Молот ОП-СКС | — | section ID says AK-47, but the resolved name is Molot OP-SKS |
| wpn_ak47_old | weapons | 436 | present — mismatch flagged | Карабин Молот ОП-СКС | — | section ID says AK-47, but the resolved name is Molot OP-SKS |
| wpn_ak74 | weapons | 403 | present | Автомат АКС-74 | — | — |
| wpn_ak74_m1 | weapons | 438 | present | Автомат Стрелка | — | — |
| wpn_ak74_m2 | weapons | 439 | present | Автомат Стрелка М | — | — |
| wpn_ak74_m3 | weapons | 353 | present — mismatch flagged | Пистолет Colt M45A1 | — | section ID says AK-74 M3, but the resolved name is Colt M45A1 |
| wpn_ak74_old | weapons | 437 | present | Автомат АКС-74 | — | — |
| wpn_ak74m | weapons | 404 | present | Автомат АКС-74Н | — | — |
| wpn_ak74pbn | weapons | 440 | present | Автомат АКС-74Н | — | — |
| wpn_ak74u | weapons | 405 | present | Автомат АКС-74УН | — | — |
| wpn_ak74u_old | weapons | 441 | present | Автомат АКС-74УН | — | — |
| wpn_akm | weapons | 401 | present | Автомат АКМН | — | — |
| wpn_aks47 | weapons | 402 | present — mismatch flagged | Пистолет-пулемёт TDI KRISS Vector | — | section/string ID says AKS47, but the resolved name is TDI KRISS Vector |
| wpn_aps | weapons | 342 | present — mismatch flagged | Пистолет АПБ | — | section ID says APS, but the resolved name is APB |
| wpn_awm | weapons | 452 | present | Винтовка AWM-F | — | — |
| wpn_beretta | weapons | 343 | present | Пистолет Beretta M9A1 | — | — |
| wpn_beretta93 | weapons | 344 | present — mismatch flagged | Пистолет FN FNX-45 | — | section ID says Beretta 93, but the resolved name is FN FNX-45 |
| wpn_bizon | weapons | 364 | present | Пистолет-пулемёт ПП-19 «Бизон» | — | — |
| wpn_bm16 | weapons | 374 | present | Обрез ТОЗ-66 | — | — |
| wpn_bm16_full | weapons | 375 | present | Ружьё ТОЗ-66 | — | — |
| wpn_colt1911 | weapons | 345 | present | Пистолет Colt M1911A1 | — | — |
| wpn_desert_eagle | weapons | 348 | present | Пистолет IMI Desert Eagle | — | — |
| wpn_eagle_m1 | weapons | 349 | present — mismatch flagged | Револьвер Taurus Raging Bull | — | section/string ID says Eagle M1, but the resolved name is Taurus Raging Bull |
| wpn_flame | weapons | 465 | present | Огнемёт | — | — |
| wpn_fn2000 | weapons | 413 | present | Автомат FN F2000 Tactical (FDE) | — | — |
| wpn_fort | weapons | 340 | present | Пистолет Форт-12 | — | — |
| wpn_fort_m1 | weapons | 341 | present — mismatch flagged | Пистолет ОЦ-33 «Пернач» | — | section/string ID says Fort M1, but the resolved name is OTs-33 Pernach |
| wpn_g36 | weapons | 414 | present | Автомат G36 | — | — |
| wpn_g36c | weapons | 415 | present — mismatch flagged | Пулемёт HK MG36 | — | section ID says G36C, but the resolved name is HK MG36 |
| wpn_gauss | weapons | 469 | present | Гаусс-винтовка | — | — |
| wpn_gauss_auto_model | weapons | 479 | missing section — non-wpn/legacy alias | Гаусс-винтовка | wpn_gauss_auto | — |
| wpn_gauss_model | weapons | 478 | missing section — non-wpn/legacy alias | Гаусс-винтовка | wpn_gauss | — |
| wpn_glock | weapons | 355, 360 | present | Пистолет Glock 17 | — | — |
| wpn_glock_m1 | weapons | 356 | present — mismatch flagged | Пистолет Glock 18C | — | string ID says Glock 17 M1, but the resolved name is Glock 18C |
| wpn_glock_m2 | weapons | 357 | present — mismatch flagged | Пистолет-пулемёт BT MP9 | — | string ID says Glock 18, but the resolved name is BT MP9 |
| wpn_gm94 | weapons | 467 | present | Гранатомёт HK M320 | — | — |
| wpn_groza | weapons | 407 | present | Автомат ОЦ-14-4А «Гроза» | — | — |
| wpn_gungauss | weapons | 350 | present | Гаусс-пистолет | — | — |
| wpn_hunters | weapons | 387 | missing section — string_id alias | Ружьё Охотника | wpn_toz34_m1 | section ID says TOZ-34 M1, but the resolved name is Hunter shotgun |
| wpn_hunters_toz_new | weapons | 394 | present — mismatch flagged | Ружьё Ореха | — | section ID says Hunters TOZ New, but the resolved name is Orex shotgun |
| wpn_knife | weapons | 464 | present | Нож 6Х4 | — | — |
| wpn_knife_m1 | weapons | 477 | present |  Нож KM2000 | — | — |
| wpn_lr300 | weapons | 459 | present — mismatch flagged | Автомат M4A1 | — | section ID says M16A4, but the resolved name is M4A1 |
| wpn_lr300_m1 | weapons | 460 | present — mismatch flagged | Автомат M4A1 | — | section ID says M16A4, but the resolved name is M4A1 |
| wpn_m110 | weapons | 453 | present — mismatch flagged | Винтовка FN SCAR-H | — | string ID says M110, but the resolved name is FN SCAR-H |
| wpn_m16a4 | weapons | 445 | present — mismatch flagged | Автомат M4A1 | — | section ID says M16A4, but the resolved name is M4A1 |
| wpn_m4a1 | weapons | 430 | missing section — string_id alias | Автомат M4A1 | wpn_m16a4 | section ID says M16A4, but the resolved name is M4A1 |
| wpn_mosin | weapons | 448 | present | Винтовка Мосина | — | — |
| wpn_mp133 | weapons | 389 | missing section — string_id alias | Короткое ружьё MP-133 | wpn_wincheaster1300 | section ID says Wincheaster 1300, but the resolved name is MP-133 |
| wpn_mp153 | weapons | 378 | present — mismatch flagged | Ружьё Remington 870 (FD) | — | string ID says Winchester M1, but the resolved name is Remington 870 |
| wpn_mp153_m1 | weapons | 379 | present | Короткое ружьё МР-153 | — | — |
| wpn_mp5 | weapons | 366 | present | Пистолет-пулемёт HK MP5 | — | — |
| wpn_mp5_m1 | weapons | 367 | present | Пистолет-пулемёт HK MP5 9x18 | — | — |
| wpn_mp5_m2 | weapons | 371 | present | Пистолет-пулемёт СР-2МП «Вереск» | — | — |
| wpn_mr308 | weapons | 456 | present | Винтовка HК MR308 HK G28 | — | — |
| wpn_orex | weapons | 388 | missing section — string_id alias | Ружьё Ореха | wpn_hunters_toz_new | section ID says Hunters TOZ New, but the resolved name is Orex shotgun |
| wpn_pb | weapons | 339 | present | Пистолет ПБ | — | — |
| wpn_pkm | weapons | 466 | present | Пулемёт ПКМ | — | — |
| wpn_pkp | weapons | 473 | present — mismatch flagged | Пулемёт ПКМ Тактический | — | string ID says PKP, but the resolved name is tactical PKM |
| wpn_pm | weapons | 338 | present | Пистолет ПМ | — | — |
| wpn_pp19 | weapons | 365 | present | Пистолет-пулемёт "Витязь-СН" | — | — |
| wpn_rg6 | weapons | 474 | present | Гранатомёт РГ-6 «Гном» | — | — |
| wpn_rpg7 | weapons | 468 | present | РПГ-7 | — | — |
| wpn_rpk | weapons | 472 | present | Пулемёт РПК | — | — |
| wpn_rpk74 | weapons | 411 | present | Пулемёт РПК-74Н | — | — |
| wpn_saiga12c | weapons | 382 | present | Карабин Сайга-12К | — | — |
| wpn_saiga12c_m1 | weapons | 383 | present | Карабин Сайга-12М | — | — |
| wpn_saiga12c_m2 | weapons | 461 | present — mismatch flagged | Карабин ВПО-101 «Вепрь Хантер» | — | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» |
| wpn_sig220 | weapons | 346 | present | Пистолет SIG-Sauer P220 | — | — |
| wpn_sig550 | weapons | 416 | present | Автомат SIG SG 552 Commando | — | — |
| wpn_soc308 | weapons | 431 | present | Автомат DS Arms SA-58 | — | — |
| wpn_soc308_m1 | weapons | 432 | present | Автомат DS Arms SA-58 custom | — | — |
| wpn_soc94 | weapons | 428 | missing section — string_id alias | Карабин ВПО-101 «Вепрь Хантер» | wpn_saiga12c_m2 | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» |
| wpn_spas12 | weapons | 380 | present | Ружьё Franchi SPAS-12 | — | — |
| wpn_spas12_m1 | weapons | 381 | present — mismatch flagged | Ружьё Benelli M4 Super 90 (M1014) | — | section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 |
| wpn_spas15 | weapons | 390 | present — mismatch flagged | Ружьё Daewoo USAS-12 | — | section/string ID says SPAS-15, but the resolved name is Daewoo USAS-12 |
| wpn_spas15_m1 | weapons | 391 | present — mismatch flagged | Ружьё Daewoo USAS-12 М1 | — | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 |
| wpn_sv98 | weapons | 451 | present | Винтовка СВ-98М | — | — |
| wpn_svd | weapons | 449 | present | Винтовка СВД | — | — |
| wpn_svu | weapons | 450 | present | Винтовка СВУ-А | — | — |
| wpn_toz34 | weapons | 376 | present | Ружьё ТОЗ-34 | — | — |
| wpn_toz34_m1 | weapons | 395 | present — mismatch flagged | Ружьё Охотника | — | section ID says TOZ-34 M1, but the resolved name is Hunter shotgun |
| wpn_ump45 | weapons | 368 | present | Пистолет-пулемёт HK UMP45 | — | — |
| wpn_usp | weapons | 347 | present | Пистолет HK USP | — | — |
| wpn_val | weapons | 408 | present | Автомат АС «Вал» | — | — |
| wpn_vintorez | weapons | 409 | present | Винтовка ВСС "Винторез" | — | — |
| wpn_vintorez_m1 | weapons | 410 | present | Винтовка "ВССМ" | — | — |
| wpn_walther | weapons | 354, 361 | present | Пистолет Walther P99 | — | — |
| wpn_wincheaster1300 | weapons | 396 | present — mismatch flagged | Короткое ружьё MP-133 | — | section ID says Wincheaster 1300, but the resolved name is MP-133 |
| wpn_winchester_m1 | weapons | 377 | present — mismatch flagged | Ружьё МР-153 | — | section ID says Winchester M1, but the resolved name is MP-153 |


### (a) Merge-table section IDs missing from `spawner.script`

| section_id | base_id | variant_id | addon_type | in-game name | alias already referenced | source file |
| :-- | :-- | :-- | :-- | :-- | :-- | :-- |
| ac10632 | — | ac10632 | scope | Прицел Barska ELECTRO SIGHT AC10632 | none — not referenced | gamedata/config/weapons/add_scopes.ltx:167 |
| g28_scope | — | g28_scope | scope | Прицел HK Schmidt Bender | none — not referenced | gamedata/config/weapons/add_scopes.ltx:274 |
| microt1 | — | microt1 | scope | Прицел Aimpoint Micro T-1 | wpn_addon_mt | gamedata/config/weapons/add_scopes.ltx:189 |
| okp | — | okp | scope | Прицел ОКП-7 | none — not referenced | gamedata/config/weapons/add_scopes.ltx:232 |
| rakurs | — | rakurs | scope | Прицел «Ракурс-А» | none — not referenced | gamedata/config/weapons/add_scopes.ltx:253 |
| specter | — | specter | scope | Прицел ELCAN SpecterDR | none — not referenced | gamedata/config/weapons/add_scopes.ltx:210 |
| wpn_addon_grenade_launcher_ag36 | — | wpn_addon_grenade_launcher_ag36 | underbarrel | Подствольный гранатомет AG36 | none — not referenced | gamedata/config/weapons/add_gl.ltx:48 |
| wpn_addon_grenade_launcher_groza | — | wpn_addon_grenade_launcher_groza | underbarrel | Подствольный гранатомет ГП-5 | none — not referenced | gamedata/config/weapons/add_gl.ltx:68 |
| wpn_addon_sil_45_p | — | wpn_addon_sil_45_p | silencer | Глушитель 11,43 мм | none — not referenced | gamedata/config/weapons/add_silencers.ltx:191 |
| wpn_addon_sil_apb | — | wpn_addon_sil_apb | silencer | Глушитель АПБ | none — not referenced | gamedata/config/weapons/add_silencers.ltx:242 |
| wpn_addon_sil_co_salvo | — | wpn_addon_sil_co_salvo | silencer | Глушитель Salvo 12 | none — not referenced | gamedata/config/weapons/add_silencers.ltx:297 |
| wpn_addon_sil_glock | — | wpn_addon_sil_glock | silencer | Глушитель Glock 18C | none — not referenced | gamedata/config/weapons/add_silencers.ltx:216 |
| wpn_addon_sil_groza | — | wpn_addon_sil_groza | silencer | Глушитель 9х39 мм «Гроза» | none — not referenced | gamedata/config/weapons/add_silencers.ltx:322 |
| wpn_addon_sil_vsk | — | wpn_addon_sil_vsk | silencer | Глушитель 9х39 мм «Вихрь» | none — not referenced | gamedata/config/weapons/add_silencers.ltx:269 |
| wpn_fake_missile | — | wpn_fake_missile | internal projectile | Заряд ВОГ-25 | none — not referenced | gamedata/config/weapons/weapons.ltx:1488 |
| wpn_fake_missile1 | — | wpn_fake_missile1 | internal projectile | Заряд ВГ-40И | none — not referenced | gamedata/config/weapons/weapons.ltx:1589 |
| wpn_fake_missile2 | — | wpn_fake_missile2 | internal projectile | Заряд M406 | none — not referenced | gamedata/config/weapons/weapons.ltx:1705 |
| wpn_rpg7_missile | — | wpn_rpg7_missile | internal projectile | UNRESOLVED | none — not referenced | gamedata/config/weapons/weapons.ltx:1360 |
| wpn_abakan_1p78 | wpn_abakan | wpn_abakan_1p78 | scope | Автомат АН-94 | none — not referenced | gamedata/config/weapons/w_abakan.ltx:360 |
| wpn_abakan_ekp | wpn_abakan | wpn_abakan_ekp | scope | Автомат АН-94 | none — not referenced | gamedata/config/weapons/w_abakan.ltx:299 |
| wpn_abakan_okp | wpn_abakan | wpn_abakan_okp | scope | Автомат АН-94 | none — not referenced | gamedata/config/weapons/w_abakan.ltx:320 |
| wpn_abakan_pso | wpn_abakan | wpn_abakan_pso | scope | Автомат АН-94 | none — not referenced | gamedata/config/weapons/w_abakan.ltx:386 |
| wpn_abakan_rakurs | wpn_abakan | wpn_abakan_rakurs | scope | Автомат АН-94 | none — not referenced | gamedata/config/weapons/w_abakan.ltx:341 |
| wpn_ak74u_snag_1p78 | wpn_aek971 | wpn_ak74u_snag_1p78 | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:391 |
| wpn_ak74u_snag_ac10632 | wpn_aek971 | wpn_ak74u_snag_ac10632 | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:249 |
| wpn_ak74u_snag_ekp | wpn_aek971 | wpn_ak74u_snag_ekp | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:321 |
| wpn_ak74u_snag_eot | wpn_aek971 | wpn_ak74u_snag_eot | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:273 |
| wpn_ak74u_snag_microt1 | wpn_aek971 | wpn_ak74u_snag_microt1 | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:297 |
| wpn_ak74u_snag_okp | wpn_aek971 | wpn_ak74u_snag_okp | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:346 |
| wpn_ak74u_snag_pso | wpn_aek971 | wpn_ak74u_snag_pso | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:425 |
| wpn_ak74u_snag_rakurs | wpn_aek971 | wpn_ak74u_snag_rakurs | scope | Автомат АК-104 «Тактик» | none — not referenced | gamedata/config/weapons/w_ak104.ltx:366 |
| wpn_ak103_1p78 | wpn_ak104 | wpn_ak103_1p78 | scope | Автомат АК-103 | none — not referenced | gamedata/config/weapons/w_ak103.ltx:337 |
| wpn_ak103_ekp | wpn_ak104 | wpn_ak103_ekp | scope | Автомат АК-103 | none — not referenced | gamedata/config/weapons/w_ak103.ltx:269 |
| wpn_ak103_okp | wpn_ak104 | wpn_ak103_okp | scope | Автомат АК-103 | none — not referenced | gamedata/config/weapons/w_ak103.ltx:293 |
| wpn_ak103_pso | wpn_ak104 | wpn_ak103_pso | scope | Автомат АК-103 | none — not referenced | gamedata/config/weapons/w_ak103.ltx:366 |
| wpn_ak103_rakurs | wpn_ak104 | wpn_ak103_rakurs | scope | Автомат АК-103 | none — not referenced | gamedata/config/weapons/w_ak103.ltx:312 |
| wpn_rpk16_ac10632 | wpn_ak108 | wpn_rpk16_ac10632 | scope | Пулемёт РПК-16 | none — not referenced | gamedata/config/weapons/w_rpk16.ltx:162 |
| wpn_rpk16_eot | wpn_ak108 | wpn_rpk16_eot | scope | Пулемёт РПК-16 | none — not referenced | gamedata/config/weapons/w_rpk16.ltx:181 |
| wpn_rpk16_microt1 | wpn_ak108 | wpn_rpk16_microt1 | scope | Пулемёт РПК-16 | none — not referenced | gamedata/config/weapons/w_rpk16.ltx:201 |
| wpn_rpk16_specter | wpn_ak108 | wpn_rpk16_specter | scope | Пулемёт РПК-16 | none — not referenced | gamedata/config/weapons/w_rpk16.ltx:221 |
| wpn_sks_1p78 | wpn_ak47 | wpn_sks_1p78 | scope | Карабин Молот ОП-СКС | none — not referenced | gamedata/config/weapons/w_sks.ltx:347 |
| wpn_sks_ekp | wpn_ak47 | wpn_sks_ekp | scope | Карабин Молот ОП-СКС | none — not referenced | gamedata/config/weapons/w_sks.ltx:259 |
| wpn_sks_okp | wpn_ak47 | wpn_sks_okp | scope | Карабин Молот ОП-СКС | none — not referenced | gamedata/config/weapons/w_sks.ltx:280 |
| wpn_sks_pso | wpn_ak47 | wpn_sks_pso | scope | Карабин Молот ОП-СКС | none — not referenced | gamedata/config/weapons/w_sks.ltx:322 |
| wpn_sks_rakurs | wpn_ak47 | wpn_sks_rakurs | scope | Карабин Молот ОП-СКС | none — not referenced | gamedata/config/weapons/w_sks.ltx:301 |
| wpn_ak74_m1_ac10632 | wpn_ak74_m1 | wpn_ak74_m1_ac10632 | scope | Автомат Стрелка | none — not referenced | gamedata/config/weapons/w_ak_strelok.ltx:79 |
| wpn_ak74_m1_eot | wpn_ak74_m1 | wpn_ak74_m1_eot | scope | Автомат Стрелка | none — not referenced | gamedata/config/weapons/w_ak_strelok.ltx:87 |
| wpn_ak74_m1_microt1 | wpn_ak74_m1 | wpn_ak74_m1_microt1 | scope | Автомат Стрелка | none — not referenced | gamedata/config/weapons/w_ak_strelok.ltx:93 |
| wpn_ak74_m1_specter | wpn_ak74_m1 | wpn_ak74_m1_specter | scope | Автомат Стрелка | none — not referenced | gamedata/config/weapons/w_ak_strelok.ltx:99 |
| wpn_ak12_ac10632 | wpn_ak74_m2 | wpn_ak12_ac10632 | scope | Автомат Стрелка М | none — not referenced | gamedata/config/weapons/w_ak_strelok_rem.ltx:319 |
| wpn_ak12_eot | wpn_ak74_m2 | wpn_ak12_eot | scope | Автомат Стрелка М | none — not referenced | gamedata/config/weapons/w_ak_strelok_rem.ltx:338 |
| wpn_ak12_microt1 | wpn_ak74_m2 | wpn_ak12_microt1 | scope | Автомат Стрелка М | none — not referenced | gamedata/config/weapons/w_ak_strelok_rem.ltx:358 |
| wpn_ak12_specter | wpn_ak74_m2 | wpn_ak12_specter | scope | Автомат Стрелка М | none — not referenced | gamedata/config/weapons/w_ak_strelok_rem.ltx:378 |
| wpn_ak74m_1p78 | wpn_ak74m | wpn_ak74m_1p78 | scope | Автомат АКС-74Н | none — not referenced | gamedata/config/weapons/w_ak74.ltx:380 |
| wpn_ak74m_ekp | wpn_ak74m | wpn_ak74m_ekp | scope | Автомат АКС-74Н | none — not referenced | gamedata/config/weapons/w_ak74.ltx:318 |
| wpn_ak74m_okp | wpn_ak74m | wpn_ak74m_okp | scope | Автомат АКС-74Н | none — not referenced | gamedata/config/weapons/w_ak74.ltx:338 |
| wpn_ak74m_pso | wpn_ak74m | wpn_ak74m_pso | scope | Автомат АКС-74Н | none — not referenced | gamedata/config/weapons/w_ak74.ltx:405 |
| wpn_ak74m_rakurs | wpn_ak74m | wpn_ak74m_rakurs | scope | Автомат АКС-74Н | none — not referenced | gamedata/config/weapons/w_ak74.ltx:359 |
| wpn_ak74u_1p78 | wpn_ak74u | wpn_ak74u_1p78 | scope | Автомат АКС-74УН | none — not referenced | gamedata/config/weapons/w_ak74u.ltx:275 |
| wpn_ak74u_ekp | wpn_ak74u | wpn_ak74u_ekp | scope | Автомат АКС-74УН | none — not referenced | gamedata/config/weapons/w_ak74u.ltx:210 |
| wpn_ak74u_okp | wpn_ak74u | wpn_ak74u_okp | scope | Автомат АКС-74УН | none — not referenced | gamedata/config/weapons/w_ak74u.ltx:233 |
| wpn_ak74u_rakurs | wpn_ak74u | wpn_ak74u_rakurs | scope | Автомат АКС-74УН | none — not referenced | gamedata/config/weapons/w_ak74u.ltx:254 |
| wpn_akm_1p78 | wpn_akm | wpn_akm_1p78 | scope | Автомат АКМН | none — not referenced | gamedata/config/weapons/w_akm.ltx:284 |
| wpn_akm_ekp | wpn_akm | wpn_akm_ekp | scope | Автомат АКМН | none — not referenced | gamedata/config/weapons/w_akm.ltx:222 |
| wpn_akm_okp | wpn_akm | wpn_akm_okp | scope | Автомат АКМН | none — not referenced | gamedata/config/weapons/w_akm.ltx:242 |
| wpn_akm_pso | wpn_akm | wpn_akm_pso | scope | Автомат АКМН | none — not referenced | gamedata/config/weapons/w_akm.ltx:310 |
| wpn_akm_rakurs | wpn_akm | wpn_akm_rakurs | scope | Автомат АКМН | none — not referenced | gamedata/config/weapons/w_akm.ltx:263 |
| wpn_vector_ac10632 | wpn_aks47 | wpn_vector_ac10632 | scope | Пистолет-пулемёт TDI KRISS Vector | none — not referenced | gamedata/config/weapons/w_vector.ltx:158 |
| wpn_vector_eot | wpn_aks47 | wpn_vector_eot | scope | Пистолет-пулемёт TDI KRISS Vector | none — not referenced | gamedata/config/weapons/w_vector.ltx:177 |
| wpn_vector_microt1 | wpn_aks47 | wpn_vector_microt1 | scope | Пистолет-пулемёт TDI KRISS Vector | none — not referenced | gamedata/config/weapons/w_vector.ltx:196 |
| wpn_vector_specter | wpn_aks47 | wpn_vector_specter | scope | Пистолет-пулемёт TDI KRISS Vector | none — not referenced | gamedata/config/weapons/w_vector.ltx:216 |
| wpn_binoc | wpn_binoc | — | base device | UNRESOLVED | none — not referenced | gamedata/config/weapons/w_binoc.ltx:2 |
| wpn_bizon_ekp | wpn_bizon | wpn_bizon_ekp | scope | Пистолет-пулемёт ПП-19 «Бизон» | none — not referenced | gamedata/config/weapons/w_bizon-2.ltx:230 |
| wpn_bizon_okp | wpn_bizon | wpn_bizon_okp | scope | Пистолет-пулемёт ПП-19 «Бизон» | none — not referenced | gamedata/config/weapons/w_bizon-2.ltx:255 |
| wpn_bizon_rakurs | wpn_bizon | wpn_bizon_rakurs | scope | Пистолет-пулемёт ПП-19 «Бизон» | none — not referenced | gamedata/config/weapons/w_bizon-2.ltx:280 |
| wpn_desert_eagle_po | wpn_desert_eagle | wpn_desert_eagle_po | named variant | Пистолет IMI Desert Eagle | none — not referenced | gamedata/config/weapons/w_desert_eagle.ltx:230 |
| wpn_fn2000_nimble_ac10632 | wpn_fn2000 | wpn_fn2000_nimble_ac10632 | scope | Автомат FN F2000 Tactical (FDE) | none — not referenced | gamedata/config/weapons/w_fn2000.ltx:300 |
| wpn_fn2000_nimble_eot | wpn_fn2000 | wpn_fn2000_nimble_eot | scope | Автомат FN F2000 Tactical (FDE) | none — not referenced | gamedata/config/weapons/w_fn2000.ltx:318 |
| wpn_fn2000_nimble_microt1 | wpn_fn2000 | wpn_fn2000_nimble_microt1 | scope | Автомат FN F2000 Tactical (FDE) | none — not referenced | gamedata/config/weapons/w_fn2000.ltx:336 |
| wpn_fn2000_nimble_specter | wpn_fn2000 | wpn_fn2000_nimble_specter | scope | Автомат FN F2000 Tactical (FDE) | none — not referenced | gamedata/config/weapons/w_fn2000.ltx:354 |
| wpn_g36_nimble_ac10632 | wpn_g36c | wpn_g36_nimble_ac10632 | scope | Пулемёт HK MG36 | none — not referenced | gamedata/config/weapons/w_mg36.ltx:162 |
| wpn_g36_nimble_eot | wpn_g36c | wpn_g36_nimble_eot | scope | Пулемёт HK MG36 | none — not referenced | gamedata/config/weapons/w_mg36.ltx:179 |
| wpn_g36_nimble_microt1 | wpn_g36c | wpn_g36_nimble_microt1 | scope | Пулемёт HK MG36 | none — not referenced | gamedata/config/weapons/w_mg36.ltx:197 |
| wpn_g36_nimble_specter | wpn_g36c | wpn_g36_nimble_specter | scope | Пулемёт HK MG36 | none — not referenced | gamedata/config/weapons/w_mg36.ltx:215 |
| wpn_gauss_auto | wpn_gauss_auto | — | base weapon | Гаусс-винтовка | wpn_gauss_auto_model | gamedata/config/weapons/w_gauss_rifle.ltx:219 |
| wpn_mp9_ac10632 | wpn_glock_m2 | wpn_mp9_ac10632 | scope | Пистолет-пулемёт BT MP9 | none — not referenced | gamedata/config/weapons/w_glock17.ltx:424 |
| wpn_mp9_eot | wpn_glock_m2 | wpn_mp9_eot | scope | Пистолет-пулемёт BT MP9 | none — not referenced | gamedata/config/weapons/w_glock17.ltx:444 |
| wpn_mp9_microt1 | wpn_glock_m2 | wpn_mp9_microt1 | scope | Пистолет-пулемёт BT MP9 | none — not referenced | gamedata/config/weapons/w_glock17.ltx:463 |
| wpn_groza_nimble | wpn_groza_nimble | — | base weapon | Автомат ОЦ-14-4А «Шторм» | st_wpn_groza_nimble | gamedata/config/weapons/w_groza.ltx:306 |
| wpn_groza_nimble_ac10632 | wpn_groza_nimble | wpn_groza_nimble_ac10632 | scope | Автомат ОЦ-14-4А «Шторм» | none — not referenced | gamedata/config/weapons/w_groza.ltx:344 |
| wpn_groza_nimble_eot | wpn_groza_nimble | wpn_groza_nimble_eot | scope | Автомат ОЦ-14-4А «Шторм» | none — not referenced | gamedata/config/weapons/w_groza.ltx:364 |
| wpn_groza_nimble_microt1 | wpn_groza_nimble | wpn_groza_nimble_microt1 | scope | Автомат ОЦ-14-4А «Шторм» | none — not referenced | gamedata/config/weapons/w_groza.ltx:383 |
| wpn_groza_nimble_specter | wpn_groza_nimble | wpn_groza_nimble_specter | scope | Автомат ОЦ-14-4А «Шторм» | none — not referenced | gamedata/config/weapons/w_groza.ltx:402 |
| wpn_scarh_ac10632 | wpn_m110 | wpn_scarh_ac10632 | scope | Винтовка FN SCAR-H | none — not referenced | gamedata/config/weapons/w_fn_scar.ltx:351 |
| wpn_scarh_eot | wpn_m110 | wpn_scarh_eot | scope | Винтовка FN SCAR-H | none — not referenced | gamedata/config/weapons/w_fn_scar.ltx:369 |
| wpn_scarh_microt1 | wpn_m110 | wpn_scarh_microt1 | scope | Винтовка FN SCAR-H | none — not referenced | gamedata/config/weapons/w_fn_scar.ltx:386 |
| wpn_scarh_specter | wpn_m110 | wpn_scarh_specter | scope | Винтовка FN SCAR-H | none — not referenced | gamedata/config/weapons/w_fn_scar.ltx:403 |
| wpn_lr300_ac10632 | wpn_m16a4 | wpn_lr300_ac10632 | scope | Автомат M4A1 | none — not referenced | gamedata/config/weapons/w_m4a1.ltx:364 |
| wpn_lr300_eot | wpn_m16a4 | wpn_lr300_eot | scope | Автомат M4A1 | none — not referenced | gamedata/config/weapons/w_m4a1.ltx:382 |
| wpn_lr300_microt1 | wpn_m16a4 | wpn_lr300_microt1 | scope | Автомат M4A1 | none — not referenced | gamedata/config/weapons/w_m4a1.ltx:401 |
| wpn_lr300_specter | wpn_m16a4 | wpn_lr300_specter | scope | Автомат M4A1 | none — not referenced | gamedata/config/weapons/w_m4a1.ltx:420 |
| wpn_mosin_pu | wpn_mosin | wpn_mosin_pu | named variant | Винтовка Мосина | none — not referenced | gamedata/config/weapons/w_mosin.ltx:237 |
| wpn_wincheaster1300_trapper_ac10632 | wpn_mp153 | wpn_wincheaster1300_trapper_ac10632 | scope | Ружьё Remington 870 (FD) | none — not referenced | gamedata/config/weapons/w_remington870.ltx:210 |
| wpn_wincheaster1300_trapper_eot | wpn_mp153 | wpn_wincheaster1300_trapper_eot | scope | Ружьё Remington 870 (FD) | none — not referenced | gamedata/config/weapons/w_remington870.ltx:228 |
| wpn_wincheaster1300_trapper_microt1 | wpn_mp153 | wpn_wincheaster1300_trapper_microt1 | scope | Ружьё Remington 870 (FD) | none — not referenced | gamedata/config/weapons/w_remington870.ltx:246 |
| wpn_mp5_m1_ac10632 | wpn_mp5_m1 | wpn_mp5_m1_ac10632 | scope | Пистолет-пулемёт HK MP5 9x18 | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:45 |
| wpn_mp5_m1_eot | wpn_mp5_m1 | wpn_mp5_m1_eot | scope | Пистолет-пулемёт HK MP5 9x18 | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:52 |
| wpn_mp5_m1_microt1 | wpn_mp5_m1 | wpn_mp5_m1_microt1 | scope | Пистолет-пулемёт HK MP5 9x18 | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:58 |
| wpn_mp5_m1_specter | wpn_mp5_m1 | wpn_mp5_m1_specter | scope | Пистолет-пулемёт HK MP5 9x18 | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:64 |
| wpn_sr2m_ac10632 | wpn_mp5_m2 | wpn_sr2m_ac10632 | scope | Пистолет-пулемёт СР-2МП «Вереск» | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:203 |
| wpn_sr2m_eot | wpn_mp5_m2 | wpn_sr2m_eot | scope | Пистолет-пулемёт СР-2МП «Вереск» | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:223 |
| wpn_sr2m_microt1 | wpn_mp5_m2 | wpn_sr2m_microt1 | scope | Пистолет-пулемёт СР-2МП «Вереск» | none — not referenced | gamedata/config/weapons/w_mp5_mod.ltx:243 |
| wpn_mp5_ac10632 | wpn_mp5 | wpn_mp5_ac10632 | scope | Пистолет-пулемёт HK MP5 | none — not referenced | gamedata/config/weapons/w_mp5.ltx:214 |
| wpn_mp5_eot | wpn_mp5 | wpn_mp5_eot | scope | Пистолет-пулемёт HK MP5 | none — not referenced | gamedata/config/weapons/w_mp5.ltx:234 |
| wpn_mp5_microt1 | wpn_mp5 | wpn_mp5_microt1 | scope | Пистолет-пулемёт HK MP5 | none — not referenced | gamedata/config/weapons/w_mp5.ltx:254 |
| wpn_mp5_specter | wpn_mp5 | wpn_mp5_specter | scope | Пистолет-пулемёт HK MP5 | none — not referenced | gamedata/config/weapons/w_mp5.ltx:273 |
| wpn_mr308_g28_scope | wpn_mr308 | wpn_mr308_g28_scope | scope | Винтовка HК MR308 HK G28 | none — not referenced | gamedata/config/weapons/w_fn_scar.ltx:572 |
| wpn_pkm_zulus_ac10632 | wpn_pkp | wpn_pkm_zulus_ac10632 | scope | Пулемёт ПКМ Тактический | none — not referenced | gamedata/config/weapons/w_pkm_mod.ltx:147 |
| wpn_pkm_zulus_eot | wpn_pkp | wpn_pkm_zulus_eot | scope | Пулемёт ПКМ Тактический | none — not referenced | gamedata/config/weapons/w_pkm_mod.ltx:165 |
| wpn_pkm_zulus_microt1 | wpn_pkp | wpn_pkm_zulus_microt1 | scope | Пулемёт ПКМ Тактический | none — not referenced | gamedata/config/weapons/w_pkm_mod.ltx:185 |
| wpn_vityaz_ac10632 | wpn_pp19 | wpn_vityaz_ac10632 | scope | Пистолет-пулемёт "Витязь-СН" | none — not referenced | gamedata/config/weapons/w_vityaz.ltx:229 |
| wpn_vityaz_eot | wpn_pp19 | wpn_vityaz_eot | scope | Пистолет-пулемёт "Витязь-СН" | none — not referenced | gamedata/config/weapons/w_vityaz.ltx:253 |
| wpn_vityaz_microt1 | wpn_pp19 | wpn_vityaz_microt1 | scope | Пистолет-пулемёт "Витязь-СН" | none — not referenced | gamedata/config/weapons/w_vityaz.ltx:281 |
| wpn_vityaz_specter | wpn_pp19 | wpn_vityaz_specter | scope | Пистолет-пулемёт "Витязь-СН" | none — not referenced | gamedata/config/weapons/w_vityaz.ltx:305 |
| wpn_rpk74m_1p78 | wpn_rpk | wpn_rpk74m_1p78 | scope | Пулемёт РПК | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:488 |
| wpn_rpk74m_ekp | wpn_rpk | wpn_rpk74m_ekp | scope | Пулемёт РПК | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:426 |
| wpn_rpk74m_okp | wpn_rpk | wpn_rpk74m_okp | scope | Пулемёт РПК | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:447 |
| wpn_rpk74m_pso | wpn_rpk | wpn_rpk74m_pso | scope | Пулемёт РПК | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:513 |
| wpn_rpk74m_rakurs | wpn_rpk | wpn_rpk74m_rakurs | scope | Пулемёт РПК | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:467 |
| wpn_rpk74_1p78 | wpn_rpk74 | wpn_rpk74_1p78 | scope | Пулемёт РПК-74Н | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:258 |
| wpn_rpk74_ekp | wpn_rpk74 | wpn_rpk74_ekp | scope | Пулемёт РПК-74Н | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:198 |
| wpn_rpk74_okp | wpn_rpk74 | wpn_rpk74_okp | scope | Пулемёт РПК-74Н | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:216 |
| wpn_rpk74_pso | wpn_rpk74 | wpn_rpk74_pso | scope | Пулемёт РПК-74Н | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:283 |
| wpn_rpk74_rakurs | wpn_rpk74 | wpn_rpk74_rakurs | scope | Пулемёт РПК-74Н | none — not referenced | gamedata/config/weapons/w_rpk74.ltx:236 |
| wpn_saiga12m_ac10632 | wpn_saiga12c_m1 | wpn_saiga12m_ac10632 | scope | Карабин Сайга-12М | none — not referenced | gamedata/config/weapons/w_saiga12_mod.ltx:154 |
| wpn_saiga12m_eot | wpn_saiga12c_m1 | wpn_saiga12m_eot | scope | Карабин Сайга-12М | none — not referenced | gamedata/config/weapons/w_saiga12_mod.ltx:172 |
| wpn_saiga12m_microt1 | wpn_saiga12c_m1 | wpn_saiga12m_microt1 | scope | Карабин Сайга-12М | none — not referenced | gamedata/config/weapons/w_saiga12_mod.ltx:190 |
| wpn_saiga12m_specter | wpn_saiga12c_m1 | wpn_saiga12m_specter | scope | Карабин Сайга-12М | none — not referenced | gamedata/config/weapons/w_saiga12_mod.ltx:210 |
| wpn_vpo101_1p78 | wpn_saiga12c_m2 | wpn_vpo101_1p78 | scope | Карабин ВПО-101 «Вепрь Хантер» | none — not referenced | gamedata/config/weapons/w_vpo101_hunter.ltx:281 |
| wpn_vpo101_ekp | wpn_saiga12c_m2 | wpn_vpo101_ekp | scope | Карабин ВПО-101 «Вепрь Хантер» | none — not referenced | gamedata/config/weapons/w_vpo101_hunter.ltx:212 |
| wpn_vpo101_okp | wpn_saiga12c_m2 | wpn_vpo101_okp | scope | Карабин ВПО-101 «Вепрь Хантер» | none — not referenced | gamedata/config/weapons/w_vpo101_hunter.ltx:235 |
| wpn_vpo101_pso | wpn_saiga12c_m2 | wpn_vpo101_pso | scope | Карабин ВПО-101 «Вепрь Хантер» | none — not referenced | gamedata/config/weapons/w_vpo101_hunter.ltx:185 |
| wpn_vpo101_rakurs | wpn_saiga12c_m2 | wpn_vpo101_rakurs | scope | Карабин ВПО-101 «Вепрь Хантер» | none — not referenced | gamedata/config/weapons/w_vpo101_hunter.ltx:258 |
| wpn_saiga12_ekp | wpn_saiga12c | wpn_saiga12_ekp | scope | Карабин Сайга-12К | none — not referenced | gamedata/config/weapons/w_saiga12c.ltx:200 |
| wpn_saiga12_okp | wpn_saiga12c | wpn_saiga12_okp | scope | Карабин Сайга-12К | none — not referenced | gamedata/config/weapons/w_saiga12c.ltx:227 |
| wpn_saiga12_rakurs | wpn_saiga12c | wpn_saiga12_rakurs | scope | Карабин Сайга-12К | none — not referenced | gamedata/config/weapons/w_saiga12c.ltx:248 |
| wpn_sig550_ac10632 | wpn_sig550 | wpn_sig550_ac10632 | scope | Автомат SIG SG 552 Commando | none — not referenced | gamedata/config/weapons/w_sig552.ltx:249 |
| wpn_sig550_eot | wpn_sig550 | wpn_sig550_eot | scope | Автомат SIG SG 552 Commando | none — not referenced | gamedata/config/weapons/w_sig552.ltx:266 |
| wpn_sig550_microt1 | wpn_sig550 | wpn_sig550_microt1 | scope | Автомат SIG SG 552 Commando | none — not referenced | gamedata/config/weapons/w_sig552.ltx:285 |
| wpn_sig550_specter | wpn_sig550 | wpn_sig550_specter | scope | Автомат SIG SG 552 Commando | none — not referenced | gamedata/config/weapons/w_sig552.ltx:304 |
| wpn_soc308_m1_ac10632 | wpn_soc308_m1 | wpn_soc308_m1_ac10632 | scope | Автомат DS Arms SA-58 custom | none — not referenced | gamedata/config/weapons/w_sa58.ltx:252 |
| wpn_soc308_m1_eot | wpn_soc308_m1 | wpn_soc308_m1_eot | scope | Автомат DS Arms SA-58 custom | none — not referenced | gamedata/config/weapons/w_sa58.ltx:269 |
| wpn_soc308_m1_microt1 | wpn_soc308_m1 | wpn_soc308_m1_microt1 | scope | Автомат DS Arms SA-58 custom | none — not referenced | gamedata/config/weapons/w_sa58.ltx:286 |
| wpn_soc308_m1_specter | wpn_soc308_m1 | wpn_soc308_m1_specter | scope | Автомат DS Arms SA-58 custom | none — not referenced | gamedata/config/weapons/w_sa58.ltx:303 |
| wpn_spas12_nimble_ac10632 | wpn_spas12_m1 | wpn_spas12_nimble_ac10632 | scope | Ружьё Benelli M4 Super 90 (M1014) | none — not referenced | gamedata/config/weapons/w_spas12.ltx:343 |
| wpn_spas12_nimble_eot | wpn_spas12_m1 | wpn_spas12_nimble_eot | scope | Ружьё Benelli M4 Super 90 (M1014) | none — not referenced | gamedata/config/weapons/w_spas12.ltx:362 |
| wpn_spas12_nimble_microt1 | wpn_spas12_m1 | wpn_spas12_nimble_microt1 | scope | Ружьё Benelli M4 Super 90 (M1014) | none — not referenced | gamedata/config/weapons/w_spas12.ltx:381 |
| wpn_usas12_m1_ac10632 | wpn_spas15_m1 | wpn_usas12_m1_ac10632 | scope | Ружьё Daewoo USAS-12 М1 | none — not referenced | gamedata/config/weapons/w_usas12_mod.ltx:217 |
| wpn_usas12_m1_eot | wpn_spas15_m1 | wpn_usas12_m1_eot | scope | Ружьё Daewoo USAS-12 М1 | none — not referenced | gamedata/config/weapons/w_usas12_mod.ltx:236 |
| wpn_usas12_m1_microt1 | wpn_spas15_m1 | wpn_usas12_m1_microt1 | scope | Ружьё Daewoo USAS-12 М1 | none — not referenced | gamedata/config/weapons/w_usas12_mod.ltx:255 |
| wpn_usas12_m1_specter | wpn_spas15_m1 | wpn_usas12_m1_specter | scope | Ружьё Daewoo USAS-12 М1 | none — not referenced | gamedata/config/weapons/w_usas12_mod.ltx:275 |
| wpn_svd_1p78 | wpn_svd | wpn_svd_1p78 | scope | Винтовка СВД | none — not referenced | gamedata/config/weapons/w_svd.ltx:302 |
| wpn_svd_ekp | wpn_svd | wpn_svd_ekp | scope | Винтовка СВД | none — not referenced | gamedata/config/weapons/w_svd.ltx:232 |
| wpn_svd_okp | wpn_svd | wpn_svd_okp | scope | Винтовка СВД | none — not referenced | gamedata/config/weapons/w_svd.ltx:256 |
| wpn_svd_pso | wpn_svd | wpn_svd_pso | scope | Винтовка СВД | none — not referenced | gamedata/config/weapons/w_svd.ltx:329 |
| wpn_svd_rakurs | wpn_svd | wpn_svd_rakurs | scope | Винтовка СВД | none — not referenced | gamedata/config/weapons/w_svd.ltx:279 |
| wpn_svu_1p78 | wpn_svu | wpn_svu_1p78 | scope | Винтовка СВУ-А | none — not referenced | gamedata/config/weapons/w_svu.ltx:284 |
| wpn_svu_ekp | wpn_svu | wpn_svu_ekp | scope | Винтовка СВУ-А | none — not referenced | gamedata/config/weapons/w_svu.ltx:227 |
| wpn_svu_okp | wpn_svu | wpn_svu_okp | scope | Винтовка СВУ-А | none — not referenced | gamedata/config/weapons/w_svu.ltx:246 |
| wpn_svu_pso | wpn_svu | wpn_svu_pso | scope | Винтовка СВУ-А | none — not referenced | gamedata/config/weapons/w_svu.ltx:308 |
| wpn_svu_rakurs | wpn_svu | wpn_svu_rakurs | scope | Винтовка СВУ-А | none — not referenced | gamedata/config/weapons/w_svu.ltx:265 |
| wpn_mp5_nimble_ac10632 | wpn_ump45 | wpn_mp5_nimble_ac10632 | scope | Пистолет-пулемёт HK UMP45 | none — not referenced | gamedata/config/weapons/w_ump45.ltx:226 |
| wpn_mp5_nimble_eot | wpn_ump45 | wpn_mp5_nimble_eot | scope | Пистолет-пулемёт HK UMP45 | none — not referenced | gamedata/config/weapons/w_ump45.ltx:245 |
| wpn_mp5_nimble_microt1 | wpn_ump45 | wpn_mp5_nimble_microt1 | scope | Пистолет-пулемёт HK UMP45 | none — not referenced | gamedata/config/weapons/w_ump45.ltx:264 |
| wpn_mp5_nimble_specter | wpn_ump45 | wpn_mp5_nimble_specter | scope | Пистолет-пулемёт HK UMP45 | none — not referenced | gamedata/config/weapons/w_ump45.ltx:284 |
| wpn_val_nimble | wpn_val_nimble | — | base weapon | Автомат СР-3М «Вихрь» | st_wpn_val_nimble | gamedata/config/weapons/w_val.ltx:331 |
| wpn_val_nimble_1p78 | wpn_val_nimble | wpn_val_nimble_1p78 | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:498 |
| wpn_val_nimble_ac10632 | wpn_val_nimble | wpn_val_nimble_ac10632 | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:551 |
| wpn_val_nimble_ekp | wpn_val_nimble | wpn_val_nimble_ekp | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:421 |
| wpn_val_nimble_eot | wpn_val_nimble | wpn_val_nimble_eot | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:576 |
| wpn_val_nimble_microt1 | wpn_val_nimble | wpn_val_nimble_microt1 | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:600 |
| wpn_val_nimble_okp | wpn_val_nimble | wpn_val_nimble_okp | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:448 |
| wpn_val_nimble_pso | wpn_val_nimble | wpn_val_nimble_pso | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:525 |
| wpn_val_nimble_rakurs | wpn_val_nimble | wpn_val_nimble_rakurs | scope | Автомат СР-3М «Вихрь» | none — not referenced | gamedata/config/weapons/w_val.ltx:473 |
| wpn_val_1p78 | wpn_val | wpn_val_1p78 | scope | Автомат АС «Вал» | none — not referenced | gamedata/config/weapons/w_val.ltx:280 |
| wpn_val_ekp | wpn_val | wpn_val_ekp | scope | Автомат АС «Вал» | none — not referenced | gamedata/config/weapons/w_val.ltx:205 |
| wpn_val_okp | wpn_val | wpn_val_okp | scope | Автомат АС «Вал» | none — not referenced | gamedata/config/weapons/w_val.ltx:230 |
| wpn_val_pso | wpn_val | wpn_val_pso | scope | Автомат АС «Вал» | none — not referenced | gamedata/config/weapons/w_val.ltx:306 |
| wpn_val_rakurs | wpn_val | wpn_val_rakurs | scope | Автомат АС «Вал» | none — not referenced | gamedata/config/weapons/w_val.ltx:255 |
| wpn_vintorez_nimble_ac10632 | wpn_vintorez_m1 | wpn_vintorez_nimble_ac10632 | scope | Винтовка "ВССМ" | none — not referenced | gamedata/config/weapons/w_vintorez_mod.ltx:100 |
| wpn_vintorez_nimble_eot | wpn_vintorez_m1 | wpn_vintorez_nimble_eot | scope | Винтовка "ВССМ" | none — not referenced | gamedata/config/weapons/w_vintorez_mod.ltx:123 |
| wpn_vintorez_nimble_microt1 | wpn_vintorez_m1 | wpn_vintorez_nimble_microt1 | scope | Винтовка "ВССМ" | none — not referenced | gamedata/config/weapons/w_vintorez_mod.ltx:147 |
| wpn_vintorez_nimble_specter | wpn_vintorez_m1 | wpn_vintorez_nimble_specter | scope | Винтовка "ВССМ" | none — not referenced | gamedata/config/weapons/w_vintorez_mod.ltx:171 |
| wpn_vintorez_1p78 | wpn_vintorez | wpn_vintorez_1p78 | scope | Винтовка ВСС "Винторез" | none — not referenced | gamedata/config/weapons/w_vintorez.ltx:292 |
| wpn_vintorez_ekp | wpn_vintorez | wpn_vintorez_ekp | scope | Винтовка ВСС "Винторез" | none — not referenced | gamedata/config/weapons/w_vintorez.ltx:216 |
| wpn_vintorez_okp | wpn_vintorez | wpn_vintorez_okp | scope | Винтовка ВСС "Винторез" | none — not referenced | gamedata/config/weapons/w_vintorez.ltx:241 |
| wpn_vintorez_pso | wpn_vintorez | wpn_vintorez_pso | scope | Винтовка ВСС "Винторез" | none — not referenced | gamedata/config/weapons/w_vintorez.ltx:318 |
| wpn_vintorez_rakurs | wpn_vintorez | wpn_vintorez_rakurs | scope | Винтовка ВСС "Винторез" | none — not referenced | gamedata/config/weapons/w_vintorez.ltx:267 |


### (b) Current spawner IDs flagged by name/ID mismatches

| spawner id | line(s) | resolved in-game name | mismatch note |
| :-- | :-- | :-- | :-- |
| wpn_aek971 | 422 | Автомат АК-104 «Тактик» | section ID says AEK-971, but the resolved name is AK-104 «Тактик» |
| wpn_ak104 | 442 | Автомат АК-103 | section ID says AK-104, but the resolved name is AK-103 |
| wpn_ak108 | 443 | Пулемёт РПК-16 | section/string ID says AK-108/AK-101, but the resolved name is RPK-16 |
| wpn_ak47 | 400 | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS |
| wpn_ak47_old | 436 | Карабин Молот ОП-СКС | section ID says AK-47, but the resolved name is Molot OP-SKS |
| wpn_ak74_m3 | 353 | Пистолет Colt M45A1 | section ID says AK-74 M3, but the resolved name is Colt M45A1 |
| wpn_aks47 | 402 | Пистолет-пулемёт TDI KRISS Vector | section/string ID says AKS47, but the resolved name is TDI KRISS Vector |
| wpn_aps | 342 | Пистолет АПБ | section ID says APS, but the resolved name is APB |
| wpn_beretta93 | 344 | Пистолет FN FNX-45 | section ID says Beretta 93, but the resolved name is FN FNX-45 |
| wpn_eagle_m1 | 349 | Револьвер Taurus Raging Bull | section/string ID says Eagle M1, but the resolved name is Taurus Raging Bull |
| wpn_fort_m1 | 341 | Пистолет ОЦ-33 «Пернач» | section/string ID says Fort M1, but the resolved name is OTs-33 Pernach |
| wpn_g36c | 415 | Пулемёт HK MG36 | section ID says G36C, but the resolved name is HK MG36 |
| wpn_glock_m1 | 356 | Пистолет Glock 18C | string ID says Glock 17 M1, but the resolved name is Glock 18C |
| wpn_glock_m2 | 357 | Пистолет-пулемёт BT MP9 | string ID says Glock 18, but the resolved name is BT MP9 |
| wpn_hunters_toz_new | 394 | Ружьё Ореха | section ID says Hunters TOZ New, but the resolved name is Orex shotgun |
| wpn_lr300 | 459 | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 |
| wpn_lr300_m1 | 460 | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 |
| wpn_m110 | 453 | Винтовка FN SCAR-H | string ID says M110, but the resolved name is FN SCAR-H |
| wpn_m16a4 | 445 | Автомат M4A1 | section ID says M16A4, but the resolved name is M4A1 |
| wpn_mp153 | 378 | Ружьё Remington 870 (FD) | string ID says Winchester M1, but the resolved name is Remington 870 |
| wpn_pkp | 473 | Пулемёт ПКМ Тактический | string ID says PKP, but the resolved name is tactical PKM |
| wpn_saiga12c_m2 | 461 | Карабин ВПО-101 «Вепрь Хантер» | section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» |
| wpn_spas12_m1 | 381 | Ружьё Benelli M4 Super 90 (M1014) | section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 |
| wpn_spas15 | 390 | Ружьё Daewoo USAS-12 | section/string ID says SPAS-15, but the resolved name is Daewoo USAS-12 |
| wpn_spas15_m1 | 391 | Ружьё Daewoo USAS-12 М1 | section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 |
| wpn_toz34_m1 | 395 | Ружьё Охотника | section ID says TOZ-34 M1, but the resolved name is Hunter shotgun |
| wpn_wincheaster1300 | 396 | Короткое ружьё MP-133 | section ID says Wincheaster 1300, but the resolved name is MP-133 |
| wpn_winchester_m1 | 377 | Ружьё МР-153 | section ID says Winchester M1, but the resolved name is MP-153 |


### (c) Current spawner IDs with no exact section header

| spawner id | line(s) | status | string-table name, if any | proposed section replacement |
| :-- | :-- | :-- | :-- | :-- |
| st_wpn_b725 | 386 | missing section — no match | Ружьё Browning B725 | — |
| st_wpn_groza_nimble | 426 | missing section — string_id alias | Автомат ОЦ-14-4А «Шторм» | wpn_groza_nimble |
| st_wpn_val_nimble | 427 | missing section — string_id alias | Автомат СР-3М «Вихрь» | wpn_val_nimble |
| wpn_addon_acog | 901 | missing section — no match | — | — |
| wpn_addon_mt | 900 | missing section — non-wpn/legacy alias | Прицел Aimpoint Micro T-1 | microt1 |
| wpn_ak_strelok | 423 | missing section — string_id alias | Автомат Стрелка | wpn_ak74_m1 |
| wpn_ak_strelok1 | 424 | missing section — string_id alias | Автомат Стрелка М | wpn_ak74_m2 |
| wpn_ak_strelok2 | 425 | missing section — no match | Автомат тюнингованный Шурупом | — |
| wpn_ak101 | 420 | missing section — string_id alias | Пулемёт РПК-16 | wpn_ak108 |
| wpn_ak103 | 421 | missing section — string_id alias | Автомат АК-103 | wpn_ak104 |
| wpn_gauss_auto_model | 479 | missing section — non-wpn/legacy alias | Гаусс-винтовка | wpn_gauss_auto |
| wpn_gauss_model | 478 | missing section — non-wpn/legacy alias | Гаусс-винтовка | wpn_gauss |
| wpn_hunters | 387 | missing section — string_id alias | Ружьё Охотника | wpn_toz34_m1 |
| wpn_m4a1 | 430 | missing section — string_id alias | Автомат M4A1 | wpn_m16a4 |
| wpn_mp133 | 389 | missing section — string_id alias | Короткое ружьё MP-133 | wpn_wincheaster1300 |
| wpn_orex | 388 | missing section — string_id alias | Ружьё Ореха | wpn_hunters_toz_new |
| wpn_soc94 | 428 | missing section — string_id alias | Карабин ВПО-101 «Вепрь Хантер» | wpn_saiga12c_m2 |


### Proposed `spawner.script` edits — review only; not applied

| old id | line(s) | proposed edit | reason |
| :-- | :-- | :-- | :-- |
| st_wpn_b725 | 386 | remove st_wpn_b725 | string ID `st_wpn_b725` exists, but no corresponding weapon section exists; add a config section first if this item is required |
| st_wpn_groza_nimble | 426 | st_wpn_groza_nimble → wpn_groza_nimble | replace stale/alias ID with section ID; resolves to Автомат ОЦ-14-4А «Шторм» |
| st_wpn_val_nimble | 427 | st_wpn_val_nimble → wpn_val_nimble | replace stale/alias ID with section ID; resolves to Автомат СР-3М «Вихрь» |
| wpn_addon_acog | 901 | remove wpn_addon_acog | no matching section or weapon string ID exists |
| wpn_addon_mt | 900 | wpn_addon_mt → microt1 | replace stale/alias ID with section ID; resolves to Прицел Aimpoint Micro T-1 |
| wpn_aek971 | 422 | keep wpn_aek971 (no safe replacement) | valid section ID; section ID says AEK-971, but the resolved name is AK-104 «Тактик» |
| wpn_ak_strelok | 423 | wpn_ak_strelok → wpn_ak74_m1 | replace stale/alias ID with section ID; resolves to Автомат Стрелка |
| wpn_ak_strelok1 | 424 | wpn_ak_strelok1 → wpn_ak74_m2 | replace stale/alias ID with section ID; resolves to Автомат Стрелка М |
| wpn_ak_strelok2 | 425 | remove wpn_ak_strelok2 | string ID `wpn-ak_strelok2` exists, but no corresponding weapon section exists; add a config section first if this item is required |
| wpn_ak101 | 420 | wpn_ak101 → wpn_ak108 | replace stale/alias ID with section ID; resolves to Пулемёт РПК-16 |
| wpn_ak103 | 421 | wpn_ak103 → wpn_ak104 | replace stale/alias ID with section ID; resolves to Автомат АК-103 |
| wpn_ak104 | 442 | keep wpn_ak104 (no safe replacement) | valid section ID; section ID says AK-104, but the resolved name is AK-103 |
| wpn_ak108 | 443 | keep wpn_ak108 (no safe replacement) | valid section ID; section/string ID says AK-108/AK-101, but the resolved name is RPK-16 |
| wpn_ak47 | 400 | keep wpn_ak47 (no safe replacement) | valid section ID; section ID says AK-47, but the resolved name is Molot OP-SKS |
| wpn_ak47_old | 436 | keep wpn_ak47_old (no safe replacement) | valid section ID; section ID says AK-47, but the resolved name is Molot OP-SKS |
| wpn_ak74_m3 | 353 | keep wpn_ak74_m3 (no safe replacement) | valid section ID; section ID says AK-74 M3, but the resolved name is Colt M45A1 |
| wpn_aks47 | 402 | keep wpn_aks47 (no safe replacement) | valid section ID; section/string ID says AKS47, but the resolved name is TDI KRISS Vector |
| wpn_aps | 342 | keep wpn_aps (no safe replacement) | valid section ID; section ID says APS, but the resolved name is APB |
| wpn_beretta93 | 344 | keep wpn_beretta93 (no safe replacement) | valid section ID; section ID says Beretta 93, but the resolved name is FN FNX-45 |
| wpn_eagle_m1 | 349 | keep wpn_eagle_m1 (no safe replacement) | valid section ID; section/string ID says Eagle M1, but the resolved name is Taurus Raging Bull |
| wpn_fort_m1 | 341 | keep wpn_fort_m1 (no safe replacement) | valid section ID; section/string ID says Fort M1, but the resolved name is OTs-33 Pernach |
| wpn_g36c | 415 | keep wpn_g36c (no safe replacement) | valid section ID; section ID says G36C, but the resolved name is HK MG36 |
| wpn_gauss_auto_model | 479 | wpn_gauss_auto_model → wpn_gauss_auto | replace stale/alias ID with section ID; resolves to Гаусс-винтовка |
| wpn_gauss_model | 478 | wpn_gauss_model → wpn_gauss | replace stale/alias ID with section ID; resolves to Гаусс-винтовка |
| wpn_glock_m1 | 356 | keep wpn_glock_m1 (no safe replacement) | valid section ID; string ID says Glock 17 M1, but the resolved name is Glock 18C |
| wpn_glock_m2 | 357 | keep wpn_glock_m2 (no safe replacement) | valid section ID; string ID says Glock 18, but the resolved name is BT MP9 |
| wpn_hunters | 387 | wpn_hunters → wpn_toz34_m1 | replace stale/alias ID with section ID; resolves to Ружьё Охотника |
| wpn_hunters_toz_new | 394 | keep wpn_hunters_toz_new (no safe replacement) | valid section ID; section ID says Hunters TOZ New, but the resolved name is Orex shotgun |
| wpn_lr300 | 459 | keep wpn_lr300 (no safe replacement) | valid section ID; section ID says M16A4, but the resolved name is M4A1 |
| wpn_lr300_m1 | 460 | keep wpn_lr300_m1 (no safe replacement) | valid section ID; section ID says M16A4, but the resolved name is M4A1 |
| wpn_m110 | 453 | keep wpn_m110 (no safe replacement) | valid section ID; string ID says M110, but the resolved name is FN SCAR-H |
| wpn_m16a4 | 445 | keep wpn_m16a4 (no safe replacement) | valid section ID; section ID says M16A4, but the resolved name is M4A1 |
| wpn_m4a1 | 430 | wpn_m4a1 → wpn_m16a4 | replace stale/alias ID with section ID; resolves to Автомат M4A1 |
| wpn_mp133 | 389 | wpn_mp133 → wpn_wincheaster1300 | replace stale/alias ID with section ID; resolves to Короткое ружьё MP-133 |
| wpn_mp153 | 378 | keep wpn_mp153 (no safe replacement) | valid section ID; string ID says Winchester M1, but the resolved name is Remington 870 |
| wpn_orex | 388 | wpn_orex → wpn_hunters_toz_new | replace stale/alias ID with section ID; resolves to Ружьё Ореха |
| wpn_pkp | 473 | keep wpn_pkp (no safe replacement) | valid section ID; string ID says PKP, but the resolved name is tactical PKM |
| wpn_saiga12c_m2 | 461 | keep wpn_saiga12c_m2 (no safe replacement) | valid section ID; section/string ID says Saiga-12C M2, but the resolved name is VPO-101 «Вепрь Хантер» |
| wpn_soc94 | 428 | wpn_soc94 → wpn_saiga12c_m2 | replace stale/alias ID with section ID; resolves to Карабин ВПО-101 «Вепрь Хантер» |
| wpn_spas12_m1 | 381 | keep wpn_spas12_m1 (no safe replacement) | valid section ID; section/string ID says SPAS-12 M1, but the resolved name is Benelli M4 |
| wpn_spas15 | 390 | keep wpn_spas15 (no safe replacement) | valid section ID; section/string ID says SPAS-15, but the resolved name is Daewoo USAS-12 |
| wpn_spas15_m1 | 391 | keep wpn_spas15_m1 (no safe replacement) | valid section ID; section/string ID says SPAS-15 M1, but the resolved name is Daewoo USAS-12 M1 |
| wpn_toz34_m1 | 395 | keep wpn_toz34_m1 (no safe replacement) | valid section ID; section ID says TOZ-34 M1, but the resolved name is Hunter shotgun |
| wpn_wincheaster1300 | 396 | keep wpn_wincheaster1300 (no safe replacement) | valid section ID; section ID says Wincheaster 1300, but the resolved name is MP-133 |
| wpn_winchester_m1 | 377 | keep wpn_winchester_m1 (no safe replacement) | valid section ID; section ID says Winchester M1, but the resolved name is MP-153 |




