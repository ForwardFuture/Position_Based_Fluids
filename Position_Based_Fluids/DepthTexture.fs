#version 330 core

out vec4 FragColor;

void main() {
	FragColor = vec4(vec3(gl_FragCoord.z/gl_FragCoord.w), 1.0);
}