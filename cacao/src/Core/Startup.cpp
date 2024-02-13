#include "Core/Log.hpp"
#include "Core/Engine.hpp"

int main(int argc, char** argv){
	//Initialize logging
	Cacao::Logging::Init();

	Cacao::Logging::EngineLog("Welcome to Cacao Engine!");

	//Start the engine (this call will yield until engine stops)
	Cacao::Engine::GetInstance()->Run();

	Cacao::Logging::EngineLog("Shutdown complete!");

	return 0;
}