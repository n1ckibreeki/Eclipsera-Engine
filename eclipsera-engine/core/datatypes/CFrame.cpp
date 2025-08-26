#include "CFrame.h"
#include <cmath>
#include <cstring>
#include <algorithm>

using namespace lb;

// ---------- helpers ----------
static inline float clampf(float x, float a, float b){ return x < a ? a : (x > b ? b : x); }
static inline float rsqrt(float x){ return 1.0f/std::sqrt(x); }

// ---------- C++ Implementation ----------
CFrame::CFrame(){ R[0]=R[4]=R[8]=1; R[1]=R[2]=R[3]=R[5]=R[6]=R[7]=0; p={0,0,0}; }
CFrame::CFrame(const Vector3Game& t): CFrame(){ p=t; }
CFrame::CFrame(float a,float b,float c,float d,float e,float f,float g,float h,float i,const Vector3Game& t){
    R[0]=a;R[1]=b;R[2]=c; R[3]=d;R[4]=e;R[5]=f; R[6]=g;R[7]=h;R[8]=i; p=t;
}

CFrame CFrame::operator*(const CFrame& B) const {
    CFrame C;
    C.R[0]=R[0]*B.R[0]+R[1]*B.R[3]+R[2]*B.R[6];
    C.R[1]=R[0]*B.R[1]+R[1]*B.R[4]+R[2]*B.R[7];
    C.R[2]=R[0]*B.R[2]+R[1]*B.R[5]+R[2]*B.R[8];
    C.R[3]=R[3]*B.R[0]+R[4]*B.R[3]+R[5]*B.R[6];
    C.R[4]=R[3]*B.R[1]+R[4]*B.R[4]+R[5]*B.R[7];
    C.R[5]=R[3]*B.R[2]+R[4]*B.R[5]+R[5]*B.R[8];
    C.R[6]=R[6]*B.R[0]+R[7]*B.R[3]+R[8]*B.R[6];
    C.R[7]=R[6]*B.R[1]+R[7]*B.R[4]+R[8]*B.R[7];
    C.R[8]=R[6]*B.R[2]+R[7]*B.R[5]+R[8]*B.R[8];
    C.p = (*this) * B.p;
    return C;
}

Vector3Game CFrame::operator*(const Vector3Game& v) const {
    return { R[0]*v.x + R[1]*v.y + R[2]*v.z + p.x,
             R[3]*v.x + R[4]*v.y + R[5]*v.z + p.y,
             R[6]*v.x + R[7]*v.y + R[8]*v.z + p.z };
}

CFrame CFrame::operator+(const Vector3Game& v) const {
    CFrame result = *this;
    result.p = result.p + v;
    return result;
}

CFrame CFrame::inverse() const {
    CFrame inv;
    // rotation inverse = transpose
    inv.R[0]=R[0]; inv.R[1]=R[3]; inv.R[2]=R[6];
    inv.R[3]=R[1]; inv.R[4]=R[4]; inv.R[5]=R[7];
    inv.R[6]=R[2]; inv.R[7]=R[5]; inv.R[8]=R[8];
    // translate by -R^T * p
    inv.p = { -(inv.R[0]*p.x + inv.R[1]*p.y + inv.R[2]*p.z),
              -(inv.R[3]*p.x + inv.R[4]*p.y + inv.R[5]*p.z),
              -(inv.R[6]*p.x + inv.R[7]*p.y + inv.R[8]*p.z) };
    return inv;
}

CFrame CFrame::lerp(const CFrame& goal, float alpha) const {
    Vector3Game new_p = p.lerp(goal.p, alpha);

    // Simple basis lerp + Gram-Schmidt
    Vector3Game rV = rightVector().lerp(goal.rightVector(), alpha).normalized();
    Vector3Game uV = upVector().lerp(goal.upVector(), alpha);
    Vector3Game lV = rV.cross(uV).normalized();
    uV = lV.cross(rV).normalized();

    CFrame result;
    result.p = new_p;
    result.R[0]=rV.x; result.R[1]=uV.x; result.R[2]=lV.x;
    result.R[3]=rV.y; result.R[4]=uV.y; result.R[5]=lV.y;
    result.R[6]=rV.z; result.R[7]=uV.z; result.R[8]=lV.z;
    return result;
}

CFrame CFrame::Angles(float rx, float ry, float rz) {
    return fromEulerAnglesXYZ(rx, ry, rz);
}

