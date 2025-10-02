#include "Cacao/TickController.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Time.hpp"
#include "SingletonGet.hpp"

#include "high_resolution_sleep.hpp"

#include <chrono>
#include <array>
#include <random>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#include <timeapi.h>
#endif

namespace Cacao {
	struct TickController::Impl {
		void DynTick(time::dseconds timestep);
		void FixedTick();
		void Runloop(std::stop_token stop);

		std::unique_ptr<std::jthread> thread;

		std::array<time::dseconds, 3> dynTickTimes;
		time::dtime_point lastTickTimeUpdate;
		time::dtime_point nextFixedTick;
		time::dtime_point lastDynTick;
	};

	TickController::TickController()
	  : running(false) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	TickController::~TickController() {
		if(running) Stop();
	}

	CACAOST_GET(TickController)

	void TickController::Start() {
		Check<BadInitStateException>(!running, "The tick controller must not be running when Start is called!");

		//Set next fixed tick time and prepare queue
		time::dtime_point now = std::chrono::steady_clock::now();
		impl->nextFixedTick = now + time::dseconds(Engine::Get().config.fixedTickRate);
		impl->lastDynTick = now;
		impl->dynTickTimes[0] = 0_ds;
		impl->dynTickTimes[1] = 0_ds;
		impl->dynTickTimes[2] = 0_ds;
		impl->lastTickTimeUpdate = now;

		//Start runloop on background thread
		auto runloop = [this](std::stop_token stop) { impl->Runloop(stop); };
		impl->thread = std::make_unique<std::jthread>(runloop);

		running = true;
	}

	void TickController::Stop() {
		Check<BadInitStateException>(running, "The tick controller must be running when Stop is called!");

		running = false;

		//Signal run loop stop
		impl->thread->request_stop();
	}

	void TickController::Impl::Runloop(std::stop_token stop) {
		while(!stop.stop_requested()) {
			//Calculate some important variables
			time::dtime_point now = std::chrono::steady_clock::now();
			time::dseconds untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			time::dseconds avgTickTime((dynTickTimes[0] + dynTickTimes[1] + dynTickTimes[2]).count() / 3.0);
			time::fseconds fixedTickRate = std::chrono::duration_cast<time::fseconds>(Engine::Get().config.fixedTickRate);

			//Check if we should run a fixed tick
			constexpr std::chrono::milliseconds fixedTickGraceWindow = 2ms;
			if(now >= (nextFixedTick - fixedTickGraceWindow) && now <= (nextFixedTick + fixedTickGraceWindow)) {
				//Run the tick
				FixedTick();

				//Variable update
				now = std::chrono::steady_clock::now();
				nextFixedTick = now + fixedTickRate;
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			} else if(now > (nextFixedTick + fixedTickGraceWindow)) {
				//If we're over halfway until the next fixed tick, skip the one we missed
				if(now <= (nextFixedTick + fixedTickGraceWindow + (fixedTickRate / 2))) {
					Logger::Engine(Logger::Level::Warn) << "Overshot a fixed tick! (Past: " << std::chrono::duration_cast<time::dmilliseconds>(now - (nextFixedTick + fixedTickGraceWindow)) << ")";
					FixedTick();
				}

				//Variable update
				now = std::chrono::steady_clock::now();
				nextFixedTick = now + fixedTickRate;
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			}

			//Update now
			now = std::chrono::steady_clock::now();

			//Clear tick time queue if it's been long enough, since this is probably out-of-date now
			constexpr std::chrono::milliseconds maxTimeSinceDynTicks = 15ms;
			if((time::dtime_point(now) - lastTickTimeUpdate) >= maxTimeSinceDynTicks) {
				Logger::Engine(Logger::Level::Warn) << "Dynamic tick time calculation reset needed!" << " (Behind: " << std::chrono::duration_cast<time::dmilliseconds>(time::dtime_point(now) - lastTickTimeUpdate) << ")";
				dynTickTimes[0] = 0_ds;
				dynTickTimes[1] = 0_ds;
				dynTickTimes[2] = 0_ds;
				lastTickTimeUpdate = now;
				avgTickTime = 0_ds;
			}

			//Update now again
			//Look we gotta be accurate, ok?
			now = std::chrono::steady_clock::now();

			//Check if we can probably run a dynamic tick
			if(avgTickTime < (untilNextFixedTick + fixedTickGraceWindow)) {
				//Run the tick
				time::dtime_point preTick = now;
				time::dseconds ts = std::chrono::duration_cast<time::dseconds>(preTick - lastDynTick);
				DynTick(ts);
				now = std::chrono::steady_clock::now();
				time::dtime_point postTick = now;
				lastDynTick = postTick;

				//Store this time in the queue
				dynTickTimes[0] = dynTickTimes[1];
				dynTickTimes[1] = dynTickTimes[2];
				dynTickTimes[2] = std::chrono::duration_cast<time::dseconds>(postTick - preTick);
				lastTickTimeUpdate = now;
			} else {
				//We'll wait out the time until the next tick more or less
				high_resolution_sleep::sleep_ms(std::chrono::duration_cast<time::dmilliseconds>(nextFixedTick - now - 0.2_dms).count());
			}
		}

//If on Windows, we have to reset the time period since high_resolution_sleep messes with it.
#ifdef _WIN32
		timeEndPeriod(1);
#endif
	}

	void TickController::Impl::DynTick(time::dseconds timestep [[maybe_unused]]) {
		//dummy
		static std::random_device randDev;
		static std::mt19937_64 rng(randDev());
		static std::uniform_int_distribution<std::mt19937::result_type> dist(1, 3);
		high_resolution_sleep::sleep_ms(dist(rng));
	}

	void TickController::Impl::FixedTick() {
		//dummy
		high_resolution_sleep::sleep_ms(2);
	}
}