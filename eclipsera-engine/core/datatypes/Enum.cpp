#include "Enum.h"
#include "lua.h"
#include "lualib.h"
#include <cstring>

// EnumItem implementation
EnumItem::EnumItem(const std::string& name, int value) : name(name), value(value) {}

bool EnumItem::LuaGet(lua_State* L, const char* k) const {
    if (!strcmp(k, "Name")) {
        lua_pushstring(L, name.c_str());
        return true;
    }
    if (!strcmp(k, "Value")) {
        lua_pushinteger(L, value);
        return true;
    }
    return false;
}

// Enum implementation
Enum::Enum(const std::string& enumName) : enumName(enumName) {}

void Enum::AddItem(const std::string& name, int value) {
    EnumItem* item = new EnumItem(name, value);
    itemsByName[name] = item;
    itemsByValue[value] = item;
}

EnumItem* Enum::GetItem(const std::string& name) {
    auto it = itemsByName.find(name);
    return (it != itemsByName.end()) ? it->second : nullptr;
}

EnumItem* Enum::GetItem(int value) {
    auto it = itemsByValue.find(value);
    return (it != itemsByValue.end()) ? it->second : nullptr;
}

bool Enum::LuaGet(lua_State* L, const char* k) const {
    auto it = itemsByName.find(k);
    if (it != itemsByName.end()) {
        Lua_PushEnumItem(L, it->second);
        return true;
    }
    return false;
}

// EnumRegistry implementation
// to be honest this is a fucking mess and I hate it, but whatever it works ig

// updated: yeah it's works lmo
EnumRegistry& EnumRegistry::Instance() {
    static EnumRegistry instance;
    return instance;
}

void EnumRegistry::RegisterEnum(const std::string& name, Enum* enumObj) {
    enums[name] = enumObj;
}

Enum* EnumRegistry::GetEnum(const std::string& name) {
    auto it = enums.find(name);
    return (it != enums.end()) ? it->second : nullptr;
}

void EnumRegistry::InitializeBuiltinEnums() {
    // UserInputType enum
    Enum* userInputType = new Enum("UserInputType");
    userInputType->AddItem("None", 0);
    userInputType->AddItem("MouseButton1", 1);
    userInputType->AddItem("MouseButton2", 2);
    userInputType->AddItem("MouseButton3", 3);
    userInputType->AddItem("MouseWheel", 4);
    userInputType->AddItem("MouseMovement", 5);
    userInputType->AddItem("Touch", 6);
    userInputType->AddItem("Keyboard", 7);
    userInputType->AddItem("Focus", 8);
    userInputType->AddItem("Accelerometer", 9);
    userInputType->AddItem("Gyroscope", 10);
    userInputType->AddItem("Gamepad1", 11);
    userInputType->AddItem("Gamepad2", 12);
    userInputType->AddItem("Gamepad3", 13);
    userInputType->AddItem("Gamepad4", 14);
    userInputType->AddItem("TextInput", 15);
    RegisterEnum("UserInputType", userInputType);
    
    // KeyCode enum
    Enum* keyCode = new Enum("KeyCode");
    keyCode->AddItem("Unknown", 0);
    keyCode->AddItem("A", 1);
    keyCode->AddItem("B", 2);
    keyCode->AddItem("C", 3);
    keyCode->AddItem("D", 4);
    keyCode->AddItem("E", 5);
    keyCode->AddItem("F", 6);
    keyCode->AddItem("G", 7);
    keyCode->AddItem("H", 8);
    keyCode->AddItem("I", 9);
    keyCode->AddItem("J", 10);
    keyCode->AddItem("K", 11);
    keyCode->AddItem("L", 12);
    keyCode->AddItem("M", 13);
    keyCode->AddItem("N", 14);
    keyCode->AddItem("O", 15);
    keyCode->AddItem("P", 16);
    keyCode->AddItem("Q", 17);
    keyCode->AddItem("R", 18);
    keyCode->AddItem("S", 19);
    keyCode->AddItem("T", 20);
    keyCode->AddItem("U", 21);
    keyCode->AddItem("V", 22);
    keyCode->AddItem("W", 23);
    keyCode->AddItem("X", 24);
    keyCode->AddItem("Y", 25);
    keyCode->AddItem("Z", 26);
    keyCode->AddItem("Zero", 27);
    keyCode->AddItem("One", 28);
    keyCode->AddItem("Two", 29);
    keyCode->AddItem("Three", 30);
    keyCode->AddItem("Four", 31);
    keyCode->AddItem("Five", 32);
    keyCode->AddItem("Six", 33);
    keyCode->AddItem("Seven", 34);
    keyCode->AddItem("Eight", 35);
    keyCode->AddItem("Nine", 36);
    keyCode->AddItem("Space", 37);
    keyCode->AddItem("Return", 38);
    keyCode->AddItem("Escape", 39);
    keyCode->AddItem("Backspace", 40);
    keyCode->AddItem("Right", 41);
    keyCode->AddItem("Left", 42);
    keyCode->AddItem("Down", 43);
    keyCode->AddItem("Up", 44);
    keyCode->AddItem("LeftShift", 45);
    keyCode->AddItem("RightShift", 46);
    keyCode->AddItem("LeftControl", 47);
    keyCode->AddItem("RightControl", 48);
    keyCode->AddItem("LeftAlt", 49);
    keyCode->AddItem("RightAlt", 50);
    keyCode->AddItem("MouseButton1", 51);
    keyCode->AddItem("MouseButton2", 52);
    keyCode->AddItem("MouseButton3", 53);
    RegisterEnum("KeyCode", keyCode);
}

// Lua helper functions
void Lua_PushEnumItem(lua_State* L, EnumItem* item) {
    if (!item) {
        lua_pushnil(L);
        return;
    }
    
    // Create a simple table with Name and Value properties so people like you can access them
    lua_createtable(L, 0, 2);
    lua_pushstring(L, item->GetName().c_str());
    lua_setfield(L, -2, "Name");
    lua_pushinteger(L, item->GetValue());
    lua_setfield(L, -2, "Value");
}

void Lua_PushEnum(lua_State* L, Enum* enumObj) {
    if (!enumObj) {
        lua_pushnil(L);
        return;
    }
    
    // Create a simple table - for now we'll just return an empty table
    // In a full implementation, this would contain all enum the fucking roblox enums items, i hope so? pls work
    lua_createtable(L, 0, 0);
}
