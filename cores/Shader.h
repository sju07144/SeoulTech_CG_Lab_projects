#pragma once
#include "Stdafx.h"
#include "Utility.h"

class Shader
{
public:
	Shader() = default;
	~Shader();

	uint32_t GetShaderID();

	void CompileShader(const std::string& fileName, GLenum shaderType);
private:
	uint32_t mShaderID = 0;
};