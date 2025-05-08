#include "Cacao/Engine.hpp"

int main() {
	Cacao::Engine::InitConfig icfg = {};
	icfg.standalone = true;
	icfg.initialRequestedBackend = "opengl";
	Cacao::Engine::Get().CoreInit(icfg);
	Cacao::Engine::Get().GfxInit();
	Cacao::Engine::Get().Run();
	Cacao::Engine::Get().GfxShutdown();
	Cacao::Engine::Get().CoreShutdown();
}