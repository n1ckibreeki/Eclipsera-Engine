// #include "Instance.h"
// #include <algorithm>

// // Optional: keep this include if you want warnings on unknown types.
// // Otherwise, remove LOGW usage below.
// #include "core/logging/Logging.h"

// namespace Bootstrap {

// Instance::Instance(std::string name, InstanceClass c)
//     : Name(std::move(name)), Class(c) {}

// Instance::~Instance() = default;

// std::shared_ptr<Instance> Instance::FindFirstChild(const std::string& name) const {
//     auto it = ChildrenByName.find(name);
//     return it == ChildrenByName.end() ? nullptr : it->second;
// }

// void Instance::SetParent(const std::shared_ptr<Instance>& parent) {
//     auto self = shared_from_this();

//     if (auto old = Parent.lock()) {
//         old->Children.erase(std::remove(old->Children.begin(), old->Children.end(), self), old->Children.end());
//         auto it = old->ChildrenByName.find(Name);
//         if (it != old->ChildrenByName.end() && it->second.get() == this) old->ChildrenByName.erase(it);
//         old->OnChildRemoved(self); // delegate cache maintenance to subclass
//     }

//     Parent = parent;

//     if (parent) {
//         parent->Children.push_back(self);
//         parent->ChildrenByName[Name] = self;
//         parent->OnChildAdded(self); // delegate cache maintenance to subclass
//     }
// }

// void Instance::Destroy() {
//     if (!Alive) return;
//     Alive = false;

//     if (auto p = Parent.lock()) {
//         auto self = shared_from_this();
//         p->OnChildRemoved(self);
//         p->Children.erase(std::remove(p->Children.begin(), p->Children.end(), self), p->Children.end());
//         auto it = p->ChildrenByName.find(Name);
//         if (it != p->ChildrenByName.end() && it->second.get() == this) p->ChildrenByName.erase(it);
//     }

//     for (auto& c : Children) if (c) c->Destroy();
//     Children.clear();
//     ChildrenByName.clear();
// }

// // ---------- type registry ----------
// static std::unordered_map<std::string, Instance::Creator>& Registry() {
//     static std::unordered_map<std::string, Instance::Creator> r;
//     return r;
// }

// void Instance::RegisterType(const std::string& type, Creator create) {
//     Registry()[type] = std::move(create);
// }

// std::shared_ptr<Instance> Instance::New(const std::string& type) {
//     auto it = Registry().find(type);
//     if (it == Registry().end()) {
// #ifdef LOGW
//         LOGW("Instance::New: unknown type '%s'", type.c_str());
// #endif
//         return nullptr;
//     }
//     return it->second();
// }

// } // namespace Bootstrap
