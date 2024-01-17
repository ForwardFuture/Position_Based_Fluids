#include "PBF.h"

//deltaTime isn't necessarily consistent with frameTime
static const float deltaTime = 1.0f / 50.0f;
static float restRho = 0.0f;
static const int MaxIteration = 5;
static const int MaxNum = 3;
static const float Kernel_h = distance * 2.0f * MaxNum;
static const float damping = 0.65f;
static const float EPS = 1e-6;
static const float Tensile_k_for_particle = 0.01f;
static const float Tensile_k_for_wall = 0.006f;
static const float Vorticity_eps = 0.000005f;
static const float Viscosity_c = 0.000001f;

static glm::vec3 predictPos[NUM];
static glm::vec3 tmpPos[NUM];
static std::vector<int>neighbors[NUM];
static std::vector<glm::vec3>additional_neighbors[NUM];
static float lambda[NUM];
static glm::vec3 delta_p[NUM];
static float Min_t[NUM];
static glm::vec3 tmpVel[NUM];
static glm::vec3 w[NUM];
static glm::vec3 eta[NUM];

float dis(glm::vec3 a, glm::vec3 b) {
	return glm::length(a - b);
}

float Poly6_density(glm::vec3 dir, float h) {
	float len = glm::length(dir);
	if (len > h)return 0.0;
	return 315 / (64 * Pi * pow(h, 9)) * pow((h * h - len * len), 3);
}

glm::vec3 Spiky_gradient(glm::vec3 dir, float h) {
	float len = glm::length(dir);
	if (len > h || fabs(len) < eps)return glm::vec3(0.0f);
	return float(-45.0f / (Pi * pow(h, 6)) * pow((h - len), 2)) * dir / len;
}

void PBF_initial() {

	//Compute restRho
	restRho = 0.0f;
	for (int i = 0; i < 2 * MaxNum + 1; i++) {
		for (int j = 0; j < 2 * MaxNum + 1; j++) {
			for (int k = 0; k < 2 * MaxNum + 1; k++) {
				float nx = 2.0f * distance * (i - MaxNum);
				float ny = 2.0f * distance * (j - MaxNum);
				float nz = 2.0f * distance * (k - MaxNum);
				if (dis(glm::vec3(0.0f), glm::vec3(nx, ny, nz)) < Kernel_h) {
					restRho += Poly6_density(glm::vec3(0.0f) - glm::vec3(nx, ny, nz), Kernel_h);
				}
			}
		}
	}
	//restRho = 33000.0f;
}

static float Constraint(int id, float h) {
	float res = Poly6_density(glm::vec3(0.0f), h);
	for (int i = 0; i < neighbors[id].size(); i++)
		res += Poly6_density(predictPos[id] - predictPos[neighbors[id][i]], h);

	for (int i = 0; i < additional_neighbors[id].size(); i++)
		res += Poly6_density(predictPos[id] - additional_neighbors[id][i], h);

	return -1 + res / (restRho /* * V[id]*/);
}

static float Constraint_Gradient_Length(int id, float h) {
	float res = 0.0;
	for (int i = 0; i < neighbors[id].size(); i++) {
		float dirnorm = glm::length(Spiky_gradient(predictPos[neighbors[id][i]] - predictPos[id], h));
		dirnorm /= restRho;
		res += dirnorm * dirnorm;
	}

	glm::vec3 tmpVec = glm::vec3(0.0f);
	for (int i = 0; i < neighbors[id].size(); i++)
		tmpVec += Spiky_gradient(predictPos[id] - predictPos[neighbors[id][i]], h) / restRho;

	for (int i = 0; i < additional_neighbors[id].size(); i++)
		tmpVec += Spiky_gradient(predictPos[id] - additional_neighbors[id][i], h) / restRho;
	
	float tmpVec_norm = glm::length(tmpVec);
	res += tmpVec_norm * tmpVec_norm;

	return res;
}

