#pragma once

#include "impl/Tex2D.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLTex2DImpl : public Tex2D::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//Texture object
		GLuint gpuTex;

		//Texture format
		GLenum format;
	};
}