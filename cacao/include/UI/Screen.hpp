#pragma once

#include "UIElement.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	/**
	 * @brief A layout of UI elements that can be displayed by a UIView
	 */
	class Screen {
	  public:
		/**
		 * @brief Add an element to the screen
		 *
		 * @param elem The element to add
		 *
		 * @throws Exception If this element already exists in the screen
		 */
		void AddElement(std::shared_ptr<UIElement> elem) {
			CheckException(!HasElement(elem), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot add a duplicate element to UI screen!");
			elements.push_back(elem);
		}

		/**
		 * @brief Check if an element is in the screen
		 *
		 * @param elem The element to search for
		 *
		 * @return Whether the element is in the screen or not
		 */
		bool HasElement(std::shared_ptr<UIElement> elem) {
			return (std::find(elements.begin(), elements.end(), elem) != elements.end());
		}

		/**
		 * @brief Remove an element from the screen
		 *
		 * @param elem The element to remove
		 *
		 * @throws Exception If this element doesn't exists in the screen
		 */
		void DeleteElement(std::shared_ptr<UIElement> elem) {
			CheckException(HasElement(elem), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot add a duplicate element to UI screen!");
			elements.erase(std::find(elements.begin(), elements.end(), elem));
		}

		/**
		 * @brief Check if the screen is dirty and needs to be re-rendered
		 * @details Will refresh the dirty state when called
		 *
		 * @return If the screen is dirty
		 */
		bool IsDirty() {
			//If we aren't dirty, check to make sure we haven't become dirty
			RefreshDirtyState();

			return dirty;
		}

		/**
		 * @brief Force the screen to be dirty
		 */
		void ForceDirty() {
			dirty = true;
		}

		/**
		 * @brief Delete all elements from the screen
		 */
		void PurgeElements();

	  private:
		//Contained elements
		std::vector<std::shared_ptr<UIElement>> elements;

		//Are we dirty (have changed)?
		bool dirty;

		//Notify that changes have been recorded so we're no longer dirty
		void NotifyClean() {
			dirty = false;
		}

		//Check to see if we are dirty
		void RefreshDirtyState();

		friend class UIView;
	};
}