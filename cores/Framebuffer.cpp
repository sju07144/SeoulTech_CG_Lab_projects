#include "Framebuffer.h"

uint32_t Framebuffer::GetFramebuffer()
{
	return mFramebuffer;
}

void Framebuffer::CreateFramebuffer(uint32_t width, uint32_t height, uint32_t colorAttachmentCount, bool isDepthStencilAttachment)
{
	glGenFramebuffers(1, &mFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

	for (uint32_t i = 0; i < colorAttachmentCount; i++)
	{
		Texture colorTexture;
		colorTexture.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, true, width, height);
		mColorTextures.push_back(colorTexture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mColorTextures[i].GetTexture(), 0);
	}
		

	mDepthStencilBuffer.CreateRenderbuffer(width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilBuffer.GetRenderbuffer());

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