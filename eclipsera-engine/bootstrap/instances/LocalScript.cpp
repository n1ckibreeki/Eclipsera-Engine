// instances/LocalScript.cpp
#include "bootstrap/instances/LocalScript.h"
#include "core/logging/Logging.h"
#include "bootstrap/Instance.h"
#include <utility>

static Instance::Registrar _reg_localscript("LocalScript", [] {
    return std::make_shared<LocalScript>("LocalScript");
});

LocalScript::LocalScript(std::string name)
    : BaseScript(std::move(name), InstanceClass::LocalScript) {
    SetRunContext(RunContext::Client);
    LOGI("LocalScript created '%s'", Name.c_str());
}

LocalScript::LocalScript(std::string name, std::string source)
    : LocalScript(std::move(name)) {
    SetSource(std::move(source));
}

LocalScript::~LocalScript() { LOGI("~LocalScript '%s'", Name.c_str()); }