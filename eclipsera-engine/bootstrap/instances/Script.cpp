// instances/Script.cpp
#include "bootstrap/instances/Script.h"
#include "core/logging/Logging.h"
#include "bootstrap/Instance.h"
#include <utility>

static Instance::Registrar _reg_script("Script", [] {
    return std::make_shared<Script>("Script", "");
});

Script::Script(std::string name)
    : BaseScript(std::move(name), InstanceClass::Script) {
    SetRunContext(RunContext::Server);
    LOGI("Script created '%s'", Name.c_str());
}

Script::Script(std::string name, std::string source)
    : Script(std::move(name)) {
    SetSource(std::move(source));
}

Script::~Script() { LOGI("~Script '%s'", Name.c_str()); }