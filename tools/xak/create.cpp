#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
#include "yaml-cpp/yaml.h"
#include "spinners.hpp"

CreateCmd::CreateCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("create", "Create a new asset pack");

	//Directories
	CLI::Option* assets = cmd->add_option("-a,--assets-dir", assetRoot, "Path to a directory containing assets to place in this pack. Subdirectories of this path will not be searched. Use --help-assets-dir to see more info.")->check(CLI::ExistingDirectory);
	CLI::Option* res = cmd->add_option("-r,--res-dir", resRoot, "Path to a directory containing arbitrary files to embed as blob resources in this pack. The folder structure will be copied as-is.")->check(CLI::ExistingDirectory);

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
				  << "\t* A 2D texture file (PNG, JPEG, WebP, Targa/TGA, or TIFF)\n"
				  << "\t* A model file (FBX, glTF2 binary (.glb), Collada (.dae), or Wavefront OBJ) containing one or more meshes and optionally textures.\n"
				  << "\t* A font file (TrueType or OpenType)\n"
				  << "\t* A sound file (MP3, WAV, Ogg Vorbis, Ogg Opus)\n\n"
				  << "Any file not in of these categories will not be placed into the asset pack.\n"
				  << "To embed an arbitrary blob resource, place it in a subpath of the directory specified in --res-dir.\n\n";
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
	out->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});
	assets->needs(out);
	res->needs(out);
	assetsDirHelp->excludes(out);

	//Register command callback function
	cmd->callback([this, assets, assetsDirHelp]() {
		if(assets->count() <= 0 && assetsDirHelp->count() <= 0) {
			std::cerr << "Either --help-assets-dir or standard command options must be passed!" << std::endl;
			exit(1);
		}
		std::unique_ptr<jms::Spinner> s;
		std::stringstream taskDesc;
		taskDesc << "Creating pack " << outPath << "...";
		if(outputLvl != OutputLevel::Silent) {
			s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
			s->start();
		}
		this->Callback();
		if(outputLvl != OutputLevel::Silent) {
			taskDesc.str("");
			if(fail) {
				taskDesc << "Pack creation failed!";
				s->finish(jms::FinishedState::FAILURE, taskDesc.str());
				exit(1);
			} else {
				taskDesc << "Created pack " << outPath << ".";
				s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
			}
		}
	});
}

void CreateCmd::Callback() {
	bool noAsset = (assetRoot.compare("") == 0);
	bool noRes = (resRoot.compare("") == 0);

	//Define asset table
	libcacaoformats::AssetPack assetTable;
	std::unordered_map<std::filesystem::path, std::string> assets;
	std::vector<std::filesystem::path> resources;
	YAML::Node addrMap;
	if(noAsset) goto res_begin;

	//Load address map
	CVLOG_NONL("Loading asset address map... ")
	addrMap = [this]() {
		std::ifstream addrMapStream(addrMapPath);
		if(!addrMapStream.is_open()) {
			XAK_ERROR_NONVOID(YAML::Node {}, "Failed to open asset address map file stream!")
		}
		try {
			YAML::Node node = YAML::Load(addrMapStream);
			if(!node.IsMap()) {
				XAK_ERROR_NONVOID(YAML::Node {}, "Asset address map is not a YAML map!")
			}
			if(node.size() < 1) {
				XAK_ERROR_NONVOID(YAML::Node {}, "Asset address map has no entries!")
			}
			return node;
		} catch(const YAML::ParserException& e) {
			XAK_ERROR_NONVOID(YAML::Node {}, "Failed to parse asset address map YAML: \"" << e.what() << "\"!")
		}
	}();
	if(fail) return;
	CVLOG("Done.")

	//Check for address map conflicts and errors
	{
		std::vector<std::string> foundAddrs;

		//Convert to a std::unordered_map for simpler iteration
		const std::unordered_map<std::string, std::string> addrMapAsMap = addrMap.as<std::unordered_map<std::string, std::string>>();
		for(const auto& [_, v] : addrMapAsMap) {
			if(std::find(foundAddrs.cbegin(), foundAddrs.cend(), v) != foundAddrs.cend()) {
				XAK_ERROR("Asset address map contains duplicate addresses!")
			}
			if(v.find_first_not_of("abcdefghghijklmnopqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ0123456789_") != std::string::npos) {
				XAK_ERROR("Address listed in map contains invalid characters! (Hint: only lowercase letters, uppercase letters, numbers, and underscores are allowed.)")
			}
			foundAddrs.push_back(v);
		}
	}

	//Search for assets
	CVLOG_NONL("Discovering assets... ")
	for(auto&& asset : std::filesystem::directory_iterator(assetRoot)) {
		if(!asset.is_regular_file()) continue;
		std::filesystem::path assetPath = asset.path();
		assets.insert_or_assign(assetPath, addrMap[assetPath.filename().string()].IsScalar() ? addrMap[assetPath.filename().string()].Scalar() : "\0");
	}
	CVLOG("Done.")
	if(noRes) goto asset_process;

	//Search for blob resources
res_begin:
	CVLOG_NONL("Discovering blob resources... ")
	for(auto&& res : std::filesystem::recursive_directory_iterator(resRoot)) {
		if(!res.is_regular_file()) continue;
		resources.push_back(res.path());
	}
	CVLOG("Done.")

	//Add blob resources to asset table
	for(const std::filesystem::path& res : resources) {
		//Log
		CVLOG_NONL("Adding blob resource " << res << "... ")

		//Create packed asset object
		libcacaoformats::PackedAsset pa = {};
		pa.kind = libcacaoformats::PackedAsset::Kind::Resource;

		//Load buffer
		pa.buffer = [res]() {
			std::ifstream stream(res);
			if(!stream.is_open()) {
				XAK_ERROR_NONVOID(std::vector<unsigned char> {}, "Failed to open blob resource data stream!")
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
					XAK_ERROR_NONVOID(std::vector<unsigned char> {}, "Failed to read blob resource data stream: \"" << ios_failure.what() << "\"!")
				}
				XAK_ERROR_NONVOID(std::vector<unsigned char> {}, "Failed to read blob resource data stream!");
			}
		}();
		if(fail) return;

		//Get relative path for identifier
		std::string rel2Root = std::filesystem::relative(res, resRoot).string();

		//Insert into table
		assetTable.insert_or_assign(rel2Root, pa);
		CVLOG("Done.")
	}
	if(noAsset) goto encode;

