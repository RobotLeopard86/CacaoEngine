#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"

ListCmd::ListCmd(CLI::App& app) {
	//List the command CLI
	cmd = app.add_subcommand("list", "List assets in a pack");

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
	});
}

void ListCmd::Callback() {
}