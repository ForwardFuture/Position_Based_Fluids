#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float GaussianBlur[31];
uniform int R2;
uniform int Screen_Width;
uniform int Screen_Height;
uniform sampler2D Image;
uniform bool Horizontal;

void main() {

	vec2 offset = 1.0 / textureSize(Image, 0);
	vec3 Color = vec3(texture(Image, TexCoord)) * GaussianBlur[0];

	if(Horizontal) {
		for(int i = 1; i <= R2; i++) {
			Color += vec3(texture(Image, TexCoord + vec2(offset.x * float(i), 0.0))) * GaussianBlur[i];
			Color += vec3(texture(Image, TexCoord - vec2(offset.x * float(i), 0.0))) * GaussianBlur[i];
		}
	}
	else {
		for(int i = 1; i <= R2; i++) {
			Color += vec3(texture(Image, TexCoord + vec2(0.0, offset.y * float(i)))) * GaussianBlur[i];
			Color += vec3(texture(Image, TexCoord - vec2(0.0, offset.y * float(i)))) * GaussianBlur[i];
		}
	}

	FragColor = vec4(Color, 1.0);
}