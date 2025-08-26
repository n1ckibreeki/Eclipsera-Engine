// ================== Scripting API (shared) ==================
// You can place this in your existing shared API .cpp file that contained RegisterSharedLibreboxAPI

// ================== Includes ==================

#include "ScriptingAPI.h"
#include "bootstrap/Instance.h"
#include "Game.h"

// Raylib
#include <raylib.h>
#include <raymath.h>

// Luau
#include "lua.h"
#include "lualib.h"
#include "luacode.h"

// Engine datatypes
#include "core/datatypes/LuaDatatypes.h"
#include "core/datatypes/Vector3Game.h"
#include "core/datatypes/CFrame.h"
#include "core/datatypes/Color3.h"
#include "core/datatypes/Random.h"
#include "core/datatypes/Enum.h"
#include "core/logging/Logging.h"

// Standard library
#include <cstring>
#include <optional>
#include "bootstrap/instances/InstanceTypes.h"
#include "bootstrap/instances/BaseScript.h"

// Forward declarations
struct Script;
static void RegisterEnumGlobal(lua_State* L);

// --- Signal glue (drop-in) ---
#include "bootstrap/signals/Signal.h"   // RTScriptSignal

struct LuaSignalUD { std::shared_ptr<RTScriptSignal> sig; };
struct LuaConnUD   { std::shared_ptr<RTScriptSignal> sig; size_t id{0}; };

static LuaSignalUD* checkSignal(lua_State* L, int idx) {
    return static_cast<LuaSignalUD*>(luaL_checkudata(L, idx, "Librebox.Signal"));
}
static LuaConnUD* checkConn(lua_State* L, int idx) {
    return static_cast<LuaConnUD*>(luaL_checkudata(L, idx, "Librebox.Connection"));
}

// Connection methods
static int l_conn_disconnect(lua_State* L){
    auto* c = checkConn(L, 1);
    if (c && c->sig) c->sig->Disconnect(c->id);
    return 0;
}
static int l_conn_index(lua_State* L){
    auto* c = checkConn(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // methods lookup
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "__methods");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) { lua_remove(L, -2); lua_remove(L, -2); return 1; }
    lua_pop(L, 2);

    if (std::strcmp(key, "Connected") == 0) {
        bool connected = c && c->sig ? c->sig->IsConnected(c->id) : false;
        lua_pushboolean(L, connected);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}
static int l_conn_gc(lua_State* L){ auto* c = checkConn(L,1); if (c) c->~LuaConnUD(); return 0; }

static void ensure_connection_meta(lua_State* L){
    if (luaL_newmetatable(L, "Librebox.Connection")) {
        lua_newtable(L);
        lua_pushcfunction(L, l_conn_disconnect, "Disconnect"); lua_setfield(L, -2, "Disconnect");
        lua_setfield(L, -2, "__methods");
        lua_pushcfunction(L, l_conn_index, "__index"); lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, l_conn_gc,    "__gc");    lua_setfield(L, -2, "__gc");
    }
    lua_pop(L, 1);
}

