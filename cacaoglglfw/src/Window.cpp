#include "Graphics/Window.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"

namespace Cacao {

	//Initialize static members
	Window* Window::instance = nullptr;
	bool Window::instanceExists = false;

	//Singleton accessor
	Window* Window::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new Window();
			instanceExists = true;
		}

		return instance;
	}

	void Window::Open(std::string windowTitle, int initialSizeX, int initialSizeY){
		Asserts::EngineAssert(!isOpen, "Can't open the window, it's already open!");

		//Initialize GLFW
		Asserts::EngineAssert(glfwInit() == GLFW_TRUE, "Could not initialize GLFW library, no window can be created.");

		//Set error callback
		glfwSetErrorCallback([](int ec, const char* message){
			//Always false assertion
			Asserts::EngineAssert(false, std::string("GLFW error ") + std::to_string(ec) + ": " + message);
		});

		//Set window initialization hints
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

		//Create window
		nativeWindow = glfwCreateWindow(initialSizeX, initialSizeY, windowTitle.c_str(), NULL, NULL);
		Asserts::EngineAssert(nativeWindow != NULL, "Failed to open the window!");

		//Set window VSync state
		SetVSyncEnabled(true);

		//Set window object size parameters
		size = { initialSizeX, initialSizeY };

		//Register window callbacks
		glfwSetCursorPosCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, double x, double y){
			DataEvent<glm::vec2> mme("MouseMove", { x, y });
			EventManager::GetInstance()->Dispatch(mme);
		});
		glfwSetScrollCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, double offX, double offY){
			DataEvent<glm::vec2> mse("MouseScroll", { offX, offY });
			EventManager::GetInstance()->Dispatch(mse);
		});
		glfwSetWindowSizeCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int x, int y){
			Window::GetInstance()->SetSize({ x, y });
			DataEvent<glm::ivec2> wre("WindowResize", { x, y });
			EventManager::GetInstance()->Dispatch(wre);
		});
		glfwSetWindowFocusCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int entered){
			Event e((entered ? "WindowFocus" : "WindowUnfocus"));
			EventManager::GetInstance()->Dispatch(e);
		});
		glfwSetKeyCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int key, int scan, int action, int mods){
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
		glfwSetMouseButtonCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, int btn, int action, int mods){
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
		glfwSetCharCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win, unsigned int code){
			DataEvent<unsigned int> kte("KeyType", code);
			EventManager::GetInstance()->Dispatch(kte);
		});
		glfwSetWindowCloseCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* win){
			Event e("WindowClose");
			EventManager::GetInstance()->Dispatch(e);
		});

		//Initialize OpenGL
		glfwMakeContextCurrent((GLFWwindow*)nativeWindow);
		int gladResult = gladLoadGL(glfwGetProcAddress);
		Asserts::EngineAssert(gladResult != 0, "Could not load Glad!");
		Logging::EngineLog(std::string("Loaded OpenGL version ") + std::to_string(GLAD_VERSION_MAJOR(gladResult)) + "." + std::to_string(GLAD_VERSION_MINOR(gladResult)), LogLevel::Trace);

		//Release context for usage by the render controller thread
		glfwMakeContextCurrent(NULL);

		isOpen = true;
	}

	void Window::Close() {
		Asserts::EngineAssert(isOpen, "Can't close window, it's not open!");

		//Destroy the window
		glfwDestroyWindow((GLFWwindow*)nativeWindow);

		//Clean up GLFW
		glfwTerminate();

		isOpen = false;
	}

	void Window::UpdateVSyncState(){
		glfwSwapInterval(useVSync);
	}

	void Window::UpdateWindowSize(){
		glfwSetWindowSize((GLFWwindow*)nativeWindow, size.x, size.y);
	}

	void Window::Update(){
		Asserts::EngineAssert(isOpen, "Can't update window, it's not open!");
		//Have GLFW check for events
		glfwPollEvents();
	}

	void Window::Present(){
		Asserts::EngineAssert(isOpen, "Can't present to closed window!");
		//Have GLFW check for events
		glfwSwapBuffers((GLFWwindow*)nativeWindow);
	}

	void Window::SetTitle(std::string title){
		Asserts::EngineAssert(isOpen, "Can't set window title, it's not open!");
		glfwSetWindowTitle((GLFWwindow*)nativeWindow, title.c_str());
	}
}