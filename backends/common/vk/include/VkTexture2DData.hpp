#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan 2D texture
	struct Texture2D::Tex2DData {
		vma::Allocation alloc;
		vk::Image image;
		vk::ImageView iview;
		vk::Sampler sampler;
		vk::Format format;
	};
}