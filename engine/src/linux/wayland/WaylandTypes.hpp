#pragma once

#include "Cacao/Window.hpp"
#include "WindowImplBase.hpp"

#include <wayland-client.h>
#include "libdecor.h"
#include "xdg-output-unstable-v1-client-protocol.h"

namespace Cacao {
	class WaylandWindowImpl : public Window::Impl {
	  public:
		wl_display* display = nullptr;
		wl_compositor* compositor = nullptr;
		wl_registry* reg = nullptr;
		wl_surface* surf = nullptr;
		zxdg_output_manager_v1* outMgr = nullptr;
		zxdg_output_v1* out = nullptr;
		wl_surface_listener surfListener = {};
		zxdg_output_v1_listener outListener = {};
		glm::ivec2 outputSize = {0, 0};
		libdecor* decor = nullptr;
		libdecor_frame* frame = nullptr;
		bool configured = false;
		glm::uvec2 lastKnownContentSize;

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