#include "Graphics/Window.hpp"

#include "glad/gles2.h"
#include "SDL3/SDL.h"
#include "glm/vec2.hpp"

#include "Core/Exception.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"
#include "SDLWindowData.hpp"
#include "KeyAndMouseLUT.hpp"

//For my sanity
#define nd ((SDLWindowData*)nativeData)

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

	//Utility for resizing the GL viewport
	void ResizeGLViewport(SDL_Window* win) {
		int fbx, fby;
		SDL_GetWindowSizeInPixels(win, &fbx, &fby);
		glViewport(0, 0, fbx, fby);
	}

	void Window::Open(std::string title, glm::uvec2 initialSize, bool startVisible, WindowMode mode) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");

		size = initialSize;

		//Initialize GLFW
		EngineAssert(SDL_Init(SDL_INIT_VIDEO) == 0, "Could not initialize SDL library, no window can be created.");

		//Set OpenGL ES settings
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		//Create window
		SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
		if(!startVisible) flags |= SDL_WINDOW_HIDDEN;
		nativeWindow = SDL_CreateWindow(windowTitle.c_str(), initialSize.x, initialSize.y, flags);
		EngineAssert(nativeWindow != NULL, "Failed to open the window!");

		//Set the window mode
		SetMode(mode);

		//Set window VSync state
		SetVSyncEnabled(true);

		//Initialize OpenGL
		nativeData = new SDLWindowData();
		nd->glContext = SDL_GL_CreateContext((SDL_Window*)nativeWindow);
		SDL_GL_MakeCurrent((SDL_Window*)nativeWindow, nd->glContext);
		int gladResult = gladLoadGLES2(SDL_GL_GetProcAddress);
		EngineAssert(gladResult != 0, "Failed to load OpenGL!");

		isOpen = true;
	}

	void Window::Close() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't close the window, it's not open!");

		//Delete the SDL OpenGL context
		SDL_GL_DeleteContext(nd->glContext);
		delete nativeData;

		//Destroy the window
		SDL_DestroyWindow((SDL_Window*)nativeWindow);

		//Clean up GLFW
		SDL_Quit();

		isOpen = false;
	}

	void Window::UpdateVSyncState() {
		SDL_GL_SetSwapInterval(useVSync);
	}

	void Window::UpdateWindowSize() {
		//Update GLFW window size
		SDL_SetWindowSize((SDL_Window*)nativeWindow, size.x, size.y);

		//Update OpenGL framebuffer size
		ResizeGLViewport((SDL_Window*)nativeWindow);
	}

	void Window::UpdateVisibilityState() {
		if(isVisible) {
			SDL_ShowWindow((SDL_Window*)nativeWindow);
		} else {
			SDL_HideWindow((SDL_Window*)nativeWindow);
		}
	}

	void Window::UpdateModeState(WindowMode lastMode) {
		if(lastMode == WindowMode::Window) {
			SDL_GetWindowPosition((SDL_Window*)nativeWindow, &windowedPosition.x, &windowedPosition.y);
		}
		switch(mode) {
			case WindowMode::Window:
				SDL_SetWindowSize((SDL_Window*)nativeWindow, size.x, size.y);
				SDL_SetWindowFullscreen((SDL_Window*)nativeWindow, false);
				SDL_SetWindowPosition((SDL_Window*)nativeWindow, windowedPosition.x, windowedPosition.y);
				SDL_SetWindowBordered((SDL_Window*)nativeWindow, SDL_TRUE);
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
				SDL_SetWindowFullscreenMode((SDL_Window*)nativeWindow, fullscreenModesV[0]);
				SDL_SetWindowFullscreen((SDL_Window*)nativeWindow, true);

				//Free the mode pointer
				SDL_free(fullscreenModesV);
				break;
			}
			case WindowMode::Borderless:
				SDL_SetWindowFullscreenMode((SDL_Window*)nativeWindow, NULL);
				SDL_SetWindowFullscreen((SDL_Window*)nativeWindow, true);
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
					ResizeGLViewport((SDL_Window*)nativeWindow);
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

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't present frame to a closed window!");
		SDL_GL_SwapWindow((SDL_Window*)nativeWindow);
	}

	void Window::SetTitle(std::string title) {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the title of a closed window!");
		SDL_SetWindowTitle((SDL_Window*)nativeWindow, title.c_str());
	}
}