#include "Shader.h"

Shader::~Shader()
{
	glDeleteShader(mShaderID);
}

uint32_t Shader::GetShaderID()
{
	return mShaderID;
}

void Shader::CompileShader(const std::string& fileName, GLenum shaderType)
{
	std::string shaderCode;

	std::ifstream shaderFile;

	// ensure ifstream objects can throw exceptions.
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		shaderFile.open(fileName);

		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();

		shaderFile.close();

		shaderCode = shaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}

	const char* rawShaderCode = shaderCode.c_str();
	
	mShaderID = glCreateShader(shaderType);
	glShaderSource(mShaderID, 1, &rawShaderCode, nullptr);
	glCompileShader(mShaderID);
	switch (shaderType)
	{
	case GL_VERTEX_SHADER:
		CheckCompileErrors(mShaderID, "VERTEX");
		break;
	case GL_TESS_CONTROL_SHADER:
		CheckCompileErrors(mShaderID, "TESS_CONTROL");
		break;
	case GL_TESS_EVALUATION_SHADER:
		CheckCompileErrors(mShaderID, "TESS_EVALUATION");
		break;
	case GL_GEOMETRY_SHADER:
		CheckCompileErrors(mShaderID, "GEOMETRY");
		break;
	case GL_FRAGMENT_SHADER:
		CheckCompileErrors(mShaderID, "FRAGMENT");
		break;
	case GL_COMPUTE_SHADER:
		CheckCompileErrors(mShaderID, "COMPUTE");
		break;

	}
}