#include "shader.h"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath) {
	std::string vShaderCode, fShaderCode;
	std::fstream vShaderFile, fShaderFile;
	vShaderFile.exceptions(std::fstream::badbit | std::fstream::failbit);
	fShaderFile.exceptions(std::fstream::badbit | std::fstream::failbit);

	try {
		vShaderFile.open(vertexShaderPath);
		fShaderFile.open(fragmentShaderPath);

		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vShaderCode = vShaderStream.str();
		fShaderCode = fShaderStream.str();
	}
	catch (std::fstream::failure e) {
		std::cout << "ERROR: SHADER_FILE NOT SUCCESSFULLY READ" << std::endl;
	}

	const char* vShaderHandle = vShaderCode.c_str();
	const char* fShaderHandle = fShaderCode.c_str();

	//Error Log
	int success;
	char errlog[512];

	//Set Vertex Shader
	unsigned int vertexshader;
	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexshader, 1, &vShaderHandle, NULL);
	glCompileShader(vertexshader);
	//Print Error Log
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexshader, 512, NULL, errlog);
		std::cout << "ERROR: VERTEX_SHADER COMPILATION FAILED\n" << errlog << std::endl;
	}

	//Set Fragment Shader
	unsigned int fragmentshader;
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentshader, 1, &fShaderHandle, NULL);
	glCompileShader(fragmentshader);
	//Print Error Log
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentshader, 512, NULL, errlog);
		std::cout << "ERROR: FRAGMENT_SHADER COMPILATION FAILED\n" << errlog << std::endl;
	}

	//Set Shader Program Object
	ID = glCreateProgram();
	glAttachShader(ID, vertexshader);
	glAttachShader(ID, fragmentshader);
	glLinkProgram(ID);
	//Print Error Log
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, errlog);
		std::cout << "ERROR: SHADER_PROGRAM LINKING FAILED\n" << errlog << std::endl;
	}

	//Delete Shader
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
}

void Shader::UseShaderProgram() {
	glUseProgram(ID);
}

void Shader::setInt(const std::string name, int value) {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string name, float value) {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloatArray(const std::string name, float* arr, int length) {
	glUniform1fv(glGetUniformLocation(ID, name.c_str()), length, arr);
}

void Shader::setMatrix(const std::string name, glm::mat4 matrix) {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}