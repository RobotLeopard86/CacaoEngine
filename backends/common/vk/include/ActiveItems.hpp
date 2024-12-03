#pragma once

#include "VkFrame.hpp"
#include "VkShaderData.hpp"

namespace Cacao {
	inline struct FrameSubmission {
		vk::Semaphore sem;
		uint32_t image;
	} submission;

	inline unsigned short frameCycle;
	inline VkFrame* activeFrame;
	inline VkShaderData* activeShader;
}
