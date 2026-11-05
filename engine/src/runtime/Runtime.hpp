#pragma once

#include "Cacao/Engine.hpp"

#include "dynalo/dynalo.hpp"
#include "astra/setup.hpp"

#include <memory>
#include <unordered_map>

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

#ifndef CACAO_RELEASE_NICKNAME
#define CACAO_RELEASE_NICKNAME "Name TBD"
#endif

using namespace Cacao;

void panic(const std::string& err, const std::string& hint);

inline class Runtime {
  public:
	Engine::InitConfig icfg = {};
	std::unique_ptr<dynalo::library> gameBinary;
	std::unordered_map<std::string, std::string> worldScan;
	std::unordered_map<std::string, std::string> resourceScan;

	struct ASTRA_REFLECT Cacaospec : public AstraReflectBase {
		struct ASTRA_REFLECT Meta : public AstraReflectBase {
			std::string pkgId;
			std::string title;
			std::string version;

			ASTRASETUP(Meta)
			virtual ~Meta() {}
		};

		std::string binary;
		std::string startupWorld;
		Meta meta;

		ASTRASETUP(Cacaospec)
		virtual ~Cacaospec() {}
	} cacaospec;

	void SetupEngine();
	void LoadGame();
	void DestroyGfxObjects();
	void Cleanup();
} rt;

void CfgLoader();