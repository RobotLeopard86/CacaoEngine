#pragma once

#include "OpenGLModule.hpp"
#include "impl/Shader.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLShaderImpl : public Shader::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;
		void Bind(bool transparent);

		//GLSL shader source code (generated from IR)
		std::string vertexGLSL, fragmentGLSL;

		//Shader program object
		GLuint program;

		//Uniform locations
		GLint transformUloc = -1, normalMatrixUloc = -1, handednessUloc = -1, renderModeUloc = -1;

		//Object data UBO (optional)
		GLuint ubo;
		GLuint uboBinding;
	};
}