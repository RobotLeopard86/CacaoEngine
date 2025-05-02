#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

namespace Cacao {
	bool Engine::CoreInit(const Engine::InitConfig& initCfg) {
		//Store config
		cfg = initCfg;

		//Say hello (this will also trigger logging initialization)
		Logger::Engine(Logger::Level::Info) << "Welcome to Cacao Engine v" << CACAO_VER << "!";

		return true;
	}

	bool Engine::GfxInit() {
		return true;
	}

	void Engine::Run() {
	}

	void Engine::Quit() {
	}

	void Engine::GfxShutdown() {
	}

	void Engine::CoreShutdown() {
	}
}