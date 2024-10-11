#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan cubemap
	struct Cubemap::CubemapData {
		vma::Allocation alloc;
		vk::Image image;
		vk::ImageView iview;
		vk::Sampler sampler;
	};
}