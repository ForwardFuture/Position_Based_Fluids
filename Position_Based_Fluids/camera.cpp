#include "camera.h"

const float PITCH = 0.0f;
const float YAW = 0.0f;
const float SPEED = 0.3f;
const float SENSITIVITY = 0.05f;
const float ZOOMMAX = 75.0f;

Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up) {
	Position = position;
	Front = glm::normalize(front);
	Up = glm::normalize(up);

	pitch = asin(Front.y) / Pi * 180.0f;
	if (pitch > 89.0f)pitch = 89.0f;
	if (pitch < -89.0f)pitch = -89.0f;

	if (fabs(Front.z) < eps) {
		if (Front.x > 0)yaw = 0.0f;
		else yaw = 180.0f;
	}
	else yaw = atan(Front.x / (-Front.z)) / Pi * 180.0f;
}

void Camera::ProcessKeyboard(GLFWwindow* window) {
	currentFrame = glfwGetTime();
	float deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	float cameraSpeed = Speed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		Position += cameraSpeed * Front;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Position -= cameraSpeed * Front;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		Position -= cameraSpeed * glm::normalize(glm::cross(Front, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		Position += cameraSpeed * glm::normalize(glm::cross(Front, Up));
	}
}

void Camera::ProcessMouse(double xpos, double ypos) {
	if (firstmouse) {
		lastX = xpos;
		lastY = ypos;
		firstmouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	xoffset *= Sensitivity;
	yoffset *= Sensitivity;

	pitch += yoffset;
	yaw += xoffset;

	if (pitch > 89.0f)pitch = 89.0f;
	if (pitch < -89.0f)pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = -cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	Front = glm::normalize(front);
}

void Camera::ProcessScroll(double xoffset, double yoffset) {
	fov -= yoffset;
	if (fov > ZOOMMAX)fov = ZOOMMAX;
	if (fov < 1.0f)fov = 1.0f;
}

glm::mat4 Camera::GetViewMatrix()const {
	return glm::lookAt(Position, Position + Front, Up);
}

float Camera::GetFOV()const {
	return fov;
}