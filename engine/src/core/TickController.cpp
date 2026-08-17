#include "Cacao/TickController.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/FrameProcessor.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/WorldManager.hpp"
#include "Cacao/Script.hpp"
#include "Cacao/Input.hpp"
#include "SingletonGet.hpp"
#include "impl/TickController.hpp"

#include "exathread.hpp"

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
			DynTick(std::chrono::duration_cast<std::chrono::seconds>(now - lastTick));
			++counter;
			lastTick = now;

			//Frame processor synchronization
			if(frameProcessorWants.exchange(false)) {
				//Tell frame processor it's good and wake it up
				tickControllerOwns.store(false);
				tickControllerOwns.notify_all();

				//Create stop callback
				std::stop_callback cb(stop, [this]() {
					tickControllerNeedsForShutdown.store(true);
				});

				//Block until:
				// a) Frame processor is done and we can resume
				// b) Stop callback fired and the loop will exit
				tickControllerOwns.wait(false);
			}
		}

		//We're exiting, hand off to frame processor
		tickControllerOwns.store(false);
		tickControllerOwns.notify_all();
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
		std::vector<Script*> scripts(actorScripts.begin(), actorScripts.end());

		//Handle children
		if(actor->GetAllChildren().size() > 0) {
			//Find them
			exathread::MultiFuture<std::vector<Script*>> childScriptsFut = Engine::Get().GetThreadPool()->batch(actor->GetAllChildren(), FindScripts);
			co_await exathread::yieldUntilComplete(childScriptsFut);

			//Merge lists and return
			std::vector<std::vector<Script*>> toMerge = childScriptsFut.results();
			toMerge.push_back(std::move(scripts));
			auto joined = std::views::join(toMerge) | std::views::common;
			co_return std::vector<Script*>(joined.begin(), joined.end());
		} else {
			co_return scripts;
		}
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