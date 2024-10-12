#pragma once

#include "vulkan/vulkan.hpp"

#include "VkUtils.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan cubemap
	struct Cubemap::CubemapData {
		Allocated<vk::Image> texture;
		vk::ImageView iview;
		vk::Sampler sampler;
		vk::DescriptorSet* boundDS;
	};
}