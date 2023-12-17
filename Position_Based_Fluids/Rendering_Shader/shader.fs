#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D DepthTexture;
uniform sampler2D ThicknessTexture;
uniform sampler2D Depth_BilateralFilter;
uniform sampler2D Thickness_GaussianBlur;
uniform sampler2D NormalTexture;

uniform vec3 CameraPos;
uniform vec3 Front;

float eps = 1e-6;

vec3 Ks = vec3(1.0);
int alpha = 32;
float gamma = 0.1;

void main() {
	//FragColor = vec4(vec3(texture(DepthTexture, TexCoord).r), 1.0);
	//FragColor = texture(ThicknessTexture, TexCoord);
	//FragColor = texture(Depth_BilateralFilter, TexCoord);
	//FragColor = texture(Thickness_GaussianBlur, TexCoord);
	//FragColor = texture(NormalTexture, TexCoord);

	
	vec3 aNormal = vec3(texture(NormalTexture, TexCoord));
	if(abs(aNormal.x) < eps && abs(aNormal.y) < eps && abs(aNormal.z) < eps)
		discard;
	aNormal = 2.0 * aNormal - 1.0;
	
	vec3 N = -Front;
	vec3 B = vec3(0.0, 1.0, 0.0);
	vec3 T = normalize(cross(B, N));
	mat3 TBN = mat3(T, B, N);
	vec3 Normal = TBN * aNormal;

	FragColor = vec4((Normal + 1.0) / 2.0, 1.0);
	
	//viewDir = 
	//lightDir = 
	//halfDir = 
	
	//Fresnel = 
}