#include "bootstrap/instances/BasePart.h"
#include "core/logging/Logging.h"
#include <cstring>
#include <cmath>

static inline float rad2deg(float r){ return r * 57.29577951308232f; }
static inline float deg2rad(float d){ return d * 0.017453292519943295f; }

BasePart::BasePart(std::string name, InstanceClass cls)
    : Instance(std::move(name), cls), CF{} {
    LOGI("BasePart created '%s' (Transparency=%.2f, Color=%.2f,%.2f,%.2f)", 
         Name.c_str(), Transparency, Color.r, Color.g, Color.b);
}

BasePart::~BasePart() = default;

bool BasePart::LuaGet(lua_State* L, const char* key) const {
    if (std::strcmp(key, "CFrame") == 0) {
        lb::push(L, CF);
        return true;
    }
    if (std::strcmp(key, "Position") == 0) {
        lb::push(L, CF.p);
        return true;
    }
    if (std::strcmp(key, "Orientation") == 0) {
        float rx, ry, rz;
        CF.toEulerAnglesXYZ(rx, ry, rz);
        lb::push(L, Vector3Game{ rad2deg(rx), rad2deg(ry), rad2deg(rz) });
        return true;
    }
    if (std::strcmp(key, "Size") == 0) {
        lb::push(L, Vector3Game::fromRay(Size));
        return true;
    }
    if (std::strcmp(key, "Transparency") == 0) {
        lua_pushnumber(L, Transparency);
        return true;
    }
    if (std::strcmp(key, "Color") == 0) {
        lb::push(L, Color3{ Color.r, Color.g, Color.b });
        return true;
    }
    return false;
}

bool BasePart::LuaSet(lua_State* L, const char* key, int valueIndex) {
    if (std::strcmp(key, "CFrame") == 0) {
        const auto* cf = lb::check<CFrame>(L, valueIndex);
        CF = *cf;
        return true;
    }
    if (std::strcmp(key, "Position") == 0) {
        const auto* v = lb::check<Vector3Game>(L, valueIndex);
        CF.p = *v;
        return true;
    }
    if (std::strcmp(key, "Orientation") == 0) {
        const auto* vdeg = lb::check<Vector3Game>(L, valueIndex);
        CFrame rot = CFrame::fromEulerAnglesXYZ(
            deg2rad(vdeg->x), deg2rad(vdeg->y), deg2rad(vdeg->z));
        // replace rotation, keep translation
        for(int i=0;i<9;i++) CF.R[i] = rot.R[i];
        return true;
    }
    if (std::strcmp(key, "Size") == 0) {
        const auto* v = lb::check<Vector3Game>(L, valueIndex);
        Size = v->toRay();
        return true;
    }
    if (std::strcmp(key, "Transparency") == 0) {
        Transparency = (float)luaL_checknumber(L, valueIndex);
        return true;
    }
    if (std::strcmp(key, "Color") == 0) {
        const auto* c = lb::check<Color3>(L, valueIndex);
        Color = { c->r, c->g, c->b };
        return true;
    }
    return false;
}
