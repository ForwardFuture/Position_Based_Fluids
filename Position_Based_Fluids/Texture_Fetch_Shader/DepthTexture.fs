#version 330 core

out vec4 FragColor;

void main() {
	//Updates only affect Colorbuffer, but we need value in Depthbuffer, thus updates unnecessary here.
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}