CFrame CFrame::fromEulerAnglesXYZ(float rx, float ry, float rz) {
    float cx = std::cos(rx), sx = std::sin(rx);
    float cy = std::cos(ry), sy = std::sin(ry);
    float cz = std::cos(rz), sz = std::sin(rz);
    CFrame m;
    // matches existing engine convention
    m.R[0] = cy*cz + sy*sx*sz; m.R[1] = cx*sz; m.R[2] = -sy*cz + cy*sx*sz;
    m.R[3] = -cy*sz + sy*sx*cz; m.R[4] = cx*cz; m.R[5] = sy*sz + cy*sx*cz;
    m.R[6] = sy*cx;             m.R[7] = -sx;   m.R[8] = cy*cx;
    m.p = {0,0,0};
    return m;
}

CFrame CFrame::fromEulerAnglesYXZ(float ry, float rx, float rz) {
    // Compose by multiplication to avoid deriving closed form
    CFrame Ry = fromEulerAnglesXYZ(0, ry, 0);
    CFrame Rx = fromEulerAnglesXYZ(rx, 0, 0);
    CFrame Rz = fromEulerAnglesXYZ(0, 0, rz);
    return Ry * Rx * Rz;
}

CFrame CFrame::fromAxisAngle(const Vector3Game& axisIn, float angle) {
    Vector3Game a = axisIn;
    float len2 = a.magnitudeSquared();
    if (len2 <= 1e-20f) return CFrame(); // identity on bad axis
    a = a * (1.0f/std::sqrt(len2));

    float c = std::cos(angle), s = std::sin(angle), t = 1.0f - c;
    float x=a.x, y=a.y, z=a.z;

    CFrame m;
    m.R[0] = t*x*x + c;     m.R[1] = t*x*y + s*z; m.R[2] = t*x*z - s*y;
    m.R[3] = t*x*y - s*z;   m.R[4] = t*y*y + c;   m.R[5] = t*y*z + s*x;
    m.R[6] = t*x*z + s*y;   m.R[7] = t*y*z - s*x; m.R[8] = t*z*z + c;
    m.p = {0,0,0};
    return m;
}

CFrame CFrame::fromOrientation(float rx, float ry, float rz) {
    // In this engine Orientation == XYZ (matches BasePart setter)
    return fromEulerAnglesXYZ(rx, ry, rz);
}

CFrame CFrame::fromMatrix(const Vector3Game& pos,
                          const Vector3Game& x,
                          const Vector3Game& y,
                          const Vector3Game& z) {
    // Expect right=x, up=y, look=z (engine's basis)
    CFrame m;
    m.p = pos;
    m.R[0]=x.x; m.R[1]=y.x; m.R[2]=z.x;
    m.R[3]=x.y; m.R[4]=y.y; m.R[5]=z.y;
    m.R[6]=x.z; m.R[7]=y.z; m.R[8]=z.z;
    return m.orthonormalized();
}

CFrame CFrame::lookAt(const Vector3Game& eye, const Vector3Game& target) {
    Vector3Game look = (target - eye).normalized();
    Vector3Game right = Vector3Game(0, 1, 0).cross(look).normalized();
    if (right.magnitudeSquared() < 1e-6f)
        right = Vector3Game(1, 0, 0).cross(look).normalized();
    Vector3Game up = look.cross(right);

    CFrame c;
    c.p = eye;
    c.R[0]=right.x; c.R[1]=up.x; c.R[2]=look.x;
    c.R[3]=right.y; c.R[4]=up.y; c.R[5]=look.y;
    c.R[6]=right.z; c.R[7]=up.z; c.R[8]=look.z;
    return c;
}

CFrame CFrame::orthonormalized(float eps) const {
    Vector3Game r = rightVector();
    Vector3Game u = upVector();
    Vector3Game l = r.cross(u);
    if (r.magnitudeSquared() < eps || u.magnitudeSquared() < eps || l.magnitudeSquared() < eps)
        return *this;
    r = r.normalized();
    l = l.normalized();
    u = l.cross(r).normalized();
    CFrame out = *this;
    out.R[0]=r.x; out.R[1]=u.x; out.R[2]=l.x;
    out.R[3]=r.y; out.R[4]=u.y; out.R[5]=l.y;
    out.R[6]=r.z; out.R[7]=u.z; out.R[8]=l.z;
    return out;
}

