#include "Cacao/TickController.hpp"
#include "Cacao/ThreadPool.hpp"
#include "Cacao/Engine.hpp"

#include <chrono>
#include <array>
#include <numeric>
#include <cmath>
#include <random>

using sclock = std::chrono::steady_clock;
using namespace std::chrono_literals;

namespace Cacao {
	struct TickController::Impl {
		void DynTick(std::chrono::milliseconds timestep);
		void FixedTick();
		void Runloop(std::stop_token stop);

		std::stop_source stopper;

		std::array<std::chrono::milliseconds, 3> dynTickTimes;
		sclock::time_point nextFixedTick, lastDynTick;
	};

	TickController::TickController()
	  : running(false) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	TickController::~TickController() {
		if(running) Stop();
	}

	void TickController::Start() {
		Check<BadInitStateException>(!running, "The tick controller must not be running when Start is called!");
		Check<BadInitStateException>(ThreadPool::Get().IsRunning(), "The thread pool must be running to start the tick controller!");

		//Set next fixed tick time and prepare queue
		impl->nextFixedTick = sclock::now() + std::chrono::milliseconds(Engine::Get().config.fixedTickRate);
		impl->lastDynTick = sclock::now();
		impl->dynTickTimes[0] = 0ms;
		impl->dynTickTimes[1] = 0ms;
		impl->dynTickTimes[2] = 0ms;

		//Put run loop in the thread pool continuously
		impl->stopper = ThreadPool::Get().ExecContinuous([this](std::stop_token stop) { impl->Runloop(stop); });

		running = true;
	}

	void TickController::Stop() {
		Check<BadInitStateException>(running, "The tick controller must be running when Stop is called!");

		running = false;

		//Signal run loop stop
		impl->stopper.request_stop();
	}

	void TickController::Impl::Runloop(std::stop_token stop) {
		while(!stop.stop_requested()) {
			//Calculate some important variables
			std::chrono::milliseconds untilNextFixedTick = std::chrono::duration_cast<std::chrono::milliseconds>(nextFixedTick - sclock::now());
			std::chrono::milliseconds avgTickTime((dynTickTimes[0] + dynTickTimes[1] + dynTickTimes[2]) / 3ms);
			std::chrono::milliseconds fixedTickRate = std::chrono::milliseconds(Engine::Get().config.fixedTickRate);

			//Check if we should run a fixed tick
			constexpr std::chrono::milliseconds fixedTickGraceWindow = 1ms;
			if(sclock::now() >= (nextFixedTick - fixedTickGraceWindow) && sclock::now() <= (nextFixedTick + fixedTickGraceWindow)) {
				//Run the tick
				FixedTick();

				//Set the next tick time
				nextFixedTick += fixedTickRate;
				untilNextFixedTick = std::chrono::duration_cast<std::chrono::milliseconds>(nextFixedTick - sclock::now());
			}

			//Did we miss a fixed tick?
			if(sclock::now() > nextFixedTick) {
				//If we're over halfway until the next fixed tick, skip the one we missed
				if(sclock::now() <= (nextFixedTick + fixedTickRate / 2)) {
					Logger::Engine(Logger::Level::Warn) << "Overshot a fixed tick!";
					FixedTick();
				}
				nextFixedTick += fixedTickRate;
				untilNextFixedTick = std::chrono::duration_cast<std::chrono::milliseconds>(nextFixedTick - sclock::now());
			}

			//Check if we can probably run a dynamic tick
			if(avgTickTime < untilNextFixedTick) {
				//Run the tick
				sclock::time_point preTick = sclock::now();
				std::chrono::milliseconds ts = std::chrono::duration_cast<std::chrono::milliseconds>(preTick - lastDynTick);
				DynTick(ts);
				sclock::time_point postTick = sclock::now();
				lastDynTick = preTick;

				//Store this time in the queue
				dynTickTimes[0] = dynTickTimes[1];
				dynTickTimes[1] = dynTickTimes[2];
				dynTickTimes[2] = std::chrono::duration_cast<std::chrono::milliseconds>(postTick - preTick);
			} else {
				//We'll wait out the time until the next tick
				std::this_thread::sleep_until(nextFixedTick - 2ms);
			}
		}
	}

	void TickController::Impl::DynTick(std::chrono::milliseconds timestep) {
		Logger::Engine(Logger::Level::Trace) << "Running a dynamic tick; it's been " << timestep << " since the last one.";
		std::random_device randDev;
		std::mt19937_64 rng(randDev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, 5);
		std::chrono::milliseconds nap = std::chrono::milliseconds(dist(rng));
		std::this_thread::sleep_for(nap);
	}

	void TickController::Impl::FixedTick() {
		Logger::Engine(Logger::Level::Trace) << "Running a fixed tick.";
		std::this_thread::sleep_for(2ms);
	}
}