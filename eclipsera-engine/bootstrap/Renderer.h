#pragma once
#include <raylib.h>

void InitRenderer();
void ShutdownRenderer();
void RenderFrame(Camera3D& camera);