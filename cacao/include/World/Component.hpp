#pragma once

#include "Entity.hpp"

#include <string>

namespace Cacao {
	//A component on an entity
	class Component {
	  public:
		const bool IsActive() {
			return _GetActiveState() && owner.lock()->active;
		}
		virtual void SetActive(bool value) {
			_SetActiveInternal(value);
		}
		virtual std::string GetKind() {
			return "_BASECOMPONENT";
		}

		//Virtual destructor
		virtual ~Component() {}

	  protected:
		//Access the owner
		std::weak_ptr<Entity>& GetOwner() {
			return owner;
		}

		//Internal direct active value setter
		void _SetActiveInternal(bool v) {
			active = v;
		}

	  private:
		//The entity owning this component
		std::weak_ptr<Entity> owner [[maybe_unused]];

		//Is this component active?
		bool active;
		bool _GetActiveState() {
			return active;
		}

		//Set the owner
		void SetOwner(std::weak_ptr<Entity> owner) {
			this->owner = owner;
		}

		friend Entity;
	};
}