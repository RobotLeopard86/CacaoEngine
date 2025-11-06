#include "Cacao/TickController.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Time.hpp"
#include "Cacao/WorldManager.hpp"
#include "Cacao/Script.hpp"
#include "SingletonGet.hpp"

#include "exathread.hpp"
#include "high_resolution_sleep.hpp"

#include <atomic>
#include <chrono>
#include <array>
#include <memory>
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
		time::dtime_point nextFixedTick;
		time::dtime_point lastDynTick;

		time::dtime_point lastMinute;
		unsigned int missedOrLateFixedTicks;
		bool loggedExcessiveMisses;
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
		//Set next fixed tick time and prepare queue
		time::dtime_point now = std::chrono::steady_clock::now();
		nextFixedTick = now + Engine::Get().config.fixedTickInterval;
		lastDynTick = now;
		dynTickTimes[0] = 0_ds;
		dynTickTimes[1] = 0_ds;
		dynTickTimes[2] = 0_ds;
		lastMinute = now;
		missedOrLateFixedTicks = 0;
		loggedExcessiveMisses = false;

		//Setup world lock
		std::unique_lock worldLock(TickController::Get().snapshotControl.mutex);

		while(!stop.stop_requested()) {
			//Calculate some important variables
			now = std::chrono::steady_clock::now();
			std::chrono::milliseconds fixedTickInterval = Engine::Get().config.fixedTickInterval;
			time::dseconds untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			time::dseconds avgTickTime = (dynTickTimes[0] + dynTickTimes[1] + dynTickTimes[2]) / 3.0;

			//Check for frame processor snapshot request
			if(TickController::Get().snapshotControl.request.exchange(false, std::memory_order_acq_rel)) {
				//Release lock
				worldLock.unlock();

				//Allow frame processor to run
				TickController::Get().snapshotControl.grant.release();

				//Wait until it's done by blocking on the done semaphore
				while(!TickController::Get().snapshotControl.done.try_acquire()) {
					std::this_thread::yield();
					if(stop.stop_requested()) return;
				}

				//We're done, so relock the world state and update our variables
				worldLock.lock();
				now = std::chrono::steady_clock::now();
				while(now > nextFixedTick) nextFixedTick += fixedTickInterval;
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			}

			//Update fixed tick miss counter
			if((now - lastMinute) >= 1min) {
				lastMinute = now;
				missedOrLateFixedTicks = 0;
				loggedExcessiveMisses = false;
			}

			//Check if we should run a fixed tick
			constexpr time::dseconds fixedTickGraceWindow = 1ms;
			if(now >= (nextFixedTick - fixedTickGraceWindow) && now <= (nextFixedTick + fixedTickGraceWindow)) {
				//Run the tick
				FixedTick();

				//Update tick state
				nextFixedTick += fixedTickInterval;
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);
			} else if(now > (nextFixedTick + fixedTickGraceWindow)) {
				//If we're not halfway until the next fixed tick, we'll run the tick, but otherwise we'll skip it
				if(now < nextFixedTick + (fixedTickInterval / 2)) {
					FixedTick();
				}

				//Update tick state
				//Ensure we have at least a full interval until the next tick
				do {
					nextFixedTick += fixedTickInterval;
				} while((nextFixedTick - now) < (fixedTickInterval - fixedTickGraceWindow));
				untilNextFixedTick = std::chrono::duration_cast<time::dseconds>(nextFixedTick - now);

				//Update miss tracker
				++missedOrLateFixedTicks;
				if(!loggedExcessiveMisses && missedOrLateFixedTicks > ceil((1000.0 / fixedTickInterval.count() * 60) * 0.05)) {
					Logger::Engine(Logger::Level::Warn) << "5% of fixed ticks in the last minute were late or missed!";
					loggedExcessiveMisses = true;
				}
			}

			//Clear tick time queue if it's been long enough, since this is probably out-of-date now
			const static std::chrono::milliseconds maxTimeSinceDynTicks = fixedTickInterval * 2;
			if((now - lastDynTick) >= maxTimeSinceDynTicks) {
				Logger::Engine(Logger::Level::Warn) << "Dynamic tick time calculation reset needed!" << " (Behind: " << std::chrono::duration_cast<time::dmilliseconds>(now - lastDynTick) << ")";
				dynTickTimes[0] = 0_ds;
				dynTickTimes[1] = 0_ds;
				dynTickTimes[2] = 0_ds;
				now = std::chrono::steady_clock::now();
				avgTickTime = 0_ds;
			}

			//Update now
			now = std::chrono::steady_clock::now();

			//Check if we can probably run a dynamic tick
			if(avgTickTime <= untilNextFixedTick) {
				//Run the tick
				time::dtime_point preTick = now;
				time::dseconds ts = std::chrono::duration_cast<time::dseconds>(preTick - lastDynTick);
				DynTick(ts);
				time::dtime_point postTick = std::chrono::steady_clock::now();
				lastDynTick = postTick;

				//Store this time in the queue
				dynTickTimes[0] = dynTickTimes[1];
				dynTickTimes[1] = dynTickTimes[2];
				dynTickTimes[2] = std::chrono::duration_cast<time::dseconds>(postTick - preTick);
			} else {
				//We'll wait out the time until the next tick more or less
				std::chrono::milliseconds wt = std::chrono::duration_cast<std::chrono::milliseconds>(nextFixedTick - now - fixedTickGraceWindow);
				if(wt.count() > 0) {
					high_resolution_sleep::sleep_ms(wt.count());
				}
			}
		}

//If on Windows, we have to reset the time period since high_resolution_sleep messes with it.
#ifdef _WIN32
		timeEndPeriod(1);
#endif
	}

	void TickController::Impl::DynTick(time::dseconds timestep [[maybe_unused]]) {
		//Acquire active world
		std::shared_ptr<World> world = WorldManager::Get().GetActiveWorld();
		if(!world) return;

		//Find scripts
		std::vector<std::shared_ptr<Script>> scripts;
		std::mutex scriptsMtx;
		const auto scriptFinder = [world, &scripts, &scriptsMtx](ActorHandle ah) -> exathread::VoidTask {
			auto impl = [world, &scripts, &scriptsMtx](ActorHandle ah, auto& iref) -> exathread::VoidTask {
				//Inactive stop
				if(!ah->IsActive()) co_return;

				//Process components
				for(auto& [ctype, cptr] : ah->GetAllComponents()) {
					//Is this a script?
					if(auto sptr = std::dynamic_pointer_cast<Script>(cptr)) {
						std::lock_guard lk(scriptsMtx);
						scripts.push_back(sptr);
					}
				}

				//Handle children
				exathread::MultiFuture<void> childrenFut = Engine::Get().GetThreadPool()->batch(world->GetRootChildren(), iref, iref);
				co_await exathread::yieldUntilComplete(childrenFut);
			};
			impl(ah, impl);
			co_return;
		};
		exathread::MultiFuture<void> scriptsFut = Engine::Get().GetThreadPool()->batch(world->GetRootChildren(), scriptFinder);
		scriptsFut.await();
	}

	void TickController::Impl::FixedTick() {
		//dummy
		high_resolution_sleep::sleep_ms(2);
	}
}