//Add assets to asset table
asset_process:
	for(const auto& [asset, addr] : assets) {
		//Read file buffer
		std::stringstream logmsg;
		logmsg << "Checking if file " << asset << " ";
		if(addr.compare("\0") != 0) logmsg << "(assigned to address \"" << addr << "\") ";
		logmsg << "is an asset... ";
		CVLOG_NONL(logmsg.str())
		libcacaoformats::PackedAsset pa;
		pa.buffer = [asset]() {
			std::ifstream stream(asset, std::ios::binary);
			if(!stream.is_open()) {
				XAK_ERROR_NONVOID(std::vector<unsigned char> {}, "Failed to open asset data stream!")
			}
			try {
				//Grab size
				stream.clear();
				stream.exceptions(std::ios::failbit | std::ios::badbit);
				stream.seekg(0, std::ios::end);
				std::size_t size = stream.tellg();
				stream.seekg(0, std::ios::beg);

				//Read data
				std::vector<unsigned char> contents(size);
				stream.read(reinterpret_cast<char*>(contents.data()), size);

				return contents;
			} catch(std::ios_base::failure& ios_failure) {
				if(errno == 0) {
					XAK_ERROR_NONVOID(std::vector<unsigned char> {}, "Failed to read asset data stream: \"" << ios_failure.what() << "\"!")
				}
				XAK_ERROR_NONVOID(std::vector<unsigned char> {}, "Failed to read asset data stream!");
			}
		}();
		if(fail) return;

#pragma pack(push, 1)
		//TGA header struct (TGA has no magic number so we have to parse its header)
		struct TGAHeader {
			uint8_t idLen;
			uint8_t colormapType;
			uint8_t imageType;
			uint16_t cmapFirstEntry;
			uint16_t cmapLen;
			uint8_t cmapEntrySz;
			uint16_t originX;
			uint16_t originY;
			uint16_t width;
			uint16_t height;
			uint8_t pixelDepth;
			uint8_t imageDescriptor;
		};
#pragma pack(pop)

		//Check header for validity
		std::size_t pabSz = pa.buffer.size();
		if(pabSz >= 2) {
			if(pa.buffer[0] == 0xFF && pa.buffer[1] == 0xE0) {
				//MP3 audio
				pa.kind = libcacaoformats::PackedAsset::Kind::Sound;
				goto asset_ok;
			} else if(pa.buffer[0] == 'v' && pa.buffer[1] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 'o' && pa.buffer[1] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 'g' && pa.buffer[1] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 's' && pa.buffer[1] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 'f' && pa.buffer[1] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 'l' && pa.buffer[1] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			}
		}
		if(pabSz >= 3) {
			if(std::string str {(char)pa.buffer[0], (char)pa.buffer[1], (char)pa.buffer[2]}; str.compare("ID3") == 0) {
				//MP3 audio with an ID3 tag
				pa.kind = libcacaoformats::PackedAsset::Kind::Sound;
				goto asset_ok;
			} else if(pa.buffer[0] == 0xFF && pa.buffer[1] == 0xD8 && pa.buffer[2] == 0xFF) {
				//JPEG image
				pa.kind = libcacaoformats::PackedAsset::Kind::Tex2D;
				goto asset_ok;
			} else if(pa.buffer[0] == 'v' && pa.buffer[1] == 't' && pa.buffer[2] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 'v' && pa.buffer[1] == 'n' && pa.buffer[2] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 'v' && pa.buffer[1] == 'p' && pa.buffer[2] == ' ') {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			}
		}
		if(pabSz >= 4) {
			std::string str {(char)pa.buffer[0], (char)pa.buffer[1], (char)pa.buffer[2], (char)pa.buffer[3]};
			if(str.compare("OTTO") == 0) {
				//OpenType font
				pa.kind = libcacaoformats::PackedAsset::Kind::Font;
				goto asset_ok;
			} else if(str.compare("glTF") == 0) {
				//glTF binary model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(pa.buffer[0] == 0xCA && pa.buffer[1] == 0xCA && pa.buffer[2] == 0x00) {
				//Cacao packed container (still need to check type)
				if(pa.buffer[3] == 0xC4) {
					//Cubemap
					pa.kind = libcacaoformats::PackedAsset::Kind::Cubemap;
					goto asset_ok;
				} else if(pa.buffer[3] == 0x1B) {
					//Shader
					pa.kind = libcacaoformats::PackedAsset::Kind::Shader;
					goto asset_ok;
				} else if(pa.buffer[3] == 0x3E) {
					//Material
					pa.kind = libcacaoformats::PackedAsset::Kind::Material;
					goto asset_ok;
				}
			} else if(pa.buffer[0] == 'I' && pa.buffer[1] == 'I' && pa.buffer[2] == '*' && pa.buffer[3] == 0x00) {
				//TIFF image
				pa.kind = libcacaoformats::PackedAsset::Kind::Tex2D;
				goto asset_ok;
			}
		}
		if(pabSz >= 5 && pa.buffer[0] == 0x00 && pa.buffer[1] == 0x01 && pa.buffer[2] == 0x00 && pa.buffer[3] == 0x00 && pa.buffer[4] == 0x00) {
			//TrueType font
			pa.kind = libcacaoformats::PackedAsset::Kind::Font;
			goto asset_ok;
		}
		if(pabSz >= 6) {
			std::string str {(char)pa.buffer[0], (char)pa.buffer[1], (char)pa.buffer[2], (char)pa.buffer[3], (char)pa.buffer[4], (char)pa.buffer[5]};
			if(str.compare("mtllib") == 0) {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			} else if(str.compare("usemtl") == 0) {
				//OBJ model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			}
		}
		if(pabSz >= 8 && pa.buffer[0] == 0x89 && pa.buffer[1] == 0x50 && pa.buffer[2] == 0x4E && pa.buffer[3] == 0x47 &&
			pa.buffer[4] == 0x0D && pa.buffer[5] == 0x0A && pa.buffer[6] == 0x1A && pa.buffer[7] == 0x0A) {

			//PNG image
			pa.kind = libcacaoformats::PackedAsset::Kind::Tex2D;
			goto asset_ok;
		}
		if(pabSz >= 12) {
			if(std::string str {(char)pa.buffer[0], (char)pa.buffer[1], (char)pa.buffer[2], (char)pa.buffer[3], (char)pa.buffer[4], (char)pa.buffer[5],
				   (char)pa.buffer[6], (char)pa.buffer[7], (char)pa.buffer[8], (char)pa.buffer[9], (char)pa.buffer[10], (char)pa.buffer[11]};
				str.starts_with("RIFF")) {

				if(str.ends_with("WEBP")) {
					//WebP image
					pa.kind = libcacaoformats::PackedAsset::Kind::Tex2D;
					goto asset_ok;
				} else if(str.ends_with("WAVE")) {
					//WAV audio
					pa.kind = libcacaoformats::PackedAsset::Kind::Sound;
					goto asset_ok;
				}
			}
		}
		if(pabSz >= 18) {
			if(std::string str {(char)pa.buffer[0], (char)pa.buffer[1], (char)pa.buffer[2], (char)pa.buffer[3], (char)pa.buffer[4], (char)pa.buffer[5],
				   (char)pa.buffer[6], (char)pa.buffer[7], (char)pa.buffer[8], (char)pa.buffer[9], (char)pa.buffer[10], (char)pa.buffer[11],
				   (char)pa.buffer[12], (char)pa.buffer[13], (char)pa.buffer[14], (char)pa.buffer[15], (char)pa.buffer[16], (char)pa.buffer[17]};
				str.compare("Kaydara FBX Binary") == 0) {
				//FBX model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			}
		}
		if(pabSz >= 23) {
			std::string str {(char)pa.buffer[0], (char)pa.buffer[1], (char)pa.buffer[2], (char)pa.buffer[3], (char)pa.buffer[4], (char)pa.buffer[5],
				(char)pa.buffer[6], (char)pa.buffer[7], (char)pa.buffer[8], (char)pa.buffer[9], (char)pa.buffer[10], (char)pa.buffer[11],
				(char)pa.buffer[12], (char)pa.buffer[13], (char)pa.buffer[14], (char)pa.buffer[15], (char)pa.buffer[16], (char)pa.buffer[17],
				(char)pa.buffer[18], (char)pa.buffer[19], (char)pa.buffer[20], (char)pa.buffer[21], (char)pa.buffer[22], (char)pa.buffer[23],
				(char)pa.buffer[24], (char)pa.buffer[25], (char)pa.buffer[26], (char)pa.buffer[27], (char)pa.buffer[28], (char)pa.buffer[29],
				(char)pa.buffer[30], (char)pa.buffer[31], (char)pa.buffer[32], (char)pa.buffer[33], (char)pa.buffer[34], (char)pa.buffer[35]};

			if(str.starts_with("OggS")) {
				if(str.find("vorbis") != std::string::npos) {
					//Ogg Vorbis audio
					pa.kind = libcacaoformats::PackedAsset::Kind::Sound;
					goto asset_ok;
				} else if(str.find("OpusHead") != std::string::npos) {
					//Ogg Opus audio
					pa.kind = libcacaoformats::PackedAsset::Kind::Sound;
					goto asset_ok;
				}
			}
		}
		if(pabSz >= 62) {
			//Get past (maybe) the XML header to find the COLLADA tag
			auto xmlIt = std::find(pa.buffer.cbegin(), pa.buffer.cend(), '>');
			if(xmlIt == pa.buffer.cend()) goto asset_skip;
			unsigned int xml = std::distance(pa.buffer.cbegin(), xmlIt);
			std::string str {(char)pa.buffer[xml + 1], (char)pa.buffer[xml + 2], (char)pa.buffer[xml + 3], (char)pa.buffer[xml + 4],
				(char)pa.buffer[xml + 5], (char)pa.buffer[xml + 6], (char)pa.buffer[xml + 7], (char)pa.buffer[xml + 8], (char)pa.buffer[xml + 9]};

			if(str.find("<COLLADA") != std::string::npos) {
				//Collada model
				pa.kind = libcacaoformats::PackedAsset::Kind::Model;
				goto asset_ok;
			}
		}
		if(pabSz >= sizeof(TGAHeader)) {
			//Obtain the TGA header
			TGAHeader tga = {};
			std::memcpy(&tga, pa.buffer.data(), sizeof(TGAHeader));

			//Do some checks on the header
			if(tga.colormapType > 1) goto asset_skip;
			if(tga.imageType == 0) goto asset_skip;
			if(tga.width < 1 || tga.height < 1) goto asset_skip;
			if(tga.pixelDepth != 8 && tga.pixelDepth != 15 && tga.pixelDepth != 16 && tga.pixelDepth != 24 && tga.pixelDepth != 32) goto asset_skip;
			if(tga.colormapType == 1) {
				if(tga.imageType != 1 && tga.imageType != 9) goto asset_skip;
				if(tga.cmapEntrySz != 8 && tga.cmapEntrySz != 15 && tga.cmapEntrySz != 16 && tga.cmapEntrySz != 24 && tga.cmapEntrySz != 32) goto asset_skip;
			} else {
				if(tga.imageType != 2 && tga.imageType != 3 && tga.imageType != 10 && tga.imageType != 11) goto asset_skip;
			}

			//Valid (probably) TGA image
			pa.kind = libcacaoformats::PackedAsset::Kind::Tex2D;
			goto asset_ok;
		}

		//Non-asset
	asset_skip:
		CVLOG("Skipped (not an asset).")
		continue;
	asset_ok:
		CVLOG("Done.")

		//Auto-generate address if not listed
		std::string trueAddr = addr;
		if(addr.compare("\0") == 0) {
			CVLOG_NONL("Generating address... ")
			std::stringstream gen;
			gen << asset.filename().stem().string();
			int counter = 0;
			std::string base = gen.str();
			std::string work = base;
			do {
				work = base;
				work += std::to_string(counter++);
			} while(assetTable.contains(work));
			trueAddr = work;
			CVLOG("Done.")
		}

		//Add to table
		CVLOG_NONL("Adding asset \"" << trueAddr << "\"... ")
		assetTable.insert_or_assign(trueAddr, pa);
		CVLOG("Done.")
	}

encode:
	//Encode asset pack
	CVLOG_NONL("Encoding pack... ")
	libcacaoformats::PackedContainer pc = [&assetTable]() {
		try {
			libcacaoformats::PackedEncoder enc;
			return enc.EncodeAssetPack(assetTable);
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