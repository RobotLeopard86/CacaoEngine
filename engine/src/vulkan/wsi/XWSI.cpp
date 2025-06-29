#include "Module.hpp"
#include "x11/X11Types.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"
#include "WSI.hpp"

namespace Cacao {
	void X_CreateSurface() {
		//Get window attributes
		xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(WIN_IMPL(X11).connection, WIN_IMPL(X11).window);
		xcb_get_window_attributes_reply_t* reply = xcb_get_window_attributes_reply(WIN_IMPL(X11).connection, cookie, nullptr);
		Check<ExternalException>(reply != nullptr, "Failed to get window attributes!");

		//Make surface
		Check<ExternalException>(vulkan->physDev.getXcbPresentationSupportKHR(0, WIN_IMPL(X11).connection, reply->visual) == VK_TRUE, "Device does not support Vulkan presentation to X11!");
		vk::XcbSurfaceCreateInfoKHR xci({}, WIN_IMPL(X11).connection, WIN_IMPL(X11).window);
		vulkan->surface = vulkan->instance.createXcbSurfaceKHR(xci);

		//Free XCB reply
		free(reply);
	}
}