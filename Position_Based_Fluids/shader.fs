#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D DepthTexture;
uniform sampler2D ThicknessTexture;
uniform sampler2D Thickness_GaussianBlur;

void main() {
	//FragColor = vec4(vec3(texture(DepthTexture, TexCoord).r), 1.0);
	//FragColor = texture(ThicknessTexture, TexCoord);
	FragColor = texture(Thickness_GaussianBlur, TexCoord);
}