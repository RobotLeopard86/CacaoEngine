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
		  : pivot(0.0f, 0.0f), rotation(0.0f), anchor(AnchorPoint::Center), preserveAspect(true), offsetFromAnchor(0.0f, 0.0f), size(5, 5), depth(1), active(true), dirty(false) {}

		glm::vec2 GetPivot() {
			return pivot;
		}
		float GetRotation() {
			return rotation;
		}
		AnchorPoint GetAnchor() {
			return anchor;
		}
		bool IsAspectPreserved() {
			return preserveAspect;
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
		void SetPivot(glm::vec2 p) {
			pivot = p;
			dirty = true;
		}
		void SetRotation(float r) {
			rotation = r;
			dirty = true;
		}
		void SetAnchor(AnchorPoint a) {
			anchor = a;
			dirty = true;
		}
		void SetAspectPreservation(bool val) {
			preserveAspect = val;
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

	  protected:
		//Pivot point within object bounds (0 to 1 per axis)
		glm::vec2 pivot;

		//Rotation around pivot in degrees
		float rotation;

		//Designated anchor point
		AnchorPoint anchor;

		//Should the object's aspect ratio be preserved when scaling?
		bool preserveAspect;

		//Offset from anchor point (percentage of screen)
		glm::vec2 offsetFromAnchor;

		//Object size (perecentage of screen)
		glm::vec2 size;

		//How many "layers" deep this element should be
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
	}
}