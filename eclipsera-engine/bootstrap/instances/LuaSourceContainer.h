// instances/LuaSourceContainer.h
#pragma once
#include "bootstrap/Instance.h"
#include <string>
#include <utility>

struct LuaSourceContainer : Instance {
    std::string Source;

    explicit LuaSourceContainer(std::string name, InstanceClass cls)
        : Instance(std::move(name), cls) {}
    ~LuaSourceContainer() override = default;

    void SetSource(std::string s) { Source = std::move(s); }
    const std::string& GetSource() const { return Source; }
};