#include "Graphics/Window.hpp"

#include "glad/gles2.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

#include "Core/Exception.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"

namespace Cacao {

	//Initialize static members
	Window* Window::instance = nullptr;
	bool Window::instanceExists = false;

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

	void Window::Open(std::string windowTitle, int initialSizeX, int initialSizeY, bool startVisible) {
		CheckException(!isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't open the window, it's already open!");

		//Initialize GLFW
		EngineAssert(glfwInit() == GLFW_TRUE, "Could not initialize GLFW library, no window can be created.");

		//Set error callback
		glfwSetErrorCallback([](int ec, const char* message) {
			//Always false exception
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLFW"), std::string("(GLFW error ") + std::to_string(ec) + ") " + message);
		});

		//Set window initialization hints
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		//Set initial window visibility
		if(!startVisible) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		//Create window
		nativeWindow = glfwCreateWindow(initialSizeX, initialSizeY, windowTitle.c_str(), NULL, NULL);
		EngineAssert(nativeWindow != NULL, "Failed to open the window!");

		//Set window VSync state
		SetVSyncEnabled(true);

		//Set window object size parameters
		size = {initialSizeX, initialSizeY};

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
			Window::GetInstance()->SetSize({x, y});
			DataEvent<glm::ivec2> wre("WindowResize", {x, y});
			EventManager::GetInstance()->Dispatch(wre);
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
		int gladResult = gladLoadGLES2(glfwGetProcAddress);
		EngineAssert(gladResult != 0, "Failed to load OpenGL ES!");

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

		//Update OpenGL ES framebuffer size
		int fbx, fby;
		glfwGetFramebufferSize((GLFWwindow*)nativeWindow, &fbx, &fby);
		glViewport(0, 0, fbx, fby);
	}

	void Window::UpdateVisibilityState() {
		if(isVisible) {
			glfwShowWindow((GLFWwindow*)nativeWindow);
		} else {
			glfwHideWindow((GLFWwindow*)nativeWindow);
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