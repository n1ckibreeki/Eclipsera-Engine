#include "bootstrap/instances/CameraGame.h"
#include "core/logging/Logging.h"
#include <memory>

CameraGame::CameraGame(std::string name) : Instance(std::move(name), InstanceClass::Camera) {
    LOGI("CameraGame created");
}
CameraGame::~CameraGame() = default;

static Instance::Registrar _reg_cam("Camera", []{ return std::make_shared<CameraGame>("Camera"); });
