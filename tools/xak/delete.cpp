#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
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
	libcacaoformats::AssetPack work;
	{
		//Get packed container
		CVLOG_NONL("Reading pack... ")
		libcacaoformats::PackedContainer pak = [this]() {
			std::ifstream stream(inPath);
			if(!stream.is_open()) {
				XAK_ERROR_NONVOID(libcacaoformats::PackedContainer {}, "Failed to open pack file stream!")
			}
			try {
				return libcacaoformats::PackedContainer::FromStream(stream);
			} catch(const std::exception& e) {
				XAK_ERROR_NONVOID(libcacaoformats::PackedContainer {}, "Failed to create pack object: \"" << e.what() << "\"!")
			}
		}();
		if(fail) return;
		CVLOG("Done.")

		//Decode the pack
		CVLOG_NONL("Decoding pack... ")
		work = [&pak]() {
			try {
				libcacaoformats::PackedDecoder dec;
				return dec.DecodeAssetPack(pak);
			} catch(const std::exception& e) {
				XAK_ERROR_NONVOID(libcacaoformats::AssetPack {}, "Failed to decode asset pack: \"" << e.what() << "\"!")
			}
		}();
		if(fail) return;
		CVLOG("Done.")
	}

	//Find and delete the requested assets
	for(const std::string asset : toDelete) {
		CVLOG_NONL("Deleting asset \"" << asset << "\"... ")
		if(!work.contains(asset)) {
			XAK_ERROR("Pack does not contain asset \"" << asset << "\"!")
		}
		work.erase(asset);
		CVLOG("Done.")
	}

	//Encode modified pack
	CVLOG_NONL("Encoding modified pack... ")
	libcacaoformats::PackedContainer pc = [&work]() {
		try {
			libcacaoformats::PackedEncoder enc;
			return enc.EncodeAssetPack(work);
		} catch(const std::exception& e) {
			XAK_ERROR_NONVOID(libcacaoformats::PackedContainer {}, "Failed to encode asset pack: \"" << e.what() << "\"!")
		}
	}();
	if(fail) return;
	CVLOG("Done.")

	//Write pack to output file
	CVLOG_NONL("Writing back changes... ")
	std::ofstream outStream(inPath, std::ios::binary);
	if(!outStream.is_open()) {
		XAK_ERROR("Failed to open output stream!")
	}
	pc.ExportToStream(outStream);
	CVLOG("Done.")
}