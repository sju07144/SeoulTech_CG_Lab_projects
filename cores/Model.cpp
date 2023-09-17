#include "Model.h"

Model::Model(const std::string& path)
{
	LoadModel(path);
}

Model::~Model()
{
	for (auto& modelComponent : mModelComponents)
		modelComponent.mesh.DeleteMemory();
}

void Model::LoadModel(const std::string& path)
{
	ParseDirectoryName(path);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		throw std::runtime_error("Cannot read the model!");

	ProcessNode(scene->mRootNode, scene);
}

std::vector<ModelComponent> Model::GetModelComponents()
{
	return mModelComponents;
}

std::vector<ModelComponent>& Model::GetModelComponentsByReference()
{
	return mModelComponents;
}

void Model::ParseDirectoryName(const std::string& path)
{
	auto tokens = Split(path, '\\');
	std::stringstream ss;
	for (auto i = tokens.begin(); i != tokens.end() - 1; i++)
	{
		ss << *i;
		ss << '\\';
	}

	mDirectoryName = ss.str();
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		auto meshIndex = node->mMeshes[i];
		ProcessMesh(scene->mMeshes[meshIndex], scene);
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene);
}
void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	ModelComponent modelComponent;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumVertices);

	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	modelComponent.mesh = Mesh(vertices, indices);
	modelComponent.mesh.ConfigureMesh(); // dynamic한 경우, LoadModel, ProcessNode, ProcessMesh 함수에 GLenum usage 파라미터 추가

	if (scene->HasMaterials())
	{
		if (mesh->mMaterialIndex >= 0)
		{
			Material materialData{};

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			aiColor3D ka(0.0f, 0.0f, 0.0f);
			material->Get(AI_MATKEY_COLOR_AMBIENT, ka);
			materialData.ka = glm::vec3(ka.r, ka.g, ka.b);
			
			aiColor3D kd(0.0f, 0.0f, 0.0f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
			materialData.kd = glm::vec3(kd.r, kd.g, kd.b);
			
			aiColor3D ks(0.0f, 0.0f, 0.0f);
			material->Get(AI_MATKEY_COLOR_SPECULAR, ks);
			materialData.ks = glm::vec3(ks.r, ks.g, ks.b);
			
			materialData.metallic = 0.3f;
			materialData.roughness = 0.8f;
			materialData.ao = 0.1f;
			
			modelComponent.material = materialData;

			LoadTexture(material, aiTextureType_DIFFUSE, "albedoMap", modelComponent.albedoMaps);
			LoadTexture(material, aiTextureType_SPECULAR, "specularMap", modelComponent.specularMaps);
		}
	}

	mModelComponents.push_back(modelComponent);
}

void Model::LoadTexture(aiMaterial* mat, aiTextureType textureType, 
	const std::string& textureName,  std::vector<Texture>& textureContainer)
{
	uint32_t textureCount = mat->GetTextureCount(textureType);
	if (textureCount == 0)
		return;

	Texture texture;
	for (uint32_t i = 0; i < textureCount; i++)
	{
		aiString path;

		mat->GetTexture(textureType, i, &path); // revise path

		std::string pathName;
		pathName.assign(path.C_Str());

		pathName = mDirectoryName + pathName;

		bool skip = false;
		for (auto& texture : mTextureLoaded)
		{
			const auto& textureFileName = texture.GetTextureFileName();
			if (!textureFileName.empty() && textureFileName == pathName)
			{
				textureContainer.push_back(texture);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			texture.SetTextureFileName(pathName);
			texture.CreateTexture2D(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

			textureContainer.push_back(texture);

			mTextureLoaded.push_back(texture);
		}
	}
}