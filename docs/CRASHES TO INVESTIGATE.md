1. logless crash, happened when putting on stalker_outfit and getting attacked by blind dog on cordon

[08.09.26 10:22:09.846] [9940] check cook-af_spirit_2: good=false anom_pass=nil
[08.09.26 10:22:28.735] [9940] !! INCREASE ['OGSR_MEMORY_CONTROL'] LUAJIT MEMORY USAGE: [35910 KB], MIN|MAX: [35910|35910 KB]
[08.09.26 10:22:46.500] [9940] ! [CGameObject::net_Spawn]: changed visual_name[physics\box\expl_dinamit] found in esc_physic_destroyable_object_0034, keep original physics\balon\kanistra.ogf instead
[08.09.26 10:22:46.503] [9940] Resitred recalc container
[08.09.26 10:23:08.421] [9940] ! [CGameObject::net_Spawn]: changed visual_name[physics\balon\bomb_01] found in esc_explosive_mobiltank_0000, keep original physics\balon\kanistra.ogf instead
[08.09.26 10:23:22.220] [9940] ! [fl] bpui=1 body_skips=0 hands_skips=0
[08.09.26 10:23:22.544] [9940] [OUTFIT_ANIM_DIAG] inventory_open outfit_id=nil outfit=nil
[08.09.26 10:23:22.544] [9940] [ogse_signals]: signal 'on_update' time limited in 'scheduled' queue: 24147. Last call spent 23914, script_name = @c:\games\nlc_ogsr_he\gamedata\scripts\alpet\schedule.script. Next is 5 from 60.
[08.09.26 10:23:24.029] [9940] [OUTFIT_ANIM_DIAG] queue event=slot generation=1 object_id=21760 object=stalker_outfit start_id=nil delay_ms=350
[08.09.26 10:23:24.048] [9940] ! [fl] bpui=1 body_skips=603 hands_skips=201
[08.09.26 10:23:24.159] [9940] [OUTFIT_ANIM_DIAG] inventory_hide
[08.09.26 10:23:24.159] [9940] ! [fl] bpui=0 body_skips=639 hands_skips=213
[08.09.26 10:23:24.384] [9940] [OUTFIT_ANIM_DIAG] resolve event=slot generation=1 start_id=nil final_id=21760 final_outfit=stalker_outfit elapsed_ms=357
[08.09.26 10:23:24.384] [9940] [OUTFIT_ANIM_DIAG] defer_full_backpack outfit=stalker_outfit reason=wait_for_close
[08.09.26 10:23:25.893] [9940] [OUTFIT_ANIM_DIAG] full_backpack_close_complete pending=true outfit=stalker_outfit deferred_ms=1509
[08.09.26 10:23:25.893] [9940] [OUTFIT_ANIM_DIAG] request kind=equip outfit=stalker_outfit hud=outfit_animation_hud key=anm_equip cam=itemuse_anm_effects\liz_outfit_equip.anm active_item=nil equipped_id=21760 equipped=stalker_outfit start_delay_ms=75
[08.09.26 10:23:25.968] [9940] [OUTFIT_ANIM_DIAG] play_begin kind=equip outfit=stalker_outfit hud=outfit_animation_hud equipped_id=21760 equipped=stalker_outfit elapsed_ms=75
[08.09.26 10:23:25.970] [9940] [OUTFIT_ANIM_DIAG] play_result kind=equip outfit=stalker_outfit hud=outfit_animation_hud key=anm_equip raw_duration=3933 duration_ms=3933 fallback=false
[08.09.26 10:23:27.993] [9940] ***************************[ScriptCrashHandler]**********************************
[08.09.26 10:23:27.993] [9940] stack traceback:
[08.09.26 10:23:27.993] [9940] *********************************************************************************
[08.09.26 10:23:27.993] [9940] ********************************************************************************
[08.09.26 10:23:27.993] [9940] !![LogStackTrace] Thread: [X-RAY Primary thread], ExceptionCode: [c0000005]
[08.09.26 10:23:28.586] [9940] !!Unhandled exception stack trace:
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70EDBE577], Fun: [CStalkerAnimationManager::update()] + [2471 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xrGame\stalker_animation_manager_update.cpp-->195] + [2428 byte(s)]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70EA80C79], Fun: [CCustomMonster::UpdatePositionAnimation()] + [377 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xrGame\CustomMonster.cpp-->501] + [286 byte(s)]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70EA807E6], Fun: [CCustomMonster::UpdateCL()] + [2342 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xrGame\CustomMonster.cpp-->468]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E9500AA], Fun: [CAI_Stalker::UpdateCL()] + [1130 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xrGame\Ai\Stalker\ai_stalker.cpp-->596]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3C9D48], Fun: [CObjectList::SingleUpdate()] + [200 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\xr_object_list.cpp-->134]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3C9D26], Fun: [CObjectList::SingleUpdate()] + [166 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\xr_object_list.cpp-->127]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3CA1FB], Fun: [CObjectList::Update()] + [875 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\xr_object_list.cpp-->186] + [11 byte(s)]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3C2803], Fun: [IGame_Level::OnFrame()] + [35 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\IGame_Level.cpp-->117]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E6CBD09], Fun: [CLevel::OnFrame()] + [1769 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xrGame\Level.cpp-->425]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E401299], Fun: [CRegistrator<pureFrame>::Process()] + [73 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\pure.h-->144]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E400B85], Fun: [CRenderDevice::FrameMove()] + [517 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\device.cpp-->501]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3FFEA3], Fun: [CRenderDevice::on_idle()] + [563 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\device.cpp-->261]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E400867], Fun: [CRenderDevice::Run()] + [807 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\device.cpp-->455] + [68 byte(s)]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3B6182], Fun: [Startup()] + [1282 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\x_ray.cpp-->245]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70E3B8639], Fun: [WinMain()] + [7529 byte(s)], File-->Line: [D:\Documents\GitHub\nlc_improved_engine_new\ogsr_engine\xr_3da\x_ray.cpp-->449] + [7451 byte(s)]
! Module: [C:\Games\NLC_OGSR_HE\bin_x64\xrEngine.exe], AddrPC.Offset: [00007FF70F0EF3DA], Fun: [__scrt_common_main_seh()] + [262 byte(s)], File-->Line: [D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl-->288] + [33 byte(s)]
! Module: [C:\WINDOWS\System32\KERNEL32.DLL], AddrPC.Offset: [00007FFC3A4BCCB7], Fun: [BaseThreadInitThunk()] + [23 byte(s)]
! Module: [C:\WINDOWS\SYSTEM32\ntdll.dll], AddrPC.Offset: [00007FFC3AE0AD6C], Fun: [RtlUserThreadStart()] + [44 byte(s)]
! 
[08.09.26 10:23:28.587] [9940] ********************************************************************************
[08.09.26 10:23:29.781] [9940] --Saved dump file to [c:\games\nlc_ogsr_he\appdata\logs\xray_hawkl_09-08-26_10-23-29.mdmp]

2. some weird low fps when loading a save. fps can be increased when looking at specific direction. loading the same save seems to fix the issue. 