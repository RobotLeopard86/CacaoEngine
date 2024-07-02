#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glm/glm.hpp"

#include <vector>
#include <memory>

namespace Cacao {
	//Anchor points for UI positioning
	enum class AnchorPoint {
		Center,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
		TopCenter,
		BottomCenter,
		LeftCenter,
		RightCenter
	}

	//Base class for all UI elements
	class UIElement {
	  public:
		UIElement()
		  : position(0.0f, 0.0f), size(200, 100), rotation(0.0f), active(true), dirty(false) {}

	  protected:
		//Pivot point within object bounds (0 to 1 per axis)
		glm::vec2 pivot;

		//Rotation around pivot in degrees
		float rotation;

		//Designated anchor point
		AnchorPoint anchor;

		//Scaling preferences
		bool scaleX, scaleY;

		//Offset from anchor point (percentage of screen)
		glm::vec2 offsetFromAnchor;

		//Object size (perecentage of screen)
		glm::vec2 size;

		//How many "layers" deep this element should be
		//Example: depth 3 object is behind depth 2 object
		unsigned short depth;

		//Are we active?
		bool active;

		//Draw this element
		//For backend implementation
		virtual void Draw(glm::uvec2 area) = 0;

		//Are we dirty (have changed)?
		bool dirty;

		//Notify that changes have been recorded so we're no longer dirty
		void NotifyClean() {
			dirty = false;
		}

		friend class UIView;
	}
}