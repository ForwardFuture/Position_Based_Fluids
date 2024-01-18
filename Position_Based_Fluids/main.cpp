#include "Global.h"
#include "particles.h"
#include "light.h"
#include "shader.h"
#include "camera.h"
#include "KDTree.h"
#include "PBF.h"
#include "SSF.h"

//glm::vec3 MyPos = glm::vec3(halflen, halflen * 2.0f, halflen * 8.0f);
glm::vec3 MyPos = glm::vec3(0.0f, halflen * 3.0f, halflen * 8.0f);
Camera camera = Camera(MyPos, glm::vec3(0.0f) - MyPos);

std::vector<float> primitives;
std::vector<unsigned int> indices;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.ProcessMouse(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessScroll(xoffset, yoffset);
}

void WindowCheck(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void GenerateSphere() {

	for (int y = 0; y <= Y_SEGMENTS; y++)
	{
		for (int x = 0; x <= X_SEGMENTS; x++)
		{
			float xSegment = (float)x / (float)X_SEGMENTS * 2.0f;
			float ySegment = (float)y / (float)Y_SEGMENTS;

			float yPos = std::cos(ySegment * Pi);
			float nowradius = sqrt(1.0f - yPos * yPos);

			float xPos = nowradius * std::cos(xSegment * Pi);
			float zPos = nowradius * std::sin(xSegment * Pi);


			primitives.push_back(xPos);
			primitives.push_back(yPos);
			primitives.push_back(zPos);
		}
	}


	for (int j = 0; j < X_SEGMENTS; j++) {
		indices.push_back(j);
		indices.push_back(j + X_SEGMENTS + 1);
		indices.push_back(j + X_SEGMENTS + 2);
	}

	for (int j = tot_vertices_number - 1 - X_SEGMENTS; j < tot_vertices_number - 1; j++) {
		indices.push_back(j);
		indices.push_back(j - X_SEGMENTS - 1);
		indices.push_back(j - X_SEGMENTS);
	}

	for (int i = 0; i < Y_SEGMENTS - 2; i++) {
		for (int j = 0; j < X_SEGMENTS; j++) {

			int now_begin_num = (i + 1) * (X_SEGMENTS + 1);
			int next_begin_num = (i + 2) * (X_SEGMENTS + 1);

			indices.push_back(now_begin_num + j);
			indices.push_back(next_begin_num + j);
			indices.push_back(next_begin_num + j + 1);

			indices.push_back(now_begin_num + j);
			indices.push_back(next_begin_num + j + 1);
			indices.push_back(now_begin_num + j + 1);
		}
	}
}

int main() {
	//Initialize
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(Width, Height, "Position_Based_Fluids", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize glad" << std::endl;
		glfwTerminate();
		return -1;
	}

	//Setup
	glViewport(0, 0, Width, Height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//Primitives & Indices & Vertices
	GenerateSphere();

	float nowx = -halflen, nowy = -halflen, nowz = -halflen;
	for (int i = 0; i < NUM; i++) {
		particles[i] = Particles(glm::vec3(nowx, nowy + Yoffset, nowz), glm::vec3(0.0f), i);

		nowy += (fabs(nowz - halflen) < eps && fabs(nowx - halflen) < eps) ? (2.0f * distance) : 0;
		nowx += (fabs(nowz - halflen) < eps) ? (2.0f * distance) : 0;
		nowx = (nowx - halflen > eps) ? -halflen : nowx;
		nowz += (2.0f * distance);
		nowz = (nowz - halflen > eps) ? -halflen : nowz;
	}

	//VAO & VBO & EBO
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * primitives.size(), &primitives[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	PBF_initial();

	SSF_initial();

	//RenderingLoop
	while (!glfwWindowShouldClose(window)) {
		WindowCheck(window);

		//Position_Based_Fluids Simulation
		//PositionBasedFluids();

		//Screen_Space_Fluids Rendering
		ScreenSpaceFluids(window, camera, VAO);

		glfwPollEvents();
		camera.ProcessKeyboard(window);
	}

	glfwTerminate();

	return 0;
}