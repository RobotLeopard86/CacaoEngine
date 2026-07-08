#pragma once

#include "Cacao/GPU.hpp"
#include "DllHelper.hpp"

#include "crossguid/guid.hpp"

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

		/**
		 * @brief Check the current FPS (frames per second) of the engine
		 *
		 * @return The FPS
		 */
		unsigned int GetCurrentFPS();

		/**
		 * @brief Describes during what phase of rendering a custom callback should be run
		 */
		enum class Phase {
			Opaque,	   ///<Opaque geometry pass
			Transparent///<Transparent geometry pass
		};

		/**
		 * @brief Register a custom rendering callback
		 *
		 * @param callback The function to run
		 * @param phase What rendering phase to run the callback in
		 * @param runPost Whether to run the callback post-phase (if true) or pre-phase (if false)
		 *
		 * @return A GUID that can be used to unregister the callback
		 */
		xg::Guid RegisterRenderingCallback(std::function<void(std::unique_ptr<CommandBuffer>&)> callback, Phase phase, bool runPost);

		/**
		 * @brief Unregister a custom rendering callback
		 *
		 * @param callbackGUID The GUID returned from the registration function to identify the callback
		 *
		 * @throws NonexistentValueException If no registered callback is associated with the provided GUID
		 */
		void UnregisterRenderingCallback(const xg::Guid& callbackGUID);

		///@cond
		struct Impl;
		///@endcond

	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		bool running;

		FrameProcessor();
		~FrameProcessor();
	};
}