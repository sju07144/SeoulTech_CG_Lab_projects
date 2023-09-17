#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Mesh.h"
#include "Stdafx.h"
#include "Texture.h"
#include "Utility.h"

struct ModelComponent
{
	glm::mat4 world = glm::mat4(1.0f);
	Mesh mesh;
	Material material;
	bool isTexture = false;
	std::vector<Texture> albedoMaps;
	std::vector<Texture> specularMaps;
	std::vector<Texture> normalMaps;
	std::vector<Texture> metallicMaps;
	std::vector<Texture> roughnessMaps;
};


class Model
{
public:
	Model() = default;
	Model(const std::string& path);
	Model(const Model& model) = delete;
	Model operator=(const Model& model) = delete;
	~Model();

	void LoadModel(const std::string& path);

	std::vector<ModelComponent> GetModelComponents();
	std::vector<ModelComponent>& GetModelComponentsByReference();
private:
	void ParseDirectoryName(const std::string& path);

	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene);

	void LoadTexture(aiMaterial* mat, aiTextureType textureType, 
		const std::string& textureName, std::vector<Texture>& textureContainer);
private:
	std::vector<ModelComponent> mModelComponents;

	std::vector<Texture> mTextureLoaded; // using texture loading optimization.

	std::string mDirectoryName;
};