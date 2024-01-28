#include "Core/Log.hpp"
#include "Core/Engine.hpp"

int main(int argc, char** argv){
	//Initialize logging
	Citrus::Logging::Init();

	Citrus::Logging::EngineLog("Welcome to Citrus Engine!");

	//Start the engine (this call will yield until engine stops)
	Citrus::Engine::GetInstance()->Run();

	Citrus::Logging::EngineLog("Shutdown complete!");

	return 0;
}