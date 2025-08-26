#pragma once
#include <memory>
#include "bootstrap/services/Service.h"
#include "bootstrap/signals/Signal.h"
struct lua_State;

struct RunService : Service {
    std::shared_ptr<RTScriptSignal> PreRender;
    std::shared_ptr<RTScriptSignal> PreAnimation;
    std::shared_ptr<RTScriptSignal> PreSimulation;
    std::shared_ptr<RTScriptSignal> PostSimulation;
    std::shared_ptr<RTScriptSignal> Heartbeat;

    RunService();
    void EnsureSignals() const;
    bool LuaGet(lua_State* L, const char* key) const override;
};