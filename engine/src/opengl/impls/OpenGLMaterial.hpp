#pragma once

#include "impl/Material.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLMaterialImpl : public Material::Impl {
	  public:
		void Upload(std::unique_ptr<CommandBuffer>& cmd) override;
	};
}