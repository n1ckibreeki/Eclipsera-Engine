#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <limits>
#include "lua.h"
#include "lualib.h"

#include "bootstrap/LuaScheduler.h"

struct RTScriptSignal : std::enable_shared_from_this<RTScriptSignal> {
    struct Listener {
        size_t id{0};
        int    funcRef{LUA_NOREF};
        bool   once{false};
        bool   parallel{false};
        bool   connected{true};
        size_t activePos{npos};
        // Reusable coroutine for non-parallel listeners
        lua_State* co{nullptr};
        int threadRef{LUA_NOREF};  // registry ref that owns the thread
        static constexpr size_t npos = std::numeric_limits<size_t>::max();
    };
    struct Waiter {
        enum class Kind { Script, Task };
        Kind        kind{Kind::Task};
        BaseScript* script{nullptr};
        lua_State*  co{nullptr};
    };

    explicit RTScriptSignal(LuaScheduler* s);
    ~RTScriptSignal();

    // Lua bindings:
    size_t Connect(lua_State* L, bool once=false, bool parallel=false);
    int    Wait(lua_State* L);                      // yields
    void   Fire(lua_State* L, int firstArgIdx, int argc);
    void   Close();                                 // disconnect all, do not resume waiters

    // Connection handles:
    void   Disconnect(size_t id);
    bool   IsConnected(size_t id) const;
    bool   IsClosed() const { return closed; }

private:
    LuaScheduler* sched{};
    lua_State*    Lm{};
    bool          closed{false};

    size_t nextId{1};
    std::vector<Listener> listeners;                 // stable indices
    std::unordered_map<size_t,size_t> id2idx;        // id -> listeners[idx]
    std::vector<size_t> activeIdx;                   // connected listener indices
    std::vector<size_t> tmpActive;                   // per-fire snapshot
    std::vector<Waiter> waiters;

    void wakeWaitersWithArgsOnNextFrame(lua_State* src, int firstArgIdx, int argc);
    void callListenersDeferred(lua_State* src, int firstArgIdx, int argc);
};