// Space transforms
Vector3Game CFrame::vectorToWorldSpace(const Vector3Game& v) const {
    return { R[0]*v.x + R[1]*v.y + R[2]*v.z,
             R[3]*v.x + R[4]*v.y + R[5]*v.z,
             R[6]*v.x + R[7]*v.y + R[8]*v.z };
}
Vector3Game CFrame::vectorToObjectSpace(const Vector3Game& v) const {
    // multiply by R^T
    return { R[0]*v.x + R[3]*v.y + R[6]*v.z,
             R[1]*v.x + R[4]*v.y + R[7]*v.z,
             R[2]*v.x + R[5]*v.y + R[8]*v.z };
}
Vector3Game CFrame::pointToWorldSpace(const Vector3Game& v) const {
    Vector3Game w = vectorToWorldSpace(v);
    return { w.x + p.x, w.y + p.y, w.z + p.z };
}
Vector3Game CFrame::pointToObjectSpace(const Vector3Game& v) const {
    Vector3Game d = v - p;
    return { R[0]*d.x + R[3]*d.y + R[6]*d.z,
             R[1]*d.x + R[4]*d.y + R[7]*d.z,
             R[2]*d.x + R[5]*d.y + R[8]*d.z };
}

// Decompositions (radians)
void CFrame::toEulerAnglesXYZ(float& rx, float& ry, float& rz) const {
    float sx = -R[7];
    float cx = std::sqrt(std::max(0.0f, 1.0f - sx*sx));
    rx = std::atan2(sx, cx);

    if (cx > 1e-6f) {
        float sy = R[6] / cx;
        float cy = R[8] / cx;
        float sz = R[1] / cx;
        float cz = R[4] / cx;
        ry = std::atan2(sy, cy);
        rz = std::atan2(sz, cz);
    } else {
        // gimbal lock
        ry = std::atan2(-R[2], R[0]);
        rz = 0.0f;
    }
}

void CFrame::toEulerAnglesYXZ(float& ry, float& rx, float& rz) const {
    // For order Y * X * Z derived in analysis
    float sx = R[5 - 3]; // R12 in row-major: index 5? Wait mapping: R[0..2] row0, [3..5] row1, [6..8] row2
    // R12 = element row 1 col 2 -> index 5:
    sx = R[5];
    rx = std::asin(clampf(sx, -1.0f, 1.0f));
    float cx = std::cos(rx);

    if (std::fabs(cx) > 1e-6f) {
        float sy = R[2] / cx;   // R02
        float cy = R[8] / cx;   // R22
        ry = std::atan2(sy, cy);

        float sz = -R[3] / cx;  // -R10
        float cz =  R[4] / cx;  //  R11
        rz = std::atan2(sz, cz);
    } else {
        // gimbal lock: choose rz = 0, solve ry from R01,R00
        rz = 0.0f;
        ry = std::atan2(R[1], R[0]);
    }
}

