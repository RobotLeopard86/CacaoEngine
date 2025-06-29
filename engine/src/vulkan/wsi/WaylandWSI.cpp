#include "Module.hpp"
#include "wayland/WaylandTypes.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"
#include "WSI.hpp"

namespace Cacao {
	void Wayland_CreateSurface() {
		Check<ExternalException>(vulkan->physDev.getWaylandPresentationSupportKHR(0, WIN_IMPL(Wayland).display) == VK_TRUE, "Device does not support Vulkan presentation to Wayland!");
		vk::WaylandSurfaceCreateInfoKHR wci({}, WIN_IMPL(Wayland).display, WIN_IMPL(Wayland).surf);
		vulkan->surface = vulkan->instance.createWaylandSurfaceKHR(wci);
	}
}