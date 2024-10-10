#include "SDLHooks.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"
#include "Core/Exception.hpp"
#include "SDLWindowData.hpp"
#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"

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

		//Create swapchain and image views
		GenSwapchain();
	}

	void CleanupGraphicsAPI() {}

	void ResizeViewport(SDL_Window* win) {}

	void Window::UpdateVSyncState() {}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!")
	}
}