#pragma once

#include "Actor.hpp"
#include "DllHelper.hpp"

#include <chrono>

namespace Cacao {
	/**
	 * @brief A component that
	 */
	class CACAO_API ASTRA_REFLECT Script : public Component {
	  public:
		/**
		 * @brief Runs each dynamic tick that the script is enabled and in an active World
		 *
		 * @warning This callback is called in a @b non-deterministic order; do not expect any form of consistent sequencing between scripts
		 *
		 * @param timestep The time in seconds since the last dynamic tick (not necessarily when the script was executed)
		 */
		virtual void OnDynTick([[maybe_unused]] std::chrono::seconds timestep) {};

		ASTRASETUP(Script)

#ifdef _ASTRAGENERATE
		Script() {}
#endif
	  protected:
#ifndef _ASTRAGENERATE
		Script() {}
		friend struct astra::CommonActions<Script>;
#endif
	};
}