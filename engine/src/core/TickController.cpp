#include "Cacao/TickController.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/WorldManager.hpp"
#include "Cacao/Script.hpp"
#include "Cacao/Input.hpp"
#include "SingletonGet.hpp"
#include "impl/TickController.hpp"
#include "cacaoengine.astra.hpp"//To complete type definitions for reflectable types

#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>
#include <thread>

using namespace std::chrono_literals;

namespace Cacao {
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
		impl->thread->join();
	}

	unsigned int TickController::GetCurrentTPS() {
		return std::accumulate(impl->tpsMeasures.begin(), impl->tpsMeasures.end(), 0) / TPS_AVG_WINDOW;
	}

	void TickController::Impl::Runloop(std::stop_token stop) {
		//Setup variables
		counter = 0;
		lastSecond = clock::now();
		lastTick = clock::now();

		//Main runloop
		while(!stop.stop_requested()) {
			//Get now
			clock::time_point now = clock::now();
			if((now - lastSecond) >= 1s) {
				lastSecond = now;
				for(unsigned int i = tpsMeasures.size() - 1; i > 0; --i) tpsMeasures[i] = tpsMeasures[i - 1];
				tpsMeasures[0] = counter;
				counter = 0;
			}

			//Throttle if needed
			if(counter > Engine::Get().GetRuntimeConfig().maxTPS) std::this_thread::sleep_until(lastSecond + 1s);

			//Run next dynamic tick
			DynTick(std::chrono::duration_cast<std::chrono::duration<float>>(now - lastTick).count());
			++counter;
			lastTick = now;

			//GPU manager synchronization
			if(gpuMgrWants.exchange(false)) {
				//Tell GPU manager it's good to make a frame
				tickControllerOwns.store(false);
				tickControllerOwns.notify_all();

				//Create stop callback
				std::stop_callback cb(stop, [this]() {
					tickControllerNeedsForShutdown.store(true);
				});

				//Block until:
				// a) GPU manager is done and we can resume
				// b) Stop callback fired and the loop will exit
				tickControllerOwns.wait(false);
			}
		}

		//We're exiting, hand off to the GPU manager
		tickControllerOwns.store(false);
		tickControllerOwns.notify_all();
	}


	void TickController::Impl::DynTick(float timestep) {
		//Freeze input state
		Input::Get().FreezeInputState();

		//Check TPS and FPS
		if(Input::Get().IsKeyPressed(CACAO_KEY_P)) Logger::Engine(Logger::Level::Trace).LogFormatted("{} TPS, {} FPS", TickController::Get().GetCurrentTPS(), GPUManager::Get().GetCurrentFPS());
		static bool vUp = true;
		if(Input::Get().IsKeyPressed(CACAO_KEY_V)) {
			if(vUp) GPUManager::Get().SetVSync(!GPUManager::Get().IsVSynced());
			vUp = false;
		} else {
			vUp = true;
		}

		//Acquire active world
		std::shared_ptr<World> world = WorldManager::Get().GetActiveWorld();
		if(!world) return;

		//Execute scripts
		static std::vector<Script*> scriptsThisTick;
		{
			std::lock_guard lk(tickScriptsMtx);
			scriptsThisTick.resize(scriptsThisTick.size());
			for(std::size_t i = 0; i < tickScripts.size(); ++i) scriptsThisTick[i] = tickScripts[i];
		}
		for(Script* s : scriptsThisTick) {
			s->OnDynTick(timestep);
		}
	}
}