#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"

Texture::Texture(const std::string& textureFileName)
{
	mTextureFileName = textureFileName;
}

Texture::Texture(const std::vector<std::string>& cubeMapFileNames)
{
	mCubeMapFileNames = cubeMapFileNames;
}

// public methods
void Texture::SetTextureFileName(const std::string& textureFileName)
{
	mTextureFileName = textureFileName;
}
void Texture::SetCubeMapFileName(const std::vector<std::string>& cubeMapFileNames)
{
	mCubeMapFileNames = cubeMapFileNames;
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
	GLenum minFilterType, GLenum magFilterType, bool isMipmap,
	bool nullData, int width, int height, GLenum textureFormat)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	// Set the texture wrapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTType);

	// Set texture filtering parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterType);

	if (nullData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, nullptr);
		if (isMipmap)
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
			if (isMipmap)
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

void Texture::CreateHDRTexture2D(GLenum wrapSType, GLenum wrapTType, 
	GLenum minFilterType, GLenum magFilterType, bool isMipmap,
	bool nullData, int width, int height, GLenum textureInternalFormat, GLenum textureFormat)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	// Set the texture wrapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTType);

	// Set texture filtering parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterType);

	if (nullData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, textureInternalFormat, width, height, 0, textureFormat, GL_FLOAT, nullptr);
		if (isMipmap)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		// Load image, create texture and generate mipmaps.
		int _width, _height, _nrChannels;
		stbi_set_flip_vertically_on_load(true);
		float* data = stbi_loadf(mTextureFileName.c_str(), &_width, &_height, &_nrChannels, 0);
		if (data)
		{
			GLenum internalFormat, format;
			if (_nrChannels == 1)
			{
				internalFormat = GL_RED;
				format = GL_RED;
			}
			else if (_nrChannels == 3)
			{
				internalFormat = GL_RGB16F;
				format = GL_RGB;
			}
			else if (_nrChannels == 4)
			{
				internalFormat = GL_RGBA16F;
				format = GL_RGB16F;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, GL_FLOAT, data);
			if (isMipmap)
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

void Texture::CreateTextureCube(GLenum wrapSType, GLenum wrapTType, GLenum wrapRType, 
	GLenum minFilterType, GLenum magFilterType, bool isMipmap,
	bool nullData, int width, int height, GLenum textureFormat)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapSType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapTType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapRType);

	if (nullData)
	{
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				0, textureFormat, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, nullptr
			);
		}
		if (isMipmap)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	else
	{
		int _width, _height, nrChannels;
		// stbi_set_flip_vertically_on_load(true);
		for (uint32_t i = 0; i < mCubeMapFileNames.size(); i++)
		{
			unsigned char* data = stbi_load(mCubeMapFileNames[i].c_str(), &_width, &_height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, textureFormat, _width, _height, 0, textureFormat, GL_UNSIGNED_BYTE, data
				);
				if (isMipmap)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << mCubeMapFileNames[i] << std::endl;
				stbi_image_free(data);
			}
		}
	}
}

void Texture::CreateHDRTextureCube(GLenum wrapSType, GLenum wrapTType, GLenum wrapRType, 
	GLenum minFilterType, GLenum magFilterType, bool isMipmap,
	bool nullData, int width, int height, GLenum textureInternalFormat, GLenum textureFormat)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapSType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapTType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapRType);

	if (nullData)
	{
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, textureInternalFormat, width, height, 0, textureFormat, GL_FLOAT, nullptr
			);
		}
		if (isMipmap)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	else
	{
		int _width, _height, nrChannels;
		// stbi_set_flip_vertically_on_load(true);
		for (uint32_t i = 0; i < mCubeMapFileNames.size(); i++)
		{
			float* data = stbi_loadf(mCubeMapFileNames[i].c_str(), &_width, &_height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, textureInternalFormat, _width, _height, 0, textureFormat, GL_FLOAT, data
				);
				if (isMipmap)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << mCubeMapFileNames[i] << std::endl;
				stbi_image_free(data);
			}
		}
	}
}