// Signal methods
static int l_signal_connect(lua_State* L){
    auto* s = checkSignal(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_remove(L, 1);                 // remove 'self'; function shifts to index 1
    size_t id = s->sig->Connect(L, /*once*/false, /*parallel*/false);

    void* mem = lua_newuserdata(L, sizeof(LuaConnUD));
    new (mem) LuaConnUD{ s->sig, id };
    luaL_getmetatable(L, "Librebox.Connection");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_signal_once(lua_State* L){
    auto* s = checkSignal(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_remove(L, 1);                 // remove 'self'; function now at index 1
    size_t id = s->sig->Connect(L, /*once*/true, /*parallel*/false);

    void* mem = lua_newuserdata(L, sizeof(LuaConnUD));
    new (mem) LuaConnUD{ s->sig, id };
    luaL_getmetatable(L, "Librebox.Connection");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_signal_wait(lua_State* L){
    auto* s = checkSignal(L, 1);
    return s->sig->Wait(L);       // yields; resumed with fired args
}
static int l_signal_gc(lua_State* L){ auto* s = checkSignal(L,1); if (s) s->~LuaSignalUD(); return 0; }

static int l_signal_tostring(lua_State* L) {
    lua_pushliteral(L, "RTScriptSignal");
    return 1;
}

static void ensure_signal_meta(lua_State* L){
    if (luaL_newmetatable(L, "Librebox.Signal")) {
        lua_newtable(L);
        lua_pushcfunction(L, l_signal_connect, "Connect"); lua_setfield(L, -2, "Connect");
        lua_pushcfunction(L, l_signal_once,    "Once");    lua_setfield(L, -2, "Once");
        lua_pushcfunction(L, l_signal_wait,    "Wait");    lua_setfield(L, -2, "Wait");
        lua_setfield(L, -2, "__methods");

        // existing inline __index lambda is here…
        lua_pushcfunction(L, 
            [](lua_State* L)->int{
                lua_getmetatable(L, 1);
                lua_getfield(L, -1, "__methods");
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
                if (lua_isfunction(L, -1)) { lua_remove(L, -2); lua_remove(L, -2); return 1; }
                lua_pop(L, 2);
                lua_pushnil(L);
                return 1;
            }, "__index");
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, l_signal_gc, "__gc"); lua_setfield(L, -2, "__gc");

        // new __tostring
        lua_pushcfunction(L, l_signal_tostring, "tostring");
        lua_setfield(L, -2, "__tostring");
    }
    lua_pop(L, 1);
    ensure_connection_meta(L);
}

// exported symbol used by RunService.cpp
void Lua_PushSignal(lua_State* L, const std::shared_ptr<RTScriptSignal>& sig) {
    ensure_signal_meta(L);
    void* mem = lua_newuserdata(L, sizeof(LuaSignalUD));
    new (mem) LuaSignalUD{ sig };
    luaL_getmetatable(L, "Librebox.Signal");
    lua_setmetatable(L, -2);
}
// --- end Signal glue ---


// ================== Lua <-> Instance ==================
void Lua_PushInstance(lua_State* L, const std::shared_ptr<Instance>& inst) {
    if (!inst) { lua_pushnil(L); return; }
    void* userdata = lua_newuserdata(L, sizeof(std::shared_ptr<Instance>));
    new (userdata) std::shared_ptr<Instance>(inst);
    luaL_getmetatable(L, "Librebox.Instance");
    lua_setmetatable(L, -2);
}

static std::shared_ptr<Instance>* l_check_instance(lua_State* L, int n) {
    return static_cast<std::shared_ptr<Instance>*>(
        luaL_checkudata(L, n, "Librebox.Instance"));
}

static int l_instance_gc(lua_State* L) {
    auto* inst = l_check_instance(L, 1);
    if (inst) inst->~shared_ptr<Instance>();
    return 0;
}

static int l_instance_eq(lua_State* L) {
    auto* a = l_check_instance(L, 1);
    auto* b = l_check_instance(L, 2);
    bool same = a && b && *a && *b && a->get() == b->get();
    lua_pushboolean(L, same);
    return 1;
}

static int l_instance_tostring(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr) {
        lua_pushliteral(L, "Instance");
        return 1;
    }
    auto inst = *inst_ptr;
    std::string s = inst->GetClassName();
    // if (s == "Game") s = "DataModel";
    lua_pushlstring(L, s.c_str(), s.size());
    return 1;
}

// ================== Attribute Marshalling ==================

static void push_attribute(lua_State* L, const Attribute& a) {
    if (std::holds_alternative<bool>(a)) {
        lua_pushboolean(L, std::get<bool>(a));
    } else if (std::holds_alternative<double>(a)) {
        lua_pushnumber(L, std::get<double>(a));
    } else if (std::holds_alternative<std::string>(a)) {
        const auto& s = std::get<std::string>(a);
        lua_pushlstring(L, s.data(), s.size());
    } else if (std::holds_alternative<::Vector3>(a)) {
        Vector3Game v = Vector3Game::fromRay(std::get<::Vector3>(a));
        lb::push(L, v);
    } else if (std::holds_alternative<::Color>(a)) {
        const ::Color c = std::get<::Color>(a);
        lua_createtable(L, 4, 0);
        lua_pushinteger(L, c.r); lua_rawseti(L, -2, 1);
        lua_pushinteger(L, c.g); lua_rawseti(L, -2, 2);
        lua_pushinteger(L, c.b); lua_rawseti(L, -2, 3);
        lua_pushinteger(L, c.a); lua_rawseti(L, -2, 4);
    } else {
        lua_pushnil(L);
    }
}

static bool read_attribute(lua_State* L, int idx, Attribute& out) {
    switch (lua_type(L, idx)) {
        case LUA_TBOOLEAN: out = (bool)lua_toboolean(L, idx); return true;
        case LUA_TNUMBER:  out = (double)lua_tonumber(L, idx); return true;
        case LUA_TSTRING: {
            size_t len = 0;
            const char* s = lua_tolstring(L, idx, &len);
            out = std::string(s, len);
            return true;
        }
        case LUA_TUSERDATA: {
            const Vector3Game* v = lb::check<Vector3Game>(L, idx);
            out = v->toRay();
            return true;
        }
        case LUA_TTABLE: {
            lua_rawgeti(L, idx, 1); lua_rawgeti(L, idx, 2);
            lua_rawgeti(L, idx, 3); lua_rawgeti(L, idx, 4);
            if (lua_isnumber(L, -4) && lua_isnumber(L, -3)
             && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
                ::Color c{};
                c.r = (unsigned char)lua_tointeger(L, -4);
                c.g = (unsigned char)lua_tointeger(L, -3);
                c.b = (unsigned char)lua_tointeger(L, -2);
                c.a = (unsigned char)lua_tointeger(L, -1);
                out = c;
                lua_pop(L, 4);
                return true;
            }
            lua_pop(L, 4);
            return false;
        }
        default:
            return false;
    }
}

// ================== Instance Methods ==================

static int m_SetAttribute(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) return 0;
    auto inst = *inst_ptr;
    const char* name = luaL_checkstring(L, 2);
    Attribute v{};
    if (!read_attribute(L, 3, v)) {
        luaL_error(L, "SetAttribute: unsupported value type for '%s'", name);
        return 0;
    }
    inst->SetAttribute(name, v);
    return 0;
}

static int m_GetAttribute(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    auto inst = *inst_ptr;
    const char* name = luaL_checkstring(L, 2);
    auto v = inst->GetAttribute(name);
    if (!v) { lua_pushnil(L); return 1; }
    push_attribute(L, *v);
    return 1;
}

static int m_GetAttributes(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_newtable(L); return 1; }
    auto inst = *inst_ptr;
    lua_newtable(L);
    for (const auto& [k, v] : inst->GetAttributes()) {
        push_attribute(L, v);
        lua_setfield(L, -2, k.c_str());
    }
    return 1;
}

