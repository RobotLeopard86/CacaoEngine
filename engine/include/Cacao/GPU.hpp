#pragma once

#include <future>
#include <memory>

#include "DllHelper.hpp"

#include "glm/glm.hpp"

namespace Cacao {
	/**
	 * @brief A structure for usage by the GPU manager to invoke a set of GPU commands
	 */
	class CACAO_API CommandBuffer {
	  public:
		/**
		 * @brief Create a new empty command buffer
		 */
		static std::unique_ptr<CommandBuffer> Create();

		///@cond
		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&);
		CommandBuffer& operator=(CommandBuffer&&);
		///@endcond

		virtual ~CommandBuffer() {};

	  protected:
		CommandBuffer() {}

		uint64_t token;

		virtual void Execute() {};

		friend class FrameProcessor;
		friend class PALModule;

		virtual bool SetupContext(bool rendering = false) {
			return true;
		}
		virtual void StartRendering(glm::vec3 clearColor) {}
		virtual void EndRendering() {}
	};

	/**
	 * @brief The centralized GPU interaction system
	 *
	 * @warning Wild usage of this system is <b>not recommended</b> since you will end up fighting with the world renderer. For this reason, you should only access this interface during the pre- and post-render hooks in the FrameProcessor (not yet implemented).
	 */
	class CACAO_API GPUManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static GPUManager& Get();

		///@cond
		GPUManager(const GPUManager&) = delete;
		GPUManager(GPUManager&&) = delete;
		GPUManager& operator=(const GPUManager&) = delete;
		GPUManager& operator=(GPUManager&&) = delete;
		///@endcond

		/**
		 * @brief Start the GPU manager
		 *
		 * @throws BadInitStateException If the GPU manager is already running
		 * @throws BadStateException If the graphics backend and window are not connected
		 */
		void Start();

		/**
		 * @brief Stop the GPU manager
		 *
		 * @throws BadInitStateException If the GPU manager is not running
		 */
		void Stop();

		/**
		 * @brief Check if the GPU manager is running
		 *
		 * @return Whether the GPU manager is running
		 */
		bool IsRunning() const {
			return running;
		}

		/**
		 * @brief Submit a task to the GPU for processing
		 *
		 * @param cmd The CommandBuffer containing the task to execute
		 *
		 * @returns A future that will resolve when the task has finished executing
		 */
		std::shared_future<void> Submit(std::unique_ptr<CommandBuffer> cmd);

		/**
		 * @brief Set the V-Sync state
		 *
		 * @note Depending on what rendering API is in use, the change may not take effect instantly. However, it is guaranteed to take effect.
		 *
		 * @param newState Whether V-Sync should be enabled
		 *
		 * @throws BadInitStateException If the GPU manager is not running
		 */
		void SetVSync(bool newState);

		///@cond
		class Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
		friend class PAL;

		bool running;

		GPUManager();
		~GPUManager();
	};
}