#pragma once
#include "bootstrap/services/Service.h"
#include "core/datatypes/Color3.h"

struct lua_State;

struct Lighting : Service {
    double ClockTime{12.0};
    double Brightness{2.0};
    Color3 Ambient{0.63f, 0.63f, 0.63f};

    explicit Lighting(std::string name = "Lighting")
        : Service(std::move(name), InstanceClass::Lighting) {}

    bool LuaGet(lua_State* L, const char* key) const override;
    bool LuaSet(lua_State* L, const char* key, int valueIndex) override;
};