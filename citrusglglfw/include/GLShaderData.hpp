#pragma once

#include "glad/gl.h"
#include <map>

namespace Citrus {
	//Struct for data required for an OpenGL shader
	struct GLShaderData {
		GLint gpuID;
		std::map<const char*, GLint> uniformLocations;
		std::string vertexCode, fragmentCode;
	};
}