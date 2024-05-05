#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Utilities/MiscUtils.hpp"

#include <iostream>
#include <filesystem>
#include <cstdlib>

int main(int argc, char* argv[]) {
	//Change to executable directory
	if(argc < 1) {
		//Panic if we don't have argv[0]
		std::cerr << "CACAO ENGINE STARTUP FAILURE: PROGRAM PATH MUST BE PROVIDED!\n";
		exit(1);
	}
	if(!std::filesystem::exists(argv[0])) {
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

//Windows main
#if defined(WIN32) && defined(CACAO_USE_WINMAIN)

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//Convert lpCmdLine
	int argc;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	if(argvW == nullptr) {
		//Handle error
		return 1;
	}

	//Convert wide char arguments to multibyte
	char** argv = new char*[argc];
	for(int i = 0; i < argc; ++i) {
		int len = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
		argv[i] = new char[len];
		WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], len, nullptr, nullptr);
	}

	//Call standard main
	int result = main(argc, argv);

	//Clean up
	LocalFree(argvW);
	for(int i = 0; i < argc; ++i) {
		delete[] argv[i];
	}
	delete[] argv;

	return result;
}

#endif