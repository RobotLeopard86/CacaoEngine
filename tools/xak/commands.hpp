#pragma once

#include "CLI11.hpp"

#include "toolutil.hpp"

#define PACK_FILE_EXTENSION ".xak"

#include <filesystem>

inline bool fail = false;

#define XAK_ERROR(...) \
	ERROR(__VA_ARGS__) \
	fail = true;       \
	return;

#define XAK_ERROR_NONVOID(r, ...) \
	ERROR(__VA_ARGS__)            \
	fail = true;                  \
	return r;

class CreateCmd {
  public:
	CreateCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
	std::filesystem::path assetRoot;
	std::filesystem::path resRoot;
	std::filesystem::path outPath;
	std::filesystem::path addrMapPath;
};

class ListCmd {
  public:
	ListCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
	std::filesystem::path inPath;
	bool doAssets, doResources, assetMeta;
};

class ExtractCmd {
  public:
	ExtractCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
	std::filesystem::path inPath;
	std::filesystem::path outDir;
	std::vector<std::string> toExtract;
	bool all;
};

class MergeCmd {
  public:
	MergeCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
	std::vector<std::filesystem::path> inPaks;
	std::filesystem::path outPath;
};

class DelCmd {
  public:
	DelCmd(CLI::App&);
	void Callback();

  private:
	CLI::App* cmd;
	std::filesystem::path inPath;
	std::vector<std::string> toDelete;
};