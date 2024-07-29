#pragma once

#include "SDL3/SDL.h"

//Defines a set of "hook" functions that are called during the common SDL code to execute graphics API-specific code

namespace Cacao {
	void ConfigureSDL();
	SDL_WindowFlags GetSDLFlags();
	void SetupGraphicsAPI(SDL_Window* win);
	void CleanupGraphicsAPI();
	void ResizeViewport(SDL_Window* win);
}