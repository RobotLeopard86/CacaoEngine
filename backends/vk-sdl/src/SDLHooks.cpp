#include "SDLHooks.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "SDLWindowData.hpp"
#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"

constexpr std::array<vk::Format, 2> acceptableFormats {{vk::Format::eB8G8R8A8Srgb, vk::Format::eR8G8B8A8Srgb}};

namespace Cacao {
	void ConfigureSDL() {}

	SDL_WindowFlags GetSDLFlags() {
		return SDL_WINDOW_VULKAN;
	}

	void SetupGraphicsAPI(SDL_Window* win) {
		//Create window surface
		VkSurfaceKHR cSurface;
		EngineAssert(SDL_Vulkan_CreateSurface(win, vk_instance, nullptr, &cSurface), "Could not create window surface!");
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
		dev.destroySwapchainKHR(swapchain);
		vk_instance.destroySurfaceKHR(surface);
	}

	void ResizeViewport(SDL_Window* win) {
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