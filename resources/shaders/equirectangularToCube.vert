#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

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
};

struct VS_OUT
{
	vec3 texCoords;
};

uniform SceneConstant sceneConstant;
out VS_OUT vs_out;

void main()
{
	vs_out.texCoords = aPos;
	gl_Position = sceneConstant.projection * sceneConstant.view * vec4(aPos, 1.0f);
}