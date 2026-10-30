#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "Cacao/Script.hpp"
#include "Cacao/TickController.hpp"

#define TPS_AVG_WINDOW 5

namespace Cacao {
	using clock = std::chrono::steady_clock;

	struct TickController::Impl {
		//Methods
		void DynTick(float timestep);
		void Runloop(std::stop_token stop);

		//The thread of doom
		std::unique_ptr<std::jthread> thread;

		//TPS tracking
		unsigned int counter;
		clock::time_point lastSecond;
		clock::time_point lastTick;
		std::array<unsigned int, TPS_AVG_WINDOW> tpsMeasures;

		//GPU manager synchronization
		std::atomic<bool> tickControllerOwns = true;
		std::atomic<bool> gpuMgrWants = false;
		std::atomic<bool> tickControllerNeedsForShutdown = false;

		//List of scripts to run each tick
		std::vector<Script*> tickScripts;
		std::mutex tickScriptsMtx;
	};
}