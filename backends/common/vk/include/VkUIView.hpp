#pragma once

#include "VkUtils.hpp"

#include "UI/UIView.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Material.hpp"
#include "UI/Text.hpp"

#include <vector>
#include <variant>

namespace Cacao {
	struct UIView::Buffer {
		Allocated<vk::Image> tex;
		vk::ImageView view;
	};

	inline vk::CommandBuffer* uiCmd;
	inline std::vector<std::variant<Allocated<vk::Image>, Allocated<vk::Buffer>, vk::ImageView>> allocatedObjects;
	inline std::vector<std::shared_ptr<Material>> tempMatHandles;

	void PreprocessTextRenderable(Text::Renderable* renderable, glm::uvec2 screenSize);
	struct UIVertex {
		glm::vec3 vert;
		glm::vec2 tc;
	};
	struct CharacterInfo {
		std::array<UIVertex, 6> vertices;
		struct Glyph {
			Allocated<vk::Image> image;
			vk::ImageView view;
		} glyph;
	};
	inline std::map<Text::Renderable*, std::vector<CharacterInfo>> trCharInfos;
}