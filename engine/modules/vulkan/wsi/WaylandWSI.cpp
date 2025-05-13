#include "Module.hpp"
#include "wayland/WaylandTypes.hpp"
#include "LinuxRouter.hpp"
#include "Cacao/Exceptions.hpp"

#define wli Window::Get().impl->wl

namespace Cacao {
	void WaylandConnect() {
		Check<ExternalException>(vulkan->physDev.getWaylandPresentationSupportKHR(0, wli->display) == VK_TRUE, "Device does not support Vulkan presentation to Wayland!");
		vk::WaylandSurfaceCreateInfoKHR wci({}, wli->display, wli->surf);
		vulkan->surface = vulkan->instance.createWaylandSurfaceKHR(wci);
	}
}