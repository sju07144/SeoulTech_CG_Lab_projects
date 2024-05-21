#include "ImageBasedLight.h"

const uint32_t ImageBasedLight::mCubeMapSize = 1024;
const uint32_t ImageBasedLight::mIrradianceMapSize = 32;
const uint32_t ImageBasedLight::mPrefilterMapSize = 128;
const uint32_t ImageBasedLight::mBrdfLUTSize = 512;

ImageBasedLight::ImageBasedLight(
	const std::string& shaderDirectoryName, 
	const std::string& equirectangularMapFileName)
	: mShaderDirectoryName(shaderDirectoryName),
	mEquirectangularMapFileName(equirectangularMapFileName)
{ }

void ImageBasedLight::SetDirectoryAndFileName(
	const std::string& shaderDirectoryName, 
	const std::string& equirectangularMapFileName,
	const std::string& hdrName)
{
	mShaderDirectoryName = shaderDirectoryName;
	mEquirectangularMapFileName = equirectangularMapFileName;
	mHDRName = hdrName;
}

void ImageBasedLight::BuildResources()
{
	BuildMeshes();
	BuildTextures();
	BuildFramebuffers();
	BuildMatrices();
	BuildRenderItems();
}
void ImageBasedLight::DeleteResources()
{
	mCaptureFramebuffer.DeleteFramebuffer();

	for (auto& texture : mBasicTextures)
		texture.second.DeleteTexture();

	mBrdfLUTQuad.DeleteMesh();
	mCubeMapBox.DeleteMesh();
}

void ImageBasedLight::Draw(uint32_t equiToCubeProgramID, uint32_t irradianceProgramID, uint32_t prefilterProgramID, uint32_t brdfLUTProgramID)
{
	DrawCubeMap(equiToCubeProgramID);
	DrawIrradianceMap(irradianceProgramID);
	DrawPreFilteredEnvironmentMap(prefilterProgramID);
	DrawBRDFLookUpTable(brdfLUTProgramID);
}

Texture* ImageBasedLight::GetCubeMap()
{
	return &mBasicTextures["cubeMap"];
}
Texture* ImageBasedLight::GetIrradianceMap()
{
	return &mBasicTextures["irradianceMap"];
}
Texture* ImageBasedLight::GetPreFilteredEnvironmentMap()
{
	return &mBasicTextures["prefilterMap"];
}
Texture* ImageBasedLight::GetBRDFLookUpTable()
{
	return &mBasicTextures["brdfLUT"];
}

void ImageBasedLight::BuildMeshes()
{
	BasicGeometryGenerator geoGenerator;
	mCubeMapBox = geoGenerator.CreateBox(2.0f, 2.0f, 2.0f);
	mCubeMapBox.ConfigureMesh();

	mBrdfLUTQuad = geoGenerator.CreateQuad(-1.0f, -1.0f, 2.0f, 2.0f, 0.0f);
	mBrdfLUTQuad.ConfigureMesh();
}

void ImageBasedLight::BuildTextures()
{
	Texture equirectengularMap(mEquirectangularMapFileName);
	std::string texName = "equirectangularMap";
	equirectengularMap.CreateHDRTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false);
	mBasicTextures.insert({ texName, std::move(equirectengularMap) });

	Texture cubeMap;
	texName = "cubeMap";
	cubeMap.CreateHDRTextureCube(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, false, true, mCubeMapSize, mCubeMapSize);
	mBasicTextures.insert({ texName, std::move(cubeMap) });

	Texture irradianceMap;
	texName = "irradianceMap";
	irradianceMap.CreateHDRTextureCube(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false, true, mIrradianceMapSize, mIrradianceMapSize);
	mBasicTextures.insert({ texName, std::move(irradianceMap) });

	Texture prefilterMap;
	texName = "prefilterMap";
	prefilterMap.CreateHDRTextureCube(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true, true, mPrefilterMapSize, mPrefilterMapSize);
	mBasicTextures.insert({ texName, std::move(prefilterMap) });

	Texture brdfLUT;
	texName = "brdfLUT";
	brdfLUT.CreateHDRTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false, true, mBrdfLUTSize, mBrdfLUTSize, GL_RG16F, GL_RG);
	mBasicTextures.insert({ texName, std::move(brdfLUT) });
}

void ImageBasedLight::BuildFramebuffers()
{
	mCaptureFramebuffer.CreateFramebuffer(512, 512, 1, false);
}

void ImageBasedLight::BuildMatrices()
{
	mCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	mCaptureViews =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
}

void ImageBasedLight::BuildRenderItems()
{
	mImageBasedLightRenderItem.mesh = &mCubeMapBox;
	mImageBasedLightRenderItem.world = glm::mat4(1.0f);
	mImageBasedLightRenderItem.equirectangularMap = &mBasicTextures["equirectangularMap"];
	mImageBasedLightRenderItem.environmentMap = &mBasicTextures["cubeMap"];
	mImageBasedLightRenderItem.irradianceMap = &mBasicTextures["irradianceMap"];
	mImageBasedLightRenderItem.prefilterMap = &mBasicTextures["prefilterMap"];
	mImageBasedLightRenderItem.brdfLUT = &mBasicTextures["brdfLUT"];
}

