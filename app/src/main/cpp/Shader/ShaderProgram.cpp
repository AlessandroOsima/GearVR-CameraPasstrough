
#include "ShaderProgram.h"
#include <sstream>
#include <iostream>
#include "GLUtilities.h"
#include <assert.h>
#include <android/asset_manager.h>
#include "Logger/Logger.h"
#include "Resources.h"
#include <vector>

ShaderProgram::ShaderProgram()
{

	ProgramID = glCreateProgram();
    glCheckError("glCreateProgram()");

}


ShaderProgram::ShaderProgram(ShaderProgram && Program) : 
	VertexShaderID(std::move(Program.VertexShaderID)),
	FragmentShaderID(std::move(Program.FragmentShaderID)),
	ProgramID(std::move(Program.ProgramID)),
	Usable(std::move(Program.Usable))
{
	//Invalidate the other program to avoid shader deletion on destruction
	Program.Usable = false;
}

void ShaderProgram::operator=(ShaderProgram && Program)
{
	VertexShaderID = std::move(Program.VertexShaderID);
	FragmentShaderID = std::move(Program.FragmentShaderID);
	ProgramID = std::move(Program.ProgramID);
	Usable = std::move(Program.Usable);

	//Invalidate the other program to avoid shader deletion on destruction
	Program.Usable = false;
}

ShaderProgram::~ShaderProgram()
{
	if (Usable)
	{
		glCheckFunction(glDeleteShader(VertexShaderID));
		glDeleteShader(FragmentShaderID);
		glDeleteProgram(ProgramID);
	}
}

bool ShaderProgram::BindBufferToUniform(uint32_t BufferID, uint32_t BindingLocation, const char * BindingName)
{
	unsigned int shaderUniformID = GetUniformBufferBlockIndex(BindingName);

	if (shaderUniformID != GL_INVALID_INDEX)
	{
		glCheckFunction(glUniformBlockBinding(GetShaderProgramID(), shaderUniformID, BindingLocation));
		glCheckFunction(glBindBufferBase(GL_UNIFORM_BUFFER, BindingLocation, BufferID));

		return true;
	}

	return false;
}

bool ShaderProgram::CompileShaderFile(const std::string &ShaderFilename, ShaderType Type,
									  AAssetManager *AssetManager)
{

    std::string fullPath = ResourceManager::GetShadersForlder()+"/"+ShaderFilename;

	AAsset * asset = AAssetManager_open(AssetManager, fullPath.c_str(), AASSET_MODE_BUFFER);

    if (!asset)
    {
        std::stringstream stream;
        stream << "Unable to open shader file " << ShaderFilename << std::ends;
        Logger::GetLogger().LogString(stream.str(), LogType::ERROR);

        return false;
    }

    auto lenght = AAsset_getLength(asset);

    std::vector<char> data(lenght);

    AAsset_read(asset, (void*)data.data(), lenght);
    data.push_back('\0');

    AAsset_close(asset);

    return CompileShader(data.data(), Type);
}

bool ShaderProgram::CompileShader(const std::string & ShaderSource, ShaderType Type)
{

	const char * fileContent = ShaderSource.c_str();

	GLint compilationSuccess = 0;
	GLuint CompiledID = 0;

	switch (Type)
	{
	case ShaderType::Vertex:
	{
		CompiledID = VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        glCheckError("glCreateShader(GL_VERTEX_SHADER)");
		glCheckFunction(glShaderSource(VertexShaderID, 1, &fileContent, NULL));
		glCheckFunction(glCompileShader(VertexShaderID));
		break;
	}
	case ShaderType::Fragment:
	{
		CompiledID = FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        glCheckError("glCreateShader(GL_FRAGMENT_SHADER)");
		glCheckFunction(glShaderSource(FragmentShaderID, 1, &fileContent, NULL));
		glCheckFunction(glCompileShader(FragmentShaderID));

		break;
	}

	default:
		break;
	}

	glCheckFunction(glGetShaderiv(CompiledID, GL_COMPILE_STATUS, &compilationSuccess));

	GLint logSize = 0;
	glCheckFunction(glGetShaderiv(CompiledID, GL_INFO_LOG_LENGTH, &logSize));

	if (logSize > 1)
	{
		char * logMessage = new char[logSize];

		glCheckFunction(glGetShaderInfoLog(CompiledID, logSize, NULL, logMessage));

		std::stringstream stream;
		stream << "Shader " << CompiledID << " compile log: \n" << logMessage << std::ends;
		Logger::GetLogger().LogString(stream.str(), LogType::LOG);

		delete[] logMessage;
	}

	if (compilationSuccess)
	{
		glCheckFunction(glAttachShader(ProgramID, CompiledID));
	}

	Usable = compilationSuccess == 1;

	return Usable;
}

void ShaderProgram::LinkProgram()
{
	glCheckFunction(glLinkProgram(ProgramID));

	GLint linkSuccess;
	glCheckFunction(glGetProgramiv(ProgramID, GL_LINK_STATUS, &linkSuccess));

	std::stringstream stream;
	stream << "Shader " << ProgramID << " link result is : " << linkSuccess << std::ends;
	Logger::GetLogger().LogString(stream.str(), LogType::LOG);

	GLint logSize;
	glCheckFunction(glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &logSize));

	if (logSize > 1)
	{
		char * logMessage = new char[logSize];

		glCheckFunction(glGetProgramInfoLog(ProgramID, logSize, NULL, logMessage));

		stream.clear();
		stream << "Shader " << ProgramID << " link log: \n" << logMessage << std::ends;
		Logger::GetLogger().LogString(stream.str(), LogType::LOG);

		delete[] logMessage;
	}
}

void ShaderProgram::UseProgram()
{
	glCheckFunction(glUseProgram(ProgramID));
}

unsigned int ShaderProgram::GetUniformBufferBlockIndex(const std::string & BlockName)
{
	unsigned int index = glGetUniformBlockIndex(ProgramID, BlockName.c_str());
	glCheckError("glGetUniformBlockIndex(ProgramID, BlockName.c_str())");
	return index;
}

int ShaderProgram::GetUniformIndex(const std::string & UniformName) const
{
	return glGetUniformLocation(ProgramID, UniformName.c_str());
}

void ShaderProgram::SetUniformMatrix4(int Location, const glm::mat4 &Mat4Val)
{
	glUniformMatrix4fv(Location, 1, false, glm::value_ptr(Mat4Val));
}


void ShaderProgram::SetUniformMatrix3(int Location, const glm::mat3 &Mat3Val)
{
	glUniformMatrix3fv(Location, 1, false, glm::value_ptr(Mat3Val));
}

void ShaderProgram::SetUniformVector4(int Location, const glm::vec4 &Vec4Val)
{
	glUniform4f(Location, Vec4Val.x, Vec4Val.y, Vec4Val.z, Vec4Val.w);
}

void ShaderProgram::SetUniformVector3(int Location, const glm::vec3 &Vec3Val)
{
	glUniform3f(Location, Vec3Val.x, Vec3Val.y, Vec3Val.z);
}

void ShaderProgram::SetUniformFloat(int Location, float FloatVal)
{
	glUniform1f(Location, FloatVal);
}

void ShaderProgram::SetUniformInt(int Location, int IntVal)
{
	glCheckFunction(glUniform1i(Location, IntVal))
}

void ShaderProgram::SetUniformMatrix4(int Location, float (**Mat4))
{
    glUniformMatrix4fv(Location, 1, false, &(Mat4[0][0]));
}
