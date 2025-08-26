#pragma once
#include "bootstrap/services/Service.h"
#include <vector>
#include <memory>

struct Part;
struct CameraGame;

struct Workspace : Service {
    std::shared_ptr<CameraGame> camera;
    std::vector<std::shared_ptr<Part>> parts;

    explicit Workspace(std::string name = "Workspace");
    ~Workspace() override;
};