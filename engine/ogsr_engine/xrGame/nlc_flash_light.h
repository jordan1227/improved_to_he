#pragma once

// NLC: short-lived scripted point lights (lightning, Pulse discharge). Fire and forget:
// Lua spawns a flash with an attack/hold/fade envelope and the engine owns, updates
// and destroys the light, so no light object lives in Lua (gamedata sivol_pulse_anomaly.script).

struct lua_State;

namespace nlc_flash_light
{
void on_frame(); // CLevel::OnFrame
void clear(); // CLevel::~CLevel: lights go before the level and the renderer state
void script_register(lua_State* L);
} // namespace nlc_flash_light
