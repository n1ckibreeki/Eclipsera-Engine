#include "Signal.h"

RTScriptSignal::RTScriptSignal(LuaScheduler* s) : sched(s) {
    Lm = s ? s->GetMainState() : nullptr;
    listeners.reserve(5120);
    activeIdx.reserve(5120);
    tmpActive.reserve(5120);
    id2idx.reserve(5120);
}

RTScriptSignal::~RTScriptSignal() {
    Close();
}

size_t RTScriptSignal::Connect(lua_State* L, bool once, bool parallel){
    if (closed || !Lm) return 0;
    luaL_checktype(L, 1, LUA_TFUNCTION);

    // move callback to main, take a registry ref (Luau API)
    lua_pushvalue(L, 1);
    lua_xmove(L, Lm, 1);
    int ref = lua_ref(Lm, -1); // pops the value

    Listener li;
    li.id = nextId++;
    li.funcRef = ref;
    li.once = once;
    li.parallel = parallel;
    li.connected = true;

    if (!li.parallel) {
        // Prepare a reusable coroutine for sequential callbacks
        lua_newthread(Lm);
        li.co = lua_tothread(Lm, -1);
        luaL_sandboxthread(li.co);
        li.threadRef = lua_ref(Lm, -1); // pops that shit thread
    }

    const size_t idx = listeners.size();
    listeners.push_back(li);
    id2idx[li.id] = idx;

    activeIdx.push_back(idx);
    listeners[idx].activePos = activeIdx.size() - 1;

    return li.id;
}

bool RTScriptSignal::IsConnected(size_t id) const {
    auto it = id2idx.find(id);
    if (it == id2idx.end()) return false;
    size_t idx = it->second;
    return idx < listeners.size() && listeners[idx].connected;
}

void RTScriptSignal::Disconnect(size_t id){
    auto it = id2idx.find(id);
    if (it == id2idx.end()) return;

    const size_t idx = it->second;
    Listener& li = listeners[idx];

    if (li.connected) {
        li.connected = false;
        if (Lm && li.funcRef != LUA_NOREF) {
            lua_unref(Lm, li.funcRef);
            li.funcRef = LUA_NOREF;
        }
        if (Lm && li.threadRef != LUA_NOREF) {
            lua_unref(Lm, li.threadRef);
            li.threadRef = LUA_NOREF;
            li.co = nullptr;
        }
        // O(1) remove from active list
        const size_t pos = li.activePos;
        if (pos != Listener::npos && pos < activeIdx.size()) {
            const size_t lastIdx = activeIdx.back();
            activeIdx[pos] = lastIdx;
            listeners[lastIdx].activePos = pos;
            activeIdx.pop_back();
            li.activePos = Listener::npos;
        }
    }

    id2idx.erase(it);
}

int RTScriptSignal::Wait(lua_State* L){
    if (closed) return lua_yield(L, 0);

    if (!sched) {

    //
    //⠀⣞⢽⢪⢣⢣⢣⢫⡺⡵⣝⡮⣗⢷⢽⢽⢽⣮⡷⡽⣜⣜⢮⢺⣜⢷⢽⢝⡽⣝
    //⠸⡸⠜⠕⠕⠁⢁⢇⢏⢽⢺⣪⡳⡝⣎⣏⢯⢞⡿⣟⣷⣳⢯⡷⣽⢽⢯⣳⣫⠇
    //⠀⠀⢀⢀⢄⢬⢪⡪⡎⣆⡈⠚⠜⠕⠇⠗⠝⢕⢯⢫⣞⣯⣿⣻⡽⣏⢗⣗⠏⠀
    //⠀⠪⡪⡪⣪⢪⢺⢸⢢⢓⢆⢤⢀⠀⠀⠀⠀⠈⢊⢞⡾⣿⡯⣏⢮⠷⠁⠀⠀
    //⠀⠀⠀⠈⠊⠆⡃⠕⢕⢇⢇⢇⢇⢇⢏⢎⢎⢆⢄⠀⢑⣽⣿⢝⠲⠉⠀⠀⠀⠀
    //⠀⠀⠀⠀⠀⡿⠂⠠⠀⡇⢇⠕⢈⣀⠀⠁⠡⠣⡣⡫⣂⣿⠯⢪⠰⠂⠀⠀⠀⠀
    //⠀⠀⠀⠀⡦⡙⡂⢀⢤⢣⠣⡈⣾⡃⠠⠄⠀⡄⢱⣌⣶⢏⢊⠂⠀⠀⠀⠀⠀⠀
    //⠀⠀⠀⠀⢝⡲⣜⡮⡏⢎⢌⢂⠙⠢⠐⢀⢘⢵⣽⣿⡿⠁⠁⠀⠀⠀⠀⠀⠀⠀
    //⠀⠀⠀⠀⠨⣺⡺⡕⡕⡱⡑⡆⡕⡅⡕⡜⡼⢽⡻⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    //⠀⠀⠀⠀⣼⣳⣫⣾⣵⣗⡵⡱⡡⢣⢑⢕⢜⢕⡝⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    //⠀⠀⠀⣴⣿⣾⣿⣿⣿⡿⡽⡑⢌⠪⡢⡣⣣⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    //⠀⠀⠀⡟⡾⣿⢿⢿⢵⣽⣾⣼⣘⢸⢸⣞⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    //⠀⠀⠀⠀⠁⠇⠡⠩⡫⢿⣝⡻⡮⣒⢽⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    //         No scheduler?

    // CUT IT OUT LOLLLLLLLLLL BRO WORK 
        luaL_error(L, "No scheduler");
        return 0;
    }

    if (auto* self = static_cast<BaseScript*>(lua_getthreaddata(L))) {
        sched->SetWaitEvent(self);
        waiters.push_back(Waiter{Waiter::Kind::Script, self, nullptr});
    } else {
        sched->SetTaskWaitEvent(L);
        waiters.push_back(Waiter{Waiter::Kind::Task, nullptr, L});
    }
    return lua_yield(L, 0);
}

