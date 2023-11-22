#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[21];
uniform float W;
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D ThicknessTexture;

void main() {
	float tot = 0;
	for(int i = -10; i <= 10; i++) {
		for(int j = -10; j <= 10; j++) {
			tot += texture(ThicknessTexture, vec2(TexCoord.s + 1.0 * i / Screen_Width, TexCoord.t + 1.0 * j / Screen_Height)).r 
			* GaussianBlur[abs(i)] * GaussianBlur[abs(j)];
		}
	}
	
	FragColor = vec4(vec3(tot / W), 1.0);
}