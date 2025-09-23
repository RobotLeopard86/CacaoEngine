#pragma once

#include "World/Component.hpp"

namespace Cacao {
	/**
	 * @brief Base class for a script
	 */
	class Script : public Component {
	  public:
		/**
		 * @brief Runs every dynamic tick
		 *
		 * @param timestep The time in seconds since the last dynamic tick
		 */
		virtual void OnTick(double timestep) {}

		/**
		 * @brief Runs every fixed tick
		 * @warning Fixed ticks are not yet implemented in the engine
		 */
		virtual void OnFixedTick() {}

		///@brief Runs on activation
		virtual void OnActivate() {}

		///@brief Runs on deactivation
		virtual void OnDeactivate() {}

		/**
		 * @brief Set the activation state
		 *
		 * @param val The new activation state
		 *
		 * @note Runs OnActive or OnDeactivate if the state changes
		 */
		void SetActive(bool val) override final {
			bool prevActive = this->IsActive();
			_SetActiveInternal(val);
			if(this->IsActive() && !prevActive) {
				OnActivate();
				return;
			}
			if(!this->IsActive() && prevActive) {
				OnDeactivate();
				return;
			}
		}

		///@brief Gets the type of this componet. Needed for safe downcasting from Component
		std::string GetKind() override final {
			return "SCRIPT";
		}

		/**
		 * @brief Check if two EventConsumers are equal
		 * @details Compares memory addresses to perform check
		 */
		const bool operator==(Script rhs) {
			return (this == &rhs);
		}

		//Virtual destructor
		virtual ~Script() {}
	};
}