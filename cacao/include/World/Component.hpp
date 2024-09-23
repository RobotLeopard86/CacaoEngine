#pragma once

#include "Entity.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief A component on an Entity
	 */
	class Component {
	  public:
		/**
		 * @brief Check if this component should be functionally active
		 * @details "Functionally active" means taking into account both if the component itself is active and its owning Entity is active
		 *
		 * @return Whether the component functionally active
		 */
		const bool IsActive() {
			return _GetActiveState() && owner.lock()->IsActive();
		}

		/**
		 * @brief Set if this component is active
		 *
		 * @note Is virtual because components may wish to run custom logic here
		 */
		virtual void SetActive(bool value) {
			_SetActiveInternal(value);
		}

		///@brief Get the component type (not useful here because this is the base component)
		virtual std::string GetKind() {
			return "_BASECOMPONENT";
		}

		/**
		 * @brief Get the owning Entity of this component
		 *
		 * @return A non-owning reference to the owning entity
		 */
		std::weak_ptr<Entity> GetOwner() {
			return owner;
		}

		//Virtual destructor
		virtual ~Component() {}

	  protected:
		///@cond
		void _SetActiveInternal(bool v) {
			active = v;
		}
		///@endcond

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