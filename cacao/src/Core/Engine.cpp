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

		//Load the game module
		Logging::EngineLog("Loading game module...");
		EngineAssert(std::filesystem::exists("launchconfig.cacao.yml"), "No launch config file exists!");
		YAML::Node launchRoot = YAML::LoadFile("launchconfig.cacao.yml");
		EngineAssert(launchRoot.IsMap(), "Launch config is not a map!");
		EngineAssert(launchRoot["launch"].IsScalar(), "Launch config does not contain the \"launch\" parameter!");
		EngineAssert(std::filesystem::exists(launchRoot["launch"].Scalar() + "/launch." + dynalo::native::name::extension()), "Specified launch target does not contain a launch module!");
		dynalo::library lib(launchRoot["launch"].Scalar() + "/launch." + dynalo::native::name::extension());

		//Open the window
		Window::GetInstance()->Open("Cacao Engine", 1280, 720);

		//Start the thread pool (subtract two threads for the dedicated dynamic tick and render controllers)
		Logging::EngineLog("Setting up thread pool graphics contexts...");
		for(size_t i = 0; i < std::thread::hardware_concurrency() - 2; i++){
			loanedContexts.insert_or_assign(i, _CreateGraphicsContext());
		}
		Logging::EngineLog("Starting thread pool...");
		threadPool.reset(std::thread::hardware_concurrency() - 2, [](){
			//Get and set up a graphics context
			NativeData* graphicsContext = Engine::GetInstance()->RequestGraphicsContext(BS::this_thread::get_index().value());
			Engine::GetInstance()->SetupGraphicsContext(graphicsContext);
		});

		//Set up common skybox resources
		threadPool.submit_task([]() {
			Skybox::CommonSetup();
		}).wait();

		//Launch game module
		Logging::EngineLog("Running game module startup hook...");
		auto launchFunc = lib.get_function<void(void)>("_CacaoLaunch");
		launchFunc();

		//Start the dynamic tick and rendering controller
		Logging::EngineLog("Starting controllers...");
		DynTickController::GetInstance()->Start();
		RenderController::GetInstance()->Start();

		Logging::EngineLog("Engine startup complete!");

		//Engine run loop (for now this mostly does nothing)
		while(run){
			//Update window
			Window::GetInstance()->Update();
		}

		Logging::EngineLog("Shutting down engine...");

		//Stop the dynamic tick and rendering controllers
		Logging::EngineLog("Stopping controllers...");
		DynTickController::GetInstance()->Stop();
		RenderController::GetInstance()->Stop();

		//Cleanup common skybox resources
		threadPool.submit_task([]() {
			Skybox::CommonCleanup();
		}).wait();

		//"Stop" thread pool (really we just pause it so no new tasks can come in, actual thread pool destruction happens at program exit)
		//This is necessary so that we can clean up any thread pool resources
		Logging::EngineLog("Stopping thread pool...");
		threadPool.pause();
		Logging::EngineLog("Destroying thread pool graphics contexts...");
		for(auto it : loanedContexts){
			_DeleteGraphicsContext(it.second);
			delete it.second;
		}
		loanedContexts.clear();

		//Let game module close
		//We don't explictly close the library because the destructor does that for us
		//Doing it now would actually cause a runtime error
		Logging::EngineLog("Running game module shutdown hook...");
		auto gameStopFunc = lib.get_function<void(void)>("_CacaoExiting");
		gameStopFunc();

		//Close window
		Window::GetInstance()->Close();

		//Shutdown event manager
		Logging::EngineLog("Shutting down event manager...");
		EventManager::GetInstance()->Shutdown();
	}

	void Engine::Stop() {
		run.store(false);
	}

	NativeData* Engine::RequestGraphicsContext(size_t poolID){
		EngineAssert(loanedContexts.contains(poolID), "No graphics context loaned to specified pool ID! You have been yelled at in the console, as you were warned.");
		return loanedContexts[poolID];
	}

}