#pragma once
#include "LuaDatatypes.h"
#include <raylib.h>
#include <cmath> // For sqrtf

struct Vector3Game {
    float x, y, z;

    constexpr Vector3Game(): x(0),y(0),z(0) {}
    constexpr Vector3Game(float X,float Y,float Z): x(X),y(Y),z(Z) {}
    
    // Convert to Raylib's Vector3
    ::Vector3 toRay() const { return {x,y,z}; }
    // NEW: Convert from Raylib's Vector3
    static Vector3Game fromRay(const ::Vector3& v) { return {v.x, v.y, v.z}; }

    // --- Operators ---
    Vector3Game operator+(const Vector3Game& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3Game operator-(const Vector3Game& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3Game operator-() const { return {-x, -y, -z}; }
    Vector3Game operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3Game operator/(float s) const { return {x / s, y / s, z / s}; }

    // --- Methods ---
    float magnitudeSquared() const { return x*x + y*y + z*z; }
    float magnitude() const { return sqrtf(magnitudeSquared()); }
    Vector3Game normalized() const {
        float mag = magnitude();
        if (mag > 0.00001f) return *this / mag;
        return {0, 0, 0};
    }
    float dot(const Vector3Game& v) const { return x*v.x + y*v.y + z*v.z; }
    Vector3Game cross(const Vector3Game& v) const {
        return { y * v.z - z * v.y,  z * v.x - x * v.z,  x * v.y - y * v.x };
    }
    Vector3Game lerp(const Vector3Game& goal, float alpha) const {
        return *this + (goal - *this) * alpha;
    }
};

// Traits specialization
namespace lb {
template<> struct Traits<Vector3Game> {
    static const char* MetaName()    { return "Librebox.Vector3"; }
    static const char* GlobalName()  { return "Vector3"; }
    static lua_CFunction Ctor();
    static const luaL_Reg* Methods();
    static const luaL_Reg* MetaMethods();
    static const luaL_Reg* Statics();
};
} // namespace lb
