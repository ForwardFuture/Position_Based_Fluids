#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D DepthTexture;
uniform sampler2D ThicknessTexture;
uniform sampler2D Depth_BilateralFilter;
uniform sampler2D Thickness_GaussianBlur;
uniform sampler2D NormalTexture;
uniform samplerCube skybox;

uniform vec3 CameraPos;
uniform vec3 Front;

uniform float Width;
uniform float Height;

uniform mat4 VP;

float eps = 1e-6;

vec3 fluid_color = vec3(0.36078431, 0.7019607843, 0.8);
vec3 Ks = vec3(1.0);
int alpha = 32;
float gamma = 0.1;
float n = 1.33;
float F0 = pow((1.0 - n) / (1.0 + n), 2.0);

float fresnel(float costheta) {
	return F0 + (1 - F0) * pow(1.0 - costheta, 5.0);
}

void main() {
	//FragColor = vec4(vec3(texture(DepthTexture, TexCoord).r), 1.0);
	//FragColor = texture(ThicknessTexture, TexCoord);
	//FragColor = texture(Depth_BilateralFilter, TexCoord);
	//FragColor = texture(Thickness_GaussianBlur, TexCoord);
	//FragColor = texture(NormalTexture, TexCoord);

	// ViewDir
	float x, y, z;
	x = 2.0 * (gl_FragCoord.x / Width) - 1.0;
	y = 2.0 * (gl_FragCoord.y / Height) - 1.0;
	z = 2.0 * (texture(Depth_BilateralFilter, TexCoord).r) - 1.0;
	vec4 ClipSpace = vec4(x, y, z, 1.0);
	vec4 WorldSpaceStar = inverse(VP) * ClipSpace;
	vec3 WorldSpace = WorldSpaceStar.xyz / WorldSpaceStar.w;
	
	vec3 viewDir = CameraPos - WorldSpace;
	vec3 normalized_viewDir = normalize(viewDir);

	// Normal(Still have problems)
	vec3 TexNormal = vec3(texture(NormalTexture, TexCoord));
	if(abs(TexNormal.x) < eps && abs(TexNormal.y) < eps && abs(TexNormal.z) < eps)
		discard;
	TexNormal = 2.0 * normalize(TexNormal) - normalize(vec3(1.0));
	
	vec3 N = -Front;
	vec3 B = vec3(0.0, 1.0, 0.0);
	vec3 T = normalize(cross(B, N));
	mat3 TBN = mat3(T, B, N);
	vec3 Normal = normalize(TBN * TexNormal);

	// Shading
	float Fresnel = fresnel(dot(normalized_viewDir, Normal));
	float Thickness = texture(Thickness_GaussianBlur, TexCoord).r;
	float beta = Thickness * gamma;
	vec3 a = mix(fluid_color, vec3(texture(skybox, -viewDir + beta * Normal)), exp(-Thickness));//need calculation

	FragColor = vec4(a * (1 - Fresnel), 1.0);

	//vec3 halfDir;
}