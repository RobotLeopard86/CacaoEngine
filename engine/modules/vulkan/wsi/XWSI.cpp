#include "Module.hpp"
#include "x11/X11Types.hpp"
#include "LinuxRouter.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"

namespace Cacao {
	void XConnect() {
		//Get window attributes
		xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(IMPL(Window).x->connection, IMPL(Window).x->window);
		xcb_get_window_attributes_reply_t* reply = xcb_get_window_attributes_reply(IMPL(Window).x->connection, cookie, nullptr);
		Check<ExternalException>(reply != nullptr, "Failed to get window attributes!");

		//Make surface
		Check<ExternalException>(vulkan->physDev.getXcbPresentationSupportKHR(0, IMPL(Window).x->connection, reply->visual) == VK_TRUE, "Device does not support Vulkan presentation to X11!");
		vk::XcbSurfaceCreateInfoKHR xci({}, IMPL(Window).x->connection, IMPL(Window).x->window);
		vulkan->surface = vulkan->instance.createXcbSurfaceKHR(xci);

		//Free XCB reply
		free(reply);
	}
}