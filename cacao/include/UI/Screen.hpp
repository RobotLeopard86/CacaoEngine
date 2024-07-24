#pragma once

#include "UIElement.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	//The top-level contents of a UI display
	//Can be nested in other screens with a NestedScreen element
	class Screen {
	  public:
		//Add an element
		void AddElement(std::shared_ptr<UIElement> elem) {
			CheckException(!HasElement(elem), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot add a duplicate element to UI screen!")
			elements.push_back(elem);
		}

		//Check if an element is contained
		bool HasElement(std::shared_ptr<UIElement> elem) {
			return (std::find(elements.begin(), elements.end(), elem) != elements.end());
		}

		//Remove an element
		void DeleteElement(std::shared_ptr<UIElement> elem) {
			CheckException(HasElement(elem), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot add a duplicate element to UI screen!")
			elements.erase(std::find(elements.begin(), elements.end(), elem));
		}

		//Check if screen is dirty
		bool IsDirty() {
			//If we aren't dirty, check to make sure we haven't become dirty
			RefreshDirtyState();

			return dirty;
		}

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