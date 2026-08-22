#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoasset.hpp"
#include "spinners.hpp"

DelCmd::DelCmd(CLI::App& app) {
	//Delete the command CLI
	cmd = app.add_subcommand("delete", "Delete assets from a pack");

	//Input pack
	cmd->add_option("input", inPath, "Path to an asset pack file to read as input")->required()->check(CLI::ExistingFile);

	//Deletion
	cmd->add_option("assets", toDelete, "Assets to delete from the pack")->required();

	//Register command callback function
	cmd->callback([this]() {
		if(toDelete.size() <= 0) {
			std::cerr << "At least one asset to remove must be specified!" << std::endl;
			exit(1);
		}
		std::unique_ptr<jms::Spinner> s;
		std::stringstream taskDesc;
		taskDesc << "Deleting assets from " << inPath << "...";
		if(outputLvl != OutputLevel::Silent) {
			s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
			s->start();
		}
		this->Callback();
		if(outputLvl != OutputLevel::Silent) {
			taskDesc.str("");
			if(fail) {
				taskDesc << "Asset deletion failed!";
				s->finish(jms::FinishedState::FAILURE, taskDesc.str());
				exit(1);
			} else {
				taskDesc << "Deleted assets from " << inPath << ".";
				s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
			}
		}
	});
}

void DelCmd::Callback() {
	//Load the pack
	CVLOG_NONL("Reading pack... ")
	libcacaoasset::AssetPack pack = [this]() -> libcacaoasset::AssetPack {
		try {
			return libcacaoasset::AssetPack::OpenFromFile(inPath.string());
		} catch(...) {
			XAK_ERROR_NONVOID(libcacaoasset::AssetPack::CreateEmpty(), "Failed to open asset pack!")
		}
	}();
	CVLOG("Done.")

	//Delete the requested assets
	for(const std::string& asset : toDelete) {
		CVLOG_NONL("Deleting asset \"" << asset << "\"... ")
		pack.DeleteResource(asset);
		CVLOG("Done.")
	}

	//Write pack to output file
	CVLOG_NONL("Writing output file " << inPath << "... ")
	std::ofstream outStream(inPath, std::ios::binary);
	if(!outStream.is_open()) {
		XAK_ERROR("Failed to open output file stream!")
	}
	pack.Export(&outStream);
	CVLOG("Done.")
}