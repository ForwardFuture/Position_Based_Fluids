#include "SSF.h"

static unsigned int FBO;
static unsigned int DepthTexture;
static unsigned int ThicknessTexture;

static unsigned int Depth_BilateralFilter;
static unsigned int Thickness_GaussianBlur;
static unsigned int NormalTexture;

static glm::mat4 MVP, model, view, projection;

//R must be odd. If R is updated to an even number, relevant code segment needs to be changed.
static const int R1 = 5;
static const int R2 = 31;
static float gaussian_sigma_Depth = 1000.0f;
static float gaussian_sigma_Thickness = 1000.0f;
static float bilateral_sigma = 1000.0f;
static float W_Depth = 0.0f;
static float W_Thickness = 0.0f;

static float gaussian_kernel_Depth[R1];
static float gaussian_kernel_Thickness[R2];
static float bilateral_kernel[256];

static float TwoTriangles[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
};
static unsigned int TwoTrianglesIndices[] = {
	0, 1, 2,
	1, 2, 3
};
static unsigned int TwoTriangles_VAO, TwoTriangles_VBO, TwoTriangles_EBO;

static void PrepareParameter() {

	for (int i = 0; i <= (R1 - 1) / 2; i++) {
		gaussian_kernel_Depth[i] = exp(-1.0 * i * i / (2.0 * gaussian_sigma_Depth * gaussian_sigma_Depth));
	}

	W_Depth = 0.0f;
	for (int i = 0; i < R1; i++) {
		for (int j = 0; j < R1; j++) {
			W_Depth += gaussian_kernel_Depth[abs(i - (R1 - 1) / 2)] * gaussian_kernel_Depth[abs(j - (R1 - 1) / 2)];
		}
	}

	for (int i = 0; i <= (R2 - 1) / 2; i++) {
		gaussian_kernel_Thickness[i] = exp(-1.0 * i * i / (2.0 * gaussian_sigma_Thickness * gaussian_sigma_Thickness));
	}

	W_Thickness = 0.0f;
	for (int i = 0; i < R2; i++) {
		for (int j = 0; j < R2; j++) {
			W_Thickness += gaussian_kernel_Thickness[abs(i - (R2 - 1) / 2)] * gaussian_kernel_Thickness[abs(j - (R2 - 1) / 2)];
		}
	}

	for (int i = 0; i < 256; i++) {
		bilateral_kernel[i] = exp(-1.0 * i * i / (2.0 * bilateral_sigma * bilateral_sigma));
	}

	glGenVertexArrays(1, &TwoTriangles_VAO);
	glGenBuffers(1, &TwoTriangles_VBO);
	glGenBuffers(1, &TwoTriangles_EBO);

	glBindVertexArray(TwoTriangles_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, TwoTriangles_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TwoTriangles), TwoTriangles, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TwoTriangles_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TwoTrianglesIndices), TwoTrianglesIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

static void Draw(Camera camera, unsigned int VAO, Shader NowshaderProgram) {
	//Use shaderProgram
	NowshaderProgram.UseShaderProgram();

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

		NowshaderProgram.setMatrix("MVP", MVP);

		glDrawElements(GL_TRIANGLES, 3 * 8, GL_UNSIGNED_INT, 0);
	}

	//Unbind VAO
	glBindVertexArray(0);
}

static void getDepthTexture(Camera camera, unsigned int VAO) {
	
	//Shader
	Shader DepthTextureShader("WithMVP.vs", "DepthTexture.fs");
	//Texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &DepthTexture);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//DepthTest
	glEnable(GL_DEPTH_TEST);
	//Framebuffer & Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	Draw(camera, VAO, DepthTextureShader);

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void getThicknessTexture(Camera camera, unsigned int VAO) {

	//Shader
	Shader ThicknessTextureShader("WithMVP.vs", "ThicknessTexture.fs");
	//Texture
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &ThicknessTexture);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	//Cullface
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//Framebuffer & Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ThicknessTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Draw(camera, VAO, ThicknessTextureShader);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void DepthTexture_BilateralFilter() {
	
	//Shader
	Shader BilateralFilter("NoMVP.vs", "BilateralFilter.fs");
	//Texture
	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &Depth_BilateralFilter);
	glBindTexture(GL_TEXTURE_2D, Depth_BilateralFilter);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Framebuffer & Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Depth_BilateralFilter, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glBindVertexArray(TwoTriangles_VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	BilateralFilter.UseShaderProgram();
	BilateralFilter.setFloatArray("GaussianBlur", gaussian_kernel_Depth, R1);
	BilateralFilter.setInt("R1", R1);
	BilateralFilter.setFloatArray("BilateralFilter", bilateral_kernel, 256);
	BilateralFilter.setInt("DepthTexture", 0);
	BilateralFilter.setInt("Screen_Width", Width);
	BilateralFilter.setInt("Screen_Height", Height);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void ThicknessTexture_GaussianBlur() {

	//Shader
	Shader GaussianBlur("NoMVP.vs", "GaussianBlur.fs");
	//Texture
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &Thickness_GaussianBlur);
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Framebuffer & Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Thickness_GaussianBlur, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glBindVertexArray(TwoTriangles_VAO);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	GaussianBlur.UseShaderProgram();
	GaussianBlur.setFloatArray("GaussianBlur", gaussian_kernel_Thickness, R2);
	GaussianBlur.setInt("R2", R2);
	GaussianBlur.setFloat("W", W_Thickness);
	GaussianBlur.setInt("ThicknessTexture", 1);
	GaussianBlur.setInt("Screen_Width", Width);
	GaussianBlur.setInt("Screen_Height", Height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void getNormalTexture() {

	//Shader
	Shader NormalTextureShader("NoMVP.vs", "NormalTexture.fs");
	//Texture
	glActiveTexture(GL_TEXTURE4);
	glGenTextures(1, &NormalTexture);
	glBindTexture(GL_TEXTURE_2D, NormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Framebuffer & Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, NormalTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glBindVertexArray(TwoTriangles_VAO);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Depth_BilateralFilter);
	NormalTextureShader.UseShaderProgram();
	NormalTextureShader.setInt("Depth_BilateralFilter", 2);
	NormalTextureShader.setInt("Screen_Width", Width);
	NormalTextureShader.setInt("Screen_Height", Height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void Rendering(GLFWwindow* window) {

	Shader shaderProgram("NoMVP.vs", "shader.fs");
	shaderProgram.UseShaderProgram();

	glBindVertexArray(TwoTriangles_VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Depth_BilateralFilter);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, NormalTexture);
	shaderProgram.setInt("DepthTexture", 0);
	shaderProgram.setInt("ThicknessTexture", 1);
	shaderProgram.setInt("Depth_BilateralFilter", 2);
	shaderProgram.setInt("Thickness_GaussianBlur", 3);
	shaderProgram.setInt("NormalTexture", 4);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(window);

	glBindVertexArray(0);
}

void ScreenSpaceFluids(GLFWwindow* window, Camera camera, unsigned int VAO) {

	PrepareParameter();

	glGenFramebuffers(1, &FBO);
	getDepthTexture(camera, VAO);
	getThicknessTexture(camera, VAO);

	DepthTexture_BilateralFilter();
	ThicknessTexture_GaussianBlur();
	getNormalTexture();

	Rendering(window);
}