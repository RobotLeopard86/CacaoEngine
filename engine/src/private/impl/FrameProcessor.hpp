#pragma once

#include "Cacao/FrameProcessor.hpp"
#include "Cacao/EventConsumer.hpp"

#include <thread>
#include <atomic>

namespace Cacao {
	struct FrameProcessor::Impl {
		void Runloop(std::stop_token stop);

		std::unique_ptr<std::jthread> thread;
		std::atomic<unsigned int> numFramesInFlight;

		std::atomic_bool swapchainRegen;
		EventConsumer resizeConsumer;
	};
}