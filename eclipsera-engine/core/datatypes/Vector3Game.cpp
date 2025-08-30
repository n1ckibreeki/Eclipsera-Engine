#include "Vector3Game.h"
#include <cmath>
#include <cstring>
using namespace lb;

// --- Constructor ---
static int v3_new(lua_State *L)
{
    float x = (float)luaL_optnumber(L, 1, 0);
    float y = (float)luaL_optnumber(L, 2, 0);
    float z = (float)luaL_optnumber(L, 3, 0);
    push(L, Vector3Game{x, y, z});
    return 1;
}

// --- Metamethods ---
static int v3_tostring(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    lua_pushfstring(L, "%f, %f, %f", v->x, v->y, v->z);
    return 1;
}
static int v3_add(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    push(L, *a + *b);
    return 1;
}
static int v3_sub(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    push(L, *a - *b);
    return 1;
}
static int v3_mul(lua_State *L)
{
    if (lua_isnumber(L, 1))
    {
        float s = (float)lua_tonumber(L, 1);
        auto *v = check<Vector3Game>(L, 2);
        push(L, *v * s);
    }
    else
    {
        auto *v = check<Vector3Game>(L, 1);
        float s = (float)luaL_checknumber(L, 2);
        push(L, *v * s);
    }
    return 1;
}
static int v3_div(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    float s = (float)luaL_checknumber(L, 2);
    push(L, *v / s);
    return 1;
}
static int v3_unm(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    push(L, -(*v));
    return 1;
}

// --- Methods ---
static int v3_abs(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    push(L, v->abs());
    return 1;
}
static int v3_ceil(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    push(L, v->ceil());
    return 1;
}
static int v3_floor(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    push(L, v->floor());
    return 1;
}
static int v3_sign(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    push(L, v->sign());
    return 1;
}
static int v3_cross(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    push(L, a->cross(*b));
    return 1;
}
static int v3_angle(lua_State* L){
    auto* a = check<Vector3Game>(L, 1);
    auto* b = check<Vector3Game>(L, 2);

    if(lua_gettop(L) >= 3){
        auto axis = check<Vector3Game>(L, 3);
        float ang = a->angle(*b, axis);
        lua_pushnumber(L, ang);
        return 1;
    } else {
        float ang = a->angle(*b);
        lua_pushnumber(L, ang);
        return 1;
    }
}
static int v3_dot(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    lua_pushnumber(L, a->dot(*b));
    return 1;
}
static int v3_fuzzyeq(lua_State* L){
    auto* a = check<Vector3Game>(L,1);
    auto* b = check<Vector3Game>(L,2);
    float epsilon = 1e-5f;
    if(lua_gettop(L) >= 3) epsilon = (float)luaL_checknumber(L,3);
    lua_pushboolean(L, a->fuzzyEq(*b, epsilon));
    return 1;
}
static int v3_lerp(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    float alpha = (float)luaL_checknumber(L, 3);
    push(L, a->lerp(*b, alpha));
    return 1;
}
static int v3_max(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    push(L, a->max(*b));
    return 1;
}
static int v3_min(lua_State *L)
{
    auto *a = check<Vector3Game>(L, 1);
    auto *b = check<Vector3Game>(L, 2);
    push(L, a->min(*b));
    return 1;
}

// --- __index for properties + methods ---
static int v3_index(lua_State *L)
{
    auto *v = check<Vector3Game>(L, 1);
    const char *key = luaL_checkstring(L, 2);

    // --- Properties ---
    if (strcmp(key, "X") == 0)
    {
        lua_pushnumber(L, v->x);
        return 1;
    }
    if (strcmp(key, "Y") == 0)
    {
        lua_pushnumber(L, v->y);
        return 1;
    }
    if (strcmp(key, "Z") == 0)
    {
        lua_pushnumber(L, v->z);
        return 1;
    }
    if (strcmp(key, "Magnitude") == 0)
    {
        lua_pushnumber(L, v->magnitude());
        return 1;
    }
    if (strcmp(key, "Unit") == 0)
    {
        push(L, v->normalized());
        return 1;
    }

    // --- Fallback to methods table ---
    luaL_getmetatable(L, Traits<Vector3Game>::MetaName());
    lua_getfield(L, -1, "__methods"); // methods table
    lua_getfield(L, -1, key);
    return 1;
}

// --- Methods table ---
static const luaL_Reg V3_METHODS[] = {
    {"Abs", v3_abs},
    {"Ceil", v3_ceil},
    {"Floor", v3_floor},
    {"Sign", v3_sign},
    {"Cross", v3_cross},
    {"Angle", v3_angle},
    {"Dot", v3_dot},
    {"FuzzyEq", v3_fuzzyeq},
    {"Lerp", v3_lerp},
    {"Max", v3_max},
    {"Min", v3_min},
    {nullptr, nullptr}};

// --- Metatable table ---
static const luaL_Reg V3_META[] = {
    {"__tostring", v3_tostring},
    {"__add", v3_add},
    {"__sub", v3_sub},
    {"__mul", v3_mul},
    {"__div", v3_div},
    {"__unm", v3_unm},
    {"__index", v3_index},
    {nullptr, nullptr}};

// --- Traits hookup ---
lua_CFunction Traits<Vector3Game>::Ctor() { return v3_new; }
const luaL_Reg *Traits<Vector3Game>::Methods() { return V3_METHODS; }
const luaL_Reg *Traits<Vector3Game>::MetaMethods() { return V3_META; }
const luaL_Reg *Traits<Vector3Game>::Statics() { return nullptr; }