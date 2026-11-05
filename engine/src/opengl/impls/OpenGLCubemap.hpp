#pragma once

#include "impl/Cubemap.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLCubemapImpl : public Cubemap::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;

		//Texture object
		GLuint gpuTex;
	};
}