#pragma once

#include "SDL3/SDL_video.h"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Struct for data required for an SDL window
	struct Window::WindowData {
		SDL_Window* win;
	};
}