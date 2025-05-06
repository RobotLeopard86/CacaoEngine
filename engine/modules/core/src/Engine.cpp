#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/ThreadPool.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/AudioManager.hpp"
#include "Freetype.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

namespace Cacao {
	void Engine::CoreInit(const Engine::InitConfig& initCfg) {
		Check<BadStateException>(state == State::Dead, "Engine must be in dead state to run core initialization!");

		//Store config
		cfg = initCfg;

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

		state.store(State::Alive);

		//We are here
		Logger::Engine(Logger::Level::Info) << "Reached target Core Initialization.";
	}

	void Engine::GfxInit() {
		Check<BadStateException>(state == State::Alive, "Engine must be in alive state to run graphics initialization!");
		state.store(State::Stopped);
	}

	void Engine::Run() {
		Check<BadStateException>(state == State::Stopped, "Engine must be in stopped state to start!");
		state.store(State::Running);
	}

	void Engine::Quit() {
		Check<BadStateException>(state == State::Running, "Engine must be in running state to quit!");
		state.store(State::Stopped);
	}

	void Engine::GfxShutdown() {
		Check<BadStateException>(state == State::Stopped, "Engine must be in stopped state to run graphics shutdown!");
		state.store(State::Alive);
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
		state.store(State::Dead);
		Logger::Engine(Logger::Level::Info) << "Engine shutdown complete.";
	}
}