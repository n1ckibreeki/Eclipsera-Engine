#include "Game.h"
#include "ScriptingAPI.h"
#include "bootstrap/instances/InstanceTypes.h"
#include "bootstrap/services/Service.h"
#include "core/datatypes/Enum.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include <cstring>

// Engine
#include "core/logging/Logging.h"

std::shared_ptr<Game> g_game; // global definition

Game::Game(std::string name)
    : Instance(std::move(name), InstanceClass::Game) {
    LOGI("Game created");
}

Game::~Game() = default;

void Game::Init() {
    LOGI("Game::Init begin");

    // cmere enums, i need you, pls, i beg you
    // YES Good boy
    EnumRegistry::Instance().InitializeBuiltinEnums();

    // Lua scheduler
    luaScheduler = std::make_unique<LuaScheduler>();
    if (luaScheduler && luaScheduler->GetMainState()) {
        RegisterSharedLibreboxAPI(luaScheduler->GetMainState());
    }

    // expose global "game"
    auto* L = luaScheduler ? luaScheduler->GetMainState() : nullptr;
    if (L) {
        Lua_PushInstance(L, shared_from_this());
        lua_setglobal(L, "game");
    }

    // --- Precreate core services under 'game'
    const char* defaults[] = { "Workspace", "RunService", "Lighting", "UserInputService" };
    for (const char* n : defaults) {
        Service::Create(n);
    }

    // cache Workspace pointer
    workspace = std::dynamic_pointer_cast<Workspace>(Service::Get("Workspace"));

    // create camera inside Workspace if missing
    if (workspace && !workspace->camera) {
        workspace->camera = std::make_shared<CameraGame>("Camera");
        workspace->camera->SetParent(workspace);
        workspace->camera->SetName("CurrentCamera");
    }

    // expose Workspace global
    if (L && workspace) {
        Lua_PushInstance(L, workspace);
        lua_setglobal(L, "Workspace");
    }

    LOGI("Game::Init done");
}

void Game::Shutdown() {
    LOGI("Game::Shutdown begin");

    if (g_game) {
        g_game->Destroy();
    }

    luaScheduler.reset();
    LOGI("Game::Shutdown end");
}

static std::shared_ptr<Instance>* checkInstanceUD(lua_State* L, int idx) {
    return static_cast<std::shared_ptr<Instance>*>(
        luaL_checkudata(L, idx, "Librebox.Instance"));
}

// game:GetService(name)
static int l_game_getservice(lua_State* L) {
    auto* inst_ptr = checkInstanceUD(L, 1);
    if (!inst_ptr || !*inst_ptr) { luaL_error(L, "GetService: invalid self"); return 0; }
    const char* name = luaL_checkstring(L, 2);
    auto svc = Service::Get(name);
    if (!svc) { luaL_error(L, "GetService: '%s' is not a valid service", name); return 0; }
    Lua_PushInstance(L, svc);
    return 1;
}

// idk how old you are but findservice is very fucking old already, its like 2010 ig? i cannot remember properly mb...
// you shouldn't use this but whatever :shrug:
static int l_game_findservice(lua_State* L) {
    auto* inst_ptr = checkInstanceUD(L, 1);
    if (!inst_ptr || !*inst_ptr) { lua_pushnil(L); return 1; }
    const char* name = luaL_checkstring(L, 2);
    Lua_PushInstance(L, Service::Get(name));
    return 1;
}

bool Game::LuaGet(lua_State* L, const char* key) const {
    if (std::strcmp(key, "GetService") == 0) { lua_pushcfunction(L, l_game_getservice, "GetService"); return true; }
    if (std::strcmp(key, "FindService") == 0){ lua_pushcfunction(L, l_game_findservice,"FindService"); return true; }
    return false;
}
