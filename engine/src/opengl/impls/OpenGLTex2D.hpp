#pragma once

#include "impl/Tex2D.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLTex2DImpl : public Tex2D::Impl {
	  public:
		std::optional<std::shared_future<void>> Realize(bool& success) override;
		void DropRealized() override;
		bool DoWaitAsyncForSync() const override {
			return false;
		}

		//Texture object
		GLuint gpuTex;

		//Texture format
		GLenum format;
	};
}