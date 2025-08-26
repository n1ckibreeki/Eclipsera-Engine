#pragma once
#include "LuaDatatypes.h"
#include "Vector3Game.h"
#include <cstdint>

class Random {
public:
    Random();
    explicit Random(uint64_t seed);

    int64_t NextInteger(int64_t min, int64_t max);
    double  NextNumber();
    double  NextNumber(double min, double max);
    Vector3Game NextUnitVector();
    void Shuffle(lua_State* L, int tblIndex);

private:
    uint64_t state_{0};
    uint64_t inc_{0};

    uint32_t pcg32();
    uint64_t pcg64();
    static uint64_t mix64(uint64_t x);
};

// ---------- Lua bindings ----------
namespace lb {
template<> struct Traits<Random> {
    static const char* MetaName()   { return "Librebox.Random"; }
    static const char* GlobalName() { return "Random"; }
    static lua_CFunction Ctor();
    static const luaL_Reg* Methods();
    static const luaL_Reg* MetaMethods();
    static const luaL_Reg* Statics() { return nullptr; }
};
} // namespace lb
