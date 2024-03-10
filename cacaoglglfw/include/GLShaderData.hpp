#pragma once

#include "glad/gl.h"

#include "Utilities/MiscUtils.hpp"

#include <string>

namespace Cacao {
	//Struct for data required for an OpenGL shader
	struct GLShaderData : public NativeData {
		GLuint gpuID, cacaoDataUBO;
		std::string vertexCode, fragmentCode;
	};
}