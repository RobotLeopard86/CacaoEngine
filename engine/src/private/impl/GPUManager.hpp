#pragma once

#include "Cacao/GPU.hpp"

#include <stop_token>

namespace Cacao {
	class GPUManager::Impl {
	  public:
		virtual std::shared_future<void> SubmitCmdBuffer(CommandBuffer&& cmd) = 0;
		virtual void RunloopStart() = 0;
		virtual void RunloopStop() = 0;
		virtual void RunloopIteration() = 0;

		virtual ~Impl() = default;

		void Runloop(std::stop_token stop);
		std::stop_source stopper;

		struct VSyncRequest {
			bool needChange;
			bool value;
			std::mutex mtx;
		} vsreq;
	};
}