#include "commands.hpp"

#include <filesystem>
#include <string>

#include "libcacaoformats.hpp"
#include "libcacaoimage.hpp"

ExtractCmd::ExtractCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("extract", "Extract face images from a cubemap");

	//Input
	cmd->add_option("input", inPath, "Path to an packed cubemap file (.xjc) to read as input")->required()->check(CLI::ExistingFile);

	//Face settings
	CLI::Option* allOpt = cmd->add_flag("-A,--all-faces", doAll, "Extract all face images from the cubemap");
	std::vector<std::string> _f;
	CLI::Option* facesOpt = cmd->add_option("-f,--face", _f, "Extract specific faces from the cubemap (left, right, up, down, front, back)")->excludes(allOpt)->check([this](const std::string& s) {
		if(auto it = std::find_if(validFaces.cbegin(), validFaces.cend(), [s](const char* v) {
			   return std::string(v).compare(s) == 0;
		   });
			it != validFaces.cend()) {
			//We'll turn this into the index for later also
			faces.emplace_back(std::distance(std::begin(validFaces), it));
			return "";
		} else {
			return "Invalid face name!";
		}
	});
	allOpt->excludes(facesOpt);

	//Output directory
	CLI::Option* outOpt = cmd->add_option("-o", out, "Directory to place output files in")->check([](const std::string& outdir) {
		if(CLI::NonexistentPath(outdir).compare("") == 0) return "";
		if(CLI::ExistingDirectory(outdir).compare("") == 0) return "";
		return "The output directory must either be a directory to write into or a nonexistent directory!";
	});
	outOpt->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});

	//Command behavior
	cmd->callback([this]() {
		this->Callback();
	});
}

void ExtractCmd::Callback() {
	if(doAll) {
		for(uint8_t i = 0; i < 6; ++i) faces.emplace_back(i);
	}
	if(faces.size() <= 0) {
		CUBE_ERROR("No faces selected for extraction!")
	}
	std::filesystem::create_directories(out);

	//Load the input file
	VLOG_NONL("Opening input file " << inPath << "... ");
	std::ifstream in(inPath, std::ios::binary);
	if(!in.is_open()) {
		CUBE_ERROR("Failed to open input file stream for reading!")
	}
	VLOG("Done.")

	//Decode the file
	VLOG_NONL("Decoding input file... ")
	libcacaoformats::PackedDecoder dec;
	std::array<libcacaoimage::Image, 6> decoded;
	try {
		libcacaoformats::PackedContainer pc = libcacaoformats::PackedContainer::FromStream(in);
		decoded = dec.DecodeCubemap(pc);
	} catch(const std::runtime_error& e) {
		CUBE_ERROR(e.what());
	}
	VLOG("Done.")

	//Extract the requested faces
	std::string original = inPath.filename().stem().string();
	for(uint8_t i : faces) {
		//Open stream
		std::stringstream fname;
		fname << original << "_" << validFaces[i] << ".png";
		std::filesystem::path outfile = out / fname.str();
		VLOG("Opening output stream for " << validFaces[i] << ": " << outfile << "... ")
		std::ofstream outStream(outfile, std::ios::binary);
		if(!in.is_open()) {
			CUBE_ERROR("Failed to open output file stream for writing!")
		}
		VLOG("Done.")

		//Encode and write contents
		VLOG("Writing output... ")
		try {
			libcacaoimage::encode::EncodePNG(decoded[i], outStream);
		} catch(const std::runtime_error& e) {
			CUBE_ERROR(e.what())
		}
	}
}