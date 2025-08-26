#pragma once
#include "bootstrap/services/Service.h"
#include "bootstrap/signals/Signal.h"
#include <memory>
#include "raylib.h"

class UserInputService : public Service {
public:
    UserInputService();
    
    bool LuaGet(lua_State* L, const char* k) const override;
    bool LuaSet(lua_State* L, const char* k, int idx) override;
    void Update();

private:
    void EnsureSignals() const;
    void PushInputObject(lua_State* L, const char* inputType, const char* keyName, Vector2 mousePos);

    std::shared_ptr<RTScriptSignal> InputBegan;
    std::shared_ptr<RTScriptSignal> InputEnded;
    std::shared_ptr<RTScriptSignal> InputChanged;
    std::shared_ptr<RTScriptSignal> MouseMoved;

    float lastMouseX = 0;
    float lastMouseY = 0;
};