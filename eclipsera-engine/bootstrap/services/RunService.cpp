#include "bootstrap/services/RunService.h"
#include <cstring>
#include "bootstrap/Game.h"
#include "core/logging/Logging.h"
#include "bootstrap/Instance.h"

extern void Lua_PushSignal(lua_State* L, const std::shared_ptr<RTScriptSignal>& sig);

RunService::RunService() : Service("RunService", InstanceClass::RunService) {}

void RunService::EnsureSignals() const {
    auto* self = const_cast<RunService*>(this);
    LuaScheduler* sch = (g_game && g_game->luaScheduler) ? g_game->luaScheduler.get() : nullptr;
    if (!self->PreRender)      self->PreRender      = std::make_shared<RTScriptSignal>(sch);
    if (!self->PreAnimation)   self->PreAnimation   = std::make_shared<RTScriptSignal>(sch);
    if (!self->PreSimulation)  self->PreSimulation  = std::make_shared<RTScriptSignal>(sch);
    if (!self->PostSimulation) self->PostSimulation = std::make_shared<RTScriptSignal>(sch);
    if (!self->Heartbeat)      self->Heartbeat      = std::make_shared<RTScriptSignal>(sch);
    if (!self->FixedStep)      self->FixedStep      = std::make_shared<RTScriptSignal>(sch);
    if (!self->PostStep)      self->PostStep      = std::make_shared<RTScriptSignal>(sch);
}

bool RunService::LuaGet(lua_State* L, const char* key) const {
    EnsureSignals();
    if (std::strcmp(key, "PreRender")      == 0) { Lua_PushSignal(L, PreRender);      return true; }
    if (std::strcmp(key, "PreAnimation")   == 0) { Lua_PushSignal(L, PreAnimation);   return true; }
    if (std::strcmp(key, "PreSimulation")  == 0) { Lua_PushSignal(L, PreSimulation);  return true; }
    if (std::strcmp(key, "PostSimulation") == 0) { Lua_PushSignal(L, PostSimulation); return true; }
    if (std::strcmp(key, "Heartbeat")      == 0) { Lua_PushSignal(L, Heartbeat);      return true; }
    if (std::strcmp(key, "RenderStepped")  == 0) { Lua_PushSignal(L, PreRender);      return true; }
    if (std::strcmp(key, "Stepped")        == 0) { Lua_PushSignal(L, PreSimulation);  return true; }
    if (std::strcmp(key, "FixedStep")      == 0) { Lua_PushSignal(L, FixedStep);      return true; }
    if (std::strcmp(key, "PostStep")      == 0) { Lua_PushSignal(L, PostStep);      return true; }
    return false;
}

static Instance::Registrar s_regRunService("RunService", []{
    return std::make_shared<RunService>();
});
