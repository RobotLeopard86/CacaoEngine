#include "Cacao/Engine.hpp"

#include "CLI11.hpp"

#include <filesystem>

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine game runtime", std::filesystem::path(argv[0]).filename().string());
	Cacao::Engine::InitConfig icfg = {};
	icfg.standalone = true;
	icfg.initialRequestedBackend = "vulkan";

	//Backend option
	app.add_option("--backend,-B", icfg.initialRequestedBackend, "The preferred backend to use. One of ['opengl', 'vulkan'];")->default_val("vulkan")->check([](const std::string& v) {
		const std::array<std::string, 2> okBackends = {"opengl", "vulkan"};
		return (std::find(okBackends.cbegin(), okBackends.cend(), v) != okBackends.cend() ? "" : "The provided value is not valid!");
	});

#ifdef __linux__
	//X11 option
	app.add_flag_callback("--x11-only,-x", [&icfg]() { icfg.preferredWindowProvider = "x11"; }, "Force the usage of X11 for windowing.")->default_val(false);
#endif

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