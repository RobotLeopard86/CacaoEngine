#pragma once

#include "Utilities/MiscUtils.hpp"
#include "UI/UIRenderable.hpp"

#include "glm/glm.hpp"

#include <vector>
#include <memory>

namespace Cacao {
	/**
	 * @brief A point on the screen to anchor a UIElement to
	 */
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

	/**
	 * @brief Base class for all UI elements
	 */
	class UIElement {
	  public:
		///@brief Create a new UI element with default settings
		UIElement()
		  : anchor(AnchorPoint::Center), offsetFromAnchor(0.0f, 0.0f), size(5, 5), depth(1), active(true), dirty(false) {}

		/**
		 * @brief Get the current anchor point
		 *
		 * @return The anchor point
		 */
		AnchorPoint GetAnchor() {
			return anchor;
		}

		/**
		 * @brief Get the current offset from the anchor point
		 *
		 * @return The offset from the anchor point
		 */
		glm::vec2 GetOffsetFromAnchor() {
			return offsetFromAnchor;
		}

		/**
		 * @brief Get the current size
		 *
		 * @return The size of the element
		 */
		glm::vec2 GetSize() {
			return size;
		}

		/**
		 * @brief Get the current depth
		 *
		 * @return The element depth
		 */
		unsigned short GetDepth() {
			return depth;
		}

		/**
		 * @brief Get the current activation state
		 *
		 * @return Whether the element is active or not
		 */
		bool IsActive() {
			return active;
		}

		/**
		 * @brief Set the anchor point and make the element dirty
		 *
		 * @param a The new anchor point
		 */
		void SetAnchor(AnchorPoint a) {
			anchor = a;
			dirty = true;
		}

		/**
		 * @brief Set the anchor point offset and make the element dirty
		 *
		 * @param o The new offset. Range is 0-1 as a percentage of the screen.
		 *
		 * @see offsetFromAnchor for range
		 */
		void SetOffsetFromAnchor(glm::vec2 o) {
			offsetFromAnchor = o;
			dirty = true;
		}

		/**
		 * @brief Set the size and make the element dirty
		 *
		 * @param o The new size. Range is 0-1 as a percentage of the screen.
		 */
		void SetSize(glm::vec2 s) {
			size = s;
			dirty = true;
		}

		/**
		 * @brief Set the depth and make the element dirty
		 *
		 * @param a The new depth (higher = further back)
		 */
		void SetDepth(unsigned short d) {
			depth = d;
			dirty = true;
		}

		/**
		 * @brief Activate or deactivate the element and make the element dirty
		 *
		 * @param a The new activation state
		 */
		void SetActive(bool a) {
			active = a;
			dirty = true;
		}

		/**
		 * @brief Check if the object is dirty (has it changed and not been re-rendered)
		 *
		 * @return
		 */
		bool IsDirty() {
			return dirty;
		}

		/**
		 * @brief Create a renderable version of the element to draw
		 *
		 * @param displaySize The size of the area in which the screen is being drawn
		 */
		virtual std::shared_ptr<UIRenderable> MakeRenderable(glm::uvec2 displaySize) {
			std::shared_ptr<UIRenderable> retval = std::make_shared<UIRenderable>();
			retval->screenPos = {0, 0};
			retval->size = {0, 0};
			retval->depth = 0;
			return retval;
		}

		void CommonRenderableSetup(std::shared_ptr<UIRenderable> out, glm::uvec2 screenSize);

	  protected:
		///@brief Point to anchor to on the screen
		AnchorPoint anchor;

		/**
		 * @brief Offset from anchor point
		 * @details Positive X is right, negative X is left. Positive Y is down, negative Y is up.
		 */
		glm::vec2 offsetFromAnchor;

		///@brief Object size (perecentage of screen, 0-1)
		glm::vec2 size;

		///@brief How many "layers" back the element is, higher = further back
		unsigned short depth;

		///@brief Is the element active?
		bool active;

		///@brief If the element has changed and not been re-rendered
		bool dirty;

		///@cond
		void NotifyClean() {
			dirty = false;
		}
		///@endcond

		friend class UIView;
	};
}