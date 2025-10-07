#include "Cacao/FrameProcessor.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/GPU.hpp"
#include "SingletonGet.hpp"

#include <array>
#include <thread>

namespace Cacao {
	struct FrameProcessor::Impl {
		void Runloop(std::stop_token stop);

		std::unique_ptr<std::jthread> thread;
	};

	FrameProcessor::FrameProcessor()
	  : running(false) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	FrameProcessor::~FrameProcessor() {
		if(running) Stop();
	}

	CACAOST_GET(FrameProcessor)

	void FrameProcessor::Start() {
		Check<BadInitStateException>(!running, "The frame processor must not be running when Start is called!");
		Check<BadStateException>(GPUManager::Get().IsRunning(), "The GPU manager must be running when Start is called on the frame processor!");

		//Start runloop on background thread
		auto runloop = [this](std::stop_token stop) { impl->Runloop(stop); };
		impl->thread = std::make_unique<std::jthread>(runloop);

		running = true;
	}

	void FrameProcessor::Stop() {
		Check<BadInitStateException>(running, "The frame processor must be running when Stop is called!");

		running = false;

		//Signal run loop stop
		impl->thread->request_stop();
	}

	void FrameProcessor::Impl::Runloop(std::stop_token stop) {
		while(!stop.stop_requested()) {
		}
	}
}