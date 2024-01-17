#pragma once

#include "Global.h"
#include "light.h"
#include "camera.h"
#include "shader.h"
#include "particles.h"

void SSF_initial();

void ScreenSpaceFluids(GLFWwindow* window, Camera camera, unsigned int VAO);