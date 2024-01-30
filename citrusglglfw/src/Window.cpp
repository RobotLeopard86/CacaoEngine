#include "Graphics/Window.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"

namespace Citrus {

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
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		//Create window
		nativeWindow = glfwCreateWindow(initialSizeX, initialSizeY, windowTitle.c_str(), glfwGetPrimaryMonitor(), NULL);
		Asserts::EngineAssert(nativeWindow != NULL, "Failed to open the window!");

		//Set window VSync state
		SetVSyncEnabled(true);

		//Set window object size parameters
		size = { initialSizeX, initialSizeY };

		//Register window callbacks
		glfwSetCursorPosCallback(nativeWindow, [](GLFWwindow* win, int x, int y){
			EventManager::GetInstance()->Dispatch(DataEvent<glm::vec2>("MouseMove", { x, y }));
		});
		glfwSetScrollCallback(nativeWindow, [](GLFWwindow* win, int offX, int offY){
			EventManager::GetInstance()->Dispatch(DataEvent<glm::vec2>("MouseScroll", { offX, offY }));
		});
		glfwSetWindowSizeCallback(nativeWindow, [](GLFWwindow* win, int x, int y){
			Window::GetInstance()->SetSize({ x, y });
			EventManager::GetInstance()->Dispatch(DataEvent<glm::vec2>("WindowResize", { x, y }));
		});
		glfwSetWindowFocusCallback(nativeWindow, [](GLFWwindow* win, int entered){
			EventManager::GetInstance()->Dispatch(Event{(entered ? "WindowFocus" : "WindowUnfocus")});
		});
		glfwSetKeyCallback(nativeWindow, [](GLFWwindow* win, int key, int scan, int action, int mods){
			switch(action) {
			case GLFW_PRESS:
				EventManager::GetInstance()->Dispatch(DataEvent<int>("KeyDown", key));
				break;
			case GLFW_RELEASE:
				EventManager::GetInstance()->Dispatch(DataEvent<int>("KeyUp", key));
				break;
			default:
				break;
			}
		});
		glfwSetMouseButtonCallback(nativeWindow, [](GLFWwindow* win, int btn, int action, int mods){
			switch(action) {
			case GLFW_PRESS:
				EventManager::GetInstance()->Dispatch(DataEvent<int>("MousePress", btn));
				break;
			case GLFW_RELEASE:
				EventManager::GetInstance()->Dispatch(DataEvent<int>("MouseRekease", btn));
				break;
			default:
				break;
			}
		});
		glfwSetCharCallback(nativeWindow, [](GLFWwindow* win, unsigned int code){
			EventManager::GetInstance()->Dispatch(DataEvent<unsigned int>("KeyType", code));
		});
		glfwSetWindowCloseCallback(nativeWindow, [](GLFWwindow* win){
			EventManager::GetInstance()->Dispatch(Event{"WindowClose"});
		});

		//Initialize OpenGL
		Asserts::EngineAssert(gladLoaderLoadGL() != 0, "Could not load Glad!");

		isOpen = true;
	}

	void Window::Close() {
		Asserts::EngineAssert(isOpen, "Can't close window, it's not open!");

		//Destroy the window
		glfwDestroyWindow(nativeWindow);

		//Clean up GLFW
		glfwTerminate();

		isOpen = false;
	}

	void Window::UpdateVSyncState(){
		glfwSwapInterval(useVSync);
	}

	void Window::UpdateWindowSize(){
		glfwSetWindowSize(nativeWindow, size.x, size.y);
	}

	void Window::Update(){
		//Have GLFW check for events
		glfwPollEvents();

		//Swap buffers
		glfwSwapBuffers();
	}
}