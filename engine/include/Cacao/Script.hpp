#pragma once

#include "Component.hpp"

namespace Cacao {
	class Script : public Component {
	  public:
		/**
		 * @brief Runs when the World containing the script is activated or the script is first mounted on an Actor
		 */
		virtual void OnLoad() {};

		/**
		 * @brief Runs when the script is enabled or when the owning Actor becomes active if the script was already enabled
		 */
		virtual void OnEnable() {};

		/**
		 * @brief Runs when the script is disabled or when the owning Actor becomes inactive if the script was already enabled
		 */
		virtual void OnDisable() {};

		/**
		 * @brief Runs each dynamic tick that the script is enabled and in an active World
		 *
		 * Most logic should be executed here.
		 *
		 * @param timestep The time in seconds since the last dynamic tick (not necessarily when the script was executed)
		 */
		virtual void OnDynTick(double timestep) {};

		/**
		 * @brief Runs each fixed tick that the script is enabled and in an active World
		 *
		 * @warning Avoid doing too much complicated stuff here to avoid fixed tick slowdowns.
		 */
		virtual void OnFixedTick() {};

	  protected:
		Script();

		void OnEnableStateChange() override;
	};
}