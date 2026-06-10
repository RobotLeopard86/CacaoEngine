#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoasset.hpp"
#include "spinners.hpp"

MergeCmd::MergeCmd(CLI::App& app) {
	//Merge the command CLI
	cmd = app.add_subcommand("merge", "Merge multiple assets packs into a single pack");

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
	std::unordered_map<std::string, std::filesystem::path> origins;
	libcacaoasset::AssetPack out = libcacaoasset::AssetPack::CreateEmpty();

	//Load each pack
	for(const std::filesystem::path& pakPath : inPaks) {
		//Get packed container
		CVLOG_SINGLE("Processing pack " << pakPath << "...")
		CVLOG_NONL("\tReading pack... ")
		libcacaoasset::AssetPack pack = [&pakPath]() -> libcacaoasset::AssetPack {
			try {
				return libcacaoasset::AssetPack::OpenFromFile(pakPath);
			} catch(...) {
				XAK_ERROR_NONVOID(libcacaoasset::AssetPack::CreateEmpty(), "Failed to open asset pack!")
			}
		}();
		CVLOG("Done.")

		//Check for conflicts with already loaded assets
		CVLOG_NONL("\tChecking for conflicts... ")
		for(const std::string& res : pack.ListResources()) {
			for(const std::string& res2 : out.ListResources()) {
				if(res.compare(res2) == 0) {
					XAK_ERROR("Resource address collision on resource \"" << res << "\", found in " << origins.at(res) << " and " << pakPath << "!")
				}
			}
		}
		CVLOG("Done.")

		//Record origins and merge
		for(const std::string& res : pack.ListResources()) {
			libcacaoasset::Resource r = pack.GetResource(res);
			out.PutResource(res, std::move(r));
			origins.insert_or_assign(res, pakPath);
		}
	}

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
	out.Export(&outStream);
	CVLOG("Done.")
}