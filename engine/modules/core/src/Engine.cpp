#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/ThreadPool.hpp"
#include "Cacao/Exceptions.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

namespace Cacao {
	bool Engine::CoreInit(const Engine::InitConfig& initCfg) {
		Check<BadStateException>(state == State::Dead, "Engine must be in dead state to run core initialization!");

		//Store config
		cfg = initCfg;

		//Say hello (this will also trigger logging initialization)
		Logger::Engine(Logger::Level::Info) << "Welcome to Cacao Engine v" << CACAO_VER << "!";

		//Start thread pool
		Logger::Engine(Logger::Level::Info) << "Starting thread pool...";
		ThreadPool::Get().Start();

		/* ------------------------------------- *\
		|*      PLACEHOLDER: BUNDLE LOADING      *|
		\* ------------------------------------- */

		/* ---------------------------------- *\
		|*      PLACEHOLDER: AUDIO SETUP      *|
		\* ---------------------------------- */

		/* ------------------------------------ *\
		|*      PLACEHOLDER: FREETYPE INIT      *|
		\* ------------------------------------ */

		state.store(State::Alive);
		return true;
	}

	bool Engine::GfxInit() {
		Check<BadStateException>(state == State::Alive, "Engine must be in alive state to run graphics initialization!");
		state.store(State::Stopped);
		return true;
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

		//Stop thread pool
		Logger::Engine(Logger::Level::Info) << "Stopping thread pool...";
		ThreadPool::Get().Stop();

		//Final goodbye message
		state.store(State::Dead);
		Logger::Engine(Logger::Level::Info) << "Engine shutdown complete.";
	}
}