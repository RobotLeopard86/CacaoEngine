#include "Bytestream.hpp"
#include "YAMLValidate.hpp"
#include "commands.hpp"

#include <exception>
#include <filesystem>
#include <string>

#include "libcacaoasset.hpp"
#include "libcacaoimage.hpp"

#include "spinners.hpp"

CreateCmd::CreateCmd(CLI::App& app) {
	//Create the command CLI
	cmd = app.add_subcommand("create", "Create a new cubemap");

	//Input
	cmd->add_option("input", inPath, "Input cubemap definition file (.ajc) path")->required()->check(CLI::ExistingFile)->transform([](const std::string& p) {
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
		std::stringstream taskDesc;
		taskDesc << "Creating cubemap from from " << inPath << "...";
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
			taskDesc << "Generated " << outPath << ".";
			s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
		}
	});
}

std::ifstream CreateCmd::OpenFile(const std::string& pathStr) {
	//We need to chdir to the path of the file to make the absolute calculation correct
	CVLOG_NONL("\tChecking file path " << pathStr << "... ")
	std::filesystem::path cur = std::filesystem::current_path();
	std::filesystem::current_path(inPath.parent_path());
	std::filesystem::path p = std::filesystem::absolute(pathStr);
	std::filesystem::current_path(cur);

	//Ensure the file exists
	if(!std::filesystem::exists(p)) {
		CUBE_ERROR("Input file specifies a nonexistent file!")
	}
	CVLOG("Done.")

	//Open the stream
	CVLOG_NONL("\tOpening file stream... ")
	std::ifstream in(p, std::ios::binary);
	if(!in.is_open()) {
		CUBE_ERROR("Failed to open face file stream for reading!")
	}
	CVLOG("Done.")
	return in;
}

void CreateCmd::Callback() {
	//Load the input file
	CVLOG_NONL("Opening input file " << inPath << "... ");
	std::ifstream in(inPath);
	if(!in.is_open()) {
		CUBE_ERROR("Failed to open input file stream for reading!")
	}
	CVLOG("Done.")

	//Decode the definition file
	CVLOG_NONL("Parsing definition...")
	YAML::Node root;
	try {
		root = YAML::Load(in);
	} catch(const std::exception& e) {
		CUBE_ERROR("Failed to parse definition: " << e.what());
	}
	YAML::Node front = root["front"];
	ValidateYAMLNode(front, YAML::NodeType::value::Scalar, "cubemap definition", "front (positive Z) face");
	YAML::Node back = root["back"];
	ValidateYAMLNode(back, YAML::NodeType::value::Scalar, "cubemap definition", "back (negative Z) face");
	YAML::Node top = root["top"];
	ValidateYAMLNode(top, YAML::NodeType::value::Scalar, "cubemap definition", "top (positive Y) face");
	YAML::Node bottom = root["bottom"];
	ValidateYAMLNode(bottom, YAML::NodeType::value::Scalar, "cubemap definition", "bottom (negative Y) face");
	YAML::Node left = root["left"];
	ValidateYAMLNode(left, YAML::NodeType::value::Scalar, "cubemap definition", "left (negative X) face");
	YAML::Node right = root["right"];
	ValidateYAMLNode(right, YAML::NodeType::value::Scalar, "cubemap definition", "right (positive X) face");
	CVLOG("Done.")

	//Load each face image
	CVLOG_SINGLE("Loading face images...");
	libcacaoimage::Image frontImg, backImg, topImg, bottomImg, leftImg, rightImg;
	{
		std::ifstream stream = OpenFile(front.Scalar());
		frontImg = libcacaoimage::decode::DecodeGeneric(stream);
	}
	{
		std::ifstream stream = OpenFile(back.Scalar());
		backImg = libcacaoimage::decode::DecodeGeneric(stream);
	}
	{
		std::ifstream stream = OpenFile(top.Scalar());
		topImg = libcacaoimage::decode::DecodeGeneric(stream);
	}
	{
		std::ifstream stream = OpenFile(bottom.Scalar());
		bottomImg = libcacaoimage::decode::DecodeGeneric(stream);
	}
	{
		std::ifstream stream = OpenFile(left.Scalar());
		leftImg = libcacaoimage::decode::DecodeGeneric(stream);
	}
	{
		std::ifstream stream = OpenFile(right.Scalar());
		rightImg = libcacaoimage::decode::DecodeGeneric(stream);
	}

	//Encode and write the file
	CVLOG_SINGLE("Generating output... ")
	libcacaoasset::Cubemap cmap;
	CVLOG_NONL("\tRe-encoding images...");
	{
		obytestream out(cmap.front);
		libcacaoimage::encode::EncodeWebP(frontImg, out);
	}
	{
		obytestream out(cmap.back);
		libcacaoimage::encode::EncodeWebP(backImg, out);
	}
	{
		obytestream out(cmap.top);
		libcacaoimage::encode::EncodeWebP(topImg, out);
	}
	{
		obytestream out(cmap.bottom);
		libcacaoimage::encode::EncodeWebP(bottomImg, out);
	}
	{
		obytestream out(cmap.left);
		libcacaoimage::encode::EncodeWebP(leftImg, out);
	}
	{
		obytestream out(cmap.right);
		libcacaoimage::encode::EncodeWebP(rightImg, out);
	}
	CVLOG("Done.")
	CVLOG_NONL("\tWriting output file " << outPath << "... ");
	std::ofstream out(outPath, std::ios::binary);
	if(!in.is_open()) {
		CUBE_ERROR("Failed to open output file stream for writing!")
	}
	libcacaoasset::EncodeCubemap(cmap, &out);
	CVLOG("Done.")
}