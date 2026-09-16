local root = 'C:/Games/NLC_OGSR_HE files/improved_to_he/gamedata/scripts/'
local checks=0
local function check(v,label) assert(v,label);checks=checks+1 end
local function near(a,b,label) check(math.abs(a-b)<0.000001,label) end
local now, alive, selected = 0,true,nil
local objects, signals = {},{}
db={actor={id=function() return 1 end,alive=function() return alive end,
 object=function() return selected end}}
game={get_game_time=function() local t=now;return {diffSec=function(_,other) return t-other.t end,t=t} end}
level={object_by_id=function(id) return objects[id] end}
local consumed=0
misc={release_obj=function(obj) consumed=consumed+1;objects[obj:id()]=nil end}
function time_global() return now*1000 end
function log2() end
local function load_module(name,path)
 local env=setmetatable({}, {__index=_G});env.this=env;_G[name]=env
 local f=assert(loadfile(root..path));setfenv(f,env);f();return env
end
local function item(id,section,charge,parent)
 local o={id=function() return id end,section=function() return section end,
 parent=function() return parent end,condition=function(self) return self.charge end,
 set_condition=function(self,v) self.charge=v end,charge=charge}
 objects[id]=o;return o
end
local bat=load_module('he_watch_battery','he_watch_battery.script')
local aborts,stops,skips=0,0,0
he_watch_ui={abort=function() aborts=aborts+1 end}
vergas_watch={g_watch={on_watch_state_changed=function(_,available) stops=stops+1 end,
 skip_unavailable_alarm=function() skips=skips+1 end,stop_alarm_output=function() end}}
watch_gate={has_watch=function() return selected~=nil end}
sivol_pda={get_glitch_intensity=function() return 0 end}
sivol_emp_config={WATCH_EMP={surge_duration_min=2000,surge_duration_max=8000,
 critical_threshold=0.95,critical_chance=0.2,outage_min=4000,outage_max=8000}}
math.random=function(a,b) return a or 0.1 end
local status=load_module('he_watch_status','he_watch_status.script')
bat.attach({subscribe=function(_,s) signals[s.signal]=s.fun end})
check(not status.is_operational(),'absent watch unavailable')
local watch=item(10,'device_watch',1,db.actor);selected=watch
check(status.is_operational(),'new watch powered')
now=3*86400;near(bat.get_charge(),0.5,'three game days = half')
now=6*86400;check(not status.is_operational(),'six game days = empty')
near(watch:condition(),0,'empty clamped');check(aborts>=2 and skips>0,'empty closes editor and skips alarm')
local battery=item(20,'batt_torch',1,db.actor)
check(bat.recharge(20,10),'AA recharge accepted');near(watch:condition(),1,'AA full')
check(consumed==1 and status.is_operational(),'one consumed, resumes')
check(not bat.recharge(20,10),'replayed ID rejected')
item(21,'batt_torch',1,db.actor)
check(not bat.recharge(21,10) and consumed==1,'full rejects without consumption')
now=now+30;signals.on_save();near(watch:condition(),1-30/(6*86400),'save flushes fractional minute')
local saved=watch:condition();signals.on_load_before_all();now=now+500000
near(bat.get_charge(),saved,'load keeps condition without offline catchup')
now=now+60;near(bat.get_charge(),saved-60/(6*86400),'post-load drain resumes')
saved=watch:condition();now=now-10000;near(bat.get_charge(),saved,'backward clock no charge gain/loss')
local spare=item(11,'device_watch',0.4,db.actor)
selected=spare;now=now+86400;near(bat.get_charge(),0.4,'new selected watch gets fresh baseline')
near(watch:condition(),saved,'spare unchanged')
selected=nil;now=now+86400;near(bat.get_charge(),0,'absence clears clock')
selected=spare;near(bat.get_charge(),0.4,'return after absence no catchup')
local npc={id=function() return 2 end}
item(22,'batt_torch',1,npc)
check(not bat.recharge(22,11),'foreign battery rejected')
spare.parent=function() return npc end
check(not bat.recharge(21,11),'foreign watch rejected')
spare.parent=function() return db.actor end
item(23,'vodka',1,db.actor);check(not bat.recharge(23,11),'wrong source rejected')
check(not bat.recharge(21,999),'missing target rejected')
check(not bat.recharge(11,11),'same ID rejected')
status.on_critical_event(1);check(not status.is_operational(),'EMP active')
check(bat.recharge(21,11),'recharge during EMP');check(not status.is_operational(),'recharge does not bypass EMP')
now=now+10;check(status.is_operational(),'EMP expires normally')
spare.charge=0;signals.on_load_before_all();check(not status.is_operational(),'empty load unavailable on first query')
spare.charge=0.75;bat.reset();bat.get_charge();now=now+8*86400
near(bat.get_charge(),0,'long sleep clamps empty')
spare.charge=0.5;bat.reset();bat.get_charge();now=now+30;signals.on_drop_before_all(spare)
near(spare.charge,0.5-30/(6*86400),'drop flushes fractional interval')
alive=false;check(not bat.is_powered(),'dead actor unavailable')
print('PASS '..checks..' watch battery checks (mocked Lua 5.1, not engine proof)')
