#pragma once

#include "glad/gl.h"

#include <string>

namespace Cacao {
	//Struct for data required for an OpenGL shader
	struct GLShaderData : public NativeData {
		GLuint gpuID, cacaoDataUBO;
		bool uboLinked;
		std::string vertexCode, fragmentCode;
	};
}