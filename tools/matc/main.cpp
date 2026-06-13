#include "CLI11.hpp"
#include "libjaguar/MathTypes.hpp"
#include "spinners.hpp"

#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <string>

#include "toolutil.hpp"
#include "YAMLValidate.hpp"

#include "libcacaoasset.hpp"

#define MAT_FILE_EXTENSION ".xjm"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

#ifndef CACAO_RELEASE_NICKNAME
#define CACAO_RELEASE_NICKNAME "Name TBD"
#endif

#ifndef COMPILER_VER
#define COMPILER_VER "unknown"
#endif

enum class ScalarKind {
	Neutral,
	Float,
	SignedInt,
	UnsignedInt
};

ScalarKind classifyScalar(const std::string& s) {
	if(s == "true" || s == "false") return ScalarKind::Neutral;
	if(s.back() == 'u') return ScalarKind::UnsignedInt;
	if(s.find('.') != std::string::npos ||
		s.find('e') != std::string::npos ||
		s.find('E') != std::string::npos) return ScalarKind::Float;
	if(s.front() == '-') return ScalarKind::SignedInt;
	return ScalarKind::Neutral;
}

std::string stripUnsignedSuffix(const std::string& s) {
	return (s.back() == 'u') ? s.substr(0, s.size() - 1) : s;
}

ScalarKind identifyScalar(const YAML::Node& seq, const std::string& context) {
	ScalarKind committed = ScalarKind::Neutral;
	for(std::size_t i = 0; i < seq.size(); ++i) {
		ScalarKind k = classifyScalar(seq[i].as<std::string>());
		if(k == ScalarKind::Neutral) continue;
		if(committed == ScalarKind::Neutral) {
			committed = k;
			continue;
		}
		CheckException(committed == k,
			std::format("While parsing {}, conflicting numeric types in sequence at index {}", context, i));
	}
	return (committed == ScalarKind::Neutral) ? ScalarKind::SignedInt : committed;
}

libcacaoasset::Material::Storage parseVec(const YAML::Node& seq, const std::string& context) {
	const std::size_t N = seq.size();
	CheckException(N >= 2 && N <= 4,
		std::format("While parsing {}, vector must have 2–4 components, got {}", context, N));

	const ScalarKind kind = identifyScalar(seq, context);
	switch(kind) {
		case ScalarKind::Float: {
			auto get = [&](std::size_t i) { return std::stof(seq[i].as<std::string>()); };
			if(N == 2) return libjaguar::Vector<float, 2> {{get(0)}, {get(1)}};
			if(N == 3) return libjaguar::Vector<float, 3> {{get(0)}, {get(1)}, {get(2)}};
			return libjaguar::Vector<float, 4> {{get(0)}, {get(1)}, {get(2)}, {get(3)}};
			break;
		}
		case ScalarKind::UnsignedInt: {
			auto get = [&](std::size_t i) { return static_cast<unsigned int>(std::stoul(stripUnsignedSuffix(seq[i].as<std::string>()))); };
			if(N == 2) return libjaguar::Vector<unsigned int, 2> {{get(0)}, {get(1)}};
			if(N == 3) return libjaguar::Vector<unsigned int, 3> {{get(0)}, {get(1)}, {get(2)}};
			return libjaguar::Vector<unsigned int, 4> {{get(0)}, {get(1)}, {get(2)}, {get(3)}};
			break;
		}
		case ScalarKind::SignedInt: {
			auto get = [&](std::size_t i) { return std::stoi(seq[i].as<std::string>()); };
			if(N == 2) return libjaguar::Vector<int, 2> {{get(0)}, {get(1)}};
			if(N == 3) return libjaguar::Vector<int, 3> {{get(0)}, {get(1)}, {get(2)}};
			return libjaguar::Vector<int, 4> {{get(0)}, {get(1)}, {get(2)}, {get(3)}};
			break;
		}
		default: throw std::runtime_error("impossible to get here");
	}
}

