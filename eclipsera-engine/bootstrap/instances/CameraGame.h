#pragma once
#include "bootstrap/Instance.h"

struct CameraGame : Instance {
    ::Vector3 Position{0.0f,10.0f,20.0f};
    ::Vector3 Target{0.0f,0.0f,0.0f};
    CameraGame(std::string name = "Camera");
    ~CameraGame() override;
};