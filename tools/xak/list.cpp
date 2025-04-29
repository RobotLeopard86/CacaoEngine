#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"

ListCmd::ListCmd(CLI::App& app) {
	//List the command CLI
	cmd = app.add_subcommand("list", "List assets in a pack");
	cmd->excludes(app.get_option("-q"));
	cmd->excludes(app.get_option("-V"));

	//Input
	cmd->add_option("input", inPath, "Path to an asset pack file to read as input")->required()->check(CLI::ExistingFile);

	//Output control
	doAssets = true;
	doResources = true;
	CLI::Option* assetsOnly = cmd->add_flag_callback("-A,--assets-only", [this]() { doResources = false; }, "Only list assets");
	CLI::Option* resOnly = cmd->add_flag_callback("-R,--resources-only", [this]() { doAssets = false; }, "Only list resources");
	assetsOnly->excludes(resOnly);
	resOnly->excludes(assetsOnly);

	//Metadata
	assetMeta = true;
	CLI::Option* noMeta = cmd->add_flag_callback("--no-meta", [this]() { assetMeta = false; }, "Disable printing of asset types")->excludes(resOnly);

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
		if(fail) exit(1);
	});
}

void ListCmd::Callback() {
	//Open input file
	std::ifstream in(inPath, std::ios::binary);
	if(!in.is_open()) {
		XAK_ERROR("Failed to open input file stream for reading!")
	}

	//Decode the file
	libcacaoformats::PackedDecoder dec;
	std::map<std::string, libcacaoformats::PackedAsset> decoded;
	try {
		libcacaoformats::PackedContainer pc = libcacaoformats::PackedContainer::FromStream(in);
		decoded = dec.DecodeAssetPack(pc);
	} catch(const std::runtime_error& e) {
		XAK_ERROR(e.what());
	}

	//Sort files by type
	std::vector<std::string> a, r;
	for(const auto& [addr, pa] : decoded) {
		if(pa.kind == libcacaoformats::PackedAsset::Kind::Resource && doResources) {
			r.push_back(addr);
			continue;
		}
		if(doAssets) {
			a.push_back(addr);
			continue;
		}
	}

	//Results
	if(a.size() > 0) {
		std::cout << "Assets:" << std::endl;
		for(const std::string& asset : a) {
			std::cout << asset;
			if(assetMeta) {
				std::cout << " (";
				switch(decoded[asset].kind) {
					case libcacaoformats::PackedAsset::Kind::Cubemap:
						std::cout << "Cubemap)";
						break;
					case libcacaoformats::PackedAsset::Kind::Shader:
						std::cout << "Shader)";
						break;
					case libcacaoformats::PackedAsset::Kind::Material:
						std::cout << "Material)";
						break;
					case libcacaoformats::PackedAsset::Kind::Font:
						std::cout << "Font)";
						break;
					case libcacaoformats::PackedAsset::Kind::Model:
						std::cout << "Model)";
						break;
					case libcacaoformats::PackedAsset::Kind::Sound:
						std::cout << "Sound)";
						break;
					case libcacaoformats::PackedAsset::Kind::Tex2D:
						std::cout << "2D Texture)";
						break;
					default:
						std::cout << "Unknown)";
						break;
				}
			}
			std::cout << std::endl;
		}
	}
	if(r.size() > 0) {
		if(a.size() > 0) std::cout << std::endl;
		std::cout << "Resources:" << std::endl;
		for(const std::string& res : r) {
			std::cout << res << std::endl;
		}
	}
}