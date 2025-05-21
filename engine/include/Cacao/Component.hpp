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
		 * @brief Check if the component is enabled
		 * @details This takes into account both if the component itself is enabled and its owning Actor is active
		 *
		 * @note This will return false if the owning Actor is inactive
		 */
		const bool IsEnabled() {
			return actor.get().IsActive() && enabled;
		}

		/**
		 * @brief Activate or deactivate the component
		 *
		 * @param state The new activation state
		 */
		void SetEnabled(bool state) {
			enabled = state;
			OnEnableStateChange();
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

		virtual void OnEnableStateChange() {};

		std::reference_wrapper<Actor> actor [[maybe_unused]];

		friend class Actor;

	  private:
		bool enabled;
	};
}