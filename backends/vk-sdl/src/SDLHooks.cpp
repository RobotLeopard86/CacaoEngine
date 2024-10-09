#include "SDLHooks.hpp"

#include "SDL3/SDL.h"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "SDLWindowData.hpp"

namespace Cacao {
	void ConfigureSDL() {}

	SDL_WindowFlags GetSDLFlags() {
		return SDL_WINDOW_VULKAN;
	}

	void SetupGraphicsAPI(SDL_Window* win) {}

	void CleanupGraphicsAPI() {}

	void ResizeViewport(SDL_Window* win) {}

	void Window::UpdateVSyncState() {}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
	}
}