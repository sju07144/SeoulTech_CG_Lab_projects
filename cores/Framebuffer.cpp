#include "Framebuffer.h"

uint32_t Framebuffer::GetFramebuffer()
{
	return mFramebuffer;
}

void Framebuffer::CreateFramebuffer(uint32_t width, uint32_t height, 
	uint32_t colorAttachmentCount, bool isDefaultTexture2D,
	bool isDepthAttachment, bool isStencilAttachment)
{
	// Create one framebuffer.
	glGenFramebuffers(1, &mFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

	// Check when using just null-texture2D(e.g., not texture cube)
	if (isDefaultTexture2D)
	{
		for (uint32_t i = 0; i < colorAttachmentCount; i++)
		{
			Texture colorTexture;
			colorTexture.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, true, width, height);
			mColorTextures.push_back(colorTexture);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mColorTextures[i].GetTexture(), 0);
		}
	}
		

	mDepthStencilBuffer.CreateRenderbuffer(width, height, isStencilAttachment);

	// Check whether stencil buffer uses.
	if (isStencilAttachment)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilBuffer.GetRenderbuffer());
	else
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilBuffer.GetRenderbuffer());

	// Check whether framebuffer completes.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::DeleteFramebuffer()
{
	mDepthStencilBuffer.DeleteRenderbuffer();
	for (auto& colorTexture : mColorTextures)
		colorTexture.DeleteTexture();
	glDeleteFramebuffers(1, &mFramebuffer);
}

void Framebuffer::ResizeDepthStencilBuffer(uint32_t width, uint32_t height)
{
	mDepthStencilBuffer.ResizeRenderbuffer(width, height);
}