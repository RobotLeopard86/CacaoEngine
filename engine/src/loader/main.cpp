#include "Cacao/Engine.hpp"
#include "Cacao/Identity.hpp"
#include "include/cacaoloader.hpp"

#include "CLI11.hpp"

#include <filesystem>

int main(int argc, char* argv[]) {
	//Identify game client
	const Cacao::ClientIdentity client = Cacao::Loader::SelfIdentify();

	//Configure CLI
	CLI::App app(client.displayName, std::filesystem::path(argv[0]).filename().string());
	Cacao::Engine::InitConfig icfg = {};
	icfg.standalone = true;
	icfg.initialRequestedBackend = "vulkan";
	icfg.clientID = client;

	//Backend option
	app.add_option("--backend,-B", icfg.initialRequestedBackend, "The preferred backend to use. One of ['opengl', 'vulkan'];")->default_val("vulkan")->check([](const std::string& v) {
		const std::array<std::string, 2> okBackends = {"opengl", "vulkan"};
		return (std::find(okBackends.cbegin(), okBackends.cend(), v) != okBackends.cend() ? "" : "The provided value is not valid!");
	});

#ifdef __linux__
	//X11 option
	app.add_flag_callback("--x11-only,-x", [&icfg]() { icfg.preferredWindowProvider = "x11"; }, "Force the usage of X11 for windowing.")->default_val(false);
#endif

	//Logging options
	app.add_flag_callback("--quiet,-q", [&icfg]() { icfg.suppressConsoleLogging = true; }, "Suppress all console logging output.")->default_val(false);
	app.add_flag_callback("--no-file-log,-N", [&icfg]() { icfg.suppressFileLogging = true; }, "Suppress creation and use of logfile.")->default_val(false);

	//Parse
	CLI11_PARSE(app, argc, argv);

	//Engine initialization
	Cacao::Engine::Get().CoreInit(icfg);
	Cacao::Engine::Get().GfxInit();

	//Run
	Cacao::Engine::Get().Run();

	//Engine shutdown
	Cacao::Engine::Get().GfxShutdown();
	Cacao::Engine::Get().CoreShutdown();
}

//Windows
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