void Texture::CreateTexture2D(const TextureInfo& textureSetup)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	// Set the texture wrapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureSetup.wrapSType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureSetup.wrapTType);

	// Set texture filtering parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureSetup.minFilterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureSetup.magFilterType);

	if (textureSetup.nullData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, textureSetup.textureFormat, textureSetup.width, textureSetup.height, 
			0, textureSetup.textureFormat, GL_UNSIGNED_BYTE, nullptr);
		if (textureSetup.isMipmap)
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
			if (textureSetup.isMipmap)
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
void Texture::CreateHDRTexture2D(const TextureInfo& textureSetup)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	// Set the texture wrapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureSetup.wrapSType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureSetup.wrapTType);

	// Set texture filtering parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureSetup.minFilterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureSetup.magFilterType);

	if (textureSetup.nullData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, textureSetup.textureInternalFormat, textureSetup.width, textureSetup.height, 
			0, textureSetup.textureFormat, GL_FLOAT, nullptr);
		if (textureSetup.isMipmap)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		// Load image, create texture and generate mipmaps.
		int _width, _height, _nrChannels;
		stbi_set_flip_vertically_on_load(true);
		float* data = stbi_loadf(mTextureFileName.c_str(), &_width, &_height, &_nrChannels, 0);
		if (data)
		{
			GLenum internalFormat, format;
			if (_nrChannels == 1)
			{
				internalFormat = GL_RED;
				format = GL_RED;
			}
			else if (_nrChannels == 3)
			{
				internalFormat = GL_RGB16F;
				format = GL_RGB;
			}
			else if (_nrChannels == 4)
			{
				internalFormat = GL_RGBA16F;
				format = GL_RGB16F;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, GL_FLOAT, data);
			if (textureSetup.isMipmap)
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
void Texture::CreateTextureCube(const TextureInfo& textureSetup)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, textureSetup.minFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, textureSetup.magFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, textureSetup.wrapSType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, textureSetup.wrapTType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, textureSetup.wrapRType);

	if (textureSetup.nullData)
	{
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, textureSetup.textureInternalFormat, textureSetup.width, textureSetup.height, 
				0, textureSetup.textureFormat, GL_UNSIGNED_BYTE, nullptr
			);
		}
		if (textureSetup.isMipmap)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	else
	{
		int _width, _height, nrChannels;
		// stbi_set_flip_vertically_on_load(true);
		for (uint32_t i = 0; i < mCubeMapFileNames.size(); i++)
		{
			unsigned char* data = stbi_load(mCubeMapFileNames[i].c_str(), &_width, &_height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, textureSetup.textureInternalFormat, _width, _height, 
					0, textureSetup.textureFormat, GL_UNSIGNED_BYTE, data
				);
				if (textureSetup.isMipmap)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << mCubeMapFileNames[i] << std::endl;
				stbi_image_free(data);
			}
		}
	}
}
void Texture::CreateHDRTextureCube(const TextureInfo& textureSetup)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, textureSetup.minFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, textureSetup.magFilterType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, textureSetup.wrapSType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, textureSetup.wrapTType);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, textureSetup.wrapRType);

	if (textureSetup.nullData)
	{
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, textureSetup.textureInternalFormat, textureSetup.width, textureSetup.height, 
				0, textureSetup.textureFormat, GL_FLOAT, nullptr
			);
		}
		if (textureSetup.isMipmap)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	else
	{
		int _width, _height, nrChannels;
		// stbi_set_flip_vertically_on_load(true);
		for (uint32_t i = 0; i < mCubeMapFileNames.size(); i++)
		{
			float* data = stbi_loadf(mCubeMapFileNames[i].c_str(), &_width, &_height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, textureSetup.textureInternalFormat, _width, _height, 
					0, textureSetup.textureFormat, GL_FLOAT, data
				);
				if (textureSetup.isMipmap)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << mCubeMapFileNames[i] << std::endl;
				stbi_image_free(data);
			}
		}
	}
}

void Texture::DeleteTexture()
{
	glDeleteTextures(1, &mTexture);
}