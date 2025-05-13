#include "LinuxWSI.hpp"
#include "LinuxRouter.hpp"
#include "Cacao/Window.hpp"

namespace Cacao {
	void VulkanModule::Connect() {
		if(Window::Get().impl->useX) {
			XConnect();
		} else {
			WaylandConnect();
		}
		connected = true;
	}
}