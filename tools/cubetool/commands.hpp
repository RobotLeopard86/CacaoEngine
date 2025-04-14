#pragma once

#include "CLI11.hpp"

#include "toolutil.hpp"

#include <filesystem>

#define CUBE_ERROR(...)                                                         \
	std::cerr << "\x1b[0m\x1b[1;91mERROR: \x1b[0m" << __VA_ARGS__ << std::endl; \
	exit(1);

class CreateCmd {
  public:
	CreateCmd(CLI::App&);
	void Callback();

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
	const std::array<const char*, 6> validFaces = {"right", "left", "up", "down", "front", "back"};
	std::vector<uint8_t> faces;
	std::filesystem::path inPath;
	std::filesystem::path out;
};