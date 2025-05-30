#include "../Module.hpp"
#include "WSI.hpp"
#include "Cacao/Exceptions.hpp"

#include <array>

constexpr std::array<vk::Format, 2> acceptableFormats {{vk::Format::eB8G8R8A8Srgb, vk::Format::eR8G8B8A8Srgb}};

namespace Cacao {
	void VulkanModule::Connect() {
		//Create surface
		CreateSurface();

		//Set format
		auto formats = physDev.getSurfaceFormatsKHR(surface);
		if(auto it = std::find_if(formats.begin(), formats.end(), [](vk::SurfaceFormatKHR sf) {
			   return sf.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && std::find(acceptableFormats.begin(), acceptableFormats.end(), sf.format) != acceptableFormats.end();
		   });
			it != formats.end()) {
			surfaceFormat = *it;
		} else {
			Check<ExternalException>(false, "The surface does not support the one of the required formats!");
		}

		//Create initial swapchain
		GenSwapchain();

		//Register swapchain regeneration consumer
		vulkan->resizer = EventConsumer([](Event& e) {
			GenSwapchain();
		});
		EventManager::Get().SubscribeConsumer("WindowResize", vulkan->resizer);

		connected = true;
	}
}