#include "LinuxWSI.hpp"
#include "WSI.hpp"
#include "Cacao/Window.hpp"

namespace Cacao {
	void CreateSurface() {
		if(Window::Get().impl->useX) {
			XConnect();
		} else {
			WaylandConnect();
		}
	}
}