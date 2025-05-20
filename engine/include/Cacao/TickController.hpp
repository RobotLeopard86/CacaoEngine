#pragma once

#include "DllHelper.hpp"
#include "Exceptions.hpp"

#include <memory>
#include <chrono>

namespace Cacao {
	/**
	 * @brief Singleton in charge of dispatching tick calls
	 */
	class CACAO_API TickController {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static TickController& Get() {
			static TickController _instance;
			return _instance;
		}

		///@cond
		TickController(const TickController&) = delete;
		TickController(TickController&&) = delete;
		TickController& operator=(const TickController&) = delete;
		TickController& operator=(TickController&&) = delete;
		///@endcond

		/**
		 * @brief Start the controller
		 *
		 * @throws BadInitStateException If the controller is already running
		 * @throws BadInitStateException If the thread pool is not running
		 */
		void Start();

		/**
		 * @brief Stop the controller
		 *
		 * @throws BadInitStateException If the controller is not running
		 */
		void Stop();

		/**
		 * @brief Check if the controller is running
		 *
		 * @return Whether the controller is running
		 */
		bool IsRunning();

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		bool running;

		TickController();
		~TickController();
	};
}