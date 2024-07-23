#include "GLFWHooks.hpp"

#include "GLHeaders.hpp"
#include "GLFW/glfw3.h"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "GLFWWindowData.hpp"

namespace Cacao {
	void SetGLFWHints() {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	}

	void SetupGraphicsAPI(GLFWwindow* win) {
		//Make the GL context current
		glfwMakeContextCurrent(win);
		int gladResult = gladLoadGLES2(glfwGetProcAddress);
		EngineAssert(gladResult != 0, "Failed to load OpenGL ES!");
	}

	void CleanupGraphicsAPI() {}

	void ResizeViewport(GLFWwindow* win) {
		int fbx, fby;
		glfwGetFramebufferSize(win, &fbx, &fby);
		glViewport(0, 0, fbx, fby);
	}

	void Window::UpdateVSyncState() {
		glfwSwapInterval(useVSync);
	}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
		glfwSwapBuffers(nativeData->win);
	}
}