libcacaoasset::Material::Storage parseMatrix(const YAML::Node& rows, const std::string& context) {
	const std::size_t N = rows.size();
	CheckException(N >= 2 && N <= 4,
		std::format("While parsing {}, matrix must be 2x2, 3x3, or 4x4, got {} rows", context, N));
	for(std::size_t r = 0; r < N; ++r) {
		ValidateYAMLNode(rows[r], YAML::NodeType::Sequence, context,
			std::format("matrix row {}", r));
		CheckException(rows[r].size() == N,
			std::format("While parsing {}, matrix row {} has {} columns, expected {}",
				context, r, rows[r].size(), N));
	}
	ScalarKind committed = ScalarKind::Neutral;
	for(std::size_t r = 0; r < N; ++r) {
		for(std::size_t c = 0; c < N; ++c) {
			ScalarKind k = classifyScalar(rows[r][c].as<std::string>());
			if(k == ScalarKind::Neutral) continue;
			if(committed == ScalarKind::Neutral) {
				committed = k;
				continue;
			}
			CheckException(committed == k,
				std::format("While parsing {}, conflicting numeric types in matrix at [{},{}]",
					context, r, c));
		}
	}
	CheckException(committed == ScalarKind::Float || committed == ScalarKind::Neutral,
		std::format("While parsing {}, only float matrices are supported", context));
	auto get = [&](std::size_t r, std::size_t c) {
		return std::stof(rows[r][c].as<std::string>());
	};
	if(N == 2) {
		libjaguar::Matrix<float, 2, 2> m;
		for(std::size_t r = 0; r < 2; ++r)
			for(std::size_t c = 0; c < 2; ++c)
				m[c][r] = get(r, c);
		return m;
	} else if(N == 3) {
		libjaguar::Matrix<float, 3, 3> m;
		for(std::size_t r = 0; r < 3; ++r)
			for(std::size_t c = 0; c < 3; ++c)
				m[c][r] = get(r, c);
		return m;
	} else if(N == 4) {
		libjaguar::Matrix<float, 4, 4> m;
		for(std::size_t r = 0; r < 4; ++r)
			for(std::size_t c = 0; c < 4; ++c)
				m[c][r] = get(r, c);
		return m;
	}
	throw std::runtime_error("impossible to get here");
}

libcacaoasset::Material::Storage parseStorage(const YAML::Node& node, const std::string& context) {
	if(node.IsScalar()) {
		const std::string s = node.as<std::string>();
		if(s == "true") return true;
		if(s == "false") return false;

		switch(classifyScalar(s)) {
			case ScalarKind::Float: return std::stof(s);
			case ScalarKind::UnsignedInt: return static_cast<unsigned int>(std::stoul(stripUnsignedSuffix(s)));
			default: return std::stoi(s);//SignedInt or Neutral
		}
	}

	if(node.IsSequence()) {
		CheckException(node.size() > 0,
			std::format("While parsing {}, sequence must not be empty", context));
		if(node[0].IsSequence()) return parseMatrix(node, context);
		return parseVec(node, context);
	}

	if(node.IsMap()) {
		ValidateYAMLNode(node["address"], YAML::NodeType::Scalar, context, "TexRef.address");
		ValidateYAMLNode(node["isCubemap"], YAML::NodeType::Scalar, context, "TexRef.isCubemap");

		const std::string cb = node["isCubemap"].as<std::string>();
		CheckException(cb == "true" || cb == "false",
			std::format("While parsing {}, TexRef.isCubemap must be true or false, got '{}'", context, cb));

		return libcacaoasset::Material::TexRef {
			.address = node["address"].as<std::string>(),
			.isCubemap = (cb == "true"),
		};
	}

	throw std::runtime_error(
		std::format("While parsing {}, storage value is not a scalar, sequence, or map", context));
}

