#pragma once

#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief Base renderable form of a UI element
	 */
	struct UIRenderable {
		glm::uvec2 screenPos;///<Position on the screen in pixels at the object center, (0, 0) is top left
		glm::uvec2 size;	 ///<Size in pixels
		unsigned short depth;///<Depth

		/**
		 * @brief Draw this renderable
		 *
		 * @param screenSize The size of the screen to render to in pixels
		 * @param projection The projection matrix to draw with
		 */
		virtual void Draw(glm::uvec2 screenSize, const glm::mat4& projection) {}

		virtual ~UIRenderable() {}
	};
}