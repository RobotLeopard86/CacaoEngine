#pragma once

#include "DllHelper.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Audio system manager singleton
	 */
	class CACAO_API AudioManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static AudioManager& Get() {
			static AudioManager _instance;
			return _instance;
		}

		///@cond
		AudioManager(const AudioManager&) = delete;
		AudioManager(AudioManager&&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;
		AudioManager& operator=(AudioManager&&) = delete;
		///@endcond

		/**
		 * @brief Initialize the audio system
		 *
		 * @note This function is called by the engine during startup
		 *
		 * @throws BadInitStateException If the system was already initialized
		 *
		 * @return Whether initialization succeeded
		 */
		bool Initialize();

		/**
		 * @brief Terminate the audio system
		 *
		 * @note This function is called by the engine during shutdown
		 *
		 * @throws BadInitStateException If the system was not initialized
		 */
		void Terminate();

		/**
		 * @brief Check if the system is initialized
		 *
		 * @return Whether the audio system is initialized
		 */
		bool IsInitialized();

		/**
		 * @brief Set the global gain value
		 *
		 * @param value The new global gain value, which must be positive
		 *
		 * @throws BadValueException If the provided value was not positive
		 */
		void SetGlobalGain(float value);

		/**
		 * @brief Get the global gain value
		 *
		 * @return The current global gain value,
		 */
		float GetGlobalGain();

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		AudioManager();
		~AudioManager();
	};
}