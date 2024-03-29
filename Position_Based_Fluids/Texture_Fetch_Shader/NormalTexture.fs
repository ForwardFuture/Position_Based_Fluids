#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Depth_BilateralFilter;
uniform int Screen_Width;
uniform int Screen_Height;
uniform float Width;
uniform float Height;
uniform vec3 CameraPos;

uniform mat4 VP;

float eps = 1e-6;

// Step size function is necessary
float WidthStep = 1.0 / Width;
float HeightStep = 1.0 / Height;

float getStepSize(float depth) {
	// Calculate Step size according to depth
	float maxStep = 4.0f, minStep = 1.5f;
	return minStep + max(eps, 1.0f - depth) * (maxStep - minStep);
}

vec3 getWorldSpace(float x, float y, float z) {
	// Necessities: Depth_BilateralFilter, VP
	vec4 ClipSpace = vec4(x, y, z, 1.0);
	vec4 WorldSpaceStar = inverse(VP) * ClipSpace;
	return WorldSpaceStar.xyz / WorldSpaceStar.w;
}

void main() {

	float depth = texture(Depth_BilateralFilter, TexCoord).r;

	if(abs(depth - 1.0) < 0.00001){
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		discard;
	}

	float baseStepSize = getStepSize(depth);
	WidthStep *= baseStepSize;
	HeightStep *= baseStepSize;

	float x = gl_FragCoord.x / Width;
	float y = gl_FragCoord.y / Height;

	vec3 Origin = getWorldSpace(2.0 * x - 1.0, 2.0 * y - 1.0, 2.0 * depth - 1.0);

	// get derivative on X axis
	vec3 x_z = vec3(0.0, 0.0, 0.0);
	float x_positive_depth = texture(Depth_BilateralFilter, vec2(TexCoord.s + WidthStep, TexCoord.t)).r;
	vec3 x_positive = getWorldSpace(2.0 * (x + WidthStep) - 1.0, 2.0 * y - 1.0, 2.0 * x_positive_depth - 1.0);
	float x_negative_depth = texture(Depth_BilateralFilter, vec2(TexCoord.s - WidthStep, TexCoord.t)).r;
	vec3 x_negative = getWorldSpace(2.0 * (x - WidthStep) - 1.0, 2.0 * y - 1.0, 2.0 * x_negative_depth - 1.0);
	if(abs(depth - x_positive_depth) < abs(depth - x_negative_depth))
		x_z = x_positive - Origin;
	else x_z = Origin - x_negative;

	// get derivative on Y axis
	vec3 y_z = vec3(0.0, 0.0, 0.0);
	float y_positive_depth = texture(Depth_BilateralFilter, vec2(TexCoord.s, TexCoord.t + HeightStep)).r;
	vec3 y_positive = getWorldSpace(2.0 * x - 1.0, 2.0 * (y + HeightStep) - 1.0, 2.0 * y_positive_depth - 1.0);
	float y_negative_depth = texture(Depth_BilateralFilter, vec2(TexCoord.s, TexCoord.t - HeightStep)).r;
	vec3 y_negative = getWorldSpace(2.0 * x - 1.0, 2.0 * (y - HeightStep) - 1.0, 2.0 * y_negative_depth - 1.0);
	if(abs(depth - y_positive_depth) < abs(depth - y_negative_depth))
		y_z = y_positive - Origin;
	else y_z = Origin - y_negative;

	x_z = normalize(x_z);
	y_z = normalize(y_z);

	vec3 z = normalize(cross(x_z, y_z));

	FragColor = vec4((z + vec3(1.0)) / 2.0, 1.0);
}