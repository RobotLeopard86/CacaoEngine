#include "CLI11.hpp"
#include "spinners.hpp"

#include <exception>
#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <string>

#include "toolutil.hpp"
#include "YAMLValidate.hpp"

#include "libcacaoasset.hpp"

#include "crossguid/guid.hpp"

#define WORLD_FILE_EXTENSION ".xjw"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

#ifndef CACAO_RELEASE_NICKNAME
#define CACAO_RELEASE_NICKNAME "Name TBD"
#endif

#ifndef COMPILER_VER
#define COMPILER_VER "unknown"
#endif

libcacaoasset::World parseWorldYML(std::istream& in) {
	//Load YAML
	YAML::Node root;
	try {
		root = YAML::Load(in);
	} catch(...) {
		CheckException(false, "Failed to parse material data stream!");
	}

	//Validate and parse structure
	libcacaoasset::World out;
	YAML::Node sky = root["skybox"];
	if(sky) {
		ValidateYAMLNode(sky, YAML::NodeType::value::Scalar, [](const YAML::Node& val) { return libcacaoasset::ValidateResourceAddress(val.Scalar(), libcacaoasset::Resource::Type::Tex2D) ? "" : "Invalid resource address for skybox"; }, "world data", "skybox asset path");
		out.skybox = sky.Scalar();
	} else {
		out.skybox = "";
	}
	YAML::Node cam = root["cam"];
	ValidateYAMLNode(cam, YAML::NodeType::value::Map, [&out](const YAML::Node& node) {
			YAML::Node p = node["position"], r = node["rotation"];
			try {
				if(!(p.IsMap() && p["x"].IsScalar() && p["y"].IsScalar() && p["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for camera initial position";
				out.initialCamPos.x = std::strtof(p["x"].Scalar().c_str(), nullptr);
				out.initialCamPos.y = std::strtof(p["y"].Scalar().c_str(), nullptr);
				out.initialCamPos.z = std::strtof(p["z"].Scalar().c_str(), nullptr);
				if(!(p.IsMap() && r["x"].IsScalar() && r["y"].IsScalar() && r["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for camera initial rotation";
				out.initialCamRot.x = std::strtof(r["x"].Scalar().c_str(), nullptr);
				out.initialCamRot.y = std::strtof(r["y"].Scalar().c_str(), nullptr);
				out.initialCamRot.z = std::strtof(r["z"].Scalar().c_str(), nullptr);
			} catch(...) {
				return "Non-float value found in camera initial state";
			}
			return ""; }, "world data", "initial camera state");
	ValidateYAMLNode(root["actors"], YAML::NodeType::value::Sequence, "world data", "actors list");
	for(const YAML::Node& e : root["actors"]) {
		libcacaoasset::World::Actor actor;

		YAML::Node name = e["name"];
		ValidateYAMLNode(name, YAML::NodeType::value::Scalar, "world actor data", "actor name");
		actor.name = name.Scalar();

		YAML::Node guid = e["guid"];
		ValidateYAMLNode(guid, YAML::NodeType::value::Scalar, [](const YAML::Node& node) {
				for(char c : node.Scalar()) {
					if(c == '-') continue;
					if(c < 48 || c > 102 || (c > 57 && c < 97)) {
						std::stringstream ss;
						ss << "Invalid GUID character \"" << c << "\"";
						return ss.str();
					}
				}
				return std::string(""); }, "world actor data", "GUID");
		actor.guid = xg::Guid(guid.Scalar()).bytes();

		YAML::Node parentGUID = e["parent"];
		ValidateYAMLNode(parentGUID, YAML::NodeType::value::Scalar, [](const YAML::Node& node) {
				for(char c : node.Scalar()) {
					if(c == '-') continue;
					if(c < 48 || c > 102 || (c > 57 && c < 97)) {
						std::stringstream ss;
						ss << "Invalid GUID character \"" << c << "\"";
						return ss.str();
					}
				}
				return std::string(""); }, "world actor data", "parent GUID");
		actor.parentGUID = xg::Guid(parentGUID.Scalar()).bytes();

		YAML::Node transform = e["transform"];
		ValidateYAMLNode(transform, YAML::NodeType::value::Map, "world actor data", "initial transform");

		YAML::Node pos = transform["position"];
		ValidateYAMLNode(pos, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["x"].IsScalar()) return "X value is not a scalar";
				if(!node["y"].IsScalar()) return "Y value is not a scalar";
				if(!node["z"].IsScalar()) return "Z value is not a scalar";
				return ""; }, "world actor transform", "position property");
		actor.initialPos.x = std::strtof(pos["x"].Scalar().c_str(), nullptr);
		actor.initialPos.y = std::strtof(pos["y"].Scalar().c_str(), nullptr);
		actor.initialPos.z = std::strtof(pos["z"].Scalar().c_str(), nullptr);

		YAML::Node rot = transform["rotation"];
		ValidateYAMLNode(rot, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["x"].IsScalar()) return "X value is not a scalar";
				if(!node["y"].IsScalar()) return "Y value is not a scalar";
				if(!node["z"].IsScalar()) return "Z value is not a scalar";
				return ""; }, "world actor transform", "rotation property");
		actor.initialRot.x = std::strtof(rot["x"].Scalar().c_str(), nullptr);
		actor.initialRot.y = std::strtof(rot["y"].Scalar().c_str(), nullptr);
		actor.initialRot.z = std::strtof(rot["z"].Scalar().c_str(), nullptr);

		YAML::Node scl = transform["scale"];
		ValidateYAMLNode(scl, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["x"].IsScalar()) return "X value is not a scalar";
				if(!node["y"].IsScalar()) return "Y value is not a scalar";
				if(!node["z"].IsScalar()) return "Z value is not a scalar";
				return ""; }, "world actor transform", "scale property");
		actor.initialScale.x = std::strtof(scl["x"].Scalar().c_str(), nullptr);
		actor.initialScale.y = std::strtof(scl["y"].Scalar().c_str(), nullptr);
		actor.initialScale.z = std::strtof(scl["z"].Scalar().c_str(), nullptr);

		YAML::Node components = e["components"];
		ValidateYAMLNode(components, YAML::NodeType::value::Sequence, "world actor", "component list");
		for(const YAML::Node& c : components) {
			libcacaoasset::World::Component component;

			YAML::Node id = c["id"];
			ValidateYAMLNode(id, YAML::NodeType::value::Scalar, "world actor component", "component ID");
			component.typeID = id.Scalar();

			YAML::Node rfl = c["rfl"];
			ValidateYAMLNode(rfl, [](const YAML::Node& node) { return (node.IsDefined() ? "" : "Reflection data doesn't exist"); }, "world actor component", "component reflection data");

			//TODO: Convert reflection data from YAML to binary via Astra

			actor.components.push_back(component);
		}

		out.actors.push_back(actor);
	}

	//Return result
	return out;
}

