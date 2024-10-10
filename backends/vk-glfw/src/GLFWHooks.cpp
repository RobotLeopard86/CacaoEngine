#include "VulkanCoreObjects.hpp"

#include "GLFWHooks.hpp"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "GLFWWindowData.hpp"
#include "VkUtils.hpp"

namespace Cacao {
	void SetGLFWHints() {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	void SetupGraphicsAPI(GLFWwindow* win) {
		//Initialize GLFW Vulkan functions
		glfwInitVulkanLoader(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr);

		//Create window surface
		VkSurfaceKHR cSurface;
		EngineAssert(glfwCreateWindowSurface(vk_instance, win, nullptr, &cSurface) == VK_SUCCESS, "Could not create window surface!");
		surface = vk::SurfaceKHR(cSurface);

		//Create swapchain and image views
		GenSwapchain();
	}

	void CleanupGraphicsAPI() {}

	void ResizeViewport(GLFWwindow* win) {}

	void Window::UpdateVSyncState() {}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
	}
}