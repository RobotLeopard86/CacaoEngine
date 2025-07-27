#include "OpenGLTex2D.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> OpenGLTex2DImpl::Realize() {
		//TODO
		return std::nullopt;
	}

	void OpenGLTex2DImpl::DropRealized() {
		//TODO
	}

	Tex2D::Impl* OpenGLModule::ConfigureTex2D() {
		return new OpenGLTex2DImpl();
	}
}