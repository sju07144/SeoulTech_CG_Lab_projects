#pragma once
#include "BasicGeometryGenerator.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Stdafx.h"
#include "Texture.h"
#include "Utility.h"

class ImageBasedLight
{
public:
	ImageBasedLight() = default;

	ImageBasedLight(
		const std::string& shaderDirectoryName,
		const std::string& equirectangularMapFileName
	);

	void SetDirectoryAndFileName(
		const std::string& shaderDirectoryName,
		const std::string& equirectangularMapFileName,
		const std::string& hdrName
	);

	void BuildResources();
	void DeleteResources();

	void Draw(uint32_t equiToCubeProgramID, uint32_t irradianceProgramID, uint32_t prefilterProgramID, uint32_t brdfLUTProgramID);

	Texture* GetCubeMap();
	Texture* GetIrradianceMap();
	Texture* GetPreFilteredEnvironmentMap();
	Texture* GetBRDFLookUpTable();
private:
	void BuildMeshes();
	void BuildTextures();
	void BuildFramebuffers();
	void BuildMatrices();
	void BuildRenderItems();

	void DrawCubeMap(uint32_t programID);
	void DrawIrradianceMap(uint32_t programID);
	void DrawPreFilteredEnvironmentMap(uint32_t programID);
	void DrawBRDFLookUpTable(uint32_t programID);

	Mesh mCubeMapBox;
	Mesh mBrdfLUTQuad;

	RenderItem mImageBasedLightRenderItem;

	std::string mShaderDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\shaders\\";
	std::string mTextureDirectoryName = "..\\..\\resources\\HDR_resources\\ibr";
	std::string mEquirectangularMapFileName;

	std::string mHDRName;

	std::array<std::string, 6> mTextureNames = { "right", "left", "top", "bottom", "back", "front" };

	uint32_t cubeMapVertexShaderID;
	std::unordered_map<std::string, Texture> mBasicTextures;

	Framebuffer mCaptureFramebuffer;

	glm::mat4 mCaptureProjection;
	std::array<glm::mat4, 6> mCaptureViews;

	static const uint32_t mCubeMapSize;
	static const uint32_t mIrradianceMapSize;
	static const uint32_t mPrefilterMapSize;
	static const uint32_t mBrdfLUTSize;
};