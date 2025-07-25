#include "VulkanTex2D.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> VulkanTex2DImpl::Realize() {
		//TODO
		return std::nullopt;
	}

	void VulkanTex2DImpl::DropRealized() {
		//TODO
	}

	Tex2D::Impl* VulkanModule::ConfigureTex2D() {
		return new VulkanTex2DImpl();
	}
}