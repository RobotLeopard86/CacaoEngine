#include "Bytestream.hpp"
#include "commands.hpp"

#include <exception>
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <string>

#include "libcacaoasset.hpp"
#include "libcacaoimage.hpp"

#include "spinners.hpp"

ExtractCmd::ExtractCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("extract", "Extract face images from a cubemap");

	//Input
	cmd->add_option("input", inPath, "Input cubemap file (.xjc) path")->required()->check(CLI::ExistingFile);

	//Face settings
	CLI::Option* allOpt = cmd->add_flag("-A,--all-faces", doAll, "Extract all face images from the cubemap");
	CLI::Option* facesOpt = cmd->add_option("-f,--face", _f, "Extract specific faces from the cubemap (front, back, top, bottom, left, right)")->excludes(allOpt)->check([this](const std::string& s) {
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
	CLI::Option* outOpt = cmd->add_option("-o", outPath, "Directory to place output files in")->check([](const std::string& outdir) {
		if(CLI::NonexistentPath(outdir).compare("") == 0) return "";
		if(CLI::ExistingDirectory(outdir).compare("") == 0) return "";
		return "The output directory must either be a directory to write into or a nonexistent directory!";
	});
	outOpt->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});

	//Format
	cmd->add_option("-F,--output-format", _fmt, "Output images in a specific format (png, jpeg, webp, tga, tiff); png is default")->default_val("png")->check([this](const std::string& s) {
		if(auto it = std::find_if(validFormats.cbegin(), validFormats.cend(), [s](const char* v) {
			   return std::string(v).compare(s) == 0;
		   });
			it != validFormats.cend()) {

			//We'll turn this into the index for later also
			format = std::distance(std::begin(validFormats), it);
			return "";
		} else {
			return "Invalid output format!";
		}
	});

	//Command behavior
	cmd->callback([this]() {
		std::stringstream taskDesc;
		taskDesc << "Extracting faces from from " << inPath << "...";
		std::unique_ptr<jms::Spinner> s;
		if(outputLvl != OutputLevel::Silent) {
			s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
			s->start();
		}
		taskDesc.str("");
		try {
			this->Callback();
		} catch(const std::exception& e) {
			if(outputLvl != OutputLevel::Silent) {
				taskDesc << "Failed to create cubemap: " << e.what();
				s->finish(jms::FinishedState::FAILURE, taskDesc.str());
			}
			exit(1);
		}
		if(outputLvl != OutputLevel::Silent) {
			taskDesc << "Extracted to " << outPath << ".";
			s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
		}
	});
}

void ExtractCmd::Callback() {
	if(doAll) {
		for(uint8_t i = 0; i < 6; ++i) faces.emplace_back(i);
	}
	if(faces.size() <= 0) {
		CUBE_ERROR("No faces selected for extraction!")
	}
	std::filesystem::create_directories(outPath);

	//Load the input file
	CVLOG_NONL("Opening input file " << inPath << "... ");
	std::ifstream* in = new std::ifstream(inPath, std::ios::binary);
	if(!in || !in->is_open()) {
		CUBE_ERROR("Failed to open input file stream for reading!")
	}
	CVLOG("Done.")

	//Decode the file
	CVLOG_NONL("Decoding input file... ")
	libcacaoasset::Cubemap cmap;
	try {
		cmap = libcacaoasset::DecodeCubemap(in);
	} catch(const std::runtime_error& e) {
		CUBE_ERROR(e.what());
	}
	CVLOG("Done.")

	//Extract the requested faces
	CVLOG_SINGLE("Extracting faces...")
	std::string original = inPath.filename().stem().string();
	for(uint8_t i : faces) {
		//Open stream
		std::stringstream fname;
		fname << original << "_" << validFaces[i] << "." << validFormats[format];
		std::filesystem::path outfile = outPath / fname.str();
		CVLOG_NONL("\tOpening output stream for " << validFaces[i] << " face: " << outfile << "... ")
		std::ofstream outStream(outfile, std::ios::binary);
		if(!outStream.is_open()) {
			CUBE_ERROR("Failed to open output file stream for writing!")
		}
		CVLOG("Done.")

		//Check for WebP (if so we can avoid re-encoding)
		if(format == 2) {
			//Direct copy to stream
			CVLOG_NONL("\tWriting output... ")
			switch(i) {
				case 0:
					std::copy(cmap.front.begin(), cmap.front.end(), std::ostreambuf_iterator<char>(outStream));
					break;
				case 1:
					std::copy(cmap.back.begin(), cmap.back.end(), std::ostreambuf_iterator<char>(outStream));
					break;
				case 2:
					std::copy(cmap.top.begin(), cmap.top.end(), std::ostreambuf_iterator<char>(outStream));
					break;
				case 3:
					std::copy(cmap.bottom.begin(), cmap.bottom.end(), std::ostreambuf_iterator<char>(outStream));
					break;
				case 4:
					std::copy(cmap.left.begin(), cmap.left.end(), std::ostreambuf_iterator<char>(outStream));
					break;
				case 5:
					std::copy(cmap.right.begin(), cmap.right.end(), std::ostreambuf_iterator<char>(outStream));
					break;
				default: throw std::runtime_error("impossible to get here");
			}
			CVLOG("Done.")
		} else {
			//We have to re-encode :(

			//Select image
			CVLOG_NONL("\tRe-encoding image... ")
			ibytestream encodedIn = [i, &cmap]() {
				switch(i) {
					case 0: return ibytestream(cmap.front);
					case 1: return ibytestream(cmap.back);
					case 2: return ibytestream(cmap.top);
					case 3: return ibytestream(cmap.bottom);
					case 4: return ibytestream(cmap.left);
					case 5: return ibytestream(cmap.right);
					default: throw std::runtime_error("impossible to get here");
				}
			}();
			libcacaoimage::Image img = libcacaoimage::decode::DecodeWebP(encodedIn);
			CVLOG("Done.")

			//Encode and write contents
			CVLOG_NONL("\tWriting output... ")
			try {
				switch(format) {
					case 0:
						libcacaoimage::encode::EncodePNG(img, outStream);
						break;
					case 1:
						libcacaoimage::encode::EncodeJPEG(img, outStream);
						break;
					case 3:
						libcacaoimage::encode::EncodeTGA(img, outStream);
						break;
					case 4:
						libcacaoimage::encode::EncodeTIFF(img, outStream);
						break;
				}
			} catch(const std::runtime_error& e) {
				CUBE_ERROR(e.what())
			}
			CVLOG("Done.")
		}
	}
}