#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gAlbedo;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out float gMetallic;
layout (location = 4) out float gRoughness;
layout (location = 5) out float gAo;

struct SceneConstant
{
	mat4 view;
	mat4 projection;
};

struct Material
{
	vec3 ka;
	vec3 kd;
	vec3 ks;

	float metallic;
	float roughness;
	float ao;
};

struct VS_OUT
{
	vec3 worldPos;
	vec3 normal;
	vec2 texCoords;
};

#define NUM_DIRECTIONAL_LIGHTS 1
#define NUM_POINT_LIGHTS 4
#define NUM_SPOT_LIGHTS 0

in VS_OUT vs_out;

out vec4 color;

uniform bool isUsingTexture;
uniform bool isUsingNormalMap;

uniform Material material;
uniform sampler2D albedoMap0;
uniform sampler2D normalMap0;
uniform sampler2D metallicMap0;
uniform sampler2D roughnessMap0;

uniform SceneConstant sceneConstant;

const float PI = 3.14159265359f;

vec3 GetNormalFromMap(sampler2D normalMap);

void main()
{
	gPosition = vs_out.worldPos;
	gAlbedo = material.kd;
	gMetallic = material.metallic;
	gRoughness = material.roughness;
	gAo = material.ao;
	if (isUsingTexture)
	{
		gAlbedo = texture(albedoMap0, vs_out.texCoords).rgb;
		gMetallic = texture(metallicMap0, vs_out.texCoords).r;
		gRoughness = texture(roughnessMap0, vs_out.texCoords).r;
		gAo = material.ao;
	}

	gNormal = normalize(vs_out.normal);
	if (isUsingNormalMap)
		gNormal = GetNormalFromMap(normalMap0);
}

vec3 GetNormalFromMap(sampler2D normalMap)
{
	vec3 tangentNormal = texture(normalMap, vs_out.texCoords).xyz * 2.0f - 1.0f;

	vec3 Q1 = dFdx(vs_out.worldPos);
	vec3 Q2 = dFdy(vs_out.worldPos);
	vec2 st1 = dFdx(vs_out.texCoords);
	vec2 st2 = dFdy(vs_out.texCoords);

	vec3 N = normalize(vs_out.normal);
	vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}