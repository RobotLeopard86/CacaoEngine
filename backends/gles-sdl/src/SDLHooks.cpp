#include "SDLHooks.hpp"

#include "SDL3/SDL.h"
#include "GLHeaders.hpp"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "SDLWindowData.hpp"

#include "tinyfiledialogs.h"

namespace Cacao {
	void ConfigureSDL() {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);

		if(std::string(SDL_GetCurrentVideoDriver()).compare("x11") == 0) {
			int result = tinyfd_messageBox("Cacao Engine Warning", "On X11 with OpenGL ES, gamma correction does not apply correctly. Visuals may appear at the incorrect brightness level.\nThis issue does not occur in a Wayland session or with other backends.\n\nDo you want to proceed with visual errors?", "yesno", "warning", 0);
			if(result == 0) exit(0);
		}
	}

	SDL_WindowFlags GetSDLFlags() {
		return SDL_WINDOW_OPENGL;
	}

	void SetupGraphicsAPI(SDL_Window* win) {
		//Create and make current a GL context
		SDL_GL_MakeCurrent(win, SDL_GL_CreateContext(win));
		int gladResult = gladLoadGLES2(SDL_GL_GetProcAddress);
		EngineAssert(gladResult != 0, "Failed to load OpenGL ES!");
	}

	void CleanupGraphicsAPI() {
		SDL_GL_DeleteContext(SDL_GL_GetCurrentContext());
	}

	void ResizeViewport(SDL_Window* win) {
		int fbx, fby;
		SDL_GetWindowSizeInPixels(win, &fbx, &fby);
		glViewport(0, 0, fbx, fby);
	}

	void Window::UpdateVSyncState() {
		SDL_GL_SetSwapInterval(useVSync);
	}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
		SDL_GL_SwapWindow(nativeData->win);
	}
}