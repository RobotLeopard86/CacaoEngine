#pragma once

#include "Actor.hpp"
#include "DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief An object attached to an Actor that performs tasks on its behalf
	 */
	class CACAO_API Component {
	  public:
		/**
		 * @brief Check if the component is "functionally active"
		 * @details "Functionally active" means taking into account both if the component itself is active and its owning Actor is active
		 *
		 * @note This will return false if the owning Actor is inactive
		 */
		const bool IsActive() {
			return actor.get().IsActive() && active;
		}

		/**
		 * @brief Activate or deactivate the component
		 *
		 * @param state The new activation state
		 */
		void SetActive(bool state) {
			active = state;
		}

		/**
		 * @brief Get the owning actor of this component
		 *
		 * @return A reference to the owning actor
		 */
		Actor& GetOwner() {
			return actor;
		}

		virtual ~Component() {}

	  protected:
		Component();

		bool active;
		std::reference_wrapper<Actor> actor [[maybe_unused]];

		friend class Actor;
	};
}