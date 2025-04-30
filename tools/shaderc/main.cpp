#include "CLI11.hpp"
#include "spinners.hpp"

#include <vector>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#define SHADER_FILE_EXTENSION ".xjs"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif
#ifndef COMPILER_VER
#define COMPILER_VER "unknown"
#endif

#include "compiler.hpp"
#include "toolutil.hpp"

int main(int argc, char* argv[]) {
	//Configure CLI
	CLI::App app("Cacao Engine Shader Compiler", std::filesystem::path(argv[0]).filename().string());

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
        ss << "Compiler v" << COMPILER_VER << "\nFor Cacao Engine v" << CACAO_VER;
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
			std::filesystem::path out = autoOut / (cut + SHADER_FILE_EXTENSION);
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
	std::map<std::filesystem::path, std::filesystem::path> tasks;
	for(unsigned int i = 0; i < input.size(); i++) {
		if(!std::filesystem::exists(output[i].parent_path())) {
			std::filesystem::create_directories(output[i].parent_path());
		}
		tasks[input[i]] = output[i];
	}
	VLOG("Done.")

	//Compilation
	try {
		//Create compiler object
		CacaoShaderCompiler csc;

		//Compile
		std::unique_ptr<jms::Spinner> s;
		for(const auto& [in, out] : tasks) {
			std::stringstream taskDesc;
			taskDesc << "Compiling " << in << "...";
			if(outputLvl != OutputLevel::Silent) {
				s = std::make_unique<jms::Spinner>(taskDesc.str(), jms::dots);
				s->start();
			}
			auto [result, log] = csc.compile(in, out);
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
				ERROR("Failed to compile one or more shaders!")
				std::cerr << "====== ERROR LOG ======\n"
						  << log << "\n=======================" << std::endl;
				return 1;
			}
		}
	} catch(const std::exception& e) {
		ERROR("Exception caught during compilation: " << e.what())
		return 1;
	}

	if(outputLvl != OutputLevel::Silent) {
		std::cout << "Done." << std::endl;
	}

	return 0;
}