#pragma once

#include "vulkan/vulkan.hpp"

#include "Graphics/Shader.hpp"

namespace Cacao {
	struct VkFrame {
		vk::Fence fence;
		vk::Semaphore acquireSemaphore, renderSemaphore;
		vk::CommandBuffer cmd;
	};
}