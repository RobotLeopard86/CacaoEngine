#include "Module.hpp"
#include "Win32Types.hpp"
#include "WSI.hpp"
#include "Cacao/Exceptions.hpp"

#define wini Window::Get().impl->win

namespace Cacao {
	void CreateSurface() {
		Check<ExternalException>(vulkan->physDev.getWin32PresentationSupportKHR(0) == VK_TRUE, "Device does not support Vulkan presentation!");
		vk::Win32SurfaceCreateInfoKHR wci({}, wini->hInst, wini->hWnd);
		vulkan->surface = vulkan->instance.createWin32SurfaceKHR(wci);
	}
}