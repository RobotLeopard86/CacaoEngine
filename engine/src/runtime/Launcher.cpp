#include "CLI11.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Identity.hpp"
#include "Cacao/Log.hpp"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"

#include <bits/c++config.h>
#include <exception>
#include <filesystem>
#include <string>

#include "Runtime.hpp"

#include "astra/yaml.hpp"
#include "cacaort.astra.hpp"// IWYU pragma: keep

void panic(const std::string& err, const std::string& hint) {
	std::cerr << "\x1b[1;91mERROR: \x1b[0m" << err << "!\n"
			  << (!hint.empty() ? std::string("\x1b[1;96mHint: \x1b[0m") + hint + ".\n" : "")
			  << "\x1b[3mIf you are an end user seeing this error, please report this to the developer of the application.\x1b[0m" << std::endl;
	exit(-1);
}

int main(int argc, char* argv[]) {
	//Change directory to executable location
	if(argc < 1) {
		//Panic if we don't have argv[0]
		panic("Program argument list does not provide program path", "This is usually caused by incorrectly specifying arguments in CreateProcess on Windows or one of the exec functions on POSIX-like systems");
	}
	if(!std::filesystem::exists(argv[0])) {
		//Panic if argv[0] is bad
		panic("Program path argument is an invalid path", "This is usually caused by incorrectly specifying arguments in CreateProcess on Windows or one of the exec functions on POSIX-like systems");
	}
	std::filesystem::path exePath = std::filesystem::canonical(std::filesystem::absolute(std::filesystem::path(argv[0])));
	std::filesystem::path cdTo = exePath.parent_path();
#ifdef __APPLE__
	bool inAppBundle = false;
	for(; cdTo != std::filesystem::path("/"); cdTo = cdTo.parent_path()) {
		if(cdTo.extension().string().compare(".app") == 0) {
			inAppBundle = true;
			break;
		}
	}
	if(!inAppBundle)
		cdTo = exePath.parent_path();
	else
		(cdTo /= "Contents") /= "Resources";
#endif
	std::filesystem::current_path(cdTo);

	//Validate that required spec file exists
	if(!std::filesystem::exists("cacaospec.yml")) {
		panic("Cacaospec file not found",
			std::string("This usually means the game bundle is not correctly set up. See the documentation at https://robotleopard86.github.io/CacaoEngine/") + CACAO_VER + "/manual/bundles.html for details");
	}

	//Read and parse the spec file
	try {
		YAML::Node specRoot = YAML::LoadFile("cacaospec.yml");
		rt.cacaospec = astra::yaml::fromNode<Runtime::Cacaospec>(specRoot);
	} catch(const std::exception& e) {
		panic(std::string("Cacaospec file is invalid: ") + e.what(), std::string("This usually means the game bundle is not correctly set up. See the documentation at https://robotleopard86.github.io/CacaoEngine/") + CACAO_VER + "/manual/bundles.html for details");
	}
#ifdef __APPLE__
	if(inAppBundle) rt.cacaospec.binary = "../Frameworks/" + rt.cacaospec.binary;
#endif
	if(!std::filesystem::exists(rt.cacaospec.binary)) panic("Game binary does not exist",
		std::string("This usually means the game bundle is not correctly set up. See the documentation at https://robotleopard86.github.io/CacaoEngine/") + CACAO_VER + "/manual/bundles.html for details");
	rt.cacaospec.binary = std::filesystem::absolute(rt.cacaospec.binary).string();

	//Configure CLI
	CLI::App app(rt.cacaospec.meta.title, std::filesystem::path(argv[0]).filename().string());
	rt.icfg.startFrameProcessorWithGfxSystem = false;
	rt.icfg.initialRequestedBackend = "vulkan";
	rt.icfg.clientID = ClientIdentity {.id = rt.cacaospec.meta.pkgId, .displayName = rt.cacaospec.meta.title};

	//Backend option
	app.add_option("--backend,-B", rt.icfg.initialRequestedBackend, "The preferred backend to use. One of ['opengl', 'vulkan'];")->default_val("vulkan")->check([](const std::string& v) {
		const std::array<std::string, 2> okBackends = {"opengl", "vulkan"};
		return (std::find(okBackends.cbegin(), okBackends.cend(), v) != okBackends.cend() ? "" : "The provided value is not valid!");
	});

#ifdef __linux__
	//X11 option
	app.add_flag_callback("--x11-only,-x", []() { rt.icfg.preferredWindowProvider = "x11"; }, "Force the usage of X11 for windowing.")->default_val(false);
#endif

	//Logging options
	app.add_flag_callback("--quiet,-q", []() { rt.icfg.suppressConsoleLogging = true; }, "Suppress all console logging output.")->default_val(false);
	app.add_flag_callback("--no-file-log,-N", []() { rt.icfg.suppressFileLogging = true; }, "Suppress creation and use of logfile.")->default_val(false);

#ifdef __APPLE__
	//macOS chatter
	bool macChatter = false;
	app.add_flag("--no-suppress-syslog,-Y", macChatter, "Disable macOS system framework trace info from being printed to the console when debugging.");
#endif

	//Parse
	CLI11_PARSE(app, argc, argv);

#ifdef __APPLE__
	if(!macChatter) setenv("OS_ACTIVITY_MODE", "disable", 1);
#endif

	//Engine setup
	try {
		rt.SetupEngine();
	} catch(...) {
		Logger::Runtime(Logger::Level::Fatal) << "A fatal error has occurred in engine initialization. Exiting...";
		if(Engine::Get().GetState() == Engine::State::Ready) Engine::Get().GfxShutdown();
		if(Engine::Get().GetState() == Engine::State::Alive) Engine::Get().CoreShutdown();
		Logger::Runtime(Logger::Level::Warn) << "Engine exited with an error code!";
		Logger::Runtime(Logger::Level::Warn) << "If you are an end user seeing this error, please report it to the application developer.";
		exit(-1);
	}

	//Prepare game to run
	//This is where we load initial resources and whatnot
	try {
		rt.LoadGame();
	} catch(...) {
		Logger::Runtime(Logger::Level::Fatal) << "A fatal error has occurred in game initialization. Exiting...";
		rt.DestroyGfxObjects();
		rt.Cleanup();
		Engine::Get().GfxShutdown();
		Engine::Get().CoreShutdown();
		Logger::Runtime(Logger::Level::Warn) << "Engine exited with an error code!";
		Logger::Runtime(Logger::Level::Warn) << "If you are an end user seeing this error, please report it to the application developer.";
		exit(-1);
	}

	//Run (blocks until the game says it's done)
	Engine::Get().Run();

	//Game has stopped now, destroy graphics objects before shutting down that part of the engine
	rt.DestroyGfxObjects();
	Engine::Get().GfxShutdown();

	//Run cleanup tasks (unload game data)
	rt.Cleanup();

	//Stop the engine fully
	Engine::Get().CoreShutdown();
}

//Windows no-console wrapper
#ifdef WIN_NOCONSOLE
#include <Windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>

//This is a trampoline to call the real main() above
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//Get command line
	int argc = 0;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	if(!argvW) {
		std::cerr << "Failed to retrieve command line!" << std::endl;
		return -1;
	}

	//Convert command line to UTF-8
	//We have two vectors because we have to pass a char** to main, but we still need to keep the std::strings so the memory isn't deallocated
	std::vector<std::string> utf8Args;
	std::vector<char*> argv;
	for(int i = 0; i < argc; ++i) {
		//Get length of argument
		int argLength = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
		if(argLength <= 0) continue;

		//Real conversion now
		std::string arg(argLength - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, arg.data(), argLength, nullptr, nullptr);

		//Add to vectors (we use data() and not c_str() because c_str() returns a const pointer, which main() doesn't take)
		argv.push_back(arg.data());
		utf8Args.push_back(std::move(arg));
	}

	//Free argvW
	LocalFree(argvW);

	//Trampoline
	return main(static_cast<int>(argv.size()), argv.data());
}
#endif