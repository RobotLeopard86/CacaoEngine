#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
#include "yaml-cpp/yaml.h"

CreateCmd::CreateCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("create", "Create a new asset pack");

	//Directories
	CLI::Option* assets = cmd->add_option("-a,--assets-dir", assetRoot, "Path to a directory containing assets to place in this pack. Subdirectories of this path will not be searched. Use --help-assets-dir to see more info.")->check(CLI::ExistingDirectory);
	CLI::Option* res = cmd->add_option("-r,--res-dir", resRoot, "Path to a directory containing arbitray files to embed as resources in this pack. The folder structure will be copied as-is.")->check(CLI::ExistingDirectory);
	assets->needs(res);
	res->needs(assets);

	//Address map
	CLI::Option* addr = cmd->add_option("-M,--addr-map", addrMapPath, "Path to a file mapping asset filenames to asset addresses for engine reference")->check(CLI::ExistingFile);
	addr->needs(assets);
	assets->needs(addr);

	//Assets directory help
	const auto assetsDirHelpFunc = []() {
		std::cout << "An asset in this context refers to one of:\n"
				  << "\t* A Cacao Engine packed shader\n"
				  << "\t* A Cacao Engine packed cubemap\n"
				  << "\t* A Cacao Engine packed material\n"
				  << "\t* A 2D texture file (.png, .jpg/.jpeg, .bmp, .tga, or .hdr)\n"
				  << "\t* A font file (.ttf or .otf)\n"
				  << "\t* A sound file (.mp3, .wav, .ogg (Ogg Vorbis), or .opus (Ogg Opus))\n\n"
				  << "Any file not in of these categories will not be placed into the asset pack.\n"
				  << "To embed an arbitrary resource, place it in a subpath of the directory specified in --res-dir." << std::endl;
		exit(0);
	};
	CLI::Option* assetsDirHelp = cmd->add_flag_callback("--help-assets-dir", assetsDirHelpFunc, "View more information about the --assets-dir option")->excludes(assets, res);
	assets->excludes(assetsDirHelp);
	res->excludes(assetsDirHelp);

	//Output
	CLI::Option* out = cmd->add_option("-o", outPath, "Output file path")->required()->check([](const std::string& outfile) {
		if(CLI::NonexistentPath(outfile).compare("") == 0) return "";
		if(CLI::ExistingFile(outfile).compare("") == 0) return "";
		return "The output file must either be a file to overwrite or a nonexistent file!";
	});
	out->needs(assets);
	assets->needs(out);
	assetsDirHelp->excludes(out);

	//Register command callback function
	cmd->callback([this, assets, assetsDirHelp]() {
		if(assets->count() <= 0 && assetsDirHelp->count() <= 0) {
			XAK_ERROR("Either --help-assets-dir or standard command options must be passed!")
		}
		this->Callback();
	});
}

void CreateCmd::Callback() {
	//Load address map
	VLOG_NONL("Loading asset address map... ")
	std::ifstream addrMapStream(addrMapPath);
	if(!addrMapStream.is_open()) {
		XAK_OP_ERROR("Failed to open address map file stream!")
	}
	YAML::Node addrMap = [&addrMapStream]() {
		try {
			return YAML::Load(addrMapStream);
		} catch(const YAML::ParserException& e) {
			XAK_OP_ERROR("Failed to parse address map doc: \"" << e.what() << "\"!")
		}
	}();
	VLOG("Done.")

	//Search for assets
	VLOG_NONL("Discovering assets... ")
	std::map<std::filesystem::path, std::string> assets;
	for(auto&& asset : std::filesystem::directory_iterator(assetRoot)) {
		if(!asset.is_regular_file()) continue;
		std::filesystem::path assetPath = asset.path();
		if(!addrMap[assetPath.filename().string()].IsScalar()) {
			XAK_OP_ERROR("Provided address map does not contain a mapping for " << assetPath.filename() << " or an incorrectly formatted one, thus the map is invalid!")
		}
		assets.insert_or_assign(assetPath, addrMap[assetPath.filename().string()].Scalar());
	}
	VLOG("Done.")

	//Search for resources
	VLOG_NONL("Discovering resources... ")
	std::vector<std::filesystem::path> resources;
	for(auto&& res : std::filesystem::recursive_directory_iterator(resRoot)) {
		if(!res.is_regular_file()) continue;
		resources.push_back(res.path());
	}
	VLOG("Done.")

	//Add resources to asset table
	std::map<std::string, libcacaoformats::PackedAsset> assetTable;
	for(const std::filesystem::path& res : resources) {
		//Log
		VLOG_NONL("Loading resource " << res << "... ")

		//Create packed asset object
		libcacaoformats::PackedAsset pa = {};
		pa.kind = libcacaoformats::PackedAsset::Kind::Resource;

		//Load buffer
		pa.buffer = [res]() {
			std::ifstream stream(res);
			if(!stream.is_open()) {
				XAK_ERROR("Failed to open resource data stream!")
			}
			try {
				//Grab size
				stream.clear();
				stream.exceptions(std::ios::failbit | std::ios::badbit);
				stream.seekg(0, std::ios::end);
				auto size = stream.tellg();
				stream.seekg(0, std::ios::beg);

				//Read data
				std::vector<unsigned char> contents(size);
				stream.read(reinterpret_cast<char*>(contents.data()), size);

				return contents;
			} catch(std::ios_base::failure& ios_failure) {
				if(errno == 0) {
					XAK_ERROR("Failed to read resource data stream: \"" << ios_failure.what() << "\"!")
				}
				XAK_ERROR("Failed to read resource data stream!");
			}
		}();
		VLOG("Done.")

		//Get relative path for identifier
		std::filesystem::path rel2Root = std::filesystem::relative(res, resRoot);

		//Insert
		assetTable.insert_or_assign(rel2Root, pa);
	}
}