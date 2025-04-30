#pragma once

#include "vulkan/vulkan.hpp"

#include "VkUtils.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan 2D texture
	struct Texture2D::Tex2DData {
		Allocated<vk::Image> texture;
		vk::ImageView iview;
		vk::Format format;
	};
}