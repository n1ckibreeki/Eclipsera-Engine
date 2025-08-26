// instances/LocalScript.h
#pragma once
#include "BaseScript.h"
#include <string>

struct LocalScript : BaseScript {
    explicit LocalScript(std::string name = "LocalScript");
    LocalScript(std::string name, std::string source);  // optional compatibility
    ~LocalScript() override;
};