#include "GLFWHooks.hpp"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "GLFWWindowData.hpp"

namespace Cacao {
	void SetGLFWHints() {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	void SetupGraphicsAPI(GLFWwindow* win) {}

	void CleanupGraphicsAPI() {}

	void ResizeViewport(GLFWwindow* win) {}

	void Window::UpdateVSyncState() {}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
	}
}