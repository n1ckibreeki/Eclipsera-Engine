#include "bootstrap/services/Service.h"
#include "bootstrap/Game.h"              // for g_game
#include "core/logging/Logging.h"

std::unordered_map<std::string, std::shared_ptr<Service>> Service::registry;

Service::Service(std::string name, InstanceClass cls)
    : Instance(std::move(name), cls) {
    // do not add to registry here; we do it when first accessed
}

Service::~Service() {
    registry.erase(Name);
}

std::shared_ptr<Service> Service::Get(const std::string& name) {
    auto it = registry.find(name);
    return it == registry.end() ? nullptr : it->second;
}

std::shared_ptr<Service> Service::Create(const std::string& name){
    if (auto s = Get(name)) return nullptr;               // or throw
    auto inst = Instance::New(name);
    auto svc  = std::dynamic_pointer_cast<Service>(inst);
    if (!svc) return nullptr;
    if (g_game) svc->SetParent(g_game);
    registry[name] = svc;
    LOGI("Service created '%s'", name.c_str());
    return svc;
}
