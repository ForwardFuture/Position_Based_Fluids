#pragma once

#include "Global.h"
#include "particles.h"
#include "KDTree.h"

float dis(glm::vec3 a, glm::vec3 b);
float Poly6_density(glm::vec3 dir, float h);
glm::vec3 Spiky_gradient(glm::vec3 dir, float h);

void PBF_initial();

void PositionBasedFluids();