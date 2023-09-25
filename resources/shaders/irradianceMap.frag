#version 430 core

struct VS_OUT
{
	vec3 texCoords;
};

in VS_OUT vs_out;
out vec4 color;

uniform samplerCube environmentMap;

const float PI = 3.14159265359f;

void main()
{
	// the sample direction equals the hemisphere's orientation
	vec3 normal = normalize(vs_out.texCoords);

	vec3 irradiance = vec3(0.0f);

	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));

	float sampleDelta = 0.025f;
	float nrSamples = 0.0f;
	for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
	{
		for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
		{
			// spherical to cartesian(in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
			
			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0f / float(nrSamples));
    
    color = vec4(irradiance, 1.0f);
}