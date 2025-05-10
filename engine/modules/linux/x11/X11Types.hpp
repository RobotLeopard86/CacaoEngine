#pragma once

#include "Cacao/Window.hpp"

#include <xcb/xcb.h>

#include <string>

namespace Cacao {
	class X11Common {
	  public:
		xcb_connection_t* connection;
		xcb_window_t window;

		void CreateWindow();
		void DestroyWindow();
		void HandleEvents();
		bool Minimized();
		const glm::uvec2 ContentAreaSize();
		void Visibility(bool visible);
		void Title(const std::string& title);
		void Resize(const glm::uvec2& size);
		void ModeChange(Window::Mode mode);
		void SaveWinPos();
		void SaveWinSize();
		void RestoreWin();
	};
}