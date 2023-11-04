#include "SSF.h"

static unsigned int FBO;
static unsigned int DepthTexture;
static unsigned int ThicknessTexture;

static Shader shaderProgram("shader.vs", "shader.fs");
static glm::mat4 MVP, model, view, projection;

static void PrintTextureToFile() {
	
	static unsigned int Tmpbuffer;
	glGenFramebuffers(1, &Tmpbuffer);

	glReadPixels(0, 0, Width, Height, GL_DEPTH_COMPONENT, GL_FLOAT, &Tmpbuffer);

	stbi_write_bmp("Texture.bmp", Width, Height, 1, &Tmpbuffer);
	
}

static void Draw(Camera camera, unsigned int VAO) {
	//Use shaderProgram
	shaderProgram.UseShaderProgram();

	//Bind VAO
	glBindVertexArray(VAO);

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

		shaderProgram.setMatrix("MVP", MVP);

		glDrawElements(GL_TRIANGLES, 3 * 8, GL_UNSIGNED_INT, 0);
	}

	//Unbind VAO
	glBindVertexArray(0);
}

static void getDepthTexture(Camera camera, unsigned int VAO) {
	
	glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Draw(camera, VAO);

	PrintTextureToFile();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
}

static void getThicknessTexture(Camera camera, unsigned int VAO) {

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ThicknessTexture, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Draw(camera, VAO);

	PrintTextureToFile();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
}

void ScreenSpaceFluids(GLFWwindow* window, Camera camera, unsigned int VAO) {
	
	glGenFramebuffers(1, &FBO);
	glGenTextures(1, &DepthTexture);
	glGenTextures(1, &ThicknessTexture);

	getDepthTexture(camera, VAO);

	getThicknessTexture(camera, VAO);


	//glfwSwapBuffers(window);
}