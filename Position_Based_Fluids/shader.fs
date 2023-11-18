#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D DepthTexture;
uniform sampler2D ThicknessTexture;

void main() {
	FragColor = vec4(vec3(texture(DepthTexture, TexCoord)), 1.0);
}