#version 430 core

struct VS_OUT
{
	vec3 texCoords;
};

in VS_OUT vs_out;
out vec4 color;

uniform samplerCube environmentMap;

void main()
{
	color = texture(environmentMap, vs_out.texCoords);
}