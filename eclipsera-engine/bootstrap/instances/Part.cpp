#include "bootstrap/instances/Part.h"
#include "core/logging/Logging.h"

Part::Part(std::string name)
    : BasePart(std::move(name), InstanceClass::Part) {
    Size = {4.0f, 1.0f, 2.0f};
    LOGI("Part created '%s'", Name.c_str());
}

Part::~Part() = default;

static Instance::Registrar _reg_part("Part", [] {
    return std::make_shared<Part>("Part");
});