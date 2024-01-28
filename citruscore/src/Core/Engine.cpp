#include "Core/Engine.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"

namespace Citrus {
	//Required static variable initialization
	Engine* Engine::instance = nullptr;
	bool Engine::instanceExists = false;

	//Singleton accessor
	Engine* Engine::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new Engine();
			instanceExists = true;
		}

		return instance;
	}

	void Engine::Run(){
		//Make sure the engine will run
		run.store(true);

		//Register our dynamic and fixed tick consumers
		Logging::EngineLog("Setting up event manager...");
		EventManager::GetInstance()->SubscribeConsumer("DynamicTick", new EventConsumer([](Event& e) {
			DataEvent<float>& de = static_cast<DataEvent<float>&>(e);
			OnDynamicTick(de.GetData());
			return;
		}));
		EventManager::GetInstance()->SubscribeConsumer("FixedTick", new EventConsumer([](Event& e) {
			OnFixedTick();
			return;
		}));

		//Run client startup hook
		Logging::EngineLog("Running client startup hook...");
		OnStartup();

		Logging::EngineLog("Engine startup complete!");
		//Engine run loop
		while(run){
			DataEvent<float> dte = DataEvent<float>{ "DynamicTick" , 0.0f };
			EventManager::GetInstance()->Dispatch(dte);
		}

		Logging::EngineLog("Shutting down engine...");

		//Run client shutdown hook
		Logging::EngineLog("Running client shutdown hook...");
		OnShutdown();

		//Shutdown event manager
		EventManager::GetInstance()->Shutdown();
	}

	void Engine::Stop() {
		run.store(false);
	}

}