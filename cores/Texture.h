#pragma once
#include "Stdafx.h"

struct TextureInfo
{
	// texture format
	GLenum textureInternalFormat = GL_RGB;
	GLenum textureFormat = GL_RGB;

	// wrapping type
	GLenum wrapSType;
	GLenum wrapTType;
	GLenum wrapRType; // using texture cube

	// filter type
	GLenum minFilterType;
	GLenum magFilterType;

	// is null data
	bool nullData = false;

	// texture size
	int width = 0, height = 0;

	// can generate mipmap
	bool isMipmap = false;

	// is HDR;
	bool isHDR = false;

	// is border color
	bool isBorderColor = false;
	std::array<float, 4> borderColor;
};

class Texture
{
public:
	Texture() = default;
	Texture(const std::string& textureFileName);
	Texture(const std::vector<std::string>& cubeMapFileNames);

	void SetTextureFileName(const std::string& textureFileName);
	void SetCubeMapFileName(const std::vector<std::string>& cubeMapFileNames);
	std::string GetTextureFileName();
	uint32_t GetTexture();

	void CreateTexture2D(GLenum wrapSType, GLenum wrapTType, 
		GLenum minFilterType, GLenum magFilterType, bool isMipmap = false,
		bool nullData = false, int width = 0, int height = 0, GLenum textureFormat = GL_RGB);
	void CreateHDRTexture2D(GLenum wrapSType, GLenum wrapTType,
		GLenum minFilterType, GLenum magFilterType, bool isMipmap = false,
		bool nullData = false, int width = 0, int height = 0, GLenum textureInternalFormat = GL_RGB16F, GLenum textureFormat = GL_RGB);
	void CreateTextureCube(GLenum wrapSType, GLenum wrapTType, GLenum wrapRType, 
		GLenum minFilterType, GLenum magFilterType, bool isMipmap = false,
		bool nullData = false, int width = 0, int height = 0, GLenum textureFormat = GL_RGB);
	void CreateHDRTextureCube(GLenum wrapSType, GLenum wrapTType, GLenum wrapRType,
		GLenum minFilterType, GLenum magFilterType, bool isMipmap = false,
		bool nullData = false, int width = 0, int height = 0, GLenum textureInternalFormat = GL_RGB16F, GLenum textureFormat = GL_RGB);

	void CreateTexture2D(const TextureInfo& textureSetup);
	void CreateHDRTexture2D(const TextureInfo& textureSetup);
	void CreateTextureCube(const TextureInfo& textureSetup);
	void CreateHDRTextureCube(const TextureInfo& textureSetup);

	void DeleteTexture();
private:
	uint32_t mTexture = 0;
	std::string mTextureFileName = "";
	std::vector<std::string> mCubeMapFileNames;
};