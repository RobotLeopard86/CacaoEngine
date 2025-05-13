#include "Module.hpp"

namespace Cacao {
	std::shared_ptr<PALModule> CreateVulkanModule() {
		vulkan = std::make_shared<VulkanModule>();
		return std::static_pointer_cast<PALModule>(vulkan);
	}

	void VulkanModule::Destroy() {
		vulkan.reset();
	}

	void VulkanModule::Init() {
		didInit = true;
	}

	void VulkanModule::Term() {
		didInit = false;
	}

	void VulkanModule::Disconnect() {
		connected = false;

		//Destroy swapchain
		for(const vk::ImageView& view : swapchain.views) {
			dev.destroyImageView(view);
		}
		dev.destroySwapchainKHR(swapchain.chain);

		//Destroy surface
		instance.destroySurfaceKHR(surface);
	}
}