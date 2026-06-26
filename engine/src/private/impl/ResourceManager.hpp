#pragma once

#include "Cacao/ResourceManager.hpp"

#include <unordered_map>
#include <set>

namespace Cacao {
	struct ResourceManager::Impl {
		std::unordered_map<std::string, std::weak_ptr<Resource>> cache;
		std::unordered_map<std::type_index, ErasedLoader> loaders;
		struct ModelData {
			std::shared_ptr<Model> mdl;
			std::set<std::string> loadedMeshes;
			std::set<std::string> loadedTextures;
		};
		std::unordered_map<std::string, ModelData> models;
	};
}