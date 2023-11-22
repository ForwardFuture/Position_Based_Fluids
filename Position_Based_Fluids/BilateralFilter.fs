#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[21];
uniform float BilateralFilter[256];
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D DepthTexture;

float pos[441];

void main() {
	float tot = 0, W = 0;
	int pos_color = int(texture(DepthTexture, TexCoord).r * 255.0);

	for(int i = -10; i <= 10; i++) {
		for(int j = -10; j <= 10; j++) {
			float depth = texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
			pos[(i + 10) * 21 + j + 10] = GaussianBlur[abs(i)] * GaussianBlur[abs(j)] * BilateralFilter[abs(pos_color - int(depth * 255.0))];
			W += pos[(i + 10) * 21 + j + 10];
		}
	}

	for(int i = -10; i <= 10; i++) {
		for(int j = -10; j <= 10; j++) {
			tot += pos[(i + 10) * 21 + j + 10] * texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
		}
	}
	
	FragColor = vec4(vec3(tot / W), 1.0);
}