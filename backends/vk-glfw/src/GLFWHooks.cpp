#include "VulkanCoreObjects.hpp"

#include "GLFWHooks.hpp"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "GLFWWindowData.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"

constexpr std::array<vk::Format, 2> acceptableFormats {{vk::Format::eB8G8R8A8Srgb, vk::Format::eR8G8B8A8Srgb}};

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
		auto formats = physDev.getSurfaceFormatsKHR(surface);
		if(auto it = std::find_if(formats.begin(), formats.end(), [](vk::SurfaceFormatKHR sf) {
			   return sf.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && std::find(acceptableFormats.begin(), acceptableFormats.end(), sf.format) != acceptableFormats.end();
		   });
			it != formats.end()) {
			surfaceFormat = *it;
		} else {
			EngineAssert(false, "The surface given does not support the one of the required formats!");
		}

		//Create swapchain and image views
		try {
			GenSwapchain();
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Swapchain recreation failed: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
		}
	}

	void CleanupGraphicsAPI() {
		for(vk::ImageView& iview : imageViews) {
			dev.destroyImageView(iview);
		}
		dev.destroySwapchainKHR(swapchain);
		vk_instance.destroySurfaceKHR(surface);
	}

	void ResizeViewport(GLFWwindow* win) {
		try {
			GenSwapchain();
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Swapchain recreation failed: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
		}
	}

	void Window::UpdateVSyncState() {
		presentMode = (useVSync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
		try {
			GenSwapchain();
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Swapchain recreation failed: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
		}
	}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
		vk::PresentInfoKHR pi(submission.sem, swapchain, submission.image);
		try {
			graphicsQueue.presentKHR(pi);
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Present failed: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
		}
	}
}