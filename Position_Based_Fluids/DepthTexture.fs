#version 330 core

out vec4 FragColor;

void main() {
	//这里修改的是Color_Buffer的值，实际上需要的是Depth_Buffer中的值，因此此处无所谓着色与否
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}