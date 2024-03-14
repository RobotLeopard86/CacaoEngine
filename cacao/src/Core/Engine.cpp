#include "Core/Engine.hpp"
#include "Core/Log.hpp"
#include "Events/EventSystem.hpp"
#include "Graphics/Window.hpp"
#include "Control/DynTickController.hpp"
#include "Graphics/Rendering/RenderController.hpp"

#include "yaml-cpp/yaml.h"
#include "dynalo/dynalo.hpp"

#include <filesystem>

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

		//Set some default engine config values
		cfg.fixedTickRate = 50;
		cfg.targetDynTPS = 60;

		//Register the window close consumer
		Logging::EngineLog("Setting up event manager...");
		EventManager::GetInstance()->SubscribeConsumer("WindowClose", new EventConsumer([this](Event& e) {
			this->Stop();
			return;
		}));

		//Start the thread pool
		Logging::EngineLog("Starting thread pool...");
		threadPool.reset(std::thread::hardware_concurrency() - 2); //Subtract two for the dynamic and fixed tick controllers

		//Load the game module
		Asserts::EngineAssert(std::filesystem::exists("launchconfig.cacao.yml"), "No launch config file exists!");
		YAML::Node launchRoot = YAML::LoadFile("launchconfig.cacao.yml");
		Asserts::EngineAssert(launchRoot.IsMap(), "Launch config is not a map!");
		Asserts::EngineAssert(launchRoot["launch"].IsScalar(), "Launch config does not contain the \"launch\" parameter!");
		Asserts::EngineAssert(std::filesystem::exists(launchRoot["launch"].Scalar() + "/launch." + dynalo::native::name::extension()), "Specified launch target does not contain a launch module!");
		dynalo::library lib(launchRoot["launch"].Scalar() + "/launch." + dynalo::native::name::extension());

		//Open the window
		Window::GetInstance()->Open("Cacao Engine", 1280, 720);

		//Launch game module
		auto launchFunc = lib.get_function<void(void)>("_CacaoLaunch");
		launchFunc();

		//Start the dynamic tick and rendering controller
		DynTickController::GetInstance()->Start();
		RenderController::GetInstance()->Start();

		Logging::EngineLog("Engine startup complete!");

		lastFrame = std::chrono::steady_clock::now();

		//Engine run loop
		while(run){
			//Make sure we're only ticking every 50 ms
			double timestep = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastFrame).count();
			if(timestep < 50) continue;
			lastFrame = std::chrono::steady_clock::now();

			//Update window
			Window::GetInstance()->Update();
		}

		Logging::EngineLog("Shutting down engine...");

		//Stop the dynamic tick controller
		DynTickController::GetInstance()->Stop();
		RenderController::GetInstance()->Stop();

		//Close window
		Window::GetInstance()->Close();

		//Let game module close
		//We dont' explictly close the library because the destructor does that for us
		//Doing it now would actually cause a runtime error
		auto gameStopFunc = lib.get_function<void(void)>("_CacaoExiting");
		gameStopFunc();

		//Shutdown event manager
		EventManager::GetInstance()->Shutdown();
	}

	void Engine::Stop() {
		run.store(false);
	}

}