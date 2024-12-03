#include "Graphics/Window.hpp"

#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

#include "Core/Exception.hpp"
#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Events/EventSystem.hpp"
#include "GLFWWindowData.hpp"
#include "GLFWHooks.hpp"
#include "UI/Text.hpp"

namespace Cacao {

	//Initialize static members
	Window* Window::instance = nullptr;
	bool Window::instanceExists = false;

	//Simple friend struct to set window size without function that causes more size events
	//Also here it does minimization (yes the title is misleading, cry about it in b minor)
	struct WindowResizer {
		friend Window;

		void Resize(glm::uvec2 size) {
			ChangeSize(Window::GetInstance(), size);
		}

		void SetMinimize(bool isMinimized) {
			NotifyMinimizeState(Window::GetInstance(), isMinimized);
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
		if(auto forceX = std::getenv("CACAO_FORCE_X11"); forceX != nullptr && std::string(forceX).compare("YES") == 0) glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

		//Initialize GLFW
		EngineAssert(glfwInit() == GLFW_TRUE, "Could not initialize GLFW library!");

		//Set error callback
		glfwSetErrorCallback([](int ec, const char* message) {
			//Always false exception
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLFW"), std::string("(GLFW error ") + std::to_string(ec) + ") " + message);
		});
	}

	void Window::Open(std::string title, glm::uvec2 initialSize, bool startVisible, WindowMode mode) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");

		size = initialSize;

		//Create native data
		nativeData.reset(new WindowData());

		//Set window initialization hints
		SetGLFWHints();

		//Set initial window visibility
		if(!startVisible) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		//Create window
		nativeData->win = glfwCreateWindow(initialSize.x, initialSize.y, windowTitle.c_str(), nullptr, nullptr);
		EngineAssert(nativeData->win != nullptr, "Failed to open the window!");

		//Register window callbacks
		glfwSetCursorPosCallback(nativeData->win, [](GLFWwindow* win, double x, double y) {
			DataEvent<glm::vec2> mme("MouseMove", {x, y});
			EventManager::GetInstance()->Dispatch(mme);
		});
		glfwSetScrollCallback(nativeData->win, [](GLFWwindow* win, double offX, double offY) {
			DataEvent<glm::vec2> mse("MouseScroll", {offX, offY});
			EventManager::GetInstance()->Dispatch(mse);
		});
		glfwSetWindowSizeCallback(nativeData->win, [](GLFWwindow* win, int x, int y) {
			if(!glfwGetWindowAttrib(win, GLFW_ICONIFIED)) {
				if(Window::GetInstance()->GetMode() == WindowMode::Window) {
					WindowResizer().Resize({x, y});
				}
				ResizeViewport(win);
				DataEvent<glm::uvec2> wre("WindowResize", {x, y});
				EventManager::GetInstance()->Dispatch(wre);
			}
		});
		glfwSetWindowFocusCallback(nativeData->win, [](GLFWwindow* win, int entered) {
			Event e((entered ? "WindowFocus" : "WindowUnfocus"));
			EventManager::GetInstance()->Dispatch(e);
		});
		glfwSetKeyCallback(nativeData->win, [](GLFWwindow* win, int key, int scan, int action, int mods) {
			switch(action) {
				case GLFW_PRESS: {
					DataEvent<int> kde("KeyDown", key);
					EventManager::GetInstance()->Dispatch(kde);
					break;
				}
				case GLFW_RELEASE: {
					DataEvent<int> kue("KeyUp", key);
					EventManager::GetInstance()->Dispatch(kue);
					break;
				}
			}
		});
		glfwSetMouseButtonCallback(nativeData->win, [](GLFWwindow* win, int btn, int action, int mods) {
			switch(action) {
				case GLFW_PRESS: {
					DataEvent<int> mpe("MousePress", btn);
					EventManager::GetInstance()->Dispatch(mpe);
					break;
				}
				case GLFW_RELEASE: {
					DataEvent<int> mre("MouseRelease", btn);
					EventManager::GetInstance()->Dispatch(mre);
					break;
				}
			}
		});
		glfwSetCharCallback(nativeData->win, [](GLFWwindow* win, unsigned int code) {
			DataEvent<unsigned int> kte("KeyType", code);
			EventManager::GetInstance()->Dispatch(kte);
		});
		glfwSetWindowCloseCallback(nativeData->win, [](GLFWwindow* win) {
			Event e("WindowClose");
			EventManager::GetInstance()->Dispatch(e);
		});
		glfwSetFramebufferSizeCallback(nativeData->win, [](GLFWwindow* win, int w, int h) {
			Engine::GetInstance()->GetGlobalUIView()->SetSize({(unsigned int)w, (unsigned int)h});
		});
		glfwSetWindowIconifyCallback(nativeData->win, [](GLFWwindow* win, int iconified) {
			WindowResizer().SetMinimize(iconified == GLFW_TRUE);
		});

