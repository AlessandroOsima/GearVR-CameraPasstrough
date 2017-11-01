#pragma once
#include "GLUtilities.h"
#include <string>
#include <GLES3/gl3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm//gtc/type_ptr.hpp>
#include <android/asset_manager.h>

enum  class ShaderType
{
	Vertex,
	Fragment
};

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(ShaderProgram && Program);
	void operator=(ShaderProgram && Program);
	~ShaderProgram();
	
	inline unsigned int GetShaderProgramID() const
	{
		return ProgramID;
	}

	bool BindBufferToUniform(uint32_t BufferID, uint32_t BindingLocation, const char * BindingName);

	bool CompileShaderFile(const std::string &ShaderFilename, ShaderType Type,
                               AAssetManager *AssetManager);
    bool CompileShader(const std::string & ShaderSource, ShaderType Type);

	void LinkProgram();
	void UseProgram();

	unsigned int GetUniformBufferBlockIndex(const std::string & BlockName);
	int GetUniformIndex(const std::string & UniformName) const;

	void SetUniformMatrix4(int Location, const glm::mat4 & Mat4Val);
	void SetUniformMatrix4(int Location, float ** Mat4);
	void SetUniformMatrix3(int Location, const glm::mat3 & Mat3Val);
	void SetUniformVector4(int Location, const glm::vec4 & Vec4Val);
	void SetUniformVector3(int Location, const glm::vec3 & Vec3Val);
	void SetUniformFloat(int Location, float FloatVal);
	void SetUniformInt(int Location, int IntVal);

private:

	GLuint VertexShaderID;
	GLuint FragmentShaderID;
	GLuint ProgramID;
	bool Usable = false;
};

