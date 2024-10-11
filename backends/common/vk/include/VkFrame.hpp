#pragma once

#include "vulkan/vulkan.hpp"

namespace Cacao {
	struct VkFrame {
		vk::Fence fence;
		vk::Semaphore acquireSemaphore, renderSemaphore;
		vk::CommandBuffer cmd;
	};

	inline struct FrameSubmission {
		vk::Semaphore sem;
		uint32_t image;
	} submission;

	inline VkFrame* activeFrame;
}