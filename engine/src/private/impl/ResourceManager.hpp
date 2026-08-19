#pragma once

#include "Cacao/ResourceManager.hpp"
#include "Cacao/Model.hpp"

#include <unordered_map>
#include <set>

namespace Cacao {
	struct ResourceManager::Impl {
		//Resource cache
		std::mutex cacheProtector;
		std::unordered_map<std::string, std::weak_ptr<Resource>> cache;

		//Configured loaders
		std::unordered_map<std::type_index, ErasedLoader> loaders;

		//Model loading data
		struct ModelData {
			std::shared_ptr<Model> mdl;
			std::set<std::string> loadedMeshes;
			std::set<std::string> loadedTextures;
		};
		std::unordered_map<std::string, ModelData> models;

		//Built-in assets
		bool builtinsReady = false;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> builtinMeshes;
		std::unordered_map<std::string, std::shared_ptr<Shader>> builtinShaders;
	};
}