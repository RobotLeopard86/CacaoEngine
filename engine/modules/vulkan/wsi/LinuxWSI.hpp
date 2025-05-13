#pragma once

#include "../Module.hpp"
#include "LinuxRouter.hpp"

namespace Cacao {
#ifdef HAS_X11
	void XConnect();
#endif
#ifdef HAS_WAYLAND
	void WaylandConnect();
#endif
}