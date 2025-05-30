#include "Module.hpp"
#include "wayland/WaylandTypes.hpp"
#include "LinuxRouter.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"

namespace Cacao {
	void WaylandConnect() {
		Check<ExternalException>(vulkan->physDev.getWaylandPresentationSupportKHR(0, IMPL(Window).wl->display) == VK_TRUE, "Device does not support Vulkan presentation to Wayland!");
		vk::WaylandSurfaceCreateInfoKHR wci({}, IMPL(Window).wl->display, IMPL(Window).wl->surf);
		vulkan->surface = vulkan->instance.createWaylandSurfaceKHR(wci);
	}
}