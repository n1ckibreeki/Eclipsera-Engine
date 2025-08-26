#pragma once
#include "bootstrap/Instance.h"
#include "core/datatypes/Vector3Game.h"
#include "core/datatypes/CFrame.h"
#include "core/datatypes/Color3.h"

// Forward declare Lua
struct lua_State;

struct BasePart : Instance {
    ::Vector3 Size{1.0f,1.0f,1.0f};

    // Store transform internally as a CFrame (position + rotation)
    CFrame CF; 

    float Transparency{0.0f};
    float Reflectance{0.0f};

    bool Anchored{false};
    bool CanCollide{true};
    bool CanTouch{true};
    bool CastShadow{true};

    float Density{1.0f};
    float Friction{0.3f};
    float Elasticity{0.5f};

    Color3 Color{0.63f, 0.63f, 0.63f}; // default white

    BasePart(std::string name, InstanceClass cls);
    ~BasePart() override;

    bool LuaGet(lua_State* L, const char* key) const override;
    bool LuaSet(lua_State* L, const char* key, int valueIndex) override;
};
