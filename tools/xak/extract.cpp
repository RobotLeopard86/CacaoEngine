#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"

ExtractCmd::ExtractCmd(CLI::App& app) {
	//Extract the command CLI
	cmd = app.add_subcommand("extract", "Extract assets from a pack");

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
	});
}

void ExtractCmd::Callback() {
}