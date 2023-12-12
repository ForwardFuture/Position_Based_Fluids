#pragma once

#include "Global.h"

class Shader {
private:
	unsigned int ID;
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	void UseShaderProgram();

	void setBool(const std::string name, bool value)const;
	void setInt(const std::string name, int value)const;
	void setFloat(const std::string names, float value)const;
	void setFloatArray(const std::string name, float* arr, int length)const;
	void setMatrix(const std::string name, glm::mat4 matrix)const;
};