static int m_GetFullName(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr) { lua_pushnil(L); return 1; }
    auto inst = *inst_ptr;
    std::string full = inst->GetFullName();
    lua_pushlstring(L, full.c_str(), full.size());
    return 1;
}

static int m_Destroy(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr) return 0;
    auto inst = *inst_ptr;
    inst->Destroy();
    if (auto bs = std::dynamic_pointer_cast<BaseScript>(inst)) {
        BaseScript* current = static_cast<BaseScript*>(lua_getthreaddata(L));
        if (current == bs.get()) {
            luaL_error(L, "Script destroyed");
            return 0;
        }
    }
    return 0;
}

// legacy compatibility function
static int m_LegacyFunctionRemove(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (inst_ptr && *inst_ptr) (*inst_ptr)->LegacyFunctionRemove();
    return 0;
}

static int m_GetChildren(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_newtable(L); return 1; }
    auto vec = (*inst_ptr)->GetChildren();
    lua_createtable(L, (int)vec.size(), 0);
    int i = 1;
    for (auto& c : vec) { Lua_PushInstance(L, c); lua_rawseti(L, -2, i++); }
    return 1;
}

static int m_GetDescendants(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_newtable(L); return 1; }
    auto vec = (*inst_ptr)->GetDescendants();
    lua_createtable(L, (int)vec.size(), 0);
    int i = 1;
    for (auto& c : vec) { Lua_PushInstance(L, c); lua_rawseti(L, -2, i++); }
    return 1;
}

static int m_IsA(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushboolean(L, 0); return 1; }
    const char* name = luaL_checkstring(L, 2);
    lua_pushboolean(L, (*inst_ptr)->IsA(name));
    return 1;
}

static int m_Clone(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    Lua_PushInstance(L, (*inst_ptr)->Clone());
    return 1;
}

static int m_FindFirstChild(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    const char* name = luaL_checkstring(L, 2);
    bool recursive = lua_toboolean(L, 3);
    auto inst = *inst_ptr;
    if (!recursive) { Lua_PushInstance(L, inst->FindFirstChild(name)); return 1; }
    if (auto direct = inst->FindFirstChild(name)) { Lua_PushInstance(L, direct); return 1; }
    for (auto& d : inst->GetDescendants()) {
        if (d && d->Alive && d->Name == name) { Lua_PushInstance(L, d); return 1; }
    }
    lua_pushnil(L); return 1;
}

static int m_FindFirstChildOfClass(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    const char* cls = luaL_checkstring(L, 2);
    Lua_PushInstance(L, (*inst_ptr)->FindFirstChildOfClass(cls));
    return 1;
}

static int m_FindFirstChildWhichIsA(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    const char* cls = luaL_checkstring(L, 2);
    Lua_PushInstance(L, (*inst_ptr)->FindFirstChildWhichIsA(cls));
    return 1;
}

static int m_FindFirstAncestor(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    const char* name = luaL_checkstring(L, 2);
    Lua_PushInstance(L, (*inst_ptr)->FindFirstAncestor(name));
    return 1;
}