		//Initialize graphics api
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
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this]() {
									 Close();
								 })
				.get();
			return;
		}

		//Clean up the graphics API
		CleanupGraphicsAPI();

		//Destroy the window
		glfwDestroyWindow(nativeData->win);

		//Clean up GLFW
		glfwTerminate();

		isOpen = false;
	}

	void Window::UpdateWindowSize() {
		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this]() {
									 UpdateWindowSize();
								 })
				.get();
			return;
		}

		//Update window size
		glfwSetWindowSize(nativeData->win, size.x, size.y);

		//Update framebuffer size
		ResizeViewport(nativeData->win);
	}

	void Window::UpdateVisibilityState() {
		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this]() {
									 UpdateVisibilityState();
								 })
				.get();
			return;
		}

		if(isVisible) {
			glfwShowWindow(nativeData->win);
		} else {
			glfwHideWindow(nativeData->win);
		}
	}

	void Window::UpdateModeState(WindowMode lastMode) {
		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this, &lastMode]() {
									 UpdateModeState(lastMode);
								 })
				.get();
			return;
		}

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* modeInfo = glfwGetVideoMode(monitor);
		if(lastMode == WindowMode::Window) {
			if(glfwGetPlatform() != GLFW_PLATFORM_WAYLAND) {
				glfwGetWindowPos(nativeData->win, &windowedPosition.x, &windowedPosition.y);
			} else {
				windowedPosition = {0, 0};
			}
		}
		switch(mode) {
			case WindowMode::Window:
				if(lastMode == WindowMode::Borderless) {
					//Exiting borderless is weird so we go to fullscreen for a sec to fix that
					glfwSetWindowMonitor(nativeData->win, monitor, 0, 0, modeInfo->width, modeInfo->height, modeInfo->refreshRate);
				}
				glfwSetWindowMonitor(nativeData->win, nullptr, windowedPosition.x, windowedPosition.y, size.x, size.y, GLFW_DONT_CARE);
				glfwSetWindowSize(nativeData->win, size.x, size.y);
				break;
			case WindowMode::Fullscreen:
				glfwSetWindowMonitor(nativeData->win, monitor, 0, 0, modeInfo->width, modeInfo->height, modeInfo->refreshRate);
				break;
			case WindowMode::Borderless: {
				glm::uvec2 sizeBackup = size;
				glfwSetWindowMonitor(nativeData->win, monitor, 0, 0, modeInfo->width, modeInfo->height, GLFW_DONT_CARE);
				glfwSetWindowSize(nativeData->win, modeInfo->width, modeInfo->height);
				size = sizeBackup;
			} break;
		}
	}

	glm::uvec2 Window::GetContentAreaSize() {
		if(!isOpen) return glm::uvec2 {0};
		int x, y;

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			glm::uvec2* ret = new glm::uvec2(0);
			Engine::GetInstance()->RunOnMainThread([this, ret]() {
									 *ret = GetContentAreaSize();
								 })
				.get();
			return *ret;
		}

		glfwGetFramebufferSize(nativeData->win, &x, &y);
		return glm::uvec2 {(unsigned int)x, (unsigned int)y};
	}

	void Window::Update() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't update closed window!");

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this]() {
									 Update();
								 })
				.get();
			return;
		}

		//Have GLFW check for events
		glfwPollEvents();
	}

	void Window::SetTitle(std::string title) {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the title of a closed window!");

		//Run on the main thread if we aren't on it
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			Engine::GetInstance()->RunOnMainThread([this, &title]() {
									 SetTitle(title);
								 })
				.get();
			return;
		}

		glfwSetWindowTitle(nativeData->win, title.c_str());
	}

	void RegisterWindowingExceptions() {}
}