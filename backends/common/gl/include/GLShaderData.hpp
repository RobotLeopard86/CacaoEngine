#pragma once

#include "glad/gl.h"

#include <string>

namespace Cacao {
	//Struct for data required for an OpenGL (ES) shader
	struct Shader::ShaderData {
		GLuint gpuID, localsUBO;
		std::string vertexCode, fragmentCode;

		static GLuint uboIndexCounter;
	};
}