#pragma once

#include "VkUtils.hpp"

#include "UI/UIView.hpp"
#include "Graphics/Shader.hpp"

namespace Cacao {
	struct UIView::Buffer {
		Allocated<vk::Image> uiTex;
	};
}