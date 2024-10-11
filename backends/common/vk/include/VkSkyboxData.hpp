#pragma once

#include "vulkan/vulkan.hpp"

#include "VkUtils.hpp"

namespace Cacao {
	struct Skybox::SkyboxData {
		Allocated<vk::Buffer> vertexBuffer;
	};
}