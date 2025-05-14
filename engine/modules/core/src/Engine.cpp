#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/ThreadPool.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/AudioManager.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Freetype.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

#include <array>
#include <vector>

namespace Cacao {
	Engine::Engine()
	  : state(State::Dead) {}

	Engine::~Engine() {
		if(state == State::Running) Quit();
		if(state == State::Stopped) GfxShutdown();
		if(state == State::Alive) CoreShutdown();
	}

	Engine& Engine::Get() {
		static Engine _instance;
		return _instance;
	}

	void Engine::CoreInit(const Engine::InitConfig& initCfg) {
		Check<BadStateException>(state == State::Dead, "Engine must be in dead state to run core initialization!");

		//Store config
		icfg = initCfg;

		//Say hello (this will also trigger logging initialization)
		Logger::Engine(Logger::Level::Info) << "Welcome to Cacao Engine v" << CACAO_VER << "!";

		//Start thread pool
		Logger::Engine(Logger::Level::Trace) << "Starting thread pool...";
		ThreadPool::Get().Start();

		/* ------------------------------------- *\
		|*      PLACEHOLDER: BUNDLE LOADING      *|
		\* ------------------------------------- */

		//Initialize audio
		Logger::Engine(Logger::Level::Trace) << "Initializing audio system...";
		AudioManager::Get().Initialize();

		//Initialize FreeType
		Logger::Engine(Logger::Level::Trace) << "Initializing FreeType instance...";
		Check<ExternalException>(FT_Init_FreeType(&freeType) == FT_Err_Ok, "Failed to initialize FreeType instance!");

		//Done with stage
		Logger::Engine(Logger::Level::Info) << "Reached target Core Initialization.";

		std::lock_guard lkg(stateMtx);
		state = State::Alive;
	}

	void Engine::GfxInit() {
		Check<BadStateException>(state == State::Alive, "Engine must be in alive state to run graphics initialization!");

		//This will call the Window constructor which will decide whether X or Wayland is to be used
		//We don't care about the window yet, so we just discard it
		{
			Window::Get();
		}

		//In descending order of priority
		std::vector<std::string> backends = {"vulkan", "opengl"};
		if(!icfg.initialRequestedBackend.empty()) {
			auto it = std::find(backends.begin(), backends.end(), icfg.initialRequestedBackend);
			if(it != backends.end()) {
				backends.erase(it);
				backends.insert(backends.begin(), icfg.initialRequestedBackend);
			}
		}

		//Try to intialize backend
		bool found = false;
		std::string chosen;
		for(const std::string& backend : backends) {
			//Set module in PAL
			Logger::Engine(Logger::Level::Trace) << "Trying backend \"" << backend << "\"...";
			PAL::Get().SetModule(backend);
			try {
				if(PAL::Get().InitializeModule()) {
					found = true;
					chosen = backend;
					break;
				}
			} catch(...) {}
		}
		if(!found) {
			Logger::Engine(Logger::Level::Fatal) << "No graphics backends are available!";
			CoreShutdown();
			return;
		}
		Logger::Engine(Logger::Level::Info) << "Selected backend \"" << chosen << "\".";

		//Open window
		Logger::Engine(Logger::Level::Trace) << "Creating window...";
		Window::Get().Open("Cacao Engine", {1280, 720}, true, Window::Mode::Windowed);

		//Enable V-Sync by default
		PAL::Get().SetVSync(true);

		//Done with stage
		Logger::Engine(Logger::Level::Info) << "Reached target Graphics Initialization.";
		std::lock_guard lkg(stateMtx);
		state = State::Stopped;
	}

	void Engine::Run() {
		Check<BadStateException>(state == State::Stopped, "Engine must be in stopped state to start!");
		{
			std::lock_guard lkg(stateMtx);
			state = State::Running;
		}

		Logger::Engine(Logger::Level::Info) << "Performing final initialization tasks...";

		/* ------------------------------------------- *\
		|*      PLACEHOLDER: FINAL INITIALIZATION      *|
		\* ------------------------------------------- */

		Logger::Engine(Logger::Level::Info) << "Reached target Game Launch.";

		while(state == State::Running) {
			//Handle OS events
			Window::Get().HandleOSEvents();
		}
	}

	void Engine::Quit() {
		Check<BadStateException>(state == State::Running, "Engine must be in running state to quit!");

		//Set state
		std::lock_guard lkg(stateMtx);
		state = State::Stopped;
		Logger::Engine(Logger::Level::Info) << "Engine shutdown requested!";

		//Fire shutdown event (this (for now) will block until all consumers have responded)
		Event e("EngineShutdown");
		EventManager::Get().Dispatch(e);
	}

	void Engine::GfxShutdown() {
		Check<BadStateException>(state == State::Stopped, "Engine must be in stopped state to run graphics shutdown!");

		//Close window
		Logger::Engine(Logger::Level::Trace) << "Destroying window...";
		Window::Get().Close();

		//Unload backend
		Logger::Engine(Logger::Level::Trace) << "Terminating graphics backend...";
		PAL::Get().TerminateModule();

		std::lock_guard lkg(stateMtx);
		state = State::Alive;
	}

	void Engine::CoreShutdown() {
		Check<BadStateException>(state == State::Alive, "Engine must be in alive state to run core shutdown!");
		Logger::Engine(Logger::Level::Info) << "Shutting down engine core...";

		//Shutdown FreeType
		Logger::Engine(Logger::Level::Trace) << "Destroying FreeType instance...";
		Check<ExternalException>(FT_Done_FreeType(freeType) == FT_Err_Ok, "Failed to destroy FreeType instance!");

		//Terminate audio
		Logger::Engine(Logger::Level::Trace) << "Terminating audio system...";
		AudioManager::Get().Terminate();

		//Stop thread pool
		Logger::Engine(Logger::Level::Trace) << "Stopping thread pool...";
		ThreadPool::Get().Stop();

		//Final goodbye message
		std::lock_guard lkg(stateMtx);
		state = State::Dead;
		Logger::Engine(Logger::Level::Info) << "Engine shutdown complete.";
	}
}