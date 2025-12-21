#pragma once

#include "Cacao/ResourceManager.hpp"

#include <unordered_map>

namespace Cacao {
	struct ResourceManager::Impl {
		std::unordered_map<std::string, std::weak_ptr<Resource>> cache;
		std::unordered_map<std::type_index, ErasedLoader> loaders;
	};
}