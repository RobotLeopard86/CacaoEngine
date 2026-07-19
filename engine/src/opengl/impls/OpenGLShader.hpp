#pragma once

#include "OpenGLModule.hpp"
#include "impl/Shader.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLShaderImpl : public Shader::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;
		void Bind(OpenGLCommandBuffer* glcb, bool transparent);

		//GLSL shader source code (generated from IR)
		std::string vertexGLSL, fragmentGLSL;

		//Shader program object
		GLuint program;

		//Object data UBO (optional)
		GLuint ubo;
		GLuint uboBinding;
	};
}