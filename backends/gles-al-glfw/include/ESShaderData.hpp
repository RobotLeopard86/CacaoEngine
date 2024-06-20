#pragma once

#include "glad/gles2.h"

#include <string>

namespace Cacao {
	//Struct for data required for an OpenGL ES shader
	struct ESShaderData : public NativeData {
		GLuint gpuID, cacaoDataUBO;
		std::string vertexCode, fragmentCode;

		static GLuint uboIndexCounter;
	};
}