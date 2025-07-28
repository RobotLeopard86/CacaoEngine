#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
#include "libcacaoimage.hpp"

CreateCmd::CreateCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("create", "Create a new cubemap");

	//Input
	cmd->add_option("input", inPath, "Path to an unpacked cubemap definition file (.ajc) to read as input")->required()->check(CLI::ExistingFile)->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});

	//Output
	cmd->add_option("-o", outPath, "Output file path")->required()->check([](const std::string& outfile) {
		if(CLI::NonexistentPath(outfile).compare("") == 0) return "";
		if(CLI::ExistingFile(outfile).compare("") == 0) return "";
		return "The output file must either be a file to overwrite or a nonexistent file!";
	});

	//Register command callback function
	cmd->callback([this]() {
		this->Callback();
	});
}

void CreateCmd::Callback() {
	//Load the input file
	VLOG_NONL("Opening input file " << inPath << "... ");
	std::ifstream in(inPath);
	if(!in.is_open()) {
		CUBE_ERROR("Failed to open input file stream for reading!")
	}
	VLOG("Done.")

	//Create the loader function
	auto loaderFunc = [this](const std::string& pathStr) {
		//We need to chdir to the path of the file to make the absolute calculation correct
		VLOG_NONL("\tChecking file path " << pathStr << "... ")
		std::filesystem::path cur = std::filesystem::current_path();
		std::filesystem::current_path(inPath.parent_path());
		std::filesystem::path p = std::filesystem::absolute(pathStr);
		std::filesystem::current_path(cur);

		//Ensure the file exists
		if(!std::filesystem::exists(p)) {
			CUBE_ERROR("Input file specifies a nonexistent file!")
		}
		VLOG("Done.")

		//Open the stream
		VLOG_NONL("\tOpening file stream... ")
		std::unique_ptr<std::ifstream> in = std::make_unique<std::ifstream>(p);
		if(!in->is_open()) {
			CUBE_ERROR("Failed to open face file stream for reading!")
		}
		VLOG("Done.")

		//Convert pointer to std::istream and return
		return std::unique_ptr<std::istream>(static_cast<std::istream*>(in.release()));
	};

	//Decode the file
	VLOG("Loading input files:")
	libcacaoformats::UnpackedDecoder dec;
	std::array<libcacaoimage::Image, 6> decoded;
	try {
		decoded = dec.DecodeCubemap(in, loaderFunc);
	} catch(const std::runtime_error& e) {
		CUBE_ERROR(e.what());
	}
	//Encode the file
	VLOG_NONL("Generating output... ")
	libcacaoformats::PackedEncoder enc;
	std::pair<bool, std::string> pcGetErr = {true, ""};
	libcacaoformats::PackedContainer pc = [&]() {
		try {
			return enc.EncodeCubemap(decoded);
		} catch(const std::runtime_error& e) {
			pcGetErr = {false, e.what()};
			return libcacaoformats::PackedContainer();
		}
	}();
	if(!pcGetErr.first) {
		CUBE_ERROR(pcGetErr.second)
	}
	VLOG("Done.")

	//Write it out
	VLOG_NONL("Writing output file " << outPath << "... ");
	std::ofstream out(outPath, std::ios::binary);
	if(!in.is_open()) {
		CUBE_ERROR("Failed to open output file stream for writing!")
	}
	pc.ExportToStream(out);
	VLOG("Done.")
}