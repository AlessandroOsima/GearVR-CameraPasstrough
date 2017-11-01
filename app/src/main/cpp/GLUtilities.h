#pragma once
#include "Logger/Logger.h"
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <sstream>
#include <string>

#define GL_DEBUG 1

std::string gl_error_string(GLenum const err);
std::string gl_sync_error_string(GLenum const err);

#ifdef GL_DEBUG
#define glCheckError(functionName)  { \
  	GLuint res = glGetError(); \
	if(res != GL_NO_ERROR) \
    { \
		std::stringstream stream; \
		stream << "GL error with string code : " << gl_error_string(res) << " in function : " << functionName << std::ends; \
		Logger::GetLogger().LogString(stream.str(), LogType::ERROR); \
    } \
} \

#define glCheckFunction(function)  { \
	function; \
  	glCheckError(#function) \
} \

#else

#define glCheckError(functionName) {}

#define glCheckFunction(function)  { \
	function; \
}\

#endif

