#include "Core/Engine.hpp"
#include "Core/Log.hpp"
#include "Core/Exception.hpp"
#include "Events/EventSystem.hpp"
#include "Graphics/Window.hpp"
#include "Core/DynTickController.hpp"
#include "Audio/AudioSystem.hpp"
#include "Audio/AudioPlayer.hpp"
#include "Graphics/Rendering/RenderController.hpp"
#include "UI/FreetypeOwner.hpp"
#include "Core/RuntimeHooks.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace Cacao {
	//Required static variable initialization
	Engine* Engine::instance = nullptr;
	bool Engine::instanceExists = false;

	//Singleton accessor
	Engine* Engine::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new Engine();
			instanceExists = true;
		}

		return instance;
	}

	void Engine::CoreStartup() {
		//Initialize FreeType
		FreetypeOwner::GetInstance()->Init();

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

		//Create global UI view
		uiView.reset(new UIView());
		uiView->SetSize(Window::GetInstance()->GetSize());

		//Initialize audio system
		Logging::EngineLog("Initializing audio system...");
		AudioSystem::GetInstance()->Init();

		//Create a short-lived dummy audio player (for whatever reason this is required to get normal players working)
		{
			AudioPlayer ap;
		}

		//Call runtime startup hook
		Logging::EngineLog("Starting runtime...");
		RTStartup();

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

		//Register core exception codes
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
		Exception::RegisterExceptionCode(13, "BadValue");
		Exception::RegisterExceptionCode(14, "BadThread");

		//Start the thread pool (subtract one thread for the dedicated dynamic tick controller)
		Logging::EngineLog("Starting thread pool...");
		threadPool.reset(new thread_pool(std::thread::hardware_concurrency() - 1));

		//Initialize windowing system
		Logging::EngineLog("Intializing windowing system...");
		Window::GetInstance()->SysInit();

		if(backendInWindowScope) {
			//Open the window
			Window::GetInstance()->Open("Cacao Engine", {1280, 720}, false, WindowMode::Window);

			//Initialize rendering backend
			Logging::EngineLog("Initializing rendering backend...");
			RenderController::GetInstance()->Init();
		} else {
			//Initialize rendering backend
			Logging::EngineLog("Initializing rendering backend...");
			RenderController::GetInstance()->Init();

			//Open the window
			Window::GetInstance()->Open("Cacao Engine", {1280, 720}, false, WindowMode::Window);
		}

		//Asynchronously run core startup
		threadPool->enqueue_detach([this]() {
			this->CoreStartup();
		});

		//Run the rendering controller on the engine thread
		RenderController::GetInstance()->Run();

		//Run the shutdown functions
		CoreShutdown();
	}

	void Engine::Stop() {
		run.store(false);
	}

	void Engine::CoreShutdown() {
		Logging::EngineLog("Shutting down engine...");
		shuttingDown.store(true);

		//Clear the render queue
		RenderController::GetInstance()->ClearRenderQueue();

		//Stop the dynamic tick controller
		Logging::EngineLog("Stopping dynamic tick controller...");
		DynTickController::GetInstance()->Stop();

		//Clean up common skybox resources
		Skybox::CommonCleanup();

		//Call runtime shutdown hook
		Logging::EngineLog("Shutting down runtime...");
		RTShutdown();

		//Shut down the audio system
		Logging::EngineLog("Shutting down audio system...");
		AudioSystem::GetInstance()->Shutdown();

		//Destroy global UI view
		uiView.reset();

		if(backendInWindowScope) {
			//Shut down rendering backend
			Logging::EngineLog("Shutting down rendering backend...");
			RenderController::GetInstance()->Shutdown();

			//Close the window
			Window::GetInstance()->Close();
		} else {
			//Close the window
			Window::GetInstance()->Close();

			//Shutdown rendering backend
			Logging::EngineLog("Shutting down rendering backend...");
			RenderController::GetInstance()->Shutdown();
		}

		//Shut down windowing system
		Logging::EngineLog("Shutting down windowing system...");
		Window::GetInstance()->SysTerminate();

		//Stop thread pool
		Logging::EngineLog("Stopping thread pool...");
		threadPool.reset();

		//Shutdown the FreeType library
		delete FreetypeOwner::GetInstance();

		//Shutdown event manager
		Logging::EngineLog("Shutting down event manager...");
		EventManager::GetInstance()->Shutdown();
	}

	std::shared_future<void> Engine::RunOnMainThread(std::function<void()> func) {
		Task task(func);
		std::shared_future<void> ret = task.status->get_future().share();
		{
			std::lock_guard lk(mainThreadTaskMutex);
			mainThreadTasks.push(task);
		}
		return ret;
	}

}