#pragma once

#include "VkUtils.hpp"

#include "UI/UIView.hpp"
#include "Graphics/Shader.hpp"

namespace Cacao {
	struct UIView::Buffer {
		Allocated<vk::Image> tex;
		vk::ImageView view;
		vk::Sampler sampler;
		static vk::DescriptorSet* boundDS;
	};
}