static int m_FindFirstAncestorOfClass(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    const char* cls = luaL_checkstring(L, 2);
    Lua_PushInstance(L, (*inst_ptr)->FindFirstAncestorOfClass(cls));
    return 1;
}

static int m_FindFirstAncestorWhichIsA(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) { lua_pushnil(L); return 1; }
    const char* cls = luaL_checkstring(L, 2);
    Lua_PushInstance(L, (*inst_ptr)->FindFirstAncestorWhichIsA(cls));
    return 1;
}

static int m_IsDescendantOf(lua_State* L) {
    auto* self = l_check_instance(L, 1);
    auto* other = l_check_instance(L, 2);
    bool ok = self && *self && (*self)->Alive && other && *other && (*other)->Alive
            ? (*self)->IsDescendantOf(*other) : false;
    lua_pushboolean(L, ok);
    return 1;
}

static int m_IsAncestorOf(lua_State* L) {
    auto* self = l_check_instance(L, 1);
    auto* other = l_check_instance(L, 2);
    bool ok = self && *self && (*self)->Alive && other && *other && (*other)->Alive
            ? (*self)->IsAncestorOf(*other) : false;
    lua_pushboolean(L, ok);
    return 1;
}

static int m_ClearAllChildren(lua_State* L) {
    auto* self = l_check_instance(L, 1);
    if (self && *self && (*self)->Alive) (*self)->ClearAllChildren();
    return 0;
}

// ================== Property Access ==================

static int l_instance_index(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr) { lua_pushnil(L); return 1; }

    auto inst = *inst_ptr;
    const char* key = luaL_checkstring(L, 2);

    // Always readable, even if destroyed
    if (key[0] == 'N' && std::strcmp(key, "Name") == 0) {
        lua_pushstring(L, inst->Name.c_str());
        return 1;
    }
    if (key[0] == 'C' && std::strcmp(key, "ClassName") == 0) {
        std::string s = inst->GetClassName();
        lua_pushlstring(L, s.c_str(), s.size());
        return 1;
    }
    if (key[0] == 'P' && std::strcmp(key, "Parent") == 0) {
        Lua_PushInstance(L, inst->Parent.lock());
        return 1;
    }

    if (!inst->Alive) { lua_pushnil(L); return 1; }

    // Delegate object-specific reads to the instance
    if (inst->LuaGet(L, key)) return 1;

    // Child by name
    if (auto child = inst->FindFirstChild(key)) {
        Lua_PushInstance(L, child);
        return 1;
    }

    // Methods
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "__methods");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) {
        lua_remove(L, -2);
        lua_remove(L, -2);
        return 1;
    }
    lua_pop(L, 2);

    lua_pushnil(L);
    return 1;
}

static int l_instance_newindex(lua_State* L) {
    auto* inst_ptr = l_check_instance(L, 1);
    if (!inst_ptr || !*inst_ptr || !(*inst_ptr)->Alive) return 0;

    auto inst = *inst_ptr;
    const char* key = luaL_checkstring(L, 2);

    // Name
    if (key[0] == 'N' && std::strcmp(key, "Name") == 0) {
        const char* newName = luaL_checkstring(L, 3);
        if (inst->Name != newName) {
            if (auto p = inst->Parent.lock()) {
                auto it = p->ChildrenByName.find(inst->Name);
                if (it != p->ChildrenByName.end() && it->second.get() == inst.get()) {
                    p->ChildrenByName.erase(it);
                }
                p->ChildrenByName[newName] = inst;
            }
            inst->Name = newName;
        }
        return 0;
    }

    // Parent
    if (key[0] == 'P' && std::strcmp(key, "Parent") == 0) {
        if (lua_isnil(L, 3)) {
            inst->SetParent(nullptr);
        } else {
            auto* parent_ptr = l_check_instance(L, 3);
            if (parent_ptr) inst->SetParent(*parent_ptr);
        }
        return 0;
    }

    // ClassName writes
    if (key[0] == 'C' && std::strcmp(key, "ClassName") == 0) {
        luaL_error(L, "ClassName is read-only");
        return 0;
    }

    // Delegate object-specific writes to the instance
    if (inst->LuaSet(L, key, 3)) return 0;

    return 0;
}

// ================== Instance API ==================

static int l_Instance_new(lua_State* L) {
    const char* typeName = luaL_checkstring(L, 1);
    LOGI("Lua Instance.new('%s')", typeName);
    Lua_PushInstance(L, Instance::New(typeName));
    return 1;
}

// ================== task.* and wait ==================

extern std::shared_ptr<Game> g_game;

