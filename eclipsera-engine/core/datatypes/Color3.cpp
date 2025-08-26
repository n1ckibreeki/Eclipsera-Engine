#include "Color3.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>

using namespace lb;

// ---------- Color math ----------
static inline float clamp01(float x){ return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }

void Color3::toHSV(float& h, float& s, float& v) const {
    const float R = r, G = g, B = b;
    const float maxc = std::max(R, std::max(G, B));
    const float minc = std::min(R, std::min(G, B));
    v = maxc;
    const float delta = maxc - minc;
    if (maxc <= 0.f) { s = 0.f; h = 0.f; return; }
    s = delta / maxc;
    if (delta <= 1e-6f) { h = 0.f; return; }
    if (maxc == R)       h = (G - B) / delta + (G < B ? 6.f : 0.f);
    else if (maxc == G)  h = (B - R) / delta + 2.f;
    else                 h = (R - G) / delta + 4.f;
    h /= 6.f;
    if (h >= 1.f) h -= 1.f;
    if (h < 0.f)  h += 1.f;
}

Color3 Color3::fromHSV(float h, float s, float v) {
    h = h - std::floor(h);                 // wrap
    s = clamp01(s); v = clamp01(v);
    if (s <= 0.f) return { v, v, v };
    float H = h * 6.f;
    int   i = static_cast<int>(std::floor(H)) % 6;
    float f = H - std::floor(H);
    float p = v * (1.f - s);
    float q = v * (1.f - s * f);
    float t = v * (1.f - s * (1.f - f));
    switch (i) {
        case 0: return { v, t, p };
        case 1: return { q, v, p };
        case 2: return { p, v, t };
        case 3: return { p, q, v };
        case 4: return { t, p, v };
        default:return { v, p, q };
    }
}

Color3 Color3::fromRGB(int R, int G, int B) {
    auto clamp255 = [](int x){ return x < 0 ? 0 : (x > 255 ? 255 : x); };
    const float inv = 1.0f / 255.0f;
    return { clamp255(R) * inv, clamp255(G) * inv, clamp255(B) * inv };
}

std::string Color3::toHex() const {
    auto to8 = [&](float x){
        int v = static_cast<int>(std::lround(clamp01(x) * 255.f));
        if (v < 0) v = 0; else if (v > 255) v = 255;
        return v;
    };
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << to8(r)
        << std::setw(2) << std::setfill('0') << to8(g)
        << std::setw(2) << std::setfill('0') << to8(b);
    return oss.str(); // RRGGBB, no '#'
}

static int hexDigit(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

Color3 Color3::fromHex(const std::string& hexIn) {
    // Accept "#RRGGBB", "RRGGBB", "#RGB", or "RGB"
    std::string s;
    s.reserve(hexIn.size());
    for (char c : hexIn) if (c != '#') s.push_back(c);
    if (s.size() == 3) { // expand RGB -> RRGGBB
        std::string t;
        t.reserve(6);
        for (char c : s) { t.push_back(c); t.push_back(c); }
        s.swap(t);
    }
    if (s.size() != 6) return {0,0,0}; // fallback black
    int r = (hexDigit(s[0]) << 4) | hexDigit(s[1]);
    int g = (hexDigit(s[2]) << 4) | hexDigit(s[3]);
    int b = (hexDigit(s[4]) << 4) | hexDigit(s[5]);
    if (r < 0 || g < 0 || b < 0) return {0,0,0};
    return fromRGB(r, g, b);
}

// ---------- Lua bindings ----------

static int c3_new(lua_State* L){
    int n = lua_gettop(L);
    if (n == 0) { lb::push(L, Color3{}); return 1; }
    if (n == 3) {
        float R = (float)luaL_checknumber(L,1);
        float G = (float)luaL_checknumber(L,2);
        float B = (float)luaL_checknumber(L,3);
        lb::push(L, Color3{ R, G, B });
        return 1;
    }
    luaL_error(L, "Color3.new expects () or (r,g,b) with 0..1 floats");
    return 0;
}

static int c3_tostring(lua_State* L){
    auto* c = lb::check<Color3>(L,1);
    lua_pushfstring(L, "%f, %f, %f", c->r, c->g, c->b);
    return 1;
}

// Methods
static int c3_lerp(lua_State* L){
    auto* a = lb::check<Color3>(L,1);
    auto* b = lb::check<Color3>(L,2);
    float alpha = (float)luaL_checknumber(L,3);
    lb::push(L, a->lerp(*b, alpha));
    return 1;
}
static int c3_tohsv(lua_State* L){
    auto* c = lb::check<Color3>(L,1);
    float h,s,v; c->toHSV(h,s,v);
    lua_pushnumber(L, h);
    lua_pushnumber(L, s);
    lua_pushnumber(L, v);
    return 3;
}
static int c3_tohex(lua_State* L){
    auto* c = lb::check<Color3>(L,1);
    auto hex = c->toHex();
    lua_pushlstring(L, hex.c_str(), hex.size());
    return 1;
}

// __index for R,G,B and method fallback
static int c3_index(lua_State* L){
    auto* c = lb::check<Color3>(L,1);
    const char* key = luaL_checkstring(L,2);

    if (std::strcmp(key,"R")==0) { lua_pushnumber(L, c->r); return 1; }
    if (std::strcmp(key,"G")==0) { lua_pushnumber(L, c->g); return 1; }
    if (std::strcmp(key,"B")==0) { lua_pushnumber(L, c->b); return 1; }

    // Fallback to methods table
    luaL_getmetatable(L, Traits<Color3>::MetaName());
    lua_getfield(L, -1, "__index");            // methods table
    lua_getfield(L, -1, key);
    if (lua_isnil(L, -1)) {
        luaL_error(L, "invalid member '%s' for Color3", key);
        return 0;
    }
    return 1;
}

// Statics
static int c3_s_fromRGB(lua_State* L){
    int R = (int)luaL_checkinteger(L,1);
    int G = (int)luaL_checkinteger(L,2);
    int B = (int)luaL_checkinteger(L,3);
    lb::push(L, Color3::fromRGB(R,G,B));
    return 1;
}
static int c3_s_fromHSV(lua_State* L){
    float h = (float)luaL_checknumber(L,1);
    float s = (float)luaL_checknumber(L,2);
    float v = (float)luaL_checknumber(L,3);
    lb::push(L, Color3::fromHSV(h,s,v));
    return 1;
}
static int c3_s_fromHex(lua_State* L){
    size_t len = 0;
    const char* s = luaL_checklstring(L,1,&len);
    lb::push(L, Color3::fromHex(std::string{s, len}));
    return 1;
}

// Registration tables
static const luaL_Reg C3_METHODS[] = {
    {"Lerp",  c3_lerp},
    {"ToHSV", c3_tohsv},
    {"ToHex", c3_tohex},
    {nullptr,nullptr}
};
static const luaL_Reg C3_META[] = {
    {"__tostring", c3_tostring},
    {"__index",    c3_index},
    {nullptr,nullptr}
};
static const luaL_Reg C3_STATICS[] = {
    {"fromRGB", c3_s_fromRGB},
    {"fromHSV", c3_s_fromHSV},
    {"fromHex", c3_s_fromHex},
    {nullptr,nullptr}
};

lua_CFunction Traits<Color3>::Ctor() { return c3_new; }
const luaL_Reg* Traits<Color3>::Methods() { return C3_METHODS; }
const luaL_Reg* Traits<Color3>::MetaMethods() { return C3_META; }
const luaL_Reg* Traits<Color3>::Statics() { return C3_STATICS; }
