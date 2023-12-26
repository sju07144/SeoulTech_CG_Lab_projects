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

	mat3 TBN;

	vec4 lightSpacePos[4];
};

#define NUM_DIRECTIONAL_LIGHTS 1
#define NUM_POINT_LIGHTS 4
#define NUM_SPOT_LIGHTS 0

in VS_OUT vs_out;

out vec4 color;

uniform bool isUsingTexture;
uniform bool isUsingNormalMap;
uniform bool enableImageBasedLighting;
uniform bool enableShadow;

uniform Material material;
uniform sampler2D albedoMap0;
uniform sampler2D normalMap0;
uniform sampler2D metallicMap0;
uniform sampler2D roughnessMap0;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform sampler2D shadowMaps[4];
uniform samplerCube shadowCubeMaps[4];

uniform SceneConstant sceneConstant;

const float PI = 3.14159265359f;

vec3 GetNormalFromMap(sampler2D normalMap);
vec3 GetNormalFromMapWithPreCalculatedTBN(sampler2D normalMap);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);   

vec3 BlinnPhong(vec3 ambient, vec3 diffuse, vec3 specular,
	float shininess, vec3 surfaceColor,
	vec3 lightDir, vec3 viewDir, vec3 normal);
vec3 CookTorrance(vec3 lightDir, vec3 radiance,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);
vec3 CalculateDirectionalLight(DirectionalLight light, vec4 lightSpacePos, sampler2D shadowMap,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);
vec3 CalculatePointLight(PointLight light, samplerCube shadowCubeMap,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);
vec3 CalculateSpotLight(SpotLight light,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal);

vec3 CalculateImageBasedLight(vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal, vec3 viewReflection);

float CalculateShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir, sampler2D shadowMap);
float CalculatePointShadow(vec3 worldPos, vec3 viewPos, vec3 lightPos, samplerCube shadowCubeMap);

