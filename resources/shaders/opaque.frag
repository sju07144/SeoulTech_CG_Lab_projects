#version 430 core

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

struct Material
{
	vec3 ka;
	vec3 kd;
	vec3 ks;

	vec3 fresnelR0;
	float roughness;
};

struct VS_OUT
{
	vec3 worldPos;
	vec3 normal;
	vec2 texCoords;
};

#define NUM_DIRECTIONAL_LIGHTS 1
#define NUM_POINT_LIGHTS 0
#define NUM_SPOT_LIGHTS 0

in VS_OUT vs_out;

out vec4 color;

uniform Material material;
uniform sampler2D albedoMap0;
uniform SceneConstant sceneConstant;

vec3 BlinnPhong(vec3 ambient, vec3 diffuse, vec3 specular,
	float shininess, vec3 surfaceColor,
	vec3 lightDir, vec3 viewDir, vec3 normal);
vec3 CalculateDirectionalLight(DirectionalLight light, float roughness, vec3 surfaceColor, 
	vec3 worldPos, vec3 cameraPos, vec3 normal);
vec3 CalculatePointLight(PointLight light, float roughness, vec3 surfaceColor, 
	vec3 worldPos, vec3 cameraPos, vec3 normal);
vec3 CalculateSpotLight(SpotLight light, float roughness, vec3 surfaceColor, 
	vec3 worldPos, vec3 cameraPos, vec3 normal);

void main()
{
	vec3 finalColor = vec3(0.0f, 0.0f, 0.0f);
	vec3 surfaceColor = texture(albedoMap0, vs_out.texCoords).rgb;
	// color = vec4(surfaceColor, 1.0f);

	for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)
		finalColor += CalculateDirectionalLight(sceneConstant.directionalLights[i], material.roughness, surfaceColor,
			vs_out.worldPos, sceneConstant.cameraPos, vs_out.normal);

	color = vec4(finalColor, 1.0f);
}

vec3 BlinnPhong(vec3 ambient, vec3 diffuse, vec3 specular,
	float shininess, vec3 surfaceColor,
	vec3 lightDir, vec3 viewDir, vec3 normal)
{
	// ambient
	vec3 ambientColor = surfaceColor * ambient;

	// diffuse
	float NdotL = max(dot(normal, lightDir), 0.0f);
	vec3 diffuseColor = surfaceColor * NdotL * diffuse;

	// specular
	vec3 halfVector = normalize(lightDir + viewDir);
	float NdotH = max(dot(normal, halfVector), 0.0f);
	vec3 specularColor = surfaceColor * pow(NdotH, 32.0f) * specular;

	return (ambientColor + diffuseColor + specularColor);
}

vec3 CalculateDirectionalLight(DirectionalLight light, float roughness, vec3 surfaceColor, 
	vec3 worldPos, vec3 cameraPos, vec3 normal)
{
	vec3 lightDir = normalize(-light.direction); // light Direction reversed.
	float shininess = 1.0f - roughness;
	vec3 viewDir = normalize(cameraPos - worldPos);

	return BlinnPhong(sceneConstant.ambientLight.rgb, light.diffuse, light.specular,
		shininess, surfaceColor, lightDir, viewDir, normal);
}

vec3 CalculatePointLight(PointLight light, float roughness, vec3 surfaceColor, 
	vec3 worldPos, vec3 cameraPos, vec3 normal)
{
	vec3 lightDir = normalize(light.position - worldPos);
	float shininess = 1.0f - roughness;
	vec3 viewDir = normalize(cameraPos - worldPos);

	float distance = length(light.position - worldPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	vec3 ambient = sceneConstant.ambientLight.rgb;
	vec3 diffuse = light.diffuse;
	vec3 specular = light.specular;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return BlinnPhong(ambient, diffuse, specular,
		shininess, surfaceColor, lightDir, viewDir, normal);
}

vec3 CalculateSpotLight(SpotLight light, float roughness, vec3 surfaceColor, 
	vec3 worldPos, vec3 cameraPos, vec3 normal)
{
	vec3 lightDir = normalize(light.position - worldPos);
	float shininess = 1.0f - roughness;
	vec3 viewDir = normalize(cameraPos - worldPos);

	float distance = length(light.position - worldPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

	vec3 ambient = sceneConstant.ambientLight.rgb;
	vec3 diffuse = light.diffuse;
	vec3 specular = light.specular;

	ambient *= (attenuation * intensity);
	diffuse *= (attenuation * intensity);
	specular *= (attenuation * intensity);

	return BlinnPhong(ambient, diffuse, specular, 
		shininess, surfaceColor, lightDir, viewDir, normal);
}