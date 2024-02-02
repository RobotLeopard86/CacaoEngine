#include "Core/Engine.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"
#include "Graphics/Window.hpp"

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
			DataEvent<double>& de = static_cast<DataEvent<double>&>(e);
			OnDynamicTick(de.GetData());
			return;
		}));
		EventManager::GetInstance()->SubscribeConsumer("FixedTick", new EventConsumer([](Event& e) {
			OnFixedTick();
			return;
		}));
		EventManager::GetInstance()->SubscribeConsumer("WindowClose", new EventConsumer([this](Event& e) {
			this->Stop();
			return;
		}));

		//Start the thread pool
		Logging::EngineLog("Starting thread pool...");
		threadPool.reset(std::thread::hardware_concurrency() - 2); //Minus 2 because of (eventual) event thread and render thread

		//Open the window
		Window::GetInstance()->Open(GetWindowTitle(), 1280, 720);

		//Run client startup hook
		Logging::EngineLog("Running client startup hook...");
		OnStartup();

		Logging::EngineLog("Engine startup complete!");

		lastFrame = std::chrono::steady_clock::now();

		//Engine run loop
		while(run){
			//Calculate time step
			double timestep = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastFrame).count();
			lastFrame = std::chrono::steady_clock::now();

			//Dispatch dynamic tick event
			DataEvent<double> dte = DataEvent<double>{ "DynamicTick" , timestep };
			EventManager::GetInstance()->Dispatch(dte);

			//Update window
			Window::GetInstance()->Update();
		}

		Logging::EngineLog("Shutting down engine...");

		//Run client shutdown hook
		Logging::EngineLog("Running client shutdown hook...");
		OnShutdown();

		//Close window
		Window::GetInstance()->Close();

		//Shutdown event manager
		EventManager::GetInstance()->Shutdown();
	}

	void Engine::Stop() {
		run.store(false);
	}

}