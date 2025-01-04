#include "Graphics/Window.hpp"

#include "SDL3/SDL.h"
#include "glm/vec2.hpp"

#include "Core/Exception.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"
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
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new Window();
			instanceExists = true;
		}

		return instance;
	}

	void Window::SysInit() {
		CheckException(!sysInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't initialize the already initialized windowing system!");

#ifdef __linux__
		if(auto forceX = std::getenv("CACAO_FORCE_X11"); forceX != nullptr && std::string(forceX).compare("YES") == 0) {
			SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
			goto do_init;
		}

		//Enable Wayland if possible and the user hasn't requested X11 specifically
		for(int i = 0; i < SDL_GetNumVideoDrivers(); i++) {
			if(std::string(SDL_GetVideoDriver(i)).compare("wayland") == 0) {
				SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland");
				break;
			}
		}
#endif

	do_init:
		//Initialize SDL
		EngineAssert(SDL_Init(SDL_INIT_VIDEO), "Failed to initialize SDL!");

		sysInitialized = true;
	}

	void Window::SysTerminate() {
		CheckException(sysInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't shutdown the uninitialized windowing system!");
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't shutdown the windowing system, the window is still open!");

		//Clean up SDL
		SDL_Quit();
	}

	void Window::Open(std::string title, glm::uvec2 initialSize, bool startVisible, WindowMode mode) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");
		CheckException(sysInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't open the window, the windowing system has not been initialized yet!");

		size = initialSize;

		//Configure SDL prior to window creation
		ConfigureSDL();

		//Create window
		SDL_WindowFlags flags = GetSDLFlags() | SDL_WINDOW_RESIZABLE;
		if(!startVisible) flags |= SDL_WINDOW_HIDDEN;
		nativeWin = SDL_CreateWindow(windowTitle.c_str(), initialSize.x, initialSize.y, flags);
		if(nativeWin == nullptr) {
			std::stringstream emsg;
			emsg << "Failed to open the window: " << SDL_GetError();
			EngineAssert(false, emsg.str());
		}

		//Initialize graphics API
		SetupGraphicsAPI(nativeWin);

		//Mark window open
		isOpen = true;

		//Set the window mode
		SetMode(mode);

		//Set window VSync state
		SetVSyncEnabled(true);
	}

	void Window::Close() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't close the window, it's not open!");

		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this]() {
				Close();
			});
			func.get();
			return;
		}

		//Clean up the graphics API
		CleanupGraphicsAPI();

		//Destroy the window
		SDL_DestroyWindow(nativeWin);

		isOpen = false;
	}

	glm::uvec2 Window::GetContentAreaSize() {
		if(!isOpen) return glm::uvec2 {0};
		int x, y;

		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			glm::uvec2* ret = new glm::uvec2(0);
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this, ret]() {
				*ret = GetContentAreaSize();
			});
			func.get();
			glm::uvec2 retval = {(*ret).x, (*ret).y};
			delete ret;
			return retval;
		}

		SDL_GetWindowSizeInPixels(nativeWin, &x, &y);
		return glm::uvec2 {(unsigned int)x, (unsigned int)y};
	}

	void Window::UpdateWindowSize() {
		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this]() {
				UpdateWindowSize();
			});
			func.get();
			return;
		}

		//Update window size
		SDL_SetWindowSize(nativeWin, size.x, size.y);

		//Update viewport size
		ResizeViewport(nativeWin);
	}

	void Window::UpdateVisibilityState() {
		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this]() {
				UpdateVisibilityState();
			});
			func.get();
			return;
		}

		if(isVisible) {
			SDL_ShowWindow(nativeWin);
		} else {
			SDL_HideWindow(nativeWin);
		}
	}

	void Window::UpdateModeState(WindowMode lastMode) {
		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this, &lastMode]() {
				UpdateModeState(lastMode);
			});
			func.get();
			return;
		}

		if(lastMode == WindowMode::Window) {
			SDL_GetWindowPosition(nativeWin, &windowedPosition.x, &windowedPosition.y);
		}
		switch(mode) {
			case WindowMode::Window:
				SDL_SetWindowSize(nativeWin, size.x, size.y);
				SDL_SetWindowFullscreen(nativeWin, false);
				SDL_SetWindowPosition(nativeWin, windowedPosition.x, windowedPosition.y);
				SDL_SetWindowBordered(nativeWin, true);
				break;
			case WindowMode::Fullscreen: {
				//Get the fullscreen display modes
				int fullscreenModesC;
				SDL_DisplayMode** fullscreenModesV = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &fullscreenModesC);

				//Confirm that we have at least one
				if(fullscreenModesC < 1) {
					mode = lastMode;
					CheckException(false, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "There are no fullscreen modes available to use!");
				}

				//Set to fullscreen
				SDL_SetWindowFullscreenMode(nativeWin, fullscreenModesV[0]);
				SDL_SetWindowFullscreen(nativeWin, true);

				//Free the mode pointer
				SDL_free(fullscreenModesV);
				break;
			}
			case WindowMode::Borderless:
				SDL_SetWindowFullscreenMode(nativeWin, nullptr);
				SDL_SetWindowFullscreen(nativeWin, true);
				break;
		}
	}

	void Window::Update() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't update closed window!");

		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this]() {
				Update();
			});
			func.get();
			return;
		}

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
					if(Window::GetInstance()->GetMode() == WindowMode::Window) {
						WindowResizer().Resize({event.window.data1, event.window.data2});
					}
					Engine::GetInstance()->GetGlobalUIView()->SetSize(GetContentAreaSize());
					ResizeViewport(nativeWin);
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
					DataEvent<int> kue("KeyUp", event.key.key);
					EventManager::GetInstance()->Dispatch(kue);
					break;
				}
				case SDL_EVENT_KEY_DOWN: {
					DataEvent<int> kde("KeyDown", event.key.key);
					EventManager::GetInstance()->Dispatch(kde);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_UP: {
					DataEvent<int> mre("MouseRelease", event.button.button);
					EventManager::GetInstance()->Dispatch(mre);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_DOWN: {
					DataEvent<int> mpe("MousePress", event.button.button);
					EventManager::GetInstance()->Dispatch(mpe);
					break;
				}
				case SDL_EVENT_WINDOW_MINIMIZED: {
					Event e("WindowMinimize");
					EventManager::GetInstance()->Dispatch(e);
					minimized = true;
					break;
				}
				case SDL_EVENT_WINDOW_RESTORED: {
					Event e("WindowRestore");
					EventManager::GetInstance()->Dispatch(e);
					minimized = false;
					break;
				}
			}
		}
	}

	void Window::SetTitle(std::string title) {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the title of a closed window!");

		//Run on the engine thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			std::shared_future<void> func = Engine::GetInstance()->RunOnMainThread([this, &title]() {
				SetTitle(title);
			});
			func.get();
			return;
		}

		SDL_SetWindowTitle(nativeWin, title.c_str());
	}
}
