#pragma once

#include "VkUtils.hpp"

#include "UI/UIView.hpp"
#include "Graphics/Shader.hpp"

#include <vector>
#include <variant>

namespace Cacao {
	struct UIView::Buffer {
		Allocated<vk::Image> tex;
		vk::ImageView view;
	};

	inline vk::CommandBuffer* uiCmd;
	inline std::vector<std::variant<Allocated<vk::Image>, Allocated<vk::Buffer>, vk::ImageView>> allocatedObjects;
}