void CFrame::toAxisAngle(Vector3Game& axis, float& angle) const {
    float trace = R[0] + R[4] + R[8];
    float c = clampf((trace - 1.0f)*0.5f, -1.0f, 1.0f);
    angle = std::acos(c);

    if (angle < 1e-6f) { axis = {0,1,0}; angle = 0.0f; return; }
    if (std::fabs(PI - angle) < 1e-4f) {
        float xx = std::sqrt(std::max(0.0f, (R[0] + 1.0f)*0.5f));
        float yy = std::sqrt(std::max(0.0f, (R[4] + 1.0f)*0.5f));
        float zz = std::sqrt(std::max(0.0f, (R[8] + 1.0f)*0.5f));
        xx = std::copysign(xx, R[7] - R[5]);
        yy = std::copysign(yy, R[2] - R[6]);
        zz = std::copysign(zz, R[3] - R[1]);
        float len = std::sqrt(xx*xx + yy*yy + zz*zz);
        if (len < 1e-6f) { axis = {0,1,0}; angle = PI; return; }
        axis = { xx/len, yy/len, zz/len };
        return;
    }

    float s = 2.0f*std::sin(angle);
    axis = { (R[7]-R[5])/s, (R[2]-R[6])/s, (R[3]-R[1])/s };
    float ln = std::sqrt(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
    if (ln > 1e-6f) axis = axis * (1.0f/ln);
    else axis = {0,1,0};
}

void CFrame::toOrientation(float& rx, float& ry, float& rz) const {
    toEulerAnglesXYZ(rx, ry, rz);
}

// ---------- Lua bindings ----------

static int cf_new(lua_State* L){
    int n = lua_gettop(L);
    if (n == 0) { lb::push(L, CFrame{}); return 1; }

    if (n == 1 && luaL_testudata(L, 1, Traits<Vector3Game>::MetaName())) {
        const auto* v = lb::check<Vector3Game>(L, 1);
        lb::push(L, CFrame(*v)); return 1;
    }

    if (n == 2 && luaL_testudata(L, 1, Traits<Vector3Game>::MetaName()) && luaL_testudata(L, 2, Traits<Vector3Game>::MetaName())) {
        const auto* pos = lb::check<Vector3Game>(L, 1);
        const auto* lookAt = lb::check<Vector3Game>(L, 2);
        lb::push(L, CFrame::lookAt(*pos, *lookAt)); return 1;
    }

    if (n == 3) {
        float x=(float)luaL_checknumber(L,1), y=(float)luaL_checknumber(L,2), z=(float)luaL_checknumber(L,3);
        lb::push(L, CFrame{ Vector3Game{x,y,z} }); return 1;
    }

    if (n == 12) {
        float p_x=(float)luaL_checknumber(L,1), p_y=(float)luaL_checknumber(L,2), p_z=(float)luaL_checknumber(L,3);
        float r00=(float)luaL_checknumber(L,4), r01=(float)luaL_checknumber(L,5), r02=(float)luaL_checknumber(L,6);
        float r10=(float)luaL_checknumber(L,7), r11=(float)luaL_checknumber(L,8), r12=(float)luaL_checknumber(L,9);
        float r20=(float)luaL_checknumber(L,10), r21=(float)luaL_checknumber(L,11), r22=(float)luaL_checknumber(L,12);
        lb::push(L, CFrame{ r00,r01,r02,r10,r11,r12,r20,r21,r22, Vector3Game{p_x,p_y,p_z} });
        return 1;
    }

    luaL_error(L,"CFrame.new expects (), (Vector3), (x,y,z), (Vector3, Vector3 lookAt), or (12 numbers)");
    return 0;
}

static int cf_add(lua_State* L) {
    const auto* A = lb::check<CFrame>(L,1);
    const auto* B = lb::check<Vector3Game>(L,2);
    lb::push(L, *A + *B);
    return 1;
}

static int cf_mul(lua_State* L){
    const auto* A = lb::check<CFrame>(L,1);
    if (luaL_testudata(L, 2, Traits<CFrame>::MetaName())) {
        const auto* B = lb::check<CFrame>(L,2);
        lb::push(L, (*A) * (*B));
    } else if (luaL_testudata(L, 2, Traits<Vector3Game>::MetaName())) {
        const auto* B = lb::check<Vector3Game>(L,2);
        lb::push(L, (*A) * (*B));
    } else {
        luaL_error(L, "CFrame can only be multiplied by a CFrame or Vector3");
    }
    return 1;
}

static int cf_inverse(lua_State* L){
    const auto* A = lb::check<CFrame>(L,1);
    lb::push(L, A->inverse());
    return 1;
}
static int cf_inverse_alias(lua_State* L){ return cf_inverse(L); }

static int cf_lerp(lua_State* L) {
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* b = lb::check<CFrame>(L, 2);
    float alpha = (float)luaL_checknumber(L, 3);
    lb::push(L, a->lerp(*b, alpha));
    return 1;
}

static int cf_toworldspace(lua_State* L){
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* b = lb::check<CFrame>(L, 2);
    lb::push(L, a->toWorldSpace(*b));
    return 1;
}
static int cf_toobjectspace(lua_State* L){
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* b = lb::check<CFrame>(L, 2);
    lb::push(L, a->toObjectSpace(*b));
    return 1;
}

static int cf_pointtoworldspace(lua_State* L){
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* v = lb::check<Vector3Game>(L, 2);
    lb::push(L, a->pointToWorldSpace(*v));
    return 1;
}
static int cf_pointtoobjectspace(lua_State* L){
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* v = lb::check<Vector3Game>(L, 2);
    lb::push(L, a->pointToObjectSpace(*v));
    return 1;
}
static int cf_vectortoworldspace(lua_State* L){
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* v = lb::check<Vector3Game>(L, 2);
    lb::push(L, a->vectorToWorldSpace(*v));
    return 1;
}
static int cf_vectortoobjectspace(lua_State* L){
    const auto* a = lb::check<CFrame>(L, 1);
    const auto* v = lb::check<Vector3Game>(L, 2);
    lb::push(L, a->vectorToObjectSpace(*v));
    return 1;
}

static int cf_components(lua_State* L){
    const auto* c = lb::check<CFrame>(L,1);
    lua_pushnumber(L, c->p.x);
    lua_pushnumber(L, c->p.y);
    lua_pushnumber(L, c->p.z);
    lua_pushnumber(L, c->R[0]); lua_pushnumber(L, c->R[1]); lua_pushnumber(L, c->R[2]);
    lua_pushnumber(L, c->R[3]); lua_pushnumber(L, c->R[4]); lua_pushnumber(L, c->R[5]);
    lua_pushnumber(L, c->R[6]); lua_pushnumber(L, c->R[7]); lua_pushnumber(L, c->R[8]);
    return 12;
}

static int cf_toEulerXYZ(lua_State* L){
    const auto* c = lb::check<CFrame>(L,1);
    float rx, ry, rz; c->toEulerAnglesXYZ(rx, ry, rz);
    lua_pushnumber(L, rx); lua_pushnumber(L, ry); lua_pushnumber(L, rz);
    return 3;
}
static int cf_toEulerYXZ(lua_State* L){
    const auto* c = lb::check<CFrame>(L,1);
    float ry, rx, rz; c->toEulerAnglesYXZ(ry, rx, rz);
    lua_pushnumber(L, ry); lua_pushnumber(L, rx); lua_pushnumber(L, rz);
    return 3;
}
static int cf_toAxisAngle(lua_State* L){
    const auto* c = lb::check<CFrame>(L,1);
    Vector3Game axis; float angle;
    c->toAxisAngle(axis, angle);
    lb::push(L, axis);
    lua_pushnumber(L, angle);
    return 2;
}
static int cf_toOrientation(lua_State* L){
    const auto* c = lb::check<CFrame>(L,1);
    float rx, ry, rz; c->toOrientation(rx, ry, rz);
    lua_pushnumber(L, rx); lua_pushnumber(L, ry); lua_pushnumber(L, rz);
    return 3;
}

static int cf_tostring(lua_State* L){
    const auto* c = lb::check<CFrame>(L,1);
    lua_pushfstring(L,"%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f",
        c->p.x, c->p.y, c->p.z,
        c->R[0], c->R[1], c->R[2],
        c->R[3], c->R[4], c->R[5],
        c->R[6], c->R[7], c->R[8]);
    return 1;
}

static int cf_index(lua_State* L) {
    auto* cf = lb::check<CFrame>(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key,"Position")==0 || strcmp(key,"p")==0) { lb::push(L, cf->p); return 1; }
    if (strcmp(key,"XVector")==0 || strcmp(key,"RightVector")==0) { lb::push(L, Vector3Game{cf->R[0],cf->R[3],cf->R[6]}); return 1; }
    if (strcmp(key,"YVector")==0 || strcmp(key,"UpVector")==0)     { lb::push(L, Vector3Game{cf->R[1],cf->R[4],cf->R[7]}); return 1; }
    if (strcmp(key,"ZVector")==0)                                   { lb::push(L, Vector3Game{cf->R[2],cf->R[5],cf->R[8]}); return 1; }
    if (strcmp(key,"LookVector")==0)                                { lb::push(L, Vector3Game{-cf->R[2],-cf->R[5],-cf->R[8]}); return 1; }

    // fallback: methods table
    luaL_getmetatable(L, lb::Traits<CFrame>::MetaName()); // mt
    lua_getfield(L, -1, "__methods");                     // mt, methods
    lua_pushvalue(L, 2);                                  // mt, methods, key
    lua_rawget(L, -2);                                    // mt, methods, value
    if (lua_isfunction(L, -1)) {
        lua_remove(L, -2); // pop methods
        lua_remove(L, -2); // pop mt
        return 1;
    }
    lua_pop(L, 2); // pop methods, mt
    luaL_error(L, "invalid member '%s' for CFrame", key);
    return 0;
}