// wait(seconds?) -> yields coroutine; scheduler resumes with the actual waited seconds
static int l_wait(lua_State* L) {
    double seconds = luaL_optnumber(L, 1, 0.0);
    Script* self = (Script*)lua_getthreaddata(L);
    if (g_game && g_game->luaScheduler) {
        if (self) {
            if (seconds > 0.0) g_game->luaScheduler->SetWaitAbs(self, GetTime() + seconds);
            else               g_game->luaScheduler->SetWaitNextFrame(self);
        } else {
            // inside a task thread
            if (seconds > 0.0) g_game->luaScheduler->SetTaskWaitAbs(L, GetTime() + seconds);
            else               g_game->luaScheduler->SetTaskWaitNextFrame(L);
        }
    }
    return lua_yield(L, 0);
}

static int l_task_wait(lua_State* L) {
    return l_wait(L);
}

// task.spawn(func, ...)
static int l_task_spawn(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (!g_game || !g_game->luaScheduler) { lua_pushnil(L); return 1; }

    lua_State* LM = g_game->luaScheduler->GetMainState();
    if (!LM) { lua_pushnil(L); return 1; }

    // Create new thread on main state and sandbox it
    lua_newthread(LM);
    lua_State* co = lua_tothread(LM, -1);
    luaL_sandboxthread(co);
    int ref = lua_ref(LM, -1); // pops thread from LM
    lua_pop(LM, 1);
    
    // Move function + args into the new thread
    int nstack = lua_gettop(L); // includes function
    lua_xmove(L, co, nstack);
    int argc = nstack - 1;
    if (argc < 0) argc = 0;

    // Schedule next frame with pending arg count
    g_game->luaScheduler->ScheduleTaskNextFrame(co, ref, argc);

    // Return the thread object
    lua_getref(LM, ref);
    lua_xmove(LM, L, 1);
    return 1;
}

