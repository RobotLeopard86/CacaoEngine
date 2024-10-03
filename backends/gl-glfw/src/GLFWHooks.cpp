#include "GLHeaders.hpp"

#include "GLFWHooks.hpp"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "GLFWWindowData.hpp"

namespace Cacao {
	void SetGLFWHints() {
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	}

	void SetupGraphicsAPI(GLFWwindow* win) {
		//Make the GL context current
		glfwMakeContextCurrent(win);
		int gladResult = gladLoadGL(glfwGetProcAddress);
		EngineAssert(gladResult != 0, "Failed to load OpenGL!");
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