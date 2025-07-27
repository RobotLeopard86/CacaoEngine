#include "OpenGLCubemap.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> OpenGLCubemapImpl::Realize() {
		//TODO
		return std::nullopt;
	}

	void OpenGLCubemapImpl::DropRealized() {
		//TODO
	}

	Cubemap::Impl* OpenGLModule::ConfigureCubemap() {
		return new OpenGLCubemapImpl();
	}
}