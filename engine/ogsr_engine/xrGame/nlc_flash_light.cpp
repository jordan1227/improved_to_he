#include "stdafx.h"
#include "nlc_flash_light.h"
#include "Level.h"

// Lua:
//   id = flash_light.spawn(x, y, z, range, r, g, b, attack_ms, hold_ms, fade_ms
//                          [, flicker 0..1 [, shadow [, volumetric]]])
//   flash_light.stop(id)       -- fade out now
//   flash_light.count()
// Color is the peak color (values above 1 are allowed, like light_color in zone configs).
// flicker randomly dims each frame by up to that share (electric discharge look).

namespace nlc_flash_light
{
namespace
{
constexpr u32 MAX_FLASHES = 16; // headroom for overlapping Pulse strikes; cost is per live light, not per slot

struct Flash
{
    u32 id{};
    ref_light light;
    float range{};
    Fcolor color{};
    u32 t0{}, attack{}, hold{}, fade{};
    float flicker{};
};

xr_vector<Flash> s_flashes;
u32 s_next_id = 1;

float envelope(const Flash& f, u32 now)
{
    const u32 t = now - f.t0;
    if (t < f.attack)
        return float(t) / float(f.attack);
    if (t < f.attack + f.hold)
        return 1.f;
    const u32 tf = t - f.attack - f.hold;
    if (tf >= f.fade)
        return -1.f; // finished
    return 1.f - float(tf) / float(f.fade);
}

int l_spawn(lua_State* L)
{
    if (!g_pGameLevel || !::Render)
    {
        lua_pushinteger(L, 0);
        return 1;
    }
    if (s_flashes.size() >= MAX_FLASHES)
    {
        // the oldest flash makes room; a burst of strikes never piles up lights
        s_flashes.front().light->set_active(false);
        s_flashes.erase(s_flashes.begin());
    }
    Fvector pos;
    pos.set(float(luaL_checknumber(L, 1)), float(luaL_checknumber(L, 2)), float(luaL_checknumber(L, 3)));

    Flash f;
    f.id = s_next_id++;
    f.range = _max(0.1f, float(luaL_checknumber(L, 4)));
    f.color.set(float(luaL_checknumber(L, 5)), float(luaL_checknumber(L, 6)), float(luaL_checknumber(L, 7)), 1.f);
    f.attack = u32(_max(0.0, luaL_optnumber(L, 8, 50)));
    f.hold = u32(_max(0.0, luaL_optnumber(L, 9, 100)));
    f.fade = u32(_max(1.0, luaL_optnumber(L, 10, 400)));
    f.flicker = clampr(float(luaL_optnumber(L, 11, 0)), 0.f, 1.f);
    const bool shadow = lua_isnoneornil(L, 12) ? true : !!lua_toboolean(L, 12);
    const bool volumetric = !lua_isnoneornil(L, 13) && !!lua_toboolean(L, 13);
    f.t0 = Device.dwTimeGlobal;

    f.light = ::Render->light_create();
    f.light->set_type(IRender_Light::POINT);
    f.light->set_shadow(shadow);
    f.light->set_moveable(true);
    f.light->set_volumetric(volumetric);
    if (volumetric)
    {
        f.light->set_volumetric_intensity(0.15f);
        f.light->set_volumetric_distance(0.5f);
    }
    f.light->set_position(pos);
    f.light->set_range(f.range);
    f.light->set_color(0.f, 0.f, 0.f);
    f.light->set_active(true);

    s_flashes.push_back(f);
    lua_pushinteger(L, lua_Integer(f.id));
    return 1;
}

int l_stop(lua_State* L)
{
    const u32 id = u32(luaL_checkinteger(L, 1));
    const u32 now = Device.dwTimeGlobal;
    for (Flash& f : s_flashes)
    {
        if (f.id != id)
            continue;
        // jump into the fade from the current level
        const float k = _max(0.f, envelope(f, now));
        f.attack = 0;
        f.hold = 0;
        f.t0 = now - u32((1.f - k) * float(f.fade));
    }
    return 0;
}

int l_count(lua_State* L)
{
    lua_pushinteger(L, lua_Integer(s_flashes.size()));
    return 1;
}
} // namespace

void on_frame()
{
    if (s_flashes.empty())
        return;
    const u32 now = Device.dwTimeGlobal;
    for (auto it = s_flashes.begin(); it != s_flashes.end();)
    {
        float k = envelope(*it, now);
        if (k < 0.f)
        {
            it->light->set_active(false);
            it = s_flashes.erase(it);
            continue;
        }
        if (it->flicker > 0.f)
            k *= 1.f - it->flicker * ::Random.randF(0.f, 1.f);
        it->light->set_color(it->color.r * k, it->color.g * k, it->color.b * k);
        ++it;
    }
}

void clear()
{
    for (Flash& f : s_flashes)
        f.light->set_active(false);
    s_flashes.clear();
}

void script_register(lua_State* L)
{
    static const luaL_Reg funcs[] = {
        {"spawn", l_spawn},
        {"stop", l_stop},
        {"count", l_count},
        {nullptr, nullptr},
    };
    luaL_register(L, "flash_light", funcs);
    lua_pop(L, 1);
}
} // namespace nlc_flash_light
