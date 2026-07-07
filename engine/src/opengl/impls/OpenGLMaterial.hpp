#pragma once

#include "impl/Material.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLMaterialImpl : public Material::Impl {
	  public:
		void Upload() override;
	};
}