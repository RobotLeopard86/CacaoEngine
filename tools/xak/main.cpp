#include "CLI11.hpp"

#include <filesystem>
#include <string>

#include "commands.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

#ifndef CACAO_RELEASE_NICKNAME
#define CACAO_RELEASE_NICKNAME "Name TBD"
#endif

#ifndef XAK_VER
#define XAK_VER "unknown"
#endif

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine Asset Pack Tool", std::filesystem::path(argv[0]).filename().string());
	app.fallthrough();

	//Output control
	outputLvl = OutputLevel::Normal;
	app.add_flag_callback("-q,--quiet", []() { outputLvl = OutputLevel::Silent; }, "Suppress all output");
	app.add_flag_callback("-V,--verbose", []() { outputLvl = OutputLevel::Verbose; }, "Enable verbose output");

	//Version arg
	app.set_version_flag("-v,--version", []() {
        std::stringstream ss;
        ss << "Asset Pack Utility v" << XAK_VER << "\nFor Cacao Engine v" << CACAO_VER << " (" << CACAO_RELEASE_NICKNAME << ")";
        return ss.str(); }, "Show version info and exit");

	//Configure commands
	CreateCmd c(app);
	ListCmd l(app);
	ExtractCmd e(app);
	MergeCmd m(app);
	DelCmd d(app);
	app.require_subcommand(1);

	//Parse the CLI (this will trigger execution of the commands)
	CLI11_PARSE(app, argc, argv);
}