#include "Module.hpp"
#include "Win32Types.hpp"
#include "WSI.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"

namespace Cacao {
	void CreateSurface() {
		Check<ExternalException>(vulkan->physDev.getWin32PresentationSupportKHR(0) == VK_TRUE, "Device does not support Vulkan presentation!");
		vk::Win32SurfaceCreateInfoKHR wci({}, IMPL(Window).win->hInst, IMPL(Window).win->hWnd);
		vulkan->surface = vulkan->instance.createWin32SurfaceKHR(wci);
	}
}