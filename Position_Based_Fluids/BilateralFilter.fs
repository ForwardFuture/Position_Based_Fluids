#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[5];
uniform int R1;
uniform float BilateralFilter[256];
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D DepthTexture;

float pos[5 * 5];

void main() {
	float tot = 0, W = 0;
	int bound = (R1 - 1) / 2;
	int pos_color = int(texture(DepthTexture, TexCoord).r * 255.0);

	for(int i = -bound; i <= bound; i++) {
		for(int j = -bound; j <= bound; j++) {
			float depth = texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
			pos[(i + bound) * R1 + j + bound] = GaussianBlur[abs(i)] * GaussianBlur[abs(j)] * BilateralFilter[abs(pos_color - int(depth * 255.0))];
			W += pos[(i + bound) * R1 + j + bound];
		}
	}

	for(int i = -bound; i <= bound; i++) {
		for(int j = -bound; j <= bound; j++) {
			tot += pos[(i + bound) * R1 + j + bound] * texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
		}
	}
	
	FragColor = vec4(vec3(tot / W), 1.0);
}