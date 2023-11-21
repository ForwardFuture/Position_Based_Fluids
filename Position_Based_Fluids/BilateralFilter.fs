#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[5];
uniform float BilateralFilter[256];
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D DepthTexture;

float pos[25];

void main() {
	float tot = 0, W = 0;
	int pos_color = int(texture(DepthTexture, TexCoord).r * 255.0);

	for(int i = -2; i <= 2; i++) {
		for(int j = -2; j <= 2; j++) {
			pos[(i + 2) * 5 + j + 2] = GaussianBlur[abs(i)] * GaussianBlur[abs(j)] * 
			BilateralFilter[abs(pos_color - int(texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r * 255.0))];
			W += pos[(i + 2) * 5 + j + 2];
		}
	}

	for(int i = -2; i <= 2; i++) {
		for(int j = -2; j <= 2; j++) {
			tot += pos[(i + 2) * 5 + j + 2] * texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
		}
	}
	
	FragColor = vec4(vec3(tot / W), 1.0);
}