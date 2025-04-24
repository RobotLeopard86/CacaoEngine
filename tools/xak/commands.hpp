#pragma once

#include "CLI11.hpp"

#include "toolutil.hpp"

#define PACK_FILE_EXTENSION ".xak"

#include <filesystem>

#define XAK_ERROR(...)                                                          \
	std::cerr << "\x1b[0m\x1b[1;91mERROR: \x1b[0m" << __VA_ARGS__ << std::endl; \
	exit(1);

class CreateCmd {
  public:
	CreateCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
};