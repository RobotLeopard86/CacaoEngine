#pragma once

#include "SDL3/SDL_video.h"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Struct for data required for an SDL window
	struct SDLWindowData : public NativeData {
		SDL_GLContext glContext;
	};
}