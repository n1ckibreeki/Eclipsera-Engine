#pragma once
#include "LuaDatatypes.h"
#include <raylib.h>
#include <cmath> // For sqrtf

struct Vector3Game
{
    float x, y, z;

    constexpr Vector3Game() : x(0), y(0), z(0) {}
    constexpr Vector3Game(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    // Convert to Raylib's Vector3
    ::Vector3 toRay() const { return {x, y, z}; }
    // NEW: Convert from Raylib's Vector3
    static Vector3Game fromRay(const ::Vector3 &v) { return {v.x, v.y, v.z}; }

    // --- Operators ---
    Vector3Game operator+(const Vector3Game &v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3Game operator-(const Vector3Game &v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3Game operator-() const { return {-x, -y, -z}; }
    Vector3Game operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3Game operator/(float s) const { return {x / s, y / s, z / s}; }

    // --- Methods ---
    float magnitudeSquared() const { return x * x + y * y + z * z; }
    float magnitude() const { return sqrtf(magnitudeSquared()); }
    Vector3Game normalized() const
    {
        float mag = magnitude();
        if (mag > 0.00001f)
            return *this / mag;
        return {0, 0, 0};
    }
    Vector3Game abs() const
    {
        return {fabsf(x), fabsf(y), fabsf(z)};
    }
    Vector3Game ceil() const
    {
        return {ceilf(x), ceilf(y), ceilf(z)};
    }
    Vector3Game floor() const
    {
        return {floorf(x), floorf(y), floorf(z)};
    }
    Vector3Game sign() const
    {
        return {
            float((x > 0) - (x < 0)),
            float((y > 0) - (y < 0)),
            float((z > 0) - (z < 0))};
    }
    float angle(const Vector3Game &other, const Vector3Game *axis = nullptr) const
    {
        // Normalize input vectors
        Vector3Game u = normalized();
        Vector3Game w = other.normalized();

        // Manual clamp to [-1,1]
        float dot = u.dot(w);
        if (dot > 1.0f)
            dot = 1.0f;
        else if (dot < -1.0f)
            dot = -1.0f;

        float angle = std::acosf(dot); // angle in radians, [0, pi]=

        if (axis)
        {
            Vector3Game cross = u.cross(w);
            float sign = cross.dot(*axis);
            if (sign < 0.0f)
                angle = -angle;
        }

        return angle; // radians
    }
    Vector3Game cross(const Vector3Game &v) const
    {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
    float dot(const Vector3Game &v) const { return x * v.x + y * v.y + z * v.z; }
    bool fuzzyEq(const Vector3Game &b, float epsilon = 1e-5f) const
    {
        Vector3Game diff = *this - b;
        return diff.magnitude() <= epsilon;
    }
    Vector3Game lerp(const Vector3Game &goal, float alpha) const
    {
        return *this + (goal - *this) * alpha;
    }
    Vector3Game max(const Vector3Game &v) const
    {
        return { fmaxf(x, v.x), fmaxf(y, v.y), fmaxf(z, v.z) };
    }
    Vector3Game min(const Vector3Game &v) const
    {
        return { fminf(x, v.x), fminf(y, v.y), fminf(z, v.z) };
    }
};

// Traits specialization
namespace lb
{
    template <>
    struct Traits<Vector3Game>
    {
        static const char *MetaName() { return "Librebox.Vector3"; }
        static const char *GlobalName() { return "Vector3"; }
        static lua_CFunction Ctor();
        static const luaL_Reg *Methods();
        static const luaL_Reg *MetaMethods();
        static const luaL_Reg *Statics();
    };
} // namespace lb