// ---- Statics ----
static int cf_s_angles(lua_State* L) {
    float rx = (float)luaL_checknumber(L, 1);
    float ry = (float)luaL_checknumber(L, 2);
    float rz = (float)luaL_checknumber(L, 3);
    push(L, CFrame::Angles(rx, ry, rz));
    return 1;
}
static int cf_s_fromEulerXYZ(lua_State* L){
    float rx = (float)luaL_checknumber(L, 1);
    float ry = (float)luaL_checknumber(L, 2);
    float rz = (float)luaL_checknumber(L, 3);
    push(L, CFrame::fromEulerAnglesXYZ(rx, ry, rz));
    return 1;
}
static int cf_s_fromEulerYXZ(lua_State* L){
    float ry = (float)luaL_checknumber(L, 1);
    float rx = (float)luaL_checknumber(L, 2);
    float rz = (float)luaL_checknumber(L, 3);
    push(L, CFrame::fromEulerAnglesYXZ(ry, rx, rz));
    return 1;
}
static int cf_s_fromAxisAngle(lua_State* L){
    const auto* axis = lb::check<Vector3Game>(L, 1);
    float angle = (float)luaL_checknumber(L, 2);
    push(L, CFrame::fromAxisAngle(*axis, angle));
    return 1;
}
static int cf_s_fromOrientation(lua_State* L){
    float rx = (float)luaL_checknumber(L, 1);
    float ry = (float)luaL_checknumber(L, 2);
    float rz = (float)luaL_checknumber(L, 3);
    push(L, CFrame::fromOrientation(rx, ry, rz));
    return 1;
}
static int cf_s_fromMatrix(lua_State* L){
    // fromMatrix(pos, x, y [, z])  if z missing, compute z = x:cross(y)
    const auto* pos = lb::check<Vector3Game>(L, 1);
    const auto* x   = lb::check<Vector3Game>(L, 2);
    const auto* y   = lb::check<Vector3Game>(L, 3);
    Vector3Game z;
    if (lua_gettop(L) >= 4 && luaL_testudata(L, 4, Traits<Vector3Game>::MetaName())) {
        z = *lb::check<Vector3Game>(L, 4);
    } else {
        z = (*x).cross(*y);
    }
    push(L, CFrame::fromMatrix(*pos, *x, *y, z));
    return 1;
}
static int cf_s_lookAt(lua_State* L){
    const auto* eye = lb::check<Vector3Game>(L, 1);
    const auto* tgt = lb::check<Vector3Game>(L, 2);
    push(L, CFrame::lookAt(*eye, *tgt));
    return 1;
}

