#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"

Texture::Texture(const std::string& textureFileName)
{
	mTextureFileName = textureFileName;
}

// public methods
void Texture::SetTextureFileName(const std::string& textureFileName)
{
	mTextureFileName = textureFileName;
}
std::string Texture::GetTextureFileName()
{
	return mTextureFileName;
}
uint32_t Texture::GetTexture()
{
	return mTexture;
}

void Texture::CreateTexture2D(GLenum wrapSType, GLenum wrapTType, 
	GLenum minFilterType, GLenum maxFilterType, 
	bool nullData, int width, int height, GLenum textureFormat)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	// Set the texture wrapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTType);

	// Set texture filtering parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxFilterType);

	if (nullData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		// Load image, create texture and generate mipmaps.
		int _width, _height, _nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(mTextureFileName.c_str(), &_width, &_height, &_nrChannels, 0);
		if (data)
		{
			GLenum format;
			if (_nrChannels == 1)
				format = GL_RED;
			else if (_nrChannels == 3)
				format = GL_RGB;
			else if (_nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_2D, 0, format, _width, _height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
			stbi_image_free(data);
		}
	}
}


void Texture::CreateTextureCube(GLenum textureFormat, GLenum wrapSType, GLenum wrapTType, GLenum wrapRType, 
	GLenum minFilterType, GLenum maxFilterType)
{
}
