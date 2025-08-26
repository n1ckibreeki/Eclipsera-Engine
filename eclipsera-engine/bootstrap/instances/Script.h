// instances/Script.h
#pragma once
#include "BaseScript.h"
#include <string>

struct Script : BaseScript {
    explicit Script(std::string name = "Script");
    Script(std::string name, std::string source);  // compatibility
    ~Script() override;
};