// task.delay(seconds, func, ...)
static int l_task_delay(lua_State* L) {
    double seconds = luaL_checknumber(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (!g_game || !g_game->luaScheduler) { lua_pushnil(L); return 1; }

    lua_State* LM = g_game->luaScheduler->GetMainState();
    if (!LM) { lua_pushnil(L); return 1; }

    // Remove seconds so stack = func, ...
    lua_remove(L, 1);
    int nstack = lua_gettop(L); // func + args
    int argc = nstack - 1; if (argc < 0) argc = 0;

    // Create and sandbox thread
    lua_newthread(LM);
    lua_State* co = lua_tothread(LM, -1);
    luaL_sandboxthread(co);
    int ref = lua_ref(LM, -1);
    lua_pop(LM, 1);

    // Move func+args into the new thread
    lua_xmove(L, co, nstack);

    // Schedule for the future
    g_game->luaScheduler->ScheduleTaskAt(co, ref, GetTime() + std::max(0.0, seconds), argc);

    // Return the thread
    lua_getref(LM, ref);
    lua_xmove(LM, L, 1);
    return 1;
}

// ================== Global API Registration ==================

void RegisterSharedLibreboxAPI(lua_State* L) {
    LOGI("Registering shared Librebox API");

    // Instance metatable
    luaL_newmetatable(L, "Librebox.Instance");

    lua_newtable(L);
    lua_pushcfunction(L, m_SetAttribute, "SetAttribute"); lua_setfield(L, -2, "SetAttribute");
    lua_pushcfunction(L, m_GetAttribute, "GetAttribute"); lua_setfield(L, -2, "GetAttribute");
    lua_pushcfunction(L, m_GetAttributes,"GetAttributes");lua_setfield(L, -2, "GetAttributes");
    lua_pushcfunction(L, m_GetFullName,  "GetFullName");  lua_setfield(L, -2, "GetFullName");
    lua_pushcfunction(L, m_Destroy, "Destroy");           lua_setfield(L, -2, "Destroy");
    lua_pushcfunction(L, m_GetChildren,   "GetChildren");   lua_setfield(L, -2, "GetChildren");
    lua_pushcfunction(L, m_GetDescendants,"GetDescendants");lua_setfield(L, -2, "GetDescendants");
    lua_pushcfunction(L, m_FindFirstChild,           "FindFirstChild");           lua_setfield(L, -2, "FindFirstChild");
    lua_pushcfunction(L, m_FindFirstChildOfClass,    "FindFirstChildOfClass");    lua_setfield(L, -2, "FindFirstChildOfClass");
    lua_pushcfunction(L, m_FindFirstChildWhichIsA,   "FindFirstChildWhichIsA");   lua_setfield(L, -2, "FindFirstChildWhichIsA");
    lua_pushcfunction(L, m_FindFirstAncestor,        "FindFirstAncestor");        lua_setfield(L, -2, "FindFirstAncestor");
    lua_pushcfunction(L, m_FindFirstAncestorOfClass, "FindFirstAncestorOfClass"); lua_setfield(L, -2, "FindFirstAncestorOfClass");
    lua_pushcfunction(L, m_FindFirstAncestorWhichIsA,"FindFirstAncestorWhichIsA");lua_setfield(L, -2, "FindFirstAncestorWhichIsA");
    lua_pushcfunction(L, m_IsDescendantOf,  "IsDescendantOf");  lua_setfield(L, -2, "IsDescendantOf");
    lua_pushcfunction(L, m_IsAncestorOf,    "IsAncestorOf");    lua_setfield(L, -2, "IsAncestorOf");
    lua_pushcfunction(L, m_ClearAllChildren,"ClearAllChildren");lua_setfield(L, -2, "ClearAllChildren");
    lua_pushcfunction(L, m_Clone, "Clone"); lua_setfield(L, -2, "Clone");
    lua_pushcfunction(L, m_IsA, "IsA"); lua_setfield(L, -2, "IsA");

    // legacy functions for compat
    lua_pushcfunction(L, m_GetChildren,   "getChildren");   lua_setfield(L, -2, "getChildren");
    lua_pushcfunction(L, m_Clone, "clone"); lua_setfield(L, -2, "clone");
    lua_pushcfunction(L, m_LegacyFunctionRemove, "Remove");          lua_setfield(L, -2, "Remove");
    lua_pushcfunction(L, m_LegacyFunctionRemove, "remove");          lua_setfield(L, -2, "remove");
    lua_pushcfunction(L, m_FindFirstChild, "findFirstChild");  lua_setfield(L, -2, "findFirstChild");
    lua_pushcfunction(L, m_IsDescendantOf,  "isDescendantOf");  lua_setfield(L, -2, "isDescendantOf");

    lua_setfield(L, -2, "__methods");   

    lua_pushcfunction(L, l_instance_index,   "index");    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_instance_newindex,"newindex"); lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, l_instance_gc,      "gc");       lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, l_instance_eq,      "eq");       lua_setfield(L, -2, "__eq");
    lua_pushcfunction(L, l_instance_tostring, "tostring");lua_setfield(L, -2, "__tostring");

    lua_pop(L, 1);

    // Instance library
    lua_newtable(L);
    lua_pushcfunction(L, l_Instance_new, "new");
    lua_setfield(L, -2, "new");
    lua_setglobal(L, "Instance");

    // Engine datatypes
    lb::register_type<Vector3Game>(L);
    lb::register_type<CFrame>(L);
    lb::register_type<Color3>(L);
    lb::register_type<Random>(L);

    // Globals
    lua_pushcfunction(L, l_wait, "wait");
    lua_setglobal(L, "wait");

    // task table
    lua_newtable(L);
    lua_pushcfunction(L, l_task_wait,  "wait");  lua_setfield(L, -2, "wait");
    lua_pushcfunction(L, l_task_spawn, "spawn"); lua_setfield(L, -2, "spawn");
    lua_pushcfunction(L, l_task_delay, "delay"); lua_setfield(L, -2, "delay");
    lua_setglobal(L, "task");

    // Enum global table
    RegisterEnumGlobal(L);
}

