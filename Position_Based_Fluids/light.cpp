#include "light.h"

Light light[5] = { Light(glm::vec3(0.0f, halflen * 3.0f + Yoffset, 0.0f)),
Light(glm::vec3(halflen * 5.0f, -halflen * 0.5f, 0.0f)), Light(glm::vec3(-halflen * 5.0f, -halflen * 0.5f, 0.0f)),
Light(glm::vec3(0.0f, -halflen * 0.5f, halflen * 5.0f)), Light(glm::vec3(0.0f, -halflen * 0.5f, -halflen * 5.0f)) };

//Represent lights on Y-axis, Positive X-axis, Negative X-axis, Positive Z-axis, Negative Z-axis