libcacaoasset::Material parseMaterialYML(const YAML::Node& root, const std::string& context) {
	ValidateYAMLNode(root, YAML::NodeType::Map, context, "material root");

	ValidateYAMLNode(root["shaderAddress"], YAML::NodeType::Scalar, context, "shaderAddress");
	ValidateYAMLNode(root["transparency"], YAML::NodeType::Scalar, [](const YAML::Node& node) {
		try {
			std::string val = node.as<std::string>();
			std::transform(val.begin(), val.end(), val.begin(), ::tolower);
			if(val.compare("opaque") == 0) return "";
			if(val.compare("cutout") == 0) return "";
			if(val.compare("transparent") == 0) return "";
			return "Invalid transparency value!";
		} catch(...) {
			return "Not a string!";
		} }, context, "transparency");

	ValidateYAMLNode(root["parameters"], YAML::NodeType::Sequence, context, "parameters");
	const YAML::Node& params = root["parameters"];

	libcacaoasset::Material mat;
	mat.shaderAddress = root["shaderAddress"].as<std::string>();
	mat.parameters.reserve(params.size());

	std::string tval = root["transparency"].as<std::string>();
	std::transform(tval.begin(), tval.end(), tval.begin(), ::tolower);
	if(tval.compare("opaque") == 0)
		mat.renderMode = libcacaoasset::Material::RenderMode::Opaque;
	else if(tval.compare("cutout") == 0)
		mat.renderMode = libcacaoasset::Material::RenderMode::Cutout;
	else if(tval.compare("transparent") == 0)
		mat.renderMode = libcacaoasset::Material::RenderMode::Transparent;

	for(std::size_t i = 0; i < params.size(); ++i) {
		const std::string paramCtx = std::format("{} parameter[{}]", context, i);
		const YAML::Node& p = params[i];

		ValidateYAMLNode(p, YAML::NodeType::Map, context, std::format("parameter[{}]", i));
		ValidateYAMLNode(p["target"], YAML::NodeType::Scalar, context,
			std::format("parameter[{}].target", i));
		ValidateYAMLNode(p["value"], [](const YAML::Node& n) -> std::string { return (n.IsScalar() || n.IsSequence() || n.IsMap()) ? "" : "must be scalar, sequence, or map"; }, context, std::format("parameter[{}].value", i));

		mat.parameters.push_back(libcacaoasset::Material::Param {
			.target = p["target"].as<std::string>(),
			.storage = parseStorage(p["value"], std::format("{}.{}", paramCtx, p["target"].as<std::string>())),
		});
	}

	return mat;
}

std::pair<bool, std::string> compile(const std::filesystem::path& inPath, const std::filesystem::path& out) {
	//Open input stream
	CVLOG_NONL("\tOpening input file " << inPath << "... ");
	std::ifstream input(inPath);
	CompileCheck(input.is_open(), "Failed to open source stream!");
	CVLOG("Done.")

	//Parse input file
	CVLOG_NONL("\tParsing world data... ");
	YAML::Node root;
	try {
		root = YAML::Load(input);
	} catch(...) {
		CheckException(false, "Failed to parse material data stream!");
	}
	libcacaoasset::Material m;
	try {
		m = parseMaterialYML(root, "material");
	} catch(const std::exception& e) {
		return {false, e.what()};
	}
	CVLOG("Done.")

	//Compile and write the output
	CVLOG_NONL("\tWriting output file " << out << "... ");
	std::ofstream outStream(out, std::ios::binary);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	try {
		libcacaoasset::EncodeMaterial(m, &outStream);
	} catch(const std::exception& e) {
		return {false, e.what()};
	}
	CVLOG("Done.");

	return {true, ""};
}

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine Material Compiler", std::filesystem::path(argv[0]).filename().string());

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
        ss << "Material Compiler v" << COMPILER_VER << "\nFor Cacao Engine v" << CACAO_VER << " (" << CACAO_RELEASE_NICKNAME << ")";
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
			std::filesystem::path out = autoOut / (cut + MAT_FILE_EXTENSION);
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
			ERROR("Failed to compile one or more materials: " << log)
			return 1;
		}
	}

	if(outputLvl != OutputLevel::Silent) {
		std::cout << "Done." << std::endl;
	}

	return 0;
}