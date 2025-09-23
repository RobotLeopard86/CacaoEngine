#include "VulkanModule.hpp"
#include "Win32Types.hpp"
#include "WSI.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"

namespace Cacao {
	void Win32_CreateSurface() {
		Check<ExternalException>(vulkan->physDev.getWin32PresentationSupportKHR(0) == VK_TRUE, "Device does not support Vulkan presentation!");
		vk::Win32SurfaceCreateInfoKHR wci({}, WIN_IMPL(Win32).hInst, WIN_IMPL(Win32).hWnd);
		vulkan->surface = vulkan->instance.createWin32SurfaceKHR(wci);
	}
}