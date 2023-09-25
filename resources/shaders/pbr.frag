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

#define NUM_DIRECTIONAL_LIGHTS 0
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 0

in VS_OUT vs_out;

out vec4 color;

uniform bool isUsingTexture;
uniform bool isUsingNormalMap;
uniform bool enableIrradianceMap;

uniform Material material;
uniform sampler2D albedoMap0;
uniform sampler2D normalMap0;
uniform sampler2D metallicMap0;
uniform sampler2D roughnessMap0;

uniform samplerCube irradianceMap;

uniform SceneConstant sceneConstant;

const float PI = 3.14159265359f;

vec3 GetNormalFromMap(sampler2D normalMap);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);   

vec3 BlinnPhong(vec3 ambient, vec3 diffuse, vec3 specular,
	float shininess, vec3 surfaceColor,
	vec3 lightDir, vec3 viewDir, vec3 normal);
vec3 CalculateDirectionalLight(DirectionalLight light, 
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);
vec3 CalculatePointLight(PointLight light,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);
vec3 CalculateSpotLight(SpotLight light,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);

void main()
{
	vec3 albedo;
	float metallic, roughness, ao;
	if (isUsingTexture)
	{
		albedo = texture(albedoMap0, vs_out.texCoords).rgb;
		metallic = texture(metallicMap0, vs_out.texCoords).r;
		roughness = texture(roughnessMap0, vs_out.texCoords).r;
		ao = material.ao;
	}
	else
	{
		albedo = material.kd;
		metallic = material.metallic;
		roughness = material.roughness;
		ao = material.ao;
	}

	vec3 N;
	if (isUsingNormalMap)
		N = GetNormalFromMap(normalMap0);
	else
		N = vs_out.normal;
	vec3 V = normalize(sceneConstant.cameraPos - vs_out.worldPos);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)   
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	vec3 pointLightColor = vec3(0.0f);	

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		pointLightColor += CalculatePointLight(sceneConstant.pointLights[i],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	// ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
	vec3 ambient = vec3(0.002f);
	if (enableIrradianceMap)
	{
		vec3 kS = FresnelSchlick(max(dot(N, V), 0.0f), F0);
		vec3 kD = 1.0f - kS;
		kD *= 1.0f - metallic;	  
		vec3 irradiance = texture(irradianceMap, N).rgb;
		vec3 diffuse = irradiance * albedo;
		ambient = (kD * diffuse) * ao;
	}
	else
	{
		ambient = ambient * albedo * ao;
	}
	
	vec3 result = ambient + pointLightColor;

	// HDR tonemapping
	result = result / (result + vec3(1.0f));
	// gamma correct
	result = pow(result, vec3(1.0f / 2.2f));

	color = vec4(result, 1.0f);
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

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;
	
	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}   

vec3 CalculatePointLight(PointLight light,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal)
{
	// calculate per-light radiance
	vec3 lightDir = normalize(light.position - vs_out.worldPos);
	vec3 halfway = normalize(viewDir + lightDir);
	float distance = length(light.position - vs_out.worldPos);
	float attenuation = 1.0f / (distance * distance);
	vec3 radiance = light.diffuse * attenuation;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(normal, halfway, roughness);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);
	vec3 F = FresnelSchlick(max(dot(halfway, viewDir), 0.0f), F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001; // + 0.0001 to prevent divide by zero
	vec3 specular = numerator / denominator;

	// kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0f) - kS;
	// multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
	kD *= 1.0f - metallic;

	// scale light by NdotL
	float NdotL = max(dot(normal, lightDir), 0.0f);

	return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}