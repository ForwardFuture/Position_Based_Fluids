#include "SSF.h"

// FBO
static unsigned int FBO;
// Fetch Texture
static unsigned int DepthTexture;
static unsigned int ThicknessTexture;
static unsigned int NormalTexture;
// Blur Texture
static unsigned int Depth_BilateralFilter;
static unsigned int Thickness_GaussianBlur_Horizontal;
static unsigned int Thickness_GaussianBlur;

static glm::mat4 MVP, model, view, projection;

// NOTE: When changing R1/R2 here, remember to change relevant parameters in GaussianBlur.fs & BilateralFilter.fs
static const int R1 = 3;
static const int R2 = 30;
static float gaussian_sigma_Depth = 10.0f;
static float gaussian_sigma_Thickness = 10.0f;
static float bilateral_sigma = 10.0f;

static float gaussian_kernel_Depth[R1 + 1];
static float gaussian_kernel_Thickness[R2 + 1];
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

static float skyboxVertices[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};
static unsigned int skybox_VAO, skybox_VBO;

static void initial_kernel() {
	// Depth Blur_Gaussian Kernel
	for (int i = 0; i <= R1; i++) {
		gaussian_kernel_Depth[abs(i)] = exp(-1.0 * i * i / (2.0 * gaussian_sigma_Depth * gaussian_sigma_Depth));
	}

	// Depth Blur_Bilateral Kernel
	for (int i = 0; i < 256; i++) {
		bilateral_kernel[i] = exp(-1.0 * i * i / (2.0 * bilateral_sigma * bilateral_sigma));
	}

	// Thickness Blur_Gaussian Kernel
	for (int i = 0; i <= R2; i++) {
		gaussian_kernel_Thickness[i] = exp(-1.0 * i * i / (2.0 * gaussian_sigma_Thickness * gaussian_sigma_Thickness));
	}

	float W_Thickness = 0.0f;

	for (int i = 0; i <= R2; i++) {
		for (int j = 0; j <= R2; j++) {
			if (i == 0 && j == 0) {
				W_Thickness += gaussian_kernel_Thickness[i] * gaussian_kernel_Thickness[j];
			}
			else if (i == 0 || j == 0) {
				W_Thickness += gaussian_kernel_Thickness[i] * gaussian_kernel_Thickness[j] * 2.0f;
			}
			else W_Thickness += gaussian_kernel_Thickness[i] * gaussian_kernel_Thickness[j] * 4.0f;
		}
	}

	W_Thickness = sqrt(W_Thickness);

	for (int i = 0; i <= R2; i++) {
		gaussian_kernel_Thickness[i] /= W_Thickness;
	}

}

static void initial_two_triangles() {
	// Two Triangles VAO & VBO & EBO
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

static void initial_skybox() {
	// skybox VAO & VBO
	glGenVertexArrays(1, &skybox_VAO);
	glGenBuffers(1, &skybox_VBO);

	glBindVertexArray(skybox_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

static void initial_FBO() {

	glGenFramebuffers(1, &FBO);
}

static void initial_texture() {

	// Depth_Texture
	glGenTextures(1, &DepthTexture);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Thickness_Texture
	glGenTextures(1, &ThicknessTexture);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Depth_Bilateral_Texture
	glGenTextures(1, &Depth_BilateralFilter);
	glBindTexture(GL_TEXTURE_2D, Depth_BilateralFilter);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Thickness_Gaussian_Horizontal_Texture
	glGenTextures(1, &Thickness_GaussianBlur_Horizontal);
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur_Horizontal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Thickness_Gaussian_Texture
	glGenTextures(1, &Thickness_GaussianBlur);
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Normal_Texture
	glGenTextures(1, &NormalTexture);
	glBindTexture(GL_TEXTURE_2D, NormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void SSF_initial() {

	initial_kernel();

	initial_two_triangles();

	initial_skybox();

	initial_FBO();

	initial_texture();
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
	Shader DepthTextureShader("./Universal_Shader/WithMVP.vs", "./Texture_Fetch_Shader/DepthTexture.fs");
	//Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DepthTexture);
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
	Shader ThicknessTextureShader("./Universal_Shader/WithMVP.vs", "./Texture_Fetch_Shader/ThicknessTexture.fs");
	//Texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
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
	Shader BilateralFilter("./Universal_Shader/NoMVP.vs", "./Texture_Blur_Shader/BilateralFilter.fs");
	//Texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Depth_BilateralFilter);
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
	BilateralFilter.setFloatArray("GaussianBlur", gaussian_kernel_Depth, R1 + 1);
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

static void One_Step_GaussianBlur(Shader GaussianBlur) {

	//Texture
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur_Horizontal);
	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Framebuffer & Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Thickness_GaussianBlur_Horizontal, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR: FRAMEBUFFER NOT COMPLETE!" << std::endl;
		return;
	}

	glBindVertexArray(TwoTriangles_VAO);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ThicknessTexture);
	GaussianBlur.UseShaderProgram();
	GaussianBlur.setFloatArray("GaussianBlur", gaussian_kernel_Thickness, R2 + 1);
	GaussianBlur.setInt("R2", R2);
	GaussianBlur.setInt("Image", 1);
	GaussianBlur.setInt("Screen_Width", Width);
	GaussianBlur.setInt("Screen_Height", Height);
	GaussianBlur.setBool("Horizontal", true);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void Two_Step_GaussianBlur(Shader GaussianBlur) {

	//Texture
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur);
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
	glBindTexture(GL_TEXTURE_2D, Thickness_GaussianBlur_Horizontal);
	GaussianBlur.UseShaderProgram();
	GaussianBlur.setFloatArray("GaussianBlur", gaussian_kernel_Thickness, R2 + 1);
	GaussianBlur.setInt("R2", R2);
	GaussianBlur.setInt("Image", 1);
	GaussianBlur.setInt("Screen_Width", Width);
	GaussianBlur.setInt("Screen_Height", Height);
	GaussianBlur.setBool("Horizontal", false);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void ThicknessTexture_GaussianBlur() {

	//Shader
	Shader GaussianBlur("./Universal_Shader/NoMVP.vs", "./Texture_Blur_Shader/GaussianBlur.fs");

	One_Step_GaussianBlur(GaussianBlur);

	Two_Step_GaussianBlur(GaussianBlur);
}

static void getNormalTexture() {

	//Shader
	Shader NormalTextureShader("./Universal_Shader/NoMVP.vs", "./Texture_Fetch_Shader/NormalTexture.fs");
	//Texture
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, NormalTexture);
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

static void Rendering_skybox() {

}

static void Rendering_fluid() {

}

static void Rendering(Camera camera, GLFWwindow* window) {

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Rendering_skybox();
	Rendering_fluid();

	Shader shaderProgram("./Universal_Shader/NoMVP.vs", "./Rendering_Shader/shader.fs");
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
	shaderProgram.setFloatVec("CameraPos", camera.GetPosition());
	shaderProgram.setFloatVec("Front", camera.GetFront());

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(window);

	glBindVertexArray(0);
}

void ScreenSpaceFluids(GLFWwindow* window, Camera camera, unsigned int VAO) {

	getDepthTexture(camera, VAO);
	getThicknessTexture(camera, VAO);

	DepthTexture_BilateralFilter();
	ThicknessTexture_GaussianBlur();
	getNormalTexture();

	Rendering(camera, window);
}