std::pair<bool, std::string> compile(const std::filesystem::path& inPath, const std::filesystem::path& out) {
	//Open input stream
	CVLOG_NONL("\tOpening input file " << inPath << "... ");
	std::ifstream input(inPath);
	CompileCheck(input.is_open(), "Failed to open source stream!");
	CVLOG("Done.")

	//Parse input file
	CVLOG_NONL("\tParsing world data... ");
	libcacaoasset::World w;
	try {
		w = parseWorldYML(input);
	} catch(const std::exception& e) {
		return {false, e.what()};
	}

	//Compile and write the output
	CVLOG_NONL("\tWriting output file " << out << "... ");
	std::ofstream outStream(out, std::ios::binary);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	libcacaoasset::EncodeWorld(w, &outStream);
	CVLOG("Done.");

	return {true, ""};
}

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine World Compiler", std::filesystem::path(argv[0]).filename().string());

	//Input arg
	std::vector<std::filesystem::path> input;
	app.add_option("input", input, "Input files to compile")->required()->check(CLI::ExistingFile);

	//Output args
	std::vector<std::filesystem::path> output;
	CLI::Option* outOpt = app.add_option("-o", output, "Compilation output files")->check([](const std::string& outfile) {
		if(CLI::NonexistentPath(outfile).compare("") == 0) return "";
		if(CLI::ExistingFile(outfile).compare("") == 0) return "";
		return "Output files must either be files to overwrite or nonexistent files!";
	});
	outOpt->transform([](const std::string& p) {
		std::filesystem::path path(p);
		return std::filesystem::absolute(path).string();
	});
	std::filesystem::path autoOut;
	CLI::Option* autoOutOpt = app.add_option("-A,--auto-output", autoOut, "Automatically generate output files and place them in the specified directory")->excludes(outOpt)->check([](const std::string& outfile) {
		if(CLI::NonexistentPath(outfile).compare("") == 0) return "";
		if(CLI::ExistingDirectory(outfile).compare("") == 0) return "";
		return "Auto-output directories must either be directories to write into or nonexistent directories!";
	});
	outOpt->excludes(autoOutOpt);

	//Output control
	outputLvl = OutputLevel::Normal;
	app.add_flag_callback("-q,--quiet", []() { outputLvl = OutputLevel::Silent; }, "Suppress all output from the compiler");
	app.add_flag_callback("-V,--verbose", []() { outputLvl = OutputLevel::Verbose; }, "Enable verbose output from the compiler");

	//Version arg
	app.set_version_flag("-v,--version", []() {
        std::stringstream ss;
        ss << "Compiler v" << COMPILER_VER << "\nFor Cacao Engine v" << CACAO_VER << " (" << CACAO_RELEASE_NICKNAME << ")";
        return ss.str(); }, "Show version info and exit");

	//Parse the CLI
	CLI11_PARSE(app, argc, argv);

	//Calculate auto-output paths if requested
	if(app.count("-A") > 0) {
		for(auto& in : input) {
			std::string inStr = in.filename().string();
			std::string cut = inStr;
			if(auto period = inStr.find("."); period != std::string::npos) {
				cut = inStr.substr(0, period);
			}
			std::filesystem::path out = autoOut / (cut + WORLD_FILE_EXTENSION);
			VLOG("Will compile " << in << " -> " << out)
			output.push_back(out);
		}
		if(!std::filesystem::exists(autoOut)) {
			std::filesystem::create_directories(autoOut);
		}
	} else if(input.size() != output.size()) {
		ERROR("Input and output file counts do not match!")
		return 1;
	}

	//Merge input and output vectors
	VLOG_NONL("Preparing tasks list... ")
	std::unordered_map<std::filesystem::path, std::filesystem::path> tasks;
	for(unsigned int i = 0; i < input.size(); ++i) {
		if(!std::filesystem::exists(output[i].parent_path())) {
			std::filesystem::create_directories(output[i].parent_path());
		}
		tasks[input[i]] = output[i];
	}
	VLOG("Done.")

	//Compile
	std::unique_ptr<jms::Spinner> s;
	for(const auto& [in, out] : tasks) {
		std::stringstream taskDesc;
		taskDesc << "Compiling " << in << "...";
		if(outputLvl != OutputLevel::Silent) {
			s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
			s->start();
		}
		auto [result, log] = compile(in, out);
		if(outputLvl != OutputLevel::Silent) {
			taskDesc.str("");
			if(result) {
				taskDesc << "Compiled " << in << ".";
				s->finish(jms::FinishedState::SUCCESS, taskDesc.str());
			} else {
				taskDesc << "Failed to compile " << in << ".";
				s->finish(jms::FinishedState::FAILURE, taskDesc.str());
			}
		}
		if(!result) {
			ERROR("Failed to compile one or more worlds: " << log)
			return 1;
		}
	}

	if(outputLvl != OutputLevel::Silent) {
		std::cout << "Done." << std::endl;
	}

	return 0;
}