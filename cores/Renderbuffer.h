#pragma once
#include "Stdafx.h"

class Renderbuffer
{
public:
	Renderbuffer() = default;

	uint32_t GetRenderbuffer();

	void DeleteRenderbuffer();

	void CreateRenderbuffer(uint32_t width, uint32_t height);
private:
	uint32_t mRenderbuffer = 0;
};