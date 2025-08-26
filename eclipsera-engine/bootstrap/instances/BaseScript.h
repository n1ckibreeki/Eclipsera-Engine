// instances/BaseScript.h
#pragma once
#include "LuaSourceContainer.h"

enum class RunContext { Server, Client, Plugin };

struct BaseScript : LuaSourceContainer {
    bool Enabled{true};
    RunContext Context{RunContext::Server};

    BaseScript(std::string name, InstanceClass cls);
    ~BaseScript() override;

    void SetEnabled(bool e);
    bool IsEnabled() const;
    void Destroy() override;

    void SetRunContext(RunContext rc);
    RunContext GetRunContext() const;

    virtual void Schedule();
};