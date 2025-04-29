#include "CLI11.hpp"

#include <filesystem>
#include <string>

#include "commands.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif
#ifndef TOOL_VER
#define TOOL_VER "unknown"
#endif

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine Cubemap Tool", std::filesystem::path(argv[0]).filename().string());
	app.fallthrough();

	//Version arg
	app.set_version_flag("-v,--version", []() {
        std::stringstream ss;
        ss << "CubeTool v" << TOOL_VER << "\nFor Cacao Engine v" << CACAO_VER;
        return ss.str(); }, "Show version info and exit");

	//Output control
	outputLvl = OutputLevel::Silent;
	app.add_flag_callback("-V,--verbose", []() { outputLvl = OutputLevel::Verbose; }, "Enable verbose output from the tool");

	//Configure commands
	CreateCmd c(app);
	ExtractCmd e(app);
	app.require_subcommand(1);

	//Parse the CLI (this will trigger execution of the commands)
	CLI11_PARSE(app, argc, argv);

	return 0;
}