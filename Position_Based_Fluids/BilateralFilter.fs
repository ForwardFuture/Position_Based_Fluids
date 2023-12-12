#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[4];
uniform int R1;
uniform float BilateralFilter[256];
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D DepthTexture;

float pos[9 * 9];

void main() {
	float tot = 0, W = 0;
	int k = (2 * R1) + 1;
	int pos_color = int(texture(DepthTexture, TexCoord).r * 255.0);

	for(int i = -R1; i <= R1; i++) {
		for(int j = -R1; j <= R1; j++) {
			float depth = texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
			pos[(i + R1) * k + j + R1] = GaussianBlur[abs(i)] * GaussianBlur[abs(j)] * BilateralFilter[abs(pos_color - int(depth * 255.0))];
			W += pos[(i + R1) * k + j + R1];
		}
	}

	for(int i = -R1; i <= R1; i++) {
		for(int j = -R1; j <= R1; j++) {
			tot += pos[(i + R1) * k + j + R1] * texture(DepthTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r;
		}
	}
	
	FragColor = vec4(vec3(tot / W), 1.0);
}