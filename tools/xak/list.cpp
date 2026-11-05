#include "commands.hpp"

#include <string>

#include "libcacaoasset.hpp"

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
	CLI::Option* resOnly = cmd->add_flag_callback("-R,--resources-only", [this]() { doAssets = false; }, "Only list blob resources");
	assetsOnly->excludes(resOnly);
	resOnly->excludes(assetsOnly);

	//Metadata
	assetMeta = true;
	cmd->add_flag_callback("--no-meta", [this]() { assetMeta = false; }, "Disable printing of asset types")->excludes(resOnly);

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
		if(fail) exit(1);
	});
}

void ListCmd::Callback() {
	//Load the pack
	libcacaoasset::AssetPack pack = [this]() -> libcacaoasset::AssetPack {
		try {
			return libcacaoasset::AssetPack::OpenFromFile(inPath.string());
		} catch(...) {
			XAK_ERROR_NONVOID(libcacaoasset::AssetPack {}, "Failed to open asset pack!")
		}
	}();

	//Sort files by type
	std::vector<std::string> a, r;
	for(const std::string& addr : pack.ListResources()) {
		libcacaoasset::Resource res = pack.GetResource(addr);
		if(res.type == libcacaoasset::Resource::Type::Blob && doResources) {
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
				switch(pack.GetResource(asset).type) {
					case libcacaoasset::Resource::Type::Cubemap:
						std::cout << "Cubemap)";
						break;
					case libcacaoasset::Resource::Type::Shader:
						std::cout << "Shader)";
						break;
					case libcacaoasset::Resource::Type::Material:
						std::cout << "Material)";
						break;
					case libcacaoasset::Resource::Type::Font:
						std::cout << "Font)";
						break;
					case libcacaoasset::Resource::Type::Model:
						std::cout << "Model)";
						break;
					case libcacaoasset::Resource::Type::Audio:
						std::cout << "Audio)";
						break;
					case libcacaoasset::Resource::Type::Tex2D:
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
		std::cout << "Blob Resources:" << std::endl;
		for(const std::string& res : r) {
			std::cout << res << std::endl;
		}
	}
}