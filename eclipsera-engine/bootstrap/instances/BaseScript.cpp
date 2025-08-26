// instances/BaseScript.cpp
#include "bootstrap/instances/BaseScript.h"
#include "bootstrap/Game.h"
#include "bootstrap/LuaScheduler.h"
#include "bootstrap/ScriptingAPI.h"
#include "core/logging/Logging.h"

extern std::shared_ptr<Game> g_game;

BaseScript::BaseScript(std::string name, InstanceClass cls)
    : LuaSourceContainer(std::move(name), cls) {
    LOGI("BaseScript created '%s'", Name.c_str());
}

BaseScript::~BaseScript() { LOGI("~BaseScript '%s'", Name.c_str()); }

void BaseScript::SetEnabled(bool e) { Enabled = e; }
bool BaseScript::IsEnabled() const { return Enabled; }

void BaseScript::SetRunContext(RunContext rc) { Context = rc; }
RunContext BaseScript::GetRunContext() const { return Context; }

void BaseScript::Schedule() {
    if (!Enabled) {
        LOGI("Script '%s' not scheduled (disabled).", Name.c_str());
        return;
    }
    if (!g_game || !g_game->luaScheduler) {
        LOGE("Cannot schedule script '%s', Lua scheduler not available.", Name.c_str());
        return;
    }

    auto selfSp = std::static_pointer_cast<BaseScript>(shared_from_this());

    g_game->luaScheduler->AddScript(
        selfSp,
        Name,
        GetSource(),
        [selfSp](lua_State* co, BaseScript*) {
            if (g_game && g_game->workspace) Lua_PushInstance(co, g_game->workspace);
            else lua_pushnil(co);
            lua_setglobal(co, "workspace");

            Lua_PushInstance(co, selfSp);
            lua_setglobal(co, "script");
        });
}

void BaseScript::Destroy() {
    // Cancel the coroutine if scheduled.
    if (g_game && g_game->luaScheduler) {
        auto selfSp = std::static_pointer_cast<BaseScript>(shared_from_this());
        g_game->luaScheduler->StopScript(selfSp.get());
    }
    // Then tear down the instance tree.
    LuaSourceContainer::Destroy();
}