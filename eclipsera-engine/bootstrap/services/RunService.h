#pragma once
#include <memory>
#include "bootstrap/services/Service.h"
#include "bootstrap/signals/Signal.h"
struct lua_State;

struct RunService : Service {
    double fixedRate = 1000.0;
    double accumulator = 0.0;

    std::shared_ptr<RTScriptSignal> PreRender;
    std::shared_ptr<RTScriptSignal> PreAnimation;
    std::shared_ptr<RTScriptSignal> PreSimulation;
    std::shared_ptr<RTScriptSignal> PostSimulation;
    std::shared_ptr<RTScriptSignal> Heartbeat;
    std::shared_ptr<RTScriptSignal> FixedStep;
    std::shared_ptr<RTScriptSignal> PostStep;

    RunService();
    void EnsureSignals() const;
    bool LuaGet(lua_State* L, const char* key) const override;
};