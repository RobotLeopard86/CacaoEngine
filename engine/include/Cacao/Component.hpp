#pragma once

#include "Actor.hpp"
#include "Cacao/ComponentExporter.hpp"
#include "DllHelper.hpp"

#include <memory>

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
		bool IsEnabled() const {
			return actor.lock()->IsActive() && enabled;
		}

		/**
		 * @brief Activate or deactivate the component
		 *
		 * @param state The new activation state
		 */
		void SetEnabled(bool state) {
			enabled = state;
			if(IsEnabled()) {
				OnEnable();
			}
		}

		/**
		 * @brief Get the owning actor of this component
		 *
		 * @return A reference to the owning actor
		 *
		 * @throws NonexistentValueException If the actor no longer exists
		 */
		std::shared_ptr<Actor> GetOwner() const {
			Check<NonexistentValueException>(!actor.expired(), "Cannot get expired Actor from Component!");
			return actor.lock();
		}

		virtual ~Component() {}

	  protected:
		Component();

		/**
		 * @brief Runs when the component is first mounted on an Actor
		 *
		 * @note All setup should be performed here, <b>NOT</b> in the constructor. Only when this function is called is the component properly configured.
		 */
		virtual void OnMount() {};

		/**
		 * @brief Runs when the component is enabled or when the owning Actor becomes active if the component was already enabled
		 */
		virtual void OnEnable() {};

		/**
		 * @brief Runs when the component is disabled or when the owning Actor becomes inactive if the component was already enabled
		 */
		virtual void OnDisable() {};

		/**
		 * @warning Do not lock and store this pointer for longer than necessary (i.e. function scope)! Doing so will break world tree unwinding!
		 */
		std::weak_ptr<Actor> actor [[maybe_unused]];

		friend class Actor;
		friend class ComponentExporter;

	  private:
		bool enabled;
		std::shared_ptr<ComponentExporter> expHnd;
	};
}