#include "Global.h"
#include "particles.h"
#include "shader.h"
#include "camera.h"
#include "KDTree.h"
#include "PBF.h"
#include "SSF.h"

//glm::vec3 MyPos = glm::vec3(halflen, halflen * 2.0f, halflen * 8.0f);
glm::vec3 MyPos = glm::vec3(0.0f, halflen * 4.0f, halflen * 8.0f);
Camera camera = Camera(MyPos, glm::vec3(0.0f) - MyPos);

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
	float primitives[] = {
		0.0f, radius, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, radius, 0.0f, 0.0f, 1.0f,
		radius, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -radius, 0.0f, 0.0f, -1.0f,
		-radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 1,
		5, 2, 1,
		5, 3, 2,
		5, 4, 3,
		5, 1, 4
	};

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(primitives), primitives, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	PBF_initial();

	SSF_initial();

	//RenderingLoop
	while (!glfwWindowShouldClose(window)) {
		WindowCheck(window);

		//Position_Based_Fluids Simulation
		PositionBasedFluids();

		//Screen_Space_Fluids Rendering
		ScreenSpaceFluids(window, camera, VAO);

		glfwPollEvents();
		camera.ProcessKeyboard(window);
	}

	glfwTerminate();

	return 0;
}