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

	mat4 invView;
	mat4 invProjection;

	vec3 cameraPos;
	vec3 cameraFront;

	vec2 screenSize;

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
	vec2 texCoords;

	vec4 lightSpacePos[4];
};

#define NUM_DIRECTIONAL_LIGHTS 0
#define NUM_POINT_LIGHTS 0
#define NUM_SPOT_LIGHTS 0

in VS_OUT vs_out;

out vec4 color;

uniform bool isUsingTexture;
uniform bool isUsingNormalMap;
uniform bool enableImageBasedLighting;
uniform bool enableShadow;

uniform Material material;
// uniform sampler2D positionMap0;
uniform sampler2D albedoMap0;
uniform sampler2D normalMap0;
uniform sampler2D metallicMap0;
uniform sampler2D roughnessMap0;
uniform sampler2D metallicRoughnessMap0;
uniform sampler2D aoMap0;
uniform sampler2D maskMap0;
uniform sampler2D depthMap0;
uniform sampler2D viewMap0;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform sampler2D shadowMaps[4];
uniform samplerCube shadowCubeMaps[4];

uniform SceneConstant sceneConstant;

const float PI = 3.14159265359f;

vec3 ScreenToWorld(vec2 screenCoords, float depth);
vec3 WorldPosFromDepth(float depth);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);   

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
	vec3 V, vec3 N, vec3 R);

float CalculateShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir, sampler2D shadowMap);
float CalculatePointShadow(vec3 worldPos, vec3 viewPos, vec3 lightPos, samplerCube shadowCubeMap);

void main()
{
	vec3 albedo = texture(albedoMap0, vs_out.texCoords).rgb;
	float alphaChannel = texture(albedoMap0, vs_out.texCoords).a;
	float metallic = texture(metallicMap0, vs_out.texCoords).r;
	float roughness = texture(roughnessMap0, vs_out.texCoords).r;
	float ao = texture(aoMap0, vs_out.texCoords).r;
	float mask = texture(maskMap0, vs_out.texCoords).r;
	float depth = texture(depthMap0, vs_out.texCoords).r;

	// vec2 pixelCoords = gl_FragCoord.xy;
	// vec3 position = ScreenToWorld(pixelCoords, depth);
	// vec3 position = texture(positionMap0, vs_out.texCoords).rgb;
	vec3 position = vs_out.worldPos;
	// vec3 position = WorldPosFromDepth(depth);

	vec3 N = 2.0 * texture(normalMap0, vs_out.texCoords).rgb - 1.0;

	vec3 V = normalize(sceneConstant.cameraPos + normalize(sceneConstant.cameraFront) * 0.3 - position);
	// vec3 V = 2.0 * texture(viewMap0, vs_out.texCoords).rgb - 1.0;
	// vec3 V = normalize(sceneConstant.cameraPos - position);
	vec3 R = reflect(-V, N);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)   
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 pointLightColor = vec3(0.0);	

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		pointLightColor += CalculatePointLight(sceneConstant.pointLights[i],
			shadowCubeMaps[i],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	vec3 directionalLightColor = vec3(0.0);	

	for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)
	{
		directionalLightColor += CalculateDirectionalLight(sceneConstant.directionalLights[i],
			vs_out.lightSpacePos[i], shadowMaps[0],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	vec3 spotLightColor = vec3(0.0);	

	for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
	{
		spotLightColor += CalculateSpotLight(sceneConstant.spotLights[i],
			albedo, metallic, roughness, ao, F0,
			V, N);
	}

	// ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).

	vec3 ambient = vec3(0.002);
	ambient = ambient * albedo * ao;
	if (enableImageBasedLighting)
		ambient = CalculateImageBasedLight(albedo, metallic, roughness, ao, F0, V, N, R);
	
	vec3 result = ambient + pointLightColor + directionalLightColor + spotLightColor;

	// First, initialize background color. Then, Check whether image-based light can be enabled or not.
	color = vec4(albedo, alphaChannel);

	// For debugging
	// vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	// 
	// vec3 kS = F;
	// vec3 kD = 1.0 - kS;
	// kD *= 1.0 - metallic;
	// 
	// vec3 irradiance = texture(irradianceMap, N).rgb;
	// vec3 diffuse = irradiance * albedo;
	// 
	// const float MAX_REFLECTION_LOD = 4.0;
	// vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	// vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	// vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	// 
	// result = kD * diffuse;
	
	if (mask == 1.0)
	{		
		// HDR tonemapping
		result = result / (result + vec3(1.0));
		// gamma correct
		result = pow(result, vec3(1.0 / 2.2));
	
		color = vec4(result, alphaChannel);
		// color = vec4(prefilteredColor, 1.0);
	}
}

vec3 ScreenToWorld(vec2 screenCoords, float depth)
{
	// Normalize screen coordinates to NDC
	vec2 normalizedCoords = (2.0 * screenCoords - sceneConstant.screenSize) / sceneConstant.screenSize;

	float z = depth * 2.0 - 1.0;

	// Add homogeneous coordinate
	vec4 ndcCoords = vec4(normalizedCoords, z, 1.0);

	// Convert to view space
	vec4 viewCoords = sceneConstant.invProjection * ndcCoords;

	// Divide by W to get homogeneous coordinates
	viewCoords /= viewCoords.w;

	// Convert to world coordinates
	vec4 worldCoords = sceneConstant.invView * viewCoords;

	return worldCoords.xyz;
}
vec3 WorldPosFromDepth(float depth)
{
	float z = depth * 2.0 - 1.0;

	vec4 clipSpacePosition = vec4(vs_out.texCoords * 2.0 - 1.0, z, 1.0);

	vec4 viewSpacePosition = sceneConstant.invProjection * clipSpacePosition;

	viewSpacePosition /= viewSpacePosition.w;

	vec4 worldSpacePosition = sceneConstant.invView * viewSpacePosition;

	return worldSpacePosition.xyz;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
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
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
	vec3 F = FresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
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
	kD *= (1.0 - metallic);

	// scale light by NdotL
	float NdotL = max(dot(normal, lightDir), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

vec3 CalculatePointLight(PointLight light, samplerCube shadowCubeMap,
	vec3 albedo, float metallic, float roughness, float ao, vec3 F0,
	vec3 viewDir, vec3 normal)
{
	// calculate per-light radiance
	vec3 lightDir = normalize(light.position - vs_out.worldPos);
	float distance = length(light.position - vs_out.worldPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = light.diffuse * attenuation;

	// calculate shadow.
	float shadow = 0.0;
	// if (enableShadow)
	// 	shadow = CalculatePointShadow(vs_out.worldPos, sceneConstant.cameraPos, light.position, shadowCubeMap);
	// else
	// 	shadow = 0.0f;

	return (1.0 - shadow) * CookTorrance(lightDir, radiance,
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
	float shadow = 0.0;
	if (enableShadow)
		shadow = CalculateShadow(lightSpacePos, normal, lightDir, shadowMap);
	else
		shadow = 0.0;

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
	vec3 V, vec3 N, vec3 R)
{
	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	
	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
	
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