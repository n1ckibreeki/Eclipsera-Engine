#include "Vector3Game.h"
#include <cstring> // For strcmp
using namespace lb;

// --- Constructor ---
static int v3_new(lua_State* L){
    float x=(float)luaL_optnumber(L,1,0), y=(float)luaL_optnumber(L,2,0), z=(float)luaL_optnumber(L,3,0);
    push(L, Vector3Game{x,y,z});
    return 1;
}

// --- Metamethods ---
static int v3_tostring(lua_State* L){
    auto* v = check<Vector3Game>(L,1);
    lua_pushfstring(L,"%f, %f, %f",v->x,v->y,v->z); return 1;
}
static int v3_add(lua_State* L){
    auto* a=check<Vector3Game>(L,1); auto* b=check<Vector3Game>(L,2);
    push(L, *a + *b); return 1;
}
static int v3_sub(lua_State* L){
    auto* a=check<Vector3Game>(L,1); auto* b=check<Vector3Game>(L,2);
    push(L, *a - *b); return 1;
}
static int v3_mul(lua_State* L){ // scalar * v OR v * scalar
    if (lua_isnumber(L,1)) {
        float s=(float)lua_tonumber(L,1); auto* v=check<Vector3Game>(L,2);
        push(L, *v * s);
    } else {
        auto* v=check<Vector3Game>(L,1); float s=(float)luaL_checknumber(L,2);
        push(L, *v * s);
    }
    return 1;
}
static int v3_div(lua_State* L){ // v / scalar
    auto* v=check<Vector3Game>(L,1); float s=(float)luaL_checknumber(L,2);
    if (s == 0.0f) {
        // --- FIX WAS HERE ---
        luaL_error(L, "division by zero");
        return 0; 
    }
    push(L, *v / s); return 1;
}
static int v3_unm(lua_State* L){ // -v
    auto* v=check<Vector3Game>(L,1);
    push(L, -(*v)); return 1;
}

// --- Methods ---
static int v3_dot(lua_State* L) {
    auto* a=check<Vector3Game>(L,1); auto* b=check<Vector3Game>(L,2);
    lua_pushnumber(L, a->dot(*b));
    return 1;
}
static int v3_cross(lua_State* L) {
    auto* a=check<Vector3Game>(L,1); auto* b=check<Vector3Game>(L,2);
    push(L, a->cross(*b));
    return 1;
}
static int v3_lerp(lua_State* L) {
    auto* a=check<Vector3Game>(L,1);
    auto* b=check<Vector3Game>(L,2);
    float alpha = (float)luaL_checknumber(L,3);
    push(L, a->lerp(*b, alpha));
    return 1;
}

// --- __index for properties and methods ---
static int v3_index(lua_State* L) {
    auto* v = check<Vector3Game>(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "X") == 0) lua_pushnumber(L, v->x);
    else if (strcmp(key, "Y") == 0) lua_pushnumber(L, v->y);
    else if (strcmp(key, "Z") == 0) lua_pushnumber(L, v->z);
    else if (strcmp(key, "Magnitude") == 0) lua_pushnumber(L, v->magnitude());
    else if (strcmp(key, "Unit") == 0) push(L, v->normalized());
    else {
        // Look for method in metatable
        luaL_getmetatable(L, Traits<Vector3Game>::MetaName());
        lua_getfield(L, -1, "__index"); // This should be the methods table
        lua_getfield(L, -1, key);
        if (lua_isnil(L, -1)) {
            // --- FIX WAS HERE ---
            luaL_error(L, "invalid member '%s' for Vector3", key);
            return 0;
        }
    }
    return 1;
}

static const luaL_Reg V3_METHODS[] = {
    {"Dot", v3_dot},
    {"Cross", v3_cross},
    {"Lerp", v3_lerp},
    {nullptr,nullptr}
};

static const luaL_Reg V3_META[] = {
    {"__tostring", v3_tostring},
    {"__add",      v3_add},
    {"__sub",      v3_sub},
    {"__mul",      v3_mul},
    {"__div",      v3_div},
    {"__unm",      v3_unm},
    {"__index",    v3_index},
    {nullptr,nullptr}
};

lua_CFunction Traits<Vector3Game>::Ctor() { return v3_new; }
const luaL_Reg* Traits<Vector3Game>::Methods() { return V3_METHODS; }
const luaL_Reg* Traits<Vector3Game>::MetaMethods() { return V3_META; }
const luaL_Reg* Traits<Vector3Game>::Statics() { return nullptr; }