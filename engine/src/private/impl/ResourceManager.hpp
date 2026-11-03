#pragma once

#include "Cacao/ResourceManager.hpp"
#include "Cacao/Model.hpp"

#include <unordered_map>
#include <set>
#include <mutex>

namespace Cacao {
	struct ResourceManager::Impl {
		//Resource cache
		std::mutex cacheProtector;
		std::unordered_map<std::string, std::weak_ptr<Resource>> cache;

		//Configured loaders
		std::unordered_map<std::type_index, ErasedLoader> loaders;

		//Model loading data
		struct ModelData {
			std::mutex mutex;
			std::shared_future<std::shared_ptr<Model>> modelFuture;
			std::shared_ptr<Model> mdl;
			std::set<std::string> loadedMeshes;
			std::set<std::string> loadedTextures;
		};
		std::mutex modelProtector;
		std::unordered_map<std::string, std::shared_ptr<ModelData>> models;

		//Built-in assets
		std::atomic_bool builtinsReady = false;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> builtinMeshes;
		std::unordered_map<std::string, std::shared_ptr<Shader>> builtinShaders;
	};
}