#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Depth_BilateralFilter;
uniform int Screen_Width;
uniform int Screen_Height;

void main() {

	float depth = texture(Depth_BilateralFilter, TexCoord).r;

	if(abs(depth - 1.0) < 0.00001){
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 x_z = vec3(0.0, 0.0, 0.0);
	float x_positive = texture(Depth_BilateralFilter, vec2(TexCoord.s + 1.0 / Screen_Width, TexCoord.t)).r;
	float x_negative = texture(Depth_BilateralFilter, vec2(TexCoord.s - 1.0 / Screen_Width, TexCoord.t)).r;
	if(abs(depth - x_positive) < abs(depth - x_negative))
		x_z = vec3(1.0 / Screen_Width, 0.0, x_positive - depth);
	else x_z = vec3(-1.0 / Screen_Width, 0.0, x_negative - depth);

	vec3 y_z = vec3(0.0, 0.0, 0.0);
	float y_positive = texture(Depth_BilateralFilter, vec2(TexCoord.s, TexCoord.t + 1.0 / Screen_Height)).r;
	float y_negative = texture(Depth_BilateralFilter, vec2(TexCoord.s, TexCoord.t - 1.0 / Screen_Height)).r;
	if(abs(depth - y_positive) < abs(depth - y_negative))
		y_z = vec3(0.0, 1.0 / Screen_Height, y_positive - depth);
	else y_z = vec3(0.0, -1.0 / Screen_Height, y_negative - depth);

	vec3 z = vec3(x_z[1] * y_z[2] - x_z[2] * y_z[1], x_z[2] * y_z[0] - x_z[0] * y_z[2], x_z[0] * y_z[1] - x_z[1] * y_z[0]);
	if(z.z < 0) z = -z;
	float len = sqrt(z.x * z.x + z.y * z.y + z.z * z.z);
	z = z / len;

	FragColor = vec4(z, 1.0);
}