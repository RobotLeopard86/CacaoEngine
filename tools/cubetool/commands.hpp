#pragma once

#include "CLI11.hpp"

#include "toolutil.hpp"

#include <filesystem>

#define CUBE_ERROR(...)                                                         \
	std::cerr << "\x1b[0m\x1b[1;91mERROR: \x1b[0m" << __VA_ARGS__ << std::endl; \
	throw std::runtime_error("no");

class CreateCmd {
  public:
	CreateCmd(CLI::App&);
	void Callback();
	std::ifstream OpenFile(const std::string& pathStr);

  private:
	CLI::App* cmd;
	std::filesystem::path inPath;
	std::filesystem::path outPath;
};

class ExtractCmd {
  public:
	ExtractCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
	bool doAll;

	const std::array<const char*, 6> validFaces = {"front", "back", "top", "bottom", "left", "right"};
	const std::array<const char*, 5> validFormats = {"png", "jpeg", "webp", "tga", "tiff"};

	std::vector<std::string> _f;//Ignore these; we use the integer versions below that get converted but we have to save these
	std::string _fmt;			//Ignore these; we use the integer versions below that get converted but we have to save these
	std::vector<uint8_t> faces;
	uint8_t format = 0;
	std::filesystem::path inPath;
	std::filesystem::path outPath;
};