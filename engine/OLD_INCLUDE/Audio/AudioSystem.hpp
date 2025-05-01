#pragma once

#include "Sound.hpp"
#include "World/Entity.hpp"
#include "Utilities/Task.hpp"

#include <queue>
#include <mutex>

namespace Cacao {
	/**
	 * @brief Global audio system
	 */
	class AudioSystem {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static AudioSystem* Get();

		/**
		 * @brief Initialize the audio system
		 *
		 * @note This function is called by the engine during startup
		 *
		 * @throws Exception If the system was already initialized
		 */
		void Init();

		/**
		 * @brief Shut down the audio system
		 *
		 * @note This function is called by the engine during shutdown
		 *
		 * @throws Exception If the system was not initialized
		 */
		void Shutdown();

		/**
		 * @brief Set the global gain value
		 *
		 * @param value The new global gain value, which must be positive
		 *
		 * @throws Exception If the provided value was not positive
		 */
		void SetGlobalGain(float value);

		/**
		 * @brief Get the global gain value
		 *
		 * @return The current global gain value,
		 */
		float GetGlobalGain();

		/**
		 * @brief Check if the audio system is initialized
		 *
		 * @return If the system is initialized
		 */
		bool IsInitialized() {
			return isInitialized;
		}

	  private:
		//Singleton members
		static AudioSystem* instance;
		static bool instanceExists;

		bool isInitialized;

		//Audio context and device
		struct ALCObjects;
		ALCObjects* alc;

		AudioSystem()
		  : isInitialized(false) {}
	};
}
