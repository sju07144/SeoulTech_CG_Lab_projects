#include "Renderbuffer.h"

uint32_t Renderbuffer::GetRenderbuffer()
{
	return mRenderbuffer;
}

void Renderbuffer::DeleteRenderbuffer()
{
	glDeleteRenderbuffers(1, &mRenderbuffer);
}

void Renderbuffer::CreateRenderbuffer(uint32_t width, uint32_t height)
{
	glGenRenderbuffers(1, &mRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}