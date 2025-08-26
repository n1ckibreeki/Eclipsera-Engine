// #pragma once
// #include <memory>
// #include <string>
// #include <vector>
// #include <unordered_map>
// #include <functional>

// namespace Bootstrap {

// enum class InstanceClass { Game, Workspace, Part, Script, Folder, Camera, Unknown };

// struct Instance : std::enable_shared_from_this<Instance> {
//     std::string Name;
//     InstanceClass Class{ InstanceClass::Unknown };
//     std::weak_ptr<Instance> Parent;
//     std::vector<std::shared_ptr<Instance>> Children;
//     std::unordered_map<std::string, std::shared_ptr<Instance>> ChildrenByName;
//     bool Alive{ true };

//     explicit Instance(std::string name, InstanceClass c);
//     virtual ~Instance();

//     void SetParent(const std::shared_ptr<Instance>& parent);
//     virtual void Destroy();

//     std::shared_ptr<Instance> FindFirstChild(const std::string& name) const;

//     // Pluggable factory to avoid higher-level deps here.
//     using Creator = std::function<std::shared_ptr<Instance>()>;
//     static void RegisterType(const std::string& type, Creator create);
//     static std::shared_ptr<Instance> New(const std::string& type);

// protected:
//     // Hooks for subclasses (e.g., Workspace caches). No includes needed here.
//     virtual void OnChildAdded(const std::shared_ptr<Instance>&) {}
//     virtual void OnChildRemoved(const std::shared_ptr<Instance>&) {}
// };

// } // namespace Bootstrap
