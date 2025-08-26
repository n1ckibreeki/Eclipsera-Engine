#include "bootstrap/services/Lighting.h"
#include "core/logging/Logging.h"
#include "core/datatypes/Color3.h"
#include "lua.h"
#include "lualib.h"
#include <cstring>

// Register with the service factory
static Instance::Registrar s_regLighting("Lighting", [] {
    return std::make_shared<Lighting>();
});

bool Lighting::LuaGet(lua_State* L, const char* k) const {
    if (!strcmp(k,"ClockTime")) { lua_pushnumber(L, (double)ClockTime); return true; }
    if (!strcmp(k,"Brightness")) { lua_pushnumber(L, (double)Brightness); return true; }
    if (!strcmp(k,"Ambient")) { 
        lb::push(L, Color3{ Ambient.r, Ambient.g, Ambient.b });
        return true;
    }
    // add Ambient, ClockTime, etc.
    return false;
}
bool Lighting::LuaSet(lua_State* L, const char* k, int idx) {
    if (!strcmp(k,"Brightness")) { Brightness = (float)luaL_checknumber(L, idx); return true; }
    if (!strcmp(k,"ClockTime")) { ClockTime = (float)luaL_checknumber(L, idx); return true; }
    if (!strcmp(k,"Ambient")) {
        const auto* c = lb::check<Color3>(L, idx);
        Ambient = { c->r, c->g, c->b };
        return true;
    }
    // add others
    return false;
}