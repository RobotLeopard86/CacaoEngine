#include "Graphics/Window.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

#include "Core/Exception.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"

namespace Cacao {

	//Initialize static members
	Window* Window::instance = nullptr;
	bool Window::instanceExists = false;

	//Simple friend struct to set window size without function that causes more size events
	struct WindowResizer {
		friend Window;

		void Resize(glm::ivec2 size) {
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
	void ResizeGLViewport(GLFWwindow* win) {
		int fbx, fby;
		glfwGetFramebufferSize(win, &fbx, &fby);
		glViewport(0, 0, fbx, fby);
	}

	void Window::Open(std::string title, glm::ivec2 initialSize, bool startVisible, WindowMode mode) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");

		size = initialSize;

		//Initialize GLFW
		EngineAssert(glfwInit() == GLFW_TRUE, "Could not initialize GLFW library, no window can be created.");

		//Set error callback
		glfwSetErrorCallback([](int ec, const char* message) {
			//Always false exception
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLFW"), std::string("(GLFW error ") + std::to_string(ec) + ") " + message);
		});

		//Set window initialization hints
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

		//Set initial window visibility
		if(!startVisible) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		//Create window
		nativeWindow = glfwCreateWindow(initialSize.x, initialSize.y, windowTitle.c_str(), NULL, NULL);
		EngineAssert(nativeWindow != NULL, "Failed to open the window!");

		//Set the window mode
		SetMode(mode);

		//Set window VSync state
		SetVSyncEnabled(true);

		//Register window callbacks
		glfwSetCursorPosCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, double x, double y) {
			DataEvent<glm::vec2> mme("MouseMove", {x, y});
			EventManager::GetInstance()->Dispatch(mme);
		});
		glfwSetScrollCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, double offX, double offY) {
			DataEvent<glm::vec2> mse("MouseScroll", {offX, offY});
			EventManager::GetInstance()->Dispatch(mse);
		});
		glfwSetWindowSizeCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int x, int y) {
			if(!glfwGetWindowAttrib(win, GLFW_ICONIFIED)) {
				if(Window::GetInstance()->GetCurrentMode() == WindowMode::Window) {
					WindowResizer().Resize({x, y});
				}
				ResizeGLViewport(win);
				DataEvent<glm::ivec2> wre("WindowResize", {x, y});
				EventManager::GetInstance()->Dispatch(wre);
			}
		});
		glfwSetWindowFocusCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int entered) {
			Event e((entered ? "WindowFocus" : "WindowUnfocus"));
			EventManager::GetInstance()->Dispatch(e);
		});
		glfwSetKeyCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int key, int scan, int action, int mods) {
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
		glfwSetMouseButtonCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int btn, int action, int mods) {
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
		glfwSetCharCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, unsigned int code) {
			DataEvent<unsigned int> kte("KeyType", code);
			EventManager::GetInstance()->Dispatch(kte);
		});
		glfwSetWindowCloseCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win) {
			Event e("WindowClose");
			EventManager::GetInstance()->Dispatch(e);
		});

		//Initialize OpenGL
		glfwMakeContextCurrent((GLFWwindow*)nativeWindow);
		int gladResult = gladLoadGL(glfwGetProcAddress);
		EngineAssert(gladResult != 0, "Failed to load OpenGL!");

		isOpen = true;
	}

	void Window::Close() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't close the window, it's not open!");

		//Destroy the window
		glfwDestroyWindow((GLFWwindow*)nativeWindow);

		//Clean up GLFW
		glfwTerminate();

		isOpen = false;
	}

	void Window::UpdateVSyncState() {
		glfwSwapInterval(useVSync);
	}

	void Window::UpdateWindowSize() {
		//Update GLFW window size
		glfwSetWindowSize((GLFWwindow*)nativeWindow, size.x, size.y);

		//Update OpenGL framebuffer size
		ResizeGLViewport((GLFWwindow*)nativeWindow);
	}

	void Window::UpdateVisibilityState() {
		if(isVisible) {
			glfwShowWindow((GLFWwindow*)nativeWindow);
		} else {
			glfwHideWindow((GLFWwindow*)nativeWindow);
		}
	}

	void Window::UpdateModeState(WindowMode lastMode) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* modeInfo = glfwGetVideoMode(monitor);
		if(lastMode == WindowMode::Window) {
			if(glfwGetPlatform() != GLFW_PLATFORM_WAYLAND) {
				glfwGetWindowPos((GLFWwindow*)nativeWindow, &windowedPosition.x, &windowedPosition.y);
			} else {
				windowedPosition = {0, 0};
			}
		}
		switch(mode) {
			case WindowMode::Window:
				if(lastMode == WindowMode::Borderless) {
					//Exiting borderless is weird so we go to fullscreen for a sec to fix that
					glfwSetWindowMonitor((GLFWwindow*)nativeWindow, monitor, 0, 0, modeInfo->width, modeInfo->height, modeInfo->refreshRate);
				}
				glfwSetWindowMonitor((GLFWwindow*)nativeWindow, NULL, windowedPosition.x, windowedPosition.y, size.x, size.y, GLFW_DONT_CARE);
				glfwSetWindowSize((GLFWwindow*)nativeWindow, size.x, size.y);
				break;
			case WindowMode::Fullscreen:
				glfwSetWindowMonitor((GLFWwindow*)nativeWindow, monitor, 0, 0, modeInfo->width, modeInfo->height, modeInfo->refreshRate);
				break;
			case WindowMode::Borderless: 
				{
					glm::ivec2 sizeBackup = size;
					glfwSetWindowMonitor((GLFWwindow*)nativeWindow, monitor, 0, 0, modeInfo->width, modeInfo->height, GLFW_DONT_CARE);
					glfwSetWindowSize((GLFWwindow*)nativeWindow, modeInfo->width, modeInfo->height);
					size = sizeBackup;
				}
				break;
		}
	}

	void Window::Update() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't update closed window!");
		//Have GLFW check for events
		glfwPollEvents();
	}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't present frame to a closed window!");
		glfwSwapBuffers((GLFWwindow*)nativeWindow);
	}

	void Window::SetTitle(std::string title) {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the title of a closed window!");
		glfwSetWindowTitle((GLFWwindow*)nativeWindow, title.c_str());
	}
}