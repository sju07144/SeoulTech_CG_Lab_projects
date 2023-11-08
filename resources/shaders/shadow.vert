#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 world;

void main()
{
    gl_Position = lightSpaceMatrix * world * vec4(aPos, 1.0);
}  