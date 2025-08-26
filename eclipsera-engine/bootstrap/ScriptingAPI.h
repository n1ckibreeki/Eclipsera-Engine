#pragma once
#include <memory>

struct RTScriptSignal;
struct lua_State;
struct Instance;

void RegisterSharedLibreboxAPI(lua_State* L);

// Utility used by scripts to pass Instances to Luau
void Lua_PushInstance(lua_State* L, const std::shared_ptr<Instance>& inst);
void Lua_PushSignal(lua_State* L, const std::shared_ptr<RTScriptSignal>& sig);