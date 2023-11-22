#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[51];
uniform float BilateralFilter[256];
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D DepthTexture;

float pos[2601];

void main() {
	float tot = 0, W = 0;
	int pos_color = int(texture(DepthTexture, TexCoord).r * 255.0);

	for(int i = -25; i <= 25; i++) {
		for(int j = -25; j <= 25; j++) {
			pos[(i + 25) * 51 + j + 25] = GaussianBlur[abs(i)] * GaussianBlur[abs(j)] * 
			BilateralFilter[abs(pos_color - int(texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r * 255.0))];
			W += pos[(i + 25) * 51 + j + 25];
		}
	}

	for(int i = -25; i <= 25; i++) {
		for(int j = -25; j <= 25; j++) {
			tot += pos[(i + 25) * 51 + j + 25] * texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
		}
	}
	
	FragColor = vec4(vec3(tot / W), 1.0);
}