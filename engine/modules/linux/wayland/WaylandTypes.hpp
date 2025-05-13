#pragma once

#include "Cacao/Window.hpp"

#include <wayland-client.h>
#include "libdecor.h"

namespace Cacao {
	class WaylandCommon {
	  public:
		wl_display* display = nullptr;
		wl_compositor* compositor = nullptr;
		wl_registry* reg = nullptr;
		wl_surface* surf = nullptr;
		libdecor* decor = nullptr;
		libdecor_frame* frame = nullptr;
		bool configured = false;
		bool inResize = false;

		void CreateWindow();
		void DestroyWindow();
		void HandleEvents();

		//Wayland can't check for minimization
		bool Minimized() {
			return false;
		}

		const glm::uvec2 ContentAreaSize();
		void Visibility(bool visible);
		void Title(const std::string& title);
		void Resize(const glm::uvec2& size);
		void ModeChange(Window::Mode mode);

		//Wayland can't see window position
		void SaveWinPos() {}

		void SaveWinSize();
		void RestoreWin();
	};
}