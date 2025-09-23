#pragma once

#include "impl/Cubemap.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLCubemapImpl : public Cubemap::Impl {
	  public:
		std::optional<std::shared_future<void>> Realize(bool& success) override;
		void DropRealized() override;
		bool DoWaitAsyncForSync() const override {
			return false;
		}

		//Texture object
		GLuint gpuTex;
	};
}