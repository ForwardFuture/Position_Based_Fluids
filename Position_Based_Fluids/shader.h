#pragma once

#include "Global.h"

class Shader {
private:
	unsigned int ID;
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	void UseShaderProgram();

	void setInt(const std::string name, int value);
	void setMatrix(const std::string name, glm::mat4 matrix);
};