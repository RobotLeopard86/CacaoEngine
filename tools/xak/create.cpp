#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"

CreateCmd::CreateCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("create", "Create a new asset pack");

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
	});
}

void CreateCmd::Callback() {
}