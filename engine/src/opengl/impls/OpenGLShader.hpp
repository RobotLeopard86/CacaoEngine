#pragma once

#include "impl/Shader.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLShaderImpl : public Shader::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//Shader program object
		GLuint program;
	};
}