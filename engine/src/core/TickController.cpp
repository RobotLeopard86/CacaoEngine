#include "Cacao/TickController.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/FrameProcessor.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/WorldManager.hpp"
#include "Cacao/Script.hpp"
#include "Cacao/Input.hpp"
#include "SingletonGet.hpp"

#include "exathread.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>
#include <thread>

using namespace std::chrono_literals;

#define TPS_AVG_WINDOW 5

namespace Cacao {
	using clock = std::chrono::steady_clock;

	struct TickController::Impl {
		void DynTick(std::chrono::seconds timestep);
		void Runloop(std::stop_token stop);

		std::unique_ptr<std::jthread> thread;
		unsigned int counter;
		clock::time_point lastSecond;
		clock::time_point lastTick;
		std::array<unsigned int, TPS_AVG_WINDOW> tpsMeasures;
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

		//If frame processor was started before us, then we should wait until it's done with whatever it's doing before starting
		while(!TickController::Get().snapshotControl.done.try_acquire()) {
			std::this_thread::yield();
			if(stop.stop_requested()) return;
		}

		//Main runloop
		while(!stop.stop_requested()) {
			//Check for frame processor snapshot request
			if(TickController::Get().snapshotControl.request.exchange(false, std::memory_order_acq_rel)) {
				//Allow frame processor to run
				TickController::Get().snapshotControl.grant.release();

				//Wait until it's done by blocking on the done semaphore
				while(!TickController::Get().snapshotControl.done.try_acquire()) {
					std::this_thread::yield();
					if(stop.stop_requested()) return;
				}
			}

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
			DynTick(std::chrono::duration_cast<std::chrono::seconds>(now - lastTick));
			++counter;
			lastTick = now;
		}
	}

	exathread::ValueTask<std::vector<Script*>> FindScripts(ActorRef actor) {
		//Inactive actor stop
		if(!actor->IsActive()) co_return {};

		//Get all scripts and add them to the list
		auto actorScripts = actor->GetComponentsFiltered([](const std::unique_ptr<Component>& component) {
			return (dynamic_cast<Script*>(component.get()));
		}) | std::views::transform([](const std::unordered_map<std::type_index, Component*>::value_type& item) {
			return static_cast<Script*>(item.second);
		}) | std::views::common;
		std::vector scripts(actorScripts.begin(), actorScripts.end());

		//Handle children
		exathread::MultiFuture<std::vector<Script*>> childScriptsFut = Engine::Get().GetThreadPool()->batch(actor->GetAllChildren(), FindScripts);
		co_await exathread::yieldUntilComplete(childScriptsFut);

		//Merge lists and return
		std::vector<std::vector<Script*>> toMerge = childScriptsFut.results();
		toMerge.push_back(std::move(scripts));
		auto joined = std::views::join(toMerge) | std::views::common;
		co_return std::vector<Script*>(joined.begin(), joined.end());
	}

	void TickController::Impl::DynTick(std::chrono::seconds timestep) {
		//Freeze input state
		Input::Get().FreezeInputState();

		//Check TPS and FPS
		if(Input::Get().IsKeyPressed(CACAO_KEY_P)) Logger::Engine(Logger::Level::Trace).LogFormatted("{} TPS, {} FPS", TickController::Get().GetCurrentTPS(), FrameProcessor::Get().GetCurrentFPS());
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

		//Find scripts
		if(world->GetToplevelActors().size() > 0) {
			//Wait for scripts to be returned
			exathread::MultiFuture<std::vector<Script*>> scriptsFut = Engine::Get().GetThreadPool()->batch(world->GetToplevelActors(), FindScripts);
			scriptsFut.await();

			//Execute scripts
			std::vector<std::vector<Script*>> toMerge = scriptsFut.results();
			auto joined = std::views::join(toMerge) | std::views::common;
			for(Script* s : joined) {
				s->OnDynTick(timestep);
			}
		}
	}
}