static void Find_Virtual_Particle() {

	for (int id = 0; id < NUM; id++) {
		std::vector<glm::vec3>().swap(additional_neighbors[id]);

		float check_boundary = 0.0f;
		float delta_x = 0.0f, delta_y = 0.0f, delta_z = 0.0f;

		/*
		check_boundary = -Bound - (predictPos[id].y - Kernel_h);//-Y
		if (check_boundary > eps) {
			delta_x = floor(predictPos[id].x / (2.0f * distance)) * 2.0f * distance;
			delta_z = floor(predictPos[id].z / (2.0f * distance)) * 2.0f * distance;
			for (float ny = -Bound - distance; ny > -Bound - check_boundary; ny -= 2.0f * distance) {
				for (int i = 0; i < 2 * (MaxNum + 1) + 1; i++) {
					for (int j = 0; j < 2 * (MaxNum + 1) + 1; j++) {
						float nx = 2.0f * distance * (i - MaxNum - 1) + delta_x;
						float nz = 2.0f * distance * (j - MaxNum - 1) + delta_z;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = -Bound - (predictPos[id].x - Kernel_h);//-X
		if (check_boundary > eps) {
			delta_y = floor(predictPos[id].y / (2.0f * distance)) * 2.0f * distance;
			delta_z = floor(predictPos[id].z / (2.0f * distance)) * 2.0f * distance;
			for (float nx = -Bound - distance; nx > -Bound - check_boundary; nx -= 2.0f * distance) {
				for (int i = 0; i < 2 * (MaxNum + 1) + 1; i++) {
					for (int j = 0; j < 2 * (MaxNum + 1) + 1; j++) {
						float ny = 2.0f * distance * (i - MaxNum - 1) + delta_y;
						float nz = 2.0f * distance * (j - MaxNum - 1) + delta_z;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = predictPos[id].x + Kernel_h - Bound;//X
		if (check_boundary > eps) {
			delta_y = floor(predictPos[id].y / (2.0f * distance)) * 2.0f * distance;
			delta_z = floor(predictPos[id].z / (2.0f * distance)) * 2.0f * distance;
			for (float nx = Bound + distance; nx < Bound + check_boundary; nx += 2.0f * distance) {
				for (int i = 0; i < 2 * (MaxNum + 1) + 1; i++) {
					for (int j = 0; j < 2 * (MaxNum + 1) + 1; j++) {
						float ny = 2.0f * distance * (i - MaxNum - 1) + delta_y;
						float nz = 2.0f * distance * (j - MaxNum - 1) + delta_z;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = -Bound - (predictPos[id].z - Kernel_h);//-Z
		if (check_boundary > eps) {
			delta_x = floor(predictPos[id].x / (2.0f * distance)) * 2.0f * distance;
			delta_y = floor(predictPos[id].y / (2.0f * distance)) * 2.0f * distance;
			for (float nz = -Bound - distance; nz > -Bound - check_boundary; nz -= 2.0f * distance) {
				for (int i = 0; i < 2 * (MaxNum + 1) + 1; i++) {
					for (int j = 0; j < 2 * (MaxNum + 1) + 1; j++) {
						float nx = 2.0f * distance * (i - MaxNum - 1) + delta_x;
						float ny = 2.0f * distance * (j - MaxNum - 1) + delta_y;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = predictPos[id].z + Kernel_h - Bound;//Z
		if (check_boundary > eps) {
			delta_x = floor(predictPos[id].x / (2.0f * distance)) * 2.0f * distance;
			delta_y = floor(predictPos[id].y / (2.0f * distance)) * 2.0f * distance;
			for (float nz = Bound + distance; nz < Bound + check_boundary; nz += 2.0f * distance) {
				for (int i = 0; i < 2 * (MaxNum + 1) + 1; i++) {
					for (int j = 0; j < 2 * (MaxNum + 1) + 1; j++) {
						float nx = 2.0f * distance * (i - MaxNum - 1) + delta_x;
						float ny = 2.0f * distance * (j - MaxNum - 1) + delta_y;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}
		*/

		check_boundary = -Bound - (predictPos[id].y - Kernel_h);//-Y
		if (check_boundary > eps) {
			for (float ny = -Bound - distance; ny > -Bound - check_boundary; ny -= 2.0f * distance) {
				for (int i = 0; i < 2 * MaxNum + 1; i++) {
					for (int j = 0; j < 2 * MaxNum + 1; j++) {
						float nx = 2.0f * distance * (i - MaxNum) + predictPos[id].x;
						float nz = 2.0f * distance * (j - MaxNum) + predictPos[id].z;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = -Bound - (predictPos[id].x - Kernel_h);//-X
		if (check_boundary > eps) {
			for (float nx = -Bound - distance; nx > -Bound - check_boundary; nx -= 2.0f * distance) {
				for (int i = 0; i < 2 * MaxNum + 1; i++) {
					for (int j = 0; j < 2 * MaxNum + 1; j++) {
						float ny = 2.0f * distance * (i - MaxNum) + predictPos[id].y;
						float nz = 2.0f * distance * (j - MaxNum) + predictPos[id].z;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = predictPos[id].x + Kernel_h - Bound;//X
		if (check_boundary > eps) {
			for (float nx = Bound + distance; nx < Bound + check_boundary; nx += 2.0f * distance) {
				for (int i = 0; i < 2 * MaxNum + 1; i++) {
					for (int j = 0; j < 2 * MaxNum + 1; j++) {
						float ny = 2.0f * distance * (i - MaxNum) + predictPos[id].y;
						float nz = 2.0f * distance * (j - MaxNum) + predictPos[id].z;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = -Bound - (predictPos[id].z - Kernel_h);//-Z
		if (check_boundary > eps) {
			for (float nz = -Bound - distance; nz > -Bound - check_boundary; nz -= 2.0f * distance) {
				for (int i = 0; i < 2 * MaxNum + 1; i++) {
					for (int j = 0; j < 2 * MaxNum + 1; j++) {
						float nx = 2.0f * distance * (i - MaxNum) + predictPos[id].x;
						float ny = 2.0f * distance * (j - MaxNum) + predictPos[id].y;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}

		check_boundary = predictPos[id].z + Kernel_h - Bound;//Z
		if (check_boundary > eps) {
			for (float nz = Bound + distance; nz < Bound + check_boundary; nz += 2.0f * distance) {
				for (int i = 0; i < 2 * MaxNum + 1; i++) {
					for (int j = 0; j < 2 * MaxNum + 1; j++) {
						float nx = 2.0f * distance * (i - MaxNum) + predictPos[id].x;
						float ny = 2.0f * distance * (j - MaxNum) + predictPos[id].y;
						if (dis(glm::vec3(nx, ny, nz), predictPos[id]) > Kernel_h)continue;
						additional_neighbors[id].push_back(glm::vec3(nx, ny, nz));
					}
				}
			}
		}
	}
}

void PositionBasedFluids() {

	//Step 0: Store Position
	for (int i = 0; i < NUM; i++) {
		predictPos[i] = particles[i].Pos;
	}

	//Step 1: Apply external forces
	for (int i = 0; i < NUM; i++) {

		//Gravity
		particles[i].Vel += glm::vec3(0.0f, -10.0f, 0.0f) * deltaTime;

		//Bounding
		float hitx, hity, hitz;
		if (fabs(particles[i].Vel.x) > eps)
			hitx = std::max((Bound - predictPos[i].x) / particles[i].Vel.x, (-Bound - predictPos[i].x) / particles[i].Vel.x);
		else hitx = FLOAT_INF;
		if (particles[i].Vel.y < -eps)
			hity = (-Bound - predictPos[i].y) / particles[i].Vel.y;
		else hity = FLOAT_INF;
		if (fabs(particles[i].Vel.z) > eps)
			hitz = std::max((Bound - predictPos[i].z) / particles[i].Vel.z, (-Bound - predictPos[i].z) / particles[i].Vel.z);
		else hitz = FLOAT_INF;

		//The particles may bounce back
		float nowTime = 0.0f, hitTime = 0.0f;

		nowTime = deltaTime;
		hitTime = hitx;
		while (nowTime > eps) {
			if (nowTime < hitTime) {
				predictPos[i].x += particles[i].Vel.x * nowTime;
				nowTime = 0.0f;
			}
			else {
				predictPos[i].x += particles[i].Vel.x * hitTime;
				nowTime -= hitTime;
				particles[i].Vel.x *= -damping;
				hitTime = 2.0f * Bound / fabs(particles[i].Vel.x);
			}
		}

		nowTime = deltaTime;
		hitTime = hity;
		while (nowTime > eps) {
			if (nowTime < hitTime) {
				predictPos[i].y += particles[i].Vel.y * nowTime;
				nowTime = 0.0f;
			}
			else {
				predictPos[i].y += particles[i].Vel.y * hitTime;
				nowTime -= hitTime;
				particles[i].Vel.y *= -damping;
				hitTime = FLOAT_INF;
			}
		}

		nowTime = deltaTime;
		hitTime = hitz;
		while (nowTime > eps) {
			if (nowTime < hitTime) {
				predictPos[i].z += particles[i].Vel.z * nowTime;
				nowTime = 0.0f;
			}
			else {
				predictPos[i].z += particles[i].Vel.z * hitTime;
				nowTime -= hitTime;
				particles[i].Vel.z *= -damping;
				hitTime = 2.0f * Bound / fabs(particles[i].Vel.z);
			}
		}

		/*
		predictPos[i].x = predictPos[i].x + particles[i].Vel.x * std::min(hitx, deltaTime) - damping * (particles[i].Vel.x * std::max(deltaTime - hitx, 0.0f));
		predictPos[i].y = predictPos[i].y + particles[i].Vel.y * std::min(hity, deltaTime) - damping * (particles[i].Vel.y * std::max(deltaTime - hity, 0.0f));
		predictPos[i].z = predictPos[i].z + particles[i].Vel.z * std::min(hitz, deltaTime) - damping * (particles[i].Vel.z * std::max(deltaTime - hitz, 0.0f));
		*/
	}

	//Step 2: Find Neighboring Particles
	for (int i = 0; i < NUM; i++)tmpPos[i] = particles[i].Pos, particles[i].Pos = predictPos[i];
	KDTree::BuildKDTree();
	for (int i = 0; i < NUM; i++)particles[i].Pos = tmpPos[i];
	for (int i = 0; i < NUM; i++) {
		std::vector<int>().swap(neighbors[i]);
		//¡ù¡ù¡ùBe cautious to the set of the distance of kernel function
		KDTree::FindNeighbors(predictPos[i], Kernel_h);
		while (!KDTree::neighboring.empty()) {
			neighbors[i].push_back(KDTree::neighboring.front());
			KDTree::neighboring.pop();
		}
	}
	

	//Step 3: Position Update Iteration
	for (int NowIter = 1; NowIter <= MaxIteration; NowIter++) {

		//Operation 0: Find Additional Particles
		Find_Virtual_Particle();

		//Operation 1: Calculate lambda_i
		for (int i = 0; i < NUM; i++) {
			lambda[i] = -Constraint(i, Kernel_h) / (Constraint_Gradient_Length(i, Kernel_h) + EPS);
		}

		//Operation 2: Calculate delta_p_i & Detect and response to collision (including particles and boundaries)
		for (int i = 0; i < NUM; i++) {

			//Calculate delta_p_i
			delta_p[i] = glm::vec3(0.0f);
			for (int j = 0; j < neighbors[i].size(); j++) {
				//NOTE: particle itself will not contribute to the delta_p
				if (neighbors[i][j] == i)continue;
				float s_corr = -Tensile_k_for_particle * (float)pow(Poly6_density(predictPos[i] - predictPos[neighbors[i][j]], Kernel_h) / Poly6_density(glm::vec3(1.0f, 0.0f, 0.0f) * 0.2f * Kernel_h, Kernel_h), 4);
				delta_p[i] += (lambda[i] /*/ V[i]*/ + lambda[neighbors[i][j]] /*/ V[neighbors[i][j]]*/ + s_corr)
					* Spiky_gradient(predictPos[i] - predictPos[neighbors[i][j]], Kernel_h);
			}
			
			for (int j = 0; j < additional_neighbors[i].size(); j++) {
				float s_corr = -Tensile_k_for_wall * (float)pow(Poly6_density(predictPos[i] - additional_neighbors[i][j], Kernel_h) / Poly6_density(glm::vec3(1.0f, 0.0f, 0.0f) * 0.2f * Kernel_h, Kernel_h), 4);
				delta_p[i] += (lambda[i] + s_corr) * Spiky_gradient(predictPos[i] - additional_neighbors[i][j], Kernel_h);
			}

			delta_p[i] /= restRho;

			//Detect and response to collision (particles) (can this be optimized?)
			Min_t[i] = 1.0f;
			for (int j = 0; j < i; j++) {
				float A = (float)(pow(delta_p[i].x - delta_p[j].x, 2) + pow(delta_p[i].y - delta_p[j].y, 2) + pow(delta_p[i].z - delta_p[j].z, 2));
				float B = 2 * ((predictPos[i].x - predictPos[j].x) * (delta_p[i].x - delta_p[j].x) +
					(predictPos[i].y - predictPos[j].y) * (delta_p[i].y - delta_p[j].y) +
					(predictPos[i].z - predictPos[j].z) * (delta_p[i].z - delta_p[j].z));
				float C = (float)(pow(predictPos[i].x - predictPos[j].x, 2) + pow(predictPos[i].y - predictPos[j].y, 2) + pow(predictPos[i].z - predictPos[j].z, 2));
				C -= (2.0f * distance) * (2.0f * distance);

				float DELTA = B * B - 4.0f * A * C;
				if (DELTA < eps)continue;
				float x1 = (-B - sqrt(DELTA)) / (2.0f * A), x2 = (-B + sqrt(DELTA)) / (2.0f * A);
				if (x1 < eps)x1 = FLOAT_INF;
				if (x2 < eps)x2 = FLOAT_INF;

				Min_t[i] = std::min(Min_t[i], std::min(x1, x2));
				Min_t[j] = std::min(Min_t[j], std::min(x1, x2));
			}


			//Detect and response to collision (boundaries)
			float hx, hy, hz;

			if (fabs(delta_p[i].x) > eps)
				hx = std::max((Bound - predictPos[i].x) / delta_p[i].x, (-Bound - predictPos[i].x) / delta_p[i].x);
			else hx = FLOAT_INF;
			if (delta_p[i].y < -eps)
				hy = (-Bound - predictPos[i].y) / delta_p[i].y;
			else hy = FLOAT_INF;
			if (fabs(delta_p[i].z) > eps)
				hz = std::max((Bound - predictPos[i].z) / delta_p[i].z, (-Bound - predictPos[i].z) / delta_p[i].z);
			else hz = FLOAT_INF;

			if (hx < eps)hx = FLOAT_INF;
			if (hy < eps)hy = FLOAT_INF;
			if (hz < eps)hz = FLOAT_INF;

			Min_t[i] = std::min(Min_t[i], hx);
			Min_t[i] = std::min(Min_t[i], hy);
			Min_t[i] = std::min(Min_t[i], hz);
		}

		//Operation 3: Update positions of particles
		for (int i = 0; i < NUM; i++) {
			delta_p[i] *= Min_t[i];
			predictPos[i] += delta_p[i];
		}
	}

	//Step 4: Update Velocity
	for (int i = 0; i < NUM; i++) {
		particles[i].Vel = (predictPos[i] - particles[i].Pos) / deltaTime;
	}

	//Step 5: Find Additional_Neighboring Particles
	Find_Virtual_Particle();

	//Step 6: Store Velocity
	for (int i = 0; i < NUM; i++) {
		tmpVel[i] = particles[i].Vel;
	}

	//Step 7: Apply Vorticity Confinement
	for (int i = 0; i < NUM; i++) {
		w[i] = glm::vec3(0.0f);
		for (int j = 0; j < neighbors[i].size(); j++) {
			w[i] += glm::cross((tmpVel[neighbors[i][j]] - tmpVel[i]),
				Spiky_gradient(predictPos[neighbors[i][j]] - predictPos[i], Kernel_h));
		}
		for (int j = 0; j < additional_neighbors[i].size(); j++) {
			w[i] += glm::cross((-tmpVel[i]), Spiky_gradient(additional_neighbors[i][j] - predictPos[i], Kernel_h));
		}
	}
	for (int i = 0; i < NUM; i++) {
		eta[i] = glm::vec3(0.0f);
		for (int j = 0; j < neighbors[i].size(); j++) {
			eta[i] += glm::length(w[neighbors[i][j]]) * Spiky_gradient(predictPos[i] - predictPos[neighbors[i][j]], Kernel_h);
		}
		if (glm::length(eta[i]) < eps)continue;
		glm::vec3 N = eta[i] / glm::length(eta[i]);
		particles[i].Vel += deltaTime * Vorticity_eps * (glm::cross(N, w[i]));
	}

	//Step 8: Store Velocity
	for (int i = 0; i < NUM; i++) {
		tmpVel[i] = particles[i].Vel;
	}

	//Step 9: Apply XSPH Viscosity
	for (int i = 0; i < NUM; i++) {

		for (int j = 0; j < neighbors[i].size(); j++) {
			if (neighbors[i][j] == i)continue;
			particles[i].Vel += Viscosity_c * (tmpVel[neighbors[i][j]] - tmpVel[i]) *
				Poly6_density(predictPos[i] - predictPos[neighbors[i][j]], Kernel_h);
		}
		for (int j = 0; j < additional_neighbors[i].size(); j++) {
			particles[i].Vel += Viscosity_c * (-tmpVel[i]) *
				Poly6_density(predictPos[i] - additional_neighbors[i][j], Kernel_h);
		}
	}

	//Step 10: Update Position
	for (int i = 0; i < NUM; i++) {
		particles[i].Pos = predictPos[i];
	}

	/*
	for (int i = 0; i < NUM; i++) {
		if (particles[i].Pos.y > 1.0f) {
			std::cout << i << " " << particles[i].Pos.y << std::endl;
		}
	}
	*/
}