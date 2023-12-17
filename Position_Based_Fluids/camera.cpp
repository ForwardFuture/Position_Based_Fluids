#include "camera.h"

const float PITCH = 0.0f;
const float YAW = 0.0f;
const float SPEED = 0.3f;
const float SENSITIVITY = 0.05f;
const float ZOOMMAX = 75.0f;

//NOTE: You must ensure x & y are not equal to 0
static float CalcAngle(float x, float y, int quadrant) {
	float Angle = 0.0f;

	switch (quadrant) {
	case 1:
		Angle = atan(y / x) / Pi * 180.0f;
		break;
	case 2:
		Angle = 180.0f + atan(y / x) / Pi * 180.0f;
		break;
	case 3:
		Angle = 180.0f + atan(y / x) / Pi * 180.0f;
		break;
	case 4:
		Angle = atan(y / x) / Pi * 180.0f;
		break;
	}
	return Angle;
}

static std::pair<float, float> Update(glm::vec3 front) {

	float pitch_new = asin(front.y) / Pi * 180.0f;
	if (pitch_new > 89.0f)pitch_new = 89.0f;
	if (pitch_new < -89.0f)pitch_new = -89.0f;

	float yaw_new = 0.0f;

	if (fabs(front.z) < eps) {
		if (front.x > 0)yaw_new = 90.0f;
		else yaw_new = -90.0f;
	}
	else if (fabs(front.x) < eps) {
		if (front.z > 0)yaw_new = 0.0f;
		else yaw_new = 180.0f;
	}
	else {
		int quadrant = 0;
		if (front.x > 0) {
			if (front.z > 0) quadrant = 1;
			else quadrant = 2;
		}
		else {
			if (front.z < 0)quadrant = 3;
			else quadrant = 4;
		}
		yaw_new = CalcAngle(front.z, front.x, quadrant);
	}

	return std::make_pair(pitch_new, yaw_new);
}

Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up) {
	Position = position;
	Front = glm::normalize(front);
	Up = glm::normalize(up);

	std::pair<float, float> result = Update(Front);
	pitch = result.first;
	yaw = result.second;
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

	float xoffset = lastX - xpos;
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
	front.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
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

glm::vec3 Camera::GetPosition()const {
	return Position;
}

glm::vec3 Camera::GetFront()const {
	return Front;
}