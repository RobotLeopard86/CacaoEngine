#pragma once

#include "DllHelper.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Singleton in charge of generating frame rendering command buffers and submitting them to the GPUManager
	 */
	class CACAO_API FrameProcessor {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static FrameProcessor& Get();

		///@cond
		FrameProcessor(const FrameProcessor&) = delete;
		FrameProcessor(FrameProcessor&&) = delete;
		FrameProcessor& operator=(const FrameProcessor&) = delete;
		FrameProcessor& operator=(FrameProcessor&&) = delete;
		///@endcond

		/**
		 * @brief Start the frame processor
		 *
		 * @throws BadInitStateException If the frame processor is already running
		 * @throws BadStateException If the GPU manager is not running
		 */
		void Start();

		/**
		 * @brief Stop the frame processor
		 *
		 * @throws BadInitStateException If the frame processor is not running
		 */
		void Stop();

		/**
		 * @brief Check if the frame processor is running
		 *
		 * @return Whether the frame processor is running
		 */
		bool IsRunning() const {
			return running;
		}

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		bool running;

		FrameProcessor();
		~FrameProcessor();
	};
}