void RTScriptSignal::wakeWaitersWithArgsOnNextFrame(lua_State* src, int firstArgIdx, int argc){
    if (!sched || !Lm) return;

    auto ws = std::move(waiters);
    waiters.clear();

    for (auto& w : ws){
        lua_State* co = (w.kind == Waiter::Kind::Script)
                      ? sched->GetScriptThread(w.script)
                      : w.co;
        if (!co) continue;
        if (!lua_checkstack(co, argc)) continue;

        for (int i = 0; i < argc; ++i) {
            lua_pushvalue(src, firstArgIdx + i);
            lua_xmove(src, co, 1);
        }

        if (w.kind == Waiter::Kind::Script) {
            sched->ResumeScriptNextFrame(w.script, argc);
        } else {
            sched->WakeTaskNextFrame(co, argc);
        }
    }
}

void RTScriptSignal::callListenersDeferred(lua_State* src, int firstArgIdx, int argc){
    if (!sched || !Lm) return;

    // additions won't run this time..
    tmpActive.assign(activeIdx.begin(), activeIdx.end());

    for (size_t idx : tmpActive){
        if (idx >= listeners.size()) continue;
        Listener& l = listeners[idx];
        if (!l.connected || l.funcRef == LUA_NOREF) continue;

        // Call the function directly on the main thread instead of using coroutines?
        lua_getref(Lm, l.funcRef);
        
        // Push arguments
        for (int i = 0; i < argc; ++i) {
            lua_pushvalue(src, firstArgIdx + i);
            lua_xmove(src, Lm, 1);
        }
        
        // Call the function directly
        const int result = lua_pcall(Lm, argc, 0, 0);
        if (result != LUA_OK) {
            const char* error = lua_tostring(Lm, -1);
            printf("Lua callback error: %s\n", error); // FUCK OFF
            lua_pop(Lm, 1);
        }

        if (l.once) {
            // safe with tmpActive
            Disconnect(l.id);
        }
    }

    tmpActive.clear();
}

void RTScriptSignal::Fire(lua_State* L, int firstArgIdx, int argc){
    if (closed) return;
    callListenersDeferred(L, firstArgIdx, argc);
    wakeWaitersWithArgsOnNextFrame(L, firstArgIdx, argc);
}

void RTScriptSignal::Close(){
    if (closed) return;
    closed = true;

    for (auto& l : listeners){
        if (Lm && l.funcRef != LUA_NOREF) {
            lua_unref(Lm, l.funcRef);
        }
        if (Lm && l.threadRef != LUA_NOREF) {
            lua_unref(Lm, l.threadRef);
        }
        l.connected = false;
        l.funcRef = LUA_NOREF;
        l.threadRef = LUA_NOREF;
        l.co = nullptr;
        l.activePos = Listener::npos;
    }

    listeners.clear();
    id2idx.clear();
    activeIdx.clear();
    tmpActive.clear();
}
