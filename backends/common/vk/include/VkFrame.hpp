#include "vulkan/vulkan.hpp"

namespace Cacao {
	struct VkFrame {
		vk::Fence fence;
		vk::Semaphore acquireSemaphore, renderSemaphore;
		vk::CommandBuffer cmd;
	};
}