#pragma once

#include "glad/gl.h"
#include <string>

namespace Cacao {
	//Struct for data required for an OpenGL shader
	struct GLShaderData : public NativeData {
		GLuint gpuID;
		GLuint ubo;
		bool uboInUse;
		std::string vertexCode, fragmentCode;

		~GLShaderData() {
			if(uboInUse) glDeleteBuffers(1, &ubo);
		}
	};
}