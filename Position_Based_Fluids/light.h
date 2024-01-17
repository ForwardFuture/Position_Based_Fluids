#pragma once

#include "Global.h"

class Light {
public:
	glm::vec3 Position, Intensity;

	Light() {
		Position = glm::vec3(0.0f);
		Intensity = glm::vec3(1.0f);
	}
	Light(glm::vec3 P, glm::vec3 I = glm::vec3(1.0f)) :Position(P), Intensity(I) {}

};

extern Light light[5];