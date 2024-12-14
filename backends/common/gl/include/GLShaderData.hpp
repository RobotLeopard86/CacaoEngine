#pragma once

#include "glad/gl.h"

#include <string>

namespace Cacao {
	//Struct for data required for an OpenGL shader
	struct Shader::ShaderData {
		GLuint gpuID;
		std::string vertexCode, fragmentCode;
		bool unusedTransform;
		GLuint transformLoc;

		static GLuint uboIndexCounter;
	};
}