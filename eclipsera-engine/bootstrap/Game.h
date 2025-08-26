#pragma once

#include <memory>
#include <string>
#include "bootstrap/Instance.h"
#include "LuaScheduler.h"
#include "bootstrap/instances/InstanceTypes.h"

class Workspace;

class Game : public Instance {
public:
    std::shared_ptr<Workspace> workspace;
    std::unique_ptr<LuaScheduler> luaScheduler;

    explicit Game(std::string name = "game");
    ~Game() override;

    void Init();
    void Shutdown();

    bool LuaGet(lua_State* L, const char* key) const override;
};

// Global
extern std::shared_ptr<Game> g_game;
