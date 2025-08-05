#pragma once

#include <future>
#include <memory>

#include "DllHelper.hpp"
#include "ImplAccessor.hpp"

namespace Cacao {
	/**
	 * @brief A structure for usage by the GPU manager to invoke a set of GPU commands
	 *
	 * @note Only the subclasses of this are recognized by the GPUManager implementations; for that reason it is not currently possible to create custom jobs except through the designated interfaces
	 */
	class CACAO_API CommandBuffer {
	  public:
		//Preset command buffer generators will exist here eventually
	  protected:
		CommandBuffer();
		void Execute();

		friend class GPUManager;
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
		 * @brief Submit a task to the GPU for processing
		 *
		 * @param cmd The CommandBuffer containing the task to execute
		 *
		 * @returns A future that will resolve when the task has finished executing
		 */
		std::shared_future<void> Submit(const CommandBuffer& cmd);

		/**
		 * @brief Submit a task to the GPU for processing, detached from the caller
		 *
		 * @warning If using this function, there is <b>no way</b> to tell if the command has executed. For this reason, it is <b>strongly encouraged</b> to use Submit instead.
		 *
		 * @param cmd The CommandBuffer containing the task to execute
		 */
		void SubmitDetached(const CommandBuffer& cmd);

		///@cond
		class Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		GPUManager();
		~GPUManager();
	};
}