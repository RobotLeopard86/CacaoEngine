#include "VulkanCubemap.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> VulkanCubemapImpl::Realize(bool& success) {
		//TODO
		return std::nullopt;
	}

	void VulkanCubemapImpl::DropRealized() {
		//TODO
	}

	Cubemap::Impl* VulkanModule::ConfigureCubemap() {
		return new VulkanCubemapImpl();
	}
}