void main()
{
	vec3 albedo = material.kd;
	float metallic = material.metallic;
	float roughness = material.roughness;
	float ao = material.ao;
	if (isUsingTexture)
	{
		albedo = texture(albedoMap0, vs_out.texCoords).rgb;
		metallic = texture(metallicMap0, vs_out.texCoords).r;
		roughness = texture(roughnessMap0, vs_out.texCoords).r;
		ao = material.ao;
	}

	vec3 N = vs_out.normal;
	if (isUsingNormalMap)
		N = GetNormalFromMapWithPreCalculatedTBN(normalMap0);

	vec3 V = normalize(sceneConstant.cameraPos - vs_out.worldPos);
	vec3 R = reflect(-V, N);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)   
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	vec3 pointLightColor = vec3(0.0f);	

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		pointLightColor += CalculatePointLight(sceneConstant.pointLights[i],
			shadowCubeMaps[i],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	vec3 directionalLightColor = vec3(0.0f);	

	for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)
	{
		directionalLightColor += CalculateDirectionalLight(sceneConstant.directionalLights[i],
			vs_out.lightSpacePos[i], shadowMaps[0],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	vec3 spotLightColor = vec3(0.0f);	

	for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
	{
		spotLightColor += CalculateSpotLight(sceneConstant.spotLights[i],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	// ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
	vec3 ambient = vec3(0.002f);
	ambient = ambient * albedo * ao;
	if (enableImageBasedLighting)
		ambient = CalculateImageBasedLight(albedo, metallic, roughness, ao, F0, V, N, R);
	
	vec3 result = ambient + pointLightColor + directionalLightColor + spotLightColor;

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

vec3 GetNormalFromMapWithPreCalculatedTBN(sampler2D normalMap)
{
	vec3 tangentNormal = texture(normalMap, vs_out.texCoords).xyz * 2.0f - 1.0f;

	return normalize(vs_out.TBN * tangentNormal);
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

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}   

vec3 CookTorrance(vec3 lightDir, vec3 radiance,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal)
{
	// calculate per-light radiance
	vec3 halfway = normalize(viewDir + lightDir);

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(normal, halfway, roughness);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);
	vec3 F = FresnelSchlick(max(dot(halfway, viewDir), 0.0f), F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001f; // + 0.0001 to prevent divide by zero
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
	kD *= (1.0f - metallic);

	// scale light by NdotL
	float NdotL = max(dot(normal, lightDir), 0.0f);

	return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

vec3 CalculatePointLight(PointLight light, samplerCube shadowCubeMap,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal)
{
	// calculate per-light radiance
	vec3 lightDir = normalize(light.position - vs_out.worldPos);
	float distance = length(light.position - vs_out.worldPos);
	float attenuation = 1.0f / (distance * distance);
	vec3 radiance = light.diffuse * attenuation;

	// calculate shadow.
	float shadow = 0.0f;
	// if (enableShadow)
	// 	shadow = CalculatePointShadow(vs_out.worldPos, sceneConstant.cameraPos, light.position, shadowCubeMap);
	// else
	// 	shadow = 0.0f;

	return (1.0f - shadow) * CookTorrance(lightDir, radiance,
		albedo, metallic, roughness, ao, F0,
		viewDir, normal);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec4 lightSpacePos, sampler2D shadowMap,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal)
{
	vec3 lightDir = normalize(-light.direction);

	vec3 radiance = light.diffuse;

	// calculate shadow.
	float shadow = 0.0f;
	if (enableShadow)
		shadow = CalculateShadow(lightSpacePos, normal, lightDir, shadowMap);
	else
		shadow = 0.0f;

	return (1.0f - shadow) * CookTorrance(lightDir, radiance,
		albedo, metallic, roughness, ao, F0,
		viewDir, normal);
}

vec3 CalculateSpotLight(SpotLight light,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal)
{
	// calculate per-light radiance
	vec3 lightDir = normalize(light.position - vs_out.worldPos);

	float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	float distance = length(light.position - vs_out.worldPos);
	float attenuation = 1.0f / (distance * distance);
	vec3 radiance = light.diffuse * attenuation * intensity;

	return CookTorrance(lightDir, radiance,
		albedo, metallic, roughness, ao, F0,
		viewDir, normal);
}

vec3 CalculateImageBasedLight(vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal, vec3 viewReflection)
{
	vec3 F = FresnelSchlickRoughness(max(dot(normal, viewDir), 0.0f), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0f - kS;
	kD *= (1.0f - metallic);
	
	vec3 irradiance = texture(irradianceMap, normal).rgb;
	vec3 diffuse = irradiance * albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, viewReflection, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	
	return (kD * diffuse + specular) * ao;
}

float CalculateShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir, sampler2D shadowMap)
{
	float shadow = 0.0;
    // perform perspective divide
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	if (projCoords.z > 1.0)
	{
		shadow = 0.0;
		return shadow;
	}
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 

	// check whether current frag pos is in shadow
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
	        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
	    }    
	}
	shadow /= 9.0;

    return shadow;
}

float CalculatePointShadow(vec3 worldPos, vec3 viewPos, vec3 lightPos, samplerCube shadowCubeMap)
{
	vec3 sampleOffsetDirections[20] = vec3[]
	(
	   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);  

	float shadow = 0.0;
    // get vector between world position to light position
	vec3 worldToLight = worldPos - lightPos;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(worldToLight);

	float bias = 0.15;
	int samples = 20;
	float viewDistance = length(viewPos - worldPos);
	float diskRadius = (1.0 + (viewDistance / sceneConstant.farPlane)) / 25.0;  
	for(int i = 0; i < samples; ++i)
	{
		// use the light to fragment vector to sample from the depth map    
	    float closestDepth = texture(shadowCubeMap, worldToLight + sampleOffsetDirections[i] * diskRadius).r;
		// it is currently in linear range between [0,1]. Re-transform back to original value
	    closestDepth *= sceneConstant.farPlane;   // undo mapping [0;1]
	    if(currentDepth - bias > closestDepth)
	        shadow += 1.0;
	}
	shadow /= float(samples); 

    return shadow;
}