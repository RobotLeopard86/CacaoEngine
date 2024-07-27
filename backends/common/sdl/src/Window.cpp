#include "Graphics/Window.hpp"

#include "SDL3/SDL.h"
#include "glm/vec2.hpp"

#include "Core/Exception.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"
#include "SDLWindowData.hpp"
#include "KeyAndMouseLUT.hpp"
#include "SDLHooks.hpp"
#include "UI/Text.hpp"

namespace Cacao {

	//Initialize static members
	Window* Window::instance = nullptr;
	bool Window::instanceExists = false;

	//Simple friend struct to set window size without function that causes more size events
	struct WindowResizer {
		friend Window;

		void Resize(glm::uvec2 size) {
			ChangeSize(Window::GetInstance(), size);
		}
	};

	//Singleton accessor
	Window* Window::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new Window();
			instanceExists = true;
		}

		return instance;
	}

	void Window::Open(std::string title, glm::uvec2 initialSize, bool startVisible, WindowMode mode) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");

		size = initialSize;

		//Create native data
		nativeData.reset(new WindowData());

		//Initialize GLFW
		EngineAssert(SDL_Init(SDL_INIT_VIDEO) == 0, "Could not initialize SDL library, no window can be created.");

		//Configure SDL prior to window creation
		ConfigureSDL();

		//Create window
		SDL_WindowFlags flags = GetSDLFlags() | SDL_WINDOW_RESIZABLE;
		if(!startVisible) flags |= SDL_WINDOW_HIDDEN;
		nativeData->win = SDL_CreateWindow(windowTitle.c_str(), initialSize.x, initialSize.y, flags);
		EngineAssert(nativeData->win != NULL, "Failed to open the window!");

		//Set the window mode
		SetMode(mode);

		//Set window VSync state
		SetVSyncEnabled(true);

		//Initialize graphics API
		SetupGraphicsAPI(nativeData->win);

		isOpen = true;
	}

	void Window::Close() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't close the window, it's not open!");

		//Clean up the graphics API
		CleanupGraphicsAPI();

		//Destroy the window
		SDL_DestroyWindow(nativeData->win);

		//Clean up GLFW
		SDL_Quit();

		isOpen = false;
	}

	void Window::UpdateWindowSize() {
		//Update GLFW window size
		SDL_SetWindowSize(nativeData->win, size.x, size.y);

		//Update OpenGL framebuffer size
		ResizeViewport(nativeData->win);
	}

	void Window::UpdateVisibilityState() {
		if(isVisible) {
			SDL_ShowWindow(nativeData->win);
		} else {
			SDL_HideWindow(nativeData->win);
		}
	}

	void Window::UpdateModeState(WindowMode lastMode) {
		if(lastMode == WindowMode::Window) {
			SDL_GetWindowPosition(nativeData->win, &windowedPosition.x, &windowedPosition.y);
		}
		switch(mode) {
			case WindowMode::Window:
				SDL_SetWindowSize(nativeData->win, size.x, size.y);
				SDL_SetWindowFullscreen(nativeData->win, false);
				SDL_SetWindowPosition(nativeData->win, windowedPosition.x, windowedPosition.y);
				SDL_SetWindowBordered(nativeData->win, SDL_TRUE);
				break;
			case WindowMode::Fullscreen: {
				//Get the fullscreen display modes
				int fullscreenModesC;
				const SDL_DisplayMode** fullscreenModesV = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &fullscreenModesC);

				//Confirm that we have at least one
				if(fullscreenModesC < 1) {
					mode = lastMode;
					CheckException(false, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "There are no fullscreen modes available to use!")
				}

				//Set to fullscreen
				SDL_SetWindowFullscreenMode(nativeData->win, fullscreenModesV[0]);
				SDL_SetWindowFullscreen(nativeData->win, true);

				//Free the mode pointer
				SDL_free(fullscreenModesV);
				break;
			}
			case WindowMode::Borderless:
				SDL_SetWindowFullscreenMode(nativeData->win, NULL);
				SDL_SetWindowFullscreen(nativeData->win, true);
				break;
		}
	}

	void Window::Update() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't update closed window!");

		//Continuously fetch and process SDL events
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT: {
					Event e("WindowClose");
					EventManager::GetInstance()->Dispatch(e);
					break;
				}
				case SDL_EVENT_WINDOW_FOCUS_GAINED: {
					Event e("WindowFocus");
					EventManager::GetInstance()->Dispatch(e);
					break;
				}
				case SDL_EVENT_WINDOW_FOCUS_LOST: {
					Event e("WindowUnfocus");
					EventManager::GetInstance()->Dispatch(e);
					break;
				}
				case SDL_EVENT_WINDOW_RESIZED: {
					if(Window::GetInstance()->GetCurrentMode() == WindowMode::Window) {
						WindowResizer().Resize({event.window.data1, event.window.data2});
					}
					ResizeViewport(nativeData->win);
					DataEvent<glm::uvec2> wre("WindowResize", {event.window.data1, event.window.data2});
					EventManager::GetInstance()->Dispatch(wre);
					break;
				}
				case SDL_EVENT_MOUSE_MOTION: {
					DataEvent<glm::vec2> mme("MouseMove", {event.motion.x, event.motion.y});
					EventManager::GetInstance()->Dispatch(mme);
					break;
				}
				case SDL_EVENT_MOUSE_WHEEL: {
					DataEvent<glm::vec2> mse("MouseScroll", {event.wheel.x, event.wheel.y});
					EventManager::GetInstance()->Dispatch(mse);
					break;
				}
				case SDL_EVENT_KEY_UP: {
					DataEvent<int> kue("KeyUp", SDLKey2Cacao(event.key.keysym.sym));
					EventManager::GetInstance()->Dispatch(kue);
					break;
				}
				case SDL_EVENT_KEY_DOWN: {
					DataEvent<int> kde("KeyDown", SDLKey2Cacao(event.key.keysym.sym));
					EventManager::GetInstance()->Dispatch(kde);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_UP: {
					DataEvent<int> mre("MouseRelease", SDLMouseButton2Cacao(event.button.button));
					EventManager::GetInstance()->Dispatch(mre);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_DOWN: {
					DataEvent<int> mpe("MousePress", SDLMouseButton2Cacao(event.button.button));
					EventManager::GetInstance()->Dispatch(mpe);
					break;
				}
			}
		}
	}

	void Window::SetTitle(std::string title) {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the title of a closed window!");
		SDL_SetWindowTitle(nativeData->win, title.c_str());
	}

	void RegisterWindowingExceptions() {}

	glm::uvec2 Text::GetMonitorDPI() {
		const SDL_DisplayMode* dm = SDL_GetDesktopDisplayMode(SDL_GetDisplayForWindow(Window::GetInstance()->nativeData->win));
		return (glm::uvec2 {(unsigned int)(dm->w), (unsigned int)(dm->h)} * 96u);
	}
}