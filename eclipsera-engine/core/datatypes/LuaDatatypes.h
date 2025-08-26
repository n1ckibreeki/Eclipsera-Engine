#pragma once
#include "lua.h"
#include "lualib.h"    // Luau’s aux API
#include <new>         // placement new

namespace lb {

template<typename T> struct Traits;

// new userdata + set metatable
template<typename T>
inline T* new_ud(lua_State* L) {
    void* mem = lua_newuserdata(L, sizeof(T));
    luaL_getmetatable(L, lb::Traits<T>::MetaName());
    lua_setmetatable(L, -2);
    return static_cast<T*>(mem);
}

// strong check
template<typename T>
inline const T* check(lua_State* L, int idx) {
    return static_cast<const T*>(luaL_checkudata(L, idx, lb::Traits<T>::MetaName()));
}

// optional testudata (Luau doesn’t provide it)
inline void* luaL_testudata(lua_State* L, int idx, const char* tname) {
    if (lua_getmetatable(L, idx)) {
        luaL_getmetatable(L, tname);
        const bool eq = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        return eq ? lua_touserdata(L, idx) : nullptr;
    }
    return nullptr;
}

// push value by constructing in-place
template<typename T>
inline void push(lua_State* L, const T& v) {
    T* ud = new_ud<T>(L);

    // avoid MSVC CRT debug 'new' macro
#if defined(new)
#   pragma push_macro("new")
#   undef new
#   define LB_RESTORE_NEW 1
#endif
    ::new (static_cast<void*>(ud)) T(v);
#ifdef LB_RESTORE_NEW
#   pragma pop_macro("new")
#   undef LB_RESTORE_NEW
#endif
}

// register: metatable + global table
template<typename T>
inline void register_type(lua_State* L) {
    if (luaL_newmetatable(L, lb::Traits<T>::MetaName())) {
        // build methods table
        lua_newtable(L);                         // mt, methods
        if (const luaL_Reg* m = lb::Traits<T>::Methods()) {
            for (const luaL_Reg* r = m; r && r->name; ++r) {
                lua_pushcfunction(L, r->func, r->name);
                lua_setfield(L, -2, r->name);
            }
        }
        lua_pushvalue(L, -1);                    // mt, methods, methods
        lua_setfield(L, -3, "__methods");        // mt.__methods = methods
        lua_setfield(L, -2, "__index");          // mt.__index   = methods

        // metamethods (may override __index)
        if (const luaL_Reg* mm = lb::Traits<T>::MetaMethods()) {
            for (const luaL_Reg* r = mm; r && r->name; ++r) {
                lua_pushcfunction(L, r->func, r->name);
                lua_setfield(L, -2, r->name);
            }
        }
    }
    lua_pop(L, 1);

    // globals unchanged...
    lua_newtable(L);
    lua_pushcfunction(L, lb::Traits<T>::Ctor(), "new");
    lua_setfield(L, -2, "new");
    if (const luaL_Reg* s = lb::Traits<T>::Statics()) {
        for (const luaL_Reg* r = s; r && r->name; ++r) {
            lua_pushcfunction(L, r->func, r->name);
            lua_setfield(L, -2, r->name);
        }
    }
    lua_setglobal(L, lb::Traits<T>::GlobalName());
}

} // namespace lb
