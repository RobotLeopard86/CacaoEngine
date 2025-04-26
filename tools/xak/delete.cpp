#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"

DelCmd::DelCmd(CLI::App& app) {
	//Delete the command CLI
	cmd = app.add_subcommand("delete", "Delete assets from a pack");

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
	});
}

void DelCmd::Callback() {
}