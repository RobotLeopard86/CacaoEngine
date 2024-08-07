#pragma once

#include "Utilities/MiscUtils.hpp"
#include "UI/UIRenderable.hpp"

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
	};

	//Base class for all UI elements
	class UIElement {
	  public:
		UIElement()
		  : anchor(AnchorPoint::Center), offsetFromAnchor(0.0f, 0.0f), size(5, 5), depth(1), active(true), dirty(false) {}

		AnchorPoint GetAnchor() {
			return anchor;
		}
		glm::vec2 GetOffsetFromAnchor() {
			return offsetFromAnchor;
		}
		glm::vec2 GetSize() {
			return size;
		}
		unsigned short GetDepth() {
			return depth;
		}
		bool IsActive() {
			return active;
		}
		void SetAnchor(AnchorPoint a) {
			anchor = a;
			dirty = true;
		}
		void SetOffsetFromAnchor(glm::vec2 o) {
			offsetFromAnchor = o;
			dirty = true;
		}
		void SetSize(glm::vec2 s) {
			size = s;
			dirty = true;
		}
		void SetDepth(unsigned short d) {
			depth = d;
			dirty = true;
		}
		void SetActive(bool a) {
			active = a;
			dirty = true;
		}

		//Check if the object is dirty (has changed and changes have not been recorded)
		bool IsDirty() {
			return dirty;
		}

		virtual std::shared_ptr<UIRenderable> MakeRenderable(glm::uvec2) {
			std::shared_ptr<UIRenderable> retval = std::make_shared<UIRenderable>();
			retval->screenPos = {0, 0};
			retval->size = {0, 0};
			retval->depth = 0;
			return retval;
		}

		void CommonRenderableSetup(std::shared_ptr<UIRenderable> out, glm::uvec2 screenSize);

	  protected:
		//Designated anchor point
		AnchorPoint anchor;

		//Offset from anchor point (percentage of screen, 0...1)
		glm::vec2 offsetFromAnchor;

		//Object size (perecentage of screen, 0...1)
		glm::vec2 size;

		//How many "layers" deep this element should be
		//Depth 0 is first layer
		//Example: depth 3 object is behind depth 2 object
		unsigned short depth;

		//Are we active?
		bool active;

		//Are we dirty (have changed)?
		bool dirty;

		//Notify that changes have been recorded so we're no longer dirty
		void NotifyClean() {
			dirty = false;
		}

		friend class UIView;
	};
}