void ImageBasedLight::DrawCubeMap(uint32_t programID)
{
	glUseProgram(programID);

	SetMat4(programID, "sceneConstant.projection", mCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mImageBasedLightRenderItem.equirectangularMap->GetTexture());
	SetInt(programID, "equirectangularMap", 0);

	glViewport(0, 0, mCubeMapSize, mCubeMapSize); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFramebuffer.GetFramebuffer());
	mCaptureFramebuffer.ResizeDepthStencilBuffer(mCubeMapSize, mCubeMapSize);
	for (unsigned int i = 0; i < 6; i++)
	{
		SetMat4(programID, "sceneConstant.view", mCaptureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mImageBasedLightRenderItem.environmentMap->GetTexture(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto primitiveType = mImageBasedLightRenderItem.mesh->GetPrimitiveType();
		auto indexCount = mImageBasedLightRenderItem.mesh->GetIndexCount();
		auto indexFormat = mImageBasedLightRenderItem.mesh->GetIndexFormat();
		auto vertexAttribArray = mImageBasedLightRenderItem.mesh->GetVertexAttribArray();

		glBindVertexArray(vertexAttribArray);
		glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
		glBindVertexArray(0);

		std::string fileName = mTextureDirectoryName + "\\" + mHDRName + "\\cubemap\\" + mTextureNames[i] + ".png";
		SaveScreenshotToPNG(fileName, mCubeMapSize, mCubeMapSize);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, mImageBasedLightRenderItem.environmentMap->GetTexture());
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void ImageBasedLight::DrawIrradianceMap(uint32_t programID)
{
	glUseProgram(programID);

	SetMat4(programID, "sceneConstant.projection", mCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mImageBasedLightRenderItem.environmentMap->GetTexture());
	SetInt(programID, "environmentMap", 0);

	glViewport(0, 0, mIrradianceMapSize, mIrradianceMapSize); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFramebuffer.GetFramebuffer());
	mCaptureFramebuffer.ResizeDepthStencilBuffer(mIrradianceMapSize, mIrradianceMapSize);
	for (unsigned int i = 0; i < 6; i++)
	{
		SetMat4(programID, "sceneConstant.view", mCaptureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mImageBasedLightRenderItem.irradianceMap->GetTexture(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto primitiveType = mImageBasedLightRenderItem.mesh->GetPrimitiveType();
		auto indexCount = mImageBasedLightRenderItem.mesh->GetIndexCount();
		auto indexFormat = mImageBasedLightRenderItem.mesh->GetIndexFormat();
		auto vertexAttribArray = mImageBasedLightRenderItem.mesh->GetVertexAttribArray();

		glBindVertexArray(vertexAttribArray);
		glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
		glBindVertexArray(0);

		std::string fileName = mTextureDirectoryName + "\\" + mHDRName + "\\irradiancemap\\" + mTextureNames[i] + ".png";
		SaveScreenshotToPNG(fileName, mIrradianceMapSize, mIrradianceMapSize);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ImageBasedLight::DrawPreFilteredEnvironmentMap(uint32_t programID)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	glUseProgram(programID);

	SetMat4(programID, "sceneConstant.projection", mCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mImageBasedLightRenderItem.environmentMap->GetTexture());
	SetInt(programID, "environmentMap", 0);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFramebuffer.GetFramebuffer());
	uint32_t maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; mip++)
	{
		// resize framebuffer according to mip-level size.
		uint32_t mipWidth = static_cast<uint32_t>(mPrefilterMapSize * std::pow(0.5, mip));
		uint32_t mipHeight = static_cast<uint32_t>(mPrefilterMapSize * std::pow(0.5, mip));
		mCaptureFramebuffer.ResizeDepthStencilBuffer(mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
		SetFloat(programID, "roughness", roughness);
		for (unsigned int i = 0; i < 6; i++)
		{
			SetMat4(programID, "sceneConstant.view", mCaptureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mImageBasedLightRenderItem.prefilterMap->GetTexture(), mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto primitiveType = mImageBasedLightRenderItem.mesh->GetPrimitiveType();
			auto indexCount = mImageBasedLightRenderItem.mesh->GetIndexCount();
			auto indexFormat = mImageBasedLightRenderItem.mesh->GetIndexFormat();
			auto vertexAttribArray = mImageBasedLightRenderItem.mesh->GetVertexAttribArray();

			glBindVertexArray(vertexAttribArray);
			glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
			glBindVertexArray(0);

			std::string fileName = mTextureDirectoryName + "\\" + mHDRName + "\\prefiltermap\\" + mTextureNames[i] + "(" + std::to_string(mip) + ").png";
			SaveScreenshotToPNG(fileName, mipWidth, mipHeight);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ImageBasedLight::DrawBRDFLookUpTable(uint32_t programID)
{
	// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glBindTexture(GL_TEXTURE_CUBE_MAP, mImageBasedLightRenderItem.environmentMap->GetTexture());
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glUseProgram(programID);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFramebuffer.GetFramebuffer());
	mCaptureFramebuffer.ResizeDepthStencilBuffer(mBrdfLUTSize, mBrdfLUTSize);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mImageBasedLightRenderItem.brdfLUT->GetTexture(), 0);
	glViewport(0, 0, mBrdfLUTSize, mBrdfLUTSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mesh* quad = &mBrdfLUTQuad;
	auto primitiveType = quad->GetPrimitiveType();
	auto indexCount = quad->GetIndexCount();
	auto indexFormat = quad->GetIndexFormat();
	auto vertexAttribArray = quad->GetVertexAttribArray();

	glBindVertexArray(vertexAttribArray);
	glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
	glBindVertexArray(0);

	std::string fileName = mTextureDirectoryName + "\\" + mHDRName + "\\brdfLUT.png";
	SaveScreenshotToPNG(fileName, mBrdfLUTSize, mBrdfLUTSize);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
