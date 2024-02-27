#include "Core/Log.hpp"
#include "Core/Engine.hpp"

#include <iostream>
#include <filesystem>

int main(int argc, char** argv){
	//Change to executable directory
	if(argc < 1) {
		//Panic if we don't have argv[0]
		std::cerr << "CACAO ENGINE STARTUP FAILURE: PROGRAM PATH MUST BE PROVIDED!\n";
		exit(1);
	}
	if(!std::filesystem::exists(argv[0])){
		//Panic if argv[0] is an invalid path
		std::cerr << "CACAO ENGINE STARTUP FAILURE: PROGRAM PATH PROVIDED DOES NOT EXIST!\n";
		exit(1);
	}
	std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path());

	//Initialize logging
	Cacao::Logging::Init();

	Cacao::Logging::EngineLog("Welcome to Cacao Engine!");

	//Start the engine (this call will yield until engine stops)
	Cacao::Engine::GetInstance()->Run();

	Cacao::Logging::EngineLog("Shutdown complete!");

	return 0;
}