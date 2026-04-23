#pragma once

#include "Cacao/FrameProcessor.hpp"
#include "Cacao/EventConsumer.hpp"

#include <thread>
#include <atomic>
#include <chrono>

#define FPS_AVG_WINDOW 5

namespace Cacao {
	using clock = std::chrono::steady_clock;

	struct FrameProcessor::Impl {
		void Runloop(std::stop_token stop);

		std::unique_ptr<std::jthread> thread;
		std::atomic<unsigned int> numFramesInFlight;

		std::atomic_bool swapchainRegen;
		EventConsumer resizeConsumer;

		unsigned int counter;
		clock::time_point lastSecond;
		std::array<unsigned int, FPS_AVG_WINDOW> fpsMeasures;
	};
}