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
display_signals.on_first_update()
check(page==0 and blend==0 and overlays==0,'initial clock / no hint')
wheel(1);check(page==0,'ordinary gameplay wheel ignored')
local first=host.begin_watch_presentation('view')
check(first and not host.get_watch_view_session(),'raising excluded')
wheel(1);check(page==0 and overlays==0,'wheel ignored while raising')
finish_motion();check(host.get_watch_view_session()==first,'real active view session exposed')
update();check(page==0 and overlays==1 and hint_text=='he_watch_view_clock','view opens on clock with hint')
wheel(0);check(page==0,'zero wheel ignored')
wheel(1);check(page==1 and blend==0 and hint_text=='he_watch_view_radiation','one wheel changes page and starts slide')
update(125);check(math.abs(blend-0.5)<0.00001,'halfway through slide')
wheel(-1);check(page==0 and math.abs(blend-0.5)<0.00001,'reverse keeps current slide position')
update(125);check(math.abs(blend-0.25)<0.00001,'reverse interpolates smoothly')
update(125);check(blend==0,'reverse reaches bleeding')
wheel(-1);update(250);check(page==1 and blend==1,'opposite wheel direction also selects radiation / psy')
local writes=shader_writes;update(10);check(shader_writes==writes,'stable page does not rewrite shader each frame')
host.end_watch_presentation(first,nil);update()
check(not host.get_watch_view_session() and page==0 and blend==0 and overlays==0,'closing resets page and hint')
wheel(1);check(page==0,'wheel ignored during lowering');finish_motion()
check(slot==3,'weapon slot restored')
local second=host.begin_watch_presentation('view');finish_motion();update()
check(second~=first and page==0 and overlays==1,'new raise starts on clock')
wheel(1);update(250);paused=true;update();wheel(1)
check(page==0 and blend==0 and overlays==0,'pause resets and suppresses wheel')
paused=false;update();wheel(1);menu=true;update();wheel(-1)
check(page==0 and overlays==0,'menu resets and suppresses wheel')
menu=false;update();wheel(1);powered=false;update();wheel(1)
check(page==0 and overlays==0,'blackout resets and suppresses wheel')
powered=true;update();check(page==0 and overlays==1,'recovery resumes clock')
alarm=true;update();wheel(1);check(page==0 and overlays==0,'editor owns wheel; regular page stays inactive')
alarm=false;update();wheel(1);present=false;update();check(page==0 and overlays==0,'loss of watch clears page')
present=true;update();wheel(1);talking=true;update();check(page==0 and overlays==0,'dialogue clears page')
talking=false;update();wheel(1);alive=false;update();check(page==0 and overlays==0,'death clears page')
alive=true;update();wheel(1);display_signals.on_load_before_all()
check(page==0 and blend==0 and overlays==0,'load reset clears transient selection')
host.abort_watch_presentation(second)
local editor=host.begin_watch_presentation('alarm');finish_motion();update();wheel(1)
check(not host.get_watch_view_session() and page==0 and overlays==0,'actual alarm presentation excluded')
host.abort_watch_presentation(editor);display_signals.on_stop_modules()
check(page==0 and blend==0 and overlays==0,'shutdown cleans page and hint')
print('PASS: '..checks..' regular-watch page/input/slide lifecycle checks')
