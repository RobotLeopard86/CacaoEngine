#pragma once

#include "vulkan/vulkan.hpp"

namespace Cacao {
	void GenSwapchain();
	void GenFrameObjects();

	inline vk::PresentModeKHR presentMode;
}