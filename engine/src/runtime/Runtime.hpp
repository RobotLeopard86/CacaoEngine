#pragma once

#include "Cacao/Engine.hpp"

#include "dynalo/dynalo.hpp"

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

void panic(const std::string& err, const std::string& hint);

inline class Runtime {
  public:
	Cacao::Engine::InitConfig icfg = {};
	std::unique_ptr<dynalo::library> gameBinary;

	struct Cacaospec {
		struct Meta {
			std::string pkgId;
			std::string title;
			std::string version;
		} meta;
		std::string binary;
	} cacaospec;

	void SetupEngine();
	void LoadGame();
	void DestroyGfxObjects();
	void Cleanup();
} rt;