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
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new Window();
			instanceExists = true;
		}

		return instance;
	}

	void Engine::EarlyWindowingInit() {
#ifdef __linux__
		if(auto forceX = std::getenv("CACAO_FORCE_X11"); forceX != nullptr && std::string(forceX).compare("YES") == 0) SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#endif

		//Initialize SDL
		EngineAssert(SDL_Init(SDL_INIT_VIDEO), "Could not initialize SDL library, no window can be created.");
	}

	void Window::Open(std::string title, glm::uvec2 initialSize, bool startVisible, WindowMode mode) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");

		size = initialSize;

		//Create native data
		nativeData.reset(new WindowData());

		//Configure SDL prior to window creation
		ConfigureSDL();

		//Create window
		SDL_WindowFlags flags = GetSDLFlags() | SDL_WINDOW_RESIZABLE;
		if(!startVisible) flags |= SDL_WINDOW_HIDDEN;
		nativeData->win = SDL_CreateWindow(windowTitle.c_str(), initialSize.x, initialSize.y, flags);
		EngineAssert(nativeData->win != nullptr, "Failed to open the window!");

		//Initialize graphics API
		SetupGraphicsAPI(nativeData->win);

		//Mark window open
		isOpen = true;

		//Set the window mode
		SetMode(mode);

		//Set window VSync state
		SetVSyncEnabled(true);
	}

	void Window::Close() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't close the window, it's not open!");

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this](){
				Close();
			}).get();
			return;
		}

		//Clean up the graphics API
		CleanupGraphicsAPI();

		//Destroy the window
		SDL_DestroyWindow(nativeData->win);

		//Clean up SDL
		SDL_Quit();

		isOpen = false;
	}

	glm::uvec2 Window::GetContentAreaSize() {
		if(!isOpen) return glm::uvec2 {0};
		int x, y;

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			glm::uvec2* ret = new glm::uvec2(0);
			Engine::GetInstance()->RunOnMainThread([this, ret](){
				*ret = GetContentAreaSize();
			}).get();
			return *ret;
		}

		SDL_GetWindowSizeInPixels(nativeData->win, &x, &y);
		return glm::uvec2 {(unsigned int)x, (unsigned int)y};
	}

	void Window::UpdateWindowSize() {
		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this](){
				UpdateWindowSize();
			}).get();
			return;
		}

		//Update window size
		SDL_SetWindowSize(nativeData->win, size.x, size.y);

		//Update viewport size
		ResizeViewport(nativeData->win);
	}

	void Window::UpdateVisibilityState() {
		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this](){
				UpdateVisibilityState();
			}).get();
			return;
		}

		if(isVisible) {
			SDL_ShowWindow(nativeData->win);
		} else {
			SDL_HideWindow(nativeData->win);
		}
	}

	void Window::UpdateModeState(WindowMode lastMode) {
		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this, &lastMode](){
				UpdateModeState(lastMode);
			}).get();
			return;
		}

		if(lastMode == WindowMode::Window) {
			SDL_GetWindowPosition(nativeData->win, &windowedPosition.x, &windowedPosition.y);
		}
		switch(mode) {
			case WindowMode::Window:
				SDL_SetWindowSize(nativeData->win, size.x, size.y);
				SDL_SetWindowFullscreen(nativeData->win, false);
				SDL_SetWindowPosition(nativeData->win, windowedPosition.x, windowedPosition.y);
				SDL_SetWindowBordered(nativeData->win, true);
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
				SDL_SetWindowFullscreenMode(nativeData->win, fullscreenModesV[0]);
				SDL_SetWindowFullscreen(nativeData->win, true);

				//Free the mode pointer
				SDL_free(fullscreenModesV);
				break;
			}
			case WindowMode::Borderless:
				SDL_SetWindowFullscreenMode(nativeData->win, nullptr);
				SDL_SetWindowFullscreen(nativeData->win, true);
				break;
		}
	}

	void Window::Update() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't update closed window!");

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this](){
				Update();
			}).get();
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
					DataEvent<int> kue("KeyUp", SDLKey2Cacao(event.key.key));
					EventManager::GetInstance()->Dispatch(kue);
					break;
				}
				case SDL_EVENT_KEY_DOWN: {
					DataEvent<int> kde("KeyDown", SDLKey2Cacao(event.key.key));
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

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this, &title](){
				SetTitle(title);
			}).get();
			return;
		}

		SDL_SetWindowTitle(nativeData->win, title.c_str());
	}

	void RegisterWindowingExceptions() {}
}
