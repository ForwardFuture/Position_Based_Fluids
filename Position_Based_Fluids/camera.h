#pragma once

#include "Global.h"

extern const float PITCH;
extern const float YAW;
extern const float SPEED;
extern const float SENSITIVITY;
extern const float ZOOMMAX;

class Camera {
private:
	glm::vec3 Position, Front, Up;
	float pitch, yaw;
	float fov = 45.0f;

	float currentFrame = 0.0, lastFrame = 0.0;
	bool firstmouse = true;
	float lastX = Width / 2, lastY = Height / 2;

public:
	float Speed = SPEED;
	float Sensitivity = SENSITIVITY;

	Camera(glm::vec3 position = glm::vec3(0.0f), glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

	void ProcessKeyboard(GLFWwindow* window);
	void ProcessMouse(double xpos, double ypos);
	void ProcessScroll(double xoffset, double yoffset);

	glm::mat4 GetViewMatrix()const;
	float GetFOV()const;
};