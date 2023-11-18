#include "SSF.h"

static unsigned int FBO;
static unsigned int DepthTexture;
static unsigned int ThicknessTexture;

static glm::mat4 MVP, model, view, projection;

static void Draw(Camera camera, unsigned int VAO, Shader NowshaderProgram) {
	//Use shaderProgram
	NowshaderProgram.UseShaderProgram();

	//Bind VAO
	glBindVertexArray(VAO);

	glEnable(GL_DEPTH_TEST);

	//MVP matrix and rendering
	for (int i = 0; i < NUM; i++) {

		//model matrix
		model = glm::mat4(1.0f);
		model = glm::translate(model, particles[i].Pos);

		//view matrix
		view = glm::mat4(1.0f);
		view = camera.GetViewMatrix();

		//projection matrix
		projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.GetFOV()), Width / Height, 0.1f, 100.0f);

		MVP = projection * view * model;

		NowshaderProgram.setMatrix("MVP", MVP);

		glDrawElements(GL_TRIANGLES, 3 * 8, GL_UNSIGNED_INT, 0);
	}

	//Unbind VAO
	glBindVertexArray(0);
}

static void getDepthTexture(GLFWwindow* window, Camera camera, unsigned int VAO, Shader NowshaderProgram) {
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEPTH_TEST);
	glGenTextures(1, &DepthTexture);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Draw(camera, VAO, NowshaderProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
}

static void getThicknessTexture(Camera camera, unsigned int VAO, Shader NowshaderProgram) {

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glGenTextures(1, &ThicknessTexture);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ThicknessTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	Draw(camera, VAO, NowshaderProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
}

static void Rendering(GLFWwindow* window, Shader NowshaderProgram) {
	float TwoTriangles[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};
	unsigned int TwoTrianglesIndices[] = {
		0, 1, 2,
		1, 2, 3
	};

	NowshaderProgram.UseShaderProgram();

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TwoTriangles), TwoTriangles, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TwoTrianglesIndices), TwoTrianglesIndices, GL_STATIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	NowshaderProgram.setInt("DepthTexture", 0);
	NowshaderProgram.setInt("ThicknessTexture", 1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(window);

	glBindVertexArray(0);
}

void ScreenSpaceFluids(GLFWwindow* window, Camera camera, unsigned int VAO) {

	Shader DepthTextureShader("DepthTexture.vs", "DepthTexture.fs");
	Shader ThicknessTextureShader("ThicknessTexture.vs", "ThicknessTexture.fs");
	Shader shaderProgram("shader.vs", "shader.fs");

	glGenFramebuffers(1, &FBO);

	getDepthTexture(window, camera, VAO, DepthTextureShader);

	getThicknessTexture(camera, VAO, ThicknessTextureShader);

	Rendering(window, shaderProgram);
}