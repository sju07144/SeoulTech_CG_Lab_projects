#include "Utility.h"

void CheckCompileErrors(uint32_t id, std::string type)
{
	int success;
	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(id, 1024, nullptr, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" <<
				infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(id, 1024, nullptr, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" <<
				infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

void SetBool(uint32_t programID, const std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}
void SetInt(uint32_t programID, const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}
void SetFloat(uint32_t programID, const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void SetVec2(uint32_t programID, const std::string& name, const glm::vec2& value) 
{
    glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void SetVec2(uint32_t programID, const std::string& name, float x, float y) 
{
    glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
}
void SetVec3(uint32_t programID, const std::string& name, const glm::vec3& value) 
{
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void SetVec3(uint32_t programID, const std::string& name, float x, float y, float z) 
{
    glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
}
void SetVec4(uint32_t programID, const std::string& name, const glm::vec4& value) 
{
    glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void SetVec4(uint32_t programID, const std::string& name, float x, float y, float z, float w) 
{
    glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
}
void SetMat2(uint32_t programID, const std::string& name, const glm::mat2& mat) 
{
    glUniformMatrix2fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void SetMat3(uint32_t programID, const std::string& name, const glm::mat3& mat) 
{
    glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void SetMat4(uint32_t programID, const std::string& name, const glm::mat4& mat) 
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

std::vector<std::string> Split(std::string input, char delimiter)
{
	std::vector<std::string> answer;
	std::stringstream ss(input);
	std::string temp;

	while (getline(ss, temp, delimiter)) 
	{
		answer.push_back(temp);
	}

	return answer;
}

void SaveScreenshotToPNG(const std::string& filename, uint32_t width, uint32_t height)
{
	cv::Mat image(height, width, CV_8UC3);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (image.step & 3) ? 1 : 4);

	// set length of one complete row in destination data (doesn't need to equal img.cols)
	// glPixelStorei(GL_PACK_ROW_LENGTH, image.step / static_cast<int>(image.elemSize()));

	glReadPixels(0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	cv::flip(image, image, 0);

	cv::imwrite(filename, image);
}
