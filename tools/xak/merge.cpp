#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
#include "spinners.hpp"

MergeCmd::MergeCmd(CLI::App& app) {
	//Merge the command CLI
	cmd = app.add_subcommand("merge", "Merge two assets packs into a new pack");

	//Inputs
	cmd->add_option("input", inPaks, "Asset packs to merge")->required()->check(CLI::ExistingFile);

	//Output
	CLI::Option* out = cmd->add_option("-o", outPath, "Output file path")->required()->check([](const std::string& outfile) {
		if(CLI::NonexistentPath(outfile).compare("") == 0) return "";
		if(CLI::ExistingFile(outfile).compare("") == 0) return "";
		return "The output file must either be a file to overwrite or a nonexistent file!";
	});
	out->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});

	//Register command callback function
	cmd->callback([this]() {
		if(inPaks.size() < 2) {
			std::cerr << "At least two packs must be provided!" << std::endl;
			exit(1);
		}
		std::unique_ptr<jms::Spinner> s;
		std::stringstream taskDesc;
		taskDesc << "Merging packs...";
		if(outputLvl != OutputLevel::Silent) {
			s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
			s->start();
		}
		this->Callback();
		if(outputLvl != OutputLevel::Silent) {
			taskDesc.str("");
			if(fail) {
				taskDesc << "Failed to merge packs!";
				s->finish(jms::FinishedState::FAILURE, taskDesc.str());
				exit(1);
			} else {
				taskDesc << "Merged to " << outPath << ".";
				s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
			}
		}
	});
}

void MergeCmd::Callback() {
	std::map<std::string, std::filesystem::path> origins;
	libcacaoformats::AssetPack work;

	//Load each pack
	libcacaoformats::PackedDecoder dec;
	for(const std::filesystem::path& pakPath : inPaks) {
		//Get packed container
		CVLOG_SINGLE("Processing pack " << pakPath << "...")
		CVLOG_NONL("\tReading pack... ")
		libcacaoformats::PackedContainer pak = [&pakPath]() {
			std::ifstream stream(pakPath);
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
		CVLOG_NONL("\tDecoding pack... ")
		libcacaoformats::AssetPack out = [&pak, &dec]() {
			try {
				return dec.DecodeAssetPack(pak);
			} catch(const std::exception& e) {
				XAK_ERROR_NONVOID(libcacaoformats::AssetPack {}, "Failed to decode asset pack: \"" << e.what() << "\"!")
			}
		}();
		if(fail) return;
		CVLOG("Done.")

		//Check for conflicts with already loaded assets
		CVLOG_NONL("\tChecking for conflicts... ")
		for(const auto& [asset, _] : work) {
			for(const auto& [asset2, _] : out) {
				if(asset.compare(asset2) == 0) {
					XAK_ERROR("Asset address collision on asset \"" << asset << "\", found in " << origins.at(asset) << " and " << pakPath << "!")
				}
			}
		}
		CVLOG("Done.")

		//Record origins
		for(const auto& [asset, _] : out) {
			origins.insert_or_assign(asset, pakPath);
		}

		//Merge
		work.merge(out);
	}

	//Encode merged pack
	CVLOG_NONL("Encoding merged pack... ")
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

	//Make output directory if it doesn't exist
	if(!std::filesystem::exists(outPath.parent_path())) {
		std::filesystem::create_directories(outPath.parent_path());
	}

	//Write pack to output file
	CVLOG_NONL("Writing output file " << outPath << "... ")
	std::ofstream outStream(outPath, std::ios::binary);
	if(!outStream.is_open()) {
		XAK_ERROR("Failed to open output file stream!")
	}
	pc.ExportToStream(outStream);
	CVLOG("Done.")
}