#pragma once

#include "Cacao/Window.hpp"

namespace Cacao {
	class X11Common;
	class WaylandCommon;

	struct Window::Impl {
#ifdef HAS_X11
		std::unique_ptr<X11Common> x;
#endif
#ifdef HAS_WAYLAND
		std::unique_ptr<WaylandCommon> wl;
#endif
		bool useX = false;

		Impl();
		~Impl();
	};
}