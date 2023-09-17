#pragma once
#include "Stdafx.h"

class Texture
{
public:
	Texture() = default;
	Texture(const std::string& textureFileName);

	void SetTextureFileName(const std::string& textureFileName);
	std::string GetTextureFileName();
	uint32_t GetTexture();

	void CreateTexture2D(GLenum wrapSType, GLenum wrapTType, 
		GLenum minFilterType, GLenum maxFilterType, 
		bool nullData = false, int width = 0, int height = 0, GLenum textureFormat = GL_RGB);
	void CreateTextureCube(GLenum textureFormat, GLenum wrapSType, GLenum wrapTType, GLenum wrapRType, 
		GLenum minFilterType, GLenum maxFilterType);
private:
	uint32_t mTexture = 0;
	std::string mTextureFileName = "";
};