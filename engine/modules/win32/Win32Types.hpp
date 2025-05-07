#pragma once

#include "Cacao/Window.hpp"

#include <Windows.h>

namespace Cacao {
	struct WindowsCommon {
		HINSTANCE hInst;
		HWND hWnd;
	};

	struct Window::Impl {
		std::unique_ptr<WindowsCommon> mac;
	};
}