// ---------- Lua registry tables ----------
static const luaL_Reg CF_METHODS[] = {
    {"Inverse", cf_inverse}, {"inverse", cf_inverse_alias},
    {"Lerp", cf_lerp},
    {"ToWorldSpace", cf_toworldspace},
    {"ToObjectSpace", cf_toobjectspace},
    {"PointToWorldSpace", cf_pointtoworldspace},
    {"PointToObjectSpace", cf_pointtoobjectspace},
    {"VectorToWorldSpace", cf_vectortoworldspace},
    {"VectorToObjectSpace", cf_vectortoobjectspace},
    {"ToEulerAnglesXYZ", cf_toEulerXYZ},
    {"ToEulerAnglesYXZ", cf_toEulerYXZ},
    {"ToAxisAngle", cf_toAxisAngle},
    {"ToOrientation", cf_toOrientation},
    {"components", cf_components},
    {"GetComponents", cf_components},
    {nullptr,nullptr}
};

static const luaL_Reg CF_META[] = {
    {"__add", cf_add},
    {"__mul", cf_mul},
    {"__tostring", cf_tostring},
    {"__index", cf_index},
    {nullptr,nullptr}
};

static const luaL_Reg CF_STATICS[] = {
    {"Angles", cf_s_angles},
    {"fromEulerAnglesXYZ", cf_s_fromEulerXYZ},
    {"fromEulerAnglesYXZ", cf_s_fromEulerYXZ},
    {"fromAxisAngle", cf_s_fromAxisAngle},
    {"fromOrientation", cf_s_fromOrientation},
    {"fromMatrix", cf_s_fromMatrix},
    {"lookAt", cf_s_lookAt},
    {nullptr, nullptr}
};

lua_CFunction Traits<CFrame>::Ctor() { return cf_new; }
const luaL_Reg* Traits<CFrame>::Methods() { return CF_METHODS; }
const luaL_Reg* Traits<CFrame>::MetaMethods() { return CF_META; }
const luaL_Reg* Traits<CFrame>::Statics() { return CF_STATICS; }
