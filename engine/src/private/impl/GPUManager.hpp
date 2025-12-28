#pragma once

#include "Cacao/GPU.hpp"

#include <thread>
#include <atomic>

namespace Cacao {
	class GPUManager::Impl {
	  public:
		virtual std::shared_future<void> SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) = 0;
		virtual void RunloopStart() = 0;
		virtual void RunloopStop() = 0;
		virtual void RunloopIteration() = 0;

		virtual ~Impl() = default;

		void Runloop(std::stop_token stop);
		std::unique_ptr<std::jthread> thread;

		virtual bool UsesImmediateExecution() = 0;
		virtual unsigned int MaxFramesInFlight() {
			return UINT32_MAX;
		}
		virtual void GenSwapchain() = 0;

		struct VSyncRequest {
			std::atomic_bool needChange;
			bool value;
		} vsreq;
	};
}