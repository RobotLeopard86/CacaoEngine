#pragma once

#include "impl/Cubemap.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLCubemapImpl : public Cubemap::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//Texture object
		GLuint gpuTex;
	};
}