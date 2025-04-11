#include "libcacaoformats.hpp"
#include "CLI11.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif
#ifndef XJSX_VER
#define XJSX_VER "unknown"
#endif

int main(int argc, char* argv[]) {
	try {
		CLI::App app("Cacao Engine Shader Extractor", std::filesystem::path(argv[0]).filename().string());
		std::filesystem::path input;
		app.add_option("input", input, "Input shader file")->required()->check(CLI::ExistingFile);
		std::filesystem::path out;
		CLI::Option* outOpt = app.add_option("-o", out, "Output file prefix")->required()->check([](const std::string& outfile) {
			if(CLI::NonexistentPath(outfile).compare("") == 0) return "";
			if(CLI::ExistingFile(outfile).compare("") == 0) return "";
			return "The output file must either be a file to overwrite or a nonexistent file!";
		});
		outOpt->transform([](const std::string& p) {
			std::filesystem::path path(p);
			std::string abs = std::filesystem::absolute(path).string();
			if(abs.ends_with("/")) abs = abs.substr(0, abs.size() - 2);
			return abs;
		});
		app.set_version_flag("-v,--version", []() {
			std::stringstream ss;
			ss << "Extractor v" << XJSX_VER << "\nFor Cacao Engine v" << CACAO_VER;
			return ss.str(); }, "Show version info and exit");

		CLI11_PARSE(app, argc, argv);

		std::ifstream inStream(input);
		if(!inStream.is_open()) {
			std::cerr << "Failed to open input stream!" << std::endl;
			return 1;
		}
		libcacaoformats::PackedContainer container = libcacaoformats::PackedContainer::FromStream(inStream);

		libcacaoformats::PackedDecoder dec;
		libcacaoformats::Shader shader = dec.DecodeShader(container);

		switch(shader.type) {
			case libcacaoformats::Shader::CodeType::SPIRV: {
				std::ofstream outStream(out.replace_extension(".spv"), std::ios::binary);
				if(!outStream.is_open()) {
					std::cerr << "Failed to open output stream!" << std::endl;
					return 1;
				}
				libcacaoformats::Shader::SPIRVCode code = std::get<libcacaoformats::Shader::SPIRVCode>(shader.code);
				outStream.write(reinterpret_cast<char*>(code.data()), (code.size() * 4));
				outStream.close();
				break;
			}
			case libcacaoformats::Shader::CodeType::GLSL: {
				libcacaoformats::Shader::GLSLCode code = std::get<libcacaoformats::Shader::GLSLCode>(shader.code);
				std::ofstream outVStream(out.replace_extension(".vert"));
				if(!outVStream.is_open()) {
					std::cerr << "Failed to open output stream!" << std::endl;
					return 1;
				}
				outVStream << code.vertex;
				outVStream.close();
				std::ofstream outFStream(out.replace_extension(".frag"));
				if(!outFStream.is_open()) {
					std::cerr << "Failed to open output stream!" << std::endl;
					return 1;
				}
				outFStream << code.fragment;
				outFStream.close();
				break;
			}
			default: return 1;
		}
		return 0;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
}