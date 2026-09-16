local root = 'C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/'
local checks = 0
local function check(v, label)
    assert(v, label)
    checks = checks + 1
end
local now, days, minute, alive, present, sleeping, tf = 10000, 1, 60, true, true, false, 10
local values, queued, signals = {}, {}, {}
local beeps, stops, wakes, aborts = 0, 0, 0, 0
function time_global() return now end
function log2(...) end
function wprintf(...) end
function get_value(k) return values[k] end
function set_value(k,v) values[k] = v end
function class(name)
    local env = getfenv(2)
    local c = {}
    c.__index = c
    env[name] = setmetatable(c, {__call=function(t,...)
        local obj=setmetatable({},t);obj:__init(...);return obj
    end})
end
local actor={alive=function() return alive end,object=function() return present and {} or nil end}
db={actor=actor}
watch_gate={has_watch=function() return present end}
game={get_game_time=function() return {get=function() return 2012,5,days+1,math.floor(minute/60),minute%60,0,0 end} end,
 CTime=function() return {set=function(self,y,m,d) self.d=d end,diffSec=function(self,other) return (self.d-other.d)*86400 end} end}
level={get_time_factor=function() return tf end,set_time_factor=function(v) tf=v end}
sleep_manager={is_sleep_active=function() return sleeping end,stopper=function() wakes=wakes+1;sleeping=false;tf=10 end,reset_tf=function() tf=10 end}
function device() return {is_paused=function() return false end} end
he_pda_model={get_pda=function() return {} end}
local sound={stop=function() stops=stops+1 end}
vergas_lib={s_play=function() beeps=beeps+1;return sound end}
schedule={add=function(k,s,delay) queued[k]=s end}
ogse_signals={get_mgr=function() return {subscribe=function(_,s) end} end}
local intensity=0
sivol_pda={get_glitch_intensity=function() return intensity end}
he_watch_ui={abort=function() aborts=aborts+1 end}
local function load_module(name, path)
    local env=setmetatable({}, {__index=_G}); env.this=env; _G[name]=env
    local f=assert(loadfile(root..path));setfenv(f,env);f();return env
end
load_module('sivol_emp_config','sivol/sivol_emp_config.script')
local status=load_module('he_watch_status','he_watch_status.script')
local backend=load_module('vergas_watch','vergas/vergas_watch.script')
local w=backend.g_watch
status.attach({subscribe=function(_,s) signals[s.signal]=s.fun end})
local random_calls=0
local roll=0.1
math.random=function(a,b) random_calls=random_calls+1;if a then return a end;return roll end
local function cfg() return w:alarm_config() end
local function arm(h,m) w:enable_alarm(h,m);w:shut_up();queued={};beeps=0;w.prev_update=minute;w.prev_days=days end
local function tick(ms,mins) now=now+ms;if mins then minute=mins end;signals.on_update() end
local function outage() status.on_critical_event(1);check(not status.is_operational(),'critical outage starts') end
check(status.is_operational(),'initial availability')
intensity=0.4;check(status.get_interference()==0.4,'shared intensity');intensity=0
status.on_critical_event(0.949)
check(status.is_operational() and random_calls==0,'below threshold no random failure roll')
check(status.get_interference()>=0.949,'surviving contact still glitches')
roll=0.2;status.on_critical_event(1);check(status.is_operational(),'20 percent boundary survives')
roll=0.199;status.on_critical_event(0.95);check(not status.is_operational(),'threshold qualifies')
tick(3999);check(not status.is_operational(),'duration not early')
tick(1);check(status.is_operational(),'exact expiry recovers')
present=false;local rolls_before=random_calls;status.on_critical_event(1);check(random_calls==rolls_before and status.is_operational(),'absent watch never rolls');present=true
tick(10000);check(status.get_interference()==0,'surge expires')

-- A missed occurrence between two 6-second alarm updates must be skipped.
minute=60;arm(1,1);outage();tick(1000,61)
check(cfg().d==days+1 and cfg().f and cfg().h==1 and cfg().m==1,'missed time advanced, setting retained')
tick(3000);w:update();check(beeps==0 and not queued.wakeup_gg,'no recovery catch-up')

-- Future occurrence remains due today if recovery precedes it.
minute=70;arm(1,12);outage();tick(4000,71)
check(cfg().d==days,'future same-day occurrence retained')
minute=72;w:update();check(beeps==2 and queued.wakeup_gg~=nil,'future occurrence still rings')
local pending=queued.wakeup_gg
sleeping=true;local before_stops=stops;outage()
check(stops==before_stops+1 and cfg().b==0,'currently playing sound stopped')
tick(4000);assert(loadstring(pending))();check(wakes==0,'old delayed wake cannot fire after recovery')

-- Successful ordinary alarm and wake still function.
sleeping=true;minute=90;arm(1,31);minute=91;tick(10000);w:update()
check(beeps==2 and queued.wakeup_gg~=nil,'ordinary alarm sounds')
assert(loadstring(queued.wakeup_gg))();check(wakes==1 and not sleeping,'ordinary alarm wakes')

-- Future power gate shares all functional protections, without battery consumption.
minute=100;arm(2,0);status.set_power_available(false)
check(not status.is_operational() and cfg().f,'power off retains setting')
local oldh=cfg().h;w:enable_alarm(4,0);check(cfg().h==oldh,'cannot edit backend while unavailable')
cfg().b=5;local before_beeps=beeps;w:play_beeper({});check(beeps==before_beeps,'direct beeper entry gated');cfg().b=0
minute=121;signals.on_update();status.set_power_available(true)
check(status.is_operational() and cfg().d==days+1,'power recovery skips missed alarm')

