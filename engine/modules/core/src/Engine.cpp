#include "Cacao/Engine.hpp"
#include "Cacao/Log.hpp"

namespace Cacao {
	bool Engine::CoreInit(const Engine::InitConfig& initCfg) {
		cfg = initCfg;
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