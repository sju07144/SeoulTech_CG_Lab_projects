#pragma once
#include "Renderbuffer.h"
#include "Stdafx.h"
#include "Texture.h"

class Framebuffer
{
public:
	Framebuffer() = default;

	uint32_t GetFramebuffer();

	void CreateFramebuffer(uint32_t width, uint32_t height, 
		uint32_t colorAttachmentCount = 1, bool isDefaultTexture2D = true,
		bool isDepthAttachment = true, bool isStencilAttachment = false);

	void DeleteFramebuffer();

	void ResizeDepthStencilBuffer(uint32_t width, uint32_t height);
private:
	uint32_t mFramebuffer = 0;

	std::vector<Texture> mColorTextures;
	Renderbuffer mDepthStencilBuffer;
};