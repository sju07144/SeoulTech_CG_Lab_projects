#include "Renderbuffer.h"

uint32_t Renderbuffer::GetRenderbuffer()
{
	return mRenderbuffer;
}

void Renderbuffer::DeleteRenderbuffer()
{
	glDeleteRenderbuffers(1, &mRenderbuffer);
}

void Renderbuffer::CreateRenderbuffer(uint32_t width, uint32_t height, bool isStencilAttachment)
{
	glGenRenderbuffers(1, &mRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);

	mIsStencilAttachment = isStencilAttachment;

	if (mIsStencilAttachment)
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
}

void Renderbuffer::ResizeRenderbuffer(uint32_t width, uint32_t height)
{
	glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
	if (mRenderbuffer != 0)
	{
		if (mIsStencilAttachment)
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	}
	else
		std::cout << "Create renderbuffer first!!" << std::endl;
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
