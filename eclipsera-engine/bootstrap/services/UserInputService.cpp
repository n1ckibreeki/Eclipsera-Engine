#include "bootstrap/services/UserInputService.h"
#include "bootstrap/LuaScheduler.h"
#include "bootstrap/Game.h"
#include "core/logging/Logging.h"
#include "core/datatypes/Enum.h"
#include "raylib.h"
#include "lua.h"
#include "lualib.h"
#include <cstring>
#include <unordered_map>

extern void Lua_PushSignal(lua_State* L, const std::shared_ptr<RTScriptSignal>& sig);

// Key mapping helper
static const std::unordered_map<int, const char*> keyNameMap = {
    {KEY_A, "A"}, {KEY_B, "B"}, {KEY_C, "C"}, {KEY_D, "D"},
    {KEY_E, "E"}, {KEY_F, "F"}, {KEY_G, "G"}, {KEY_H, "H"},
    {KEY_I, "I"}, {KEY_J, "J"}, {KEY_K, "K"}, {KEY_L, "L"},
    {KEY_M, "M"}, {KEY_N, "N"}, {KEY_O, "O"}, {KEY_P, "P"},
    {KEY_Q, "Q"}, {KEY_R, "R"}, {KEY_S, "S"}, {KEY_T, "T"},
    {KEY_U, "U"}, {KEY_V, "V"}, {KEY_W, "W"}, {KEY_X, "X"},
    {KEY_Y, "Y"}, {KEY_Z, "Z"},
    {KEY_ZERO, "Zero"}, {KEY_ONE, "One"}, {KEY_TWO, "Two"},
    {KEY_THREE, "Three"}, {KEY_FOUR, "Four"}, {KEY_FIVE, "Five"},
    {KEY_SIX, "Six"}, {KEY_SEVEN, "Seven"}, {KEY_EIGHT, "Eight"},
    {KEY_NINE, "Nine"},
    {KEY_SPACE, "Space"},
    {KEY_ENTER, "Return"},
    {KEY_ESCAPE, "Escape"},
    {KEY_BACKSPACE, "Backspace"},
    {KEY_RIGHT, "Right"},
    {KEY_LEFT, "Left"},
    {KEY_DOWN, "Down"},
    {KEY_UP, "Up"},
    {KEY_LEFT_SHIFT, "LeftShift"},
    {KEY_RIGHT_SHIFT, "RightShift"},
    {KEY_LEFT_CONTROL, "LeftControl"},
    {KEY_RIGHT_CONTROL, "RightControl"},
    {KEY_LEFT_ALT, "LeftAlt"},
    {KEY_RIGHT_ALT, "RightAlt"}
};

UserInputService::UserInputService() : Service("UserInputService", InstanceClass::UserInputService) {}

void UserInputService::EnsureSignals() const {
    auto* self = const_cast<UserInputService*>(this);
    LuaScheduler* sch = (g_game && g_game->luaScheduler) ? g_game->luaScheduler.get() : nullptr;
    if (!self->InputBegan)     self->InputBegan     = std::make_shared<RTScriptSignal>(sch);
    if (!self->InputEnded)     self->InputEnded     = std::make_shared<RTScriptSignal>(sch);
    if (!self->InputChanged)   self->InputChanged   = std::make_shared<RTScriptSignal>(sch);
    if (!self->MouseMoved)     self->MouseMoved     = std::make_shared<RTScriptSignal>(sch);
}

bool UserInputService::LuaGet(lua_State* L, const char* k) const {
    EnsureSignals();
    
    if (!strcmp(k, "InputBegan"))     { Lua_PushSignal(L, InputBegan); return true; }
    if (!strcmp(k, "InputEnded"))     { Lua_PushSignal(L, InputEnded); return true; }
    if (!strcmp(k, "InputChanged"))   { Lua_PushSignal(L, InputChanged); return true; }
    if (!strcmp(k, "MouseMoved"))     { Lua_PushSignal(L, MouseMoved); return true; }
    
    if (!strcmp(k, "MousePosition")) {
        Vector2 mousePos = GetMousePosition();
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, mousePos.x);
        lua_setfield(L, -2, "X");
        lua_pushnumber(L, mousePos.y);
        lua_setfield(L, -2, "Y");
        return true;
    }

    if (!strcmp(k, "MouseEnabled"))   { lua_pushboolean(L, true); return true; }
    if (!strcmp(k, "KeyboardEnabled")) { lua_pushboolean(L, true); return true; }
    if (!strcmp(k, "MouseIconEnabled")) { lua_pushboolean(L, !IsCursorHidden()); return true; }
    
    return false;
}

bool UserInputService::LuaSet(lua_State* L, const char* k, int idx) {
    if (!strcmp(k, "MouseIconEnabled")) {
        bool enabled = lua_toboolean(L, idx);
        if (enabled) ShowCursor(); else HideCursor();
        return true;
    }
    return false;
}

