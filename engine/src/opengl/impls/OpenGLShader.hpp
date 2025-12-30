#pragma once

#include "impl/Shader.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLShaderImpl : public Shader::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//GLSL shader source code (generated from IR)
		std::string vertexGLSL, fragmentGLSL;

		//Shader program object
		GLuint program;
	};
}