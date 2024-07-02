#pragma once

#include "UIElement.hpp"

//This is required for uint64_t used by crossguid (but that doesn't include it for some reason)
#include <stdint.h>

#include "crossguid/guid.hpp"

namespace Cacao {
	//The top-level contents of a UI display
	//Can be nested in other screens
	class Screen {
	  public:
		//Add an element
		//Returns a GUID for accessing this component (DO NOT LOSE THIS)
		const xg::Guid AddElement(std::shared_ptr<UIElement> elem);

		//Check if an element is contained by its GUID
		bool HasElement(const xg::Guid& guid);

		//Remove an element by its GUID
		void DeleteElement(const xg::Guid& guid);

	  private:
		//Contained elements
		std::vector<std::shared_ptr<UIElement>> elements;

		//Are we dirty (have changed)?
		bool dirty;

		//Notify that changes have been recorded so we're no longer dirty
		void NotifyClean() {
			dirty = false;
		}

		friend class UIView;
	}
}