-- Real Lua 5.1 module tests; engine calls are mocked, no target-runtime proof.
local root = 'C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/'
local checks = 0
local function check(v,label) assert(v,label); checks=checks+1 end
local now, alive, present, paused, menu, talking, alarm, powered = 0,true,true,false,false,false,false,true
local overlays, shader_writes = 0,0
local page, blend, hint_text = nil,nil,nil
local timers, display_signals = {},{}
local slot = 3
local actor={alive=function() return alive end,object=function() return present and {} end,
    is_talking=function() return talking end,ph_capture=function() return false end,
    active_item=function() return nil end,active_slot=function() return slot end,
    activate_slot=function(_,s) slot=s end}
db={actor=actor};watch_gate={has_watch=function() return present end}
function time_global() return now end
function log2(...) end
function vector() return {} end
function device() return {is_paused=function() return paused end} end
function get_actor_obj() return {IsDetectorActive=function() return false end} end
level={main_input_receiver=function() return menu and {} or nil end,
    only_movekeys_allowed=function() return false end,only_allow_movekeys=function() end,
    set_actor_allow_ladder=function() end}
game={hud_motion_allowed=function() return true end,play_hud_motion=function() return 100 end,
    stop_hud_motion=function() end,translate_string=function(k) return k end}
sound_object={s2d=1};xr_sound={get_safe_sound_object=function() return {play_no_feedback=function() end} end}
dsh={wait_condition=function(cond,cb) if cond() then cb() else error('unexpected waiting condition') end end,
    timeout=function(_,cb) timers[#timers+1]=cb end}
he_watch_ui={is_active=function() return alarm end}
he_watch_status={is_operational=function() return powered end}
function shader_set_custom_param(name,x,y,z,w)
    assert(name=='m_he_watch_page');page,blend=x,y;shader_writes=shader_writes+1
end
function GetFontLetterica18Russian() return 1 end
function CUIStatic()
    return setmetatable({SetText=function(_,s) hint_text=s end},
        {__index=function() return function() end end})
end
function get_hud() return {AddDialogToRender=function() overlays=overlays+1 end,
    RemoveDialogToRender=function() overlays=overlays-1 end} end
local function module(name,path)
    local env=setmetatable({}, {__index=_G});env.this=env;_G[name]=env
    local f=assert(loadfile(root..path));setfenv(f,env);f();return env
end
local host=module('allcore_hide','allcore_hide.script')
local display=module('he_watch_display','he_watch_display.script')
display.attach({subscribe=function(_,s) display_signals[s.signal]=s.fun end})
local function update(dt) now=now+(dt or 0);display_signals.on_update() end
local function wheel(v) display_signals.on_wheel_rotation(v) end
local function finish_motion() local cb=table.remove(timers,1);assert(cb);cb() end
local saved_values = {}
function get_value(k,default) local v=saved_values[k];if v==nil then return default end;return v end
function set_value(k,v) saved_values[k]=v end
local PAGE='he_watch_selected_page';local HINT='he_watch_view_hint_seen'
display_signals.on_first_update();check(page==0 and overlays==0,'old saves default to clock')
local first=host.begin_watch_presentation('view');finish_motion();update()
check(overlays==1 and saved_values[HINT]==true,'first hint shown and remembered')
update(5000);wheel(1);check(saved_values[PAGE]==1 and overlays==1,'selection saved immediately; hint remains')
update(125);check(math.abs(blend-0.5)<0.00001,'slide still smooth')
update(125);check(blend==1,'slide completes')
update(4749);check(overlays==1,'hint survives until deadline')
update(1);check(overlays==0,'hint expires at ten seconds without wheel extending it')
wheel(-1);check(page==0 and overlays==0 and saved_values[PAGE]==0,'scroll does not repeat hint')
update(250);wheel(1);update(125)
host.end_watch_presentation(first,nil);update()
check(page==1 and blend==1 and saved_values[PAGE]==1,'hide retains target page and completes partial slide')
finish_motion();wheel(1);check(page==1 and saved_values[PAGE]==1,'hidden wheel ignored')
local second=host.begin_watch_presentation('view');finish_motion();update()
check(page==1 and blend==1 and overlays==0,'re-raise restores radiation without hint')
alarm=true;update();wheel(1)
check(page==0 and saved_values[PAGE]==1,'alarm temporarily overrides without changing preference')
alarm=false;update();check(page==1 and blend==1,'alarm exit restores selected page')
menu=true;update();wheel(-1);check(page==1 and saved_values[PAGE]==1,'menu retains selection and ignores wheel')
menu=false;powered=false;update();wheel(-1);check(saved_values[PAGE]==1,'blackout cannot change preference')
powered=true;update();check(page==1,'recovery retains preference')
local snapshot={};for k,v in pairs(saved_values) do snapshot[k]=v end
host.abort_watch_presentation(second);display_signals.on_load_before_all()
saved_values={[PAGE]=0,[HINT]=true};update();check(page==0,'another save restores its own clock choice')
display_signals.on_load_before_all();saved_values=snapshot;update()
check(page==1 and blend==1 and overlays==0,'saved radiation choice restored before next raise')
local third=host.begin_watch_presentation('view');finish_motion();update()
check(page==1 and overlays==0,'loaded save does not repeat consumed hint')
host.abort_watch_presentation(third);display_signals.on_load_before_all();saved_values={};update()
local fourth=host.begin_watch_presentation('view');finish_motion();update()
check(page==0 and overlays==1,'fresh save gets its own first-use hint')
host.end_watch_presentation(fourth,nil);update();finish_motion()
check(overlays==0 and saved_values[HINT]==true,'early lowering dismisses hint permanently for that save')
local fifth=host.begin_watch_presentation('view');finish_motion();update();check(overlays==0,'early-dismissed hint not repeated')
wheel(1);display_signals.on_stop_modules();check(overlays==0 and saved_values[PAGE]==1,'stop cleans transient UI without overwriting save')
print('PASS: '..checks..' page persistence / one-shot hint / lifecycle checks')
