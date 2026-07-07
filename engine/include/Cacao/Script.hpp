#pragma once

#include "Actor.hpp"

#include <chrono>

namespace Cacao {
	/**
	 * @brief A component that
	 */
	class Script : public Component {
	  public:
		/**
		 * @brief Runs each dynamic tick that the script is enabled and in an active World
		 *
		 * @warning This callback is called in a @b non-deterministic order; do not expect any form of consistent sequencing between scripts
		 *
		 * @param timestep The time in seconds since the last dynamic tick (not necessarily when the script was executed)
		 */
		virtual void OnDynTick(std::chrono::seconds timestep) {};

	  protected:
		Script();
	};
}