#include "CLI11.hpp"
#include "spinners.hpp"

#include <vector>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#include "toolutil.hpp"

#include "libcacaoformats.hpp"

#define PACK_FILE_EXTENSION ".xak"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif
#ifndef XAK_VER
#define XAK_VER "unknown"
#endif

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine World Compiler", std::filesystem::path(argv[0]).filename().string());

	//Output control
	outputLvl = OutputLevel::Normal;
	app.add_flag_callback("-V,--verbose", []() { outputLvl = OutputLevel::Verbose; }, "Enable verbose output from the compiler");

	//Version arg
	app.set_version_flag("-v,--version", []() {
        std::stringstream ss;
        ss << "Asset pack tool v" << XAK_VER << "\nFor Cacao Engine v" << CACAO_VER;
        return ss.str(); }, "Show version info and exit");

	//Parse the CLI
	CLI11_PARSE(app, argc, argv);
}