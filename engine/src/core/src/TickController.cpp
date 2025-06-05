#include "Cacao/TickController.hpp"
#include "Cacao/ThreadPool.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Time.hpp"

#include <chrono>
#include <array>
#include <numeric>
#include <cmath>
#include <random>

namespace Cacao {
	struct TickController::Impl {
		void DynTick(time::dseconds timestep);
		void FixedTick();
		void Runloop(std::stop_token stop);

		std::stop_source stopper;

		std::array<time::dseconds, 3> dynTickTimes;
		time::dtime_point lastTickTimeUpdate;
		time::dtime_point nextFixedTick, lastDynTick;
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
		time::dtime_point now = std::chrono::steady_clock::now();
		impl->nextFixedTick = now + time::dseconds(Engine::Get().config.fixedTickRate);
		impl->lastDynTick = now;
		impl->dynTickTimes[0] = 0_ds;
		impl->dynTickTimes[1] = 0_ds;
		impl->dynTickTimes[2] = 0_ds;
		impl->lastTickTimeUpdate = now;

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
			time::dtime_point now = std::chrono::steady_clock::now();
			time::dseconds untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			time::dseconds avgTickTime((dynTickTimes[0] + dynTickTimes[1] + dynTickTimes[2]).count() / 3.0);
			time::fseconds fixedTickRate = std::chrono::duration_cast<time::fseconds>(Engine::Get().config.fixedTickRate);

			//Check if we should run a fixed tick
			constexpr time::dmilliseconds fixedTickGraceWindow = 1_dms;
			if(now >= (nextFixedTick - fixedTickGraceWindow) && now <= (nextFixedTick + fixedTickGraceWindow)) {
				//Run the tick
				FixedTick();

				//Set the next tick time
				nextFixedTick += fixedTickRate;
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			}

			//Did we miss a fixed tick?
			if(now > (nextFixedTick + fixedTickGraceWindow)) {
				//If we're over halfway until the next fixed tick, skip the one we missed
				if(now <= (nextFixedTick + fixedTickGraceWindow + (fixedTickRate / 2))) {
					Logger::Engine(Logger::Level::Warn) << "Overshot a fixed tick!";
					FixedTick();
				}
				nextFixedTick += fixedTickRate;
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			}

			//Clear tick time queue if it's been long enough, since this is probably out-of-date now
			constexpr time::dmilliseconds maxTimeSinceDynTicks = 10_dms;
			if((time::dtime_point(now) - lastTickTimeUpdate) >= maxTimeSinceDynTicks) {
				Logger::Engine(Logger::Level::Warn) << "Dynamic tick time calculation reset needed!";
				dynTickTimes[0] = 0_ds;
				dynTickTimes[1] = 0_ds;
				dynTickTimes[2] = 0_ds;
				lastTickTimeUpdate = now;
				avgTickTime = 0_ds;
			}

			//Check if we can probably run a dynamic tick
			if(avgTickTime < untilNextFixedTick) {
				//Run the tick
				time::dtime_point preTick = now;
				time::dseconds ts = std::chrono::duration_cast<time::dseconds>(preTick - lastDynTick);
				DynTick(ts);
				time::dtime_point postTick = now;
				lastDynTick = preTick;

				//Store this time in the queue
				dynTickTimes[0] = dynTickTimes[1];
				dynTickTimes[1] = dynTickTimes[2];
				dynTickTimes[2] = std::chrono::duration_cast<time::dseconds>(postTick - preTick);
			} else {
				//We'll wait out the time until the next tick
				std::this_thread::sleep_until(nextFixedTick - 2ms);
			}
		}
	}

	void TickController::Impl::DynTick(time::dseconds timestep) {
		Logger::Engine(Logger::Level::Trace) << "Running a dynamic tick; it's been " << timestep << " since the last one.";
		std::random_device randDev;
		std::mt19937_64 rng(randDev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(20, 500);
		std::chrono::microseconds nap = std::chrono::microseconds(dist(rng));
		std::this_thread::sleep_for(nap);
	}

	void TickController::Impl::FixedTick() {
		Logger::Engine(Logger::Level::Trace) << "Running a fixed tick.";
		std::this_thread::sleep_for(0.2_dms);
	}
}