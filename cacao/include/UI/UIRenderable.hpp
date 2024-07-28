#pragma once

#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"

#include <string>

namespace Cacao {
	//Base for all UI objects in their renderable state
	struct UIRenderable {
		//Position on the screen in pixels, (0, 0) is top left
		//This position is the center of the object's quad
		glm::uvec2 screenPos;

		//Size of the object in pixels
		glm::uvec2 size;

		//Rotation in degrees around center
		float rot;

		//How many "layers" deep this element should be
		//Example: depth 3 object is behind depth 2 object
		unsigned short depth;

		virtual void Draw(glm::uvec2 screenSize, const glm::mat4& projection) {}

		virtual ~UIRenderable() {}
	};
}