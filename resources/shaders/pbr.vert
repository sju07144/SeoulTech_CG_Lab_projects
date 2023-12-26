#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

struct PointLight
{
	vec3 diffuse;
	vec3 specular;

	vec3 position;

	float constant;
	float linear;
	float quadratic;
};

struct DirectionalLight
{
	vec3 diffuse;
	vec3 specular;

	vec3 direction;

	mat4 lightSpaceMatrix;
};

struct SpotLight
{
	vec3 diffuse;
	vec3 specular;

	vec3 direction;
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};

struct SceneConstant
{
	mat4 view;
	mat4 projection;

	vec3 cameraPos;
	float pad0;

	vec4 ambientLight;
	DirectionalLight directionalLights[4];
	PointLight pointLights[4];
	SpotLight spotLights[4];

	float farPlane;
};

uniform mat4 world;
uniform SceneConstant sceneConstant;

struct VS_OUT
{
	vec3 worldPos;
	vec3 normal;
	vec2 texCoords;

	mat3 TBN;

	vec4 lightSpacePos[4];
};

out VS_OUT vs_out;

void main()
{
	mat4 worldViewProj = sceneConstant.projection * sceneConstant.view * world;
	gl_Position = worldViewProj * vec4(aPos, 1.0);

	vs_out.worldPos = vec3(world * vec4(aPos, 1.0));
	vs_out.normal = mat3(transpose(inverse(world))) * aNormal;
	vs_out.texCoords = aTexCoord;

	vec3 T = normalize(vec3(world * vec4(aTangent, 0.0)));
	vec3 N = normalize(vec3(world * vec4(aNormal, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);
	
	vs_out.TBN = mat3(T, B, N);

	for (int i = 0; i < 4; i++)
		vs_out.lightSpacePos[i] = sceneConstant.directionalLights[i].lightSpaceMatrix * vec4(vs_out.worldPos, 1.0);
}