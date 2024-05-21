#pragma once
#include "Stdafx.h"

class Mesh;
class Texture;

struct Vertex
{
	Vertex() = default;
	Vertex(float px, float py, float pz,
		float nx, float ny, float nz,
		float u, float v)
		: position(glm::vec3(px, py, pz)),
		normal(glm::vec3(nx, ny, nz)),
		texCoord(glm::vec2(u, v)),
		tangent(glm::vec3(0.0f))
	{ }
	Vertex(float px, float py, float pz,
		float nx, float ny, float nz,
		float u, float v,
		float tx, float ty, float tz)
		: position(glm::vec3(px, py, pz)),
		normal(glm::vec3(nx, ny, nz)),
		texCoord(glm::vec2(u, v)),
		tangent(glm::vec3(tx, ty, tz))
	{ }

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;
};

struct DirectionalLight
{
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 direction;

	glm::mat4 lightSpaceMatrix; // for shadow

	static constexpr int maxNumDirectionalLights = 4;
};

struct PointLight
{
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 position;

	std::array<glm::mat4, 6> lightSpaceMatrices; // for shadow

	float constant;
	float linear;
	float quadratic;

	static constexpr int maxNumPointLights = 4;
};

struct SpotLight
{
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 direction;
	glm::vec3 position;

	std::array<glm::mat4, 6> lightSpaceMatrices; // for shadow

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;

	static constexpr int maxNumSpotLights = 4;
};

struct Material
{
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;

	float metallic;
	float roughness;
	float ao;
};

struct RenderItem
{
	glm::mat4 world = glm::mat4(1.0f);
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	bool isTexture = false;
	std::vector<Texture*> albedoMaps;
	std::vector<Texture*> specularMaps;
	std::vector<Texture*> normalMaps;
	std::vector<Texture*> metallicMaps;
	std::vector<Texture*> roughnessMaps;
	std::vector<Texture*> metallicRoughnessMaps;
	std::vector<Texture*> aoMaps;
	std::vector<Texture*> maskMaps;
	std::vector<Texture*> depthMaps;
	std::vector<Texture*> viewMaps;

	Texture* environmentMap = nullptr;

	std::vector<Texture*> shadowMaps;
	std::vector<Texture*> shadowCubeMaps;

	Texture* equirectangularMap = nullptr;
	Texture* irradianceMap = nullptr;
	Texture* prefilterMap = nullptr;
	Texture* brdfLUT = nullptr;
};

struct Menu
{
	bool isUsingTexture = false;
	bool isUsingNormalMap = false;
	bool enableEnvironment = false;
	bool enableImageBasedLighting = false;
	bool enableShadow = false;
};

void CheckCompileErrors(uint32_t id, std::string type);

void SetBool(uint32_t programID, const std::string& name, bool value);
void SetInt(uint32_t programID, const std::string& name, int value);
void SetFloat(uint32_t programID, const std::string& name, float value);
void SetVec2(uint32_t programID, const std::string& name, const glm::vec2& value);
void SetVec2(uint32_t programID, const std::string& name, float x, float y);
void SetVec3(uint32_t programID, const std::string& name, const glm::vec3& value);
void SetVec3(uint32_t programID, const std::string& name, float x, float y, float z);
void SetVec4(uint32_t programID, const std::string& name, const glm::vec4& value);
void SetVec4(uint32_t programID, const std::string& name, float x, float y, float z, float w);
void SetMat2(uint32_t programID, const std::string& name, const glm::mat2& mat);
void SetMat3(uint32_t programID, const std::string& name, const glm::mat3& mat);
void SetMat4(uint32_t programID, const std::string& name, const glm::mat4& mat);

std::vector<std::string> Split(std::string input, char delimiter);

void SaveScreenshotToPNG(const std::string& filename, uint32_t width, uint32_t height);