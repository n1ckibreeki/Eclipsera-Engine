#include "bootstrap/instances/Workspace.h"
#include "bootstrap/instances/Part.h"
#include "bootstrap/instances/CameraGame.h"
#include <algorithm>

Workspace::Workspace(std::string name)
    : Service(std::move(name), InstanceClass::Workspace) {
    OnDescendantAdded([this](const std::shared_ptr<Instance>& c){
        if (c->Class == InstanceClass::Part)
            parts.push_back(std::static_pointer_cast<Part>(c));
        else if (c->Class == InstanceClass::Camera && !camera)
            camera = std::static_pointer_cast<CameraGame>(c);
    });
    OnDescendantRemoved([this](const std::shared_ptr<Instance>& c){
        if (c->Class == InstanceClass::Part) {
            auto sp = std::static_pointer_cast<Part>(c);
            auto it = std::find(parts.begin(), parts.end(), sp);
            if (it != parts.end()) { *it = parts.back(); parts.pop_back(); }
        } else if (c->Class == InstanceClass::Camera) {
            if (camera && camera.get() == c.get()) camera.reset();
        }
    });
}
Workspace::~Workspace() = default;

static Instance::Registrar _reg_ws("Workspace", []{
    return std::make_shared<Workspace>("Workspace");
});