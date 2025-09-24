#include "VulkanCubemap.hpp"
#include "Cacao/Engine.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanCubemapImpl::Realize(bool& success) {
		//TODO
	}

	void VulkanCubemapImpl::DropRealized() {
		//TODO
	}

	Cubemap::Impl* VulkanModule::ConfigureCubemap() {
		return new VulkanCubemapImpl();
	}
}