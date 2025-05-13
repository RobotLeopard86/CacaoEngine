#include "Module.hpp"
#include "x11/X11Types.hpp"
#include "LinuxRouter.hpp"
#include "Cacao/Exceptions.hpp"

#define xi Window::Get().impl->x

namespace Cacao {
	void XConnect() {
		//Get window attributes
		xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(xi->connection, xi->window);
		xcb_get_window_attributes_reply_t* reply = xcb_get_window_attributes_reply(xi->connection, cookie, nullptr);
		Check<ExternalException>(reply != nullptr, "Failed to get window attributes!");

		//Make surface
		Check<ExternalException>(vulkan->physDev.getXcbPresentationSupportKHR(0, xi->connection, reply->visual) == VK_TRUE, "Device does not support Vulkan presentation to X11!");
		vk::XcbSurfaceCreateInfoKHR xci({}, xi->connection, xi->window);
		vulkan->surface = vulkan->instance.createXcbSurfaceKHR(xci);

		//Free XCB reply
		free(reply);
	}
}