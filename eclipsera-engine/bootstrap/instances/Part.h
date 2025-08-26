#pragma once
#include "bootstrap/instances/BasePart.h"

struct Part : BasePart {
    Part(std::string name = "Part");
    ~Part() override;
};