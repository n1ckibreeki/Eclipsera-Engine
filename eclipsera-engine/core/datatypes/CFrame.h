#pragma once
#include "LuaDatatypes.h"
#include "Vector3Game.h"

struct CFrame {
    // 3x3 rotation (row-major) + translation p
    // Row 0: (R[0], R[1], R[2])
    // Row 1: (R[3], R[4], R[5])
    // Row 2: (R[6], R[7], R[8])
    float R[9];
    Vector3Game p;

    // --- Ctors ---
    CFrame();
    explicit CFrame(const Vector3Game& pos);
    CFrame(float r00,float r01,float r02,
           float r10,float r11,float r12,
           float r20,float r21,float r22,
           const Vector3Game& pos);

    // --- Operators ---
    CFrame operator*(const CFrame& other) const;       // compose transforms
    Vector3Game operator*(const Vector3Game& v) const; // transform point (includes translation)
    CFrame operator+(const Vector3Game& v) const;      // translate by v

    // --- Core Methods ---
    CFrame inverse() const;
    CFrame lerp(const CFrame& goal, float alpha) const;
    void toEulerAnglesXYZ(float& rx, float& ry, float& rz) const; // radians
    void toEulerAnglesYXZ(float& rx, float& ry, float& rz) const; // radians
    void toAxisAngle(Vector3Game& axis, float& angle) const;      // axis unit, angle radians
    void toOrientation(float& rx, float& ry, float& rz) const;    // alias of XYZ for this engine

    // Space transforms
    CFrame toWorldSpace(const CFrame& cf) const { return (*this) * cf; }
    CFrame toObjectSpace(const CFrame& cf) const { return this->inverse() * cf; }
    Vector3Game pointToWorldSpace(const Vector3Game& v) const;    // R*v + p
    Vector3Game pointToObjectSpace(const Vector3Game& v) const;    // R^T*(v - p)
    Vector3Game vectorToWorldSpace(const Vector3Game& v) const;    // R*v
    Vector3Game vectorToObjectSpace(const Vector3Game& v) const;    // R^T*v

    // Basis vectors (engine convention)
    Vector3Game rightVector() const { return {R[0], R[3], R[6]}; }
    Vector3Game upVector()    const { return {R[1], R[4], R[7]}; }
    Vector3Game lookVector()  const { return {R[2], R[5], R[8]}; }

    // --- Static Constructors (radians) ---
    static CFrame Angles(float rx, float ry, float rz);           // rotation only
    static CFrame fromEulerAnglesXYZ(float rx, float ry, float rz);
    static CFrame fromEulerAnglesYXZ(float ry, float rx, float rz);
    static CFrame fromAxisAngle(const Vector3Game& axis, float angle);
    static CFrame fromOrientation(float rx, float ry, float rz);  // alias of XYZ here
    static CFrame fromMatrix(const Vector3Game& pos,
                             const Vector3Game& x,
                             const Vector3Game& y,
                             const Vector3Game& z);
    static CFrame lookAt(const Vector3Game& eye, const Vector3Game& target);

    // Utilities
    CFrame orthonormalized(float eps = 1e-6f) const;
};

namespace lb {
template<> struct Traits<CFrame> {
    static const char* MetaName()   { return "Librebox.CFrame"; }
    static const char* GlobalName() { return "CFrame"; }
    static lua_CFunction Ctor();
    static const luaL_Reg* Methods();
    static const luaL_Reg* MetaMethods();
    static const luaL_Reg* Statics();
};
}
