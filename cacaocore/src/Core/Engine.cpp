#include "Core/Engine.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"
#include "Graphics/Window.hpp"
#include "Pipeline/LogicPhase.hpp"
#include "Pipeline/ProcessPhase.hpp"
#include "Pipeline/RenderPhase.hpp"

#include <future>

namespace Cacao {
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
		EventManager::GetInstance()->SubscribeConsumer("WindowClose", new EventConsumer([this](Event& e) {
			this->Stop();
			return;
		}));

		//Start the thread pool
		Logging::EngineLog("Starting thread pool...");
		threadPool.reset(std::thread::hardware_concurrency() - 3); //Minus 3 because of frame pipeline threads

		//Open the window
		Window::GetInstance()->Open("Cacao", 1280, 720);

		Logging::EngineLog("Engine startup complete!");

		lastFrame = std::chrono::steady_clock::now();

		//Engine run loop
		while(run){
			//Make sure we're only ticking every 50 ms
			double timestep = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastFrame).count();
			if(timestep < 50) continue;
			lastFrame = std::chrono::steady_clock::now();

			//Run frame pipeline
			std::future<void> logic = LogicPhase::GetInstance()->Run();
			std::future<void> process = ProcessPhase::GetInstance()->Run();
			std::future<void> render = RenderPhase::GetInstance()->Run();

			//Wait for all stages to finish
			logic.wait();
			process.wait();
			render.wait();

			//Update window
			Window::GetInstance()->Update();
		}

		Logging::EngineLog("Shutting down engine...");

		//Shutdown render phase system
		Logging::EngineLog("Shutting down render system...");
		RenderPhase::GetInstance()->Shutdown();

		//Close window
		Window::GetInstance()->Close();

		//Shutdown event manager
		EventManager::GetInstance()->Shutdown();
	}

	void Engine::Stop() {
		run.store(false);
	}

}