#pragma once

#include "Cacao/Window.hpp"

#define win Window::Get()

namespace Cacao {
	class X11Common;
	class WaylandCommon;

	struct Window::Impl {
		std::unique_ptr<X11Common> x;
		std::unique_ptr<WaylandCommon> wl;
		bool useX = false;

		Impl();
		~Impl();
	};
}