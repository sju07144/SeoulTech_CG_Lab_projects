#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

struct SceneConstant
{
	mat4 view;
	mat4 projection;
};

uniform mat4 world;
uniform SceneConstant sceneConstant;

struct VS_OUT
{
	vec3 worldPos;
	vec3 normal;
	vec2 texCoords;
};

out VS_OUT vs_out;

void main()
{
	mat4 worldViewProj = sceneConstant.projection * sceneConstant.view * world;
	gl_Position = worldViewProj * vec4(aPos, 1.0);

	vs_out.worldPos = vec3(world * vec4(aPos, 1.0));
	vs_out.normal = mat3(transpose(inverse(world))) * aNormal;
	vs_out.texCoords = aTexCoord;
}