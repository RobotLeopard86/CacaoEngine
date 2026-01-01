#pragma once

#include "DllHelper.hpp"

#include <memory>
#include <semaphore>
#include <atomic>

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
		static TickController& Get();

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
		bool IsRunning() const {
			return running;
		}

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		bool running;

		///@cond
		struct SnapshotRequestControl {
			SnapshotRequestControl() : request(false), grant(0), done(0) {}

			std::atomic_bool request;
			std::binary_semaphore grant;
			std::binary_semaphore done;
		} snapshotControl;
		friend class FrameProcessor;
		///@endcond

		TickController();
		~TickController();
	};
}