-- Midnight and multi-day accelerated sleep.
minute=1439;arm(0,0);outage();days=days+1;minute=0;tick(4000)
check(cfg().d==days+1 and cfg().h==0 and cfg().m==0,'midnight occurrence skipped')
minute=1;arm(0,2);status.set_power_available(false);days=days+3;minute=3;signals.on_update();status.set_power_available(true)
check(cfg().d==days+1,'multi-day overdue occurrence advances')

-- Alarm-owned sleep scaling is restored on failure, other changes are retained.
sleeping=true;minute=10;tf=10000;arm(0,20);w:update();check(tf==5000,'existing sleep slowdown reached')
outage();check(tf==10000,'failure restores pre-alarm sleep speed');tick(4000)
minute=30;arm(0,40);tf=10000;w:update();tf=1234;outage();check(tf==1234,'failure does not overwrite another time-factor owner');tick(4000)
minute=50;tf=10000;arm(0,51);minute=51;tick(10000);w:update();check(tf==10,'alarm reset before delayed wake')
pending=queued.wakeup_gg;outage();check(tf==10000,'failure between ring and wake restores sleep speed');tick(4000);assert(loadstring(pending))();check(wakes==1,'cancelled wake stays cancelled')

-- Load and actor teardown invalidate sound/token without writing old save config.
sleeping=false;minute=60;arm(1,1);minute=61;tick(10000);w:update();pending=queued.wakeup_gg
local oldset=set_value;local writes=0;set_value=function(...) writes=writes+1;oldset(...) end
signals.on_load_before_all();check(writes==0,'load reset does not write old save config');sleeping=true;assert(loadstring(pending))();check(wakes==1,'load invalidates pending wake')
set_value=oldset;signals.on_update();alive=false;signals.on_update();check(status.is_operational(),'actor teardown clears transient outage');alive=true;sleeping=false

-- Real shared publisher: absent watch doesn't alter original device windows.
he_pda_access={upgraded_emp_resistance=function() return 0 end}
local crit=load_module('sivol_critical_malfunction','sivol/sivol_critical_malfunction.script')
local events=0;local original_event=status.on_critical_event
status.on_critical_event=function(s) events=events+1;check(s==1,'hit severity passed intact');original_event(s) end
now=now+100000;roll=0.1;crit.trigger_from_hit(1)
check(events==1 and not status.is_operational(),'hit publishes exactly once')
crit.trigger_from_hit(1);check(events==1,'active shared outage rejects duplicate event')
tick(4000);check(status.is_operational() and crit.is_device_active('pda'),'watch recovers while PDA stays failed')
now=now+100000;crit.try_roll(true,1,0,'electra');check(events==2,'proximity publishes once')
now=now+100000;crit.try_roll(true,1,1,'electra');check(events==2,'fully shielded proximity does not publish')
status.on_critical_event=original_event

print('PASS: '..checks..' watch EMP/backend checks')

-- Load the real UI with engine-facing calls stubbed; exercise its actual signals.
signals.on_load_before_all();intensity=0
local keys, blocked, ui_signals, ready, overlays = {}, {}, {}, nil, 0
key_bindings={kUSE=1,kPDA_CLOCK=2};DIK_keys={DIK_LSHIFT=3,DIK_RSHIFT=4}
function key_state(k) return keys[k] or 0 end
function bind_to_dik(k) return k end
function dik_to_keyname(k) return 'k'..k end
function GetFontLetterica18Russian() return 1 end
function CUIStatic() return setmetatable({}, {__index=function() return function() end end}) end
function get_hud() return {AddDialogToRender=function() overlays=overlays+1 end,RemoveDialogToRender=function() overlays=overlays-1 end} end
actor.is_talking=function() return false end;actor.ph_capture=function() return false end
function get_actor_obj() return {IsDetectorActive=function() return false end} end
level.main_input_receiver=function() return nil end
level.block_action=function(k) blocked[k]=true end
level.unblock_action=function(k) blocked[k]=nil end
game.translate_string=function() return '%s %s %s' end
allcore_hide={begin_watch_presentation=function(_,cb) ready=cb;return 9 end,end_watch_presentation=function(_,cb) cb(9) end,abort_watch_presentation=function() aborts=aborts+1 end}
local ui=load_module('he_watch_ui','he_watch_ui.script')
ui.attach({subscribe=function(_,s) ui_signals[s.signal]=s.fun end})
ui_signals.on_first_update()
status.set_power_available(false);check(not ui.open(),'editor refuses unavailable watch')
status.set_power_available(true);check(ui.open(),'editor opens recovered watch')
local oldready=ready;status.set_power_available(false);oldready(9)
check(not ui.is_active() and overlays==0,'outage during raise invalidates ready callback')
status.set_power_available(true);check(ui.open(),'reopen after aborted raise');ready(9)
check(overlays==1 and blocked[1],'editor ready with Use blocked')
keys[1]=1;ui_signals.on_update();status.set_power_available(false)
check(not ui.is_active() and overlays==0 and blocked[1],'outage closes editor and holds Use suppression')
local savedh,savedm=cfg().h,cfg().m
ui.confirm();ui.disable();check(cfg().h==savedh and cfg().m==savedm and cfg().f,'stale edit cannot mutate alarm')
keys[1]=0;ui_signals.on_update();check(not blocked[1],'Use restored after physical release')
status.set_power_available(true);ui_signals.on_stop_modules();check(not blocked[2],'native alarm action released at shutdown')
print('PASS: '..checks..' total integrated EMP/alarm/editor checks')
