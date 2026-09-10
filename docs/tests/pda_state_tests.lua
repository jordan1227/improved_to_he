-- Run from the repository root with Lua 5.1. Engine services are mocked.
local passed = 0
local values, objects, servers, now, next_id, money, talking, info, presented, jam
local spawn_rejected, release_error, release_count, bonus, shadow
local function eq(a,b,label) assert(a==b,(label or "value")..": "..tostring(a).." ~= "..tostring(b)) end
local function mod(name,path)
    local env=setmetatable({},{__index=_G}); env.this=env
    local f=assert(loadfile(path or "gamedata/scripts/"..name..".script")); setfenv(f,env); f(); _G[name]=env
    return env
end
local function inventory()
    sak_inventory.rucksack={}
    sak_inventory.eatable={}
    for id,o in pairs(objects) do
        if o.owner==db.actor then
            local category=o.sect=="device_pda_npc" and sak_inventory.eatable or sak_inventory.rucksack
            local ids=category[o.sect] or {}; ids[#ids+1]=id
            category[o.sect]=ids
        end
    end
    for _,ids in pairs(sak_inventory.rucksack) do table.sort(ids) end
end
local function add(section,condition,client)
    next_id=next_id+1; local id=next_id
    local s={id=id,sect=section}; function s:name() return self.sect..self.id end
    servers[id]=s
    if client~=false then
        local o={n=id,sect=section,cond=condition or 1,owner=db.actor}
        function o:id() return self.n end
        function o:section() return self.sect end
        function o:parent() return self.owner end
        function o:condition() return self.cond end
        function o:set_condition(c) self.cond=c end
        objects[id]=o; inventory()
        if (section=="device_pda" or section=="device_pda_upgraded") and not db.actor.pda_slot then db.actor.pda_slot=id end
    end
    return id
end
local function materialize()
    for id,s in pairs(servers) do
        if not objects[id] then
            local saved=next_id; next_id=id-1; add(s.sect); next_id=saved
        end
    end
end
local function world()
    values,objects,servers,now,next_id,money,talking,info,presented,jam={},{},{},0,100,20000,true,{}, {},false
    spawn_rejected,release_error,release_count,bonus,shadow=false,nil,{},0,{}
    function get_value(k,d) if values[k]==nil then return d end return values[k] end
    function set_value(k,v) values[k]=v end
    function remove_value(k) values[k]=nil end
    function time_global() return now end
    function client_obj(id) return objects[id] end
    function log2(...) end
    function strpos(s,p) return string.find(s,p,1,true) end
    function get_string(s,k,d) return d end
    game={translate_string=function(s) return s end}
    function Frect() return {set=function(self) return self end} end
    local actor={}
    function actor:id() return 1 end
    function actor:money() return money end
    function actor:give_money(v) money=money+v end
    function actor:has_info(k) return info[k] or false end
    function actor:give_info_portion(k) info[k]=true end
    function actor:disable_info_portion(k) info[k]=nil end
    function actor:is_talking() return talking end
    function actor:active_item() return self.held end
    function actor:item_in_slot(slot) return slot==7 and objects[self.pda_slot] or nil end
    function actor:move_to_slot(item) if not self:item_in_slot(7) then self.pda_slot=item:id() end end
    function actor:object(s) for _,o in pairs(objects) do if o.sect==s and o.owner==self then return o end end end
    function actor:give_talk_message(...) end
    function actor:give_game_news(...) end
    db={actor=actor}
    local sim={}
    function sim:object(id) return servers[id] end
    function sim:release(s)
        if release_error==s.id then error("injected release failure") end
        release_count[s.id]=(release_count[s.id] or 0)+1
        if actor.pda_slot==s.id then actor.pda_slot=nil end
        objects[s.id]=nil; servers[s.id]=nil; inventory()
    end
    function alife() return sim end
    misc={spawn_to=function(s) if spawn_rejected then return end return servers[add(s,1,false)] end,
        release_obj=function(o) sim:release(servers[o:id()]) end}
    sak_inventory={rucksack={},need_update_inventory=inventory,dyn_art_base=function(s) return s end}
    function get_shadow_inv()
        return {get_content=function()
            local records={}
            for id,o in pairs(objects) do
                if o.owner==actor or shadow[id] then records[#records+1]={id=id,sect=o.sect,obj=o} end
            end
            table.sort(records,function(a,b) return a.id<b.id end)
            return {ipairs=function() return ipairs(records) end}
        end}
    end
    kotovod={tbl_arts={lvl1={"af_medusa"},lvl2={"af_fireball"}}}
    level={get_game_difficulty=function() return 1 end}
    vergas_pda={get_pda_status=function(id) return objects[id] and objects[id].status or 1 end,
        get_pda_cdata=function(id) return "a|b|c|serial"..id end,set_tbl_sn_nil=function() end,set_info_pda_nil=function() end}
    vergas_lib={str_explode=function(_,s) local t={} for v in s:gmatch("[^|]+") do t[#t+1]=v end return t end}
    pda_battery={is_charging=function() return false end,reset_discharge_clock=function() end}
    he_pda_market={cancel_startup_removal=function() values.he_pda_legitimate_acquired=true end}
    bind_pda={enable_pda=function() end}
    sak={create_items=function(_,n) bonus=bonus+n end,present_tip=function(text) presented[#presented+1]={text=text,t=now}; return true end}
    amk={present_tip=sak.present_tip}
    ui_cheat_pda=nil
    sivol_pda={reception_blocked=function() return jam,"interference" end}
    sleep_manager={is_sleep_active=function() return false end}
    timers={Get_Game_Seconds=function() return now/1000*100 end}
    ogse_signals={get_mgr=function() return {reschedule=function() end} end}
    mod("he_pda_model"); mod("he_pda_access"); mod("he_pda_news")
end
local function tick(ms) now=now+(ms or 200); he_pda_access.on_update() end
local function news(ms) now=now+(ms or 200); he_pda_news.on_update() end
local function known() info.esc_vergas_new_gg=true; info.kvest_art_done=true end
local function test(name,f) world(); f(); passed=passed+1; print("PASS "..name) end

test("addon lifecycle",function()
    local n=0; local sm={subscribe=function(_,v) assert(type(v.fun)=="function"); n=n+1 end}
    he_pda_access.attach(sm); he_pda_news.attach(sm); eq(n,5)
end)
test("Esc after confirmation and asynchronous spawn",function()
    assert(he_pda_access.buy_base()); talking=false; tick(); eq(money,20000); assert(he_pda_access.is_busy())
    materialize(); tick(); eq(money,5000); assert(he_pda_access.last_transaction_success())
    tick(); eq(money,5000); assert(not he_pda_access.can_acquire_base())
end)
test("Esc before confirmation clears selection without payment",function()
    known(); local d=add("device_pda_npc"); local a=add("af_medusa")
    assert(he_pda_access.preview_vergas_one()); he_pda_access.select_donor(); assert(he_pda_access.selection_ready())
    talking=false; tick(); assert(not he_pda_access.has_vergas_preview("one")); assert(objects[d] and objects[a])
    assert(not he_pda_access.is_busy())
end)
test("selected serial is consumed; other donor survives",function()
    known(); local d1=add("device_pda_npc"); local d2=add("device_pda_npc"); objects[d2].status=2
    local a=add("af_medusa"); he_pda_access.preview_vergas_one(); he_pda_access.next_donor(); he_pda_access.select_donor()
    assert(he_pda_access.confirm_reflash()); tick(); assert(objects[d2] and objects[a]); materialize(); tick()
    assert(objects[d1]); eq(objects[d2],nil); eq(objects[a],nil); eq(release_count[d2],1)
end)
test("stale selection cannot confirm",function()
    known(); local d=add("device_pda_npc"); add("af_medusa"); he_pda_access.preview_vergas_one(); he_pda_access.select_donor()
    alife():release(servers[d]); assert(not he_pda_access.confirm_reflash()); assert(not he_pda_access.is_busy())
end)
test("usable donor category and dyn artifact are accepted",function()
    known(); local d=add("device_pda_npc"); add("af_medusa_dyn4d",0.7)
    eq(sak_inventory.rucksack.device_pda_npc,nil); eq(sak_inventory.eatable.device_pda_npc[1],d)
    assert(he_pda_access.can_vergas_one()); assert(not he_pda_access.vergas_artifact_too_poor())
end)
test("poor artifact explanation follows difficulty threshold",function()
    known(); add("device_pda_npc"); add("af_medusa_dyn4d",0.2)
    assert(not he_pda_access.can_vergas_one()); assert(he_pda_access.vergas_artifact_too_poor())
    level.get_game_difficulty=function() return 0 end
    assert(he_pda_access.can_vergas_one()); assert(not he_pda_access.vergas_artifact_too_poor())
end)
test("Wolf hint is one-time and survives module reload",function()
    assert(he_pda_access.wolf_hint_available()); he_pda_access.wolf_hint_seen(); mod("he_pda_access")
    assert(not he_pda_access.wolf_hint_available())
end)
test("five-donor payment preserves unselected and broken donors",function()
    known(); local ids={}; for i=1,7 do ids[i]=add("device_pda_npc") end
    objects[ids[7]].status=3
    assert(he_pda_access.preview_vergas_five()); he_pda_access.next_donor()
    for i=2,6 do he_pda_access.select_donor(); if i<6 then he_pda_access.next_donor() end end
    assert(he_pda_access.selection_ready()); assert(he_pda_access.confirm_reflash()); tick(); materialize(); tick()
    assert(objects[ids[1]] and objects[ids[7]])
    for i=2,6 do eq(objects[ids[i]],nil); eq(release_count[ids[i]],1) end
end)
test("spawn rejection keeps money",function()
    spawn_rejected=true; assert(he_pda_access.buy_base()); tick(); eq(money,20000); assert(he_pda_access.last_transaction_failed())
end)
test("spawn timeout removes staged PDA without charging",function()
    he_pda_access.buy_base(); tick(); local id=values.he_pda_transaction_v2.spawn_id
    tick(11000); eq(servers[id],nil); eq(money,20000); assert(not he_pda_access.is_busy())
end)
test("changed payment removes only staged reward",function()
    he_pda_access.buy_base(); tick(); local id=values.he_pda_transaction_v2.spawn_id
    money=100; materialize(); tick(); eq(objects[id],nil); eq(money,100); assert(he_pda_access.last_transaction_failed())
end)
test("confirmed transaction resumes after load",function()
    he_pda_access.buy_base(); tick(); mod("he_pda_access"); he_pda_access.on_load(); materialize(); tick()
    eq(money,5000); assert(he_pda_access.last_transaction_success())
end)
test("upgrade retains PDA identity and retries partial consumption",function()
    info.bar_darklab_document_done=true; local p=add("device_pda",0.2)
    local a=add("af_fireball"); local b=add("af_fireball")
    assert(he_pda_access.upgrade_pda()); release_error=b; tick(); eq(release_count[a],1); assert(he_pda_access.is_busy())
    release_error=nil; mod("he_pda_access"); he_pda_access.on_load(); tick()
    eq(release_count[a],1); eq(release_count[b],1); eq(objects[p]:condition(),1); assert(he_pda_access.has_upgrade())
end)
test("upgrade without a base PDA grants one asynchronously",function()
    info.bar_darklab_document_done=true; local a=add("af_fireball"); local b=add("af_fireball")
    assert(he_pda_access.upgrade_pda()); tick(); assert(objects[a] and objects[b]); materialize(); tick()
    assert(db.actor:object("device_pda")); assert(he_pda_access.has_upgrade())
end)
test("Petruha accepts eligible TOZ in shadow storage",function()
    local id=add("wpn_bm16_full",0.95); objects[id].owner={id=function() return 9 end}; shadow[id]=true
    assert(he_pda_access.petruha_reward_available()); assert(he_pda_access.give_petruha_reward()); tick(); materialize(); tick()
    eq(objects[id],nil); eq(bonus,2); assert(info.esc_petruha_toz_done)
end)
test("difficulty thresholds and upgrade-only radiation",function()
    local id=add("wpn_bm16_full",0.48); assert(not he_pda_access.petruha_reward_available())
    level.get_game_difficulty=function() return 0 end; assert(he_pda_access.petruha_reward_available())
    values.pda_upgr_rad=true; assert(not he_pda_access.radiation_eligible()); values.he_pda_upgrade_owned=true
    assert(he_pda_access.radiation_eligible())
end)
test("acquisition history and burn permanently close base shop",function()
    values.he_pda_legitimate_acquired=true; assert(not he_pda_access.base_purchase_available())
    values.he_pda_legitimate_acquired=nil; info.pda_is_broken=true
    assert(not he_pda_access.can_acquire_base()); assert(he_pda_access.burn_dialog_available())
end)
test("battery capacity changes from 168 to 336 hours",function()
    local p=add("device_pda"); talking=false
    get_string=function() return "168" end
    timers.Get_Game_Minutes=function() return now/60000 end
    mod("pda_battery"); pda_battery.on_update(); now=60*60000; pda_battery.on_update()
    assert(math.abs(objects[p]:condition()-(1-1/168))<0.0000001)
    objects[p]:set_condition(1); values.he_pda_upgrade_owned=true; now=now+60*60000; pda_battery.on_update()
    assert(math.abs(objects[p]:condition()-(1-1/336))<0.0000001)
end)
test("startup remover cannot consume an in-flight purchase",function()
    level.name=function() return "l01_escape" end; level.set_actor_allow_pda=function() end
    alife().story_object=function() return nil end
    mod("he_pda_market"); he_pda_market.on_late_init()
    assert(he_pda_access.buy_base()); tick(); materialize(); he_pda_market.on_update()
    assert(db.actor:object("device_pda")); tick(); he_pda_market.on_update()
    eq(money,5000); assert(db.actor:object("device_pda")); eq(values.he_pda_drop_pending,nil)
end)
test("update before late-init must not protect the starter PDA",function()
    level.name=function() return "l01_escape" end; level.set_actor_allow_pda=function() end
    alife().story_object=function() return nil end
    local p=add("device_pda"); mod("he_pda_market"); he_pda_market.on_update()
    eq(values.he_pda_legitimate_acquired,nil); assert(objects[p])
    he_pda_market.on_late_init(); eq(objects[p],nil); he_pda_market.on_update()
    eq(values.he_pda_legitimate_acquired,nil); assert(he_pda_access.can_acquire_base())
end)
test("screen lock and alarm modal preserve each other's control state",function()
    local f=assert(io.open("gamedata/scripts/ui/ui_pda_addons.script","rb")); local s=f:read("*a"); f:close()
    local a=assert(s:find("function PDAAdd:UpdateScreenLock()",1,true))
    local b=assert(s:find("function PDAAdd:OnButtonStop()",a,true))
    local env=setmetatable({PDAAdd={}},{__index=_G}); local chunk=assert(loadstring(s:sub(a,b-1))); setfenv(chunk,env); chunk()
    local bg={enabled=true}; function bg:Enable(v) self.enabled=v end; function bg:IsEnabled() return self.enabled end
    level.get_pda_wnd=function() return {FindChild=function() return bg end} end
    local locked=false; sivol_pda.screen_locked=function() return locked end
    local ui=setmetatable({},{__index=env.PDAAdd})
    ui:UpdateScreenLock(); ui:SwitchMainControl(false); eq(bg.enabled,false)
    locked=true; ui:UpdateScreenLock(); ui:SwitchMainControl(true); eq(bg.enabled,false)
    locked=false; ui:UpdateScreenLock(); eq(bg.enabled,true)
    ui:SwitchMainControl(false); locked=true; ui:UpdateScreenLock(); locked=false; ui:UpdateScreenLock(); eq(bg.enabled,false)
end)
local function enqueue(text,delay,origin)
    assert(he_pda_news.enqueue("sak",text,"header",delay,5,nil,nil,nil,nil,7,origin))
end
test("FIFO backlog cannot be overtaken by newly available message",function()
    talking=false; enqueue("first",0); enqueue("second",0); news(1000); eq(#presented,0)
    add("device_pda"); enqueue("third",0); news(); eq(#presented,1); assert(presented[1].text:find("first"))
    news(5000); assert(presented[2].text:find("second")); news(5000); eq(presented[3].text,"third")
end)
test("Apostol pair retains seven real seconds after backlog",function()
    talking=false; enqueue("first",0,"gar_apostol_pair"); enqueue("second",7,"gar_apostol_pair")
    news(60000); add("device_pda"); news(); eq(#presented,1)
    news(6999); eq(#presented,1); news(1); eq(#presented,2); eq(presented[2].t-presented[1].t,7000)
end)
test("delay uses wall clock and pauses across load",function()
    talking=false; add("device_pda"); enqueue("delayed",7); news(3000); eq(#presented,0)
    mod("he_pda_news"); he_pda_news.on_load(); news(60000); eq(#presented,0)
    news(3999); eq(#presented,0); news(1); eq(#presented,1)
end)
test("message enqueued between ticks gets its full delay",function()
    talking=false; add("device_pda"); news(1); now=now+199; enqueue("delayed",1)
    news(1); eq(values.he_pda_news_queue[1].remaining,0.999)
    news(998); eq(#presented,0); news(2); eq(#presented,1)
end)
test("outage during delay blocks delivery until recovery",function()
    talking=false; add("device_pda"); enqueue("queued",1); jam=true; news(2000); eq(#presented,0)
    jam=false; news(); eq(#presented,1); assert(presented[1].text:find("he_pda_delayed_body"))
end)
test("presentation failure retains queue head",function()
    talking=false; add("device_pda"); enqueue("queued",0)
    local good=sak.present_tip; sak.present_tip=function() error("injected presentation failure") end
    news(); eq(he_pda_news.get_queue_depth(),1); sak.present_tip=good; news(); eq(he_pda_news.get_queue_depth(),0)
end)
test("development queue migration preserves text",function()
    talking=false; add("device_pda")
    values.he_pda_news_queue={{schema=1,channel="pda",route="sak",seq=1,text="legacy"}}
    news(); eq(#presented,1); assert(presented[1].text:find("legacy"))
end)
test("model conversion waits for attachment and preserves charge",function()
    talking=false; values.he_pda_upgrade_owned=true; local old=add("device_pda",0.42)
    values.alarm_time={h=12,m=30}; he_pda_model.on_update(); assert(objects[old])
    materialize(); he_pda_model.on_update(); eq(objects[old],nil)
    local p=he_pda_model.get_pda(); eq(p:section(),"device_pda_upgraded"); eq(p:condition(),0.42)
    eq(db.actor:item_in_slot(7):id(),p:id())
    eq(values.alarm_time.h,12); eq(money,20000); eq(he_pda_model.section(),"device_pda_upgraded")
    assert(not he_pda_access.can_acquire_base()); he_pda_model.on_update(); eq(release_count[old],1)
end)
test("model conversion waits for holster",function()
    talking=false; values.he_pda_upgrade_owned=true; local old=add("device_pda")
    db.actor.held=objects[old]; he_pda_model.on_update(); eq(values.he_pda_model_transfer,nil)
    db.actor.held=nil; he_pda_model.on_update(); assert(values.he_pda_model_transfer)
end)
test("model conversion resumes after load",function()
    talking=false; values.he_pda_upgrade_owned=true; local old=add("device_pda",0.37)
    he_pda_model.on_update(); mod("he_pda_model"); he_pda_model.on_load(); materialize(); he_pda_model.on_update()
    eq(objects[old],nil); eq(he_pda_model.get_pda():condition(),0.37)
end)
test("model spawn failure leaves original intact",function()
    talking=false; values.he_pda_upgrade_owned=true; local old=add("device_pda",0.6)
    spawn_rejected=true; he_pda_model.on_update(); eq(values.he_pda_model_transfer,nil); assert(objects[old])
end)
test("model conversion retries release without duplicating payment",function()
    talking=false; values.he_pda_upgrade_owned=true; local old=add("device_pda",0.55)
    he_pda_model.on_update(); materialize(); release_error=old; he_pda_model.on_update()
    eq(values.he_pda_model_transfer.phase,"commit"); release_error=nil; he_pda_model.on_update()
    eq(release_count[old],1); eq(he_pda_model.get_pda():condition(),0.55); eq(money,20000)
end)
test("upgraded model still receives queued messages",function()
    talking=false; values.he_pda_upgrade_owned=true; add("device_pda_upgraded")
    enqueue("advanced",0); news(); eq(#presented,1)
end)
test("already-converted save recovers empty PDA slot",function()
    talking=false; values.he_pda_upgrade_owned=true; local p=add("device_pda_upgraded")
    db.actor.pda_slot=nil; he_pda_model.on_update(); eq(db.actor:item_in_slot(7):id(),p)
end)
test("model recovery does not displace another slotted item",function()
    talking=false; values.he_pda_upgrade_owned=true; add("device_pda_upgraded")
    local other=add("other_slot_item"); db.actor.pda_slot=other; he_pda_model.on_update()
    eq(db.actor:item_in_slot(7):id(),other)
end)
test("notes install consumes chip and exact artifact without refilling battery",function()
    local p=add("device_pda",0.37); local chip=add("pda_chip_rad"); local a=add("af_medusa_dyn4d",0.5)
    local b=add("af_medusa_dyn5d",0.9); values.alarm_time={h=5,m=30}; values.he_pda_last_upd=123
    assert(he_pda_access.module_install_available()); assert(he_pda_access.install_notes_module()); talking=false; tick()
    eq(objects[chip],nil); eq(objects[a],nil); assert(objects[b]); eq(objects[p]:condition(),0.37)
    assert(he_pda_access.has_notes()); assert(he_pda_access.custom_marker_eligible()); assert(not he_pda_access.has_upgrade())
    assert(not he_pda_access.radiation_eligible()); eq(values.alarm_time.h,5); eq(values.he_pda_last_upd,123)
    assert(not he_pda_access.module_dialog_available())
end)
test("notes installation requires fifty percent on both difficulties",function()
    add("device_pda"); add("pda_chip_rad"); local a=add("af_medusa_dyn4d",0.49)
    for _,diff in ipairs({0,3}) do level.get_game_difficulty=function() return diff end; assert(not he_pda_access.module_install_available()) end
    objects[a]:set_condition(0.5); assert(he_pda_access.module_install_available())
end)
test("notes payment revalidated after confirmation",function()
    add("device_pda"); local chip=add("pda_chip_rad"); local a=add("af_medusa_dyn4d",0.5)
    assert(he_pda_access.install_notes_module()); objects[a]:set_condition(0.4); tick()
    assert(objects[chip] and objects[a]); assert(not he_pda_access.has_notes()); assert(he_pda_access.last_transaction_failed())
end)
test("chip cannot be eaten and pickup hint is one-time",function()
    local calls=0; sak.send_tip_local=function() calls=calls+1 end
    he_pda_access.module_pickup(); he_pda_access.module_pickup(); eq(calls,1)
    local item={section=function() return "pda_chip_rad" end,disable_use=function(self) self.disabled=true end}
    he_pda_access.before_use(item); assert(item.disabled); eq(calls,2); assert(not he_pda_access.radiation_eligible())
end)
test("missing quick-use feedback ignores PDA and suppresses last-item false alarm",function()
    talking=false; clsid={obj_medkit=1,obj_bandage=2}; key_bindings={kUSE_MEDKIT=10,kUSE_BANDAGE=11}
    level.main_input_receiver=function() return nil end
    local count=0; sak.send_tip_local=function() count=count+1 end; mod("he_local_feedback")
    he_local_feedback.on_key(1,10); eq(count,1); he_local_feedback.on_key(1,10); eq(count,1)
    now=2000; he_local_feedback.before_use({clsid=function() return 2 end}); he_local_feedback.on_key(2,11); eq(count,1)
    now=2600; he_local_feedback.on_key(2,11); eq(count,2)
end)
test("actual local tip wrapper bypasses missing and broken PDA gates",function()
    local f=assert(io.open("gamedata/scripts/sak/sak.script","rb")); local s=f:read("*a"); f:close()
    local a=assert(s:find("function send_tip_local(",1,true)); local b=assert(s:find("function send_tip(",a,true))
    local calls=0; local env=setmetatable({present_tip=function() calls=calls+1;return true end},{__index=_G})
    local chunk=assert(loadstring(s:sub(a,b-1))); setfenv(chunk,env); chunk()
    info.pda_is_broken=true; pda_battery.news_blocked=function() error("local feedback must not inspect PDA battery") end
    assert(env.send_tip_local("local")); eq(calls,1)
end)
test("actual item receipt popup does not require a PDA",function()
    talking=false
    local f=assert(io.open("gamedata/scripts/sak/sak.script","rb")); local s=f:read("*a"); f:close()
    local a=assert(s:find("function relocate_item( type,",1,true)); local b=assert(s:find("function send_treasure(",a,true))
    local calls=0; db.actor.give_game_news=function() calls=calls+1 end
    local env=setmetatable({get_inv_name=function(s) return s end,get_texture_info=function() return "icon",{} end,
        razgruzka={is_ammo=function() return false end}},{__index=_G})
    local chunk=assert(loadstring(s:sub(a,b-1))); setfenv(chunk,env); chunk()
    pda_battery.news_blocked=function() error("item receipt must not inspect PDA battery") end
    env.relocate_item("in","mutant_part",1); eq(calls,1)
end)
test("Sidor discount survives load; first acceptance remains fifteen thousand",function()
    assert(he_pda_access.sidor_initial_offer_available()); assert(he_pda_access.buy_base())
    tick(); materialize(); tick(); eq(money,5000)
    world(); he_pda_access.mark_sidor_discount(); mod("he_pda_access"); he_pda_access.on_load()
    money=10000; assert(he_pda_access.sidor_discount_offer_available()); assert(he_pda_access.buy_sidor_base())
    talking=false; tick(); materialize(); tick(); eq(money,0); tick(); eq(money,0)
end)
test("Barman charges exactly eight thousand independently of Sidor",function()
    money=7999; assert(not he_pda_access.barman_purchase_available()); assert(not he_pda_access.buy_barman_base())
    money=8000; assert(he_pda_access.buy_barman_base()); tick(); materialize(); tick(); eq(money,0)
    assert(not he_pda_access.sidor_discounted())
end)
test("intro acknowledgment persists and meeting gate uses only meeting info",function()
    assert(he_pda_access.sidor_intro_available()); assert(he_pda_access.vergas_not_met())
    info.esc_vergas_new_gg=true; assert(not he_pda_access.vergas_not_met()); assert(not he_pda_access.vergas_known())
    he_pda_access.mark_sidor_intro_seen(); mod("he_pda_access"); assert(not he_pda_access.sidor_intro_available())
end)
test("Radar moves upgraded offer from Sidor to Docent",function()
    info.bar_darklab_document_done=true; add("af_fireball",1); add("af_fireball",1)
    assert(he_pda_access.upgrade_artifacts_have()); assert(not he_pda_access.upgrade_sold_out())
    info.bar_deactivate_radar_done=true; assert(he_pda_access.upgrade_sold_out())
    assert(not he_pda_access.upgrade_pda())
    assert(he_pda_access.docent_upgrade_dialog_available()); assert(not he_pda_access.docent_upgrade_available())
end)
test("Docent accepts an af_capsule at any condition and preserves other items",function()
    info.bar_deactivate_radar_done=true; local p=add("device_pda",0.3); local cap=add("af_capsule",0.01)
    local spare=add("af_capsule",1); assert(he_pda_access.buy_docent_upgrade()); talking=false
    mod("he_pda_access"); he_pda_access.on_load(); tick()
    assert(he_pda_access.has_upgrade()); eq(objects[cap],nil); assert(objects[spare]); eq(release_count[cap],1)
    eq(money,20000); eq(objects[p]:condition(),1); assert(not he_pda_access.buy_docent_upgrade())
    he_pda_model.on_update(); materialize(); he_pda_model.on_update()
    eq(he_pda_model.get_pda():section(),"device_pda_upgraded")
end)
test("Docent asynchronous delivery revalidates capsule before payment",function()
    info.bar_deactivate_radar_done=true; local cap=add("af_capsule")
    assert(he_pda_access.buy_docent_upgrade()); tick(); assert(objects[cap]); assert(he_pda_access.is_busy())
    alife():release(servers[cap]); materialize(); tick()
    assert(not he_pda_access.has_upgrade()); assert(not he_pda_model.get_pda()); eq(money,20000)
end)
test("Docent does not overlap burn repair or charging custody",function()
    info.bar_deactivate_radar_done=true; add("af_capsule"); add("device_pda")
    info.pda_will_be_broken=true; assert(not he_pda_access.buy_docent_upgrade()); info.pda_will_be_broken=nil
    info.docent_self_pda_time_start=true; assert(not he_pda_access.buy_docent_upgrade()); info.docent_self_pda_time_start=nil
    pda_battery.is_charging=function() return true end; assert(not he_pda_access.buy_docent_upgrade())
end)
test("upgraded shielding requires ownership and leaves basic notes module unchanged",function()
    mod("sivol_emp_config","gamedata/scripts/sivol/sivol_emp_config.script")
    values.he_pda_notes_installed=true; local p=add("device_pda"); eq(he_pda_access.upgraded_emp_resistance(),0)
    values.he_pda_upgrade_owned=true; eq(he_pda_access.upgraded_emp_resistance(),0.05)
    alife():release(servers[p]); eq(he_pda_access.upgraded_emp_resistance(),0)
end)
test("Docent replaces a burnt ordinary PDA without clearing quest history",function()
    info.bar_deactivate_radar_done=true; info.pda_is_broken=true; values.pda_breaks_count=2
    add("af_capsule"); local broken=add("device_pda_npc_gg"); add("device_pda",0.2)
    assert(he_pda_access.buy_docent_upgrade()); talking=false; tick()
    assert(he_pda_access.has_upgrade()); assert(not info.pda_is_broken); eq(objects[broken],nil)
    eq(values.pda_breaks_count,2); eq(money,20000)
end)
test("emission survival is ten percent at a due cycle and consumes no regular RNG",function()
    sivol_emp_config={UPGRADED_PDA_EMISSION_SURVIVAL=0.10}
    local random=math.random; local calls=0
    math.random=function() calls=calls+1; return 0.099 end
    assert(he_pda_access.emission_burn_due()); eq(calls,0)
    add("device_pda_upgraded"); values.he_pda_upgrade_owned=true
    assert(not he_pda_access.emission_burn_due()); eq(calls,1)
    math.random=function() return 0.10 end; assert(he_pda_access.emission_burn_due())
    math.random=random
end)
test("personal shielding shortens PDA outage without changing torch or NVD",function()
    local f=assert(io.open("gamedata/scripts/sivol/sivol_emp_config.script","rb")); local s=f:read("*a"); f:close()
    local cfg=setmetatable({},{__index=_G}); local chunk=assert(loadstring(s)); setfenv(chunk,cfg); chunk(); sivol_emp_config=cfg
    f=assert(io.open("gamedata/scripts/sivol/sivol_critical_malfunction.script","rb")); s=f:read("*a"); f:close()
    local random=math.random; math.random=function(a,b) return a or 0 end
    local function outage(upgraded)
        values.he_pda_upgrade_owned=upgraded; now=0
        local env=setmetatable({},{__index=_G}); local c=assert(loadstring(s)); setfenv(c,env); c()
        env.trigger_from_hit(0); return env
    end
    add("device_pda"); local basic=outage(false); local advanced=outage(true); now=12000
    assert(basic.is_device_active("pda")); assert(not advanced.is_device_active("pda"))
    for _,device in ipairs({"torch","nvd"}) do eq(basic.is_device_active(device),advanced.is_device_active(device)) end
    math.random=random
end)
test("actual emission scheduler preserves failure quota on a survived cycle",function()
    local f=assert(io.open("gamedata/scripts/sak/sak.script","rb"));local s=f:read("*a");f:close()
    local a=assert(s:find('if db.actor:has_info("pda_will_be_broken") then',1,true))
    local b=assert(s:find('function pda_will_brock()',a,true))
    local forced=0
    local env=setmetatable({black_tip={have_black_tip=function() return false end},
        lua_random=function(lo,hi) return lo end,pda_will_brock=function() forced=forced+1 end},{__index=_G})
    local c=assert(loadstring('return function() '..s:sub(a,b-1)));setfenv(c,env);local emission=c()
    sivol_emp_config={UPGRADED_PDA_EMISSION_SURVIVAL=0.10};values.he_pda_upgrade_owned=true;add("device_pda_upgraded")
    info.val_raid_start=true;values.brocked_flag=1;values.pda_breaks_count=1
    local random=math.random;math.random=function() return 0.05 end;emission()
    eq(values.pda_breaks_count,1);assert(not info.pda_will_be_broken);eq(values.brocked_flag,5)
    values.brocked_flag=1;math.random=function() return 0.5 end;emission()
    eq(values.pda_breaks_count,2);assert(info.pda_will_be_broken)
    math.random=function() error("forced damage must not reroll") end;emission();eq(forced,1)
    assert(not info.pda_will_be_broken);math.random=random
end)
test("actual PDA exposure attenuation preserves shared EMP inputs",function()
    local f=assert(io.open("gamedata/scripts/sivol/sivol_pda.script","rb"));local s=f:read("*a");f:close()
    local a=assert(s:find('    local shield = cached_artifact_pda_shield or 0',1,true))
    local b=assert(s:find('    -- 2. Determine target mix value',a,true))
    local rolled,lesser
    local env=setmetatable({cached_artifact_pda_shield=0.2,cached_artifact_pda_glitch=0,
        electra_target=1,is_strong=true,source_type="electra",loc_base=0,cached_rain_intensity=0,loc_mix=0,
        sivol_critical_malfunction={get_hit_glitch_intensity=function() return 0 end,
            try_roll=function(_,intensity,shield) rolled={intensity,shield} end},
        update_lesser_emp_event=function(intensity) lesser=intensity end},{__index=_G})
    sivol_emp_config={UPGRADED_PDA_EMP_RESISTANCE=0.05};values.he_pda_upgrade_owned=true;add("device_pda_upgraded")
    local c=assert(loadstring(s:sub(a,b-1)..' return target,electra_eff'));setfenv(c,env);local target,exposure=c()
    assert(math.abs(target-0.76)<0.000001);eq(exposure,target);eq(rolled[1],0.8);eq(rolled[2],0.2);eq(lesser,0.8)
end)
print("PDA mocked behavior: "..passed.." tests passed (not engine acceptance)")
