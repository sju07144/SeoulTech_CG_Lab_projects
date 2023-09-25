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
	vec3 envColor = texture(environmentMap, vs_out.texCoords).rgb;
	
	envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    color = vec4(envColor, 1.0);
}