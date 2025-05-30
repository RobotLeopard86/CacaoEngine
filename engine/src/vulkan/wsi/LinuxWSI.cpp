#include "LinuxWSI.hpp"
#include "WSI.hpp"
#include "Cacao/Window.hpp"
#include "ImplAccessor.hpp"

namespace Cacao {
	void CreateSurface() {
		if(IMPL(Window).useX) {
			XConnect();
		} else {
			WaylandConnect();
		}
	}
}