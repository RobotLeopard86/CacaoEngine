#include "Core/Engine.hpp"
#include "Core/Log.hpp"
#include "Core/Exception.hpp"
#include "Events/EventSystem.hpp"
#include "Graphics/Window.hpp"
#include "Core/DynTickController.hpp"
#include "Audio/AudioController.hpp"
#include "Audio/AudioPlayer.hpp"
#include "Graphics/Rendering/RenderController.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace Cacao {
	//Required static variable initialization
	Engine* Engine::instance = nullptr;
	bool Engine::instanceExists = false;

	//Singleton accessor
	Engine* Engine::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new Engine();
			instanceExists = true;
		}

		return instance;
	}

	void Engine::CoreStartup() {
		//Register some basic exception codes
		Exception::RegisterExceptionCode(0, "External");
		Exception::RegisterExceptionCode(1, "FileNotFound");
		Exception::RegisterExceptionCode(2, "NonexistentValue");
		Exception::RegisterExceptionCode(3, "InvalidYAML");
		Exception::RegisterExceptionCode(4, "BadInitState");
		Exception::RegisterExceptionCode(5, "NullValue");
		Exception::RegisterExceptionCode(6, "BadState");
		Exception::RegisterExceptionCode(7, "FileOpenFailure");
		Exception::RegisterExceptionCode(8, "EventManager");
		Exception::RegisterExceptionCode(9, "ContainerValue");
		Exception::RegisterExceptionCode(10, "WrongType");
		Exception::RegisterExceptionCode(11, "IO");
		Exception::RegisterExceptionCode(12, "BadCompileState");

		//Load the launch configuration
		Logging::EngineLog("Loading launch config...");
		CheckException(std::filesystem::exists("launchconfig.cacao.yml"), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "No launch config file exists!")
		YAML::Node launchRoot = YAML::LoadFile("launchconfig.cacao.yml");
		CheckException(launchRoot.IsMap(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "Launch config is not a map!")
		CheckException(launchRoot["launch"].IsScalar(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "Launch config does not contain the \"launch\" parameter or it isn't a scalar!")
		CheckException(std::filesystem::exists(launchRoot["launch"].Scalar() + "/launch." + dynalo::native::name::extension()), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "No launch module found at specified launch path!")

		//Load game module and module config
		gameLib = new dynalo::library(launchRoot["launch"].Scalar() + "/launch." + dynalo::native::name::extension());
		cfg.fixedTickRate = (launchRoot["fixedTickRate"].IsScalar() ? std::stoi(launchRoot["fixedTickRate"].Scalar()) : cfg.fixedTickRate);
		cfg.targetDynTPS = (launchRoot["dynamicTPS"].IsScalar() ? std::stoi(launchRoot["dynamicTPS"].Scalar()) : cfg.targetDynTPS);
		cfg.maxFrameLag = (launchRoot["maxFrameLag"].IsScalar() ? std::stoi(launchRoot["maxFrameLag"].Scalar()) : cfg.maxFrameLag);
		if(launchRoot["title"].IsScalar()) Window::GetInstance()->SetTitle(launchRoot["title"].Scalar());
		if(launchRoot["dimensions"].IsMap() && launchRoot["dimensions"]["x"].IsScalar() && launchRoot["dimensions"]["y"].IsScalar()) {
			Window::GetInstance()->SetSize({std::stoi(launchRoot["dimensions"]["x"].Scalar()), std::stoi(launchRoot["dimensions"]["y"].Scalar())});
		}
		if(launchRoot["workingDir"].IsScalar() && std::filesystem::exists(launchRoot["workingDir"].Scalar())) std::filesystem::current_path(launchRoot["workingDir"].Scalar());

		//Register the window close consumer
		Logging::EngineLog("Setting up event manager...");
		EventManager::GetInstance()->SubscribeConsumer("WindowClose", new EventConsumer([this](Event& e) {
			this->Stop();
			return;
		}));

		//Set up common skybox resources
		std::future<void> skySetup = threadPool->enqueue([]() {
			Skybox::CommonSetup();
		});
		skySetup.wait();

		//Start audio controller
		Logging::EngineLog("Starting audio controller...");
		AudioController::GetInstance()->Start();

		//Since the audio controller is super important, we wait until it comes online
		while(!AudioController::GetInstance()->IsAudioSystemInitialized()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		//Create a short-lived dummy audio player (somehow this is required to get normal players workimng)
		{
			AudioPlayer ap;
		}

		//Launch game module
		Logging::EngineLog("Running game module startup hook...");
		auto launchFunc = gameLib->get_function<void(void)>("_CacaoLaunch");
		launchFunc();

		//Start dynamic tick controller
		Logging::EngineLog("Starting dynamic tick controller...");
		DynTickController::GetInstance()->Start();

		//Make the window visible
		Window::GetInstance()->SetWindowVisibility(true);

		Logging::EngineLog("Engine startup complete!");
	}

	void Engine::Run() {
		//Make sure the engine will run
		run.store(true);
		shuttingDown.store(false);

		//Set thread ID
		threadID = std::this_thread::get_id();

		//Set some default engine config values
		cfg.fixedTickRate = 50;
		cfg.targetDynTPS = 60;
		cfg.maxFrameLag = 10;

		//Open the window
		Window::GetInstance()->Open("Cacao Engine", {1280, 720}, false, WindowMode::Window);

		//Initialize rendering backend
		Logging::EngineLog("Initializing rendering backend...");
		RenderController::GetInstance()->Init();

		//Start the thread pool (subtract two threads for the dedicated dynamic tick and audio controllers)
		Logging::EngineLog("Starting thread pool...");
		threadPool.reset(new thread_pool(std::thread::hardware_concurrency() - 2));

		//Asynchronously run core startup
		//We never use this future as we don't intend to wait on it, but we have to do this because [[nodiscard]]
		std::future<void> startup = threadPool->enqueue([this]() {
			this->CoreStartup();
		});

		//Run the rendering controller on the main thread
		RenderController::GetInstance()->Run();

		//Shut down rendering backend
		Logging::EngineLog("Shutting down rendering backend...");
		RenderController::GetInstance()->Shutdown();

		//Close window
		Window::GetInstance()->Close();

		//Shutdown event manager
		Logging::EngineLog("Shutting down event manager...");
		EventManager::GetInstance()->Shutdown();
	}

	void Engine::Stop() {
		Logging::EngineLog("Shutting down engine...");
		shuttingDown.store(true);

		//Clear the render queue
		RenderController::GetInstance()->ClearRenderQueue();

		//Stop the dynamic tick and audio controllers
		Logging::EngineLog("Stopping controllers...");
		DynTickController::GetInstance()->Stop();
		AudioController::GetInstance()->Stop();

		//Call game module exit hook
		auto exitFunc = gameLib->get_function<void(void)>("_CacaoExiting");
		exitFunc();

		//Stop thread pool
		Logging::EngineLog("Stopping thread pool...");
		threadPool.reset();

		run.store(false);
	}

}