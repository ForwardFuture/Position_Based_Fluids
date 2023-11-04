#pragma once

#include "Global.h"

class Particles {
public:
	unsigned int id;
	glm::vec3 Pos, Vel;

	Particles() {
		id = 0;	Pos = Vel = glm::vec3(0.0f);
	}
	Particles(glm::vec3 Poss, glm::vec3 Vell, unsigned int idd) {
		Pos = Poss; Vel = Vell; id = idd;
	}
};

extern Particles particles[NUM];