void UserInputService::PushInputObject(lua_State* L, const char* inputType, const char* keyName, Vector2 mousePos) {
    lua_createtable(L, 0, 3);
    
    // UserInputType - get from global Enum table to ensure same instance
    lua_getglobal(L, "Enum");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "UserInputType");
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, inputType);
            if (!lua_isnil(L, -1)) {
                // Found the enum item in global table, use it
                lua_setfield(L, -4, "UserInputType");
                lua_pop(L, 2); // pop UserInputType table and Enum table
            } else {
                // Fallback
                lua_pop(L, 3); // pop nil, UserInputType table, and Enum table
                lua_createtable(L, 0, 1);
                lua_pushstring(L, inputType);
                lua_setfield(L, -2, "Name");
                lua_setfield(L, -2, "UserInputType");
            }
        } else {
            lua_pop(L, 2); // pop UserInputType and Enum
            lua_createtable(L, 0, 1);
            lua_pushstring(L, inputType);
            lua_setfield(L, -2, "Name");
            lua_setfield(L, -2, "UserInputType");
        }
    } else {
        lua_pop(L, 1); // pop Enum
        lua_createtable(L, 0, 1);
        lua_pushstring(L, inputType);
        lua_setfield(L, -2, "Name");
        lua_setfield(L, -2, "UserInputType");
    }
    
    // KeyCode - get from global Enum table to ensure same instance
    if (strlen(keyName) > 0) {
        lua_getglobal(L, "Enum");
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "KeyCode");
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, keyName);
                if (!lua_isnil(L, -1)) {
                    // Found the enum item in global table, use it
                    lua_setfield(L, -4, "KeyCode");
                    lua_pop(L, 2); // pop KeyCode table and Enum table
                } else {
                    // Fallback
                    lua_pop(L, 3); // pop nil, KeyCode table, and Enum table
                    lua_createtable(L, 0, 1);
                    lua_pushstring(L, keyName);
                    lua_setfield(L, -2, "Name");
                    lua_setfield(L, -2, "KeyCode");
                }
            } else {
                lua_pop(L, 2); // pop KeyCode and Enum
                lua_createtable(L, 0, 1);
                lua_pushstring(L, keyName);
                lua_setfield(L, -2, "Name");
                lua_setfield(L, -2, "KeyCode");
            }
        } else {
            lua_pop(L, 1); // pop Enum
            lua_createtable(L, 0, 1);
            lua_pushstring(L, keyName);
            lua_setfield(L, -2, "Name");
            lua_setfield(L, -2, "KeyCode");
        }
    } else {
        lua_createtable(L, 0, 1);
        lua_pushstring(L, keyName);
        lua_setfield(L, -2, "Name");
        lua_setfield(L, -2, "KeyCode");
    }
    
    // Position
    lua_createtable(L, 0, 2);
    lua_pushnumber(L, mousePos.x);
    lua_setfield(L, -2, "X");
    lua_pushnumber(L, mousePos.y);
    lua_setfield(L, -2, "Y");
    lua_setfield(L, -2, "Position");
}

void UserInputService::Update() {
    Vector2 mousePos = GetMousePosition();

    // Handle mouse movement
    if (lastMouseX != mousePos.x || lastMouseY != mousePos.y) {
        if (MouseMoved) {
            lua_State* L = g_game->luaScheduler->GetMainState();
            int top = lua_gettop(L);  // Save current stack top
            PushInputObject(L, "MouseMovement", "", mousePos);
            MouseMoved->Fire(L, top + 1, 1);  // Use absolute indices from saved top
            lua_settop(L, top);  // Restore stack
        }
        lastMouseX = mousePos.x;
        lastMouseY = mousePos.y;
    }

    // Handle keyboard
    for (const auto& [key, name] : keyNameMap) {
        if (IsKeyPressed(key)) {
            if (InputBegan) {
                lua_State* L = g_game->luaScheduler->GetMainState();
                if (L) {
                    int top = lua_gettop(L);  // Save current stack top
                    PushInputObject(L, "Keyboard", name, mousePos);
                    lua_pushboolean(L, false);
                    InputBegan->Fire(L, top + 1, 2);  // Use absolute indices from saved top
                    lua_settop(L, top);  // Restore stack
                }
            }
        }
        if (IsKeyReleased(key)) {
            if (InputEnded) {
                lua_State* L = g_game->luaScheduler->GetMainState();
                int top = lua_gettop(L);  // Save current stack top
                PushInputObject(L, "Keyboard", name, mousePos);
                lua_pushboolean(L, false);
                InputEnded->Fire(L, top + 1, 2);  // Use absolute indices from saved top
                lua_settop(L, top);  // Restore stack
            }
        }
    }

    // Handle mouse buttons
    for (int button = 0; button < 3; button++) {
        if (IsMouseButtonPressed(button)) {
            if (InputBegan) {
                lua_State* L = g_game->luaScheduler->GetMainState();
                int top = lua_gettop(L);  // Save current stack top
                const char* buttonName;
                switch (button) {
                    case 0: buttonName = "MouseButton1"; break;
                    case 1: buttonName = "MouseButton2"; break;
                    case 2: buttonName = "MouseButton3"; break;
                    default: buttonName = "Unknown"; break;
                }
                PushInputObject(L, "MouseButton", buttonName, mousePos);
                lua_pushboolean(L, false);
                InputBegan->Fire(L, top + 1, 2);  // Use absolute indices from saved top
                lua_settop(L, top);  // Restore stack
            }
        }
        if (IsMouseButtonReleased(button)) {
            if (InputEnded) {
                lua_State* L = g_game->luaScheduler->GetMainState();
                int top = lua_gettop(L);  // Save current stack top
                const char* buttonName;
                switch (button) {
                    case 0: buttonName = "MouseButton1"; break;
                    case 1: buttonName = "MouseButton2"; break;
                    case 2: buttonName = "MouseButton3"; break;
                    default: buttonName = "Unknown"; break;
                }
                PushInputObject(L, "MouseButton", buttonName, mousePos);
                lua_pushboolean(L, false);
                InputEnded->Fire(L, top + 1, 2);  // Use absolute indices from saved top
                lua_settop(L, top);  // Restore stack
            }
        }
    }
}

static Instance::Registrar s_regUserInputService("UserInputService", [] {
    return std::make_shared<UserInputService>();
});