// Register the Enum global table
static void RegisterEnumGlobal(lua_State* L) {
    lua_newtable(L); // Main Enum table
    
    // Get all enums from registry and add them
    EnumRegistry& registry = EnumRegistry::Instance();
    
    // UserInputType enum
    if (Enum* userInputType = registry.GetEnum("UserInputType")) {
        lua_newtable(L); // UserInputType table
        
        // Pre-populate all enum items as actual table entries (not metatable)
        // This ensures the same table instance is returned every time
        lua_pushstring(L, "None"); Lua_PushEnumItem(L, userInputType->GetItem("None")); lua_settable(L, -3);
        lua_pushstring(L, "MouseButton1"); Lua_PushEnumItem(L, userInputType->GetItem("MouseButton1")); lua_settable(L, -3);
        lua_pushstring(L, "MouseButton2"); Lua_PushEnumItem(L, userInputType->GetItem("MouseButton2")); lua_settable(L, -3);
        lua_pushstring(L, "MouseButton3"); Lua_PushEnumItem(L, userInputType->GetItem("MouseButton3")); lua_settable(L, -3);
        lua_pushstring(L, "MouseWheel"); Lua_PushEnumItem(L, userInputType->GetItem("MouseWheel")); lua_settable(L, -3);
        lua_pushstring(L, "MouseMovement"); Lua_PushEnumItem(L, userInputType->GetItem("MouseMovement")); lua_settable(L, -3);
        lua_pushstring(L, "Touch"); Lua_PushEnumItem(L, userInputType->GetItem("Touch")); lua_settable(L, -3);
        lua_pushstring(L, "Keyboard"); Lua_PushEnumItem(L, userInputType->GetItem("Keyboard")); lua_settable(L, -3);
        lua_pushstring(L, "Focus"); Lua_PushEnumItem(L, userInputType->GetItem("Focus")); lua_settable(L, -3);
        lua_pushstring(L, "Accelerometer"); Lua_PushEnumItem(L, userInputType->GetItem("Accelerometer")); lua_settable(L, -3);
        lua_pushstring(L, "Gyroscope"); Lua_PushEnumItem(L, userInputType->GetItem("Gyroscope")); lua_settable(L, -3);
        lua_pushstring(L, "Gamepad1"); Lua_PushEnumItem(L, userInputType->GetItem("Gamepad1")); lua_settable(L, -3);
        lua_pushstring(L, "Gamepad2"); Lua_PushEnumItem(L, userInputType->GetItem("Gamepad2")); lua_settable(L, -3);
        lua_pushstring(L, "Gamepad3"); Lua_PushEnumItem(L, userInputType->GetItem("Gamepad3")); lua_settable(L, -3);
        lua_pushstring(L, "Gamepad4"); Lua_PushEnumItem(L, userInputType->GetItem("Gamepad4")); lua_settable(L, -3);
        lua_pushstring(L, "TextInput"); Lua_PushEnumItem(L, userInputType->GetItem("TextInput")); lua_settable(L, -3);
        
        lua_setfield(L, -2, "UserInputType");
    }
    
    // KeyCode enum
    if (Enum* keyCode = registry.GetEnum("KeyCode")) {
        lua_newtable(L); // KeyCode table
        
        // Pre-populate all KeyCode enum items
        lua_pushstring(L, "Unknown"); Lua_PushEnumItem(L, keyCode->GetItem("Unknown")); lua_settable(L, -3);
        lua_pushstring(L, "A"); Lua_PushEnumItem(L, keyCode->GetItem("A")); lua_settable(L, -3);
        lua_pushstring(L, "B"); Lua_PushEnumItem(L, keyCode->GetItem("B")); lua_settable(L, -3);
        lua_pushstring(L, "C"); Lua_PushEnumItem(L, keyCode->GetItem("C")); lua_settable(L, -3);
        lua_pushstring(L, "D"); Lua_PushEnumItem(L, keyCode->GetItem("D")); lua_settable(L, -3);
        lua_pushstring(L, "E"); Lua_PushEnumItem(L, keyCode->GetItem("E")); lua_settable(L, -3);
        lua_pushstring(L, "F"); Lua_PushEnumItem(L, keyCode->GetItem("F")); lua_settable(L, -3);
        lua_pushstring(L, "G"); Lua_PushEnumItem(L, keyCode->GetItem("G")); lua_settable(L, -3);
        lua_pushstring(L, "H"); Lua_PushEnumItem(L, keyCode->GetItem("H")); lua_settable(L, -3);
        lua_pushstring(L, "I"); Lua_PushEnumItem(L, keyCode->GetItem("I")); lua_settable(L, -3);
        lua_pushstring(L, "J"); Lua_PushEnumItem(L, keyCode->GetItem("J")); lua_settable(L, -3);
        lua_pushstring(L, "K"); Lua_PushEnumItem(L, keyCode->GetItem("K")); lua_settable(L, -3);
        lua_pushstring(L, "L"); Lua_PushEnumItem(L, keyCode->GetItem("L")); lua_settable(L, -3);
        lua_pushstring(L, "M"); Lua_PushEnumItem(L, keyCode->GetItem("M")); lua_settable(L, -3);
        lua_pushstring(L, "N"); Lua_PushEnumItem(L, keyCode->GetItem("N")); lua_settable(L, -3);
        lua_pushstring(L, "O"); Lua_PushEnumItem(L, keyCode->GetItem("O")); lua_settable(L, -3);
        lua_pushstring(L, "P"); Lua_PushEnumItem(L, keyCode->GetItem("P")); lua_settable(L, -3);
        lua_pushstring(L, "Q"); Lua_PushEnumItem(L, keyCode->GetItem("Q")); lua_settable(L, -3);
        lua_pushstring(L, "R"); Lua_PushEnumItem(L, keyCode->GetItem("R")); lua_settable(L, -3);
        lua_pushstring(L, "S"); Lua_PushEnumItem(L, keyCode->GetItem("S")); lua_settable(L, -3);
        lua_pushstring(L, "T"); Lua_PushEnumItem(L, keyCode->GetItem("T")); lua_settable(L, -3);
        lua_pushstring(L, "U"); Lua_PushEnumItem(L, keyCode->GetItem("U")); lua_settable(L, -3);
        lua_pushstring(L, "V"); Lua_PushEnumItem(L, keyCode->GetItem("V")); lua_settable(L, -3);
        lua_pushstring(L, "W"); Lua_PushEnumItem(L, keyCode->GetItem("W")); lua_settable(L, -3);
        lua_pushstring(L, "X"); Lua_PushEnumItem(L, keyCode->GetItem("X")); lua_settable(L, -3);
        lua_pushstring(L, "Y"); Lua_PushEnumItem(L, keyCode->GetItem("Y")); lua_settable(L, -3);
        lua_pushstring(L, "Z"); Lua_PushEnumItem(L, keyCode->GetItem("Z")); lua_settable(L, -3);
        lua_pushstring(L, "Zero"); Lua_PushEnumItem(L, keyCode->GetItem("Zero")); lua_settable(L, -3);
        lua_pushstring(L, "One"); Lua_PushEnumItem(L, keyCode->GetItem("One")); lua_settable(L, -3);
        lua_pushstring(L, "Two"); Lua_PushEnumItem(L, keyCode->GetItem("Two")); lua_settable(L, -3);
        lua_pushstring(L, "Three"); Lua_PushEnumItem(L, keyCode->GetItem("Three")); lua_settable(L, -3);
        lua_pushstring(L, "Four"); Lua_PushEnumItem(L, keyCode->GetItem("Four")); lua_settable(L, -3);
        lua_pushstring(L, "Five"); Lua_PushEnumItem(L, keyCode->GetItem("Five")); lua_settable(L, -3);
        lua_pushstring(L, "Six"); Lua_PushEnumItem(L, keyCode->GetItem("Six")); lua_settable(L, -3);
        lua_pushstring(L, "Seven"); Lua_PushEnumItem(L, keyCode->GetItem("Seven")); lua_settable(L, -3);
        lua_pushstring(L, "Eight"); Lua_PushEnumItem(L, keyCode->GetItem("Eight")); lua_settable(L, -3);
        lua_pushstring(L, "Nine"); Lua_PushEnumItem(L, keyCode->GetItem("Nine")); lua_settable(L, -3);
        lua_pushstring(L, "Space"); Lua_PushEnumItem(L, keyCode->GetItem("Space")); lua_settable(L, -3);
        lua_pushstring(L, "Return"); Lua_PushEnumItem(L, keyCode->GetItem("Return")); lua_settable(L, -3);
        lua_pushstring(L, "Escape"); Lua_PushEnumItem(L, keyCode->GetItem("Escape")); lua_settable(L, -3);
        lua_pushstring(L, "Backspace"); Lua_PushEnumItem(L, keyCode->GetItem("Backspace")); lua_settable(L, -3);
        lua_pushstring(L, "Right"); Lua_PushEnumItem(L, keyCode->GetItem("Right")); lua_settable(L, -3);
        lua_pushstring(L, "Left"); Lua_PushEnumItem(L, keyCode->GetItem("Left")); lua_settable(L, -3);
        lua_pushstring(L, "Down"); Lua_PushEnumItem(L, keyCode->GetItem("Down")); lua_settable(L, -3);
        lua_pushstring(L, "Up"); Lua_PushEnumItem(L, keyCode->GetItem("Up")); lua_settable(L, -3);
        lua_pushstring(L, "LeftShift"); Lua_PushEnumItem(L, keyCode->GetItem("LeftShift")); lua_settable(L, -3);
        lua_pushstring(L, "RightShift"); Lua_PushEnumItem(L, keyCode->GetItem("RightShift")); lua_settable(L, -3);
        lua_pushstring(L, "LeftControl"); Lua_PushEnumItem(L, keyCode->GetItem("LeftControl")); lua_settable(L, -3);
        lua_pushstring(L, "RightControl"); Lua_PushEnumItem(L, keyCode->GetItem("RightControl")); lua_settable(L, -3);
        lua_pushstring(L, "LeftAlt"); Lua_PushEnumItem(L, keyCode->GetItem("LeftAlt")); lua_settable(L, -3);
        lua_pushstring(L, "RightAlt"); Lua_PushEnumItem(L, keyCode->GetItem("RightAlt")); lua_settable(L, -3);
        lua_pushstring(L, "MouseButton1"); Lua_PushEnumItem(L, keyCode->GetItem("MouseButton1")); lua_settable(L, -3);
        lua_pushstring(L, "MouseButton2"); Lua_PushEnumItem(L, keyCode->GetItem("MouseButton2")); lua_settable(L, -3);
        lua_pushstring(L, "MouseButton3"); Lua_PushEnumItem(L, keyCode->GetItem("MouseButton3")); lua_settable(L, -3);
        
        lua_setfield(L, -2, "KeyCode");
    }
    
    lua_setglobal(L, "Enum");
}
