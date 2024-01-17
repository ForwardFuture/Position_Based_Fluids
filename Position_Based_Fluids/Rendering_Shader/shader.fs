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

vec3 fluid_color = vec3(0.361f, 0.702f, 0.8f);
vec3 Ks = vec3(1.0);
int alpha = 32;
float gamma = 0.1;
float n = 1.33;
float F0 = pow((1.0f - n) / (1.0f + n), 2);

bool isnan(float val) {//NOTE: During calculation the value of Fresnel turns to NAN
  return (val < 0.0 || 0.0 < val || val == 0.0) ? false : true;
}

float fresnel(float costheta) {
	//NOTE: The outcome of pow may be NAN due to multiple solutions(in the case of the base is less than zero)
	float k = 1.0f - costheta;
	if(k < 0) k = eps;
	return F0 + (1.0f - F0) * pow(k, 5);
}

vec3 getWorldSpace(float x, float y, float z) {
	// Necessities: Depth_BilateralFilter, VP
	vec4 ClipSpace = vec4(x, y, z, 1.0);
	vec4 WorldSpaceStar = inverse(VP) * ClipSpace;
	return WorldSpaceStar.xyz / WorldSpaceStar.w;
}

void main() {
	//FragColor = vec4(vec3(texture(DepthTexture, TexCoord).r), 1.0);
	//FragColor = texture(ThicknessTexture, TexCoord);
	//FragColor = texture(Depth_BilateralFilter, TexCoord);
	//FragColor = texture(Thickness_GaussianBlur, TexCoord);
	//FragColor = texture(NormalTexture, TexCoord);

	// ViewDir
	vec3 WorldSpace = getWorldSpace(2.0 * (gl_FragCoord.x / Width) - 1.0, 2.0 * (gl_FragCoord.y / Height) - 1.0, 
		2.0 * (texture(Depth_BilateralFilter, TexCoord).r) - 1.0);
	
	vec3 viewDir = CameraPos - WorldSpace;
	vec3 normalized_viewDir = normalize(viewDir);

	// Normal(Still have problems)
	vec3 TexNormal = vec3(texture(NormalTexture, TexCoord));
	if(abs(TexNormal.x) < eps && abs(TexNormal.y) < eps && abs(TexNormal.z) < eps)
		discard;
	TexNormal = 2.0 * TexNormal - vec3(1.0);
	
	//vec3 N = -Front;
	//vec3 B = vec3(0.0, 1.0, 0.0);
	//vec3 T = normalize(cross(B, N));
	//mat3 TBN = mat3(T, B, N);
	//vec3 Normal = normalize(TBN * TexNormal);
	vec3 Normal = TexNormal;

	// Shading
	float Fresnel = fresnel(dot(normalized_viewDir, Normal));
	float Thickness = texture(Thickness_GaussianBlur, TexCoord).r;
	float beta = Thickness * gamma;
	vec3 a = mix(fluid_color, vec3(texture(skybox, -viewDir + beta * Normal)), exp(-Thickness));//need calculation
	vec3 b = vec3(texture(skybox, reflect(-viewDir, Normal)));

	FragColor = vec4(a * (1.0f - Fresnel) + b * Fresnel, 1.0f);
	
	//FragColor = vec4(vec3(clamp(0.0, 1.0, 1.0 - Fresnel)), 1.0);
	//FragColor = vec4((Normal + vec3(1.0)) / 2.0, 1.0);
	//FragColor = vec4((normalized_viewDir + vec3(1.0)) / 2.0, 1.0);
	//FragColor = vec4(Normal, 1.0);
	//FragColor = vec4(normalized_viewDir, 1.0);
}