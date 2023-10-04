#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

struct VS_OUT
{
	vec2 texCoords;
};

out VS_OUT vs_out;

void main()
{
	vs_out.texCoords = aTexCoord;
	gl_Position = vec4(aPos, 1.0);
}