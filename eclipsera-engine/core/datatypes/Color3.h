#pragma once
#include "LuaDatatypes.h"
#include <raylib.h>
#include <string>
#include <cmath>

struct Color3 {
    float r, g, b; // 0..1

    constexpr Color3() : r(0), g(0), b(0) {}
    constexpr Color3(float R, float G, float B) : r(R), g(G), b(B) {}

    // Raylib interop
    ::Color toRay(unsigned char a = 255) const {
        auto clamp01 = [](float x){ return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };
        auto to8 = [&](float x){ return static_cast<unsigned char>(std::lround(clamp01(x) * 255.f)); };
        return ::Color{ to8(r), to8(g), to8(b), a };
    }
    static Color3 fromRay(const ::Color& c){
        const float inv = 1.0f / 255.0f;
        return { c.r * inv, c.g * inv, c.b * inv };
    }

    // Ops
    bool operator==(const Color3& o) const { return r==o.r && g==o.g && b==o.b; }

    // Methods
    Color3 lerp(const Color3& goal, float alpha) const {
        return { r + (goal.r - r) * alpha,
                 g + (goal.g - g) * alpha,
                 b + (goal.b - b) * alpha };
    }
    void toHSV(float& h, float& s, float& v) const;       //  Color3:ToHSV()
    std::string toHex() const;                             //  Color3:ToHex()

    // Constructors (statics)
    static Color3 fromRGB(int R, int G, int B);        //  Color3.fromRGB()
    static Color3 fromHSV(float h, float s, float v);  //  Color3.fromHSV()
    static Color3 fromHex(const std::string& hex);     //  Color3.fromHex()
};

namespace lb {
template<> struct Traits<Color3> {
    static const char* MetaName()   { return "Librebox.Color3"; }
    static const char* GlobalName() { return "Color3"; }
    static lua_CFunction Ctor();                // Color3.new(...)
    static const luaL_Reg* Methods();           // :Lerp, :ToHSV, :ToHex
    static const luaL_Reg* MetaMethods();       // __tostring, __index
    static const luaL_Reg* Statics();           // .fromRGB/.fromHSV/.fromHex
};
} // namespace lb
