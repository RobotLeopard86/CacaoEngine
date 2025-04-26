#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"

MergeCmd::MergeCmd(CLI::App& app) {
	//Merge the command CLI
	cmd = app.add_subcommand("merge", "Merge two assets packs into a new pack");

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
	});
}

void MergeCmd::Callback() {
}