#pragma once
#include "lua.h"
#include <string>
#include <unordered_map>

// Base enum item class
class EnumItem {
public:
    EnumItem(const std::string& name, int value);
    
    const std::string& GetName() const { return name; }
    int GetValue() const { return value; }
    
    // Lua bindings
    bool LuaGet(lua_State* L, const char* k) const;
    
private:
    std::string name;
    int value;
};

// Enum class that contains enum items
class Enum {
public:
    Enum(const std::string& enumName);
    
    void AddItem(const std::string& name, int value);
    EnumItem* GetItem(const std::string& name);
    EnumItem* GetItem(int value);
    
    // Lua bindings
    bool LuaGet(lua_State* L, const char* k) const;
    
private:
    std::string enumName;
    std::unordered_map<std::string, EnumItem*> itemsByName;
    std::unordered_map<int, EnumItem*> itemsByValue;
};

// Global enum registry
class EnumRegistry {
public:
    static EnumRegistry& Instance();
    
    void RegisterEnum(const std::string& name, Enum* enumObj);
    Enum* GetEnum(const std::string& name);
    
    // Initialize built-in enums
    void InitializeBuiltinEnums();
    
private:
    std::unordered_map<std::string, Enum*> enums;
};

// Helper functions for Lua
void Lua_PushEnumItem(lua_State* L, EnumItem* item);
void Lua_PushEnum(lua_State* L, Enum* enumObj);
