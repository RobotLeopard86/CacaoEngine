#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
#include "spinners.hpp"

ExtractCmd::ExtractCmd(CLI::App& app) {
	//Extract the command CLI
	cmd = app.add_subcommand("extract", "Extract assets from a pack");

	//Input
	cmd->add_option("input", inPath, "Path to an asset pack file to read as input")->required()->check(CLI::ExistingFile);

	//Extraction
	CLI::Option* extract = cmd->add_option("assets", toExtract, "Assets to extract from the pack");
	CLI::Option* allOpt = cmd->add_flag("-a,--all-assets", all, "Extract all assets from the pack");
	extract->excludes(allOpt);
	allOpt->excludes(extract);

	//Output directory
	CLI::Option* outOpt = cmd->add_option("-o", outDir, "Directory to place output files in")->required()->check([](const std::string& outdir) {
		if(CLI::NonexistentPath(outdir).compare("") == 0) return "";
		if(CLI::ExistingDirectory(outdir).compare("") == 0) return "";
		return "The output directory must either be a directory to write into or a nonexistent directory!";
	});
	outOpt->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});

	//Register command callback function
	cmd->callback([this]() {
		if(toExtract.size() <= 0 && !all) {
			std::cerr << "At least one asset to extract must be specified!" << std::endl;
			exit(1);
		}
		std::unique_ptr<jms::Spinner> s;
		std::stringstream taskDesc;
		taskDesc << "Extracting assets from " << inPath << "...";
		if(outputLvl != OutputLevel::Silent) {
			s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
			s->start();
		}
		this->Callback();
		if(outputLvl != OutputLevel::Silent) {
			taskDesc.str("");
			if(fail) {
				taskDesc << "Asset extraction failed!";
				s->finish(jms::FinishedState::FAILURE, taskDesc.str());
				exit(1);
			} else {
				taskDesc << "Extracted assets to " << outDir << ".";
				s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
			}
		}
	});
}

void ExtractCmd::Callback() {
	//Load the pack
	libcacaoformats::AssetPack pak;
	{
		//Get packed container
		CVLOG_NONL("Reading pack... ")
		libcacaoformats::PackedContainer pc = [this]() {
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
		pak = [&pc]() {
			try {
				libcacaoformats::PackedDecoder dec;
				return dec.DecodeAssetPack(pc);
			} catch(const std::exception& e) {
				XAK_ERROR_NONVOID(libcacaoformats::AssetPack {}, "Failed to decode asset pack: \"" << e.what() << "\"!")
			}
		}();
		if(fail) return;
		CVLOG("Done.")
	}

	//If all assets requested, get them
	if(all) {
		for(const auto& [asset, _] : pak) {
			toExtract.push_back(asset);
		}
	}

	//Define output map
	std::map<std::filesystem::path, std::vector<unsigned char>> out;

	//Find the requested assets and assign their paths
	for(const std::string& asset : toExtract) {
		CVLOG_NONL("Checking for asset \"" << asset << "\"... ")
		if(!pak.contains(asset)) {
			XAK_ERROR("Pack does not contain asset \"" << asset << "\"!")
		}
		libcacaoformats::PackedAsset& pa = pak.at(asset);
		CVLOG("Done.")

		//Path assignment
		std::filesystem::path oasset = outDir;
		if(pa.kind == libcacaoformats::PackedAsset::Kind::Resource) {
			(oasset /= "res") /= asset;
		} else {
			oasset /= asset;
			switch(pa.kind) {
				case libcacaoformats::PackedAsset::Kind::Cubemap:
					oasset += ".xjc";
					break;
				case libcacaoformats::PackedAsset::Kind::Shader:
					oasset += ".xjs";
					break;
				case libcacaoformats::PackedAsset::Kind::Material:
					oasset += ".xjm";
					break;
				case libcacaoformats::PackedAsset::Kind::Font:
					if(pa.buffer[0] == 'O') {
						oasset += ".otf";
					} else {
						oasset += ".ttf";
					}
					break;
				case libcacaoformats::PackedAsset::Kind::Model:
					if(pa.buffer[0] == 'K') {
						oasset += ".fbx";
					} else if(pa.buffer[0] == 'g') {
						oasset += ".glb";
					} else if(pa.buffer[0] == '<') {
						oasset += ".dae";
					} else {
						oasset += ".obj";
					}
					break;
				case libcacaoformats::PackedAsset::Kind::Tex2D:
					if(pa.buffer[0] == 0x89) {
						oasset += ".png";
					} else if(pa.buffer[0] == 0xFF) {
						oasset += ".jpg";
					} else if(pa.buffer[0] == 'I') {
						oasset += ".tiff";
					} else if(pa.buffer[0] == 'R') {
						oasset += ".webp";
					} else {
						oasset += ".tga";
					}
					break;
				case libcacaoformats::PackedAsset::Kind::Sound:
					if(pa.buffer[0] == 0xFF || pa.buffer[0] == 'I') {
						oasset += ".mp3";
					} else if(pa.buffer[0] == 'R') {
						oasset += ".wav";
					} else if(pa.buffer[34] == 'a') {
						oasset += ".opus";
					} else {
						oasset += ".ogg";
					}
					break;
				default: break;
			}
		}
		out.insert_or_assign(oasset, std::move(pa.buffer));
	}

	//Prep output directory
	CVLOG_NONL("Prepping output directory " << outDir << "... ")
	if(std::filesystem::exists(outDir)) {
		std::filesystem::remove_all(outDir);
	}
	std::filesystem::create_directories(outDir);
	CVLOG("Done.")

	//Write files
	for(const auto& [path, buf] : out) {
		CVLOG_NONL("Writing asset " << path << "... ")
		bool binary = (path.extension().compare(".obj") == 0 || path.extension().compare(".dae") == 0);
		std::ofstream outStream(path, binary ? std::ios::binary : (std::ios_base::openmode)0);
		if(!outStream.is_open()) {
			if(outputLvl == OutputLevel::Verbose) {
				XAK_ERROR("Failed to open output stream!")
			} else {
				XAK_ERROR("Failed to open output stream for file " << path << "!")
			}
		}
		outStream.write(reinterpret_cast<const char*>(buf.data()), buf.size());
		outStream.flush();
		CVLOG("Done.")
	}
}