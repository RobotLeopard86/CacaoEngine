#include "VulkanModule.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"
#include "impl/Window.hpp"
#include "WSI.hpp"

#include <array>

constexpr std::array<vk::Format, 2> acceptableFormats {{vk::Format::eB8G8R8A8Srgb, vk::Format::eR8G8B8A8Srgb}};

namespace Cacao {
	void VulkanModule::Connect() {
		//Create surface
		const std::string provID = IMPL(Window).ProviderID();
#ifdef _WIN32
		if(provID.compare("win32") == 0) {
			Win32_CreateSurface();
			goto continue_connect;
		}
#endif
#ifdef HAS_X11
		if(provID.compare("x11") == 0) {
			X_CreateSurface();
			goto continue_connect;
		}
#endif
#ifdef HAS_WAYLAND
		if(provID.compare("wayland") == 0) {
			Wayland_CreateSurface();
			goto continue_connect;
		}
#endif
		Check<MiscException>(false, "Unsupported windowing provider for Vulkan!");

	continue_connect:
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
		GPU_IMPL(Vulkan).GenSwapchain();

		connected = true;
	}
}